/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#if defined (JPEG2000_SUPPORT)

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataAccessorImpl.h"
#include "DimensionDescriptor.h"
#include "Endian.h"
#include "FileResource.h"
#include "ImportDescriptor.h"
#include "Jpeg2000Importer.h"
#include "Jpeg2000Utilities.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "UtilityServices.h"

#include <errno.h>
#include <fstream>
#include <iostream>

#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

// These must be in this order
#include <openjpeg.h>
#include <opj_includes.h>
#include <j2k.h>
#include <jp2.h>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksPictures, Jpeg2000Importer);

Jpeg2000Importer::Jpeg2000Importer()
{
   setName("Jpeg2000 Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("Jpeg2000 files (*.jp2 *.j2k)");
   setShortDescription("Jpeg2000");
   setDescriptorId("{ECC55485-16FC-4154-B31B-E78EA3669B8E}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   addDependencyCopyright("OpenJPEG", Service<UtilityServices>()->getTextFromFile(":/licenses/openjpeg"));
   addDependencyCopyright("proj4", Service<UtilityServices>()->getTextFromFile(":/licenses/proj4"));
}

Jpeg2000Importer::~Jpeg2000Importer()
{}

vector<ImportDescriptor*> Jpeg2000Importer::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;
   if (filename.empty() == true)
   {
      return descriptors;
   }

   ImportDescriptor* pImportDescriptor = mpModel->createImportDescriptor(filename, "RasterElement", NULL);
   if (pImportDescriptor != NULL)
   {
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         vector<EncodingType> validDataTypes;
         validDataTypes.push_back(INT1UBYTE);
         validDataTypes.push_back(INT1SBYTE);
         validDataTypes.push_back(INT2UBYTES);
         validDataTypes.push_back(INT2SBYTES);
         validDataTypes.push_back(INT4UBYTES);
         validDataTypes.push_back(INT4SBYTES);
         validDataTypes.push_back(FLT4BYTES);
         pDescriptor->setValidDataTypes(validDataTypes);
         pDescriptor->setProcessingLocation(IN_MEMORY);

         // Create and set a file descriptor in the data descriptor
         FactoryResource<RasterFileDescriptor> pFileDescriptor;
         pFileDescriptor->setEndian(BIG_ENDIAN_ORDER);
         if (pFileDescriptor.get() != NULL)
         {
            pFileDescriptor->setFilename(filename);
            pDescriptor->setFileDescriptor(pFileDescriptor.get());
         }

         // Populate the data descriptor from the file
         bool bSuccess = populateDataDescriptor(pDescriptor);
         if (bSuccess == true)
         {
            descriptors.push_back(pImportDescriptor);
         }
         else
         {
            // Delete the import descriptor
            mpModel->destroyImportDescriptor(pImportDescriptor);
         }
      }
   }

   return descriptors;
}

unsigned char Jpeg2000Importer::getFileAffinity(const std::string& filename)
{
   if (Jpeg2000Utilities::get_file_format(filename.c_str())== -1)
   {
      return Importer::CAN_NOT_LOAD;
   }
   else
   {
      return Importer::CAN_LOAD;
   }
}

bool Jpeg2000Importer::populateDataDescriptor(RasterDataDescriptor* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return false;
   }

   RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>
      (pDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      return false;
   }

   const string& fileName = pFileDescriptor->getFilename();
   if (fileName.empty() == true)
   {
      return false;
   }

   opj_dparameters_t parameters;
   opj_event_mgr_t eventMgr;
   opj_image_t* pImage = NULL;
   vector<unsigned char> pSrc(NULL);
   int fileLength;
   opj_dinfo_t* pDinfo = NULL;
   opj_cio_t* pCio = NULL;
   opj_codestream_info_t cstrInfo;


   // configure the event callbacks
   memset(&eventMgr, 0, sizeof(opj_event_mgr_t));

   // set decoding parameters to default values
   opj_set_default_decoder_parameters(&parameters);

   FileResource pFile(fileName.c_str(), "rb");
   if (pFile.get() == NULL)
   {
      return false;
   }

   fseek(pFile, 0, SEEK_END);
   fileLength = ftell(pFile);
   fseek(pFile, 0, SEEK_SET);
   pSrc.resize(fileLength);
   fread(&pSrc[0], 1, fileLength, pFile);

   // decode the code-stream 
   parameters.decod_format = Jpeg2000Utilities::get_file_format(fileName.c_str());
   switch(parameters.decod_format) 
   {
      case Jpeg2000Utilities::J2K_CFMT:
      {
         pDinfo = opj_create_decompress(CODEC_J2K);
      }
      break;

      case Jpeg2000Utilities::JP2_CFMT:
      {
         pDinfo = opj_create_decompress(CODEC_JP2);
      }
      break;
      default:
         return false;
   }

   // catch events using our callbacks
   opj_set_event_mgr((opj_common_ptr)pDinfo, &eventMgr, stderr);

   // setup the decoder decoding parameters
   opj_setup_decoder(pDinfo, &parameters);

   // open a byte stream
   pCio = opj_cio_open((opj_common_ptr)pDinfo, &pSrc[0], fileLength);

   // decode the stream and fill the image structure
   pImage = opj_decode_with_info(pDinfo, pCio, &cstrInfo);
   if(!pImage)
   {
      return false;
   }

   // close the byte stream
   opj_cio_close(pCio);

   // Bits per pixel doesn't work in OpenJpeg V 1.5
   // If we upgrade to OpenJpg V 2.0 we need to check the library
   unsigned short bitsPerElement = 32;
   pFileDescriptor->setBitsPerElement(bitsPerElement);

   EncodingType dataType = INT1UBYTE;

   // Override with information from the filename, if present.
   unsigned int bandFactor = 0;
   QStringList parts = QString::fromStdString(fileName).split('.');
   foreach (QString part, parts)
   {
      bool error;
      EncodingType dataTypeTemp = StringUtilities::fromXmlString<EncodingType>(part.toStdString(), &error);
      if (dataTypeTemp.isValid() == true && error == false)
      {
         bandFactor = Jpeg2000Utilities::get_num_bands(dataTypeTemp);
         if (bandFactor != 0)
         {
            dataType = dataTypeTemp;
            break;
         }
      }
   }

   pDescriptor->setDataType(dataType);

   // Rows
   unsigned int numRows = pImage->y1;
   vector<DimensionDescriptor> rows = RasterUtilities::generateDimensionVector(numRows, true, false, true);
   pDescriptor->setRows(rows);
   pFileDescriptor->setRows(rows);

   // Columns
   unsigned int numColumns = pImage->x1;
   vector<DimensionDescriptor> columns = RasterUtilities::generateDimensionVector(numColumns, true, false, true);
   pDescriptor->setColumns(columns);
   pFileDescriptor->setColumns(columns);

   // Bands
   unsigned short numBands = pImage->numcomps;
   if (bandFactor > 0)
   {
      numBands /= bandFactor;
   }
   vector<DimensionDescriptor> bands = RasterUtilities::generateDimensionVector(numBands, true, false, true);
   pDescriptor->setBands(bands);
   pFileDescriptor->setBands(bands);

   // If red, green and blue bands exist, set the display mode to RGB.
   if (numBands >= 3)
   {
      pDescriptor->setDisplayBand(RED, bands[0]);
      pDescriptor->setDisplayBand(GREEN, bands[1]);
      pDescriptor->setDisplayBand(BLUE, bands[2]);
      pDescriptor->setDisplayMode(RGB_MODE);
   }

   if (pDinfo)
   {
      opj_destroy_decompress(pDinfo);
   }
   opj_destroy_cstr_info(&cstrInfo);
   opj_image_destroy(pImage);
   return true;
}

