/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QIcon>

#include "ModuleDescriptor.h"
#include "PlugIn.h"
#include "PlugInDescriptorImp.h"
#include "PlugInManagerServicesImp.h"
#include "PlugInModel.h"
#include "PlugInRegistration.h"
#include "Slot.h"

using namespace std;

PlugInModel::PlugInModel(QObject* pParent) :
   SessionItemModel(pParent),
   mpManager(Service<PlugInManagerServices>().get())
{
   // Initialization
   PlugInManagerServicesImp* pManagerImp = dynamic_cast<PlugInManagerServicesImp*>(mpManager.get());
   if (pManagerImp != NULL)
   {
      const vector<ModuleDescriptor*>& modules = pManagerImp->getModuleDescriptors();
      for (vector<ModuleDescriptor*>::size_type i = 0; i < modules.size(); ++i)
      {
         ModuleDescriptor* pModule = modules[i];
         if (pModule != NULL)
         {
            addModuleItem(pModule);
         }
      }
   }

   // Connections
   mpManager.addSignal(SIGNAL_NAME(PlugInManagerServices, ModuleCreated), Slot(this, &PlugInModel::addModule));
   mpManager.addSignal(SIGNAL_NAME(PlugInManagerServices, ModuleDestroyed), Slot(this, &PlugInModel::removeModule));
   mpManager.addSignal(SIGNAL_NAME(PlugInManagerServices, PlugInCreated), Slot(this, &PlugInModel::addPlugIn));
   mpManager.addSignal(SIGNAL_NAME(PlugInManagerServices, PlugInDestroyed), Slot(this, &PlugInModel::removePlugIn));
}

PlugInModel::~PlugInModel()
{
   // Remove the module items
   PlugInManagerServicesImp* pManagerImp = dynamic_cast<PlugInManagerServicesImp*>(mpManager.get());
   if (pManagerImp != NULL)
   {
      const vector<ModuleDescriptor*>& modules = pManagerImp->getModuleDescriptors();
      for (vector<ModuleDescriptor*>::size_type i = 0; i < modules.size(); ++i)
      {
         ModuleDescriptor* pModule = modules[i];
         if (pModule != NULL)
         {
            removeModuleItem(pModule);
         }
      }
   }
}

Qt::ItemFlags PlugInModel::flags(const QModelIndex& index) const
{
   Qt::ItemFlags itemFlags = SessionItemModel::flags(index);
   if (index.isValid() == true)
   {
      itemFlags |= Qt::ItemIsDragEnabled;
   }

   return itemFlags;
}

QVariant PlugInModel::data(const QModelIndex& index, int role) const
{
   QVariant value = SessionItemModel::data(index, role);
   if (role == Qt::DecorationRole)
   {
      SessionItemWrapper* pWrapper = reinterpret_cast<SessionItemWrapper*>(index.internalPointer());
      if (pWrapper != NULL)
      {
         if (dynamic_cast<PlugInDescriptor*>(pWrapper->getSessionItem()) != NULL)
         {
            QIcon plugInIcon = value.value<QIcon>();
            if (plugInIcon.isNull() == true)
            {
               value = QIcon(":/icons/PlugIn");
            }
         }
      }
   }

   return value;
}

void PlugInModel::addModule(Subject& subject, const string& signal, const boost::any& value)
{
   ModuleDescriptor* pModule = boost::any_cast<ModuleDescriptor*>(value);
   if (pModule != NULL)
   {
      addModuleItem(pModule);
   }
}

void PlugInModel::removeModule(Subject& subject, const string& signal, const boost::any& value)
{
   ModuleDescriptor* pModule = boost::any_cast<ModuleDescriptor*>(value);
   if (pModule != NULL)
   {
      removeModuleItem(pModule);
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

void PlugInModel::addModuleItem(ModuleDescriptor* pModule)
{
   if (pModule == NULL)
   {
      return;
   }

   SessionItemWrapper* pRootWrapper = getRootWrapper();
   if (pRootWrapper == NULL)
   {
      return;
   }

   // Add the module item
   SessionItemWrapper* pModuleWrapper = pRootWrapper->addChild(pModule);
   if (pModuleWrapper != NULL)
   {
      if (pModule->getModuleVersion() == MOD_ONE)
      {
         pModuleWrapper->setDisplayColor(Qt::blue);
      }
      // Add the plug-in descriptor items
      vector<PlugInDescriptorImp*> descriptors = pModule->getPlugInSet();
      for (vector<PlugInDescriptorImp*>::size_type i = 0; i < descriptors.size(); ++i)
      {
         PlugInDescriptorImp* pDescriptor = descriptors[i];
         SessionItem* pDescriptorItem = dynamic_cast<SessionItem*>(pDescriptor);

         if ((pDescriptor != NULL) && (pDescriptorItem != NULL))
         {
            SessionItemWrapper* pDescriptorWrapper = pModuleWrapper->addChild(pDescriptorItem);
            if (pDescriptorWrapper != NULL)
            {
               if (pDescriptor->isProduction() == false)
               {
                  pDescriptorWrapper->setDisplayColor(Qt::red);
               }
               // Add the plug-in instance items
               vector<PlugIn*> plugIns = pDescriptor->getPlugIns();
               for (vector<PlugIn*>::size_type j = 0; j < plugIns.size(); ++j)
               {
                  SessionItem* pPlugIn = dynamic_cast<SessionItem*>(plugIns[j]);
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

void PlugInModel::removeModuleItem(ModuleDescriptor* pModule)
{
   if (pModule == NULL)
   {
      return;
   }

   SessionItemWrapper* pRootWrapper = getRootWrapper();
   if (pRootWrapper != NULL)
   {
      pRootWrapper->removeChild(pModule);
   }
}
