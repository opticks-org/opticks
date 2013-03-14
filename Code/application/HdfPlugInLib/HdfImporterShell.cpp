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
#include "Filename.h"
#include "HdfImporterShell.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "UtilityServices.h"

using namespace std;

HdfImporterShell::HdfImporterShell()
{
   setName("HDF Importer");
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("Hierarchical Data Format (HDF) Base Class");
   allowMultipleInstances(true);
   executeOnStartup(false);
   addDependencyCopyright("SZIP", Service<UtilityServices>()->getTextFromFile(":/licenses/szip"));
}

HdfImporterShell::~HdfImporterShell()
{}

bool HdfImporterShell::validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const
{
   bool isValid = RasterElementImporterShell::validate(pDescriptor, errorMessage);
   if (isValid == false)
   {
      ValidationTest errorTest = getValidationError();
      if (errorTest == NO_BAND_FILES)
      {
         const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
         VERIFY(pRasterDescriptor != NULL);

         if (pRasterDescriptor->getInterleaveFormat() == BSQ)
         {
            errorMessage += "  Bands in multiple files are not supported with on-disk read-only processing.";
         }
      }
      else if (errorTest == NO_BAND_SUBSETS)
      {
         errorMessage = errorMessage.substr(0, errorMessage.length() - 1);
         errorMessage += " for interleave formats other than BSQ.";
      }
      else if ((errorTest == NO_ROW_SUBSETS) || (errorTest == NO_COLUMN_SUBSETS))
      {
         errorMessage = errorMessage.substr(0, errorMessage.length() - 1);
         errorMessage += " with on-disk read-only processing for interleave formats other than BSQ.";
      }
   }

   return isValid;
}

int HdfImporterShell::getValidationTest(const DataDescriptor* pDescriptor) const
{
   int validationTest = RasterElementImporterShell::getValidationTest(pDescriptor);
   if (pDescriptor != NULL)
   {
      if (pDescriptor->getProcessingLocation() == ON_DISK_READ_ONLY)
      {
         validationTest |= NO_BAND_FILES;

         const RasterFileDescriptor* pFileDescriptor = dynamic_cast<const RasterFileDescriptor*>(
            pDescriptor->getFileDescriptor());
         if (pFileDescriptor != NULL)
         {
            if (pFileDescriptor->getInterleaveFormat() == BSQ)
            {
               // Disabling these checks since the importer supports BSQ subsets and skip factors for on-disk read-only
               validationTest &= ~NO_BAND_SUBSETS;
               validationTest &= ~NO_ROW_SUBSETS;
               validationTest &= ~NO_COLUMN_SUBSETS;
               validationTest &= ~NO_ROW_SKIP_FACTOR;
               validationTest &= ~NO_COLUMN_SKIP_FACTOR;
            }
         }
      }
   }

   return validationTest;
}

bool HdfImporterShell::createRasterPagerPlugIn(const string& pagerName, RasterElement& raster) const
{
   const DataDescriptor* pDescriptor = raster.getDataDescriptor();
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
