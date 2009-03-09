/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINFACTORY_H
#define PLUGINFACTORY_H

#include "OptionQWidgetWrapper.h"
#include "PropertiesQWidgetWrapper.h"
#include <string>

class PlugIn;
class PlugInFactory;
void addFactory(PlugInFactory*);

/**
 * \cond INTERNAL
 * Automatically registers plug-ins in a module using convenient macros.
 *
 * Using this class requires ModuleManager.cpp to be configured for PlugInFactory use.
 * The sample ModuleManager.cpp contained in the Opticks SDK shows how to configure
 * ModuleManager.cpp for use with PlugInFactory. Direct use of this class is not needed.
 * @see PLUGINFACTORY(), PLUGINFACTORY_OPTIONWRAPPER(), PLUGINFACTORY_PROPERTIESWRAPPER()
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
   PlugInFactory(const std::string& name) : mName(name) { addFactory(this); }
   /**
    * Create a new plugin.
    *
    * @return The PlugIn object
    */
   virtual PlugIn* operator()() const { return NULL; }
   /**
    * The name of the object created by this factory.
    */
   std::string mName;

private:
   PlugInFactory() {}
};
/// \endcond

/**
 * Register a plug-in in a module.
 *
 * Each PlugIn .cpp file should use PLUGINFACTORY(pluginname) to register a factory with the module manager.
 *
 * @param name
 *        The class name of the plug-in.
 */
#define PLUGINFACTORY(name) \
   namespace \
   { \
      class name##PlugInFactory : public PlugInFactory \
      { \
      public: \
         name##PlugInFactory(const std::string& name) : PlugInFactory(name) {} \
         PlugIn* operator()() const \
         { \
            return new name; \
         } \
      }; \
      static name##PlugInFactory name##pluginFactory(#name); \
   }

/**
 * Register an options page plug-in in a module.
 *
 * Each PlugIn .cpp file should use PLUGINFACTORY_OPTIONWRAPPER(pluginname)
 * to register a factory for an OptionQWidgetWrapper with the module manager.
 *
 * @param name
 *        The class name of the options page QWidget.
 */
#define PLUGINFACTORY_OPTIONWRAPPER(name) \
   namespace \
   { \
      class name##PlugInFactory : public PlugInFactory \
      { \
      public: \
         name##PlugInFactory(const std::string& name) : PlugInFactory(name) {} \
         PlugIn* operator()() const \
         { \
            return new OptionQWidgetWrapper<name>; \
         } \
      }; \
      static name##PlugInFactory name##pluginFactory(#name); \
   }

/**
 * Register a properties page plug-in in a module.
 *
 * Each PlugIn .cpp file should use PLUGINFACTORY_PROPERTIESWRAPPER(pluginname)
 * to register a factory for an PropertiesQWidgetWrapper with the module manager.
 *
 * @param name
 *        The class name of the properties page QWidget.
 */
#define PLUGINFACTORY_PROPERTIESWRAPPER(name) \
   namespace \
   { \
      class name##PlugInFactory : public PlugInFactory \
      { \
      public: \
         name##PlugInFactory(const std::string& name) : PlugInFactory(name) {} \
         PlugIn* operator()() const \
         { \
            return new PropertiesQWidgetWrapper<name>; \
         } \
      }; \
      static name##PlugInFactory name##pluginFactory(#name); \
   }

#endif
