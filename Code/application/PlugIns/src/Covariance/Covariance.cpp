/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFile>

#include "AoiElement.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "BitMask.h"
#include "BitMaskIterator.h"
#include "DataAccessorImpl.h"
#include "DataDescriptor.h"
#include "DesktopServices.h"
#include "FileResource.h"
#include "MatrixFunctions.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterUtilities.h"
#include "Covariance.h"
#include "CovarianceGui.h"
#include "switchOnEncoding.h"
#include "TypeConverter.h"

#include <algorithm>
#include <string>
#include <vector>
using namespace std;

const string CovarianceAlgorithm::mExpectedFileHeader = "Covariance Matrix File v1.1\n";
static bool** CopySelectedPixels(const bool** pSelectedPixels, int xsize, int ysize);
static void DeleteSelectedPixels(bool** pSelectedPixels);

struct MaskInput
{
   double* pMatrix;
   int numRows;
   int numCols;
   int numBands;
   Progress* pProgress;
   const bool* pAbortFlag;
   const bool** pSelectedPixels;
   int boundingBoxX1;
   int boundingBoxY1;
   int boundingBoxX2;
   int boundingBoxY2;
   const BitMask* mpMask;

};

template<class T>
T* GetRowPtr(T* raw, int numCols, int numBands, int row, int col)
{
   return raw + numBands * (row *numCols + col);
}

template<class T>
void ComputeFactoredCov(T* pData, RasterElement* pRaster, double* pMatrix,
                        Progress* pProgress, const bool* pAbortFlag,
                        int rowFactor, int columnFactor)
{
   if (pRaster == NULL)
   {
      return;
   }
   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   unsigned int numRows = pDescriptor->getRowCount();
   unsigned int numCols = pDescriptor->getColumnCount();
   unsigned int numBands = pDescriptor->getBandCount();
   unsigned int row, col;
   unsigned int band1, band2;
   unsigned int lCount = 0;

   T *pPixel = NULL;
   vector<double> averages(numBands, 0);
   double* pAverage = &averages.front();

   if (rowFactor < 1)
   {
      rowFactor = 1;
   }
   if (columnFactor < 1)
   {
      columnFactor = 1;
   }

   memset(pMatrix, 0, sizeof(double) * numBands * numBands);

   // calculate average spectrum
   float progScale = 100.0f * static_cast<float>(rowFactor)/static_cast<float>(numRows);
   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BIP);

   DataAccessor accessor = pRaster->getDataAccessor(pRequest.release());

   T* pDataColumn(NULL);
   for (row = 0; row < numRows; row += rowFactor)
   {
      if (pProgress != NULL)
      {
         if ((pAbortFlag == NULL) || !(*pAbortFlag))
         {
            pProgress->updateProgress ("Computing Average Signature...", int(progScale*row), NORMAL);
         }
         else
         {
            break;
         }
      }
      for (col = 0; col < numCols; col += columnFactor)
      {
         accessor->toPixel(row, col);
         VERIFYNRV(accessor.isValid());
         ++lCount;
         pPixel = reinterpret_cast<T*>(accessor->getColumn());

         for (band1 = 0; band1 < numBands; ++band1)
         {
            pAverage[band1] += *pPixel;
            ++pPixel;
         }
      }
   }


   // check if aborted
   if ((pAbortFlag == NULL) || !(*pAbortFlag))
   {
      for (band1 = 0; band1 < numBands; ++band1)
      {
         pAverage[band1] /= lCount;
      }

      // compute the covariance
      FactoryResource<DataRequest> pRequest2;
      pRequest2->setInterleaveFormat(BIP);
      accessor = pRaster->getDataAccessor(pRequest2.release());

      for (row = 0; row < numRows; row += rowFactor)
      {
         if (pProgress != NULL)
         {
            if ((pAbortFlag == NULL) || !(*pAbortFlag))
            {
               pProgress->updateProgress("Computing Covariance Matrix...", int(progScale * row), NORMAL);
            }
            else
            {
               break;
            }
         }
         for (col = 0; col < numCols; col += columnFactor)
         {
            accessor->toPixel(row, col);
            VERIFYNRV(accessor.isValid());
            double* pMatrixRow = pMatrix;
            double* pMatrixColumn = NULL;
            pPixel = reinterpret_cast<T*>(accessor->getColumn());

            for (band2 = 0; band2 < numBands; ++band2, pMatrixRow += (numBands + 1))
            {
               pData = pPixel;
               pMatrixColumn = pMatrixRow;
               for (band1 = band2; band1 < numBands; ++band1)
               {
                  *pMatrixColumn += (*pData - pAverage[band1]) * (*pPixel - pAverage[band2]);
                  ++pData;
                  ++pMatrixColumn;
               }
               ++pPixel;
            }
         }
      }
   }

   // check if aborted
   if ((pAbortFlag == NULL) || !(*pAbortFlag))
   {
      for (band2 = 0; band2 < numBands; ++band2)
      {
         for (band1 = band2; band1 < numBands; ++band1)
         {
            pMatrix[band2 * numBands + band1] /= lCount;
         }
      }

      for (band2 = 0; band2 < numBands; ++band2)
      {
         for (band1 = band2 + 1; band1 < numBands; ++band1)
         {
            pMatrix[band1 * numBands + band2] = pMatrix[band2 * numBands + band1];
         }
      }
   }

   if (pProgress != NULL)
   {
      if ((pAbortFlag == NULL) || !(*pAbortFlag))
      {
         pProgress->updateProgress("Covariance Matrix Complete", 100, NORMAL);
      }
      else
      {
         pProgress->updateProgress("Aborted computing Covariance Matrix", 0, ABORT);
      }
   }
}

