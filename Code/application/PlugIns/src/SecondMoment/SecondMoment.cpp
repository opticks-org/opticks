/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFile>

#include "AppVersion.h"
#include "AoiElement.h"
#include "BitMask.h"
#include "AppVerify.h"
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
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterUtilities.h"
#include "SecondMoment.h"
#include "SecondMomentGui.h"
#include "switchOnEncoding.h"
#include "TypeConverter.h"

#include <algorithm>
#include <string>
#include <vector>
using namespace std;

const string SecondMomentAlgorithm::mExpectedFileHeader = "Second Moment Matrix File v1.1\n";
static bool **CopySelectedPixels(const bool **pSelectedPixels, int xsize, int ysize);
static void DeleteSelectedPixels(bool **pSelectedPixels);

template<class T>
T *GetRowPtr (T *raw, int numCols, int numBands, int row, int col)
{
   return raw + numBands * (row *numCols + col);
}

template<class T>
void ComputeFactoredSmm(T *pData, RasterElement* pRaster, double *pMatrix,
                        Progress *pProgress, const bool *pAbortFlag,
                        int rowFactor, int columnFactor)
{
   if (pRaster == NULL)
   {
      return;
   }

   unsigned int row, col, band;
   unsigned int i, j;
   unsigned int lCount = 0;

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   unsigned int NumRows = pDescriptor->getRowCount();
   unsigned int NumCols = pDescriptor->getColumnCount();
   unsigned int NumBands = pDescriptor->getBandCount();

   if(rowFactor < 1)
   {
      rowFactor = 1;
   }
   if(columnFactor < 1)
   {
      columnFactor = 1;
   }

   memset(pMatrix, 0, sizeof(double) * NumBands * NumBands);

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BIP);

   DataAccessor accessor = pRaster->getDataAccessor(pRequest.release());
   T *pDataColumn=NULL;
   for(row = 0; row < NumRows;)
   {
      if(pProgress != NULL)
      {
         if((pAbortFlag == NULL) || !(*pAbortFlag))
         {
            int iPercent = (100 * row) / NumRows;
            if(iPercent == 100)
            {
               iPercent = 99;
            }

            pProgress->updateProgress("Computing Second Moment Matrix...", iPercent, NORMAL);
         }
         else
         {
            break;
         }
      }
      VERIFYNRV(accessor.isValid());
      pData = static_cast<T*>(accessor->getRow());
      for(col = 0; col < NumCols; col += columnFactor)
      {
         lCount++;
         pDataColumn = pData + col * NumBands;
         double* pMatrixRow = pMatrix;
         double* pMatrixColumn = NULL;
         T* pRowIndexBand = pDataColumn;
         T* pColIndexBand = pDataColumn;
         double* pStop = NULL;
         for(band = 0; band < NumBands; band++,
                                        pRowIndexBand++,
                                        pColIndexBand = pRowIndexBand,
                                        pMatrixRow += (NumBands + 1))
         {
            pMatrixColumn = pMatrixRow;
            pStop = &pMatrixColumn[NumBands-band];
            while(pMatrixColumn != pStop)
            {
               *pMatrixColumn += (*pRowIndexBand) * (*pColIndexBand);
               ++pMatrixColumn;
               ++pColIndexBand;
            }
         }
      }
      for(i = 0; i< static_cast<unsigned int>(rowFactor); i++)
      {
         row++;
         if(row < NumRows)
         {
            accessor->nextRow();
         }
      }
   }

   for(i = 0; i < NumBands; i++)
   {
      int index = i * NumBands + i;
      for(j = i; j < NumBands; j++, index++)
      {
         pMatrix[index] /= lCount;
      }
   }

   for(i = 0; i < NumBands; i++)
   {
      for(j = i; j < NumBands; j++)
      {
         pMatrix[j * NumBands + i] = pMatrix[i * NumBands + j];
      }
   }

   if(pProgress != NULL)
   {
      if((pAbortFlag == NULL) || !(*pAbortFlag))
      {
         pProgress->updateProgress("Second Moment Matrix Complete", 99, NORMAL);
      }
      else
      {
         pProgress->updateProgress("Aborted computing Second Moment Matrix", 0, ABORT);
      }
   }
}

