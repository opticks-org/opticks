/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "StringUtilities.h"
#include "WavelengthUnitsComboBox.h"

#include <string>
#include <vector>

WavelengthUnitsComboBox::WavelengthUnitsComboBox(QWidget* pParent) :
   QComboBox(pParent)
{
   // Initialization
   setEditable(false);

   std::vector<std::string> items = StringUtilities::getAllEnumValuesAsDisplayString<WavelengthUnitsType>();
   for (std::vector<std::string>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
   {
      if (iter->empty() == false)
      {
         addItem(QString::fromStdString(*iter));
      }
   }

   // Connections
   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

WavelengthUnitsComboBox::~WavelengthUnitsComboBox()
{}

void WavelengthUnitsComboBox::setUnits(WavelengthUnitsType units)
{
   QString strUnits = QString::fromStdString(StringUtilities::toDisplayString(units));

   int index = findText(strUnits);
   setCurrentIndex(index);
}

WavelengthUnitsType WavelengthUnitsComboBox::getUnits() const
{
   WavelengthUnitsType units;
   if (currentIndex() != -1)
   {
      std::string unitsText = currentText().toStdString();
      units = StringUtilities::fromDisplayString<WavelengthUnitsType>(unitsText);
   }

   return units;
}

void WavelengthUnitsComboBox::translateActivated(int index)
{
   if (index != -1)
   {
      std::string unitsText = currentText().toStdString();
      WavelengthUnitsType units = StringUtilities::fromDisplayString<WavelengthUnitsType>(unitsText);
      emit unitsActivated(units);
   }
}