template<class T>
void ComputeMaskedCvm(T* pData, MaskInput* pInput, RasterElement* pRaster)
{
   if (pRaster == NULL)
   {
      return;
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }
   unsigned int numBands = pDescriptor->getBandCount();
   int lCount = 0;
   T* pPixel = NULL;

   vector<double> averages(numBands, 0);
   double* pAverage = &averages.front();
   memset(pInput->pMatrix, 0, sizeof(double) * pInput->numBands * pInput->numBands);
   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BIP);
   DataAccessor accessor = pRaster->getDataAccessor(pRequest.release());
   BitMaskIterator it(pInput->mpMask, pRaster);
   int numPixels = it.getCount();
   LocationType loc;
   float progScale = 100.0f / numPixels;
   int progSave = 0;
   while (it != it.end())
   {
      it.getPixelLocation(loc);
      accessor->toPixel(loc.mY, loc.mX);
      VERIFYNRV(accessor.isValid());
      if (pInput->pProgress != NULL &&
          progSave != static_cast<int>(progScale * lCount))
      {
         if ((pInput->pAbortFlag == NULL) || !(*pInput->pAbortFlag))
         {
            progSave = static_cast<int>(progScale * lCount);
            pInput->pProgress->updateProgress("Computing Covariance Matrix...",
               progSave, NORMAL);
         }
         else
         {
            break;
         }
      }
      pPixel = reinterpret_cast<T*>(accessor->getColumn());
      ++lCount;
      ++it;
      for (unsigned int band1 = 0; band1 < numBands; ++band1)
      {
         pAverage[band1] += *pPixel;
         ++pPixel;
      }
   }

   for (unsigned int band1 = 0; band1 < numBands; ++band1)
   {
      pAverage[band1] /= lCount;
   }


   // calculate covariance matrix
   accessor = pRaster->getDataAccessor(pRequest->copy());
   it.firstPixel();
   lCount = 0;
   progSave = 0;
   while (it != it.end())
   {
      if (pInput->pProgress != NULL &&
          progSave != static_cast<int>(progScale * lCount))
      {
         if ((pInput->pAbortFlag == NULL) || !(*pInput->pAbortFlag))
         {
            progSave = static_cast<int>(progScale * lCount);
            pInput->pProgress->updateProgress("Computing Covariance Matrix...",
               progSave, NORMAL);
         }
         else
         {
            break;
         }
      }
      it.getPixelLocation(loc);
      accessor->toPixel(loc.mY, loc.mX);

      VERIFYNRV(accessor.isValid());
      double* pMatrixRow = pInput->pMatrix;
      double* pMatrixColumn = NULL;
      pPixel = reinterpret_cast<T*>(accessor->getColumn());

      for (unsigned int band2 = 0; band2 < numBands; ++band2, pMatrixRow += (numBands + 1))
      {
         pData = pPixel;
         pMatrixColumn = pMatrixRow;
         for (unsigned int band1 = band2; band1 < numBands; ++band1)
         {
            *pMatrixColumn += (*pData - pAverage[band1]) * (*pPixel - pAverage[band2]);
            ++pData;
            ++pMatrixColumn;
         }
         ++pPixel;
      }
      ++lCount;
      ++it;
   }

   // check if aborted
   if ((pInput->pAbortFlag == NULL) || !(*pInput->pAbortFlag))
   {
      for (unsigned int band2 = 0; band2 < numBands; ++band2)
      {
         for (unsigned int band1 = band2; band1 < numBands; ++band1)
         {
            pInput->pMatrix[band2 * numBands + band1] /= lCount;
         }
      }

      for (unsigned int band2 = 0; band2 < numBands; ++band2)
      {
         for (unsigned int band1 = band2; band1 < numBands; ++band1)
         {
            pInput->pMatrix[band1 * numBands + band2] = pInput->pMatrix[band2 * numBands + band1];
         }
      }
   }
   if (pInput->pProgress != NULL)
   {
      if ((pInput->pAbortFlag == NULL) || !(*pInput->pAbortFlag))
      {
         pInput->pProgress->updateProgress("Covariance Matrix Complete", 99, NORMAL);
      }
      else
      {
         pInput->pProgress->updateProgress("Aborted computing Covariance Matrix", 0, ABORT);
      }
   }
}

