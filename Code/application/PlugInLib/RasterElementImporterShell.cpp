/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVerify.h"
#include "DimensionDescriptor.h"
#include "FileResource.h"
#include "GcpLayer.h"
#include "GcpList.h"
#include "Georeference.h"
#include "LatLonLayer.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterElementImporterShell.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SessionManager.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "Undo.h"

#include <limits>
using namespace std;

namespace
{
   vector<DimensionDescriptor> getSelectedDims(const vector<DimensionDescriptor>& srcDims,
      const vector<DimensionDescriptor>& chipDims)
   {
      vector<DimensionDescriptor> selectedDims;

      for (vector<DimensionDescriptor>::const_iterator srcIter = srcDims.begin(), chipIter = chipDims.begin();
         srcIter != srcDims.end() && chipIter != chipDims.end(); ++srcIter)
      {
         if (srcIter->getOriginalNumber() == chipIter->getOriginalNumber())
         {
            selectedDims.push_back(*srcIter);
            ++chipIter;
         }
      }

      return selectedDims;
   }
}

RasterElementImporterShell::RasterElementImporterShell() :
   mUsingMemoryMappedPager(false),
   mpProgress(NULL),
   mpRasterElement(NULL),
   mpGcpList(NULL)
{
   setSubtype("Raster Element");
   allowMultipleInstances(true);
   setAbortSupported(true);
}

RasterElementImporterShell::~RasterElementImporterShell()
{}

bool RasterElementImporterShell::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = mpPlugInManager->getPlugInArgList()) != NULL);
   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL));
   VERIFY(pArgList->addArg<RasterElement>(Importer::ImportElementArg()));
   return true;
}

bool RasterElementImporterShell::getOutputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = mpPlugInManager->getPlugInArgList()) != NULL);
   if (!isBatch())
   {
      VERIFY(pArgList->addArg<SpatialDataView>(Executable::ViewArg()));
   }
   return true;
}

