/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINMANAGERSERVICES_H
#define PLUGINMANAGERSERVICES_H

#include "Service.h"
#include "Subject.h"
#include "TypesFile.h"

#include <vector>
#include <string>

class DataElement;
class DynamicModule;
class Layer;
class PlotWidget;
class PlugIn;
class PlugInArg;
class PlugInArgList;
class PlugInDescriptor;
class Progress;
class View;
class WorkspaceWindow;

/**
 *  \ingroup ServiceModule
 *  Provides access to the plug-in argument list and their arguments.
 *
 *  PlugInManagerServices provides methods for creating and destroying
 *  PlugInArgs and ArgLists. These arg lists are used for passing arguments
 *  to plugins. A plug-in needs to create a new input and output arglist
 *  every time its getInputSpecification or getOutputSpecification methods
 *  are called. It is the responsibility of the caller of the plug-in to
 *  destroy the arg lists when they are done with them.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The plug-in list is updated.
 *  - A plug-in instance is created.
 *  - A plug-in instance is destroyed.
 *  - Everything else documented in Subject.
 *
 *  @see        PlugInArg, PlugInArgList
 */
class PlugInManagerServices : public Subject
{
public:
   /**
    * The type that should be returned from PlugIn::getType()
    * for types of plug-in that implement the Importer interface.
    *
    * @return Returns the type used for importer plug-ins.
    */
   static std::string ImporterType()
   {
      return "Importer";
   }
   
   /**
    * The type that should be returned from PlugIn::getType()
    * for algorithm plug-ins.
    *
    * @return Returns the type used for algorithm plug-ins.
    *
    * @see AlgorithmShell
    */
   static std::string AlgorithmType()
   {
      return "Algorithm";
   }

   /**
    *  The type that should be returned from PlugIn::getType() for dock window
    *  plug-ins.
    *
    *  @return  Returns the type used for dock window plug-ins.
    *
    *  @see     DockWindowShell
    */
   static std::string DockWindowType()
   {
      return "DockWindow";
   }

   /**
    * The type that should be returned from PlugIn::getType()
    * for types of plug-in that implement the Exporter interface.
    *
    * @return Returns the type used for exporter plug-ins.
    */
   static std::string ExporterType()
   {
      return "Exporter";
   }

   /**
    * The type that should be returned from PlugIn::getType()
    * for types of plug-in that implement the Georeference interface.
    *
    * @return Returns the type used for georeference plug-ins.
    */
   static std::string GeoreferenceType()
   {
      return "Georeference";
   }

   /**
    * The type that should be returned from PlugIn::getType()
    * for types of plug-in that implement the Interpreter interface.
    *
    * @return Returns the type used for interpreter plug-ins.
    */
   static std::string InterpreterType()
   {
      return "Interpreter";
   }

   /**
    *  The type that should be returned from PlugIn::getType()
    *  for types of plug-ins that implement the Properties interface.
    *
    *  @return  Returns the type used for properties plug-ins.
    *
    *  @see     PropertiesShell
    */
   static std::string PropertiesType()
   {
      return "Properties";
   }

   /**
    * The type that should be returned from PlugIn::getType()
    * for types of plug-in that implement the RasterPager interface.
    *
    * @return Returns the type used for raster pager plug-ins.
    */
   static std::string RasterPagerType()
   {
      return "RasterPager";
   }

   /**
    * The type that should be returned from PlugIn::getType()
    * for viewer plug-ins.
    *
    * @return Returns the type used for viewer plug-ins.
    *
    * @see ViewerShell
    */
   static std::string ViewerType()
   {
      return "Viewer";
   }

   /**
    * The type that should be returned from PlugIn::getType()
    * for wizard plug-ins.
    *
    * @return Returns the type used for wizard plug-ins.
    *
    * @see WizardShell
    */
   static std::string WizardType()
   {
      return "Wizard";
   }

   /**
    * The type that should be returned from PlugIn::getType()
    * for types of plug-ins that implement the Option interface.
    */
   static std::string OptionType()
   {
      return "Option";
   }

   /**
    *  Emitted with any<PlugIn*> when a new plug-in instance is created.
    */
   SIGNAL_METHOD(PlugInManagerServices, PlugInCreated)

   /**
    *  Emitted with any<PlugIn*> when a plug-in instance is about to be destroyed.
    */
   SIGNAL_METHOD(PlugInManagerServices, PlugInDestroyed)

   /**
    *  Emitted with any<ModuleDescriptor*> when a new module descriptor is created.
    */
   SIGNAL_METHOD(PlugInManagerServices, ModuleCreated)

   /**
    *  Emitted with any<ModuleDescriptor*> when a module descriptor is about to be destroyed.
    */
   SIGNAL_METHOD(PlugInManagerServices, ModuleDestroyed)

   /**
    *  Get running instances of a plug-in.
    *
    *  This method returns all running instances of a plug-in.
    *
    *  @param  plugInName
    *          Get instances of this named plug-in. If this string is empty, it
    *          will get all of the running instances of all plug-ins.
    *
    *  @return A vector of PlugIn pointers for running instances of the plug-in.
    */
   virtual std::vector<PlugIn*> getPlugInInstances(const std::string &plugInName=std::string()) = 0;
   
