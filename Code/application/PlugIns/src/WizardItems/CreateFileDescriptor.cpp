/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CreateFileDescriptor.h"
#include "ApplicationServices.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "DimensionDescriptor.h"
#include "Endian.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "MessageLogResource.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "Units.h"

using namespace std;

//////////////////////////
// CreateFileDescriptor //
//////////////////////////

CreateFileDescriptor::CreateFileDescriptor() :
   mpFilename(NULL),
   mEndianType(Endian::getSystemEndian())
{
   setName("Create File Descriptor");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Creates a file descriptor object");
   setDescriptorId("{4CC2A959-2EB5-44a4-83FF-BA1149D512B1}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

CreateFileDescriptor::~CreateFileDescriptor()
{
}

bool CreateFileDescriptor::getInputSpecification(PlugInArgList*& pArgList)
{
   bool bSuccess = ModelItems::getInputSpecification(pArgList);
   if (bSuccess == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);

   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   // Add args
   PlugInArg* pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Filename");                // Filename
   pArg->setType("Filename");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Data Set Location");       // Data set location
   pArg->setType("string");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Endian");                  // Endian
   pArg->setType("EndianType");
   pArg->setDefaultValue(&mEndianType);
   pArgList->addArg(*pArg);

   return true;
}

bool CreateFileDescriptor::getOutputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   // Set up list
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   // Add args
   PlugInArg* pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("File Descriptor");         // File descriptor
   pArg->setType("FileDescriptor");
   pArgList->addArg(*pArg);

   return true;
}

bool CreateFileDescriptor::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "9593537F-427E-4336-9530-F28C7C233ED7");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   VERIFY(mpFilename != NULL);

   // Create the file descriptor
   FileDescriptor* pFileDescriptor = createFileDescriptor();
   if (pFileDescriptor == NULL)
   {
      reportError("Could not create the file descriptor!", "C7BEEEF9-9398-4776-937D-266C924ECBD4");
      return false;
   }

   // Set the values in the file descriptor
   if (populateFileDescriptor(pFileDescriptor) == false)
   {
      reportError("Could not set the values in the file descriptor!", "21701817-C9C9-4B37-B9A3-4E2FC9EB9A65");
      return false;
   }

   // Set the output arg value
   if (pOutArgList != NULL)
   {
      PlugInArg* pArg = NULL;

      // File descriptor
      if (pOutArgList->getArg("File Descriptor", pArg) && (pArg != NULL))
      {
         pArg->setActualValue(pFileDescriptor);
      }
      else
      {
         // Destroy the file descriptor
         Service<ApplicationServices> pApp;
         if (pApp.get() != NULL)
         {
            ObjectFactory* pObjFact = pApp->getObjectFactory();
            if (pObjFact != NULL)
            {
               pObjFact->destroyObject(pFileDescriptor, "FileDescriptor");
            }
         }

         reportError("Could not set the file descriptor output value!", "BD70F32F-316F-4FC2-A7A4-D933300E6A55");
         return false;
      }
   }

   reportComplete();
   return true;
}

bool CreateFileDescriptor::extractInputArgs(PlugInArgList* pInArgList)
{
   bool bSuccess = ModelItems::extractInputArgs(pInArgList);
   if (bSuccess == false)
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Filename
   if ((pInArgList->getArg("Filename", pArg) == false) || (pArg == NULL))
   {
      reportError("Could not read the filename input value!", "A2C87E1B-066C-4B19-8356-1D24B9E14CC1");
      return false;
   }

   mpFilename = pArg->getPlugInArgValue<Filename>();
   if (mpFilename == NULL)
   {
      reportError("The filename input value is invalid!", "A6FC587E-5EF2-4A67-B753-1ADFABEC6856");
      return false;
   }

   // Data set location
   mDatasetLocation.erase();

   if ((pInArgList->getArg("Data Set Location", pArg) == true) && (pArg != NULL))
   {
      string* pDatasetLocation = pArg->getPlugInArgValue<string>();
      if (pDatasetLocation != NULL)
      {
         mDatasetLocation = *pDatasetLocation;
      }
   }

   // Endian
   if ((pInArgList->getArg("Endian", pArg) == true) && (pArg != NULL))
   {
      EndianType* pEndianType = pArg->getPlugInArgValue<EndianType>();
      if (pEndianType != NULL)
      {
         mEndianType = *pEndianType;
      }
   }

   return true;
}

