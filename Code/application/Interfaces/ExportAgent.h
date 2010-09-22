/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXPORT_AGENT_H
#define EXPORT_AGENT_H

#include "ExecutableAgent.h"
#include "TypesFile.h"

#include <string>

class Exporter;
class FileDescriptor;
class PlugInArgList;
class QWidget;
class SessionItem;

/**
 *  This is a helper class that makes working with Exporter
 *  plug-ins easier.  This class will manage the lifecycle of
 *  the Exporter plug-in. This class can be used
 *  to export a SessionItem.
 *
 *  You should not create an instance of this class using
 *  ObjectFactory, but you should use ExporterResource.
 *
 *  @warning If you do not call an overload of instantiate() before
 *  calling any other methods, a std::logic_error will be thrown.
 *  You cannot call instantiate() twice on the same instance
 *  or a std::logic_error will be thrown.
 *
 *  @see ExporterResource, Exporter
 */
class ExportAgent : public ExecutableAgent
{
public:

   /**
    *  Creates an invalid object for delayed initialization of an ExportAgent.
    *
    *  Creates an invalid object where no exporter is
    *  initially created.  The exporter can then be initialized later by calling
    *  setPlugIn().
    *
    *  @param   pProgress
    *           The progress object to pass into the exporter.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the exporter in batch mode
    *           or to \b false to execute the exporter in interactive mode.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(Progress* pProgress, bool batch) = 0;

   /**
    *  Creates an exporter to export data.
    *
    *  Creates the exporter but does not specify the data to
    *  export.  This method can be used if the exporter does not need a specific
    *  data value, or one will be provided later through setItem().
    *
    *  @param   exporterName
    *           The name of the exporter to create and execute.
    *  @param   pProgress
    *           The progress object to pass into the exporter.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the exporter in batch mode
    *           or to \b false to execute the exporter in interactive mode.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(std::string exporterName, Progress* pProgress, bool batch) = 0;

   /**
    *  Uses an existing exporter to export data.
    *
    *  @param   pPlugIn
    *           The exporter to execute.  The agent assumes ownership of the
    *           exporter and will destroy it as necessary upon agent
    *           destruction.
    *  @param   pProgress
    *           The progress object to pass into the exporter.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the exporter in batch mode
    *           or to \b false to execute the exporter in interactive mode.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(PlugIn* pPlugIn, Progress* pProgress, bool batch) = 0;

   /**
    *  Creates an exporter to export a view.
    *
    *  @param   exporterName
    *           The name of the exporter to create and execute.
    *  @param   pItem
    *           The item to pass into the exporter.
    *  @param   pFileDescriptor
    *           The file descriptor describing how the exporter should save
    *           the data to disk.
    *  @param   pProgress
    *           The progress object to pass into the exporter.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the exporter in batch mode
    *           or to \b false to execute the exporter in interactive mode.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(std::string exporterName, SessionItem *pItem, FileDescriptor* pFileDescriptor,
      Progress* pProgress, bool batch) = 0;

   /**
    *  Uses an existing exporter to export a session item.
    *
    *  @param   pPlugIn
    *           The exporter to execute.  The agent assumes ownership of the
    *           exporter and will destroy it as necessary upon agent
    *           destruction.
    *  @param   pItem
    *           The item to pass into the exporter.
    *  @param   pFileDescriptor
    *           The file descriptor describing how the exporter should save
    *           the data to disk.
    *  @param   pProgress
    *           The progress object to pass into the exporter.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the exporter in batch mode
    *           or to \b false to execute the exporter in interactive mode.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(PlugIn* pPlugIn, SessionItem *pItem, FileDescriptor* pFileDescriptor,
      Progress* pProgress, bool batch) = 0;

   /**
    *  Gets a pointer to the underlying exporter plug-in.
    *
    *  This differs from getPlugIn() in that it performs a dynamic_cast
    *  to type Exporter. This is a convenience function and
    *  is equivalent to dynamic_cast<Exporter*>(getPlugIn())
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return   A pointer to the underlying exporter plug-in.
    */
   virtual Exporter *getExporter() = 0;