bool RasterElementImporterShell::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }

   string importer = getName();
   if (importer.empty())
   {
      importer = "Raster Element Importer";
   }

   // Create a message log step
   StepResource pStep(string("Execute ") + importer,
      "app", "5021BA00-7744-49F8-A132-CA600DCBCA82", "Execute failed");

   // Extract the input args
   bool bSuccess = parseInputArgList(pInArgList);
   if (bSuccess == false)
   {
      return false;
   }

   // Update the log and progress with the start of the import
   string message = importer + " started!";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(message, 1, NORMAL);
   }

   if (!performImport())
   {
      return false;
   }

   if (!Service<SessionManager>()->isSessionLoading())
   {
      // Create the GcpList
      mpGcpList = createGcpList();

      // Georeference the raster element
      if (RasterElementImporterShell::getSettingAutoGeoreference() == true)
      {
         // Get the Georeference plug-in
         PlugIn* pGeoPlugIn = NULL;
         if (RasterElementImporterShell::getSettingImporterGeoreferencePlugIn() == true)
         {
            pGeoPlugIn = getGeoreferencePlugIn();
         }
         else
         {
            vector<string> plugInNames = RasterElementImporterShell::getSettingGeoreferencePlugIns();
            for (vector<string>::const_iterator iter = plugInNames.begin(); iter != plugInNames.end(); ++iter)
            {
               PlugIn* pPlugIn = mpPlugInManager->createPlugIn(*iter);
               if (pPlugIn != NULL)
               {
                  Georeference* pGeoreference = dynamic_cast<Georeference*>(pPlugIn);
                  if (pGeoreference != NULL)
                  {
                     if (pGeoreference->canHandleRasterElement(mpRasterElement) == true)
                     {
                        pGeoPlugIn = pPlugIn;
                        break;
                     }
                  }

                  mpPlugInManager->destroyPlugIn(pPlugIn);
               }
            }
         }

         // Perform the Georeference
         if (pGeoPlugIn != NULL)
         {
            ExecutableResource geoPlugIn(pGeoPlugIn, string(), mpProgress, true);
            geoPlugIn->getInArgList().setPlugInArgValue(Executable::DataElementArg(), mpRasterElement);
            geoPlugIn->getInArgList().setPlugInArgValue(Georeference::GcpListArg(), mpGcpList);
            if (geoPlugIn->execute() == false)
            {
               string message = "Could not georeference the data set.";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, WARNING);
               }

               pStep->addMessage(message, "app", "A8050A4B-824A-4E60-88E5-729367DEEAD0");
            }
         }
         else if (mpGcpList != NULL)
         {
            string message = "A georeference plug-in is not available to georeference the data set.";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(message, 0, WARNING);
            }

            pStep->addMessage(message, "app", "44E8D3C8-64C3-44DC-AB65-43F433D69DC8");
         }
      }

      // Create the view
      if (isBatch() == false)
      {
         SpatialDataView* pView = createView();
         if (pView == NULL)
         {
            message = "The view could not be created!";
            pStep->finalize(Message::Failure, message);
            return false;
         }

         // Add the view to the output arg list
         if (pOutArgList != NULL)
         {
            pOutArgList->setPlugInArgValue(Executable::ViewArg(), pView);
         }
      }
   }

   if (mpProgress != NULL)
   {
      message = "Raster element import complete!";
      mpProgress->updateProgress(message, 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

bool RasterElementImporterShell::validate(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   bool isValid = ImporterShell::validate(pDescriptor, errorMessage);
   if (isValid == false)
   {
      ValidationTest errorTest = getValidationError();
      if (errorTest == NO_PRE_POST_BAND_BYTES)
      {
         errorMessage += "  Preband and postband bytes are not supported for interleave formats other than BSQ.";
      }
      else if (errorTest == NO_BAND_FILES)
      {
         errorMessage += "  Bands in multiple files are not supported for interleave formats other than BSQ.";
      }
      else if ((errorTest == NO_INTERLEAVE_CONVERSIONS) || (errorTest == NO_ROW_SKIP_FACTOR) ||
         (errorTest == NO_COLUMN_SKIP_FACTOR) || (errorTest == NO_BAND_SUBSETS))
      {
         errorMessage = errorMessage.substr(0, errorMessage.length() - 1);
         errorMessage += " with on-disk read-only processing.";
      }
   }

   return isValid;
}

bool RasterElementImporterShell::isProcessingLocationSupported(ProcessingLocation location) const
{
   if ((location == IN_MEMORY) || (location == ON_DISK_READ_ONLY) || (location == ON_DISK))
   {
      return true;
   }

   return false;
}

QWidget* RasterElementImporterShell::getPreview(const DataDescriptor* pDescriptor, Progress* pProgress)
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   // Create a copy of the descriptor to change the loading parameters
   string previewName = string("Preview: ") + pDescriptor->getName();

   RasterDataDescriptor* pLoadDescriptor = dynamic_cast<RasterDataDescriptor*>(pDescriptor->copy(previewName, NULL));
   if (pLoadDescriptor == NULL)
   {
      return NULL;
   }

   // Set the active row and column numbers
   vector<DimensionDescriptor> newRows = pLoadDescriptor->getRows();
   for (unsigned int i = 0; i < newRows.size(); ++i)
   {
      newRows[i].setActiveNumber(i);
   }
   pLoadDescriptor->setRows(newRows);

   vector<DimensionDescriptor> newColumns = pLoadDescriptor->getColumns();
   for (unsigned int i = 0; i < newColumns.size(); ++i)
   {
      newColumns[i].setActiveNumber(i);
   }
   pLoadDescriptor->setColumns(newColumns);

   // Set the bands to load to just the first band and display it in grayscale mode
   const vector<DimensionDescriptor>& bands = pLoadDescriptor->getBands();
   if (bands.empty() == false)
   {
      DimensionDescriptor displayBand = bands.front();
      displayBand.setActiveNumber(0);

      vector<DimensionDescriptor> newBands;
      newBands.push_back(displayBand);

      pLoadDescriptor->setBands(newBands);
      pLoadDescriptor->setDisplayMode(GRAYSCALE_MODE);
      pLoadDescriptor->setDisplayBand(GRAY, displayBand);
   }

   // Set the processing location to load on-disk read-only
   pLoadDescriptor->setProcessingLocation(ON_DISK_READ_ONLY);

   // Validate the preview
   string errorMessage;
   bool bValidPreview = validate(pLoadDescriptor, errorMessage);
   if (bValidPreview == false)
   {
      // Try an in-memory preview
      pLoadDescriptor->setProcessingLocation(IN_MEMORY);
      bValidPreview = validate(pLoadDescriptor, errorMessage);
   }

   QWidget* pPreviewWidget = NULL;
   if (bValidPreview == true)
   {
      // Create the model element
      RasterElement* pRasterElement = static_cast<RasterElement*>(mpModel->createElement(pLoadDescriptor));
      if (pRasterElement != NULL)
      {
         // Add the progress and raster element to an input arg list
         PlugInArgList* pInArgList = NULL;
         bool bSuccess = getInputSpecification(pInArgList);
         if ((bSuccess == true) && (pInArgList != NULL))
         {
            bSuccess = pInArgList->setPlugInArgValue(Executable::ProgressArg(), pProgress);
            if (bSuccess)
            {
               bSuccess = pInArgList->setPlugInArgValue(Importer::ImportElementArg(), pRasterElement);
            }
         }

         // Load the data in batch mode
         bool bBatch = isBatch();
         setBatch();

         bSuccess = execute(pInArgList, NULL);

         // Restore to interactive mode if necessary
         if (bBatch == false)
         {
            setInteractive();
         }

         // Create the spatial data view
         if (bSuccess == true)
         {
            string name = pRasterElement->getName();

            SpatialDataView* pView = static_cast<SpatialDataView*>(mpDesktop->createView(name, SPATIAL_DATA_VIEW));
            if (pView != NULL)
            {
               // Set the spatial data in the view
               pView->setPrimaryRasterElement(pRasterElement);

               // Add the cube layer
               RasterLayer* pLayer = static_cast<RasterLayer*>(pView->createLayer(RASTER, pRasterElement));
               if (pLayer != NULL)
               {
                  // Get the widget from the view
                  pPreviewWidget = pView->getWidget();
               }
               else
               {
                  string message = "Could not create the cube layer!";
                  if (pProgress != NULL)
                  {
                     pProgress->updateProgress(message, 0, ERRORS);
                  }

                  mpModel->destroyElement(pRasterElement);
               }
            }
            else
            {
               string message = "Could not create the view!";
               if (pProgress != NULL)
               {
                  pProgress->updateProgress(message, 0, ERRORS);
               }

               mpModel->destroyElement(pRasterElement);
            }
         }
         else
         {
            mpModel->destroyElement(pRasterElement);
         }
      }
   }

   // Delete the data descriptor copy
   mpModel->destroyDataDescriptor(pLoadDescriptor);

   return pPreviewWidget;
}

bool RasterElementImporterShell::parseInputArgList(PlugInArgList* pInArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }

   StepResource pStep("Validate Inputs", "app", "4CDCFA20-2A40-452D-9956-264A35F8B883");

   // Extract the input args
   PlugInArg* pArg = NULL;
   bool bSuccess = false;

   // Progress
   bSuccess = pInArgList->getArg(Executable::ProgressArg(), pArg);
   if ((bSuccess == true) && (pArg != NULL))
   {
      if (pArg->isActualSet() == true)
      {
         mpProgress = reinterpret_cast<Progress*>(pArg->getActualValue());
      }
   }

   // Sensor data
   mpRasterElement = pInArgList->getPlugInArgValue<RasterElement>(Importer::ImportElementArg());
   if (mpRasterElement == NULL)
   {
      string msg = "The raster element input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(msg, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, msg);
      return false;
   }

   pStep->finalize(Message::Success);
   return true;
}

