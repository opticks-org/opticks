/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LatLonStyleComboBox.h"

#include "AppVerify.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

using namespace std;

LatLonStyleComboBox::LatLonStyleComboBox(QWidget* pParent)
: QComboBox(pParent)
{
   setEditable(false);

   addItem(QString::fromStdString(StringUtilities::toDisplayString(LATLONSTYLE_SOLID)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(LATLONSTYLE_DASHED)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(LATLONSTYLE_CROSS)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(LATLONSTYLE_NONE)));

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

void LatLonStyleComboBox::setCurrentValue(LatLonStyle value)
{
   QString strValue = QString::fromStdString(StringUtilities::toDisplayString(value));
   int index = findText(strValue);
   setCurrentIndex(index);
}

LatLonStyle LatLonStyleComboBox::getCurrentValue() const
{
   LatLonStyle retValue;
   int index = currentIndex();
   if (index != -1)
   {
      string curText = currentText().toStdString();
      retValue = StringUtilities::fromDisplayString<LatLonStyle>(curText);
   }
   return retValue;
}

void LatLonStyleComboBox::translateActivated(int newIndex)
{
   if (newIndex != -1)
   {
      string curText = currentText().toStdString();
      LatLonStyle curType = StringUtilities::fromDisplayString<LatLonStyle>(curText);
      emit valueChanged(curType);
   }
}
