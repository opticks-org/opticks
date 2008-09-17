/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODULEDESCRIPTOR_H
#define MODULEDESCRIPTOR_H 

#include "DateTimeImp.h"
#include "DynamicModule.h"
#include "PlugInManagerServices.h"
#include "SessionItem.h"
#include "SessionItemImp.h"

#include <map>
#include <string>
#include <vector>

class PlugIn;
class PlugInDescriptorImp;

/**
 *  Module Descriptor
 *
 *  This class is responsible for loading the Module, storing all information 
 *  about a Module, and providing the interface to access the Plug-Ins within 
 *  a the Module.  This class is available even when the Plug-In Module is not 
 *  loaded.
 */
class ModuleDescriptor : public SessionItem, public SessionItemImp
{
public:
   /**
    *  Default constructor for a Module
    *
    *  This constructor creates an empty Module Descriptor.
    *  No module is loaded and only default information is set.
    *
    *  @param   id
    *           The unique ID for the module.
    */
   ModuleDescriptor(const std::string& id);

   /**
    *  Main constructor for a Module
    *
    *  This constructor creates a Module Descriptor for Dynamic
    *  Library associated with the given filename parameter.  The
    *  Dynamic Library is loaded, queryied to determine if it is
    *  a real Module, and then loads all information about
    *  the Plug-Ins within the Module.  The Module is then
    *  released and its memory is freed.
    *
    *  @param   id
    *           The unique ID for the module.
    *  @param   file
    *           Full filename path of the Module.
    */
   ModuleDescriptor(const std::string& id, const std::string& file, std::map<std::string, std::string> &plugInIds);

   /**
    *  Default desctructor for a Module
    *
    *  This destructor deletes all dynamic memory associated
    *  with the ModuleDescriptor.  As part of this destructor,
    *  the destructor for the Dynamic Module is called, which
    *  unloads and deletes its associated Dynamic Link Library
    *  or Dynamic Shared Object.
    */
   ~ModuleDescriptor();

   /**
    *  Loads the Module
    *
    *  The load() method loads and Module.  This
    *  method must be called before a Plug-In is executed.
    *
    *  @return true if the operation succeeded (including if
    *          the module is already loaded), false otherwise.
    */
   virtual bool load();

   /**
    *  Unloads the module if it is currently loaded.
    */
   virtual void unload();

   /**
    *  Initialize plugin descriptors for the plugins
    *  within this module.
    *
    *  @return  This method returns true if the Module initialization
    *           is sucessful.
    */
   bool initializePlugInInformation(std::map<std::string, std::string> &plugInIds);

   /**
    *  Instantiates one of the Module's Plug-Ins
    *
    *  The createInterface() method is used to instantiate one of a
    *  Module's Plug-Ins.
    *
    *  @param   plugInNumber
    *           Plug-In to access within the Module.  First Plug-In number
    *           starts with zero.
    *
    *  @return  A pointer to the created interface.
    */
   virtual PlugIn* createInterface(unsigned int plugInNumber);

   /**
    *  Instantiates one of the Module's Plug-Ins
    *
    *  The createInterface() method is used to instantiate one of a 
    *  Module's Plug-Ins.  
    *
    *  @param   plugIn
    *           Reference to the Plug-In you want to create.
    *
    *  @return  A pointer to the created interface.
    */
   virtual PlugIn* createInterface(PlugInDescriptorImp* plugIn);

   /**
    *  Test whether a given file is a Module
    *
    *  The isModule() method is a class method that loads the
    *  Dynamic Library and tests to see if it is a valid Module.
    *  Since this method is static it can be called without
    *  creating an instance of a Module Descriptor.  Simply
    *  call ModuleDescriptor::isModule( filename ).  If the given
    *  file is a Module, the method returns true.  The test tries to
    *  call procedure that is part of the Module interface,
    *  defined in the Module.h header file.
    *
    *  @param    fileName
    *            The given file to test whether it is a valid
    *            Module.
    *
    *  @return   This method returns true if the given filename
    *            is a valid Module.
    */
   static bool isModule(const std::string& fileName);

