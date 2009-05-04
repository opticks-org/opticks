/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINREGISTRATION_H
#define PLUGINREGISTRATION_H

#include "AppConfig.h"
#include "ConfigurationSettings.h"
#include "External.h"
#include <algorithm>
#include <stdlib.h>
#include <string>
#include <vector>

class PlugIn;

#if defined(DEPRECATED_MODULE_TYPE)

#define REGISTER_MODULE_BASIC(name__, moduleNamespace__) \
   class YouCannotUseThisMacroWith_DEPRECATED_MODULE_TYPE_defined; \
   const int iYouCannotUseThisMacroWith_DEPRECATED_MODULE_TYPE_defined = \
      sizeof(YouCannotUseThisMacroWith_DEPRECATED_MODULE_TYPE_defined);

#define REGISTER_MODULE(name__) REGISTER_MODULE_BASIC(1, 1)
#define REGISTER_DYNAMIC_MODULE(name__, dynamicFactoryClass__) REGISTER_MODULE_BASIC(1, 1)
#define REGISTER_PLUGIN(moduleNamespace__, pluginname__, className__) REGISTER_MODULE_BASIC(1, 1)
#define REGISTER_PLUGIN_BASIC(moduleNamespace__, name__) REGISTER_MODULE_BASIC(1, 1)
#define GENERATE_FACTORY(moduleNamespace__) REGISTER_MODULE_BASIC(1, 1)
#define GENERATE_DYNAMIC_FACTORY(moduleNamespace__, dynamicFactoryClass__) REGISTER_MODULE_BASIC(1, 1)

#else

/**
 * \cond INTERNAL
 * This struct contains information about a version 2 plug-in module.
 */
struct OpticksModuleDescriptor
{
   unsigned int version;           /**< The version of this descriptor structure. This should be 2. */
   const char* pModuleId;          /**< A globally unique ID that identifies this module. this
                                        should be guarenteed unique to an instance of the application
                                        and it should not change between runs of the application for
                                        a specific version of the application. */
   const char* pInstantiateSymbol; /**< The name of the function that creates PlugIn objects. */
   bool debug;                     /**< True if this module contains debug plug-ins and
                                        false if thaey are release plug-ins. Certains systems may
                                        refuse to load plug-ins with mismatched debug flags. */
};
/// \endcond

/**
 * \cond INTERNAL
 * This is the function signature for opticks_get_module_descriptor()
 */
typedef struct OpticksModuleDescriptor*(*OpticksGetModuleDescriptorType)(External*);
/// \endcond

/**
 * \cond INTERNAL
 * This is the function sugnature for the function which creates plug-in instances.
 */
typedef bool(*OpticksInstantiateType)(External*, unsigned int, PlugIn**);
/// \endcond

#ifdef DEBUG
#define DEBUG_BOOL true
#else
#define DEBUG_BOOL false
#endif
#define REGISTER_MODULE_BASIC(name__, moduleNamespace__) \
   extern "C" { \
      struct OpticksModuleDescriptor name__##Descriptor = { \
         2, \
         #name__, \
         #name__ "InstantiatePlugIn", \
         DEBUG_BOOL \
      }; \
      LINKAGE struct OpticksModuleDescriptor* opticks_get_module_descriptor(External*) \
      { \
         return &name__##Descriptor; \
      } \
      LINKAGE bool name__##InstantiatePlugIn(External* pExternal, unsigned int plugInNumber, PlugIn** pPlugIn) \
      { \
         ModuleManager::instance()->setService(pExternal); \
         *pPlugIn = moduleNamespace__::getPlugIn(plugInNumber); \
         return *pPlugIn != NULL; \
      } \
   }; \
   const char* ModuleManager::mspName = NULL; \
   const char* ModuleManager::mspVersion = NULL; \
   const char* ModuleManager::mspDescription = NULL; \
   const char* ModuleManager::mspValidationKey = NULL; \
   const char* ModuleManager::mspUniqueId = NULL; \
   unsigned int ModuleManager::getTotalPlugIns() { return 0; } \
   PlugIn* ModuleManager::getPlugIn(unsigned int) { return NULL; } \