REGISTER_PLUGIN_BASIC(OpticksCovariance, Covariance);

bool Covariance::canRunBatch() const
{
   return true;
}

bool Covariance::canRunInteractive() const
{
   return true;
}

bool Covariance::populateBatchInputArgList(PlugInArgList* pArgList)
{
   static int lRowFactor = 1;
   static int lColumnFactor = 1;

   VERIFY(pArgList != NULL);
   VERIFY(populateInteractiveInputArgList(pArgList));
   VERIFY(pArgList->addArg<int>("Row Factor", &lRowFactor, "Process the first row in a group of this many rows.  "
      "A value of 1 processes all rows. A value of 2 processes 1 row and skips 1 row, etc.  A row is used if its "
      "zero based index modulo the row factor is zero."));
   VERIFY(pArgList->addArg<int>("Column Factor", &lColumnFactor, "Process the first column in a group of this many "
      "columns.  A value of 1 processes all columns. A value of 2 processes 1 column and skips 1 column, etc.  "
      "A column is used if its zero based index modulo the column factor is zero."));
   VERIFY(pArgList->addArg<Filename>("CVM File", NULL, "Store and retrieve Covariance data from this file.  "
      "If NULL, a filename based on the data set filename will be generated."));
   VERIFY(pArgList->addArg<AoiElement>("AOI", NULL, "Only calculate using pixels selected by this AOI.  If NULL, "
      "use the entire image. If row and column factors are not 1, they will also be used when determining data to "
      "process."));

   return true;
}

bool Covariance::populateInteractiveInputArgList(PlugInArgList* pArgList)
{
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<Progress>(ProgressArg(), NULL));
   VERIFY(pArgList->addArg<RasterElement>(DataElementArg(), NULL));
   bool recalc = false;
   VERIFY(pArgList->addArg<bool>("Recalculate", &recalc, "If true, the Covariance matrix will be recalculated.  "
      "If false, the plug-in will decide if the matrix needs to be recalculated. If a matrix exists in memory, "
      "it is used.  If there exists a matrix on disk, it will be loaded. Finally, the matrix will be calculated."));
   bool computeInverse(true);
   VERIFY(pArgList->addArg<bool>("ComputeInverse", &computeInverse, "If true, the inverse of the Covariance "
      "matrix will be calculated. If false, the inverse will not be calculated."));

   return true;
}