struct MaskInput
{
   double *pMatrix;
   int numRows;
   int numCols;
   int numBands;
   Progress *pProgress;
   const bool *pAbortFlag;
   const bool **pSelectedPixels;
   int boundingBoxX1;
   int boundingBoxY1;
   int boundingBoxX2;
   int boundingBoxY2;
};

template<class T>
void ComputeMaskedSmm(T *pData, MaskInput *pInput, RasterElement* pRaster)
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
   int NumRows = pDescriptor->getRowCount();
   int NumCols = pDescriptor->getColumnCount();
   int NumBands = pDescriptor->getBandCount();
   int i = 0, j = 0;
   int lRow = 0, lColumn = 0;
   //int lLocalIndex;
   int lCount = 0;
   T *pBand1 = NULL;
   T *pBand2 = NULL;
   int boundingBoxXSize = pInput->boundingBoxX2 - pInput->boundingBoxX1 + 1;
   int boundingBoxYSize = pInput->boundingBoxY2 - pInput->boundingBoxY1 + 1;

   memset(pInput->pMatrix, 0, sizeof(double) * pInput->numBands * pInput->numBands);

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BIP);
   pRequest->setRows(pDescriptor->getActiveRow(pInput->boundingBoxY1), 
      pDescriptor->getActiveRow(pInput->boundingBoxY2));
   pRequest->setColumns(pDescriptor->getActiveColumn(pInput->boundingBoxX1), 
      pDescriptor->getActiveColumn(pInput->boundingBoxX2));
   DataAccessor accessor = pRaster->getDataAccessor(pRequest.release());
   for(j = 0; j < boundingBoxYSize; j++)
   {
      VERIFYNRV(accessor.isValid());
      if(pInput->pProgress != NULL)
      {
         if((pInput->pAbortFlag == NULL) || !(*pInput->pAbortFlag))
         {
            int iPercent = (100 * j) / boundingBoxYSize;
            if(iPercent == 100)
            {
               iPercent = 99;
            }

            pInput->pProgress->updateProgress("Computing Second Moment Matrix...", iPercent, NORMAL);
         }
         else
         {
            break;
         }
      }
      for(i = 0; i < boundingBoxXSize; i++)
      {
         if(pInput->pSelectedPixels[j][i])
         {
            lCount++;
            pData = static_cast<T*>(accessor->getColumn());
            pBand2 = pData;
            for(lRow = 0; lRow < pInput->numBands; lRow++)
            {
               pBand1 = pBand2;
               int index = lRow * pInput->numBands + lRow;
               for(lColumn = lRow; lColumn < pInput->numBands; lColumn++, index++)
               {
                  pInput->pMatrix[index] += (*pBand1) * (*pBand2);
                  pBand1++;
               }
               pBand2++;
            }
         }
         accessor->nextColumn();
      }
      accessor->nextRow();
   }

   for(i = 0; i < pInput->numBands; i++)
   {
      int index = i * pInput->numBands + i;
      for(j = i; j < pInput->numBands; j++, index++)
      {
         pInput->pMatrix[index] /= lCount;
      }
   }

   for(i = 0; i < pInput->numBands; i++)
   {
      for(j = i; j < pInput->numBands; j++)
      {
         pInput->pMatrix[j * pInput->numBands + i] = pInput->pMatrix[i * pInput->numBands + j];
      }
   }

   if(pInput->pProgress != NULL)
   {
      if((pInput->pAbortFlag == NULL) || !(*pInput->pAbortFlag))
      {
         pInput->pProgress->updateProgress("Second Moment Matrix Complete", 99, NORMAL);
      }
      else
      {
         pInput->pProgress->updateProgress("Aborted computing Second Moment Matrix", 0, ABORT);
      }
   }
}

bool SecondMoment::canRunBatch() const
{
   return true;
}

bool SecondMoment::canRunInteractive() const
{
   return true;
}

