/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERELEMENTIMPORTERSHELL_H
#define RASTERELEMENTIMPORTERSHELL_H

#include "DesktopServices.h"
#include "ImporterShell.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"
#include "TypesFile.h"
#include "UtilityServices.h"

class DataDescriptor;
class GcpList;
class Progress;
class RasterElement;
class SpatialDataView;

/**
 *  \ingroup ShellModule
 *  Provides a shell class for importers loading RasterElement objects.
 *
 *  This class provides capabilities for importers to load data from a file
 *  which is stored in a RasterElement object.  The default input arg list contains
 *  two objects: Progress and RasterElement.  The default output arg list contains a
 *  single View object in interactive mode.
 *
 *  On a typical import from interactive mode, derived importers must override the
 *  getImportDescriptors() method to parse a file for the valid data set parameters.
 *  The user may then modify the fields in the DataDescriptor, and a RasterElement object
 *  is created from the modified DataDescriptor.  This RasterElement object is then set
 *  into the input arg list before execute() is called.
 *
 *  The default implementation of the execute() method calls createRasterPager()
 *  and copies the selected data into the import RasterElement.  A view is 
 *  automatically created if the plug-in is in interactive mode.  If the 
 *  DataDescriptor input arg is \c NULL, one is created and the getImportDescriptors()
 *  method is called to set the values. The plug-in contains aborting capabilities
 *  which aborts the copy in progress.
 *
 *  Importers that have a special format to load data from the file only need to override
 *  createRasterPager() to create the format-specific RasterPager.  Importers that
 *  desire more control can do so by overriding execute().  The parseInputArgList() method
 *  can be called to extract the Progress and RasterElement values from the arg list, and the
 *  createView() method can be called to create a view for the RasterElement in interactive
 *  mode.  A derived importer can also override abort() to properly allow the user to
 *  abort the import.
 *
 *  @see     ImporterShell
 */
class RasterElementImporterShell : public ImporterShell
{
public:
   /**
    *  Creates a raster element importer plug-in.
    *
    *  The constructor sets the plug-in subtype to "Raster Element" and sets
    *  the plug-in to allow multiple instances.
    *
    *  @see     getSubtype(), areMultipleInstancesAllowed()
    */
   RasterElementImporterShell();

   /**
    *  Destroys the raster element importer plug-in.
    */
   ~RasterElementImporterShell();

   /**
    *  @copydoc ImporterShell::getInputSpecification()
    *
    *  @default The default implementation adds a Progress arg and a
    *           RasterElement arg to the arg list.
    */
   bool getInputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc ImporterShell::getOutputSpecification()
    *
    *  @default The default implementation adds a SpatialDataView arg to the
    *           arg list if the plug-in is in interactive mode and does nothing
    *           if the plug-in is in batch mode.
    */
   bool getOutputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc ImporterShell::execute()
    *
    *  @default The default implementation imports the RasterElement in the
    *           input arg list by creating a RasterPager if the processing
    *           location is ProcessingLocation::ON_DISK_READ_ONLY or by
    *           creating a separate RasterElement and RasterPager and copying
    *           the data into the original RasterElement in the input arg list.
    *           A RasterElement arg needs to be present in the input arg list
    *           for the method to complete successfully.
    */
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   /**
    *  @copydoc Importer::validate()
    *
    *  @default The default implementation first calls validateBasic() and then
    *           validateDefaults().
    *
    *  @warning Importers which override createRasterPager() and not execute()
    *           should not override this method.  Override
    *           validateDefaultOnDiskReadOnly() instead to indicate which
    *           formats your RasterPager can accept.
    */
   bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

   /**
    *  @copydoc Importer::isProcessingLocationSupported()
    *
    *  @default All processing locations are supported, so the default
    *           implementation returns \b true for any valid ProcessingLocation
    *           value.
    */
   bool isProcessingLocationSupported(ProcessingLocation location) const;

