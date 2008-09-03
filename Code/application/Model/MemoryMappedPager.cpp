/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "DataRequest.h"
#include "DimensionDescriptor.h"
#include "Endian.h"
#include "EndianSwapPage.h"
#include "Filename.h"
#include "MemoryMappedPage.h"
#include "MemoryMappedPager.h"
#include "MemoryMappedMatrix.h"
#include "MemoryMappedMatrixView.h"
#include "DMutex.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"

#include <algorithm>
using namespace std;

MemoryMappedPager::MemoryMappedPager() :
   mbUseDataDescriptor(true),
   mpDataDescriptor(NULL),
   mSwapEndian(false),
   mWritable(false)
{
   setName("MemoryMappedPager");
   setCopyright("Copyright (2005) by Ball Aerospace & Technologies Corp.");
   setCreator("Ball Aerospace & Technologies Corp.");
   setDescription("Provides access to on disk data via os-level memory mapping functionality");
   setDescriptorId("{894F234C-7DAD-485c-8E62-1F1C78F1E781}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setShortDescription("Memory maps on disk data");
}

namespace {
   class MemoryMappedMatrixDeleter
   {
   public:
      void operator()(MemoryMappedMatrix *pMatrix)
      {
         delete pMatrix;
      }
   };
}

MemoryMappedPager::~MemoryMappedPager()
{
   //destroy and RasterPager objects that we
   //leased out via the getPage() method
   //but were not returned to us via the releasePage()
   //method
   map<MemoryMappedPage*,MemoryMappedMatrix*>::iterator iterPages, endPages;
   MemoryMappedPage* pCurPage;
   MemoryMappedMatrix *pCurMatrix;
   iterPages = mCurrentlyLeasedPages.begin();
   endPages = mCurrentlyLeasedPages.end();

   while (iterPages != endPages)
   {
      pCurPage = iterPages->first;
      pCurMatrix = iterPages->second;
      //destroy the memory mapped section of the file
      //associated with that Page
      pCurMatrix->release( pCurPage->getMemoryMappedMatrixView() );

      //delete the actual page that we allocated earlier
      //in the getPage() method.
      delete pCurPage;

      iterPages++;
   }
   mCurrentlyLeasedPages.clear();

   for_each(mMatrices.begin(), mMatrices.end(), MemoryMappedMatrixDeleter());
}

bool MemoryMappedPager::getInputSpecification(PlugInArgList *&argList)
{
   Service<PlugInManagerServices> pServices;
   VERIFY(pServices.get() != NULL);

   argList = pServices->getPlugInArgList();
   VERIFY(argList != NULL);

   PlugInArg* pArg = pServices->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Raster Element");
   pArg->setType("RasterElement");
   pArg->setDescription("The RasterElement which this pager is associated. "
      "If the \"Data Descriptor\" argument is not NULL, this argument is ignored.");
   pArg->setDefaultValue(NULL);
   argList->addArg(*pArg);

   pArg = pServices->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Data Descriptor");
   pArg->setType("RasterDataDescriptor");
   pArg->setDescription("The RasterDataDescriptor which this pager is associated. "
      "If this argument is not NULL, it overrides the \"Raster Element\" argument. This argument "
      "is used by other \"conversion\" pagers which wish to access data in a different format than "
      "the RasterElement's DataDescriptor specifies. An example is a pager which loads data which is "
      "100 rows by 100 columns on disk and up samples the data to 200x200. The RasterElement's DataDescriptor "
      "will specify 200x200. The \"Data Descriptor\" argument will specify 100x100 and the resulting pages will "
      "be resampled by the hypothetical pager.");
   pArg->setDefaultValue(NULL);
   argList->addArg(*pArg);

   pArg = pServices->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Filename");
   pArg->setType("Filename");
   pArg->setDefaultValue(NULL);
   argList->addArg(*pArg);

   pArg = pServices->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("isWritable");
   pArg->setType("bool");
   pArg->setDefaultValue(NULL);
   argList->addArg(*pArg);

   pArg = pServices->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Use Data Descriptor");
   pArg->setType("bool");
   pArg->setDefaultValue(&mbUseDataDescriptor);
   argList->addArg(*pArg);

   return true;
}

bool MemoryMappedPager::getOutputSpecification(PlugInArgList *&argList)
{
   //----- this plugin has no outputs arguments.
   argList = NULL;

   return true;
}

bool MemoryMappedPager::execute(PlugInArgList *pInputArgList, 
                                         PlugInArgList *pOutputArgList)
{
   PlugInArg *pArg = NULL;

   VERIFY((mMatrices.empty()) && (mpDataDescriptor == NULL) && (pInputArgList != NULL));

   //Get PlugIn Arguments

   //Get RasterElement argument or the data descriptor
   VERIFY(pInputArgList->getArg("Raster Element", pArg) && (pArg != NULL));
   RasterElement *pRaster = pArg->getPlugInArgValue<RasterElement>();
   VERIFY(pInputArgList->getArg("Data Descriptor", pArg) && (pArg != NULL));
   const RasterDataDescriptor *pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pArg->getPlugInArgValueUnsafe<DataDescriptor>());
   VERIFY(pRaster != NULL || pDescriptor != NULL);

   //Get Filename argument
   Filename* pFilename = NULL;
   VERIFY(pInputArgList->getArg("Filename", pArg) && (pArg != NULL));

   pFilename = pArg->getPlugInArgValue<Filename>();
   VERIFY(pFilename != NULL);

   //Get isWritable argument
   bool* pIsWritable = NULL;
   VERIFY(pInputArgList->getArg("isWritable", pArg) && (pArg != NULL));

   pIsWritable = pArg->getPlugInArgValue<bool>();
   VERIFY(pIsWritable != NULL);
   mWritable = *pIsWritable;

   //Get Use Data Descriptor argument
   bool* pUseDataDescriptor = NULL;
   VERIFY(pInputArgList->getArg("Use Data Descriptor", pArg) && (pArg != NULL));

   pUseDataDescriptor = pArg->getPlugInArgValue<bool>();
   VERIFY(pUseDataDescriptor != NULL);
   mbUseDataDescriptor = *pUseDataDescriptor;
   //Done getting PlugIn Arguments
   
   if(pDescriptor == NULL)
   {
      pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   }
   mpDataDescriptor = pDescriptor;

   if (mpDataDescriptor == NULL)
   {
      return false;
   }

   try
   {
      if (mbUseDataDescriptor == true)
      {
         mMatrices.push_back(new MemoryMappedMatrix(pFilename->getFullPathAndName(),
            0,
            pDescriptor->getInterleaveFormat(),
            pDescriptor->getBytesPerElement(),
            pDescriptor->getRowCount(),
            pDescriptor->getColumnCount(),
            pDescriptor->getBandCount(),
            0,
            0,
            !mWritable));
      }
      else
      {
         const RasterFileDescriptor* pFileDescriptor =
            dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
         VERIFY(pFileDescriptor != NULL);

         EndianType srcEndian = pFileDescriptor->getEndian();
         mSwapEndian = (srcEndian != Endian::getSystemEndian() 
            && pDescriptor->getBytesPerElement() > 1);

         const std::vector<const Filename*>& bandFiles = pFileDescriptor->getBandFiles();
         if (!mWritable && !bandFiles.empty())
         {
            // if the accessor is on-disk read-only and in multiple files, create
            // a matrix for each file, opening them read-only
            VERIFY(bandFiles.size() == pFileDescriptor->getBandCount());
            for (vector<const Filename*>::const_iterator iter = bandFiles.begin();
               iter != bandFiles.end(); ++iter)
            {
               const Filename *pFilename = *iter;
               VERIFY(pFilename != NULL);
               mMatrices.push_back(new MemoryMappedMatrix(pFilename->getFullPathAndName(),
                  pFileDescriptor->getHeaderBytes() + pFileDescriptor->getPrelineBytes() + pFileDescriptor->getPrebandBytes(),
                  pFileDescriptor->getInterleaveFormat(),
                  pDescriptor->getBytesPerElement(),
                  pFileDescriptor->getRowCount(),
                  pFileDescriptor->getColumnCount(),
                  1,
                  pFileDescriptor->getPostlineBytes() + pFileDescriptor->getPrelineBytes(),
                  pFileDescriptor->getPostbandBytes() + pFileDescriptor->getPrebandBytes(),
                  true));
            }
         }
         else
         {
            // get here in three conditions:
            //  1) if the accessor is in multiple files and writable, a copy has been made,
            //     create a matrix for the created temporary file
            //  2) the accessor is for a single file, and is read only.  getFullPathAndName()
            //     will return the original filename, and it should be opened read only (!isWritable() == true)
            //  3) the accessor is for a single file, and is read-write.  getFullPathAndName()
            //     will return a temporary filename, which should be opened read-write (!isWritable() == false)
            mMatrices.push_back(new MemoryMappedMatrix(pFilename->getFullPathAndName(),
               pFileDescriptor->getHeaderBytes() + pFileDescriptor->getPrelineBytes() + pFileDescriptor->getPrebandBytes(),
               pFileDescriptor->getInterleaveFormat(),
               pDescriptor->getBytesPerElement(),
               pFileDescriptor->getRowCount(),
               pFileDescriptor->getColumnCount(),
               pFileDescriptor->getBandCount(),
               pFileDescriptor->getPostlineBytes() + pFileDescriptor->getPrelineBytes(),
               pFileDescriptor->getPostbandBytes() + pFileDescriptor->getPrebandBytes(),
               !mWritable));
         }
      }
   }
   catch (...)
   {
      VERIFY(false);
   } 
   VERIFY(!mMatrices.empty());

   return true;
}