bool SecondMoment::populateBatchInputArgList(PlugInArgList *pArgList)
{
   static int lRowFactor = 1;
   static int lColumnFactor = 1;

   VERIFY(pArgList != NULL);
   VERIFY(populateInteractiveInputArgList(pArgList));
   VERIFY(pArgList->addArg<int>("Row Factor", &lRowFactor, "Process the first row in a group of this many rows. "
                                      "A value of 1 processes all rows. A value of 2 processes 1 row and skips 1 row, etc. "
                                      "A row is used if its zero based index modulo the row factor is zero."));
   VERIFY(pArgList->addArg<int>("Column Factor", &lColumnFactor, "Process the first column in a group of this many columns. "
                                      "A value of 1 processes all columns. A value of 2 processes 1 column and skips 1 column, etc. "
                                      "A column is used if its zero based index modulo the column factor is zero."));
   VERIFY(pArgList->addArg<Filename>("SMM File", NULL, "Store and retrieve second moment data from this file. If NULL, a filename "
                                          "based on the data set filename will be generated."));
   VERIFY(pArgList->addArg<AoiElement>("AOI", NULL, "Only calculate using pixels selected by this AOI. If NULL, use the entire image. "
                                          "If row and column factors are not 1, they will also be used when determining data to process."));

   return true;
}

bool SecondMoment::populateInteractiveInputArgList(PlugInArgList *pArgList)
{
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<Progress>(ProgressArg(), NULL));
   VERIFY(pArgList->addArg<RasterElement>(DataElementArg(), NULL));
   bool recalc = false;
   VERIFY(pArgList->addArg<bool>("Recalculate", &recalc, "If true, the second moment matrix will be recalculated. If false, the plug-in "
                                    "will decide if the matrix needs to be recalculated. If a matrix exists in memory, it is used. "
                                    "If there exists a matrix on disk, it will be loaded. Finally, the matrix will be calculated."));

   return true;
}

bool SecondMoment::populateDefaultOutputArgList(PlugInArgList *pArgList)
{
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<RasterElement>("Second Moment Matrix"));
   VERIFY(pArgList->addArg<RasterElement>("Inverse Second Moment Matrix"));
   return true;
}

bool SecondMoment::parseInputArgList(PlugInArgList *pArgList)
{
   Filename *pSmmFilename = NULL;

   Progress *pProgress = pArgList->getPlugInArgValue<Progress>(ProgressArg());
   if (pProgress == NULL)
   {
      return false;
   }

   mpRasterElement = pArgList->getPlugInArgValue<RasterElement>(DataElementArg());
   if (mpRasterElement == NULL)
   {
      string msg = "The raster element input value is invalid!";
      if(pProgress != NULL) pProgress->updateProgress(msg, 0, ERRORS);
      return false;
   }

   EncodingType eDataType;

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>
      (mpRasterElement->getDataDescriptor());
   if(pDescriptor == NULL)
   {
      string msg = "The sensor data input value is invalid!";
      if(pProgress != NULL) pProgress->updateProgress(msg, 0, ERRORS);
      return false;
   }
   eDataType = pDescriptor->getDataType();

   if((eDataType == INT4SCOMPLEX) || (eDataType == FLT8COMPLEX))
   {
      string msg = "Complex data is not supported!";
      if(pProgress != NULL) pProgress->updateProgress(msg, 0, ERRORS);
      return false;
   }
   VERIFY(pArgList->getPlugInArgValue("Recalculate", mInput.mRecalculate));

   if(!isInteractive())
   {
      VERIFY(pArgList->getPlugInArgValue<int>("Row Factor", mInput.mRowFactor));
      VERIFY(pArgList->getPlugInArgValue<int>("Column Factor", mInput.mColumnFactor));
      pSmmFilename = pArgList->getPlugInArgValue<Filename>("SMM File");
      mInput.mpAoi = pArgList->getPlugInArgValue<AoiElement>("AOI");
   }
   mpSecondMomentAlg = new SecondMomentAlgorithm(mpRasterElement, pProgress, isInteractive());
   mpSecondMomentAlg->setFile(pSmmFilename, mLoadIfExists);

   setAlgorithmPattern(Resource<AlgorithmPattern>(mpSecondMomentAlg));
   return true;
}