   /**
    *  Gets a pointer to the underlying exporter plug-in.
    *
    *  This differs from getPlugIn() in that it performs a dynamic_cast
    *  to type Exporter. This is a convenience function and
    *  is equivalent to dynamic_cast<Exporter*>(getPlugIn())
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return   A pointer to the underlying exporter plug-in.
    */
   virtual const Exporter *getExporter() const = 0;

   /**
    *  Gets the exporter plug-in's custom options page.
    *
    *  This gets a custom option widget using the existing
    *  agent's plug-in arguments.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return  A custom option widget or NULL if the exporter
    *           does not have a custom option widget.
    */
   virtual QWidget *getExportOptionsWidget() = 0;

   /**
    *  Sets the session item to pass into the exporter.
    *
    *  @param  pItem
    *          The session item to pass into the exporter.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    */
   virtual void setItem(SessionItem* pItem) = 0;

   /**
    *  Returns the session item that will be passed into the exporter when it
    *  is executed.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return  The session item that is passed into the exporter.  This is
    *           either the session item passed in upon instantiation of the agent
    *           or the session item obtained by calling setItem().
    */
   virtual SessionItem* getItem() const = 0;

   /**
    *  Sets the file descriptor that the exporter uses to save the file.
    *
    *  @param   pFileDescriptor
    *           The file descriptor to pass into the exporter.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    */
   virtual void setFileDescriptor(FileDescriptor* pFileDescriptor) = 0;

   /**
    *  Returns the file descriptor that will be passed into the exporter when
    *  it is executed.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return  The file descriptor that is passed into the exporter.  This is
    *           either the file descriptor passed in upon instantiation of the
    *           agent or the file descriptor obtained by calling
    *           setFileDescriptor().
    */
   virtual FileDescriptor* getFileDescriptor() = 0;

   /**
    *  Returns the file descriptor that will be passed into the exporter when
    *  it is executed.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return  The file descriptor that is passed into the exporter.  This is
    *           either the file descriptor passed in upon instantiation of the
    *           agent or the file descriptor obtained by calling
    *           setFileDescriptor().
    */
   virtual const FileDescriptor* getFileDescriptor() const = 0;

   /**
    *  Gets the default file extensions from the exporter.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return  The default extensions string returned by calling
    *           Exporter::getDefaultExtensions().
    */
   virtual std::string getDefaultExtensions() const = 0;

   /**
    *  Validate the input argument list.
    *
    *  This validates the input argument list as it will be passed to Executable::execute().
    *  If the plug-in is interactive, information and choices may be presented to the user
    *  to decide how to proceed. If the plug-in is in batch mode, information may be logged
    *  to the message log but no user interaction is possible.
    *
    *  @param errorMessage
    *         This is the error or informational message to display to the user.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return Validation result indicating if the exporter can save the data
    *
    *  @see ValidationResultType
    */
   virtual ValidationResultType validate(std::string &errorMessage) = 0;

   /**
    *  Executes the exporter to save the data.
    *
    *  This method first creates the input and output arg lists if necessary,
    *  populates the input arg list, and then executes the exporter.
    *
    *  @par Arg Values:
    *  The following input arg values are automatically populated unless the
    *  actual value in the arg has already been set and
    *  PlugInArg::isActualSet() returns \c true:
    *  - All args documented in ExecutableAgent::execute().
    *  - Exporter::ExportItemArg() is populated with the SessionItem that will
    *    be exported as returned by getItem().
    *  - Exporter::ExportDescriptorArg() is populated with the FileDescriptor
    *    to use for export as returned by getFileDescriptor().
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return  The success value returned by the exporter.
    */
   virtual bool execute() = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~ExportAgent() {}
};

#endif