Progress* RasterElementImporterShell::getProgress() const
{
   return mpProgress;
}

RasterElement* RasterElementImporterShell::getRasterElement() const
{
   return mpRasterElement;
}

int RasterElementImporterShell::getValidationTest(const DataDescriptor* pDescriptor) const
{
   int validationTest = ImporterShell::getValidationTest(pDescriptor) | VALID_CLASSIFICATION |
      RASTER_SIZE | VALID_DATA_TYPE;
   if (pDescriptor != NULL)
   {
      ProcessingLocation processingLocation = pDescriptor->getProcessingLocation();
      if (processingLocation == IN_MEMORY)
      {
         validationTest |= AVAILABLE_MEMORY;
      }
      else if (processingLocation == ON_DISK_READ_ONLY)
      {
         validationTest |= NO_INTERLEAVE_CONVERSIONS | NO_BAND_SUBSETS | NO_SKIP_FACTORS;
      }

      const RasterFileDescriptor* pFileDescriptor =
         dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
      if (pFileDescriptor != NULL)
      {
         if (pFileDescriptor->getInterleaveFormat() != BSQ)
         {
            validationTest |= NO_PRE_POST_BAND_BYTES | NO_BAND_FILES;
         }

         if (pFileDescriptor->getBandFiles().empty() == false)
         {
            validationTest |= EXISTING_BAND_FILES;
         }
      }
   }

   return validationTest;
}