bool SecondMoment::setActualValuesInOutputArgList(PlugInArgList *pArgList)
{
   VERIFY(mpSecondMomentAlg != NULL);
   VERIFY(pArgList != NULL);
   return pArgList->setPlugInArgValue<RasterElement>("Second Moment Matrix", 
                        mpSecondMomentAlg->getSecondMomentElement()) &&
          pArgList->setPlugInArgValue<RasterElement>("Inverse Second Moment Matrix", 
                        mpSecondMomentAlg->getInverseSecondMomentElement());
}

QDialog *SecondMoment::getGui(void *pAlgData)
{
   if(isInteractive() && (mpSecondMomentAlg != NULL))
   {
      delete mpSecondMomentGui; // re-initialize

      bool elementExists = false;
      elementExists |= mpModelServices->getElement("Second Moment Matrix",
         TypeConverter::toString<RasterElement>(), mpRasterElement) != NULL;
      elementExists |= mpModelServices->getElement("Inverse Second Moment Matrix",
         TypeConverter::toString<RasterElement>(), mpRasterElement) != NULL;
      mpSecondMomentGui = new SecondMomentGui(mpRasterElement,
         mInput.mRowFactor, mInput.mColumnFactor, mInput.mRecalculate, elementExists,
         Service<DesktopServices>()->getMainWidget());
   }
   return mpSecondMomentGui;
}

void SecondMoment::propagateAbort()
{
}

bool SecondMoment::extractFromGui()
{
   if(mpSecondMomentGui != NULL)
   {
      mInput.mRowFactor = mpSecondMomentGui->getRowFactor();
      mInput.mColumnFactor = mpSecondMomentGui->getColumnFactor();
      mInput.mpAoi = mpSecondMomentGui->getAoi();
      if(!mpSecondMomentGui->getUseExisting())
      {
         mpModelServices->destroyElement(mpModelServices->getElement("Second Moment Matrix",
            TypeConverter::toString<RasterElement>(), mpRasterElement));
         mpModelServices->destroyElement(mpModelServices->getElement("Inverse Second Moment Matrix",
            TypeConverter::toString<RasterElement>(), mpRasterElement));
      }
      mLoadIfExists = mpSecondMomentGui->getUseFile();
      FactoryResource<Filename> pSmmFilename;
      pSmmFilename->setFullPathAndName(mpSecondMomentGui->getFilename().toStdString());
      mpSecondMomentAlg->setFile(pSmmFilename.get(), mLoadIfExists);
      return true;
   }
   return false;
}

bool SecondMoment::hasAbort()
{
   return true;
}