bool Covariance::populateDefaultOutputArgList(PlugInArgList* pArgList)
{
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<RasterElement>("Covariance Matrix"));
   VERIFY(pArgList->addArg<RasterElement>("Inverse Covariance Matrix"));
   return true;
}

bool Covariance::parseInputArgList(PlugInArgList* pArgList)
{
   Filename* pCvmFilename = NULL;

   Progress* pProgress = pArgList->getPlugInArgValue<Progress>(ProgressArg());
   if (pProgress == NULL)
   {
      return false;
   }

   mpRasterElement = pArgList->getPlugInArgValue<RasterElement>(DataElementArg());
   if (mpRasterElement == NULL)
   {
      string msg = "The raster element input value is invalid!";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      return false;
   }

   EncodingType eDataType;

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>
      (mpRasterElement->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      string msg = "The sensor data input value is invalid!";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      return false;
   }
   eDataType = pDescriptor->getDataType();

   if ((eDataType == INT4SCOMPLEX) || (eDataType == FLT8COMPLEX))
   {
      string msg = "Complex data is not supported!";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      return false;
   }
   VERIFY(pArgList->getPlugInArgValue("Recalculate", mInput.mRecalculate));
   VERIFY(pArgList->getPlugInArgValue("ComputeInverse", mInput.mComputeInverse));

   if (!isInteractive())
   {
      VERIFY(pArgList->getPlugInArgValue<int>("Row Factor", mInput.mRowFactor));
      VERIFY(pArgList->getPlugInArgValue<int>("Column Factor", mInput.mColumnFactor));
      pCvmFilename = pArgList->getPlugInArgValue<Filename>("CVM File");
      mInput.mpAoi = pArgList->getPlugInArgValue<AoiElement>("AOI");
   }
   mpCovarianceAlg = new CovarianceAlgorithm(mpRasterElement, pProgress, isInteractive());
   mpCovarianceAlg->setFile(pCvmFilename, mLoadIfExists);

   setAlgorithmPattern(Resource<AlgorithmPattern>(mpCovarianceAlg));
   return true;
}

bool Covariance::setActualValuesInOutputArgList(PlugInArgList *pArgList)
{
   VERIFY(mpCovarianceAlg != NULL);
   VERIFY(pArgList != NULL);
   bool success(false);
   RasterElement* pElement = mpCovarianceAlg->getCovarianceElement();
   if (pElement != NULL)
   {
      success = pArgList->setPlugInArgValue<RasterElement>("Covariance Matrix", pElement);
   }

   if (success && mInput.mComputeInverse)
   {
      pElement = mpCovarianceAlg->getInverseCovarianceElement();
      if (pElement != NULL)
      {
         success = pArgList->setPlugInArgValue<RasterElement>("Inverse Covariance Matrix", pElement);
      }
      else
      {
         success = false;
      }
   }

   return success;
}

QDialog* Covariance::getGui(void* pAlgData)
{
   if (isInteractive() && (mpCovarianceAlg != NULL))
   {
      delete mpCovarianceGui; // re-initialize

      bool elementExists = false;
      elementExists |= mpModelServices->getElement("Covariance Matrix",
         TypeConverter::toString<RasterElement>(), mpRasterElement) != NULL;
      elementExists |= mpModelServices->getElement("Inverse Covariance Matrix",
         TypeConverter::toString<RasterElement>(), mpRasterElement) != NULL;
      mpCovarianceGui = new CovarianceGui(mpRasterElement,
         mInput.mRowFactor, mInput.mColumnFactor, mInput.mRecalculate, elementExists,
         Service<DesktopServices>()->getMainWidget());
   }
   return mpCovarianceGui;
}

void Covariance::propagateAbort()
{
}

