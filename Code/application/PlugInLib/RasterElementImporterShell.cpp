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
#include "Classification.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "FileResource.h"
#include "GcpList.h"
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
#include "UtilityServices.h"

#include <limits>
using namespace std;

RasterElementImporterShell::RasterElementImporterShell() :
   mUsingMemoryMappedPager(false),
   mpProgress(NULL),
   mpRasterElement(NULL),
   mIsSessionLoad(false)
{
   setSubtype("Raster Element");
   allowMultipleInstances(true);
   setAbortSupported(true);
}

RasterElementImporterShell::~RasterElementImporterShell()
{
}

bool RasterElementImporterShell::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = mpPlugInManager->getPlugInArgList()) != NULL);
   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL));
   VERIFY(pArgList->addArg<RasterElement>(Importer::ImportElementArg()));
   VERIFY(pArgList->addArg(Importer::SessionLoadArg(), false));
   return true;
}

bool RasterElementImporterShell::getOutputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = mpPlugInManager->getPlugInArgList()) != NULL);
   if (!isBatch())
   {
      VERIFY(pArgList->addArg<SpatialDataView>("View"));
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

   // Create the view
   if (isBatch() == false && !mIsSessionLoad)
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
         PlugInArg* pArg = NULL;

         bSuccess = pOutArgList->getArg("View", pArg);
         if ((bSuccess == true) && (pArg != NULL))
         {
            pArg->setActualValue(pView);
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
   bool success = validateBasic(pDescriptor, errorMessage);
   if (success)
   {
      success = validateDefaults(pDescriptor, errorMessage);
   }

   return success;
}

bool RasterElementImporterShell::isProcessingLocationSupported(ProcessingLocation location) const
{
   if ((location == IN_MEMORY) || (location == ON_DISK_READ_ONLY) || (location == ON_DISK))
   {
      return true;
   }

   return false;
}

bool RasterElementImporterShell::validateBasic(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   bool bSuccess = ImporterShell::validate(pDescriptor, errorMessage);
   if (bSuccess == false)
   {
      return false;
   }

   const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
   if (pRasterDescriptor == NULL)
   {
      errorMessage = "The data descriptor is invalid!";
      return false;
   }

   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      errorMessage = "The file descriptor is invalid!";
      return false;
   }

   const Classification* pClassification = pRasterDescriptor->getClassification();
   if (pClassification == NULL)
   {
      errorMessage = "The classification is invalid!";
      return false;
   }

   // Data set size
   uint64_t loadedRows = pRasterDescriptor->getRowCount();
   uint64_t loadedColumns = pRasterDescriptor->getColumnCount();
   uint64_t loadedBands = pRasterDescriptor->getBandCount();

   if ((loadedRows == 0) || (loadedColumns == 0) || (loadedBands == 0))
   {
      errorMessage = "The data set is empty!  Check the size of the rows, columns, and bands.";
      return false;
   }

   // Pixel size
   unsigned int bitsPerElement = pFileDescriptor->getBitsPerElement();
   if (bitsPerElement == 0)
   {
      errorMessage = "The number of bits per element is invalid!";
      return false;
   }

   // Invalid pre-band and post-band bytes
   unsigned int prebandBytes = pFileDescriptor->getPrebandBytes();
   unsigned int postbandBytes = pFileDescriptor->getPostbandBytes();
   InterleaveFormatType interleave = pFileDescriptor->getInterleaveFormat();

   if (((prebandBytes != 0) || (postbandBytes != 0)) && (interleave != BSQ))
   {
      errorMessage = "Non-BSQ formatted data cannot have pre-band bytes and post-band bytes!";
      return false;
   }

   // Multiple band file restrictions
   const vector<const Filename*>& bandFiles = pFileDescriptor->getBandFiles();
   if (bandFiles.empty() == false)
   {
      // Not enough band files for all bands
      unsigned int numBands = pFileDescriptor->getBandCount();
      if (bandFiles.size() < numBands)
      {
         char buffer[1024];
         sprintf(buffer, "The number of band files specified (%d) is less than the total "
            "number of bands to be loaded (%d)!", bandFiles.size(), numBands);

         errorMessage = string(buffer);
         return false;
      }

      // Invalid file for imported bands
      const vector<DimensionDescriptor>& bands = pRasterDescriptor->getBands();

      vector<DimensionDescriptor>::const_iterator iter;
      for (iter = bands.begin(); iter != bands.end(); ++iter)
      {
         DimensionDescriptor bandDim = *iter;
         if (bandDim.isValid())
         {
            unsigned int onDiskNumber = bandDim.getOnDiskNumber();
            if (bandFiles.size() > onDiskNumber)
            {
               // Invalid filename
               VERIFY(bandFiles[onDiskNumber] != NULL);
               string bandFilename = bandFiles[onDiskNumber]->getFullPathAndName();
               if (bandFilename.empty() == true)
               {
                  errorMessage = "One or more of the band filenames of the bands to load is invalid!";
                  return false;
               }

               // Existing file
               LargeFileResource bandFile;
               if (!bandFile.open(bandFilename, O_RDONLY | O_BINARY, S_IREAD))
               {
                  errorMessage = "The band file: " + bandFilename + " does not exist!";
                  return false;
               }
            }
         }
      }

      // Non-BSQ data
      if (interleave != BSQ)
      {
         errorMessage = "Cannot load non-BSQ data in multiple files!";
         return false;
      }
   }

   // Valid memory
   ProcessingLocation processingLocation = pRasterDescriptor->getProcessingLocation();
   if (processingLocation == IN_MEMORY)
   {
      unsigned int bytesPerElement = pRasterDescriptor->getBytesPerElement();
      uint64_t dataSize = loadedRows * loadedColumns * loadedBands * bytesPerElement;
      uint64_t maxMemoryAvail = mpUtilities->getMaxMemoryBlockSize();
#if PTR_SIZE > 4
      uint64_t totalRam = mpUtilities->getTotalPhysicalMemory();
      if (totalRam < maxMemoryAvail)
      {
         maxMemoryAvail = totalRam;
      }
#endif

      if (dataSize > maxMemoryAvail)
      {
         errorMessage = "Cube cannot be loaded into memory, use a different processing location or subset the image!";
         return false;
      }
   }

   // Classification -- warn the user, but do not refuse to load the file
   FactoryResource<Classification> pSystemClassification;
   pSystemClassification->setLevel(Service<UtilityServices>()->getDefaultClassification());
   if (pClassification->hasGreaterLevel(pSystemClassification.get()) == true)
   {
      errorMessage += "THIS FILE CONTAINS CLASSIFIED INFORMATION WHICH SHOULD NOT BE PROCESSED ON THIS SYSTEM!\n"
         "THIS MAY CONSTITUTE A SECURITY VIOLATION WHICH SHOULD BE REPORTED TO YOUR SECURITY OFFICER!\n";
      StepResource pStep("Validate", "app", "1A881267-6A96-4eb2-A9D3-7D30334B0A0B", errorMessage);
   }

   return true;
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
            bSuccess = pInArgList->setPlugInArgValue(ProgressArg(), pProgress);
            if (bSuccess)
            {
               bSuccess = pInArgList->setPlugInArgValue(ImportElementArg(), pRasterElement);
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
   bSuccess = pInArgList->getArg(ProgressArg(), pArg);
   if ((bSuccess == true) && (pArg != NULL))
   {
      if (pArg->isActualSet() == true)
      {
         mpProgress = reinterpret_cast<Progress*>(pArg->getActualValue());
      }
   }

   pInArgList->getPlugInArgValue<bool>(SessionLoadArg(), mIsSessionLoad);

   // Sensor data
   mpRasterElement = pInArgList->getPlugInArgValue<RasterElement>(ImportElementArg());
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

bool RasterElementImporterShell::validateDefaultOnDiskReadOnly(const DataDescriptor* pDescriptor,
                                                               std::string& errorMessage) const
{
   const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
   if (pRasterDescriptor == NULL)
   {
      errorMessage = "The data descriptor is invalid!";
      return false;
   }

   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      errorMessage = "The file descriptor is invalid!";
      return false;
   }

   ProcessingLocation loc = pDescriptor->getProcessingLocation();
   if (loc == ON_DISK_READ_ONLY)
   {
      // Invalid filename
      const Filename& filename = pFileDescriptor->getFilename();
      if (filename.getFullPathAndName().empty() == true)
      {
         errorMessage = "The filename is invalid!";
         return false;
      }

      // Existing file
      LargeFileResource file;
      if (file.open(filename.getFullPathAndName(), O_RDONLY | O_BINARY, S_IREAD) == false)
      {
         errorMessage = "The file: " + string(filename) + " does not exist!";
         return false;
      }

      // File size
      const vector<const Filename*>& bandFiles = pFileDescriptor->getBandFiles();
      if (bandFiles.empty() == true)
      {
         uint64_t numRows = pFileDescriptor->getRowCount();
         uint64_t numColumns = pFileDescriptor->getColumnCount();
         uint64_t numBands = pFileDescriptor->getBandCount();
         unsigned int headerBytes = pFileDescriptor->getHeaderBytes();
         unsigned int bitsPerElement = pFileDescriptor->getBitsPerElement();
         unsigned int prelineBytes = pFileDescriptor->getPrelineBytes();
         unsigned int postlineBytes = pFileDescriptor->getPostlineBytes();
         unsigned int prebandBytes = pFileDescriptor->getPrebandBytes();
         unsigned int postbandBytes = pFileDescriptor->getPostbandBytes();
         unsigned int trailerBytes = pFileDescriptor->getTrailerBytes();

         uint64_t requiredSize = static_cast<uint64_t>(headerBytes) +
            (numRows * ((numBands * numColumns * bitsPerElement / 8) + prelineBytes + postlineBytes)) +
            (numBands * (prebandBytes + postbandBytes)) + trailerBytes;
         if (file.fileLength() < static_cast<int64_t>(requiredSize))
         {
            errorMessage = "The size of the file does not match the current parameters!";
            return false;
         }
      }

      // Interleave conversions
      InterleaveFormatType dataInterleave = pRasterDescriptor->getInterleaveFormat();
      InterleaveFormatType fileInterleave = pFileDescriptor->getInterleaveFormat();
      if (pFileDescriptor->getBandCount() > 1 && dataInterleave != fileInterleave)
      {
         errorMessage = "Interleave format conversions are not supported with on-disk read-only processing"
            " of data with more than one band!";
         return false;
      }

      //Subset
      unsigned int loadedBands = pRasterDescriptor->getBandCount();
      unsigned int fileBands = pFileDescriptor->getBandCount();

      if (loadedBands != fileBands)
      {
         errorMessage = "Band subsets are not supported with on-disk read-only processing!";
         return false;
      }

      unsigned int skipFactor = 0;
      if (!RasterUtilities::determineSkipFactor(pRasterDescriptor->getRows(), skipFactor)
         || skipFactor != 0)
      {
         errorMessage = "Skip factors are not supported for rows or columns with on-disk read-only processing.";
         return false;
      }
      if (!RasterUtilities::determineSkipFactor(pRasterDescriptor->getColumns(), skipFactor)
         || skipFactor != 0)
      {
         errorMessage = "Skip factors are not supported for rows or columns with on-disk read-only processing.";
         return false;
      }
   }

   return true;
}

bool RasterElementImporterShell::validateDefaults(const DataDescriptor* pDescriptor, std::string& errorMessage) const
{
   const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
   if (pRasterDescriptor == NULL)
   {
      errorMessage = "The data descriptor is invalid!";
      return false;
   }

   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      errorMessage = "The file descriptor is invalid!";
      return false;
   }

   // Processing location restrictions
   ProcessingLocation processingLocation = pRasterDescriptor->getProcessingLocation();
   if (!isProcessingLocationSupported(processingLocation))
   {
      errorMessage = "The requested processing location is not supported!";
      return false;
   }
   if (processingLocation == ON_DISK_READ_ONLY)
   {
      return validateDefaultOnDiskReadOnly(pDescriptor, errorMessage);
   }

   return true;
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

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Re-evalutate this code when plug-ins " \
   "are being loaded into the global symbol space (tclarke)")
   // This was changed from a try/catch due to a problem with the exceptions 
   // not being caught and propagating up to QApplication::notify on solaris.
   // it is believed this is due to how we load plug-ins;
   // they are loaded into a private symbol space. When this changes,
   // re-evaluate the try/catch model.
   if (pDescriptor->getProcessingLocation() == ON_DISK_READ_ONLY)
   {
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
         char buffer[256];
         sprintf (buffer, "%llu bad value(s) found in data.\nBad values set to %f.", badValueCount, value);
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(buffer, 100, WARNING);
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
   if ((isBatch() == true) || (mpRasterElement == NULL))
   {
      return NULL;
   }

   StepResource pStep("Create view", "app", "F41DCDE3-A5C9-4CE7-B9D4-7DF5A9063840");
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Creating view...", 99, NORMAL);
   }

   // Get the data set name
   string name = mpRasterElement->getName();
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

   // Block undo actions when creating the layers
   UndoLock lock(pView);

   // Add the cube layer
   RasterLayer* pLayer = static_cast<RasterLayer*>(pView->createLayer(RASTER, mpRasterElement));
   if (pLayer != NULL)
   {
      // Set the initial cube layer properties
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<RasterDataDescriptor*>(mpRasterElement->getDataDescriptor());
      if (pDescriptor != NULL)
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
      string message = "Could not create the cube layer!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, WARNING);
      }

      pStep->addMessage(message, "app", "3F06A978-3F1A-4E03-BBA7-E295A8B7CF72");
   }

   // Create a GCP layer for the GCPs in the file
   GcpList* pGcpList = createGcpList();
   if (pGcpList != NULL)
   {
      if (pView->createLayer(GCP_LAYER, pGcpList) == NULL)
      {
         string message = "Could not create a GCP layer for the GCPs in the file.";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(message, 0, WARNING);
         }

         pStep->addMessage(message, "app", "78424C02-B767-472E-8EC9-8F1B9D11698A");
      }
   }

   pStep->finalize(Message::Success);
   return pView;
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
