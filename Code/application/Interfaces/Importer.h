/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORTER_H
#define IMPORTER_H

#include "TypesFile.h"

#include <string>
#include <vector>

class DataDescriptor;
class ImportDescriptor;
class Progress;
class QWidget;

/**
 *  Interface specific to importer plug-ins.
 *
 *  Defines the importer specific interface to all importer plug-ins.
 *  This interface contains all importer specific operations.
 */
class Importer
{
public:
   /**
    *  The name for an import element argument.
    *
    *  Input arguments with this name will be automatically populated with the
    *  created DataElement whose data should be populated by the importer when
    *  the plug-in is executed with an ImportAgent.  Arguments with this name
    *  should be of the type DataElement or one of its subclasses.
    *
    *  @see     ImportAgent::execute()
    */
   static std::string ImportElementArg() { return "Import Element"; }

   /**
    *  Returns the default file extensions recognized by the importer.
    *
    *  @return  The file extensions recognized by the importer as a string.
    *           The string consists of a description followed by one or more
    *           file extensions separated by a space.  Multiple file
    *           types may be specified with a double semicolon.  Examples
    *           include "ENVI Header Files (*.hdr)", "TIFF Files (*.tif *.tiff)",
    *           and "Source Files (*.c*);;Header Files (*.h)".
    */
   virtual std::string getDefaultExtensions() const = 0;

   /**
    *  Queries whether the user can select the given processing location.
    *
    *  @param   location
    *           The processing location being queried to determine if it
    *           is supported.
    *
    *  @return  Returns \b true if the user can select the given processing
    *           location, otherwise returns \b false.
    *
    *  @see     ProcessingLocation
    */
   virtual bool isProcessingLocationSupported(ProcessingLocation location) const = 0;