bool RasterElementImporterShell::performImport() const
{
   Progress* pProgress = getProgress();
   StepResource pStep("Perform import", "app", "762EF4BB-8813-4e45-B1BD-4CD237F7C151");

   FAIL_IF(mpRasterElement == NULL, "Could not find RasterElement", return false);

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(
      mpRasterElement->getDataDescriptor());
   FAIL_IF(pDescriptor == NULL, "Could not find RasterDataDescriptor", return false);

   string message;

//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Re-evaluate this code when plug-ins " \
//   "are being loaded into the global symbol space (tclarke)")
   // This was changed from a try/catch due to a problem with the exceptions 
   // not being caught and propagating up to QApplication::notify on solaris.
   // it is believed this is due to how we load plug-ins;
   // they are loaded into a private symbol space. When this changes,
   // re-evaluate the try/catch model.
   if (pDescriptor->getProcessingLocation() == ON_DISK_READ_ONLY)
   {
      Service<SessionManager> pSessionManager;
      if (pSessionManager->isSessionLoading() == false)
      {
         RasterFileDescriptor* pOrigFileDescriptor = dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
         std::vector<DimensionDescriptor> orgRows = pOrigFileDescriptor->getRows();
         std::vector<DimensionDescriptor> orgColumns = pOrigFileDescriptor->getColumns();
         std::vector<DimensionDescriptor> orgBands = pOrigFileDescriptor->getBands();

         std::vector<DimensionDescriptor>::iterator iter;
         unsigned int i = 0;
         for (iter = orgRows.begin(), i = 0; iter != orgRows.end(); ++iter, ++i)
         {
            iter->setActiveNumber(i);
         }
         for (iter = orgColumns.begin(), i = 0; iter != orgColumns.end(); ++iter, ++i)
         {
            iter->setActiveNumber(i);
         }
         for (iter = orgBands.begin(), i = 0; iter != orgBands.end(); ++iter, ++i)
         {
            iter->setActiveNumber(i);
         }
         vector<DimensionDescriptor> selectedRows = getSelectedDims(orgRows,
            pDescriptor->getRows());
         vector<DimensionDescriptor> selectedColumns = getSelectedDims(orgColumns,
            pDescriptor->getColumns());
         vector<DimensionDescriptor> selectedBands = getSelectedDims(orgBands,
            pDescriptor->getBands());

         if (!RasterUtilities::chipMetadata(mpRasterElement->getMetadata(),
            selectedRows, selectedColumns, selectedBands))
         {
            return checkAbortOrError("Could not chip metadata", pStep.get());
         }
      }

      if (createRasterPager(mpRasterElement) == false)
      {
         return checkAbortOrError("Could not create pager for RasterElement", pStep.get());
      }
   }
   else
   {
      if (mpRasterElement->createDefaultPager() == false)
      {
         return checkAbortOrError("Could not allocate resources for new RasterElement", pStep.get());
      }
      RasterDataDescriptor* pSourceDescriptor = RasterUtilities::generateUnchippedRasterDataDescriptor(mpRasterElement);
      if (pSourceDescriptor == NULL)
      {
         return checkAbortOrError("Could not get unchipped RasterDataDescriptor", pStep.get());
      }

      ModelResource<RasterElement> pSourceRaster(pSourceDescriptor);
      if (pSourceRaster.get() == NULL)
      {
         return checkAbortOrError("Could not create source RasterElement", pStep.get());
      }

      if (createRasterPager(pSourceRaster.get()) == false)
      {
         return checkAbortOrError("Could not create pager for source RasterElement", pStep.get());
      }

      if (copyData(pSourceRaster.get()) == false)
      {
         return checkAbortOrError("Could not copy data from source RasterElement", pStep.get());
      }

      double value = 0.0;
      uint64_t badValueCount = mpRasterElement->sanitizeData(value);
      if (badValueCount != 0)
      {
         if (mpProgress != NULL)
         {
            string message = StringUtilities::toDisplayString(badValueCount) + " bad value(s) found in data.\n" +
               "Bad values set to " + StringUtilities::toDisplayString(value);
            mpProgress->updateProgress(message, 100, WARNING);
         }
      }
   }

   pStep->finalize(Message::Success);
   return true;
}