   /**
    *  @copydoc Importer::getPreview()
    *
    *  @default The default implementation creates and returns a preview of the
    *           given data set.  A SpatialDataView is created in grayscale mode
    *           using only the data in the first band of the data descriptor.
    *           The data is loaded on-disk read-only to avoid unnecessary
    *           memory allocation.  However, if the data cannot be loaded
    *           on-disk as determined with a call to validate(), it will be
    *           loaded into memory to create the preview.
    */
   QWidget* getPreview(const DataDescriptor* pDescriptor, Progress* pProgress);

protected:
   /**
    *  Extracts the progress and sensor data from the given input arg list.
    *
    *  This method extracts the progress and sensor data values from the input
    *  args in the given arg list.  The values are stored and can be retrieved by
    *  calling the getProgress() and getRasterElement() methods.
    *
    *  @param   pInArgList
    *           The input arg list for which to extract the progress and raster
    *           element values.
    *
    *  @return  Returns <b>true</b> if the sensor data value was successfully
    *           extracted from the arg list.  <b>false</b> is returned if the raster
    *           element value could not be extracted from the arg list.  The return value
    *           does not depend on successfully extractinf the progress value since
    *           the Progress object is not required to successfully load the element.
    *
    *  @see     getProgress(), getRasterElement()
    */
   virtual bool parseInputArgList(PlugInArgList* pInArgList);

   /**
    *  Returns the Progress object extracted from the input arg list.
    *
    *  @return  A pointer to the Progress object extracted from the input arg list.
    *           <b>NULL</b> is returned if the parseInputArgList() method has not
    *           been called or if the parsed input arg list does not contain a valid
    *           Progress arg value.
    *
    *  @see     parseInputArgList()
    */
   Progress* getProgress() const;

   /**
    *  Returns the RasterElement object extracted from the input arg list.
    *
    *  @return  A pointer to the RasterElement object extracted from the input arg list.
    *           <b>NULL</b> is returned if the parseInputArgList() method has not
    *           been called or if the parsed input arg list does not contain a valid
    *           RasterElement arg value.
    *
    *  @see     parseInputArgList()
    */
   RasterElement* getRasterElement() const;

   /**
    *  Determines if the default RasterElementImporterShell::execute()
    *  can import with this DataDescriptor.
    *  
    *  This method calls validateDefaultOnDiskReadOnly() for 
    *  ProcessingLocation::ON_DISK_READ_ONLY.
    *
    *  @param   pDescriptor
    *           The data descriptor to query if it can be successfully
    *           imported.
    *  @param   errorMessage
    *           An error message that is populated with the reason why Generic Importer
    *           cannot load the given data descriptor.
    *
    *  @return  Returns <b>true</b> if the default execute can successfully import
    *           the given data descriptor; otherwise returns <b>false</b>.
    */
   bool validateDefaults(const DataDescriptor* pDescriptor, std::string& errorMessage) const;
   
