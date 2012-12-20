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

#include <string>
#include <vector>

class DataDescriptor;
class GcpLayer;
class GcpList;
class LatLonLayer;
class PlugIn;
class Progress;
class RasterElement;
class RasterLayer;
class SpatialDataView;
class Step;

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
    *           the test conditions in getValidationTest().  If the base class
    *           implementation validates successfully, the gray, red, green, and
    *           blue display bands in the RasterDataDescriptor are checked
    *           against the loaded bands.  If the display band will not be
    *           loaded, a warning is added to \em errorMessage and the method
    *           returns \c true.
    */
   virtual bool validate(const DataDescriptor* pDescriptor,
      const std::vector<const DataDescriptor*>& importedDescriptors, std::string& errorMessage) const;

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
    *           does not depend on successfully extracting the progress value since
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
    *  - \link ImporterShell::VALID_GEOREFERENCE_PARAMETERS VALID_GEOREFERENCE_PARAMETERS \endlink
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
    *  Imports the raster data.
    *
    *  This method imports the raster data by creating a RasterPager if the
    *  processing location is \link ProcessingLocation::ON_DISK_READ_ONLY
    *  ON_DISK_READ_ONLY\endlink or by creating a separate RasterElement and
    *  RasterPager and copying the data into the original RasterElement.
    *
    *  The parseInputArgList() method must be called before calling this method.
    *  This method is called from the default implementation of execute().
    *
    *  @return  Returns \c true if the import succeeded, or \c false if the
    *           import fails or is aborted.  If \c false is returned, the
    *           Progress object returned by getProgress() will have an
    *           appropriate message.
    */
   virtual bool performImport() const;

   /**
    *  Georeferences the raster data.
    *
    *  This method georeferences the raster data by executing the Georeference
    *  plug-in returned from GeoreferenceDescriptor::getGeoreferencePlugInName()
    *  if GeoreferenceDescriptor::getGeoreferenceOnImport() returns \c true.  If
    *  the Georeference plug-in fails, the failure message is added as a warning
    *  to the Progress object returned by getProgress().
    *
    *  This method is called from the default implementation of execute() after
    *  createGcpList() is called.
    */
   virtual void performGeoreference() const;

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
    *  Creates a raster layer in the given view.
    *
    *  This method is called from the default implementation of createView()
    *  after the view is created.
    *
    *  @param   pView
    *           The view in which to create the raster layer.
    *  @param   pStep
    *           The message log step for creating the view.
    *
    *  @return  A pointer to the created raster layer.
    *
    *  @default The default implementation of this method calls
    *           SpatialDataView::createLayer() with the raster element
    *           extracted from the input arg list as the element to display in
    *           the layer.
    *
    *  @see     getRasterElement()
    */
   virtual RasterLayer* createRasterLayer(SpatialDataView* pView, Step* pStep) const;

   /**
    *  Creates a GCP layer in the given view.
    *
    *  This method is called from the default implementation of createView()
    *  after the raster layer is created.
    *
    *  @param   pView
    *           The view in which to create the GCP layer.
    *  @param   pStep
    *           The message log step for creating the view.
    *
    *  @return  A pointer to the created GCP layer.
    *
    *  @default The default implementation of this method calls
    *           SpatialDataView::createLayer() with the GCP list returned from
    *           createGcpList() as the element to display in the layer.  If
    *           createGcpList() returns \c NULL, the default implementation
    *           does nothing.
    *
    *  @see     createRasterLayer()
    */
   virtual GcpLayer* createGcpLayer(SpatialDataView* pView, Step* pStep) const;

   /**
    *  Creates a latitude/longitude layer in the given view.
    *
    *  This method is called from the default implementation of createView()
    *  after the GCP layer is created.  This method is not called if the
    *  \link  GeoreferenceDescriptor::getSettingCreateLayer()
    *  GeoreferenceDescriptor::CreateLayer\endlink setting is \c false.
    *
    *  @param   pView
    *           The view in which to create the latitude/longitude layer.
    *  @param   pStep
    *           The message log step for creating the view.
    *
    *  @return  A pointer to the created latitude/longitude layer.
    *
    *  @default The default implementation of this method executes the
    *           "Georeference" plug-in to create the latitude/longitude layer
    *           based on the previously georeferenced raster element.  If the
    *           layer is successfully created, the layer is shown or hidden
    *           based on the value of the
    *           \link GeoreferenceDescriptor::getSettingDisplayLayer()
    *           GeoreferenceDescriptor::DisplayLayer\endlink setting.  If the
    *           raster element was not successfully georeferenced, the default
    *           implementation does nothing.
    *
    *  @see     createGcpLayer()
    */
   virtual LatLonLayer* createLatLonLayer(SpatialDataView* pView, Step* pStep) const;

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
    *  Returns the GcpList object returned from createGcpList().
    *
    *  @return  A pointer to the GcpList object created as a result of the call
    *           to createGcpList().  \c NULL is returned if the createGcpList()
    *           method has not yet been called.
    */
   GcpList* getGcpList() const;

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
   GcpList* mpGcpList;
};

#endif