bool RasterElementImporterShell::checkAbortOrError(string message, Step* pStep, bool checkForError) const
{
   bool error = true;
   if (isAborted())
   {
      error = false;
      message = getName() + " Aborted!";
   }
   else if (!checkForError)
   {
      return true;
   }

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(message, 0, error ? ERRORS : ABORT);
   }

   pStep->finalize(error ? Message::Failure : Message::Abort, message);
   return false;
}

SpatialDataView* RasterElementImporterShell::createView() const
{
   if (mpRasterElement == NULL)
   {
      return NULL;
   }

   StepResource pStep("Create view", "app", "F41DCDE3-A5C9-4CE7-B9D4-7DF5A9063840");
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Creating view...", 99, NORMAL);
   }

   // Get the data set name
   const string& name = mpRasterElement->getName();
   if (name.empty() == true)
   {
      string message = "The data set name is invalid!  A view cannot be created.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return NULL;
   }

   // Create the spatial data window
   SpatialDataView* pView = NULL;

   SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(mpDesktop->createWindow(name, SPATIAL_DATA_WINDOW));
   if (pWindow != NULL)
   {
      pView = pWindow->getSpatialDataView();
   }

   if (pView == NULL)
   {
      string message = "Could not create the view window!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return NULL;
   }

   // Set the spatial data in the view
   pView->setPrimaryRasterElement(mpRasterElement);

   // Create the layers
   {
      UndoLock lock(pView);
      createRasterLayer(pView, pStep.get());
      createGcpLayer(pView, pStep.get());
      createLatLonLayer(pView, pStep.get());
   }

   // Check for at least one layer in the view
   LayerList* pLayerList = pView->getLayerList();
   VERIFYRV(pLayerList != NULL, NULL);

   if (pLayerList->getNumLayers() == 0)
   {
      mpDesktop->deleteWindow(pWindow);

      string message = "The view contains no layers, so it will not be created.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return NULL;
   }

   pStep->finalize(Message::Success);
   return pView;
}

RasterLayer* RasterElementImporterShell::createRasterLayer(SpatialDataView* pView, Step* pStep) const
{
   if ((pView == NULL) || (mpRasterElement == NULL))
   {
      return NULL;
   }

   RasterLayer* pLayer = static_cast<RasterLayer*>(pView->createLayer(RASTER, mpRasterElement));
   if (pLayer != NULL)
   {
      // Log the initial cube layer properties
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<RasterDataDescriptor*>(mpRasterElement->getDataDescriptor());
      VERIFYRV(pDescriptor != NULL, NULL);

      if (pStep != NULL)
      {
         DimensionDescriptor grayBand = pDescriptor->getDisplayBand(GRAY);
         DimensionDescriptor redBand = pDescriptor->getDisplayBand(RED);
         DimensionDescriptor greenBand = pDescriptor->getDisplayBand(GREEN);
         DimensionDescriptor blueBand = pDescriptor->getDisplayBand(BLUE);
         DisplayMode displayMode = pDescriptor->getDisplayMode();

         if (grayBand.isOriginalNumberValid())
         {
            pStep->addProperty("Gray Band", grayBand.getOriginalNumber());
         }

         if (redBand.isOriginalNumberValid())
         {
            pStep->addProperty("Red Band", redBand.getOriginalNumber());
         }
         else
         {
            pStep->addProperty("Red Band", "No Band Displayed");
         }

         if (greenBand.isOriginalNumberValid())
         {
            pStep->addProperty("Green Band", greenBand.getOriginalNumber());
         }
         else
         {
            pStep->addProperty("Green Band", "No Band Displayed");
         }

         if (blueBand.isOriginalNumberValid())
         {
            pStep->addProperty("Blue Band", blueBand.getOriginalNumber());
         }
         else
         {
            pStep->addProperty("Blue Band", "No Band Displayed");
         }

         pStep->addProperty("Display Mode", StringUtilities::toDisplayString(displayMode));
      }
   }
   else
   {
      string message = "Could not create the raster layer.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, WARNING);
      }

      if (pStep != NULL)
      {
         pStep->addMessage(message, "app", "3F06A978-3F1A-4E03-BBA7-E295A8B7CF72");
      }
   }

   return pLayer;
}

