/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORTAGENT_H
#define IMPORTAGENT_H

#include "EnumWrapper.h"
#include "ExecutableAgent.h"

#include <map>
#include <string>
#include <vector>

class DataElement;
class ImportDescriptor;

/**
 *  A convenience class that makes working with Importer plug-ins easier.
 *
 *  This class manages the lifecycle of an Importer plug-in.  This class can be
 *  used to import one or more files or just individual data sets within one or
 *  more files.
 *
 *  After executing the importer, the imported data sets can be retrieved by
 *  calling getImportedElements().  You should not create an instance of this
 *  class using ObjectFactory, but you should use ImporterResource instead.
 *
 *  @warning    If you do not call an overload of instantiate() before calling
 *              any other methods, a std::logic_error will be thrown.  You
 *              cannot call instantiate() twice on the same instance or a
 *              std::logic_error will be thrown.
 *
 *  @see        ImporterResource, Importer
 */
class ImportAgent : public ExecutableAgent
{
public:
   /**
    *  Specifies the conditions in which the user can edit the import data
    *  parameters via the import options dialog.
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
    *  Creates an invalid object where no importer is initially created.  The
    *  importer can then be initialized later by calling setPlugIn().
    *
    *  @param   pProgress
    *           The progress object to pass into the importer.  If \c NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \c true to execute the importer in batch mode
    *           or to \c false to execute the importer in interactive mode.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than once on
    *           this instance.
    */
   virtual void instantiate(Progress* pProgress, bool batch) = 0;

   /**
    *  Creates an importer to import data.
    *
    *  Creates the importer but does not specify the files or data sets to
    *  import.  The setFilename(), setFilenames(), or setDatasets() method must
    *  therefore be called before calling execute().
    *
    *  @param   importerName
    *           The name of the importer to create and execute.
    *  @param   pProgress
    *           The progress object to pass into the importer.  If \c NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \c true to execute the importer in batch mode
    *           or to \c false to execute the importer in interactive mode.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than once on
    *           this instance.
    */
   virtual void instantiate(const std::string& importerName, Progress* pProgress, bool batch) = 0;

   /**
    *  Creates an importer to import data from a single file.
    *
    *  When importing data from a file, all data sets returned by
    *  Importer::getImportDescriptors() are imported.
    *
    *  @param   importerName
    *           The name of the importer to create and execute.
    *  @param   filename
    *           The file from which to load the data.
    *  @param   pProgress
    *           The progress object to pass into the importer.  If \c NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \c true to execute the importer in batch mode
    *           or to \c false to execute the importer in interactive mode.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than once on
    *           this instance.
    */
   virtual void instantiate(const std::string& importerName, const std::string& filename, Progress* pProgress,
      bool batch) = 0;

   /**
    *  Creates an importer to import data from multiple files.
    *
    *  When importing data from multiple files, all data sets returned by
    *  Importer::getImportDescriptors() are imported.
    *
    *  @param   importerName
    *           The name of the importer to create and execute.
    *  @param   filenames
    *           The files from which to load the data.
    *  @param   pProgress
    *           The progress object to pass into the importer.  If \c NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \c true to execute the importer in batch mode
    *           or to \c false to execute the importer in interactive mode.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than once on
    *           this instance.
    */
   virtual void instantiate(const std::string& importerName, const std::vector<std::string>& filenames,
      Progress* pProgress, bool batch) = 0;

   /**
    *  Creates an importer to import individual data sets from one or more
    *  files.
    *
    *  @param   importerName
    *           The name of the importer to create and execute.
    *  @param   descriptors
    *           The files and data sets to import.  The descriptor values should
    *           have been obtained by calling Importer::getImportDescriptors().
    *           The provided import descriptors will be owned by the agent.
    *  @param   pProgress
    *           The progress object to pass into the importer.  If \c NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \c true to execute the importer in batch mode
    *           or to \c false to execute the importer in interactive mode.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than once on
    *           this instance.
    */
   virtual void instantiate(const std::string& importerName,
      const std::map<std::string, std::vector<ImportDescriptor*> >& descriptors, Progress* pProgress, bool batch) = 0;