#define REGISTER_MODULE(name__) \
   GENERATE_FACTORY(name__) \
   REGISTER_MODULE_BASIC(name__, name__)

#define REGISTER_DYNAMIC_MODULE(name__, dynamicFactoryClass__) \
   GENERATE_DYNAMIC_FACTORY(name__, dynamicFactoryClass__) \
   REGISTER_MODULE_BASIC(name__, name__) 

class PlugIn;
class PlugInFactory;

/**
 * \cond INTERNAL
 * Automatically registers plug-ins in a module using convenient macros.
 *
 * Using this class requires ModuleManager.cpp to be configured for PlugInFactory use.
 * The sample ModuleManager.cpp contained in the Opticks SDK shows how to configure
 * ModuleManager.cpp for use with PlugInFactory. Direct use of this class is not needed.
 * @see REGISTER_PLUGIN(), REGISTER_PLUGIN_ADVANCED()
 */
class PlugInFactory
{
public:
   /**
    * Construct a new factory for a given name adding it to the factory list for this module.
    *
    * @param name
    *        The name of the object created by this factory.
    */
   PlugInFactory() {}

   /**
    * Create a new plugin.
    *
    * @return The PlugIn object
    */
   virtual PlugIn* createPlugIn(const std::string& name) = 0;

   virtual std::vector<std::string> getPlugInNames() = 0;
};
/// \endcond

/**
 * \cond INTERNAL
 * Automatically registers plug-ins in a module using convenient macros.
 *
 * Using this class requires ModuleManager.cpp to be configured for PlugInFactory use.
 * The sample ModuleManager.cpp contained in the Opticks SDK shows how to configure
 * ModuleManager.cpp for use with PlugInFactory. Direct use of this class is not needed.
 * @see REGISTER_PLUGIN(), REGISTER_PLUGIN_ADVANCED()
 */
class BasicPlugInFactory : public PlugInFactory
{
public:
   /**
    * Construct a new factory for a given name adding it to the factory list for this module.
    *
    * @param name
    *        The name of the object created by this factory.
    */
   BasicPlugInFactory(const std::string& name) : PlugInFactory()
   {
      addPlugInName(name);
   }
   virtual std::vector<std::string> getPlugInNames()
   {
      return mPlugInNames;
   }

private:
   BasicPlugInFactory() {}
   void addPlugInName(std::string name)
   {
      mPlugInNames.push_back(name);
   }

   std::vector<std::string> mPlugInNames;
};
/// \endcond

/**
 * KIP - Document this
 */
class DynamicPlugInFactory : public PlugInFactory
{
public:
   DynamicPlugInFactory() {}

   void setModuleId(const std::string& moduleId)
   {
      mModuleId = moduleId;
   }

   virtual void init(bool locate, std::vector<std::string>& dynamicPlugIns) = 0;

   std::vector<std::string> getPlugInNames()
   {
      Service<ConfigurationSettings> pSettings;
      std::string plugInsKey = "_" + mModuleId + "DynamicPlugInNames_";
      std::vector<std::string> names;
      if (pSettings->isSessionSetting(plugInsKey))
      {
         const std::vector<std::string>* pNames = NULL;
         pNames = dv_cast<std::vector<std::string> >(&pSettings->getSetting(plugInsKey));
         if (pNames != NULL)
         {
            names = *pNames;
         }
         std::vector<std::string> tempCopy = names;
         init(false, tempCopy);
      }
      else
      {
         init(true, names);
         pSettings->setSessionSetting(plugInsKey, names);
      }
      return names;
   }
protected:
   std::string mModuleId;
};

/**
 * \cond INTERNAL
 * Used to sort plug-in factories.
 */
struct FactoryPtrComparator
{
   bool operator()(std::pair<std::string, PlugInFactory*> pLhs, std::pair<std::string, PlugInFactory*> pRhs)
   {
      if (pLhs.second == NULL || pRhs.second == NULL)
      {
         return false;
      }
      size_t leftCount = std::count(pLhs.first.begin(), pLhs.first.end(), '_');
      size_t rightCount = std::count(pRhs.first.begin(), pRhs.first.end(), '_');
      if (leftCount != rightCount)
      {
         return leftCount > rightCount;
      }
      return pLhs.first < pRhs.first;
   }
};
/// \endcond


