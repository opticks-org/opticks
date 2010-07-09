/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataElement.h"
#include "MetadataWidget.h"
#include "PropertiesMetadata.h"

PropertiesMetadata::PropertiesMetadata()
{
   setName("Metadata Properties");
   setPropertiesName("Metadata");
   setDescription("General setting properties of a data element");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT_MSG);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{8BD01A33-6CB0-4D3F-94E7-6C1FC77B7679}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

PropertiesMetadata::~PropertiesMetadata()
{}

bool PropertiesMetadata::initialize(SessionItem* pSessionItem)
{
   MetadataWidget* pMetadataPage = dynamic_cast<MetadataWidget*>(getWidget());
   if (pMetadataPage == NULL)
   {
      return false;
   }

   mMetadata.clear();

   DataElement* pElement = dynamic_cast<DataElement*>(pSessionItem);
   if (pElement != NULL)
   {
      const DynamicObject* pMetadata = pElement->getMetadata();
      if (pMetadata != NULL)
      {
         mMetadata.merge(pMetadata);
         pMetadataPage->setMetadata(&mMetadata);
         return PropertiesShell::initialize(pSessionItem);
      }
   }

   return false;
}

bool PropertiesMetadata::applyChanges()
{
   DataElement* pElement = dynamic_cast<DataElement*>(getSessionItem());
   if (pElement != NULL)
   {
      DynamicObject* pMetadata = pElement->getMetadata();
      if (pMetadata != NULL)
      {
         pMetadata->clear();
         pMetadata->adoptiveMerge(&mMetadata);
         return true;
      }
   }

   return false;
}

QWidget* PropertiesMetadata::createWidget()
{
   QWidget* pWidget = new MetadataWidget();
   return pWidget;
}
