/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORT_AGENT_H
#define IMPORT_AGENT_H

#include "EnumWrapper.h"
#include "ExecutableAgent.h"

#include <string>
#include <vector>

class DataElement;
class ImportDescriptor;

/**
 *  This is a helper class that makes working with Importer
 *  plug-ins easier.  This class will manage the lifecycle of
 *  the Importer plug-in. This class can be used
 *  to import a file or just individual datasets with the
 *  file.  After executing the importer, the imported data sets
 *  can be retrieved by calling getImportedElements().
 *  You should not create an instance of this class using
 *  ObjectFactory, but you should use ImporterResource.
 *
 *  @warning If you do not call an overload of instantiate() before
 *  calling any other methods, a std::logic_error will be thrown.
 *  You cannot call instantiate() twice on the same instance
 *  or a std::logic_error will be thrown.
 *
 *  @see        ImporterResource, Importer
 */
class ImportAgent : public ExecutableAgent
{
public:
   /**
    *  Specifies the conditions in which the user can edit the import data
    *  parameters.
    *
    *  @see     setEditType()
    */
   enum EditTypeEnum
   {
      NEVER_EDIT,       /**< The user can never edit the import parameters
                             before the import is performed. */
      AS_NEEDED_EDIT,   /**< The user can only edit the import parameters if
                             the importer cannot import the data with the
                             current parameters. */
      ALWAYS_EDIT       /**< The user is always given the chance to edit the
                             import parameters before the import is
                             performed. */
   };

   /**
    *  @EnumWrapper  EditTypeEnum
    */
   typedef EnumWrapper<EditTypeEnum> EditType;

   /**
    *  Creates an invalid object for delayed initialization of an ImportAgent.
    *
    *  Creates an invalid object where no importer is initially created.
    *  The importer can then be initialized later by calling
    *  setPlugIn().
    *
    *
    *  @param   pProgress
    *           The progress object to pass into the importer.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the importer in batch mode
    *           or to \b false to execute the importer in interactive mode.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(Progress* pProgress, bool batch) = 0;

   /**
    *  Creates an importer to import data.
    *
    *  Creates the importer but does not specify the file or data sets to
    *  import.  The setFilename() method or setImportDescriptors() method
    *  must therefore be called before calling execute().
    *
    *  @param   importerName
    *           The name of the importer to create and execute.
    *  @param   pProgress
    *           The progress object to pass into the importer.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the importer in batch mode
    *           or to \b false to execute the importer in interactive mode.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(const std::string& importerName, Progress* pProgress, bool batch) = 0;

   /**
    *  Creates an importer to import data from a file.
    *
    *  When importing data from a file, all data sets returned by
    *  Importer::getImportDescriptors() are imported.
    *
    *  @param   importerName
    *           The name of the importer to create and execute.
    *  @param   filename
    *           The file from which to load the data.
    *  @param   pProgress
    *           The progress object to pass into the importer.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the importer in batch mode
    *           or to \b false to execute the importer in interactive mode.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(const std::string& importerName, const std::string& filename, Progress* pProgress,
      bool batch) = 0;

   /**
    *  Creates an importer to import individual data sets.
    *
    *  @param   importerName
    *           The name of the importer to create and execute.
    *  @param   descriptors
    *           The data sets to import.  The descriptor values should have
    *           been obtained by calling Importer::getImportDescriptors().  The
    *           provided descriptors will be owned by this agent.
    *  @param   pProgress
    *           The progress object to pass into the importer.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the importer in batch mode
    *           or to \b false to execute the importer in interactive mode.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(const std::string& importerName, const std::vector<ImportDescriptor*>& descriptors,
                    Progress* pProgress, bool batch) = 0;

   /**
    *  Uses an existing importer to import individual data sets.
    *
    *  @param   pPlugIn
    *           The importer to execute.  The agent assumes ownership of the
    *           importer and will destroy it as necessary upon agent
    *           destruction.
    *  @param   descriptors
    *           The data sets to import.  The descriptor values should have
    *           been obtained by calling Importer::getImportDescriptors().  The
    *           provided descriptors will be owned by this agent.
    *  @param   pProgress
    *           The progress object to pass into the importer.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the importer in batch mode
    *           or to \b false to execute the importer in interactive mode.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(PlugIn* pPlugIn, const std::vector<ImportDescriptor*>& descriptors, Progress* pProgress,
                    bool batch) = 0;

   /**
    *  Sets the agent to import all data from a file.
    *
    *  This method sets the file to use to import data.  %Any existing data
    *  descriptors are removed but not deleted.  After calling this method,
    *  the next call to execute() will call Importer::getImportDescriptors() to
    *  determine the data sets to import.
    *
    *  @param   filename
    *           The name of the file to import.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    */
   virtual void setFilename(const std::string& filename) = 0;