GcpLayer* RasterElementImporterShell::createGcpLayer(SpatialDataView* pView, Step* pStep) const
{
   if ((pView == NULL) || (mpGcpList == NULL))
   {
      return NULL;
   }

   GcpLayer* pLayer = static_cast<GcpLayer*>(pView->createLayer(GCP_LAYER, mpGcpList));
   if (pLayer == NULL)
   {
      string message = "Could not create a GCP layer for the GCPs in the file.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, WARNING);
      }

      if (pStep != NULL)
      {
         pStep->addMessage(message, "app", "78424C02-B767-472E-8EC9-8F1B9D11698A");
      }
   }

   return pLayer;
}

LatLonLayer* RasterElementImporterShell::createLatLonLayer(SpatialDataView* pView, Step* pStep) const
{
   if ((pView == NULL) || (mpRasterElement == NULL) || (mpRasterElement->isGeoreferenced() == false))
   {
      return NULL;
   }

   string resultsName = "GEO_RESULTS";
   bool displayLayer = true;     // Always set to display the layer; otherwise the plug-in will not create it

   ExecutableResource geoDisplayPlugIn("Georeference", string(), mpProgress, true);
   PlugInArgList& inArgList = geoDisplayPlugIn->getInArgList();
   VERIFY(inArgList.setPlugInArgValue(Executable::DataElementArg(), mpRasterElement));
   VERIFY(inArgList.setPlugInArgValue(Executable::ViewArg(), pView));
   VERIFY(inArgList.setPlugInArgValue("Results Name", &resultsName));
   VERIFY(inArgList.setPlugInArgValue("Display Layer", &displayLayer));
   if (geoDisplayPlugIn->execute() == false)
   {
      string message = "Could not create the latitude/longitude layer for the georeference data.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, WARNING);
      }

      if (pStep != NULL)
      {
         pStep->addMessage(message, "app", "04BCD4A9-9981-497D-8151-FE51A5149A3C");
      }

      return NULL;
   }

   PlugInArgList& outArgList = geoDisplayPlugIn->getOutArgList();

   LatLonLayer* pLayer = outArgList.getPlugInArgValue<LatLonLayer>("Latitude/Longitude Layer");
   if ((pLayer != NULL) && (RasterElementImporterShell::getSettingDisplayLatLonLayer() == false))
   {
      if (pView->hideLayer(pLayer) == false)
      {
         string message = "Could not hide the latitude/longitude layer.";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(message, 0, WARNING);
         }

         if (pStep != NULL)
         {
            pStep->addMessage(message, "app", "EA1FA2D1-8D06-424B-83FE-960DE21F0D80");
         }
      }
   }

   return pLayer;
}

