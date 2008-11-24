/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINMANAGERSERVICESIMP_H
#define PLUGINMANAGERSERVICESIMP_H

#include "PlugInManagerServices.h"
#include "SubjectImp.h"

#include <map>
#include <set>
#include <string>
#include <vector>

class DataElement;
class DynamicModule;
class Layer;
class ModuleDescriptor;
class PlotWidget;
class PlugIn;
class PlugInArg;
class PlugInArgList;
class PlugInDescriptor;
class PlugInDescriptorImp;
class Progress;
class View;
class WorkspaceWindow;

class PlugInManagerServicesImp : public PlugInManagerServices, public SubjectImp
{
public:
   static PlugInManagerServicesImp* instance();
   static void destroy();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SUBJECTADAPTER_METHODS(SubjectImp)

   // Module
   DynamicModule* getDynamicModule(const std::string& value);
   bool destroyDynamicModule(DynamicModule* pModule);
   ModuleDescriptor* getModuleDescriptor(const std::string& moduleFilename);
   ModuleDescriptor* getModuleDescriptorByName(const std::string& moduleName);
   const std::vector<ModuleDescriptor*>& getModuleDescriptors() const;

   // Plug-ins
   void buildPlugInList(const std::string& plugInPath);
   void clear();
   std::vector<PlugInDescriptor*> getPlugInDescriptors(const std::string& plugInType = "") const;
   std::vector<PlugIn*> getPlugInInstances(const std::string& plugInName = std::string());
   PlugIn* createPlugIn(const std::string& plugInName);
   PlugIn* createPlugInInstance(const std::string& plugInName, const std::string& id = std::string());
   bool destroyPlugIn(PlugIn* pPlugIn);
   PlugInDescriptor* getPlugInDescriptor(const std::string& plugInName) const;
   PlugInDescriptor* getPlugInDescriptor(PlugIn* pPlugIn) const;
   void listPlugIns( bool showModules, bool showPlugIns, bool fullDetail );
   Progress* getProgress(PlugIn* pPlugIn);
   void executeStartupPlugIns(Progress* pProgress) const;

   // Plug-in args
   PlugInArgList* getPlugInArgList();
   PlugInArg* getPlugInArg();
   bool destroyPlugInArgList(PlugInArgList* pArgList);
   const std::vector<std::string>& getArgTypes();

protected:
   PlugInManagerServicesImp();
   virtual ~PlugInManagerServicesImp();

   ModuleDescriptor* addModule(const std::string& moduleFilename, std::map<std::string, std::string>& plugInIds);
   bool containsModule(ModuleDescriptor* pModule);
   bool removeModule(ModuleDescriptor* pModule, std::map<std::string, std::string>& plugInIds);

private:
   static PlugInManagerServicesImp* spInstance;
   static bool mDestroyed;
   
   std::vector<std::string> mExcludedPlugIns;
   std::vector<ModuleDescriptor*> mModules;
   std::map<std::string, PlugInDescriptorImp*> mPlugIns;
};

#endif
