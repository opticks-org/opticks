/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ComplexComponentComboBox.h"

#include "AppVerify.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

using namespace std;

ComplexComponentComboBox::ComplexComponentComboBox(QWidget* pParent)
: QComboBox(pParent)
{
   setEditable(false);

   addItem(QString::fromStdString(StringUtilities::toDisplayString(COMPLEX_MAGNITUDE)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(COMPLEX_PHASE)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(COMPLEX_INPHASE)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(COMPLEX_QUADRATURE)));

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

void ComplexComponentComboBox::setCurrentValue(ComplexComponent value)
{
   QString strValue = QString::fromStdString(StringUtilities::toDisplayString(value));
   int index = findText(strValue);
   setCurrentIndex(index);
}

ComplexComponent ComplexComponentComboBox::getCurrentValue() const
{
   ComplexComponent retValue;
   int index = currentIndex();
   if (index != -1)
   {
      string curText = currentText().toStdString();
      retValue = StringUtilities::fromDisplayString<ComplexComponent>(curText);
   }
   return retValue;
}

void ComplexComponentComboBox::translateActivated(int newIndex)
{
   if (newIndex != -1)
   {
      string curText = currentText().toStdString();
      ComplexComponent curType = StringUtilities::fromDisplayString<ComplexComponent>(curText);
      emit valueChanged(curType);
   }
}