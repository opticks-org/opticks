/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "HdfImporterShell.h"
#include "CachedPager.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "DataAccessorImpl.h"
#include "DimensionDescriptor.h"
#include "Filename.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "ProgressTracker.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"

using namespace std;

HdfImporterShell::HdfImporterShell()
{
   setName("HDF Importer");
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("Hierarchical Data Format (HDF) Base Class");
   allowMultipleInstances(true);
   executeOnStartup(false);
}

HdfImporterShell::~HdfImporterShell()
{
}

bool HdfImporterShell::validateDefaultOnDiskReadOnly(const DataDescriptor* pDescriptor, string& errorMessage) const
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

   ProcessingLocation processingLocation = pRasterDescriptor->getProcessingLocation();
   if (processingLocation == ON_DISK_READ_ONLY)
   {
      // Multiple band files
      const vector<const Filename*>& bandFiles = pFileDescriptor->getBandFiles();
      if (bandFiles.empty() == false)
      {
         errorMessage = "Bands in multiple files are not supported!";
         return false;
      }

      InterleaveFormatType fileInterleave = pFileDescriptor->getInterleaveFormat();
      InterleaveFormatType dataInterleave = pRasterDescriptor->getInterleaveFormat();

      if (dataInterleave != fileInterleave)
      {
         errorMessage = "Interleave format conversions are not supported!";
         return false;
      }

      // Subset
      unsigned int loadedRows = pRasterDescriptor->getRowCount();
      unsigned int loadedColumns = pRasterDescriptor->getColumnCount();
      unsigned int loadedBands = pRasterDescriptor->getBandCount();
      unsigned int fileRows = pFileDescriptor->getRowCount();
      unsigned int fileColumns = pFileDescriptor->getColumnCount();
      unsigned int fileBands = pFileDescriptor->getBandCount();

      if ((loadedRows != fileRows) || (loadedColumns != fileColumns) || (fileInterleave != BSQ && loadedBands != fileBands))
      {
         errorMessage = "Subsets are not supported with on-disk read-only processing!";
         return false;
      }
   }

   return true;
}

bool HdfImporterShell::createRasterPagerPlugIn(const string& pagerName,
                                                  RasterElement& raster) const
{
   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(raster.getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return false;
   }

   const FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
   if (pFileDescriptor == NULL)
   {
      return false;
   }

   const string& hdfName = pFileDescriptor->getDatasetLocation();

   bool bWritable = false;
   if (pDescriptor->getProcessingLocation() == ON_DISK)
   {
      bWritable = true;
   }

   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   PlugIn* pPlugIn = pPlugInManager->createPlugIn(pagerName);
   if (pPlugIn != NULL)
   {
      Executable* pExecutable = dynamic_cast<Executable*>(pPlugIn);
      if (pExecutable != NULL)
      {
         // set up input args
         PlugInArgList* pInArgs = NULL;
         pExecutable->getInputSpecification(pInArgs);
         FactoryResource<Filename> pFilename;
         VERIFY(pFilename.get() != NULL);

         if (pInArgs != NULL)
         {
            // sav removed the following line and replaced it with pFilename->setFullPathAndName(sensor.getFilename());
            pFilename->setFullPathAndName(raster.getFilename());
            PlugInArg* pArg = NULL;
            if (pInArgs->getArg(CachedPager::PagedFilenameArg(), pArg) && pArg != NULL)
            {
               pArg->setActualValue(pFilename.get());
            }

            if (pInArgs->getArg("HDF Name", pArg) && pArg != NULL)
            {
               pArg->setActualValue(&hdfName);
            }

            if (pInArgs->getArg(CachedPager::PagedElementArg(), pArg) && pArg != NULL)
            {
               pArg->setActualValue(&raster);
            }
         }
         else
         {
            pPlugInManager->destroyPlugIn(pPlugIn);
            return false;
         }

         bool success = pExecutable->execute(pInArgs, NULL);

         // whether the PlugIn succeeds or fails, we still need to do this part
         pPlugInManager->destroyPlugInArgList(pInArgs);

         RasterPager* pPager = dynamic_cast<RasterPager*>(pPlugIn);
         if (pPager == NULL || success == false)
         {
            pPlugInManager->destroyPlugIn(pPlugIn);
            return false;
         }
         else
         {
            raster.setPager(pPager);
         }
      }
   }
   else
   {
      return false;
   }

   return true;
}
