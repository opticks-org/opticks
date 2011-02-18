/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DtedImporter.h"
#include "CachedPager.h"
#include "Classification.h"
#include "Endian.h"
#include "ImportDescriptor.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "TypesFile.h"

#include <string>
using namespace std;


REGISTER_PLUGIN_BASIC(OpticksDTED, DtedImporter);

DtedImporter::DtedImporter()
{
   setName("DTED Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
#if defined(WIN_API)
   setExtensions("DTED Header Files (*.dt?)");
#else
   setExtensions("DTED Header Files (*.dt? *.DT?)");
#endif
   setDescription("Import Digital Terrain Elevation Data (DTED) Files");
   setDescriptorId("{433CC37D-2F29-47e7-8675-CA1129B77C89}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

DtedImporter::~DtedImporter()
{
}

vector<ImportDescriptor*> DtedImporter::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;

   if (filename.empty() == false)
   {
      bool bSuccess = false;

      FILE* pFile = fopen(filename.c_str(), "rb");
      if (pFile != NULL)
      {
         bSuccess = mUhl_h.readHeader(pFile);
         if (bSuccess == true)
         {
            bSuccess = mDsi_h.readHeader(pFile);
         }

         if (bSuccess == true)
         {
            bSuccess = mAcc_h.readHeader(pFile);
         }

         fclose(pFile);
      }

      if (bSuccess == true)
      {
         const EncodingType dataType(INT2UBYTES);
         RasterDataDescriptor* pDescriptor = RasterUtilities::generateRasterDataDescriptor(filename, NULL,
            mUhl_h.getLongCount(), mUhl_h.getLatCount(), 1, BIP, dataType, IN_MEMORY);
         if (pDescriptor != NULL)
         {
            // Data types
            pDescriptor->setValidDataTypes(vector<EncodingType>(1, dataType));

            // Classification
            FactoryResource<Classification> pClassification;
            if (pClassification.get() != NULL)
            {
               string secCode;
               secCode.append(1, mDsi_h.getSecurityCode());
               pClassification->setLevel(secCode);

               pDescriptor->setClassification(pClassification.get());
            }

            // Bad values
            vector<int> badValues;
            badValues.push_back(0);
            pDescriptor->setBadValues(badValues);
            
            RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(
               RasterUtilities::generateAndSetFileDescriptor(pDescriptor, filename, "", BIG_ENDIAN_ORDER));

            if (pFileDescriptor != NULL)
            {
               // Header bytes
               pFileDescriptor->setHeaderBytes(mAcc_h.getTotalHeaderSize());

               // Preline bytes
               pFileDescriptor->setPrelineBytes(8);

               // Postline bytes
               pFileDescriptor->setPostlineBytes(4);

               // GCPs
               list<GcpPoint> gcps;

               GcpPoint upperLeft;
               upperLeft.mPixel.mX = 0.0;
               upperLeft.mPixel.mY = 0.0;
               upperLeft.mCoordinate.mX = mDsi_h.getNWLatCorner();
               upperLeft.mCoordinate.mY = mDsi_h.getNWLongCorner();
               gcps.push_back(upperLeft);

               GcpPoint upperRight;
               upperRight.mPixel.mX = mUhl_h.getLongCount() - 1.0;
               upperRight.mPixel.mY = 0.0;
               upperRight.mCoordinate.mX = mDsi_h.getNELatCorner();
               upperRight.mCoordinate.mY = mDsi_h.getNELongCorner();
               gcps.push_back(upperRight);

               GcpPoint lowerLeft;
               lowerLeft.mPixel.mX = 0.0;
               lowerLeft.mPixel.mY = mUhl_h.getLatCount() - 1.0;
               lowerLeft.mCoordinate.mX = mDsi_h.getSWLatCorner();
               lowerLeft.mCoordinate.mY = mDsi_h.getSWLongCorner();
               gcps.push_back(lowerLeft);

               GcpPoint lowerRight;
               lowerRight.mPixel.mX = mUhl_h.getLongCount() - 1.0;
               lowerRight.mPixel.mY = mUhl_h.getLatCount() - 1.0;
               lowerRight.mCoordinate.mX = mDsi_h.getSELatCorner();
               lowerRight.mCoordinate.mY = mDsi_h.getSELongCorner();
               gcps.push_back(lowerRight);

               GcpPoint center;
               center.mPixel.mX = mUhl_h.getLongCount() / 2.0 - 0.5;
               center.mPixel.mY = mUhl_h.getLatCount() / 2.0 - 0.5;
               center.mCoordinate.mX = (mDsi_h.getNWLatCorner() + mDsi_h.getSWLatCorner()) / 2.0;
               center.mCoordinate.mY = (mDsi_h.getNWLongCorner() + mDsi_h.getNELongCorner()) / 2.0;
               gcps.push_back(center);

               pFileDescriptor->setGcps(gcps);
            }

            Service<ModelServices> pModel;

            ImportDescriptor* pImportDescriptor = pModel->createImportDescriptor(pDescriptor);
            if (pImportDescriptor != NULL)
            {
               descriptors.push_back(pImportDescriptor);
            }
            else
            {
               pModel->destroyDataDescriptor(pDescriptor);
            }
         }
      }
   }

   return descriptors;
}

unsigned char DtedImporter::getFileAffinity(const string& filename)
{
   if (getImportDescriptors(filename).empty())
   {
      return Importer::CAN_NOT_LOAD;
   }
   else
   {
      return Importer::CAN_LOAD;
   }
}

bool DtedImporter::createRasterPager(RasterElement* pRaster) const
{
   VERIFY(pRaster != NULL);
   DataDescriptor *pDescriptor = pRaster->getDataDescriptor();
   VERIFY(pDescriptor != NULL);
   FileDescriptor *pFileDescriptor = pDescriptor->getFileDescriptor();
   VERIFY(pFileDescriptor != NULL);

   string filename = pRaster->getFilename();
   Progress *pProgress = getProgress();

   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(filename);

   ExecutableResource pagerPlugIn("GDAL Raster Pager", string(), pProgress);
   pagerPlugIn->getInArgList().setPlugInArgValue(CachedPager::PagedElementArg(), pRaster);
   pagerPlugIn->getInArgList().setPlugInArgValue(CachedPager::PagedFilenameArg(), pFilename.get());

   bool success = pagerPlugIn->execute();

   RasterPager *pPager = dynamic_cast<RasterPager*>(pagerPlugIn->getPlugIn());
   if (!success || pPager == NULL)
   {
      string message = "Execution of GDAL Raster Pager failed!";
      if (pProgress != NULL) pProgress->updateProgress(message, 0, ERRORS);
      return false;
   }

   pRaster->setPager(pPager);
   pagerPlugIn->releasePlugIn();

   return true;
}

