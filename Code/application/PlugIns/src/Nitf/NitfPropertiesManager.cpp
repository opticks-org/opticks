/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataDescriptor.h"
#include "DataElement.h"
#include "DynamicObject.h"
#include "NitfConstants.h"
#include "NitfPropertiesManager.h"
#include "NitfProperties.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Slot.h"
#include "StringUtilities.h"

REGISTER_PLUGIN(OpticksNitf, NitfPropertiesManager, Nitf::NitfPropertiesManager);

namespace Nitf
{

NitfPropertiesManager::NitfPropertiesManager()
{
   setName("NITF Properties Manager");
   setType("Manager");
   setCreator("Ball Aerospace & Technologies Corp.");
   setVersion(APP_VERSION_NUMBER);
   setCopyright(APP_COPYRIGHT);
   setDescription("Adds properties sheets to applicable data elements.");
   setDescriptorId("{0fc0b80e-18fe-4d26-8db8-ef23d8bac00c}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setWizardSupported(false);
   destroyAfterExecute(false);
   executeOnStartup(true);
   mpDesktop.addSignal(SIGNAL_NAME(DesktopServices, AboutToShowPropertiesDialog),
                Slot(this, &NitfPropertiesManager::addProperties));
}

NitfPropertiesManager::~NitfPropertiesManager()
{
}

bool NitfPropertiesManager::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool NitfPropertiesManager::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool NitfPropertiesManager::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   mpDesktop.reset(Service<DesktopServices>().get());
   std::vector<PlugInDescriptor*> descs =
      Service<PlugInManagerServices>()->getPlugInDescriptors(PlugInManagerServices::PropertiesType());
   for (std::vector<PlugInDescriptor*>::iterator desc = descs.begin(); desc != descs.end(); ++desc)
   {
      if ((*desc)->getSubtype() == Nitf::Properties::SubType())
      {
         PlugInResource pPropsPlugin((*desc)->getName());
         Nitf::Properties* pProps = pPropsPlugin.get() == NULL ? NULL :
            dynamic_cast<Nitf::Properties*>(pPropsPlugin.get());
         if (pProps != NULL)
         {
            mPropertiesPlugins.insert(std::make_pair(pProps->getTypeName(), (*desc)->getName()));
         }
      }
   }
   return true;
}

PlugInResource NitfPropertiesManager::getPropertyPlugIn(const std::string& name, const DynamicObject& metadata)
{
   std::map<std::string, std::string>::const_iterator propPluginName = mPropertiesPlugins.find(name);
   if (propPluginName != mPropertiesPlugins.end())
   {
      PlugInResource pPropsPlugin(propPluginName->second);
      Nitf::Properties* pProps = pPropsPlugin.get() == NULL ? NULL :
         dynamic_cast<Nitf::Properties*>(pPropsPlugin.get());
      if (pProps != NULL)
      {
         if (pProps->canDisplayMetadata(metadata))
         {
            return pPropsPlugin;
         }
      }
   }
   return PlugInResource(NULL);
}

void NitfPropertiesManager::addProperties(Subject& subject, const std::string& signal, const boost::any& val)
{
   std::pair<SessionItem*, std::vector<std::string>*> data =
         boost::any_cast<std::pair<SessionItem*, std::vector<std::string>*> >(val);
   VERIFYNRV(data.second);
   DataElement* pElement = dynamic_cast<DataElement*>(data.first);
   DataDescriptor* pDesc = (pElement == NULL) ? NULL : pElement->getDataDescriptor();
   const DynamicObject* pMeta = (pDesc == NULL) ? NULL : pDesc->getMetadata();
   const DynamicObject* pTres = (pMeta == NULL) ? NULL : dv_cast<DynamicObject>(
      &pMeta->getAttributeByPath(Nitf::NITF_METADATA + "/" + Nitf::TRE_METADATA));
   if (pTres != NULL)
   {
      std::vector<std::string> treNames;
      pTres->getAttributeNames(treNames);
      for (std::vector<std::string>::const_iterator treName = treNames.begin(); treName != treNames.end(); ++treName)
      {
         const DynamicObject& treMetadata = dv_cast<DynamicObject>((*pTres).getAttribute(*treName));
         PlugInResource pTrePropsPlugin(getPropertyPlugIn(*treName, treMetadata));
         if (pTrePropsPlugin.get() != NULL)
         {
            data.second->push_back(pTrePropsPlugin->getName());
         }
      }
   }
}

}