   /**
    *  Filters importers displayed in the import dialog based on subtype.
    *
    *  If the agent executes in interactive mode and no filename has been set,
    *  the import dialog is displayed for the user to choose a file to import
    *  and the importer to use.  This method filters the available importers in
    *  the dialog based on the given subtype.  Only importers that have a
    *  subtype equal to the given subtype will be displayed in the dialog.
    *
    *  By default, if this method is not called, all importers are displayed
    *  in the import dialog.
    *
    *  This method does nothing if ApplicationServices::isInteractive() returns
    *  \c false.
    *
    *  @param   subtype
    *           The subtype for importers that should be displayed in the
    *           import dialog.  Passing in an empty string will display all
    *           importers in the import dialog.
    *
    *  @throw   std::logic_error
    *           Thrown if the ImportAgent::instantiate() method has not yet
    *           been called.
    *
    *  @see     ImportAgent::instantiate()
    */
   virtual void setImporterSubtype(const std::string& subtype) = 0;

   /**
    *  Returns the subtype of the importers displayed in the import dialog.
    *
    *  @throw   std::logic_error
    *           Thrown if ImportAgent::instantiate() method has not yet been
    *           called.
    *
    *  @return  Returns the subtype of the importers displayed in the import
    *           dialog.  Returns an empty string if all importers are displayed
    *           or if ApplicationServices::isInteractive() returns \c false.
    */
   virtual std::string getImporterSubtype() const = 0;

   /**
    *  Allows the user to edit the import data parameters before importing the
    *  data.
    *
    *  This method sets when the user should be able to edit the data to be
    *  imported.  If editing is enabled, the import options dialog is displayed
    *  for the user to make any desired changes.
    *
    *  The default state of the agent is to allow the user to edit the data
    *  on an as-needed basis.
    *
    *  This method does nothing if ApplicationServices::isInteractive() returns
    *  \c false.
    *
    *  @param   editType
    *           The conditions in which the user can edit the import data
    *           parameters.
    *
    *  @throw   std::logic_error
    *           Thrown if the ImportAgent::instantiate() method has not yet
    *           been called.
    */
   virtual void setEditType(EditType editType) = 0;

   /**
    *  Returns the conditions in which the user can edit the import data
    *  parameters before importing the data.
    *
    *  @throw   std::logic_error
    *           Thrown if ImportAgent::instantiate() method has not yet been
    *           called.
    *
    *  @return  Returns the conditions in which the user can edit the import
    *           data parameters.  Returns EditType::NEVER_EDIT if
    *           ApplicationServices::isInteractive() returns \c false.
    */
   virtual EditType getEditType() const = 0;

   /**
    *  Adds the imported file to the most recently used (MRU) file list.
    *
    *  This method allows for imported files to be added to the MRU file list
    *  so that users can quickly reimport the data at a later time.  Typically
    *  files should only be added to the MRU list if the user initiated the
    *  import.  Otherwise, it may confuse the user to see a file in the list
    *  that was not directly imported.
    *
    *  The filename that is added to the MRU file list is the filename that is
    *  passed into the ImporterResource constructor or into the
    *  ImportAgent::setFilename() method.  If a filename is not set in the
    *  agent and the import dialog is displayed, the file that the user selects
    *  in the dialog is added to the MRU file list.  Otherwise, no filename is
    *  added to the MRU list.
    *
    *  By default, if this method is not called, imported files are not added
    *  to the MRU file list.
    *
    *  @param   updateList
    *           Set this parameter to \c true to add imported files to the MRU
    *           file list.
    *
    *  @throw   std::logic_error
    *           Thrown if the ImportAgent::instantiate() method has not yet
    *           been called.
    *
    *  @see     ImportAgent::instantiate()
    */
   virtual void updateMruFileList(bool updateList) = 0;