   virtual const bool isValidatedModule() const;

   /**
    *  Get the version of the Module
    *
    *  The getVersion() method is used to get the Version of the 
    *  Module.  Version is a string in the follwing format "1.2.3".
    *
    *  @return   This method returns the string version of the Module.
    */
   const std::string getVersion() const
   {
      return mVersion;
   }

   /**
    *  Get the description of the Module
    *
    *  The getDescription() method is used to get a string describing
    *  the purpose and operation of the Module.
    *
    *  @return   This method returns a string containing the 
    *            description of the Module.
    */
   const std::string getDescription() const
   {
      return mDescription;
   }

   /**
    *  Get the file name of the Module.
    *
    *  The getFileName() method is used to get a string containing
    *  the full path name of the file.
    *
    *  @return   This method returns the string file name of the Module.
    */
   const std::string getFileName() const
   {
      return mFileName;
   }

   /**
    *  Get the file size of the Module in bytes.
    *
    *  The getFileSize() method is used to get the size in bytes of
    *  the Module file.
    *
    *  @return   This method returns size in bytes of the Module file.  
    */
   const double getFileSize() const
   {
      return mFileSize;
   }

   /**
    *  Get the file modification date of the Module.
    *
    *  The getFileDate() method returns a string representing the
    *  last date of the Module file's modification.  The format of
    *  the date is "MM/DD/YY".
    *
    *  @return   This method returns a string representing the
    *            last modification date of the Module file.
    */
   const DateTime* getFileDate() const
   {
      return &mFileDate;
   }

   /**
    *  Return the number of Plug-Ins within the Module
    *
    *  The getNumPlugIns() method returns number of Plug-Ins
    *  within the Module. 
    *
    *  @return  The method returns the number of Plug-Ins within
    *           the Module.
    */
   const unsigned int getNumPlugIns() const
   {
      return mPlugInTotal;
   }

   /**
    *  Returns a list of Descriptors representing the Module's Plug-Ins
    *
    *  The getPlugInSet() method returns a list of Plug-In Descriptors
    *  representing of Plug-Ins within the Module.
    *
    *  @return   This method returns a vector of Plug-In Descriptors.
    */
   std::vector<PlugInDescriptorImp*> getPlugInSet() const
   {
      return mPlugins;
   }

   /**
    *  Has the Module been loaded?
    *
    *  The isLoaded() method returns true if the Module has been loaded
    *  and is resident in memory.
    *
    *  @return  The method returns true if the Module is in memory.
    */
   virtual bool isLoaded() const
   {
      if (mpModule == NULL)
      {
         return false;
      }

      return mpModule->isLoaded();
   }

   /**
    *  Retrieves the UUID from the specified module.
    *
    *  This method opens the specified module file and retrieves its unique
    *  id. If the file isn't a plug-in module, an empty string will be returned.
    *
    *  @param filename
    *            The filename of the plug-in module to read the id from.
    *
    *  @return The unique id specified by the module.
    */
   static std::string getModuleId(const std::string &filename);
   SESSIONITEMACCESSOR_METHODS(SessionItemImp)
   /**
    * @copydoc SessionItem::serialize()
    */
   virtual bool serialize(SessionItemSerializer &serializer) const;

   /**
    * @copydoc SessionItem::deserialize()
    */
   virtual bool deserialize(SessionItemDeserializer &deserializer);

protected:
   std::string  mVersion;
   std::string  mDescription;
   unsigned int mPlugInTotal;
   std::string  mValidationKey;

   std::string  mFileName;
   double       mFileSize;
   DateTimeImp  mFileDate;

   std::vector<PlugInDescriptorImp*> mPlugins;
   DynamicModule* mpModule;
   Service<PlugInManagerServices> mpPluginMgrSvc;
};

#endif
