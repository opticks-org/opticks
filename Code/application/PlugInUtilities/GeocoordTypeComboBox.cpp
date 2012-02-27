/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "GeocoordTypeComboBox.h"
#include "StringUtilities.h"

#include <string>

GeocoordTypeComboBox::GeocoordTypeComboBox(QWidget* pParent) :
   QComboBox(pParent)
{
   setEditable(false);

   addItem(QString::fromStdString(StringUtilities::toDisplayString(GEOCOORD_LATLON)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(GEOCOORD_UTM)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(GEOCOORD_MGRS)));

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

GeocoordTypeComboBox::~GeocoordTypeComboBox()
{}

void GeocoordTypeComboBox::setGeocoordType(GeocoordType geocoordType)
{
   int index = -1;
   if (geocoordType.isValid() == true)
   {
      std::string typeText = StringUtilities::toDisplayString(geocoordType);
      if (typeText.empty() == false)
      {
         index = findText(QString::fromStdString(typeText));
      }
   }

   setCurrentIndex(index);
}

GeocoordType GeocoordTypeComboBox::getGeocoordType() const
{
   GeocoordType geocoordType;
   if (currentIndex() != -1)
   {
      std::string typeText = currentText().toStdString();
      if (typeText.empty() == false)
      {
         geocoordType = StringUtilities::fromDisplayString<GeocoordType>(typeText);
      }
   }

   return geocoordType;
}

void GeocoordTypeComboBox::translateActivated(int index)
{
   GeocoordType geocoordType;
   if (index != -1)
   {
      std::string typeText = itemText(index).toStdString();
      if (typeText.empty() == false)
      {
         geocoordType = StringUtilities::fromDisplayString<GeocoordType>(typeText);
      }
   }

   emit geocoordTypeChanged(geocoordType);
}