   /**
    *  Determines if this importer can import with this DataDescriptor
    *  with a ProcessingLocation::ON_DISK_READ_ONLY.
    *
    *  Importers which do not replace execute() should override this method to
    *  validate for their RasterPager.  validateDefaults() will call this
    *  method when appropriate to allow validation to fail for the particular
    *  RasterPager restrictions.
    *
    *  @param   pDescriptor
    *           The data descriptor to query if it can be successfully
    *           imported.
    *  @param   errorMessage
    *           An error message that is populated with the reason why Generic Importer
    *           cannot load the given data descriptor.
    *
    *  @return  Returns <b>true</b> if the RasterPager can successfully import
    *           the given data descriptor on-disk read-only; otherwise returns <b>false</b>.
    */
   virtual bool validateDefaultOnDiskReadOnly(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

   /**
    *  This method validates the DataDescriptor for basic problems.
    *
    *  This implementation checks the following criteria in the
    *  specified order:
    *  - All checks in ImporterShell::validate()
    *  - Non-NULL data descriptor
    *  - Non-NULL file descriptor
    *  - Non-zero number of rows, columns, and bands
    *  - Non-zero number of bits per pixel
    *  - No preband or postband bytes for non-BSQ data
    *    (ProcessingLocation::IN_MEMORY only)
    *  - If band files are specified, enough band files for the number of
    *    bands in the file descriptor
    *  - Each of the band filenames of the bands to import is not empty
    *  - Each of the band files of the bands to import exist on disk
    *  - No multiple band files with non-BSQ data
    *  - The amount of required memory can be successfully allocated
    *    (ProcessingLocation::IN_MEMORY only)
    *
    *  <b>Warning:</b> This method should not be called from within execute()
    *  because the memory allocation check should not be performed because any
    *  memory for the data element will have already been allocated, and the
    *  check for a non-existing data element in ImporterShell::validate() will
    *  fail since the data element has already been created.
    *
    *  @param   pDescriptor
    *           The data descriptor to query if it can be successfully
    *           imported.
    *  @param   errorMessage
    *           An error message that is populated with the reason why the
    *           importer cannot load the given data descriptor.  This message
    *           will be displayed to the user.
    *
    *  @return  Returns \c true if the basic checks succeed; otherwise returns
    *           \c false.
    *
    *  @see     DataDescriptor, FileDescriptor
    */
   bool validateBasic(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

   /**
    * Perform the a default import.
    *
    * This method performs a default import by creating a RasterPager if the processing
    * location is ProcessingLocation::ON_DISK_READ_ONLY or by creating a separate 
    * RasterElement and RasterPager and copying the data into the original RasterElement.
    *
    * parseInputArgList() must be called before calling this method.  This method is 
    * called from the default implementation of execute().
    *
    * @return \c true if the operation succeeded, or \c false otherwise.  Failure
    *         may be due to an abort.  In case of failure, the Progress will have an
    *         appropriate message.
    */
   virtual bool performImport() const;

   /**
    *  Creates a view for the imported data set.
    *
    *  This method creates a view for the imported data set in interactive mode only.
    *  Prior to calling this method, the parseInputArgList() method must be called to
    *  extract the RasterElement.  This method is called from the default implemetation
    *  of execute() after the data has been loaded from the file.
    *
    *  A new message log step is created and the initially displayed bands and display
    *  mode are added as properies to the step.
    *
    *  @return  A pointer to the created view.  <b>NULL</b> is returned if the plug-in
    *           is in batch mode or the raster element has not been extracted from the
    *           input arg list.
    */
   virtual SpatialDataView* createView() const;

   /**
    *  Creates a GcpList element for the GCPs contained in the raster element
    *  file descriptor.
    *
    *  Prior to calling this method, the parseInputArgList() method must be
    *  called to extract the raster element.  This method is called from the
    *  default implemetation of createView() after the raster layer has been
    *  created for the raster element.
    *
    *  @return  A pointer to the created GCP list.  \c NULL is returned if the
    *           raster element has not been extracted from the input arg list.
    *
    *  @see     getRasterElement()
    */
   virtual GcpList* createGcpList() const;

   /**
    *  Create and set a RasterPager for a given RasterElement.
    *
    *  Most importers should override this method instead of execute().
    *  Overriding this method will allow the importer to get all
    *  of the copy and conversion capabilities provided by the default execute().
    *
    *  @param pRaster
    *         The RasterElement to create the RasterPager for.
    *
    *  @return True if the pager was successfully created, false otherwise.
    */
   virtual bool createRasterPager(RasterElement* pRaster) const;

   /**
    *  Copy data from the source element to the imported one.
    *
    *  @param pSrcElement
    *         The source element to copy from.  The active rows, columns,
    *         and bands should be a superset of those being imported.
    *
    *  @return True if the copy was successful, false otherwise.
    */
   bool copyData(const RasterElement* pSrcElement) const;

   Service<DesktopServices> mpDesktop;
   Service<ModelServices> mpModel;
   Service<PlugInManagerServices> mpPlugInManager;
   Service<UtilityServices> mpUtilities;

private:
   bool checkAbortOrError(std::string message, Step* pStep, bool checkForError = true) const;

   mutable bool mUsingMemoryMappedPager;
   Progress* mpProgress;
   RasterElement* mpRasterElement;
   bool mIsSessionLoad;
};

#endif
