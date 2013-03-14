/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "RegionUnitsComboBox.h"

#include "AppVerify.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

using namespace std;

RegionUnitsComboBox::RegionUnitsComboBox(QWidget* pParent)
: QComboBox(pParent)
{
   setEditable(false);

   addItem(QString::fromStdString(StringUtilities::toDisplayString(RAW_VALUE)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(PERCENTAGE)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(PERCENTILE)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(STD_DEV)));

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

void RegionUnitsComboBox::setCurrentValue(RegionUnits value)
{
   QString strValue = QString::fromStdString(StringUtilities::toDisplayString(value));
   int index = findText(strValue);
   setCurrentIndex(index);
}

RegionUnits RegionUnitsComboBox::getCurrentValue() const
{
   RegionUnits retValue;
   int index = currentIndex();
   if (index != -1)
   {
      string curText = currentText().toStdString();
      retValue = StringUtilities::fromDisplayString<RegionUnits>(curText);
   }
   return retValue;
}

void RegionUnitsComboBox::translateActivated(int newIndex)
{
   if (newIndex != -1)
   {
      string curText = currentText().toStdString();
      RegionUnits curType = StringUtilities::fromDisplayString<RegionUnits>(curText);
      emit valueChanged(curType);
   }
}