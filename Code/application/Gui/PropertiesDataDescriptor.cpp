/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DataDescriptorWidget.h"
#include "DataElement.h"
#include "PropertiesDataDescriptor.h"

PropertiesDataDescriptor::PropertiesDataDescriptor() :
   mpDescriptor(NULL)
{
   setName("Data Descriptor Properties");
   setPropertiesName("Data");
   setDescription("General setting properties of a data descriptor");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT_MSG);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{75092A96-A74A-4E4B-83A8-1E27282BD6F9}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

PropertiesDataDescriptor::~PropertiesDataDescriptor()
{}

bool PropertiesDataDescriptor::initialize(SessionItem* pSessionItem)
{
   DataDescriptorWidget* pDescriptorPage = dynamic_cast<DataDescriptorWidget*>(getWidget());
   if (pDescriptorPage == NULL)
   {
      return false;
   }

   const DataElement* pElement = dynamic_cast<DataElement*>(pSessionItem);
   if (pElement != NULL)
   {
      const DataDescriptor* pDescriptor = pElement->getDataDescriptor();
      if (pDescriptor != NULL)
      {
         mpDescriptor = DataDescriptorResource<DataDescriptor>(pDescriptor->copy());
         if (mpDescriptor.get() != NULL)
         {
            pDescriptorPage->setDataDescriptor(mpDescriptor.get(), false);    // Only allow the user to edit the units
            return PropertiesShell::initialize(pSessionItem);
         }
      }
   }

   return false;
}

bool PropertiesDataDescriptor::applyChanges()
{
   if (mpDescriptor.get() == NULL)
   {
      return false;
   }

   DataElement* pElement = dynamic_cast<DataElement*>(getSessionItem());
   if (pElement != NULL)
   {
      DataDescriptor* pDescriptor = pElement->getDataDescriptor();
      if (pDescriptor != NULL)
      {
         return pDescriptor->clone(mpDescriptor.get());
      }
   }

   return false;
}

QWidget* PropertiesDataDescriptor::createWidget()
{
   QWidget* pWidget = new DataDescriptorWidget();
   return pWidget;
}