RasterPage* MemoryMappedPager::getPage(DataRequest *pOriginalRequest, 
      DimensionDescriptor startRow,
      DimensionDescriptor startColumn,
      DimensionDescriptor startBand)
{
   //ensure only one thread enters this code at a time
   VERIFYRV((mpDataDescriptor != NULL) && (!mMatrices.empty()) && pOriginalRequest != NULL, NULL);

   unsigned int bandIndex = startBand.getActiveNumber();

   if (pOriginalRequest->getWritable() == true && (mWritable == false || mSwapEndian == true))
   {
      return NULL;
   }

   mta::MutexLock mutex(mMutex);

   InterleaveFormatType interleave;
   unsigned int numBands = 0;
   unsigned int numColumns = 0;
   unsigned int interlineBytes = 0;
   unsigned int offsetRow = 0; /* OFFSET-SUBCUBING */
   unsigned int offsetCol = 0; /* OFFSET-SUBCUBING */

   if (mbUseDataDescriptor == true)
   {
      interleave = mpDataDescriptor->getInterleaveFormat();
      numColumns = mpDataDescriptor->getColumnCount();
      numBands = mpDataDescriptor->getBandCount();
   }
   else
   {
      const RasterFileDescriptor* pFileDescriptor =
         dynamic_cast<const RasterFileDescriptor*>(mpDataDescriptor->getFileDescriptor());
      if (pFileDescriptor == NULL)
      {
         return false;
      }

      interleave = pFileDescriptor->getInterleaveFormat();
      numBands = pFileDescriptor->getBandCount();
      numColumns = pFileDescriptor->getColumnCount();
      interlineBytes = pFileDescriptor->getPostlineBytes() + pFileDescriptor->getPrelineBytes();

      DimensionDescriptor rowDim;
      const vector<DimensionDescriptor>& rows = mpDataDescriptor->getRows();
      if (rows.empty() == false)
      {
         rowDim = rows.front();
      }

      DimensionDescriptor fileRowDim;
      const vector<DimensionDescriptor>& fileRows = pFileDescriptor->getRows();
      if (fileRows.empty() == false)
      {
         fileRowDim = fileRows.front();
      }

      if (rowDim.isValid() && fileRowDim.isValid())
      {
         offsetRow = rowDim.getOnDiskNumber() - fileRowDim.getOnDiskNumber();
      }

      DimensionDescriptor columnDim;
      const vector<DimensionDescriptor>& columns = mpDataDescriptor->getColumns();
      if (columns.empty() == false)
      {
         columnDim = columns.front();
      }

      DimensionDescriptor fileColumnDim;
      const vector<DimensionDescriptor>& fileColumns = pFileDescriptor->getColumns();
      if (fileColumns.empty() == false)
      {
         fileColumnDim = fileColumns.front();
      }

      if (columnDim.isValid() && fileColumnDim.isValid())
      {
         offsetCol = columnDim.getOnDiskNumber() - fileColumnDim.getOnDiskNumber();
      }
   }

   unsigned int bytesPerElement = mpDataDescriptor->getBytesPerElement();
   unsigned long segmentSize = 0;
   unsigned long rowSize = 0;
   unsigned int numRows = 0;
   //determine the size of the segment we need to memory map
   //depending on whether the interleave is BIP or BSQ
   if((interleave == BIP) || (numBands == 1))
   {
      rowSize = (numColumns * numBands * bytesPerElement + interlineBytes);
   }
   else if(interleave == BSQ)
   {
      rowSize = (numColumns * bytesPerElement + interlineBytes);
      if (mMatrices.size() != 1)
      {
         VERIFY(pOriginalRequest->getStopBand().getActiveNumber() < mMatrices.size());
      }
   }
   else if(interleave == BIL)
   {
      rowSize = numColumns * numBands * bytesPerElement;
   }

   unsigned int concurrentRows = pOriginalRequest->getConcurrentRows();
   segmentSize = concurrentRows * rowSize;
   numRows = concurrentRows;

   //get the MemoryMappedMatrixView of a let segmentSize large
   MemoryMappedMatrixView* pView = NULL;
   MemoryMappedMatrix *pMatrix = mMatrices.front();
   if (mMatrices.size() > 1)
   {
      VERIFYRV(bandIndex < mMatrices.size(), NULL);
      pMatrix = mMatrices[bandIndex];
   }
   VERIFYRV(pMatrix != NULL, NULL);
   pView = pMatrix->getView(segmentSize);
   VERIFYRV(pView != NULL, NULL);

   //ask the MemoryMappedMatrixView for a pointer starting
   //at the given location
   if (mMatrices.size() > 1)
   {
      bandIndex = 0;
   }
   char* pRawCubePointer = reinterpret_cast<char*>(pView->getSegment(startRow.getActiveNumber() + offsetRow,
                                                   startColumn.getActiveNumber() + offsetCol, bandIndex));
   if (pRawCubePointer == NULL)
   {
      return NULL;
   }

   //we know have a pointer in raw memory that has
   //been memory mapped, so now create a RasterPage
   //and return it.
   MemoryMappedPage* pPage = new MemoryMappedPage;
   pPage->setRawData(pRawCubePointer);
   pPage->setMemoryMappedMatrixView(pView);
   pPage->setNumRows(numRows);
   pPage->setNumColumns(numColumns);
   pPage->setInterlineBytes(interlineBytes);

   if (mSwapEndian)
   {
      EndianSwapPage *pEndianPage = new EndianSwapPage(pPage->getRawData(), mpDataDescriptor->getDataType(),
                                                       numRows, numColumns, rowSize - interlineBytes, interlineBytes,
                                                       pPage->getMemoryMappedMatrixView()->getEndOfSegment());

      pMatrix->release(pView);
      delete pPage;

      return pEndianPage;
   }

   mCurrentlyLeasedPages[pPage] = pMatrix;

   return pPage;
}

void MemoryMappedPager::releasePage(RasterPage* pPage)
{
   VERIFYNRV(pPage != NULL);

   //ensure only one thread enters this code at a time
   mta::MutexLock mutex(mMutex);

   if (mSwapEndian)
   {
      delete static_cast<EndianSwapPage*>(pPage);
   }
   else
   {
      MemoryMappedPage* pOurPage = static_cast<MemoryMappedPage*>(pPage);

      map<MemoryMappedPage*, MemoryMappedMatrix*>::iterator foundIter;
      foundIter = mCurrentlyLeasedPages.find(pOurPage);
      if(foundIter != mCurrentlyLeasedPages.end())
      {
         //we leased the page out from this instance,
         //so now we can release the resources used for it.
         
         //destroy the memory mapped section of the file
         //associated with that page
         foundIter->second->release(pOurPage->getMemoryMappedMatrixView());

         //remove the page from the vector
         mCurrentlyLeasedPages.erase(foundIter);
         
         //delete the actual page that we allocated earlier
         //in the getPage() method.
         delete pOurPage;
      }
   }
}

int MemoryMappedPager::getSupportedRequestVersion() const
{
   return 1;
}