   /**
    *  Uses an existing importer to import individual data sets from one or more
    *  files.
    *
    *  @param   pPlugIn
    *           The importer to execute.  The agent assumes ownership of the
    *           importer and will destroy it as necessary upon agent
    *           destruction.
    *  @param   descriptors
    *           The files and data sets to import.  The descriptor values should
    *           have been obtained by calling Importer::getImportDescriptors().
    *           The provided import descriptors will be owned by the agent.
    *  @param   pProgress
    *           The progress object to pass into the importer.  If \c NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \c true to execute the importer in batch mode
    *           or to \c false to execute the importer in interactive mode.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than once on
    *           this instance.
    */
   virtual void instantiate(PlugIn* pPlugIn, const std::map<std::string, std::vector<ImportDescriptor*> >& descriptors,
      Progress* pProgress, bool batch) = 0;

   /**
    *  Sets the agent to import all data from a single file.
    *
    *  This method sets one file from which to import data.  %Any existing data
    *  descriptors are removed and deleted.  After calling this method, the next
    *  call to execute() will call Importer::getImportDescriptors() to determine
    *  the data sets to import.
    *
    *  @param   filename
    *           The name of the file to import.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @see     setFilenames(), setDatasets()
    */
   virtual void setFilename(const std::string& filename) = 0;

   /**
    *  Sets the agent to import data from multiple files.
    *
    *  This method sets multiple files from which to import data.  %Any existing
    *  data descriptors are removed and deleted.  After calling this method, the
    *  next call to execute() will call Importer::getImportDescriptors() to
    *  determine the data sets to import.
    *
    *  @param   filenames
    *           The names of the files to import.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @see     setFilename(), setDatasets()
    */
   virtual void setFilenames(const std::vector<std::string>& filenames) = 0;

   /**
    *  Filters importers displayed in the import dialog based on subtype.
    *
    *  If the agent executes in interactive mode and no filenames have been set,
    *  the import dialog is displayed for the user to choose the importer and
    *  one or more files to import.  This method filters the available importers
    *  in the dialog based on the given subtype.  Only importers that have a
    *  subtype equal to the given subtype will be displayed in the dialog.
    *
    *  By default, if this method is not called, all importers are displayed in
    *  the import dialog.
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
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @see     getImporterSubtype()
    */
   virtual void setImporterSubtype(const std::string& subtype) = 0;

   /**
    *  Returns the subtype of the importers displayed in the import dialog.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  Returns the subtype of the importers displayed in the import
    *           dialog.  Returns an empty string if all importers are displayed
    *           or if ApplicationServices::isInteractive() returns \c false.
    *
    *  @see     setImporterSubtype()
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
    *  The default state of the agent is to never allow the user to edit the
    *  data before import.
    *
    *  This method does nothing if ApplicationServices::isInteractive() returns
    *  \c false.
    *
    *  @param   editType
    *           The conditions in which the user can edit the import data
    *           parameters.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @see     getEditType()
    */
   virtual void setEditType(EditType editType) = 0;

   /**
    *  Returns the conditions in which the user can edit the import data
    *  parameters before importing the data.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  Returns the conditions in which the user can edit the import
    *           data parameters.  Returns EditType::NEVER_EDIT if
    *           ApplicationServices::isInteractive() returns \c false.
    *
    *  @see     setEditType()
    */
   virtual EditType getEditType() const = 0;

   /**
    *  Adds the imported files to the most recently used (MRU) file list.
    *
    *  This method allows for imported files to be added to the MRU file list
    *  so that users can quickly reimport the data at a later time.  Typically
    *  files should only be added to the MRU list if the user initiated the
    *  import.  Otherwise, it may confuse the user to see a file in the list
    *  that was not directly imported.
    *
    *  The filename that is added to the MRU file list is the filename that is
    *  passed into the instantiate(), setFilename(), setFilenames() or
    *  setDatasets() method.  If a filename is not set in the agent and the
    *  import dialog is displayed, the files that the user selects in the dialog
    *  are added to the MRU file list.  Otherwise, no files are added to the MRU
    *  list.
    *
    *  By default, if this method is not called, imported files are not added
    *  to the MRU file list.
    *
    *  @param   updateList
    *           Set this parameter to \c true to add imported files to the MRU
    *           file list.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    */
   virtual void updateMruFileList(bool updateList) = 0;

   /**
    *  Queries whether an imported file is added to the most recently used
    *  (MRU) file list.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  Returns \c true if an imported file is added to the MRU file
    *           list, otherwise returns \c false.
    *
    *  @see     updateMruFileList()
    */
   virtual bool isMruFileListUpdated() const = 0;

   /**
    *  Sets the files and data sets to import.
    *
    *  This method sets the files and their data sets that will be imported.
    *  The agent will take ownership over the provided import descriptors.
    *  After calling this method, the next call to execute() will import these
    *  data sets and will not call Importer::getImportDescriptors() to obtain
    *  the data sets to import.
    *
    *  @param   datasets
    *           The files and the import descriptors to import.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @see     setFilename(), setFilenames()
    */
   virtual void setDatasets(const std::map<std::string, std::vector<ImportDescriptor*> >& datasets) = 0;

