/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataElement.h"
#include "PropertiesWavelengths.h"
#include "RasterDataDescriptor.h"
#include "WavelengthsWidget.h"

PropertiesWavelengths::PropertiesWavelengths()
{
   setName("Wavelength Properties");
   setPropertiesName("Wavelengths");
   setDescription("Wavelengths associated with bands of a raster element");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT_MSG);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{13C14B7E-50A4-49CD-B438-66BF88C9A9CB}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

PropertiesWavelengths::~PropertiesWavelengths()
{}

bool PropertiesWavelengths::initialize(SessionItem* pSessionItem)
{
   WavelengthsWidget* pWavelengthsPage = dynamic_cast<WavelengthsWidget*>(getWidget());
   if (pWavelengthsPage == NULL)
   {
      return false;
   }

   DataElement* pElement = dynamic_cast<DataElement*>(pSessionItem);
   if (pElement != NULL)
   {
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         const DynamicObject* pMetadata = pElement->getMetadata();
         if (pMetadata != NULL)
         {
            mWavelengths.initializeFromDynamicObject(pMetadata, true);
            pWavelengthsPage->setWavelengths(pDescriptor->getBands(), &mWavelengths);
            return PropertiesShell::initialize(pSessionItem);
         }
      }
   }

   return false;
}

bool PropertiesWavelengths::applyChanges()
{
   DataElement* pElement = dynamic_cast<DataElement*>(getSessionItem());
   if (pElement != NULL)
   {
      if (dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor()) != NULL)
      {
         DynamicObject* pMetadata = pElement->getMetadata();
         if (pMetadata != NULL)
         {
            return mWavelengths.applyToDynamicObject(pMetadata);
         }
      }
   }

   return false;
}

QWidget* PropertiesWavelengths::createWidget()
{
   QWidget* pWidget = new WavelengthsWidget();
   return pWidget;
}
