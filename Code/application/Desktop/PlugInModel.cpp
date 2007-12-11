/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleDescriptor.h"
#include "PlugIn.h"
#include "PlugInDescriptorImp.h"
#include "PlugInManagerServicesImp.h"
#include "PlugInModel.h"
#include "Slot.h"

using namespace std;

PlugInModel::PlugInModel(QObject* pParent) :
   SessionItemModel(pParent),
   mpManager(PlugInManagerServicesImp::instance()),
   mbManagerDeleted(false)
{
   // Initialization
   initialize();

   // Connections
   mpManager->attach(SIGNAL_NAME(PlugInManagerServices, PlugInCreated), Slot(this, &PlugInModel::addPlugIn));
   mpManager->attach(SIGNAL_NAME(PlugInManagerServices, PlugInDestroyed), Slot(this, &PlugInModel::removePlugIn));
   mpManager->attach(SIGNAL_NAME(PlugInManagerServices, ModuleCreated), Slot(this, &PlugInModel::addModule));
   mpManager->attach(SIGNAL_NAME(PlugInManagerServices, ModuleDestroyed), Slot(this, &PlugInModel::removeModule));
   mpManager->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &PlugInModel::plugInManagerDeleted));
}

PlugInModel::~PlugInModel()
{
   if (mbManagerDeleted == false)
   {
      mpManager->detach(SIGNAL_NAME(PlugInManagerServices, PlugInCreated), Slot(this, &PlugInModel::addPlugIn));
      mpManager->detach(SIGNAL_NAME(PlugInManagerServices, PlugInDestroyed), Slot(this, &PlugInModel::removePlugIn));
      mpManager->detach(SIGNAL_NAME(PlugInManagerServices, ModuleCreated), Slot(this, &PlugInModel::addModule));
      mpManager->detach(SIGNAL_NAME(PlugInManagerServices, ModuleDestroyed), Slot(this, &PlugInModel::removeModule));
      mpManager->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &PlugInModel::plugInManagerDeleted));
   }
}

void PlugInModel::addPlugIn(Subject& subject, const string& signal, const boost::any& value)
{
   PlugIn* pPlugIn = boost::any_cast<PlugIn*>(value);

   SessionItem* pItem = dynamic_cast<SessionItem*>(pPlugIn);
   if (pItem != NULL)
   {
      string plugInName = pItem->getName();

      PlugInDescriptor* pDescriptor = mpManager->getPlugInDescriptor(plugInName);
      if (pDescriptor != NULL)
      {
         SessionItemWrapper* pDescriptorWrapper = getWrapper(pDescriptor);
         if (pDescriptorWrapper != NULL)
         {
            pDescriptorWrapper->addChild(pItem);
         }
      }
   }
}

void PlugInModel::removePlugIn(Subject& subject, const string& signal, const boost::any& value)
{
   PlugIn* pPlugIn = boost::any_cast<PlugIn*>(value);

   SessionItem* pItem = dynamic_cast<SessionItem*>(pPlugIn);
   if (pItem != NULL)
   {
      string plugInName = pItem->getName();

      PlugInDescriptor* pDescriptor = mpManager->getPlugInDescriptor(plugInName);
      if (pDescriptor != NULL)
      {
         SessionItemWrapper* pDescriptorWrapper = getWrapper(pDescriptor);
         if (pDescriptorWrapper != NULL)
         {
            pDescriptorWrapper->removeChild(pItem);
         }
      }
   }
}

void PlugInModel::addModule(Subject& subject, const string& signal, const boost::any& value)
{
   ModuleDescriptor *pModule = boost::any_cast<ModuleDescriptor*>(value);
   SessionItemWrapper* pRootWrapper = getRootWrapper();
   SessionItemWrapper* pModuleWrapper = pRootWrapper->addChild(pModule);
   if(pModuleWrapper != NULL)
   {
      vector<PlugInDescriptorImp*> descriptors = pModule->getPlugInSet();
      for (vector<PlugInDescriptorImp*>::size_type j = 0; j < descriptors.size(); ++j)
      {
         PlugInDescriptorImp* pDescriptor = descriptors[j];
         SessionItem* pDescriptorItem = dynamic_cast<SessionItem*>(pDescriptor);
         if ((pDescriptor != NULL) && (pDescriptorItem != NULL))
         {
            SessionItemWrapper* pDescriptorWrapper = pModuleWrapper->addChild(pDescriptorItem);
            if (pDescriptorWrapper != NULL)
            {
               vector<PlugIn*> plugIns = pDescriptor->getPlugIns();
               for (vector<PlugIn*>::size_type k = 0; k < plugIns.size(); ++k)
               {
                  SessionItem* pPlugIn = dynamic_cast<SessionItem*>(plugIns[k]);
                  if (pPlugIn != NULL)
                  {
                     pDescriptorWrapper->addChild(pPlugIn);
                  }
               }
            }
         }
      }
   }
}

void PlugInModel::removeModule(Subject& subject, const string& signal, const boost::any& value)
{
   ModuleDescriptor *pModule = boost::any_cast<ModuleDescriptor*>(value);
   SessionItemWrapper* pRootWrapper = getRootWrapper();
   pRootWrapper->removeChild(pModule);
}
void PlugInModel::plugInManagerDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   mbManagerDeleted = true;
}

void PlugInModel::initialize()
{
   // Clear any existing data
   clear();

   // Add the modules and plug-ins
   SessionItemWrapper* pRootWrapper = getRootWrapper();

   const vector<ModuleDescriptor*>& modules = mpManager->getModuleDescriptors();
   for (vector<ModuleDescriptor*>::size_type i = 0; i < modules.size(); ++i)
   {
      ModuleDescriptor* pModule = modules[i];
      if (pModule != NULL)
      {
         SessionItemWrapper* pModuleWrapper = pRootWrapper->addChild(pModule);
         if (pModuleWrapper != NULL)
         {
            vector<PlugInDescriptorImp*> descriptors = pModule->getPlugInSet();
            for (vector<PlugInDescriptorImp*>::size_type j = 0; j < descriptors.size(); ++j)
            {
               PlugInDescriptorImp* pDescriptor = descriptors[j];
               SessionItem* pDescriptorItem = dynamic_cast<SessionItem*>(pDescriptor);
               if ((pDescriptor != NULL) && (pDescriptorItem != NULL))
               {
                  SessionItemWrapper* pDescriptorWrapper = pModuleWrapper->addChild(pDescriptorItem);
                  if (pDescriptorWrapper != NULL)
                  {
                     vector<PlugIn*> plugIns = pDescriptor->getPlugIns();
                     for (vector<PlugIn*>::size_type k = 0; k < plugIns.size(); ++k)
                     {
                        SessionItem* pPlugIn = dynamic_cast<SessionItem*>(plugIns[k]);
                        if (pPlugIn != NULL)
                        {
                           pDescriptorWrapper->addChild(pPlugIn);
                        }
                     }
                  }
               }
            }
         }
      }
   }

   // Reset the model to update the view
   reset();
}