   /**
    *  Returns import descriptors for each valid data element in a given file.
    *
    *  This method is called for the importer to parse a given file and create
    *  valid import descriptors for each data element in the file that can be
    *  used to import the data.  The import descriptor objects can be created
    *  by calling ModelServices::createImportDescriptor().
    *
    *  @param   filename
    *           Full path and name of the file to parse for data elements to
    *           import.
    *
    *  @return  A vector containing valid import descriptors for each data
    *           element in the file.  An empty vector should be returned if the
    *           importer does not support the data in the given file.
    *
    *  @see     ImportDescriptor
    */
   virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename) = 0;

   /**
    *  Returns a value indicating if this importer can load this file and how well
    *  it can load this file.  The Importer should look at as little of the file
    *  as required as quickly as possible when determining the affinity.
    *
    *  @param filename
    *         Full path and name of the file.
    *
    *  @return The value returned should be one of the following:
    *          <ul>
    *            <li>Importer::CAN_NOT_LOAD</li>
    *            <li>Importer::CAN_LOAD_WITH_USER_INPUT</li>
    *            <li>Importer::CAN_LOAD_FILE_TYPE</li>
    *            <li>Importer::CAN_LOAD</li>
    *          </ul>
    *          It can be any unsigned char value if a specific
    *          Importer requires more granularity.
    */
   virtual unsigned char getFileAffinity(const std::string& filename) = 0;

   /**
    * When an importer returns this value from getFileAffinity()
    * it means the importer can not load any data from the given file.
    */
   static const unsigned char CAN_NOT_LOAD = 0;

   /**
    * When an importer returns this value from getFileAffinity()
    * it means the importer may be able to load data from the file if 
    * provided additional user input.  The given Importer should return
    * a non-empty vector of a single default constructed Data Descriptor from
    * getImportDescriptors() if given the same filename.  For example,
    * the "Generic Importer" will load raw data from any file as long as
    * it formatted BIP, BSQ, or BIL, but will still require additional
    * information from the user.
    */
   static const unsigned char CAN_LOAD_WITH_USER_INPUT = 64;

   /**
    * When an importer returns this value from getFileAffinity()
    * it means the importer may be able to load data from the file if 
    * provided additional user input.  The given Importer should return
    * a non-empty vector of DataDescriptors that are partially completed
    * from getImportDescriptors() if given the same filename that
    * will require additional user input.  For example, the "Generic Hdf5
    * Importer" will locate all data within a Hdf5 file it could possibly
    * load and return DataDescriptors for each, but will still require
    * additional information from the user.
    */
   static const unsigned char CAN_LOAD_FILE_TYPE = 128;

   /**
    * When an importer returns this value from getFileAffinity()
    * it means the importer can load data from the given file and the Importer
    * will return a non-empty vector from getImportDescriptors() if
    * given the same filename.
    *
    * An importer can return a value greater than Importer::CAN_LOAD
    * if it wishes to override a particular importer.  For example,
    * a specialized NITF importer may know details of how to load
    * NITF files from a particular source.
    */
   static const unsigned char CAN_LOAD = 192;

   /**
    *  Returns a preview of a given data set.
    *
    *  This method provides the means by which the user can preview a data set
    *  before importing.  Derived importers can override this method to create
    *  a Qt widget displaying preview contents.
    *
    *  This method is called by the core when the user views a preview of a
    *  data set before importing.  The core application assumes ownership of
    *  the Qt widget, so the importer should not delete it.  A View object can
    *  also be created for the preview, where the returned value should be
    *  View::getWidget().
    *
    *  @param   pDescriptor
    *           The data set to preview.
    *  @param   pProgress
    *           A progress object in which the importer can report progress
    *           while getting the preview.
    *
    *  @return  A pointer to a Qt widget containing the preview contents that
    *           will be displayed to the user.  The core application assumes
    *           ownership of the widget and will delete it when necessary.
    *           \b NULL is returned if no preview is available for the given
    *           data set.
    */
   virtual QWidget* getPreview(const DataDescriptor* pDescriptor, Progress* pProgress) = 0;

   /**
    *  Queries whether a given data descriptor can be successfully loaded by
    *  the importer.
    *
    *  This method is called for the importer to parse the current settings in
    *  the data descriptor to see if it supports loading the data as currently
    *  specified in the data descriptor.  This allows importers that do not
    *  support certain combinations of values to indicate as such.  This method
    *  is called each time the user changes a value in the import options
    *  dialog.  This method is also called by ImporterAgent::execute()
    *  before executing the importer.  If a non-empty errorMessage is returned
    *  and this method returns true, the message will be added to the
    *  Importers Progress object as a warning.
    *
    *  @param   pDescriptor
    *           The data descriptor to query if it can be successfully
    *           imported.
    *  @param   errorMessage
    *           An error message that is populated with the reason why the
    *           importer cannot load the given data descriptor.  This message
    *           will be displayed to the user.  If this method
    *           returns true, this message will be displayed to the user
    *           as a warning.  If this method returns false, this message
    *           will be displayed to the user as an error.
    *
    *  @return  Returns <b>true</b> if the importer can successfully import
    *           the given data descriptor; otherwise returns <b>false</b>.
    *
    *  @see     DataDescriptor, FileDescriptor
    */
   virtual bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const = 0;

   /**
    *  Returns a widget to display custom import option values.
    *
    *  This method provides an interface for which specialized import options
    *  for a data set can be displayed to the user.  The method returns a Qt
    *  widget that is added to the default import options dialog.  The importer
    *  should create the widget with a NULL parent, and should destroy the
    *  widget when the importer itself is destroyed.
    *
    *  Importers should call QWidget::setWindowTitle() on the widget that is
    *  returned to set the name that appears on the tab in the import options
    *  dialog.  If the window title is not set, the importer name is displayed.
    *
    *  @param   pDescriptor
    *           The data set for which to set the current values in the widget.
    *
    *  @return  A QWidget that will be displayed as an additional tab in the
    *           default import options dialog.  <b>NULL</b> should be returned
    *           if the importer does not have custom options to display to the
    *           user.
    */
   virtual QWidget* getImportOptionsWidget(DataDescriptor* pDescriptor) = 0;

   /**
    *  Modifies a data descriptor before it is imported.
    *
    *  This method is called after the user has made changes to the data
    *  descriptor contained in an ImportDescriptor returned in
    *  getImportDescriptors() and before the element is created for import.
    *
    *  @warning It is generally not recommended for an importer to modify
    *           values that the user has the capability to change in the import
    *           options dialog since any value that the user sets would be
    *           overridden without the user's knowledge.
    *
    *  @param   pDescriptor
    *           The data set to modify before import.
    */
   virtual void polishDataDescriptor(DataDescriptor* pDescriptor) = 0;

protected:
   /**
    *  Since the Importer interface is usually used in conjunction with the
    *  PlugIn and Executable interfaces, this should be destroyed by casting to
    *  the PlugIn interface and calling PlugInManagerServices::destroyPlugIn().
    */
   virtual ~Importer() {}
};

#endif