bool Jpeg2000Importer::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }

   // Create a message log step
   StepResource pStep("Execute JPEG2000 Importer", "app", "CE0A6D86-5708-42f9-8486-A0D037355659", "Execute failed");

   // Extract the input args
   bool bSuccess = parseInputArgList(pInArgList);
   if (!bSuccess)
   {
      return false;
   }

   // Update the log and progress with the start of the import
   Progress* pProgress = getProgress();
   if (pProgress != NULL)
   {
      pProgress->updateProgress("JPEG2000 Importer Started", 1, NORMAL);
   }

   if (!performImport())
   {
      return false;
   }

   // Create the view
   if (!isBatch() && !Service<SessionManager>()->isSessionLoading())
   {
      SpatialDataView* pView = createView();
      if (pView == NULL)
      {
         pStep->finalize(Message::Failure, "The view could not be created.");
         return false;
      }

      // Add the view to the output arg list
      if (pOutArgList != NULL)
      {
         pOutArgList->setPlugInArgValue("View", pView);
      }
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Jpeg2000 Import Complete.", 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

bool Jpeg2000Importer::createRasterPager(RasterElement* pRasterElement) const
{
   VERIFY(pRasterElement != NULL);
   DataDescriptor* pDescriptor = pRasterElement->getDataDescriptor();
   VERIFY(pDescriptor != NULL);
   FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
   VERIFY(pFileDescriptor != NULL);

   const string& filename = pRasterElement->getFilename();
   Progress* pProgress = getProgress();

   StepResource pStep("Create pager for Jpeg2000", "app", "35E3A23E-DD3B-4d9b-AB40-310176D93223");

   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(filename);

   ExecutableResource pagerPlugIn("Jpeg2000Pager", string(), pProgress);
   pagerPlugIn->getInArgList().setPlugInArgValue("Raster Element", pRasterElement);
   pagerPlugIn->getInArgList().setPlugInArgValue("Filename", pFilename.get());
   bool success = pagerPlugIn->execute();

   RasterPager* pPager = dynamic_cast<RasterPager*>(pagerPlugIn->getPlugIn());
   if ((pPager == NULL) || (success == false))
   {
      string message = "Execution of Jpeg2000Pager failed!";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   pRasterElement->setPager(pPager);
   pagerPlugIn->releasePlugIn();

   pStep->finalize();
   return true;
}

QWidget* Jpeg2000Importer::getPreview(const DataDescriptor* pDescriptor, Progress* pProgress)
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
      if (pLoadDescriptor->getBandCount() >= 3)
      {
         pLoadDescriptor->setDisplayBand(RED, bands[0]);
         pLoadDescriptor->setDisplayBand(GREEN, bands[1]);
         pLoadDescriptor->setDisplayBand(BLUE, bands[2]);
         pLoadDescriptor->setDisplayMode(RGB_MODE);
      }
      else
      {
         DimensionDescriptor displayBand = bands.front();

         vector<DimensionDescriptor> newBands;
         newBands.push_back(displayBand);

         pLoadDescriptor->setBands(newBands);
         displayBand.setActiveNumber(0);
         pLoadDescriptor->setDisplayMode(GRAYSCALE_MODE);
         pLoadDescriptor->setDisplayBand(GRAY, displayBand);
      }
   }

   // Validate the preview
   string errorMessage;
   // Try an in-memory preview
   pLoadDescriptor->setProcessingLocation(IN_MEMORY);
   bool bValidPreview = validate(pLoadDescriptor, vector<const DataDescriptor*>(), errorMessage);

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

bool Jpeg2000Importer::isProcessingLocationSupported(ProcessingLocation location) const
{
   return location == IN_MEMORY;
}

#endif