   /**
    *  Queries whether an imported file is added to the most recently used
    *  (MRU) file list.
    *
    *  @throw   std::logic_error
    *           Thrown if ImportAgent::instantiate() method has not yet been
    *           called.
    *
    *  @return  Returns \c true if an imported file is added to the MRU file
    *           list, otherwise returns \c false.
    *
    *  @see     updateMruFileList()
    */
   virtual bool isMruFileListUpdated() const = 0;

   /**
    *  Sets the import options dialog to be displayed before importing the
    *  data.
    *
    *  By default, the agent does not show the import options dialog unless
    *  this method is called with a value of \c true.
    *
    *  This method does nothing if ApplicationServices::isInteractive() returns
    *  \c false.
    *
    *  @deprecated   This method is obsolete and should not be called.  It may
    *                be removed in a future version.  Call setEditType()
    *                instead.
    *
    *  @param   showDialog
    *           Set to \c true to display the options dialog or to \c false to
    *           not display the dialog before importing the data.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @see     isOptionsDialogShown()
    */
   virtual void showOptionsDialog(bool showDialog) = 0;

   /**
    *  Queries whether the import options dialog is displayed before importing
    *  the data.
    *
    *  @deprecated   This method is obsolete and should not be called.  It may
    *                be removed in a future version.  Call getEditType()
    *                instead.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  Returns \c true if the options dialog is displayed or \c false
    *           if the dialog is not displayed before the data is imported.
    *           This method also returns \c false if showOptionsDialog() has
    *           not been called or if ApplicationServices::isInteractive()
    *           returns \c false.
    */
   virtual bool isOptionsDialogShown() const = 0;

   /**
    *  Sets the data sets to import.
    *
    *  This method sets the data sets that will be imported.  The agent will
    *  take ownership over the provided import descriptors.  After calling this
    *  method, the next call to execute() will import these data sets and will
    *  not call Importer::getImportDescriptors() to obtain the data sets to
    *  import.
    *
    *  @param   descriptors
    *           The data sets to import.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    */
   virtual void setImportDescriptors(const std::vector<ImportDescriptor*>& descriptors) = 0;

   /**
    *  Returns the data sets that will be used on import.
    *
    *  This method calls returns the data sets specified in instantiate() or
    *  from setImportDescriptors().  If the import descriptors have not been
    *  set, Importer::getImportDescriptors() is called using the filename set
    *  in instantiate() or from setFilename().  These descriptors are owned by
    *  the agent.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  The import descriptors that will be used on import.
    */
   virtual std::vector<ImportDescriptor*> getImportDescriptors() = 0;

   /**
    *  Gets the default file extensions from the importer.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return  The default extensions string returned by calling
    *           Importer::getDefaultExtensions().
    */
   virtual std::string getDefaultExtensions() const = 0;

   /**
    *  Executes the importer to load the data.
    *
    *  This method first creates the input and output arg lists if necessary,
    *  populates the input arg list, and then executes the importer.
    *
    *  If the import descriptors of the data sets to import have not been set,
    *  getImportDescriptors() is called.  The model element is created using
    *  the data descriptor contained in each import descriptor, and the
    *  ExecutableAgent::execute() base class method is called to execute the
    *  importer.  All successfully imported data can be retrieved by calling
    *  getImportedElements().
    *
    *  @par Arg Values:
    *  The following input arg values are automatically populated unless the
    *  actual value in the arg has already been set and
    *  PlugInArg::isActualSet() returns \c true:
    *  - All args documented in ExecutableAgent::execute().
    *  - Importer::ImportElementArg() is populated with the DataElement that
    *    was created for each data set based on each DataDescriptor contained
    *    in the ImportDescriptor returned from getImportDescriptors().
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  If the options dialog is displayed, this method returns
    *           \c false if the user cancels the dialog or if the user
    *           deselects all data sets to import in the dialog.  Otherwise,
    *           this method returns the success value returned by the importer.
    *
    *  @see     isOptionsDialogShown()
    */
   virtual bool execute() = 0;

   /**
    *  Returns all successfully imported data elements.
    *
    *  The execute() method must be called prior to calling this method for
    *  the returned vector to be populated.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return  A vector containing pointers to the data elements that were
    *           created as a result of calling execute().  An empty vector is
    *           returned if execute() has not previously been called or if the
    *           importer was not able to import any data.
    */
   virtual std::vector<DataElement*> getImportedElements() const = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~ImportAgent() {}
};

#endif
