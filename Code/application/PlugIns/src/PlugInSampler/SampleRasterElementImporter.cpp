/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "Filename.h"
#include "ImportDescriptor.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SampleRasterElementImporter.h"
#include "TypeConverter.h"

#define NUM_ROWS 10
#define NUM_COLS 5

SampleRasterPage::SampleRasterPage(int row, int col) :
   mRow(row),
   mCol(col)
{
   for (int r = 0; r < NUM_ROWS; ++r)
   {
      for (int c = 0; c < NUM_COLS; ++c)
      {
         mData.push_back(r * NUM_COLS + c);
      }
   }
}

SampleRasterPage::~SampleRasterPage()
{
}

void* SampleRasterPage::getRawData()
{
   return reinterpret_cast<void*>(&mData[mRow * NUM_COLS + mCol]);
}

unsigned int SampleRasterPage::getNumRows()
{
   return NUM_ROWS - mRow;
}

unsigned int SampleRasterPage::getNumColumns()
{
   return NUM_COLS;
}

unsigned int SampleRasterPage::getNumBands()
{
   return 1;
}

unsigned int SampleRasterPage::getInterlineBytes()
{
   return 0;
}
// End of SampleRasterPage

SampleRasterPager::SampleRasterPager()
{
}

SampleRasterPager::~SampleRasterPager()
{
}

RasterPage* SampleRasterPager::getPage(DataRequest* pOriginalRequest,
                                       DimensionDescriptor startRow,
                                       DimensionDescriptor startColumn,
                                       DimensionDescriptor startBand)
{
   return new SampleRasterPage(startRow.getOriginalNumber(), startColumn.getOriginalNumber());
}

void SampleRasterPager::releasePage(RasterPage* pPage)
{
   delete dynamic_cast<SampleRasterPage*>(pPage);
}

int SampleRasterPager::getSupportedRequestVersion() const
{
   return 1;
}
// End of SampleRasterPager

REGISTER_PLUGIN_BASIC(OpticksPlugInSampler, SampleRasterElementImporter);

SampleRasterElementImporter::SampleRasterElementImporter()
{
   setName("Sample RasterElement Importer");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setExtensions("Sample Files (*.sample)");
   setDescription("Import sample data. It's really a static in-memory array that simulates an importer.");
   setSubtype("Sample");
   setDescriptorId("{D1EBFE37-E365-4E49-92B5-B8DE0CA0B03D}");
   setProductionStatus(false);
   setWizardSupported(false);
}

SampleRasterElementImporter::~SampleRasterElementImporter()
{
}

unsigned char SampleRasterElementImporter::getFileAffinity(const std::string& filename)
{
   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(filename);
   if (pFilename->getExtension() == "sample")
   {
      return Importer::CAN_LOAD;
   }
   return Importer::CAN_NOT_LOAD;
} // End getFileAffinity

bool SampleRasterElementImporter::isProcessingLocationSupported(ProcessingLocation location) const
{
   return location == IN_MEMORY;
} // End isProcessingLocationSupported

std::vector<ImportDescriptor*> SampleRasterElementImporter::getImportDescriptors(const std::string& filename)
{
   std::vector<ImportDescriptor*> descriptors;
   ImportDescriptorResource pDesc(filename, TypeConverter::toString<RasterElement>());
   VERIFYRV(pDesc.get() != NULL, descriptors);
   RasterDataDescriptor* pDataDesc = RasterUtilities::generateRasterDataDescriptor(filename, NULL, NUM_ROWS,
      NUM_COLS, INT1UBYTE, IN_MEMORY);
   if (pDataDesc == NULL)
   {
      return descriptors;
   }
   pDesc->setDataDescriptor(pDataDesc);
   if (RasterUtilities::generateAndSetFileDescriptor(pDataDesc, filename, "", LITTLE_ENDIAN_ORDER) == NULL)
   {
      return descriptors;
   }
   descriptors.push_back(pDesc.release());
   return descriptors;
} // End getImportDescriptors

bool SampleRasterElementImporter::createRasterPager(RasterElement* pRaster) const
{
   VERIFY(pRaster != NULL);

   SampleRasterPager* pPager = new SampleRasterPager();
   return pRaster->setPager(pPager);
}