#define REGISTER_PLUGIN(moduleNamespace__, pluginname__, className__) \
   namespace moduleNamespace__ \
   { \
      void addFactory(PlugInFactory*); \
      class pluginname__##PlugInFactory : public BasicPlugInFactory \
      { \
      public: \
         pluginname__##PlugInFactory(const std::string& name) : BasicPlugInFactory(name) \
         { \
            moduleNamespace__::addFactory(this); \
         } \
         PlugIn* createPlugIn(const std::string& name) \
         { \
            return new className__; \
         } \
      }; \
      static pluginname__##PlugInFactory pluginname__##pluginFactory(#pluginname__); \
   }

#define REGISTER_PLUGIN_BASIC(moduleNamespace__, name__) REGISTER_PLUGIN(moduleNamespace__, name__, name__)

#define GENERATE_FACTORY(moduleNamespace__) \
   namespace moduleNamespace__ { \
   std::vector<std::pair<std::string, PlugInFactory*> >& factories() \
   { \
      static std::vector<std::pair<std::string, PlugInFactory*> > sFactories; \
      return sFactories; \
   } \
   void addFactory(PlugInFactory* pFactory) \
   { \
      if (pFactory != NULL) \
      { \
         std::vector<std::string> names = pFactory->getPlugInNames(); \
         for (std::vector<std::string>::iterator nameIter = names.begin(); nameIter != names.end(); ++nameIter) \
         { \
            factories().push_back(make_pair(*nameIter, pFactory)); \
         } \
      } \
   } \
   PlugIn* getPlugIn(unsigned int plugInNumber) \
   { \
      static bool factoriesSorted = false; \
      if (!factoriesSorted) \
      { \
         factoriesSorted = true; \
         FactoryPtrComparator comp; \
         std::sort(factories().begin(), factories().end(), comp); \
      } \
      if (plugInNumber >= factories().size()) \
      { \
         return NULL; \
      } \
      const std::pair<std::string, PlugInFactory*>& plugin = factories()[plugInNumber]; \
      return plugin.second->createPlugIn(plugin.first); \
   } \
   }

#define GENERATE_DYNAMIC_FACTORY(moduleNamespace__, dynamicFactoryClass__) \
   namespace moduleNamespace__ { \
   static dynamicFactoryClass__ sDynamicFactoryInstance__; \
   std::vector<std::pair<std::string, PlugInFactory*> >& factories() \
   { \
      static std::vector<std::pair<std::string, PlugInFactory*> > sFactories; \
      return sFactories; \
   } \
   void addFactory(PlugInFactory* pFactory) \
   { \
      if (pFactory != NULL) \
      { \
         std::vector<std::string> names = pFactory->getPlugInNames(); \
         for (std::vector<std::string>::iterator nameIter = names.begin(); nameIter != names.end(); ++nameIter) \
         { \
            factories().push_back(make_pair(*nameIter, pFactory)); \
         } \
      } \
   } \
   PlugIn* getPlugIn(unsigned int plugInNumber) \
   { \
      static bool factoriesSorted = false; \
      if (!factoriesSorted) \
      { \
         factoriesSorted = true; \
         DynamicPlugInFactory* pFac = &sDynamicFactoryInstance__; \
         addFactory(pFac); \
         FactoryPtrComparator comp; \
         std::sort(factories().begin(), factories().end(), comp); \
      } \
      if (plugInNumber >= factories().size()) \
      { \
         return NULL; \
      } \
      const std::pair<std::string, PlugInFactory*>& plugin = factories()[plugInNumber]; \
      return plugin.second->createPlugIn(plugin.first); \
   } \
   }

#endif

/**
 *  \cond INTERNAL
 *  Plug-in management within a module
 *
 *  The module manager is used to inform the main application about the
 *  plug-ins within the module.  It is also used to create, destroy, and 
 *  provide access to the plug-ins.  Plug-in developers edit this class to 
 *  build a plug-in module composed of their plug-ins. This is a singleton 
 *  class, where only one instance of this class exists at a given time.  Use 
 *  the instance() method to get a reference to the class.
 */