FileDescriptor* CreateFileDescriptor::createFileDescriptor() const
{
   FactoryResource<FileDescriptor> pFileDescriptor;
   VERIFY(pFileDescriptor.get() != NULL);

   return pFileDescriptor.release();
}

bool CreateFileDescriptor::populateFileDescriptor(FileDescriptor* pFileDescriptor) const
{
   if (pFileDescriptor == NULL)
   {
      return false;
   }

   if (mpFilename != NULL)
   {
      pFileDescriptor->setFilename(*mpFilename);
   }

   pFileDescriptor->setDatasetLocation(mDatasetLocation);
   pFileDescriptor->setEndian(mEndianType);

   return true;
}

/////////////////////////////////////
// CreateRasterFileDescriptor //
/////////////////////////////////////

CreateRasterFileDescriptor::CreateRasterFileDescriptor() :
   mpHeaderBytes(NULL),
   mpTrailerBytes(NULL),
   mpPrelineBytes(NULL),
   mpPostlineBytes(NULL),
   mpPrebandBytes(NULL),
   mpPostbandBytes(NULL),
   mBitsPerElement(0),
   mNumRows(0),
   mNumColumns(0),
   mNumBands(0),
   mpPixelSizeX(NULL),
   mpPixelSizeY(NULL),
   mpUnitsName(NULL),
   mpUnitsType(NULL),
   mpUnitsScale(NULL),
   mpUnitsRangeMin(NULL),
   mpUnitsRangeMax(NULL),
   mInterleave(BIP),
   mpBandFiles(NULL)
{
   setName("Create Raster File Descriptor");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Creates a file descriptor object for a raster element");
   setDescriptorId("{EC235FE7-BFE3-44ae-B442-D1C35B9B9DD9}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

CreateRasterFileDescriptor::~CreateRasterFileDescriptor()
{
}

bool CreateRasterFileDescriptor::getInputSpecification(PlugInArgList*& pArgList)
{
   bool bSuccess = CreateFileDescriptor::getInputSpecification(pArgList);
   if (bSuccess == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);

   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   // Add args
   PlugInArg* pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Header Bytes");               // Header bytes
   pArg->setType("unsigned int");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Trailer Bytes");              // Trailer bytes
   pArg->setType("unsigned int");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Preline Bytes");              // Preline bytes
   pArg->setType("unsigned int");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Postline Bytes");             // Postline bytes
   pArg->setType("unsigned int");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Preband Bytes");              // Preband bytes
   pArg->setType("unsigned int");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Postband Bytes");             // Postband bytes
   pArg->setType("unsigned int");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Bits Per Element");           // Bits per element
   pArg->setType("unsigned int");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Number of Rows");             // Number of rows
   pArg->setType("unsigned int");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Number of Columns");          // Number of columns
   pArg->setType("unsigned int");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Number of Bands");            // Number of bands
   pArg->setType("unsigned int");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("X Pixel Size");               // X pixel size
   pArg->setType("double");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Y Pixel Size");               // Y pixel size
   pArg->setType("double");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Units Name");                 // Units name
   pArg->setType("string");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Units Type");                 // Units type
   pArg->setType("UnitType");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Units Scale Factor");         // Units scale factor
   pArg->setType("double");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Units Range Minimum");        // Units range min
   pArg->setType("double");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Units Range Maximum");        // Units range max
   pArg->setType("double");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Interleave Format");          // Interleave format
   pArg->setType("InterleaveFormatType");
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Band Files");                 // Band files
   pArg->setType("vector<Filename>");
   pArgList->addArg(*pArg);


   return true;
}

bool CreateRasterFileDescriptor::getOutputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   // Set up list
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   // Add args
   PlugInArg* pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("File Descriptor");            // File descriptor
   pArg->setType("RasterFileDescriptor");
   pArgList->addArg(*pArg);

   return true;
}

bool CreateRasterFileDescriptor::extractInputArgs(PlugInArgList* pInArgList)
{
   bool bSuccess = CreateFileDescriptor::extractInputArgs(pInArgList);
   if (bSuccess == false)
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Header bytes
   if ((pInArgList->getArg("Header Bytes", pArg) == true) && (pArg != NULL))
   {
      mpHeaderBytes = pArg->getPlugInArgValue<unsigned int>();
   }

   // Trailer bytes
   if ((pInArgList->getArg("Trailer Bytes", pArg) == true) && (pArg != NULL))
   {
      mpTrailerBytes = pArg->getPlugInArgValue<unsigned int>();
   }

   // Preline bytes
   if ((pInArgList->getArg("Preline Bytes", pArg) == true) && (pArg != NULL))
   {
      mpPrelineBytes = pArg->getPlugInArgValue<unsigned int>();
   }

   // Postline bytes
   if ((pInArgList->getArg("Postline Bytes", pArg) == true) && (pArg != NULL))
   {
      mpPostlineBytes = pArg->getPlugInArgValue<unsigned int>();
   }

   // Preband bytes
   if ((pInArgList->getArg("Preband Bytes", pArg) == true) && (pArg != NULL))
   {
      mpPrebandBytes = pArg->getPlugInArgValue<unsigned int>();
   }

   // Postband bytes
   if ((pInArgList->getArg("Postband Bytes", pArg) == true) && (pArg != NULL))
   {
      mpPostbandBytes = pArg->getPlugInArgValue<unsigned int>();
   }

   // Bits per element
   if ((pInArgList->getArg("Bits Per Element", pArg) == false) || (pArg == NULL))
   {
      reportError("Could not read the bits per element input value!", "72D9B492-3CFB-4556-9D06-5CC4D253DF09");
      return false;
   }

   unsigned int* pBitsPerElement = pArg->getPlugInArgValue<unsigned int>();
   if (pBitsPerElement == NULL)
   {
      reportError("The bits per element input value is invalid!", "CFF12035-D572-4AC9-87B0-8A4EAAF94391");
      return false;
   }

   mBitsPerElement = *pBitsPerElement;

   // Number of rows
   if ((pInArgList->getArg("Number of Rows", pArg) == false) || (pArg == NULL))
   {
      reportError("Could not read the number of rows input value!", "45B2D7D2-945E-4119-981D-4B77552B9BA7");
      return false;
   }

   unsigned int* pNumRows = pArg->getPlugInArgValue<unsigned int>();
   if (pNumRows == NULL)
   {
      reportError("The number of rows input value is invalid!", "7643F54B-534B-4D62-A4B1-F9C034572E5C");
      return false;
   }

   mNumRows = *pNumRows;

   // Number of columns
   if ((pInArgList->getArg("Number of Columns", pArg) == false) || (pArg == NULL))
   {
      reportError("Could not read the number of columns input value!", "DAA1D146-995F-487F-AFBB-9A876C639D70");
      return false;
   }

   unsigned int* pNumColumns = pArg->getPlugInArgValue<unsigned int>();
   if (pNumColumns == NULL)
   {
      reportError("The number of columns input value is invalid!", "D94943F4-7AA4-4F52-AEE0-943D8844766B");
      return false;
   }

   mNumColumns = *pNumColumns;

   // Number of bands
   if ((pInArgList->getArg("Number of Bands", pArg) == false) || (pArg == NULL))
   {
      reportError("Could not read the number of bands input value!", "967EDF3B-C3D0-40E4-ACF9-B41D3C1B3468");
      return false;
   }

   unsigned int* pNumBands = pArg->getPlugInArgValue<unsigned int>();
   if (pNumBands == NULL)
   {
      reportError("The number of bands input value is invalid!", "383ACACE-A03F-40CB-B84E-B383C47FA747");
      return false;
   }

   mNumBands = *pNumBands;

   // X pixel size
   if ((pInArgList->getArg("X Pixel Size", pArg) == true) && (pArg != NULL))
   {
      mpPixelSizeX = pArg->getPlugInArgValue<double>();
   }

   // Y pixel size
   if ((pInArgList->getArg("Y Pixel Size", pArg) == true) && (pArg != NULL))
   {
      mpPixelSizeY = pArg->getPlugInArgValue<double>();
   }

   // Units name
   if ((pInArgList->getArg("Units Name", pArg) == true) && (pArg != NULL))
   {
      mpUnitsName = pArg->getPlugInArgValue<string>();
   }

   // Units type
   if ((pInArgList->getArg("Units Type", pArg) == true) && (pArg != NULL))
   {
      mpUnitsType = pArg->getPlugInArgValue<UnitType>();
   }

   // Units scale factor
   if ((pInArgList->getArg("Units Scale Factor", pArg) == true) && (pArg != NULL))
   {
      mpUnitsScale = pArg->getPlugInArgValue<double>();
   }

   // Units range min
   if ((pInArgList->getArg("Units Range Minimum", pArg) == true) && (pArg != NULL))
   {
      mpUnitsRangeMin = pArg->getPlugInArgValue<double>();
   }

   // Units range max
   if ((pInArgList->getArg("Units Range Maximum", pArg) == true) && (pArg != NULL))
   {
      mpUnitsRangeMax = pArg->getPlugInArgValue<double>();
   }

   // Interleave format
   if ((pInArgList->getArg("Interleave Format", pArg) == false) || (pArg == NULL))
   {
      reportError("Could not read the interleave format input value!", "D5E0AF10-9BB8-4A66-8D51-ED64AC1C86F2");
      return false;
   }

   InterleaveFormatType* pInterleave = pArg->getPlugInArgValue<InterleaveFormatType>();
   if (pInterleave == NULL)
   {
      reportError("The interleave format input value is invalid!", "782FB002-7E8D-4C70-A25A-4C51486FEBDF");
      return false;
   }

   mInterleave = *pInterleave;

   // Band files
   if ((pInArgList->getArg("Band Files", pArg) == true) && (pArg != NULL))
   {
      mpBandFiles = pArg->getPlugInArgValue<vector<Filename*> >();
   }

   return true;
}

FileDescriptor* CreateRasterFileDescriptor::createFileDescriptor() const
{
   FactoryResource<RasterFileDescriptor> pFileDescriptor;
   VERIFY(pFileDescriptor.get() != NULL);

   return pFileDescriptor.release();
}

bool CreateRasterFileDescriptor::populateFileDescriptor(FileDescriptor* pFileDescriptor) const
{
   if (CreateFileDescriptor::populateFileDescriptor(pFileDescriptor) == false)
   {
      return false;
   }

   RasterFileDescriptor* pRasterFileDescriptor = dynamic_cast<RasterFileDescriptor*>(pFileDescriptor);
   if (pRasterFileDescriptor == NULL)
   {
      return false;
   }

   // Header bytes
   if (mpHeaderBytes != NULL)
   {
      pRasterFileDescriptor->setHeaderBytes(*mpHeaderBytes);
   }

   // Trailer bytes
   if (mpTrailerBytes != NULL)
   {
      pRasterFileDescriptor->setTrailerBytes(*mpTrailerBytes);
   }

   // Preline bytes
   if (mpPrelineBytes != NULL)
   {
      pRasterFileDescriptor->setPrelineBytes(*mpPrelineBytes);
   }

   // Postline bytes
   if (mpPostlineBytes != NULL)
   {
      pRasterFileDescriptor->setPostlineBytes(*mpPostlineBytes);
   }

   // Preband bytes
   if (mpPrebandBytes != NULL)
   {
      pRasterFileDescriptor->setPrebandBytes(*mpPrebandBytes);
   }

   // Postband bytes
   if (mpPostbandBytes != NULL)
   {
      pRasterFileDescriptor->setPostbandBytes(*mpPostbandBytes);
   }

   // Bits per element
   pRasterFileDescriptor->setBitsPerElement(mBitsPerElement);

   // Rows
   vector<DimensionDescriptor> rows = RasterUtilities::generateDimensionVector(mNumRows, true, false, true);
   pRasterFileDescriptor->setRows(rows);

   // Columns
   vector<DimensionDescriptor> columns = RasterUtilities::generateDimensionVector(mNumColumns, true, false, true);
   pRasterFileDescriptor->setColumns(columns);

   // Bands
   vector<DimensionDescriptor> bands = RasterUtilities::generateDimensionVector(mNumBands, true, false, true);
   pRasterFileDescriptor->setBands(bands);

   // X pixel size
   if (mpPixelSizeX != NULL)
   {
      pRasterFileDescriptor->setXPixelSize(*mpPixelSizeX);
   }

   // Y pixel size
   if (mpPixelSizeY != NULL)
   {
      pRasterFileDescriptor->setYPixelSize(*mpPixelSizeY);
   }

   // Units
   if ((mpUnitsName != NULL) || (mpUnitsType != NULL) || (mpUnitsScale != NULL) ||
      (mpUnitsRangeMin != NULL) || (mpUnitsRangeMax != NULL))
   {
      const Units* pOrigUnits = pRasterFileDescriptor->getUnits();

      FactoryResource<Units> pUnits;
      VERIFY(pUnits.get() != NULL);

      // Name
      if (mpUnitsName != NULL)
      {
         pUnits->setUnitName(*mpUnitsName);
      }
      else if (pOrigUnits != NULL)
      {
         pUnits->setUnitName(pOrigUnits->getUnitName());
      }

      // Type
      if (mpUnitsType != NULL)
      {
         pUnits->setUnitType(*mpUnitsType);
      }
      else if (pOrigUnits != NULL)
      {
         pUnits->setUnitType(pOrigUnits->getUnitType());
      }

      // Scale
      if (mpUnitsScale != NULL)
      {
         pUnits->setScaleFromStandard(*mpUnitsScale);
      }
      else if (pOrigUnits != NULL)
      {
         pUnits->setScaleFromStandard(pOrigUnits->getScaleFromStandard());
      }

      // Range minimum
      if (mpUnitsRangeMin != NULL)
      {
         pUnits->setRangeMin(*mpUnitsRangeMin);
      }
      else if (pOrigUnits != NULL)
      {
         pUnits->setRangeMin(pOrigUnits->getRangeMin());
      }

      // Range maximum
      if (mpUnitsRangeMax != NULL)
      {
         pUnits->setRangeMax(*mpUnitsRangeMax);
      }
      else if (pOrigUnits != NULL)
      {
         pUnits->setRangeMax(pOrigUnits->getRangeMax());
      }

      pRasterFileDescriptor->setUnits(pUnits.get());
   }

   // Interleave format
   pRasterFileDescriptor->setInterleaveFormat(mInterleave);

   // Band files
   if (mpBandFiles != NULL)
   {
      vector<const Filename*> bandFiles;

      vector<Filename*>::const_iterator iter;
      for (iter = mpBandFiles->begin(); iter != mpBandFiles->end(); ++iter)
      {
         const Filename* pFilename = *iter;
         if (pFilename != NULL)
         {
            bandFiles.push_back(pFilename);
         }
      }

      pRasterFileDescriptor->setBandFiles(bandFiles);
   }

   return true;
}

