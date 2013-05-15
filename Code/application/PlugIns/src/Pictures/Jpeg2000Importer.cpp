/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "CachedPager.h"
#include "DimensionDescriptor.h"
#include "FileResource.h"
#include "ImportDescriptor.h"
#include "Jpeg2000Importer.h"
#include "Jpeg2000Utilities.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "UtilityServices.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksPictures, Jpeg2000Importer);

std::map<std::string, std::vector<std::string> > Jpeg2000Importer::msWarnings;
std::map<std::string, std::vector<std::string> > Jpeg2000Importer::msErrors;

Jpeg2000Importer::Jpeg2000Importer()
{
   setName("JPEG2000 Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("JPEG2000 Files (*.jp2 *.j2k *.j2c *.jpc)");
   setShortDescription("JPEG2000");
   setDescriptorId("{ECC55485-16FC-4154-B31B-E78EA3669B8E}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   addDependencyCopyright("OpenJPEG", Service<UtilityServices>()->getTextFromFile(":/licenses/openjpeg"));
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

   vector<string>& warnings = msWarnings[filename];
   warnings.clear();

   vector<string>& errors = msErrors[filename];
   errors.clear();

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
   // Read the image header info from the file
   opj_image_t* pImage = getImageInfo(filename, false);
   if (pImage != NULL)
   {
      opj_image_destroy(pImage);
      return Importer::CAN_LOAD;
   }

   return Importer::CAN_NOT_LOAD;
}

bool Jpeg2000Importer::validate(const DataDescriptor* pDescriptor,
                                const std::vector<const DataDescriptor*>& importedDescriptors,
                                string& errorMessage) const
{
   if (RasterElementImporterShell::validate(pDescriptor, importedDescriptors, errorMessage) == false)
   {
      return false;
   }

   VERIFY(pDescriptor != NULL);

   const FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
   VERIFY(pFileDescriptor != NULL);

   const Filename& filenameObj = pFileDescriptor->getFilename();
   string filename = filenameObj.getFullPathAndName();

   // Report errors obtained when populating the import descriptors
   map<string, vector<string> >::const_iterator errorIter = msErrors.find(filename);
   if (errorIter != msErrors.end())
   {
      const std::vector<std::string>& errors = errorIter->second;
      if (errors.empty() == false)
      {
         errorMessage = StringUtilities::join(errors, "\n");
         return false;
      }
   }

   // Report warnings obtained when populating the import descriptors
   map<string, vector<string> >::const_iterator warningIter = msWarnings.find(filename);
   if (warningIter != msWarnings.end())
   {
      const std::vector<std::string>& warnings = warningIter->second;
      if (warnings.empty() == false)
      {
         if (errorMessage.empty() == false)
         {
            errorMessage += "\n";
         }

         errorMessage += StringUtilities::join(warnings, "\n");
      }
   }

   return true;
}

opj_image_t* Jpeg2000Importer::getImageInfo(const string& filename, bool logErrors) const
{
   if (filename.empty() == true)
   {
      return NULL;
   }

   string filenameCopy = filename;

   // open a byte stream from the file
   FileResource pFile(filename.c_str(), "rb");
   if (pFile.get() == NULL)
   {
      return NULL;
   }

   opj_stream_t* pStream = opj_stream_create_default_file_stream(pFile, true);
   if (pStream == NULL)
   {
      return NULL;
   }

   // decode the code-stream
   opj_dparameters_t parameters;
   opj_set_default_decoder_parameters(&parameters);
   parameters.decod_format = Jpeg2000Utilities::get_file_format(filename.c_str());

   opj_codec_t* pCodec = NULL;
   switch (parameters.decod_format)
   {
   case Jpeg2000Utilities::J2K_CFMT:
      pCodec = opj_create_decompress(OPJ_CODEC_J2K);
      break;

   case Jpeg2000Utilities::JP2_CFMT:
      pCodec = opj_create_decompress(OPJ_CODEC_JP2);
      break;

   default:
      break;
   }

   if (pCodec == NULL)
   {
      opj_stream_destroy(pStream);
      return NULL;
   }

   // setup the callbacks to report warnings and errors
   if (logErrors == true)
   {
      opj_set_warning_handler(pCodec, Jpeg2000Importer::reportWarning, &filenameCopy);
      opj_set_error_handler(pCodec, Jpeg2000Importer::reportError, &filenameCopy);
   }
   else
   {
      opj_set_warning_handler(pCodec, Jpeg2000Importer::defaultCallback, NULL);
      opj_set_error_handler(pCodec, Jpeg2000Importer::defaultCallback, NULL);
   }

   // setup the decoder decoding parameters
   if (opj_setup_decoder(pCodec, &parameters) == OPJ_FALSE)
   {
      opj_stream_destroy(pStream);
      opj_destroy_codec(pCodec);
      return NULL;
   }

   // decode the stream and fill the image structure
   opj_image_t* pImage = NULL;
   if (opj_read_header(pStream, pCodec, &pImage) == OPJ_FALSE)
   {
      opj_stream_destroy(pStream);
      opj_destroy_codec(pCodec);
      return NULL;
   }

   // cleanup
   opj_stream_destroy(pStream);
   opj_destroy_codec(pCodec);

   return pImage;
}

bool Jpeg2000Importer::populateDataDescriptor(RasterDataDescriptor* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return false;
   }

   RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   VERIFY(pFileDescriptor != NULL);

   const string& fileName = pFileDescriptor->getFilename();
   if (fileName.empty() == true)
   {
      return false;
   }

   opj_image_t* pImage = getImageInfo(fileName, true);
   if (pImage == NULL)
   {
      return false;
   }

   // Bits per element
   unsigned int bitsPerElement = pImage->comps->prec;
   pFileDescriptor->setBitsPerElement(bitsPerElement);

   // Data type
   EncodingType dataType = INT1UBYTE;
   if (bitsPerElement <= 8)
   {
      if (pImage->comps->sgnd)
      {
         dataType = INT1SBYTE;
      }
      else
      {
         dataType = INT1UBYTE;
      }
   }
   else if (bitsPerElement <= 16)
   {
      if (pImage->comps->sgnd)
      {
         dataType = INT2SBYTES;
      }
      else
      {
         dataType = INT2UBYTES;
      }
   }
   else if (bitsPerElement <= 32)
   {
      if (pImage->comps->sgnd)
      {
         dataType = INT4SBYTES;
      }
      else
      {
         dataType = INT4UBYTES;
      }
   }
   else if (bitsPerElement <= 64)
   {
      dataType = FLT8BYTES;
   }

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
   unsigned int numRows = pImage->comps->h;
   vector<DimensionDescriptor> rows = RasterUtilities::generateDimensionVector(numRows, true, false, true);
   pDescriptor->setRows(rows);
   pFileDescriptor->setRows(rows);

   // Columns
   unsigned int numColumns = pImage->comps->w;
   vector<DimensionDescriptor> columns = RasterUtilities::generateDimensionVector(numColumns, true, false, true);
   pDescriptor->setColumns(columns);
   pFileDescriptor->setColumns(columns);

   // Bands
   unsigned int numBands = pImage->numcomps;
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

   opj_image_destroy(pImage);
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

   ExecutableResource pagerPlugIn("JPEG2000 Pager", string(), pProgress);
   pagerPlugIn->getInArgList().setPlugInArgValue(CachedPager::PagedElementArg(), pRasterElement);
   pagerPlugIn->getInArgList().setPlugInArgValue(CachedPager::PagedFilenameArg(), pFilename.get());
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

void Jpeg2000Importer::defaultCallback(const char* pMessage, void* pClientData)
{
   // Do nothing
   Q_UNUSED(pMessage);
   Q_UNUSED(pClientData);
}

void Jpeg2000Importer::reportWarning(const char* pMessage, void* pClientData)
{
   string* pFilename = reinterpret_cast<string*>(pClientData);
   if ((pFilename != NULL) && (pMessage != NULL))
   {
      vector<string>& warnings = msWarnings[*pFilename];
      warnings.push_back(string(pMessage));
   }
}

void Jpeg2000Importer::reportError(const char* pMessage, void* pClientData)
{
   string* pFilename = reinterpret_cast<string*>(pClientData);
   if ((pFilename != NULL) && (pMessage != NULL))
   {
      vector<string>& errors = msErrors[*pFilename];
      errors.push_back(string(pMessage));
   }
}