bool Covariance::extractFromGui()
{
   if (mpCovarianceGui != NULL)
   {
      mInput.mRowFactor = mpCovarianceGui->getRowFactor();
      mInput.mColumnFactor = mpCovarianceGui->getColumnFactor();
      mInput.mpAoi = mpCovarianceGui->getAoi();
      if (!mpCovarianceGui->getUseExisting())
      {
         mpModelServices->destroyElement(mpModelServices->getElement("Covariance Matrix",
            TypeConverter::toString<RasterElement>(), mpRasterElement));
         mpModelServices->destroyElement(mpModelServices->getElement("Inverse Covariance Matrix",
            TypeConverter::toString<RasterElement>(), mpRasterElement));
      }
      mLoadIfExists = mpCovarianceGui->getUseFile();
      FactoryResource<Filename> pCvmFilename;
      pCvmFilename->setFullPathAndName(mpCovarianceGui->getFilename().toStdString());
      mpCovarianceAlg->setFile(pCvmFilename.get(), mLoadIfExists);
      return true;
   }
   return false;
}

bool Covariance::hasAbort()
{
   return true;
}

Covariance::Covariance() :
   AlgorithmPlugIn(&mInput),
   mpCovarianceAlg(NULL),
   mpCovarianceGui(NULL),
   mpRasterElement(NULL),
   mLoadIfExists(true)
{
   setName("Covariance");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("Covariance Matrix");
   setDescription("Compute Covariance Matrix");
   setDescriptorId("{4EE5591E-6834-47b4-B9FF-6BC1650F1F61}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Covariance::~Covariance()
{
}

bool CovarianceAlgorithm::preprocess()
{
   mpNewRasterElement = NULL;
   mpNewInvRasterElement = NULL;
   return true;
}

bool CovarianceAlgorithm::processAll()
{
   StepResource pStep("Generate Covariance matrix", "app", "EEA60078-71E9-40d9-B842-978E750DF0CF",
      "Unable to generate Covariance matrix.");
   mpStep = pStep.get();

   const RasterDataDescriptor* pDescriptor = NULL;
   const char* pCubeName = NULL;
   void* pData = NULL;
   unsigned int numRows(0);
   unsigned int numColumns(0);
   unsigned int numBands(0);
   EncodingType eType;

   RasterElement* pRasterElement = getRasterElement();
   if (pRasterElement == NULL)
   {
      reportProgress(ERRORS, 0, "No data set provided");
      return false;
   }

   pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      reportProgress(ERRORS, 0, "Could not access data descriptor");
      return false;
   }

   if (pDescriptor->getBandCount() == 1)
   {
      reportProgress(ERRORS, 0, "Cannot perform Covariance on 1 band data!");
      return false;
   }

   numRows = pDescriptor->getRowCount();
   numColumns = pDescriptor->getColumnCount();
   numBands = pDescriptor->getBandCount();

   { // scope the accessor
      FactoryResource<DataRequest> pRequest;
      pRequest->setInterleaveFormat(BIP);
      DataAccessor accessor = pRasterElement->getDataAccessor(pRequest.release());
      
      if (!accessor.isValid())
      {
         reportProgress(ERRORS, 0, "Could not access data");
         return false;
      }
   }
   ModelResource<RasterElement> pCvmElement(static_cast<RasterElement*>(mpModelServices->getElement(
      "Covariance Matrix", TypeConverter::toString<RasterElement>(), pRasterElement)));
   ModelResource<RasterElement> pInvCvmElement(static_cast<RasterElement*>(
      mpModelServices->getElement("Inverse Covariance Matrix",
      TypeConverter::toString<RasterElement>(), pRasterElement)));

   if (mInput.mRecalculate == true || pCvmElement.get() == NULL)
   {
      // Recalculation requested so destroy current elements or if cvm doesn't exist need to
      // destroy any existing inverse. Existing element destroyed when ModelResource assigned
      // new object (NULL in this case)
      pCvmElement = ModelResource<RasterElement>(static_cast<RasterElement*>(NULL));
      pInvCvmElement = ModelResource<RasterElement>(static_cast<RasterElement*>(NULL));
   }

   if (pCvmElement.get() == NULL)
   {
      pCvmElement = ModelResource<RasterElement>(RasterUtilities::createRasterElement(
         "Covariance Matrix", numBands, numBands, FLT8BYTES, true, pRasterElement));
      if (pCvmElement.get() == NULL)
      {
         reportProgress(ERRORS, 0, "Error creating Covariance matrix.");
         return false;
      }

      bool loadedFromFile(false);
      if (mLoadIfExists && mInput.mRecalculate == false)  // try to load cvm from file
      {
         loadedFromFile = readMatrixFromDisk(mCvmFile, pCvmElement.get());
      }

      if (loadedFromFile == false)                        // need to compute cvm
      {
         eType = pDescriptor->getDataType();

         // check that entire data block of element is in memory
         VERIFY(pCvmElement->getRawData() != NULL);
         if (mInput.mpAoi == NULL)
         {
            switchOnEncoding(eType, ComputeFactoredCov, pData, pRasterElement, 
               static_cast<double*>(pCvmElement->getRawData()), getProgress(), 
               &mAbortFlag, mInput.mRowFactor, mInput.mColumnFactor);
         }
         else
         {
            const BitMask* pMask = mInput.mpAoi->getSelectedPoints();
            if (pMask == NULL)
            {
               reportProgress(ERRORS, 0, "Error getting mask from AOI");
               return false;
            }
            else // pMask != NULL
            {
               BitMaskIterator it(pMask, pRasterElement);
               if (it.getCount() == 0)
               {
                  reportProgress(ERRORS, 0, "Error getting selected pixels from AOI");
                  return false;
               }
               else
               {
                  int x1 = 0;
                  int y1 = 0;
                  int x2 = 0;
                  int y2 = 0;
                  pMask->getBoundingBox(x1, y1, x2, y2);
                  const bool** pSelectedPixels = const_cast<BitMask*>(pMask)->getRegion(x1, y1, x2, y2);
                  MaskInput input;
                  input.pMatrix = static_cast<double*>(pCvmElement->getRawData());
                  input.numRows = numRows;
                  input.numCols = numColumns;
                  input.numBands = numBands;
                  input.pProgress = getProgress();
                  input.pAbortFlag = &mAbortFlag;
                  input.pSelectedPixels = pSelectedPixels;
                  input.mpMask = pMask;
                  input.boundingBoxX1 = x1;
                  input.boundingBoxX2 = x2;
                  input.boundingBoxY1 = y1;
                  input.boundingBoxY2 = y2;

                  switchOnEncoding(eType, ComputeMaskedCvm, pData, &input, pRasterElement);
               }
            }
         }

         if (mAbortFlag)
         {
            reportProgress(ABORT, 0, "Aborted creation of Covariance Matrix");
            return false;
         }

         writeMatrixToDisk(mCvmFile, pCvmElement.get());
      }
   }
   else
   {
      // check that existing cvm is right size and data type
      const RasterDataDescriptor* pDesc = static_cast<const RasterDataDescriptor*>(pCvmElement->getDataDescriptor());
      VERIFY(pDesc != NULL);
      unsigned int rows = pDesc->getRowCount();
      unsigned int cols = pDesc->getColumnCount();
      EncodingType dataType = pDesc->getDataType();
      if (rows != numBands || cols != numBands)
      {
         reportProgress(ERRORS, 0, "Dimensions of existing Covariance matrix do not match current data set.");
         return false;
      }
      if (dataType != FLT8BYTES)
      {
         reportProgress(ERRORS, 0, "Existing Covariance matrix has wrong data type.");
         return false;
      }
   }

   if (mInput.mComputeInverse)
   {
      bool validInverse(false);
      if (pInvCvmElement.get() == NULL)
      {
         pInvCvmElement = ModelResource<RasterElement>(RasterUtilities::createRasterElement(
            "Inverse Covariance Matrix", numBands, numBands, FLT8BYTES, true, pRasterElement));
         if (pInvCvmElement.get() == NULL)
         {
            reportProgress(ERRORS, 0, "Error creating inverse Covariance matrix.");
            return false;
         }
         VERIFY(pCvmElement->getRawData() != NULL);
         VERIFY(pInvCvmElement->getRawData() != NULL);
         validInverse = MatrixFunctions::invertSquareMatrix1D(
            static_cast<double*>(pInvCvmElement->getRawData()), 
            static_cast<double*>(pCvmElement->getRawData()), numBands);
      }
      else
      {
         // check that existing inverse is right size and data type
         const RasterDataDescriptor* pDesc = static_cast<const RasterDataDescriptor*>(
            pInvCvmElement->getDataDescriptor());
         VERIFY(pDesc != NULL);
         unsigned int rows = pDesc->getRowCount();
         unsigned int cols = pDesc->getColumnCount();
         EncodingType dataType = pDesc->getDataType();
         if (rows != numBands || cols != numBands)
         {
            reportProgress(ERRORS, 0, "Dimensions of the existing inverse of Covariance matrix "
               "do not match current data set.");
            return false;
         }
         if (dataType != FLT8BYTES)
         {
            reportProgress(ERRORS, 0, "Existing inverse of Covariance matrix has wrong data type.");
            return false;
         }
         validInverse = true;
      }

      if (validInverse)
      {
         // store to be added to the output arg list
         mpNewInvRasterElement = pInvCvmElement.release();
      }
      else
      {
         reportProgress(ERRORS, 0, "Could not compute the inverse of the Covariance matrix.");
         return false;
      }
   }

   // store Cvm to be added to the output arg list
   mpNewRasterElement = pCvmElement.release();

   pStep->finalize(Message::Success);
   reportProgress(NORMAL, 100, "Covariance Matrix Complete");

   return true;
}

bool CovarianceAlgorithm::postprocess()
{
   return true;
}

void CovarianceAlgorithm::setFile(const Filename* pCvmFile, bool loadIfExists)
{
   if (pCvmFile != NULL)
   {
      mCvmFile = pCvmFile->getFullPathAndName();
   }
   mLoadIfExists = loadIfExists;
}

string CovarianceAlgorithm::getFilename() const
{
   return mCvmFile;
}

RasterElement* CovarianceAlgorithm::getCovarianceElement() const
{
   return mpNewRasterElement;
}

RasterElement* CovarianceAlgorithm::getInverseCovarianceElement() const
{
   return mpNewInvRasterElement;
}

bool CovarianceAlgorithm::readMatrixFromDisk(string filename, RasterElement* pElement) const
{
   if (filename.empty() || pElement == NULL)
   {
      return false;
   }
   StepResource pStep("Read Covariance matrix from disk", "app", "131981ED-8D4C-4a36-AE37-64B04C903921",
                        "Unable to read Covariance matrix from disk");
   pStep->addProperty("Filename", filename);

   const RasterDataDescriptor* pDesc = static_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
   VERIFY(pDesc != NULL);
   unsigned int numRows = pDesc->getRowCount();
   unsigned int numCols = pDesc->getColumnCount();
   VERIFY(numRows == numCols);  // should be a square matrix
   FactoryResource<DataRequest> pRequest;
   pRequest->setWritable(true);
   DataAccessor accessor = pElement->getDataAccessor(pRequest.release());
   VERIFY(accessor.isValid());

   FileResource pFile(filename.c_str(), "rt");
   if (pFile.get() == NULL)
   {
      reportProgress(ERRORS, 0, "Unable to read Covariance matrix from disk");
      return false;
   }
   string progressMessage = "Reading Covariance matrix from disk";
   reportProgress(NORMAL, 0, progressMessage);

   vector<char> fileHeader(mExpectedFileHeader.length() + 1);
   if (fgets(&fileHeader.front(), fileHeader.capacity(), pFile) == NULL ||
      memcmp(&fileHeader.front(), mExpectedFileHeader.c_str(), fileHeader.size()) != 0)
   {
      reportProgress(WARNING, 0, "The file version is incorrect. "
         "The CVM file at \"" + filename + "\" will be overwritten with a current file.\n");
      return false;
   }

   unsigned int numBands = 0;
   int numFieldsRead = fscanf(pFile, "%d\n", &numBands);
   if (numFieldsRead != 1)
   {
      reportProgress(ERRORS, 0, "Unable to read number of bands in Covariance matrix from disk");
      return false;
   }
   else
   {
      pStep->addProperty("Cube Bands", numRows);  // Covariance matrix rows and cols are equal to num bands
      pStep->addProperty("File Bands", numBands);
      if (numBands != numRows)
      {
         reportProgress(ERRORS, 0, "Mismatch between number of bands in cube and in CVM file.");
         return false;
      }

      double* pData(NULL);
      for (unsigned int row = 0; row < numRows; ++row)
      {
         int iPercent = row * 100 / numRows;
         reportProgress(NORMAL, iPercent, progressMessage);

         VERIFY(accessor.isValid())
         for (unsigned int col = 0; col < numCols; ++col)
         {
            pData = static_cast<double*>(accessor->getColumn());
            numFieldsRead = fscanf(pFile, "%lg ", pData);
            if (numFieldsRead != 1)
            {
               pStep->addProperty("Row", row + 1);
               reportProgress(ERRORS, 0, "Unable to read Covariance matrix from disk");
               return false;
            }
            accessor->nextColumn();
         }
         accessor->nextRow();
      }
      reportProgress(NORMAL, 100, "Covariance matrix successfully read from disk");
   }

   pStep->finalize(Message::Success);

   return true;
}

bool CovarianceAlgorithm::writeMatrixToDisk(string filename, const RasterElement* pElement) const
{
   if (filename.empty() || pElement == NULL)
   {
      return false;
   }

   StepResource pStep("Save Covariance matrix to disk", "app", "AE4CBEED-CB4C-492f-AEA3-57C9A6239015",
                        "Unable to save Covariance matrix to disk");
   pStep->addProperty("Filename", filename);

   const RasterDataDescriptor* pDesc = static_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
   VERIFY(pDesc != NULL);
   unsigned int numRows = pDesc->getRowCount();
   unsigned int numCols = pDesc->getColumnCount();
   VERIFY(numRows == numCols);  // should be a square matrix
   FactoryResource<DataRequest> pRequest;
   DataAccessor accessor = pElement->getDataAccessor(pRequest.release());
   VERIFY(accessor.isValid());

   FileResource pFile(filename.c_str(), "wt");
   if (pFile.get() == NULL)
   {
      reportProgress(WARNING, 100, "Unable to save Covariance matrix to disk");
      return false;
   }
   fprintf(pFile, "%s", mExpectedFileHeader.c_str());
   fprintf(pFile, "%d\n", numRows);
   for (unsigned int i = 0; i < numRows; ++i)
   {
      if (accessor.isValid() == false)
      {
         // need to close and destroy the file
         return false;
      }
      for (unsigned int j = 0; j < numCols; ++j)
      {
         fprintf(pFile, "%.15e ", *static_cast<double*>(accessor->getColumn()));
         accessor->nextColumn();
      }
      fprintf(pFile, "\n");
      accessor->nextRow();
   }

   reportProgress(NORMAL, 100, "Covariance matrix saved to disk as " + filename);
   pStep->finalize(Message::Success);

   return true;
}

CovarianceAlgorithm::CovarianceAlgorithm(RasterElement* pRasterElement, Progress* pProgress, bool interactive) :
   AlgorithmPattern(pRasterElement, pProgress, interactive, NULL),
   mpNewRasterElement(NULL),
   mpNewInvRasterElement(NULL),
   mAbortFlag(false),
   mLoadIfExists(true)
{
   if (pRasterElement != NULL)
   {
      mCvmFile = pRasterElement->getFilename();
      if (!mCvmFile.empty())
      {
         mCvmFile += ".cvm";
      }
   }
}

bool CovarianceAlgorithm::canAbort() const
{
   return true;
}

bool CovarianceAlgorithm::doAbort()
{
   mAbortFlag = true;
   return true;
}

bool CovarianceAlgorithm::initialize(void* pAlgorithmData)
{
   if (pAlgorithmData != NULL)
   {
      mInput = *static_cast<Input*>(pAlgorithmData);
   }
   return true;
}
