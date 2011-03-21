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

#include "ConfigurationSettings.h"
#include "DesktopServices.h"
#include "ImporterShell.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"
#include "TypesFile.h"
#include "UtilityServices.h"

#include <string>
#include <vector>

class DataDescriptor;
class GcpList;
class PlugIn;
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
   virtual ~RasterElementImporterShell();

   SETTING(AutoGeoreference, RasterElementImporter, bool, true)
   SETTING(ImporterGeoreferencePlugIn, RasterElementImporter, bool, true)
   SETTING(GeoreferencePlugIns, RasterElementImporter, std::vector<std::string>, std::vector<std::string>())
   SETTING(DisplayLatLonLayer, RasterElementImporter, bool, false)

   /**
    *  @copydoc ImporterShell::getInputSpecification()
    *
    *  @default The default implementation adds a Progress arg and a
    *           RasterElement arg to the arg list.
    */
   virtual bool getInputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc ImporterShell::getOutputSpecification()
    *
    *  @default The default implementation adds a SpatialDataView arg to the
    *           arg list if the plug-in is in interactive mode and does nothing
    *           if the plug-in is in batch mode.
    */
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);

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
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   /**
    *  @copydoc Importer::validate()
    *
    *  @default The default implementation calls the base class implementation
    *           and provides better error messages where appropriate based on
    *           the test conditions in getValidationTest().
    */
   virtual bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

   /**
    *  @copydoc Importer::isProcessingLocationSupported()
    *
    *  @default All processing locations are supported, so the default
    *           implementation returns \b true for any valid ProcessingLocation
    *           value.
    */
   virtual bool isProcessingLocationSupported(ProcessingLocation location) const;

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
   virtual QWidget* getPreview(const DataDescriptor* pDescriptor, Progress* pProgress);

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
    *  @copydoc ImporterShell::getValidationTest()
    *
    *  \par
    *  The following raster-specific tests are added to the base class defaults
    *  listed above:
    *  - \link ImporterShell::VALID_CLASSIFICATION VALID_CLASSIFICATION \endlink
    *  - \link ImporterShell::RASTER_SIZE RASTER_SIZE \endlink
    *  - \link ImporterShell::VALID_DATA_TYPE VALID_DATA_TYPE \endlink
    *  .
    *  \par
    *  Additionally, the following test is added if the ::ProcessingLocation is
    *  ::IN_MEMORY:
    *  - \link ImporterShell::AVAILABLE_MEMORY AVAILABLE_MEMORY \endlink
    *  .
    *  \par
    *  The following tests are added if the ::ProcessingLocation is
    *  ::ON_DISK_READ_ONLY:
    *  - \link ImporterShell::NO_INTERLEAVE_CONVERSIONS NO_INTERLEAVE_CONVERSIONS \endlink
    *  - \link ImporterShell::NO_BAND_SUBSETS NO_BAND_SUBSETS \endlink
    *  - \link ImporterShell::NO_SKIP_FACTORS NO_SKIP_FACTORS \endlink
    *  .
    *  \par
    *  The following tests are added if the ::InterleaveFormatType is not ::BSQ:
    *  - \link ImporterShell::NO_PRE_POST_BAND_BYTES NO_PRE_POST_BAND_BYTES \endlink
    *  - \link ImporterShell::NO_BAND_FILES NO_BAND_FILES \endlink
    *  .
    *  \par
    *  The following test is added if multiple band files are present:
    *  - \link ImporterShell::EXISTING_BAND_FILES EXISTING_BAND_FILES \endlink
    *  .
    */
   virtual int getValidationTest(const DataDescriptor* pDescriptor) const;

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
    *  extract the RasterElement.  This method is called from the default implementation
    *  of execute() after the createGcpList() method.
    *
    *  A new message log step is created and the initially displayed bands and display
    *  mode are added as properties to the step.
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
    *  default implementation of execute() after the performImport() method.
    *
    *  @return  A pointer to the created GCP list.  \c NULL is returned if the
    *           raster element has not been extracted from the input arg list.
    *
    *  @see     getRasterElement()
    */
   virtual GcpList* createGcpList() const;

   /**
    *  Creates a Georeference plug-in that can be used to georeference the
    *  raster data.
    *
    *  This method is called from within execute() if the configuration settings
    *  to auto-georeference and to get the georeference plug-in from the
    *  importer are enabled.  The method is called after calling createGcpList(),
    *  regardless of whether a valid GCP list is returned.
    *
    *  The default implementation of this method creates an instance of the
    *  GCP %Georeference plug-in and calls Georeference::canHandleRasterElement()
    *  to see if it can georeference the raster data.  If the plug-in supports
    *  the raster data, it is returned.  Otherwise the plug-in is destroyed and
    *  \c NULL is returned.
    *
    *  Derived importers should override this method if a better Georeference
    *  plug-in is available for the raster data.
    *
    *  @return  A pointer to the Georeference plug-in that will be used to
    *           georeference the raster data.  The plug-in should support the
    *           raster element returned by getRasterElement() such that
    *           Georeference::canHandleRasterElement() returns \c true.
    *           Ownership of the plug-in is transferred to the shell and will
    *           be destroyed automatically.  \c NULL is returned if no plug-in
    *           is available to georeference the raster data, or if the raster
    *           data does not contain georeference information.
    *
    *  @see     getRasterElement(), getSettingAutoGeoreference(),
    *           getSettingImporterGeoreferencePlugIn()
    */
   virtual PlugIn* getGeoreferencePlugIn() const;

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
};

#endif