   /**
    *  Returns the data sets for all files that will be used on import.
    *
    *  This method returns all data sets in all files specified in instantiate()
    *  or setDatasets().  If the import descriptors have not been set,
    *  Importer::getImportDescriptors() is called using the filenames set in
    *  instantiate(), setFilename(), or setFilenames().
    *
    *  Regardless of how they are obtained, all import descriptors stored and
    *  returned by this method are owned by the agent.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  The import descriptors that will be used on import.
    *
    *  @see     getImportDescriptors(const std::string&)
    */
   virtual std::vector<ImportDescriptor*> getImportDescriptors() = 0;

   /**
    *  Returns the data sets for a given file that will be used on import.
    *
    *  This method returns all data sets in the given file that have been
    *  specified in instantiate() or setDatasets().  If the import descriptors
    *  have not been set, Importer::getImportDescriptors() is called using the
    *  filenames set in instantiate(), setFilename(), or setFilenames().
    *
    *  Regardless of how they are obtained, all import descriptors stored and
    *  returned by this method are owned by the agent.
    *
    *  @param   filename
    *           The file for which to get its data sets to import.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  The import descriptors for the given file that will be used on
    *           import.  An empty vector is returned if the file has not
    *           previously been by calling instantiate(), setFilename(),
    *           setFilenames(), or setDatasets().
    *
    *  @see     getImportDescriptors(), getDatasets()
    */
   virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename) = 0;

   /**
    *  Returns the files and data sets that will be used on import.
    *
    *  This method returns the files and data sets specified in instantiate() or
    *  setDatasets().  If the import descriptors have not been set,
    *  Importer::getImportDescriptors() is called using the filenames set in
    *  instantiate(), setFilename(), or setFilenames().
    *
    *  Regardless of how they are obtained, all import descriptors stored and
    *  returned by this method are owned by the agent.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  The filenames and import descriptors that will be used on
    *           import.
    *
    *  @see     getImportDescriptors(), getImportDescriptors(const std::string&)
    */
   virtual std::map<std::string, std::vector<ImportDescriptor*> > getDatasets() = 0;

   /**
    *  Returns the default file extensions from the importer.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  The default extensions string returned by calling
    *           Importer::getDefaultExtensions().  An empty string is returned
    *           if no importer has been set.
    */
   virtual std::string getDefaultExtensions() const = 0;

   /**
    *  Executes the importer to load the data from the files.
    *
    *  This method creates the input and output arg lists if necessary,
    *  populates the input arg list, and then executes the importer.
    *
    *  If the import descriptors of the files to import have not been set,
    *  getImportDescriptors() is called.  A model element is created for each
    *  data set using the data descriptor contained in each import descriptor,
    *  and the ExecutableAgent::execute() base class method is called to execute
    *  the importer.  All successfully imported data elements can be retrieved
    *  by calling getImportedElements().
    *
    *  @par Arg Values:
    *  The following input arg values are automatically populated unless the
    *  actual value in the arg has already been set and PlugInArg::isActualSet()
    *  returns \c true:
    *  - All args documented in ExecutableAgent::execute().
    *  - Importer::ImportElementArg() is populated with the DataElement that was
    *    created for each data set based on each DataDescriptor contained in the
    *    ImportDescriptor returned from getImportDescriptors().
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  Returns \c true if at least one data set is imported
    *           successfully.  Returns \c false for any of the following
    *           conditions:
    *           - The options dialog is displayed and the user cancels it.
    *           - None of the data descriptors selected for import validate.
    *           - %Any data element could not be created.
    *           - The importer fails to import all data sets selected for
    *             import.
    *
    *  @see     setEditType()
    */
   virtual bool execute() = 0;

   /**
    *  Returns all successfully imported data elements.
    *
    *  The execute() method must be called prior to calling this method for
    *  the returned vector to be populated.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  A vector containing pointers to the data elements that were
    *           created as a result of calling execute().  An empty vector is
    *           returned if execute() has not previously been called or if the
    *           importer was not able to import any data.
    */
   virtual std::vector<DataElement*> getImportedElements() const = 0;

protected:
   /**
    *  Destroys the agent.
    *
    *  This object should not be destroyed directly.  It is most commonly
    *  destroyed by ImporterResource.
    */
   virtual ~ImportAgent()
   {}
};

#endif
