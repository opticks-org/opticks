/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FillStyleComboBox.h"

#include "AppVerify.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

using namespace std;

FillStyleComboBox::FillStyleComboBox(QWidget* pParent)
: QComboBox(pParent)
{
   setEditable(false);

   addItem(QString::fromStdString(StringUtilities::toDisplayString(SOLID_FILL)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(HATCH)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(EMPTY_FILL)));

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

void FillStyleComboBox::setCurrentValue(FillStyle value)
{
   QString strValue = QString::fromStdString(StringUtilities::toDisplayString(value));
   int index = findText(strValue);
   setCurrentIndex(index);
}

FillStyle FillStyleComboBox::getCurrentValue() const
{
   FillStyle retValue;
   int index = currentIndex();
   if (index != -1)
   {
      string curText = currentText().toStdString();
      retValue = StringUtilities::fromDisplayString<FillStyle>(curText);
   }
   return retValue;
}

void FillStyleComboBox::translateActivated(int newIndex)
{
   if (newIndex != -1)
   {
      string curText = currentText().toStdString();
      FillStyle curType = StringUtilities::fromDisplayString<FillStyle>(curText);
      emit valueChanged(curType);
   }
}