GcpList* RasterElementImporterShell::createGcpList() const
{
   if (mpRasterElement == NULL)
   {
      return NULL;
   }

   const RasterDataDescriptor* pDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(mpRasterElement->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      return NULL;
   }

   const list<GcpPoint>& gcps = pFileDescriptor->getGcps();
   if (gcps.empty() == true)
   {
      return NULL;
   }

   // Create the GCP list
   GcpList* pGcpList = static_cast<GcpList*>(mpModel->createElement("Corner Coordinates", "GcpList", mpRasterElement));
   if (pGcpList != NULL)
   {
      unsigned int onDiskStartRow = 0;
      unsigned int onDiskStartColumn = 0;

      unsigned int numActiveRows = pDescriptor->getRowCount();
      unsigned int numActiveColumns = pDescriptor->getColumnCount();
      unsigned int numOnDiskRows = pFileDescriptor->getRowCount();
      unsigned int numOnDiskColumns = pFileDescriptor->getColumnCount();

      if ((numActiveRows != numOnDiskRows) || (numActiveColumns != numOnDiskColumns))
      {
         const vector<DimensionDescriptor>& activeRows = pDescriptor->getRows();
         if (activeRows.empty() == false)
         {
            DimensionDescriptor rowDim = activeRows.front();
            if (rowDim.isOnDiskNumberValid())
            {
               onDiskStartRow = rowDim.getOnDiskNumber();
            }
         }

         const vector<DimensionDescriptor>& activeColumns = pDescriptor->getColumns();
         if (activeColumns.empty() == false)
         {
            DimensionDescriptor columnDim = activeColumns.front();
            if (columnDim.isOnDiskNumberValid())
            {
               onDiskStartColumn = columnDim.getOnDiskNumber();
            }
         }
      }

      // Add the GCPs to the GCP list
      list<GcpPoint> adjustedGcps;

      list<GcpPoint>::const_iterator iter;
      for (iter = gcps.begin(); iter != gcps.end(); ++iter)
      {
         GcpPoint gcp = *iter;
         gcp.mPixel.mX = gcp.mPixel.mX - onDiskStartColumn;
         gcp.mPixel.mY = gcp.mPixel.mY - onDiskStartRow;

         adjustedGcps.push_back(gcp);
      }

      pGcpList->addPoints(adjustedGcps);
   }

   return pGcpList;
}

GcpList* RasterElementImporterShell::getGcpList() const
{
   return mpGcpList;
}

PlugIn* RasterElementImporterShell::getGeoreferencePlugIn() const
{
   if (mpRasterElement == NULL)
   {
      return NULL;
   }

   ExecutableResource geoPlugIn("GCP Georeference", string(), mpProgress, true);

   const Georeference* pGeoreference = dynamic_cast<const Georeference*>(geoPlugIn->getPlugIn());
   if (pGeoreference != NULL)
   {
      if (pGeoreference->canHandleRasterElement(mpRasterElement) == true)
      {
         return geoPlugIn->releasePlugIn();
      }
   }

   return NULL;
}

bool RasterElementImporterShell::createRasterPager(RasterElement* pRaster) const
{
   string srcFile = pRaster->getFilename();
   if (srcFile.empty())
   {
      return false;
   }
   {
      //scoping to ensure the file is closed before calling createMemoryMappedPager
      LargeFileResource srcFileRes;
      if (!srcFileRes.open(srcFile, O_RDONLY | O_BINARY, S_IREAD))
      {
         return false;
      }
   }
   mUsingMemoryMappedPager = pRaster->createMemoryMappedPager();
   return mUsingMemoryMappedPager;
}

bool RasterElementImporterShell::copyData(const RasterElement* pSrcElement) const
{
   VERIFY(pSrcElement != NULL && mpRasterElement != NULL);

   const RasterDataDescriptor* pSrcDescriptor = dynamic_cast<const RasterDataDescriptor*>(
      pSrcElement->getDataDescriptor());
   RasterDataDescriptor* pChipDescriptor = dynamic_cast<RasterDataDescriptor*>(
      mpRasterElement->getDataDescriptor());
   VERIFY(pSrcDescriptor != NULL && mpRasterElement != NULL);

   vector<DimensionDescriptor> selectedRows = getSelectedDims(pSrcDescriptor->getRows(),
      pChipDescriptor->getRows());
   vector<DimensionDescriptor> selectedColumns = getSelectedDims(pSrcDescriptor->getColumns(),
      pChipDescriptor->getColumns());
   vector<DimensionDescriptor> selectedBands = getSelectedDims(pSrcDescriptor->getBands(),
      pChipDescriptor->getBands());

   bool success = true;

   Service<SessionManager> pSessionManager;
   if (pSessionManager->isSessionLoading() == false)
   {
      success = RasterUtilities::chipMetadata(mpRasterElement->getMetadata(), selectedRows, selectedColumns,
         selectedBands);
   }

   success = success && pSrcElement->copyDataToChip(mpRasterElement, selectedRows, 
      selectedColumns, selectedBands, mAborted, mpProgress);

   return success;
}
