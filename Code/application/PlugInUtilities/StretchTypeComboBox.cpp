/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "StretchTypeComboBox.h"

#include "AppVerify.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

using namespace std;

StretchTypeComboBox::StretchTypeComboBox(QWidget* pParent)
: QComboBox(pParent)
{
   setEditable(false);

   addItem(QString::fromStdString(StringUtilities::toDisplayString(LINEAR)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(EXPONENTIAL)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(LOGARITHMIC)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(EQUALIZATION)));

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

void StretchTypeComboBox::setCurrentValue(StretchType value)
{
   QString strValue = QString::fromStdString(StringUtilities::toDisplayString(value));
   int index = findText(strValue);
   setCurrentIndex(index);
}

StretchType StretchTypeComboBox::getCurrentValue() const
{
   StretchType retValue;
   int index = currentIndex();
   if (index != -1)
   {
      string curText = currentText().toStdString();
      retValue = StringUtilities::fromDisplayString<StretchType>(curText);
   }
   return retValue;
}

void StretchTypeComboBox::translateActivated(int newIndex)
{
   if (newIndex != -1)
   {
      string curText = currentText().toStdString();
      StretchType curType = StringUtilities::fromDisplayString<StretchType>(curText);
      emit valueChanged(curType);
   }
}