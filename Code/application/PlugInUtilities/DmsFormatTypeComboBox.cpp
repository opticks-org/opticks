/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DmsFormatTypeComboBox.h"

#include "AppVerify.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

using namespace std;

DmsFormatTypeComboBox::DmsFormatTypeComboBox(QWidget* pParent)
: QComboBox(pParent)
{
   setEditable(false);

   addItem(QString::fromStdString(StringUtilities::toDisplayString(DMS_FULL)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(DMS_FULL_DECIMAL)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(DMS_MINUTES_DECIMAL)));

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

void DmsFormatTypeComboBox::setCurrentValue(DmsFormatType value)
{
   QString strValue = QString::fromStdString(StringUtilities::toDisplayString(value));
   int index = findText(strValue);
   setCurrentIndex(index);
}

DmsFormatType DmsFormatTypeComboBox::getCurrentValue() const
{
   DmsFormatType retValue;
   int index = currentIndex();
   if (index != -1)
   {
      string curText = currentText().toStdString();
      retValue = StringUtilities::fromDisplayString<DmsFormatType>(curText);
   }
   return retValue;
}

void DmsFormatTypeComboBox::translateActivated(int newIndex)
{
   if (newIndex != -1)
   {
      string curText = currentText().toStdString();
      DmsFormatType curType = StringUtilities::fromDisplayString<DmsFormatType>(curText);
      emit valueChanged(curType);
   }
}