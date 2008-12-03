/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "DataElement.h"
#include "MetadataWidget.h"
#include "PropertiesDataElement.h"

PropertiesDataElement::PropertiesDataElement()
{
   setName("Data Element Properties");
   setPropertiesName("Metadata");
   setDescription("General setting properties of a data element");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT_MSG);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{8BD01A33-6CB0-4D3F-94E7-6C1FC77B7679}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

PropertiesDataElement::~PropertiesDataElement()
{
}

bool PropertiesDataElement::initialize(SessionItem* pSessionItem)
{
   MetadataWidget* pMetadataPage = dynamic_cast<MetadataWidget*>(getWidget());
   if (pMetadataPage == NULL)
   {
      return false;
   }

   DataElement* pElement = dynamic_cast<DataElement*>(pSessionItem);
   if (pElement != NULL)
   {
      DynamicObject* pMetadata = pElement->getMetadata();
      if (pMetadata != NULL)
      {
         pMetadataPage->setMetadata(pMetadata);
         return true;
      }
   }

   return false;
}

bool PropertiesDataElement::applyChanges()
{
   MetadataWidget* pMetadataPage = dynamic_cast<MetadataWidget*>(getWidget());
   VERIFY(pMetadataPage != NULL);

   return pMetadataPage->applyChanges();
}

QWidget* PropertiesDataElement::createWidget()
{
   QWidget* pWidget = new MetadataWidget();
   return pWidget;
}