class ModuleManager
{
protected:
   /**
    *  Constructor which can NOT be called outside this class.
    *
    *  The default constructor is protected to control the number of
    *  instances of this class.  Use the instance() method to obtain
    *  access to the class.
    */
   ModuleManager();

   /**
    *  Destructor which can not be invoked outside this class.
    *
    *  This destructor deletes all dynamic memory associated with the
    *  class.
    */
   ~ModuleManager();

public:
   /**
    *  Returns the instance of this singleton class.
    *
    *  This method controls the instantiation of this class and
    *  returns a reference to the singleton instance.  If the class
    *  has not been instantiated, it creates the instance, stores
    *  it internally, and then returns a reference to the new 
    *  reference.
    *
    *  @return  This method returns the singleton class instance.
    */
   static ModuleManager* instance();

   /**
    *  Gets the name of the module.
    *
    *  This method is used to get the name of the module.
    *
    *  @return  This method returns the string name of the module.
    */
   static char *getName()
   {
      return (char*)mspName;
   }

   /**
    *  Gets the version of the module.
    *
    *  This method is used to get the version of the module.  The
    *  version is a string in the following format: "1.2.3".
    *
    *  @return  This method returns the string version of the module.
    */
   static char *getVersion()
   {
      return (char*)mspVersion;
   }

   /**
    *  Gets the description of the module.
    *
    *  This method is used to get a string describing the purpose
    *  and operation of the module.
    *
    *  @return  A string containing the description of the module.
    */
   static char *getDescription()
   {
      return (char*)mspDescription;
   }

   /**
    *  Returns the number of plug-ins within the module.
    *
    *  This method may be implemented to allow for a dynamic number
    *  of plug-ins.  This may be useful for a plug-in suite which can
    *  be configured based on data files present.  Each time this method
    *  is called, it may return a different number.
    *
    *  @return  The number of plug-ins within the module.
    */
   static unsigned int getTotalPlugIns();

   static char *getValidationKey()
   {
      return (char*)mspValidationKey;
   }

   /**
    *  Sets the reference to the main application External services.
    *
    *  This method sets the reference to the services class provided
    *  by the main application.  The services class can be used to
    *  query the main application for services that it provides and
    *  get access to those services.
    *
    *  @param   address
    *           The External services class to set.
    */
   void setService( External* address )
   {
      mpServices = address;
   }

   /**
    *  Gets the reference to the main application External services.
    *
    *  This method gets the reference to the services class provided
    *  by the main application.  The services class can be used to
    *  query the main application for services that it provides and
    *  get access to those services.
    *
    *  @return  Returns a reference to the main application External
    *           services.
    *
    *  @see     External
    */
   External* getService()
   {
      return mpServices;
   }

   /** 
    *  Creates an instance of a plug-in.
    *
    *  This method is used to create an instance of a Plug-In.  PLUG-IN
    *  DEVELOPERS SHOULD EDIT THIS CLASS TO ADD INSTANCE CREATION FOR THEIR
    *  PLUG-IN.
    *
    *  Plug-in developers are responsible for ensuring a consistent ordering
    *  to plug-ins for each time the module is queried for its plug-in count.
    *  If this module has dynamically defined plug-ins, the mapping of plug-in
    *  number to plug-in must remain constant until getTotalPlugIns() is called

    *
    *  @param   plugInNumber
    *           The plug-in to create within the module.  The number of the
    *           first plug-in in the module is zero.
    *
    *  @return  This method returns a pointer to the new plug-in.
    *
    */
   PlugIn* getPlugIn( unsigned int plugInNumber );

   /**
    *  Returns the UUID for the module.
    *
    *  This method returns a unique id for the module. This value must be the
    *  same between instances of the application. It should be formatted as a
    *  UUID.
    *
    *  @return This method returns a module's unique id.
    */
   static char *getUniqueId()
   {
      return const_cast<char*>(mspUniqueId);
   }

private:
   static const char*    mspName;
   static const char*    mspVersion;
   static const char*    mspDescription;
   static const char*    mspValidationKey;
   static const char*    mspUniqueId;

   External*             mpServices;

   static ModuleManager* mspSingleton;
};
/// \endcond

#endif
