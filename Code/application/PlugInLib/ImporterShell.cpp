/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "Classification.h"
#include "DataDescriptor.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "ImporterShell.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInManagerServices.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"
#include "UtilityServices.h"
#include "Wavelengths.h"

#include <sstream>
using namespace std;

ImporterShell::ImporterShell()
{
   setType(PlugInManagerServices::ImporterType());
   setWizardSupported(false);
}

ImporterShell::~ImporterShell()
{}

string ImporterShell::getDefaultExtensions() const
{
   return mExtension;
}

bool ImporterShell::isProcessingLocationSupported(ProcessingLocation location) const
{
   return (location == IN_MEMORY);
}

QWidget* ImporterShell::getPreview(const DataDescriptor* pDescriptor, Progress* pProgress)
{
   return NULL;
}

bool ImporterShell::validate(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   mValidationError = ValidationTest();

   // Check for no validation
   int validationTest = getValidationTest(pDescriptor);
   if (validationTest == NO_VALIDATION)
   {
      return true;
   }

   // Always validate the data descriptor and file descriptor
   if (pDescriptor == NULL)
   {
      errorMessage = "The data set information is invalid.";
      return false;
   }

   const FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
   if (pFileDescriptor == NULL)
   {
      errorMessage = "The data set does not contain valid file information.";
      return false;
   }

   // Existing file
   const string& filename = pFileDescriptor->getFilename();
   if (validationTest & EXISTING_FILE)
   {
      // Valid filename
      if (filename.empty() == true)
      {
         errorMessage = "The filename is invalid.";
         mValidationError = EXISTING_FILE;
         return false;
      }

      // Existing file
      LargeFileResource file(true);
      if (!file.open(filename.c_str(), O_RDONLY | O_BINARY, S_IREAD))
      {
         errorMessage = "The file: " + filename + " does not exist.";
         mValidationError = EXISTING_FILE;
         return false;
      }
   }

   // Existing data element
   if (validationTest & NO_EXISTING_DATA_ELEMENT)
   {
      const string& name = pDescriptor->getName();
      const string& type = pDescriptor->getType();
      DataElement* pParent = pDescriptor->getParent();

      Service<ModelServices> pModel;
      if (pModel->getElement(name, type, pParent) != NULL)
      {
         errorMessage = "The data set currently exists.  It may have already been imported.";
         mValidationError = NO_EXISTING_DATA_ELEMENT;
         return false;
      }
   }

   // Valid classification
   Service<UtilityServices> pUtilities;
   if (validationTest & VALID_CLASSIFICATION)
   {
      // Existing Classification object
      const Classification* pClassification = pDescriptor->getClassification();
      if (pClassification == NULL)
      {
         errorMessage = "The required classification does not exist.";
         mValidationError = VALID_CLASSIFICATION;
         return false;
      }

      // Unauthorized classification level on the system - warn the user, but continue to load the file
      FactoryResource<Classification> pSystemClassification;
      pSystemClassification->setLevel(pUtilities->getDefaultClassification());
      if (pClassification->hasGreaterLevel(pSystemClassification.get()) == true)
      {
         errorMessage = "THIS FILE CONTAINS CLASSIFIED INFORMATION WHICH SHOULD NOT BE PROCESSED ON THIS SYSTEM!\n"
            "THIS MAY CONSTITUTE A SECURITY VIOLATION WHICH SHOULD BE REPORTED TO YOUR SECURITY OFFICER!\n";
         StepResource pStep("Validate", "app", "1A881267-6A96-4eb2-A9D3-7D30334B0A0B", errorMessage);
      }
   }

   // Valid metadata
   if (validationTest & VALID_METADATA)
   {
      if (pDescriptor->getMetadata() == NULL)
      {
         errorMessage = "The required metadata does not exist.";
         mValidationError = VALID_METADATA;
         return false;
      }
   }

   // Processing location
   if (validationTest & VALID_PROCESSING_LOCATION)
   {
      if (isProcessingLocationSupported(pDescriptor->getProcessingLocation()) == false)
      {
         errorMessage = "The specified processing location is not supported.";
         mValidationError = VALID_PROCESSING_LOCATION;
         return false;
      }
   }

   // If no RasterDataDescriptor or RasterFileDescriptor tests are performed, end here
   if (validationTest < RASTER_SIZE)
   {
      return true;
   }

   // Since raster tests have been specified, always validate the raster data descriptor and raster file descriptor
   const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
   if (pRasterDescriptor == NULL)
   {
      errorMessage = "The data set does not contain raster information.";
      return false;
   }

   const RasterFileDescriptor* pRasterFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
   if (pRasterFileDescriptor == NULL)
   {
      errorMessage = "The file does not contain valid raster data.";
      return false;
   }

   // Raster size
   if (validationTest & RASTER_SIZE)
   {
      // Data set size
      unsigned int loadedRows = pRasterDescriptor->getRowCount();
      unsigned int loadedColumns = pRasterDescriptor->getColumnCount();
      unsigned int loadedBands = pRasterDescriptor->getBandCount();

      if ((loadedRows == 0) || (loadedColumns == 0) || (loadedBands == 0))
      {
         errorMessage = "The data set is empty.  Check the size of the rows, columns, and bands.";
         mValidationError = RASTER_SIZE;
         return false;
      }

      // Pixel size
      if (pRasterFileDescriptor->getBitsPerElement() == 0)
      {
         errorMessage = "The number of bits per element is invalid.";
         mValidationError = RASTER_SIZE;
         return false;
      }
   }

   // Data type
   if (validationTest & VALID_DATA_TYPE)
   {
      const std::vector<EncodingType>& dataTypes = pRasterDescriptor->getValidDataTypes();
      if (std::find(dataTypes.begin(), dataTypes.end(), pRasterDescriptor->getDataType()) == dataTypes.end())
      {
         errorMessage = "The data type is not valid for this data set.";
         mValidationError = VALID_DATA_TYPE;
         return false;
      }
   }

   // Header bytes
   if (validationTest & NO_HEADER_BYTES)
   {
      if (pRasterFileDescriptor->getHeaderBytes() > 0)
      {
         errorMessage = "The file has an invalid number of header bytes.";
         mValidationError = NO_HEADER_BYTES;
         return false;
      }
   }

   // Preline and postline bytes
   if (validationTest & NO_PRE_POST_LINE_BYTES)
   {
      if ((pRasterFileDescriptor->getPrelineBytes() > 0) || (pRasterFileDescriptor->getPostlineBytes() > 0))
      {
         errorMessage = "The file has an invalid number of preline and/or postline bytes.";
         mValidationError = NO_PRE_POST_LINE_BYTES;
         return false;
      }
   }

   // Preband and postband bytes
   if (validationTest & NO_PRE_POST_BAND_BYTES)
   {
      if ((pRasterFileDescriptor->getPrebandBytes() > 0) || (pRasterFileDescriptor->getPostbandBytes() > 0))
      {
         errorMessage = "The file has an invalid number of preband and/or postband bytes.";
         mValidationError = NO_PRE_POST_BAND_BYTES;
         return false;
      }
   }

   // Trailer bytes
   if (validationTest & NO_TRAILER_BYTES)
   {
      if (pRasterFileDescriptor->getTrailerBytes() > 0)
      {
         errorMessage = "The file has an invalid number of trailer bytes.";
         mValidationError = NO_TRAILER_BYTES;
         return false;
      }
   }

   // File size
   int64_t requiredSize = RasterUtilities::calculateFileSize(pRasterFileDescriptor);
   if ((validationTest & FILE_SIZE) == FILE_SIZE)
   {
      // Existing file
      LargeFileResource file;
      VERIFY(file.open(filename, O_RDONLY | O_BINARY, S_IREAD) == true);

      // File size
      if (requiredSize < 0)
      {
         errorMessage = "Unable to determine the required file size.";
         mValidationError = FILE_SIZE;
         return false;
      }

      if (file.fileLength() < requiredSize)
      {
         errorMessage = "The size of the file does not match the current parameters.";
         mValidationError = FILE_SIZE;
         return false;
      }
   }

   // Band files
   const vector<const Filename*>& bandFiles = pRasterFileDescriptor->getBandFiles();
   if (validationTest & NO_BAND_FILES)
   {
      if (bandFiles.empty() == false)
      {
         errorMessage = "This data set cannot have band data in multiple files.";
         mValidationError = NO_BAND_FILES;
         return false;
      }
   }

   // Existing band files and band file sizes
   if (validationTest & EXISTING_BAND_FILES)
   {
      // Enough band files for all bands
      unsigned int numBands = pRasterFileDescriptor->getBandCount();
      if (bandFiles.size() < numBands)
      {
         errorMessage = "The number of band files specified is less than the total number of bands to be loaded.";
         mValidationError = EXISTING_BAND_FILES;
         return false;
      }

      // Invalid file for imported bands
      for (vector<const Filename*>::size_type i = 0; i < bandFiles.size(); ++i)
      {
         const Filename* pFilename = bandFiles[i];
         if (pFilename == NULL)
         {
            stringstream streamMessage;
            streamMessage << "Band filename " << i + 1 << " is missing.";
            errorMessage = streamMessage.str();
            mValidationError = EXISTING_BAND_FILES;
            return false;
         }

         // Invalid filename
         string bandFilename = pFilename->getFullPathAndName();
         if (bandFilename.empty() == true)
         {
            stringstream streamMessage;
            streamMessage << "Band filename " << i + 1 << " is invalid.";
            errorMessage = streamMessage.str();
            mValidationError = EXISTING_BAND_FILES;
            return false;
         }

         // Existing file
         LargeFileResource bandFile;
         if (!bandFile.open(bandFilename, O_RDONLY | O_BINARY, S_IREAD))
         {
            stringstream streamMessage;
            streamMessage << "Band file " << i + 1 << " does not exist.";
            errorMessage = streamMessage.str();
            mValidationError = EXISTING_BAND_FILES;
            return false;
         }

         // File size
         if ((validationTest & BAND_FILE_SIZES) == BAND_FILE_SIZES)
         {
            if (requiredSize < 0)
            {
               errorMessage = "Unable to determine the required band file size.";
               mValidationError = BAND_FILE_SIZES;
               return false;
            }

            if (bandFile.fileLength() < requiredSize)
            {
               stringstream streamMessage;
               streamMessage << "The size of band file " << i + 1 << " does not match the required size "
                  "for the current parameters.";
               errorMessage = streamMessage.str();
               mValidationError = BAND_FILE_SIZES;
               return false;
            }
         }
      }
   }

   // Band names
   const DynamicObject* pMetadata = pRasterDescriptor->getMetadata();
   if ((validationTest & VALID_BAND_NAMES) == VALID_BAND_NAMES)
   {
      VERIFY(pMetadata != NULL);

      string namesPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, NAMES_METADATA_NAME, END_METADATA_NAME };

      // If band names are present in the metadata, check the number of names against the number of bands
      // If band names are not present in the metadata, then succeed
      const vector<string>* pBandNames = dv_cast<vector<string> >(&pMetadata->getAttributeByPath(namesPath));
      if (pBandNames != NULL)
      {
         if (pBandNames->size() != pRasterFileDescriptor->getBandCount())
         {
            errorMessage = "The number of band names in the metadata does not match the number of bands.";
            mValidationError = VALID_BAND_NAMES;
            return false;
         }
      }
   }

   // Wavelengths
   if ((validationTest & VALID_WAVELENGTHS) == VALID_WAVELENGTHS)
   {
      VERIFY(pMetadata != NULL);

      // If wavelengths are present in the metadata, check the number of wavelengths against the number of bands
      // If wavelengths are not present in the metadata, then succeed
      FactoryResource<Wavelengths> pWavelengths;
      if (pWavelengths->initializeFromDynamicObject(pMetadata, false) == true)
      {
         if (pWavelengths->getNumWavelengths() != pRasterFileDescriptor->getBandCount())
         {
            errorMessage = "The number of wavelengths in the metadata does not match the number of bands.";
            mValidationError = VALID_WAVELENGTHS;
            return false;
         }
      }
   }

   // Interleave conversions
   if (validationTest & NO_INTERLEAVE_CONVERSIONS)
   {
      InterleaveFormatType dataInterleave = pRasterDescriptor->getInterleaveFormat();
      InterleaveFormatType fileInterleave = pRasterFileDescriptor->getInterleaveFormat();
      if ((pRasterFileDescriptor->getBandCount() > 1) && (dataInterleave != fileInterleave))
      {
         errorMessage = "Interleave format conversions are not supported.";
         mValidationError = NO_INTERLEAVE_CONVERSIONS;
         return false;
      }
   }

   // Skip factors
   if (validationTest & NO_ROW_SKIP_FACTOR)
   {
      if (pRasterDescriptor->getRowSkipFactor() > 0)
      {
         errorMessage = "Row skip factors are not supported.";
         mValidationError = NO_ROW_SKIP_FACTOR;
         return false;
      }
   }

   if (validationTest & NO_COLUMN_SKIP_FACTOR)
   {
      if (pRasterDescriptor->getColumnSkipFactor() > 0)
      {
         errorMessage = "Column skip factors are not supported.";
         mValidationError = NO_COLUMN_SKIP_FACTOR;
         return false;
      }
   }

   // Subsets
   if (validationTest & NO_ROW_SUBSETS)
   {
      if (pRasterDescriptor->getRowCount() != pRasterFileDescriptor->getRowCount())
      {
         errorMessage = "Row subsets are not supported.";
         mValidationError = NO_ROW_SUBSETS;
         return false;
      }
   }

   if (validationTest & NO_COLUMN_SUBSETS)
   {
      if (pRasterDescriptor->getColumnCount() != pRasterFileDescriptor->getColumnCount())
      {
         errorMessage = "Column subsets are not supported.";
         mValidationError = NO_COLUMN_SUBSETS;
         return false;
      }
   }

   if (validationTest & NO_BAND_SUBSETS)
   {
      if (pRasterDescriptor->getBandCount() != pRasterFileDescriptor->getBandCount())
      {
         errorMessage = "Band subsets are not supported.";
         mValidationError = NO_BAND_SUBSETS;
         return false;
      }
   }

   // Available memory
   if (validationTest & AVAILABLE_MEMORY)
   {
      unsigned int loadedRows = pRasterDescriptor->getRowCount();
      unsigned int loadedColumns = pRasterDescriptor->getColumnCount();
      unsigned int loadedBands = pRasterDescriptor->getBandCount();
      unsigned int bytesPerElement = pRasterDescriptor->getBytesPerElement();

      uint64_t dataSize = loadedRows * loadedColumns * loadedBands * bytesPerElement;
      uint64_t maxMemoryAvail = pUtilities->getMaxMemoryBlockSize();
#if PTR_SIZE > 4
      uint64_t totalRam = pUtilities->getTotalPhysicalMemory();
      if (totalRam < maxMemoryAvail)
      {
         maxMemoryAvail = totalRam;
      }
#endif

      if (dataSize > maxMemoryAvail)
      {
         errorMessage = "The data set cannot be loaded into memory.  Use a different "
            "processing location or specify a subset.";
         mValidationError = AVAILABLE_MEMORY;
         return false;
      }
   }

   return true;
}

QWidget* ImporterShell::getImportOptionsWidget(DataDescriptor* pDescriptor)
{
   return NULL;
}

void ImporterShell::polishDataDescriptor(DataDescriptor* pDescriptor)
{
   // Do nothing to the data descriptor
}

void ImporterShell::setExtensions(const string& extensions)
{
   mExtension = extensions;
}

int ImporterShell::getValidationTest(const DataDescriptor* pDescriptor) const
{
   return EXISTING_FILE | NO_EXISTING_DATA_ELEMENT | VALID_PROCESSING_LOCATION;
}

ImporterShell::ValidationTest ImporterShell::getValidationError() const
{
   return mValidationError;
}
