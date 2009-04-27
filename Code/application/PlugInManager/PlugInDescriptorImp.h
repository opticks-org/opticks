/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINDESCRIPTORIMP_H
#define PLUGINDESCRIPTORIMP_H

#include "PlugInDescriptor.h"
#include "SessionItemImp.h"

#include <map>
#include <string>
#include <vector>

class PlugIn;
class PlugInArgList;
class Progress;
class ProgressAdapter;

/**
 *  Describes a plug-in.
 *
 *  This class store all descriptive information about a plug-in.  This class
 *  is available even when the plug-in module is not loaded.  This class also
 *  provides a means to access all instances of a plug-in.
 */
class PlugInDescriptorImp : public PlugInDescriptor, public SessionItemImp
{
public:
   /**
    *  Creates the plug-in descriptor.
    *
    *  The constructor uses the given plug-in and stores the data in the PlugIn
    *  interface, the abort flag and plug-in arg lists from the Executable
    *  interface, and potentially other information from other plug-in
    *  interfaces.
    *
    *  @param   id
    *           The unique ID for the plug-in descriptor.
    *  @param   pPlugIn
    *           The plug-in from which to create the descriptor.
    */
   PlugInDescriptorImp(const std::string& id, PlugIn* pPlugIn);

   /**
    *  Destroys the plug-in descriptor.
    *
    *  This destructor also destroys all instances of the plug-in.
    */
   ~PlugInDescriptorImp();

   /**
    *  Sets the name of the module containing this plug-in.
    *
    *  @param   moduleName
    *           The name of the module containing this plug-in.
    */
   void setModuleName(const std::string& moduleName);

   /**
    *  Sets the filename of the module containing this plug-in.
    * 
    *  @param   moduleFilename
    *           The filename of the module containing this plug-in.
    */
   void setModuleFileName(const std::string& moduleFilename);

   /**
    *  Sets the index number of the plug-in within the module.
    *
    *  This method sets a number signifying the location of the plug-in within
    *  the module.
    *
    *  @param   ulPlugIn
    *           The number of the plug-in within the module.  The first plug-in
    *           has a number of 0, the second plug-in has a number of 1, and so
    *           on.
    */
   void setPlugInNumber(unsigned int ulPlugIn);

   /**
    *  Returns the name of the module containing this plug-in.
    *
    *  @return  The name of the module containing this plug-in.
    */
   std::string getModuleName() const;

   /**
    *  Returns the filename of the module containing this plug-in.
    *
    *  @return  The filename of the module containing this plug-in.
    */
   std::string getModuleFileName() const;

   /**
    *  Returns the index number of the plug-in within the module.
    *
    *  This method returns a number signifying the location of the plug-in
    *  within the module.
    *
    *  @return  The number of the plug-in within the module.  The number
    *           ranges from one to the number of plug-in in the module
    *           minus one.
    */
   unsigned int getPlugInNumber() const;

   std::string getVersion() const;
   bool isProduction() const;
   std::string getCreator() const;
   std::string getCopyright() const;
   std::map<std::string, std::string> getDependencyCopyright() const;
   std::string getDescription() const;
   std::string getShortDescription() const;
   std::string getType() const;
   std::string getSubtype() const;
   bool areMultipleInstancesAllowed() const;

   bool hasExecutableInterface() const;
   bool isExecutedOnStartup() const;
   bool isDestroyedAfterExecute() const;
   const std::vector<std::string>& getMenuLocations() const;
   bool hasAbort() const;
   bool hasBatchSupport() const;
   bool hasInteractiveSupport() const;
   bool hasWizardSupport() const;
   const PlugInArgList* getBatchInputArgList() const;
   const PlugInArgList* getInteractiveInputArgList() const;
   const PlugInArgList* getBatchOutputArgList() const;
   const PlugInArgList* getInteractiveOutputArgList() const;

   bool hasImporterInterface() const;
   bool hasExporterInterface() const;
   bool hasInterpreterInterface() const;
   std::string getFileExtensions() const;

   bool hasTestableInterface() const;
   bool isTestable() const;

   bool addPlugIn(PlugIn* pPlugIn);
   bool containsPlugIn(PlugIn* pPlugIn) const;
   std::vector<PlugIn*> getPlugIns() const;
   int getNumPlugIns() const;
   Progress* getProgress(PlugIn* pPlugIn);
   bool destroyPlugIn(PlugIn* pPlugIn);

   SESSIONITEMACCESSOR_METHODS(SessionItemImp)

   /**
    * @copydoc SessionItem::serialize()
    */
   virtual bool serialize(SessionItemSerializer& serializer) const;

   /**
    * @copydoc SessionItem::deserialize()
    */
   virtual bool deserialize(SessionItemDeserializer& deserializer);

private:
   void destroyPlugIns();

   std::string mModuleName;
   std::string mModuleFilename;
   unsigned int mPlugInNumber;

   std::string mVersion;
   bool mProductionStatus;
   std::string mCreator;
   std::string mCopyright;
   std::map<std::string, std::string> mDependencyCopyright;
   std::string mDescription;
   std::string mShortDescription;
   std::string mType;
   std::string mSubtype;
   bool mAllowMultipleInstances;

   bool mExecutableInterface;
   bool mExecuteOnStartup;
   bool mDestroyAfterExecute;
   std::vector<std::string> mMenuLocations;
   bool mAbort;
   bool mWizardSupport;
   bool mBatchSupport;
   bool mInteractiveSupport;
   PlugInArgList* mpBatchInArgList;
   PlugInArgList* mpInteractiveInArgList;
   PlugInArgList* mpBatchOutArgList;
   PlugInArgList* mpInteractiveOutArgList;

   bool mImporterInterface;
   bool mExporterInterface;
   bool mInterpreterInterface;
   std::string mFileExtensions;

   bool mTestableInterface;
   bool mTestable;

   std::map<PlugIn*, ProgressAdapter*> mPlugIns;
};

#endif
