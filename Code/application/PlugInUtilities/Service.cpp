/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Service.h"
#include "ApplicationServices.h"
#include "DesktopServices.h"
#include "ModuleManager.h"
#include "SessionExplorer.h"
#include "UtilityServices.h"
#include <stdexcept>

class AnimationServices;
class ModelServices;
class PlugInManagerServices;
class SessionManager;

template<>
ApplicationServices* Service<ApplicationServices>::get() const
{
   ApplicationServices* pT = NULL;
   ModuleManager::instance()->getService()->queryInterface("ApplicationServices2", reinterpret_cast<void**>(&pT));
   return pT;
}

template<>
UtilityServices* Service<UtilityServices>::get() const
{
   UtilityServices* pT = NULL;
   ModuleManager::instance()->getService()->queryInterface("UtilityServices2", reinterpret_cast<void**>(&pT));
   return pT;
}

template<>
ModelServices* Service<ModelServices>::get() const
{
   ModelServices* pT = NULL;
   ModuleManager::instance()->getService()->queryInterface("ModelServices2", reinterpret_cast<void**>(&pT));
   return pT;
}

template<>
AnimationServices* Service<AnimationServices>::get() const
{
   AnimationServices* pT = NULL;
   ModuleManager::instance()->getService()->queryInterface("AnimationServices2", reinterpret_cast<void**>(&pT));
   return pT;
}

template<>
DesktopServices* Service<DesktopServices>::get() const
{
   DesktopServices* pT = NULL;
   ModuleManager::instance()->getService()->queryInterface("DesktopServices2", reinterpret_cast<void**>(&pT));
   return pT;
}

template<>
PlugInManagerServices* Service<PlugInManagerServices>::get() const
{
   PlugInManagerServices* pT = NULL;
   ModuleManager::instance()->getService()->queryInterface("PlugInManagerServices2", reinterpret_cast<void**>(&pT));
   return pT;
}

template <>
SessionManager* Service<SessionManager>::get() const
{
   Service<ApplicationServices> pApp;
   SessionManager* pManager = pApp->getSessionManager();
   return pManager;
}

template <>
ConfigurationSettings* Service<ConfigurationSettings>::get() const
{
   Service<ApplicationServices> pApp;
   ConfigurationSettings* pConfig = pApp->getConfigurationSettings();
   return pConfig;
}

template <>
ObjectFactory* Service<ObjectFactory>::get() const
{
   Service<ApplicationServices> pApp;
   ObjectFactory* pObjFact = pApp->getObjectFactory();
   return pObjFact;
}

template <>
DataVariantFactory* Service<DataVariantFactory>::get() const
{
   Service<ApplicationServices> pApp;
   DataVariantFactory* pObjFact = pApp->getDataVariantFactory();
   return pObjFact;
}

template <>
MessageLogMgr* Service<MessageLogMgr>::get() const
{
   Service<UtilityServices> pUtil;
   MessageLogMgr* pMsgMgr = pUtil->getMessageLog();
   return pMsgMgr;
}

template<>
SessionExplorer *Service<SessionExplorer>::get() const
{
   Service<DesktopServices> pDesktop;
   SessionExplorer* pExplorer = dynamic_cast<SessionExplorer*>(pDesktop->getWindow("Session Explorer", DOCK_WINDOW));
   if (pExplorer == NULL)
   {
      throw std::logic_error("Attempting to use SessionExplorer before it has been created "
         "or after it has been deleted");
   }

   return pExplorer;
}
