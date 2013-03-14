/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"

#include <cstdlib>
#include <hdf5.h>
#include <string>

#include "AppVersion.h"
#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataDescriptor.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "Hdf5Attribute.h"
#include "Hdf5Dataset.h"
#include "Hdf5File.h"
#include "Hdf5Group.h"
#include "Hdf5ImporterShell.h"
#include "Hdf5Pager.h"
#include "Hdf5Resource.h"
#include "Hdf5Utilities.h"
#include "ObjectResource.h"
#include "ProgressTracker.h"
#include "RasterElement.h"
#include "UtilityServices.h"

using namespace std;
using namespace HdfUtilities;

Hdf5ImporterShell::Hdf5ImporterShell()
{
   setExtensions("HDF5 Files (*.h5)");
   addDependencyCopyright("HDF5", Service<UtilityServices>()->getTextFromFile(":/licenses/hdf5"));
}

bool Hdf5ImporterShell::getFileHandle(hid_t& fileHandle, Hdf5FileResource& fileResource) const
{
   fileHandle = -1;

   const RasterElement* pElement = getRasterElement();
   if (pElement != NULL)
   {
      Hdf5PagerFileHandle* pPager = dynamic_cast<Hdf5PagerFileHandle*>(pElement->getPager());
      if (pPager != NULL)
      {
         // Able to retrieve already open file from the pager
         fileHandle = pPager->getFileHandle();
      }
   }

   if (fileHandle < 0)
   {
      if (fileResource.get() == NULL)
      {
         // Not able to retrieve already open file handle; open the file and return.
         fileResource = Hdf5FileResource(pElement->getFilename());
      }

      if (*fileResource >= 0)
      {
         fileHandle = *fileResource;
      }
   }

   return (fileHandle >= 0);
}

bool Hdf5ImporterShell::createRasterPager(RasterElement *pRaster) const
{
   return HdfImporterShell::createRasterPagerPlugIn("Hdf5Pager", *pRaster);
}