SecondMoment::SecondMoment() :
   AlgorithmPlugIn(&mInput),
   mpSecondMomentAlg(NULL),
   mpSecondMomentGui(NULL),
   mpRasterElement(NULL),
   mLoadIfExists(true)
{
   setName("Second Moment");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("Second Moment Matrix");
   setDescription("Compute Second Moment Matrix");
   setDescriptorId("{7FDE70AA-7182-488b-ABCD-452CBAE8436E}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SecondMoment::~SecondMoment()
{
}

bool SecondMomentAlgorithm::preprocess()
{
   mpNewRasterElement = NULL;
   mpNewInvRasterElement = NULL;
   return true;
}

bool SecondMomentAlgorithm::processAll()
{
   StepResource pStep("Generate SecondMoment matrix", "app", "365B9658-2380-4f41-B344-AD96D9DBFA00",
                              "Unable to generate SecondMoment matrix.");
   mpStep = pStep.get();

   const RasterDataDescriptor* pDescriptor = NULL;
   const char *pCubeName = NULL;
   void *pData = NULL;
   unsigned int numRows = 0;
   unsigned int numColumns = 0;
   unsigned int numBands = 0;
   EncodingType eType = UNKNOWN;

   RasterElement *pRasterElement = getRasterElement();
   if(pRasterElement == NULL)
   {
      reportProgress(ERRORS, 0, "No data set provided");
      return false;
   }

   pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
   if(pDescriptor == NULL)
   {
      reportProgress(ERRORS, 0, "Could not access data descriptor");
      return false;
   }

   if (pDescriptor->getBandCount() == 1)
   {
      reportProgress(ERRORS, 0, "Cannot perform Second Moment on 1 band data!");
      return false;
   }

   numRows = pDescriptor->getRowCount();
   numColumns = pDescriptor->getColumnCount();
   numBands = pDescriptor->getBandCount();

   { // scope the accessor
      FactoryResource<DataRequest> pRequest;
      pRequest->setInterleaveFormat(BIP);
      DataAccessor accessor = pRasterElement->getDataAccessor(pRequest.release());
      
      if(!accessor.isValid())
      {
         reportProgress(ERRORS, 0, "Could not access data");
         return false;
      }
   }
   ModelResource<RasterElement> pElement(static_cast<RasterElement*>(mpModelServices->getElement("Second Moment Matrix",
      TypeConverter::toString<RasterElement>(), pRasterElement)));
   ModelResource<RasterElement> pInvElement(static_cast<RasterElement*>(mpModelServices->getElement("Inverse Second Moment Matrix",
      TypeConverter::toString<RasterElement>(), pRasterElement)));

   if (mInput.mRecalculate == true)
   {
      // Recalculation requested
      if (pElement.get() != NULL)
      {
         VERIFY(mpModelServices->destroyElement(pElement.release()));
      }

      if (pInvElement.get() != NULL)
      {
         VERIFY(mpModelServices->destroyElement(pInvElement.release()));
      }
   }
   else
   {
      if (pElement.get() != NULL || pInvElement.get() != NULL)
      {
         // recalculation not requested and the data is already calculated...just return it.
         double *pResultsData = NULL, *pInvResultsData = NULL;
         if(pElement.get() == NULL)
         {
            pElement = ModelResource<RasterElement>(RasterUtilities::createRasterElement(
               "Second Moment Matrix", numBands, numBands, FLT8BYTES, true, pRasterElement));
            pResultsData = reinterpret_cast<double*>(pElement->getRawData());
            pInvResultsData = reinterpret_cast<double*>(pInvElement->getRawData());
            VERIFY(MatrixFunctions::invertSquareMatrix1D(pResultsData, pInvResultsData, numBands));
         }
         else if(pInvElement.get() == NULL)
         {
            pInvElement = ModelResource<RasterElement>(RasterUtilities::createRasterElement(
               "Inverse Second Moment Matrix", numBands, numBands, FLT8BYTES, true, pRasterElement)); 
            pResultsData = reinterpret_cast<double*>(pElement->getRawData());
            pInvResultsData = reinterpret_cast<double*>(pInvElement->getRawData());
            VERIFY(MatrixFunctions::invertSquareMatrix1D(pInvResultsData, pResultsData, numBands));
         }
         else
         {
            pResultsData = reinterpret_cast<double*>(pElement->getRawData());
            pInvResultsData = reinterpret_cast<double*>(pInvElement->getRawData());
         }
         mpNewRasterElement = pElement.release();
         mpNewInvRasterElement = pInvElement.release();
         writeMatrixToDisk(mSmmFile, pResultsData, numBands);
         reportProgress(NORMAL, 100, "Second Moment Matrix Complete");
         pStep->finalize(Message::Success);
         return true;
      }
   }

   pElement = ModelResource<RasterElement>(RasterUtilities::createRasterElement(
      "Second Moment Matrix", numBands, numBands, FLT8BYTES, true, pRasterElement));
   pInvElement = ModelResource<RasterElement>(RasterUtilities::createRasterElement(
      "Inverse Second Moment Matrix", numBands, numBands, FLT8BYTES, true, pRasterElement)); 
   VERIFY(pElement.get() != NULL && pInvElement.get() != NULL);

   double *pResultsData = reinterpret_cast<double*>(pElement->getRawData());
   double *pInvResultsData = reinterpret_cast<double*>(pInvElement->getRawData());

   if(pResultsData == NULL || pInvResultsData == NULL)
   {
      reportProgress(ERRORS, 0, "Error allocating memory for results");
      return false;
   }

   QFile smmFile(QString::fromStdString(mSmmFile));
   // load the results only if recalculation was not requested
   if(!mInput.mRecalculate && mLoadIfExists &&
      smmFile.exists() && readMatrixFromDisk(mSmmFile, pResultsData, numBands))
   {
      VERIFY(MatrixFunctions::invertSquareMatrix1D(pInvResultsData, pResultsData, numBands));
      mpNewRasterElement = pElement.release();
      mpNewInvRasterElement = pInvElement.release();
   }
   else
   {
      eType = pDescriptor->getDataType();

      if(mInput.mpAoi == NULL)
      {
         switchOnEncoding(eType, ComputeFactoredSmm, pData, pRasterElement, pResultsData, 
            getProgress(), &mAbortFlag, mInput.mRowFactor, mInput.mColumnFactor);
      }
      else
      {
         const BitMask *pMask = mInput.mpAoi->getSelectedPoints();
         if(pMask == NULL)
         {
            reportProgress(ERRORS, 0, "Error getting mask from AOI");
            return false;
         }
         else // pMask != NULL
         {
            int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
            pMask->getBoundingBox(x1, y1, x2, y2);
            const bool **pSelectedPixels = const_cast<BitMask*>(pMask)->getRegion(x1, y1, x2, y2);
            if(pSelectedPixels == NULL)
            {
               reportProgress(ERRORS, 0, "Error getting selected pixels from AOI");
               return false;
            }
            else // pSelectedPixels != NULL
            {
               MaskInput input;
               input.pMatrix = pResultsData;
               input.numRows = numRows;
               input.numCols = numColumns;
               input.numBands = numBands;
               input.pProgress = getProgress();
               input.pAbortFlag = &mAbortFlag;
               input.pSelectedPixels = pSelectedPixels;
               input.boundingBoxX1 = x1;
               input.boundingBoxX2 = x2;
               input.boundingBoxY1 = y1;
               input.boundingBoxY2 = y2;

               switchOnEncoding(eType, ComputeMaskedSmm, pData, &input, pRasterElement);
            }
         }
      }

      if(mAbortFlag)
      {
         reportProgress(ABORT, 0, "Aborted creation of Second Moment Matrix");
         return false;
      }
      else
      {
         VERIFY(MatrixFunctions::invertSquareMatrix1D(pInvResultsData, pResultsData, numBands));
      }

      mpNewRasterElement = pElement.release();
      mpNewInvRasterElement = pInvElement.release();

      writeMatrixToDisk(mSmmFile, pResultsData, numBands);
   }
   pStep->finalize(Message::Success);
   reportProgress(NORMAL, 100, "Second Moment Matrix Complete");

   return true;
}

bool SecondMomentAlgorithm::postprocess()
{
   return true;
}

void SecondMomentAlgorithm::setFile(const Filename *pSmmFile, bool loadIfExists)
{
   if(pSmmFile != NULL)
   {
      mSmmFile = pSmmFile->getFullPathAndName();
   }
   mLoadIfExists = loadIfExists;
}

RasterElement *SecondMomentAlgorithm::getSecondMomentElement() const
{
   return mpNewRasterElement;
}

RasterElement *SecondMomentAlgorithm::getInverseSecondMomentElement() const
{
   return mpNewInvRasterElement;
}

bool SecondMomentAlgorithm::readMatrixFromDisk(string filename, double *pData, int numBands) const
{
   if(filename.empty())
   {
      return false;
   }
   StepResource pStep("Read second moment matrix from disk", "app", "2FE4BD92-4F9E-4e14-A51A-07E497E6EAA6",
                        "Unable to read SecondMoment matrix from disk");
   pStep->addProperty("Filename", filename);

   FileResource pFile(filename.c_str(), "rt");
   if (pFile.get() == NULL)
   {
      reportProgress(ERRORS, 0, "Unable to read SecondMoment matrix from disk");
      return false;
   }
   string progressMessage = "Reading SecondMoment matrix from disk";
   reportProgress(NORMAL, 0, progressMessage);

   vector<char> fileHeader(mExpectedFileHeader.length() + 1);
   if (fgets(&fileHeader.front(), fileHeader.capacity(), pFile) == NULL ||
      memcmp(&fileHeader.front(), mExpectedFileHeader.c_str(), fileHeader.size()) != 0)
   {
      reportProgress(WARNING, 0, "The file version is incorrect. "
         "The SMM file at \"" + filename + "\" will be overwritten with a current file.\n");
      return false;
   }

   int lnumBands = 0;
   int numFieldsRead = fscanf(pFile, "%d\n", &lnumBands);
   if(numFieldsRead != 1)
   {
      reportProgress(ERRORS, 0, "Unable to read number of bands in SecondMoment matrix from disk");
      return false;
   }
   else
   {
      pStep->addProperty("Cube Bands", numBands);
      pStep->addProperty("File Bands", lnumBands);
      if(lnumBands != numBands)
      {
         reportProgress(ERRORS, 0, "Mismatch between number of bands in cube and in SMM file.");
         return false;
      }
      for(int i = 0; i < numBands; i++)
      {
         for(int j = 0; j < numBands; j++)
         {
            numFieldsRead = fscanf(pFile, "%lg ", &(pData[i * numBands + j]));
            if(numFieldsRead != 1)
            {
               pStep->addProperty("Row", i + 1);
               reportProgress(ERRORS, 0, "Unable to read SecondMoment matrix from disk");
               return false;
            }
         }
         int iPercent = 100 * (i + 1) / numBands;
         if(iPercent == 100)
         {
            iPercent = 99;
         }
         reportProgress(NORMAL, iPercent, progressMessage);
      }
      reportProgress(NORMAL, 100, "SecondMoment matrix successfully read from disk");
   }

   pStep->finalize(Message::Success);

   return true;
}

bool SecondMomentAlgorithm::writeMatrixToDisk(string filename, const double *pData, int numBands) const
{
   if(filename.empty())
   {
      return false;
   }

   StepResource pStep("Save second moment matrix to disk", "app", "5A145DEF-5953-428e-98BA-D14A630669EF",
                        "Unable to save SecondMoment matrix to disk");
   pStep->addProperty("Filename", filename);

   FileResource pFile(filename.c_str(), "wt");
   if(pFile.get() == NULL)
   {
      reportProgress(WARNING, 100, "Unable to save SecondMoment matrix to disk");
      return false;
   }
   fprintf(pFile, "%s", mExpectedFileHeader.c_str());
   fprintf(pFile, "%d\n", numBands);
   for(int i = 0; i < numBands; i++)
   {
      for(int j = 0; j < numBands; j++)
      {
         fprintf(pFile, "%.15e ", pData[i * numBands + j]);
      }
      fprintf(pFile, "\n");
   }

   reportProgress(NORMAL, 100, "SecondMoment matrix saved to disk as " + filename);;
   pStep->finalize(Message::Success);

   return true;
}

SecondMomentAlgorithm::SecondMomentAlgorithm(RasterElement *pRasterElement, Progress *pProgress, bool interactive) :
   AlgorithmPattern(pRasterElement, pProgress, interactive, NULL),
   mpNewRasterElement(NULL),
   mpNewInvRasterElement(NULL),
   mAbortFlag(false),
   mLoadIfExists(true)
{
   if(pRasterElement != NULL)
   {
      mSmmFile = pRasterElement->getFilename();
      if(!mSmmFile.empty())
      {
         mSmmFile += ".smm";
      }
   }
}

bool SecondMomentAlgorithm::canAbort() const
{
   return true;
}

bool SecondMomentAlgorithm::doAbort()
{
   mAbortFlag = true;

   return true;
}

bool SecondMomentAlgorithm::initialize(void *pAlgorithmData)
{
   if(pAlgorithmData != NULL)
   {
      mInput = *static_cast<Input*>(pAlgorithmData);
   }
   return true;
}