   /**
    *  Creates a new instance of a plug-in.
    *
    *  This method instantiates the plug-in of the given name
    *  and returns a reference to the new plug-in.
    *
    *  @param   plugInName
    *           The name of the plug-in to create.
    *
    *  @return  A pointer to the new plug-in.  NULL is returned if
    *           the plug-in cannot be created.  This may occur if
    *           an instance of the plug in already exists and the
    *           plug-in does not support multiple instances.
    *
    *  @see     PlugIn::areMultipleInstancesAllowed()
    */
   virtual PlugIn* createPlugIn(const std::string& plugInName) = 0;

   /**
    *  Destroys an instance of a plug-in.
    *
    *  @param   pPlugIn
    *           The plug-in to destroy.
    *
    *  @return  TRUE if the plug-in was successfully destroyed, otherwise FALSE.
    */
   virtual bool destroyPlugIn(PlugIn* pPlugIn) = 0;

   /**
    * Returns a list of all PlugInDescriptors that match the given type
    * as returned by PlugIn::getType().
    *
    * @param plugInType
    *        The type of plug-ins for which to return descriptors for.  If
    *        any empty string is provided, descriptors for all available
    *        plug-ins will be returned.
    * 
    * @return Returns a list all PlugInDescriptors that match the given type.
    */
   virtual std::vector<PlugInDescriptor*> getPlugInDescriptors(const std::string& plugInType = "") const = 0;

   /** 
    * Returns the PlugInDescriptor for the given plug-in.
    *
    * @param plugInName
    *        The plug-in to return the descriptor for.
    *
    * @return Returns the PlugInDescriptor for the given plug-in or NULL if no plug-in
    *         has been registered with the given name.
    */
   virtual PlugInDescriptor* getPlugInDescriptor(const std::string& plugInName) const = 0;

   /**
    *  Create an empty plug-in arg list.
    *  
    *  Creates an empty plug-in arg list to have PlugInArgs added
    *  to it.
    *
    *  @return  A pointer to the new list, or NULL if unsuccessful.
    */
   virtual PlugInArgList* getPlugInArgList() = 0;

   /**
    *  Destroys a plug-in arg list.
    *
    *  This method destroys a plug-in arg list, including all of its
    *  plug-in args. The args should not be destroyed prior to calling
    *  this method.
    *
    *  @param   pArgList
    *           A pointer to the arg-list to destroy.
    *
    *  @return  TRUE if the list and all of its args were destroyed or
    *           FALSE otherwise.
    */
   virtual bool destroyPlugInArgList(PlugInArgList* pArgList) = 0;

   /**
    *  Get the names of the known types.
    *
    *  This method is returns the names of each recognized arg type.  Plug-ins
    *  can retrieve this list to query if a data type is in the known list.  The
    *  wizard builder uses this list connect plug-in in sequence according to their
    *  type names.  If an arg type exists that is not in this list, the wizard builder
    *  indicates it as an unknown type.
    *
    *  @return  A vector of strings specifying the name of each known arg type.
    */
   virtual const std::vector<std::string>& getArgTypes() = 0;

   /**
    *  Creates a plug-in arg.
    *
    *  Creates a blank plug-in arg that can be added to a PlugInArgList.
    *
    *  @return  A pointer to the new arg, or NULL if unsuccessful.
    */
   virtual PlugInArg* getPlugInArg() = 0;

   /**
    *  Creates an empty dynamic module.
    *
    *  @param   value
    *           The name of the plug-in.
    *
    *  @return  A pointer to the new DynamicModule, or NULL if
    *           the module could not be created.
    *
    *  @see     DynamicModule::load()
    */
   virtual DynamicModule* getDynamicModule(const std::string& value) = 0;

   /**
    *  Destroys a dynamic module.
    *
    *  @param   pModule
    *           A pointer to the dynamic module to destroy.
    *
    *  @return  TRUE if the dynamic module was destroyed or
    *           FALSE otherwise.
    */
   virtual bool destroyDynamicModule(DynamicModule* pModule) = 0;

   /**
    *  Retrieves a non-thread-safe Progress object for a given plug-in.
    *
    *  This method retrieves a valid Progress object for the given plug-in, which
    *  remains valid throughout the life of the plug-in.  Because the Progress object
    *  set into the plug-in's input arg list may be deleted when Executable::execute()
    *  returns, this method should be called for plug-ins that will stay loaded after
    *  Executable::execute() returns and that need a valid Progress object.  This is
    *  especially useful for plug-ins that create modeless dialogs.
    *
    *  If a thread-safe Progress object is needed for a background plug-in running in
    *  a separate thread, use UtilityServices::getProgress() instead.
    *
    *  @param   pPlugIn
    *           The plug-in for which to get a Progress object.
    *
    *  @return  A pointer to the Progress object for the given plug-in.
    *
    *  @see     Executable::isDestroyedAfterExecute()
    */
   virtual Progress* getProgress(PlugIn* pPlugIn) = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~PlugInManagerServices() {}
};

#endif
