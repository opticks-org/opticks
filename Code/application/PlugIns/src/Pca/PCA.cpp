/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include "AppAssert.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "AoiElement.h"
#include "ApplicationServices.h"
#include "BitMaskIterator.h"
#include "ConfigurationSettings.h"
#include "DataAccessorImpl.h"
#include "DimensionDescriptor.h"
#include "EigenPlotDlg.h"
#include "Filename.h"
#include "FileResource.h"
#include "MatrixFunctions.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PCA.h"
#include "PcaDlg.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StatisticsDlg.h"
#include "switchOnEncoding.h"
#include "Undo.h"

#include <fstream>
#include <limits>
#include <math.h>
#include <typeinfo>
using namespace std;

template<class T>
T *GetPixelPtr(T *raw, int numCols, int numBands, int row, int col)
{
   return raw + numBands * (row *numCols + col);
}

template<class T>
void ComputeFactoredCov(T *pData, RasterElement* pRaster, double **pMatrix,
                        Progress *pProgress, const bool *pAbortFlag,
                        int rowFactor, int columnFactor)
{
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

   // calculate average spectrum
   float progScale = 100.0f * static_cast<float>(rowFactor)/static_cast<float>(numRows);
   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BIP);
   DataAccessor accessor = pRaster->getDataAccessor(pRequest.release());
   for (row = 0; row < numRows; row += rowFactor)
   {
      VERIFYNRV(accessor.isValid());
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
         ++lCount;
         pPixel = reinterpret_cast<T*>(accessor->getColumn());

         for (band1 = 0; band1 < numBands; ++band1)
         {
            pAverage[band1] += *pPixel;
            ++pPixel;
         }
         accessor->nextColumn();
      }
      accessor->nextRow();
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
         VERIFYNRV(accessor.isValid());
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
            pPixel = reinterpret_cast<T*>(accessor->getColumn());

            for (band2 = 0; band2 < numBands; ++band2)
            {
               pData = pPixel;
               for (band1 = band2; band1 < numBands; ++band1)
               {
                  pMatrix[band2][band1] += (*pData - pAverage[band1]) * (*pPixel - pAverage[band2]);
                  ++pData;
               }
               ++pPixel;
            }
            accessor->nextColumn();
         }
         accessor->nextRow();
      }
   }

   // check if aborted
   if ((pAbortFlag == NULL) || !(*pAbortFlag))
   {
      for (band2 = 0; band2 < numBands; ++band2)
      {
         for (band1 = band2; band1 < numBands; ++band1)
         {
            pMatrix[band2][band1] /= lCount;
         }
      }

      for (band2 = 0; band2 < numBands; ++band2)
      {
         for (band1 = band2 + 1; band1 < numBands; ++band1)
         {
            pMatrix[band1][band2] = pMatrix[band2][band1];
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

struct MaskInput
{
   RasterElement* mpRaster;
   double** mpMatrix;
   Progress* mpProgress;
   const bool* mpAbortFlag;
   const BitMask* mpMask;
};

template<class T>
void ComputeMaskedCov(T* pData, MaskInput* pInput)
{
   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>
      (pInput->mpRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   unsigned int numBands = pDescriptor->getBandCount();
   unsigned int band1, band2;
   unsigned int lCount = 0;
   vector<double> averages(numBands, 0);
   double* pAverage = &averages.front();
   T *pPixel = NULL;

   // calculate average spectrum
   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BIP);

   DataAccessor accessor = pInput->mpRaster->getDataAccessor(pRequest->copy());
   BitMaskIterator it(pInput->mpMask, pInput->mpRaster);
   int numPixels = it.getCount();
   float progScale = 100.0f / numPixels;
   LocationType loc;
   pPixel = reinterpret_cast<T*>(accessor->getColumn());
   int mask = 0;
   int progSave = 0;
   if (it == it.end())
   {
      pInput->mpProgress->updateProgress("No pixels Selected", 0, ERRORS);
   }

   while (it != it.end())
   {
      if (pInput->mpProgress != NULL &&
          progSave != static_cast<int>(progScale * mask))
      {
         if ((pInput->mpAbortFlag == NULL) || !(*pInput->mpAbortFlag))
         {
            progSave = static_cast<int>(progScale * mask);
            pInput->mpProgress->updateProgress("Computing Average Signature...",
               progSave , NORMAL);
         }
         else
         {
            break;
         }
      }
      it.getPixelLocation(loc);
      accessor->toPixel(loc.mY, loc.mX);
      VERIFYNRV(accessor.isValid());
      pPixel = reinterpret_cast<T*>(accessor->getColumn());
      ++lCount;
      for (band1 = 0; band1 < numBands; ++band1)
      {
         pAverage[band1] += *pPixel;
         ++pPixel;
      }
      ++it;
      ++mask;
   }

   // check if aborted
   if ((pInput->mpAbortFlag == NULL) || !(*pInput->mpAbortFlag))
   {
      for (band1 = 0; band1 < numBands; ++band1)
      {
         pAverage[band1] /= lCount;
      }

      // calculate covariance matrix
      accessor = pInput->mpRaster->getDataAccessor(pRequest->copy());
      it.firstPixel();
      mask = 0;
      unsigned int numPixels = it.getCount();
      float progScale = 100.0f / numPixels;
      LocationType loc;
      progSave = 0;
      while (it != it.end())
      {
         if (pInput->mpProgress != NULL &&
             progSave != static_cast<int>(progScale * mask))
         {
            if ((pInput->mpAbortFlag == NULL) || !(*pInput->mpAbortFlag))
            {
               progSave = static_cast<int>(progScale * mask);
               pInput->mpProgress->updateProgress("Computing Covariance Matrix...",
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
         pPixel = reinterpret_cast<T*>(accessor->getColumn());
         for (band2 = 0; band2 < numBands; ++band2)
         {
            pData = pPixel;
            for (band1 = band2; band1 < numBands; ++band1)
            {
               pInput->mpMatrix[band2][band1] += (*pPixel-pAverage[band2]) * (*pData-pAverage[band1]);
               ++pData;
            }
            ++pPixel;
         }
         ++mask;
         ++it;
      }
   }

   // check if aborted
   if ((pInput->mpAbortFlag == NULL) || !(*pInput->mpAbortFlag))
   {
      for (band2 = 0; band2 < numBands; ++band2)
      {
         for (band1 = band2; band1 < numBands; ++band1)
         {
            pInput->mpMatrix[band2][band1] /= lCount;
         }
      }

      for (band2 = 0; band2 < numBands; ++band2)
      {
         for (band1 = band2; band1 < numBands; ++band1)
         {
            pInput->mpMatrix[band1][band2] = pInput->mpMatrix[band2][band1];
         }
      }
   }

   if (pInput->mpProgress != NULL)
   {
      if ((pInput->mpAbortFlag == NULL) || !(*pInput->mpAbortFlag))
      {
         pInput->mpProgress->updateProgress("Covariance Matrix Complete", 100, NORMAL);
      }
      else
      {
         pInput->mpProgress->updateProgress("Aborted computing Covariance Matrix", 0, ABORT);
      }
   }
}

template<class T>
void ComputePcaValue(T *pData, double* pPcaValue, double *pCoefficients, unsigned int numBands)
{
   T* pInput = pData;
   double* pCoef = pCoefficients;
   *pPcaValue = 0.0;

   for (unsigned int i = 0; i < numBands; ++i)
   {
      *pPcaValue += static_cast<double>(*pInput) * (*pCoef);
      ++pInput;
      ++pCoef;
   }
}

template <class T>
void ComputePcaRow(T* pData, double* pPcaData,  double* pCoefficients, unsigned int numCols, unsigned int numBands,
                   double* pMinValue, double* pMaxValue)
{
   T* pColumn = pData;
   T* pInput = NULL;
   double* pValue = pPcaData;
   double* pCoef = NULL;

   for (unsigned int col = 0; col < numCols; ++col)
   {
      *pValue = 0.0;
      pInput = pColumn;
      pCoef = pCoefficients;
      for (unsigned int band = 0; band < numBands; ++band)
      {
         *pValue += (*pCoef * static_cast<double>(*pInput));
         ++pCoef;
         ++pInput;
      }
      if (*pValue > *pMaxValue)
      {
         *pMaxValue = *pValue;
      }

      if (*pValue < *pMinValue)
      {
         *pMinValue = *pValue;
      }

      ++pValue;
      pColumn += numBands;
   }
}

template <class T>
void StorePcaRow(T* pPcaData, double* pCompValues, unsigned int numCols, unsigned int numComponents, double minVal,
                 double scaleFactor)
{
   T* pOutput = pPcaData;
   double* pInput = pCompValues;
   for (unsigned int col = 0; col < numCols; ++col)
   {
      *pOutput = static_cast<T>((*pInput - minVal) * scaleFactor + 0.5);
      ++pInput;
      pOutput += numComponents;
   }
}

template <class T>
void StorePcaValue(T* pPcaData, double* pValue, double* pMinVal, double* pScaleFactor)
{
   *pPcaData = static_cast<T>((*pValue - *pMinVal) * (*pScaleFactor) + 0.5);
}

REGISTER_PLUGIN_BASIC(OpticksPCA, PCA);

PCA::PCA() :
   mUseEigenValPlot(false),
   mMaxScaleValue(0),
   mpAoiBitMask(NULL),
   mUseAoi(false),
   mDisplayResults(true),
   mNumRows(0),
   mNumColumns(0),
   mNumBands(0),
   mpMatrixValues(NULL),
   mNumComponentsToUse(0),
   mpProgress(NULL),
   mpView(NULL),
   mpRaster(NULL),
   mpPCARaster(NULL),
   mpSecondMomentMatrix(NULL),
   mpCovarianceMatrix(NULL),
   mpStep(NULL),
   mCalcMethod(SECONDMOMENT)
{
   setName("Principal Component Analysis");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("PCA");
   setDescription("Run Principal Component Analysis on data cube.");
   setMenuLocation("[General Algorithms]\\Principal Component Analysis");
   setDescriptorId("{7D2F39B3-31BA-4ef1-B326-7ADCD7F92186}");
   allowMultipleInstances(true);
   setAbortSupported(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

PCA::~PCA()
{
   if (mpAoiBitMask != NULL)
   {
      mpObjFact->destroyObject(mpAoiBitMask, "BitMask");
   }
}

bool PCA::getInputSpecification(PlugInArgList*& pArgList)
{
   // Set up list
   pArgList = mpPlugInMgr->getPlugInArgList();
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<Progress>(ProgressArg(), NULL));
   VERIFY(pArgList->addArg<RasterElement>(DataElementArg(), NULL));

   if (isBatch()) // need additional info in batch mode
   {
      /*
         Cases for batch mode:
         1) Use PCA File
            a) SMM
            b) Covariance
            c) Correlation Coefficients
         2) Don't Use PCA File
            a) AOI
               i) SMM
                  A) Use SMM File / Don't use
            b) No AOI
               i) SMM
                  A) Use SMM File / Don't use
      */

      VERIFY(pArgList->addArg<bool>("Use Transform File", NULL));
      VERIFY(pArgList->addArg<Filename>("Transform Filename", NULL));
      VERIFY(pArgList->addArg<string>("Transform Type", NULL));
      VERIFY(pArgList->addArg<bool>("Use AOI", false));
      VERIFY(pArgList->addArg<string>("AOI Name", false));
      VERIFY(pArgList->addArg<int>("Components", NULL));
      VERIFY(pArgList->addArg<EncodingType>("Output Encoding Type", NULL));
      VERIFY(pArgList->addArg<int>("Max Scale Value", NULL));
      VERIFY(pArgList->addArg<RasterElement>("Second Moment Matrix", NULL));
      VERIFY(pArgList->addArg<RasterElement>("Covariance Matrix", NULL));
      VERIFY(pArgList->addArg<bool>("Display Results", false));
   }

   return true;
}

bool PCA::getOutputSpecification(PlugInArgList*& pArgList)
{
   // Set up list
   pArgList = mpPlugInMgr->getPlugInArgList();
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<SpatialDataView>(ViewArg(), NULL));
   VERIFY(pArgList->addArg<RasterElement>("Corrected Data Cube", NULL));
   return true;
}

bool PCA::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Perform PCA", "app", "74394A49-702A-4609-83B9-69363E28488D");
   mpStep = pStep.get();

   try
   {
      bool bLoadFromFile = false;
      QString message;
      QString transformFilename;
      if (!extractInputArgs(pInArgList))
      {
         pStep->finalize(Message::Failure, "Unable to extract arguments.");
         return false;
      }

      if (mpRaster == NULL)
      {
         pStep->finalize(Message::Failure, "No raster element available.");
         return false;
      }

      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if (pDescriptor == NULL)
      {
         pStep->finalize(Message::Failure, "PCA received null pointer to the data descriptor from the raster element");
         return false;
      }

      mNumRows = pDescriptor->getRowCount();
      mNumColumns = pDescriptor->getColumnCount();
      mNumBands = pDescriptor->getBandCount();

      vector<string> aoiNames = mpModel->getElementNames(mpRaster, TypeConverter::toString<AoiElement>());
      int iResult = 0;

      if (!isBatch())
      {
         PcaDlg dlg(aoiNames, mNumBands, mpDesktop->getMainWidget());

         bool inputValid = false;
         while (!inputValid)
         {
            if (dlg.exec() == QDialog::Rejected)
            {
               pStep->finalize(Message::Abort);
               return false;
            }

            mUseEigenValPlot = dlg.selectNumComponentsFromPlot();
            mNumComponentsToUse = dlg.getNumComponents();
            mOutputDataType = dlg.getOutputDataType();
            mMaxScaleValue = dlg.getMaxScaleValue();

            transformFilename = dlg.getTransformFilename();
            if (!transformFilename.isEmpty())
            {
               bLoadFromFile = true;
            }
            else
            {
               QString strMethod = dlg.getCalcMethod();
               if (strMethod.contains("second", Qt::CaseInsensitive) > 0)
               {
                  mCalcMethod = SECONDMOMENT;
               }
               else if (strMethod.contains("covariance", Qt::CaseInsensitive) > 0)
               {
                  mCalcMethod = COVARIANCE;
               }
               else
               {
                  mCalcMethod = CORRCOEF;
               }
            }

            mRoiName = dlg.getRoiName();
            if (!mRoiName.isEmpty())
            {
               pStep->addProperty("AOI", mRoiName.toStdString());
               // check if any pixels are selected in the AOI
               AoiElement* pAoi = getAoiElement(mRoiName.toStdString());
               VERIFY(pAoi != NULL);
               const BitMask* pMask = pAoi->getSelectedPoints();
               int numPoints = pMask->getCount();
               if (numPoints > 0 || pAoi->getAllPointsToggled())
               {
                  mUseAoi = true;
                  inputValid = true;
                  mpAoiBitMask = reinterpret_cast<BitMask*>(mpObjFact->createObject("BitMask"));
                  mpAoiBitMask->merge(*pMask);
               }
               else
               {
                  message = "No pixels are currently selected in AOI";
                  QMessageBox::critical( NULL, "PCA", message );
                  pStep->finalize(Message::Failure, message.toStdString());
                  inputValid = false;
               }
            }
            else  // use whole image
            {
               mRoiName = "Whole Image";
               mUseAoi = false;
               inputValid = true;
            }
         }
      }
      else // batch mode
      {
         // extract batch-mode only args
         string xformType;
         pInArgList->getPlugInArgValue("Transform Type", xformType);
         if (xformType == "Second Moment")
         {
            mCalcMethod = SECONDMOMENT;
         }
         else if (xformType == "Covariance")
         {
            mCalcMethod = COVARIANCE;
         }
         else if (xformType == "Correlation Coefficient")
         {
            mCalcMethod = CORRCOEF;
         }
         else
         {
            pStep->finalize(Message::Failure, "Bad Transform Type!");
            return false;
         }

         pInArgList->getPlugInArgValue("Use Transform File", bLoadFromFile);
         if (bLoadFromFile)
         {
            Filename* pFn = pInArgList->getPlugInArgValue<Filename>("Transform Filename");
            if (pFn == NULL)
            {
               pStep->finalize(Message::Failure, "No Transform Filename specified!");
               return false;
            }
            transformFilename = QString::fromStdString(pFn->getFullPathAndName());
         }

         // arg extraction so we can continue and do the actual calculations
         PlugInArg* pComponentsArg = NULL;
         int numComponents = static_cast<int>(mNumBands);
         if (pInArgList->getArg("Components", pComponentsArg) == true &&
            pComponentsArg != NULL && pComponentsArg->isActualSet() &&
            pInArgList->getPlugInArgValue("Components", numComponents) == false)
         {
            pStep->finalize(Message::Failure, "Unable to determine number of components.");
            return false;
         }

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Number of Components input argument " \
   "should be an unsigned int (dadkins)")
         mNumComponentsToUse = static_cast<unsigned int>(numComponents);
         if (mNumComponentsToUse <= 0 || mNumBands < mNumComponentsToUse)
         {
            pStep->finalize(Message::Failure, "Invalid number of components specified!");
            return false;
         }

         //set default condition
         pInArgList->getPlugInArgValue<bool>("Use AOI", mUseAoi);
         if (mUseAoi == true)
         {
            bool aoiFailure = false;
            string roiName;
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : AOI Name should be an AoiElement (dadkins)")
            if (pInArgList->getPlugInArgValue("AOI Name", roiName) == false)
            {
               pStep->finalize(Message::Failure, "Must specify the AOI name when choosing to process over an AOI!");
               return false;
            }

            // process the AOI
            mRoiName = QString::fromStdString(roiName);
            if (mRoiName.isEmpty())
            {
               aoiFailure = true;
            }
            else  // use the AOI
            {
               // check if any pixels are selected in the AOI
               AoiElement* pAoi = getAoiElement(mRoiName.toStdString());
               if (pAoi == NULL)
               {
                  pStep->finalize(Message::Failure, "Specified AOI does not exist.");
                  return false;
               }

               const BitMask* pMask = pAoi->getSelectedPoints();
               if ((pMask != NULL) && (pMask->getCount() > 0))
               {
                  mpAoiBitMask = reinterpret_cast<BitMask*>(mpObjFact->createObject("BitMask"));
                  mpAoiBitMask->merge(*pMask);
               }
               else
               {
                  aoiFailure = true;
               }
            }

            if (aoiFailure)
            {
               // if any AOI loading failure, use the whole image!
               mRoiName = "Whole Image";
               mUseAoi = false;
            }

         }

         // output data type argument
         PlugInArg* pEncodingTypeArg = NULL;
         mOutputDataType = pDescriptor->getDataType();
         if (pInArgList->getArg("Output Encoding Type", pEncodingTypeArg) == true &&
            pEncodingTypeArg != NULL && pEncodingTypeArg->isActualSet() &&
            pInArgList->getPlugInArgValue("Output Encoding Type", mOutputDataType) == false)
         {
            pStep->finalize(Message::Failure, "Unable to determine output encoding type.");
            return false;
         }

         if (mOutputDataType.isValid() == false)
         {
            pStep->finalize(Message::Failure, "Invalid output encoding type.");
            return false;
         }

         if (pInArgList->getPlugInArgValue("Max Scale Value", mMaxScaleValue) == false)
         {
            pStep->finalize(Message::Failure, "Invalid Maximum Scale Value!");
            return false;
         }

         int maxThreshold = 0;
         switch (mOutputDataType)
         {
         case INT1SBYTE:
            maxThreshold = numeric_limits<char>::max();
            break;
         case INT1UBYTE:
            maxThreshold = numeric_limits<unsigned char>::max();
            break;
         case INT2SBYTES:
            maxThreshold = numeric_limits<short>::max();
            break;
         case INT2UBYTES:
            maxThreshold = numeric_limits<unsigned short>::max();
            break;
         default:
            maxThreshold = numeric_limits<int>::max();
            break;
         }

         if (mMaxScaleValue > maxThreshold)
         {
            pStep->finalize(Message::Failure, "Bad Maximum Scale Value!");
            return false;
         }

         // Second Moment RasterElement
         mpSecondMomentMatrix = pInArgList->getPlugInArgValue<RasterElement>("Second Moment Matrix");

         // Covariance RasterElement
         mpCovarianceMatrix = pInArgList->getPlugInArgValue<RasterElement>("Covariance Matrix");

         // Display Results
         VERIFY(pInArgList->getPlugInArgValue("Display Results", mDisplayResults));
      }

      // log PCA options
      if (bLoadFromFile)
      {
         pStep->addProperty("PCA File", transformFilename.toStdString());
      }
      else
      {
         pStep->addProperty("Number of Components", mNumComponentsToUse);
         pStep->addProperty("Calculation Method", 
            (mCalcMethod == SECONDMOMENT ? "Second Moment" :
                        (mCalcMethod == COVARIANCE ? "Covariance" : "Correlation Coefficient")));
      }

      // create array for component coefficients
      MatrixFunctions::MatrixResource<double> pMatrixValues(mNumBands, mNumBands);
      mpMatrixValues = pMatrixValues;
      if (!mpMatrixValues)
      {
         pStep->finalize(Message::Failure, "Unable to obtain memory needed to calculate PCA coefficients");
         return false;
      }
      if (bLoadFromFile)
      {
         if (!readInPCAtransform(transformFilename))
         {
            if (isAborted())
            {
               pStep->finalize(Message::Abort);
            }
            else
            {
               pStep->finalize(Message::Failure, "Error loading transform file");
            }

            return false;
         }
      }
      else
      {
         // get statistics to use for PCA
         if (!getStatistics(aoiNames))
         {
            if (isAborted())
            {
               pStep->finalize(Message::Abort);
            }
            else
            {
               pStep->finalize(Message::Failure, "Error determining statistics");
            }

            return false;
         }

         // Calculate PCA coefficients
         calculateEigenValues();
         if (isAborted())
         {
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress("PCA Aborted", 0, ABORT);
            }

            pStep->finalize(Message::Abort);
            return false;
         }

         // Save PCA transform
         QString filename = QString::fromStdString(mpRaster->getFilename());

         switch (mCalcMethod)
         {
         case SECONDMOMENT:
            filename += ".pcasmm";
            break;
         case COVARIANCE:
            filename += ".pcacvm";
            break;
         case CORRCOEF:
            filename += ".pcaccm";
            break;
         default:
            filename += ".pca";
            break;
         }
         if (!isBatch())
         {
            filename = QFileDialog::getSaveFileName(NULL, "Choose filename to save PCA Transform",
               filename, "PCA files (*.pca*);;All Files (*.*)");
         }
         if (!filename.isEmpty())
         {
            writeOutPCAtransform(filename);
         }
      }

      // Create the PCA sensor data
      if (!createPCACube())
      {
         if (isAborted())
         {
            pStep->finalize(Message::Abort);
         }
         else
         {
            pStep->finalize(Message::Failure, "Error allocating result cube");
         }

         return false;
      }

      // compute PCAcomponents
      bool bSuccess;
      if (mUseAoi)
      {
         bSuccess = computePCAaoi();
      }
      else
      {
         bSuccess = computePCAwhole();
      }

      if (!bSuccess)
      {
         mpModel->destroyElement(mpPCARaster);
         if (isAborted())
         {
            pStep->finalize(Message::Abort);
         }
         else
         {
            pStep->finalize(Message::Failure, "Error computing PCA components");
         }

         return false;
      }

      // Create the spectral data window
      if (!createPCAView())
      {
         if (mpPCARaster != NULL)
         {
            mpModel->destroyElement(mpPCARaster);
            if (isAborted())
            {
               pStep->finalize(Message::Abort);
            }
            else
            {
               pStep->finalize(Message::Failure, "Error creating view");
            }

            return false;
         }
      }

      // Set the values in the output arg list
      if (pOutArgList != NULL)
      {
         VERIFY(pOutArgList->setPlugInArgValue(ViewArg(), mpView));
         VERIFY(pOutArgList->setPlugInArgValue("Corrected Data Cube", mpPCARaster));
      }

      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Principle Component Analysis completed", 100, NORMAL);
      }
   }
   catch (const bad_alloc&)
   {
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Out of memory", 0, ERRORS);
      }
      pStep->finalize(Message::Failure, "Out of memory");

      return false;
   }

   pStep->finalize(Message::Success);
   return true;
}

bool PCA::abort()
{
   Executable* pSecondMoment = dynamic_cast<Executable*>(mpSecondMoment->getPlugIn());
   Executable* pCovariance = dynamic_cast<Executable*>(mpCovariance->getPlugIn());
   if (pSecondMoment != NULL)
   {
      pSecondMoment->abort();
   }
   if (pCovariance != NULL)
   {
      pCovariance->abort();
   }

   return ExecutableShell::abort();
}

bool PCA::extractInputArgs(const PlugInArgList* pArgList)
{
   if (pArgList == NULL)
   {
      mpStep->finalize(Message::Failure, "PCA received null input argument list");
      return false;
   }

   mpProgress = pArgList->getPlugInArgValue<Progress>(ProgressArg());
   mpRaster = pArgList->getPlugInArgValue<RasterElement>(DataElementArg());
   if (mpRaster == NULL)
   {
      mMessage = "The input raster element was null";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }
      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      mMessage = "Unable to access data descriptor for original data set!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }
      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   EncodingType dataType = pDescriptor->getDataType();
   if ((dataType == INT4SCOMPLEX) || (dataType == FLT8COMPLEX))
   {
      mMessage = "Complex data is not supported!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }
      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   if (pDescriptor->getBandCount() == 1)
   {
      mMessage = "Cannot perform PCA on 1 band data!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }
      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   return true;
}

bool PCA::createPCACube()
{
   string outputName = mpRaster->getFilename();
   if (outputName.empty() == true)
   {
      outputName = mpRaster->getName();
      if (outputName.empty() == true)
      {
         mMessage = "Could not access the cube's name!";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 0, ERRORS);
         }

         mpStep->finalize(Message::Failure, mMessage);
         return false;
      }
   }

   StepResource pStep(mMessage, "app", "2EA8BA45-E826-4bb1-AB94-968D29184F80", "Can't create spectral cube");

   int loc = outputName.rfind('.');
   if (loc == string::npos)
   {
      loc = outputName.length();
   }
   string addOn = "";
   switch (mCalcMethod)
   {
   case SECONDMOMENT:
      addOn = "_pca_smm";
      break;
   case COVARIANCE:
      addOn = "_pca_cvm";
      break;
   case CORRCOEF:
      addOn = "_pca_ccm";
      break;
   default:
      addOn = "_pca";
      break;
   }
   outputName = outputName.insert(loc, addOn);

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      mMessage = "Could not access the cube's data descriptor!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   ProcessingLocation outLocation = pDescriptor->getProcessingLocation();

   mpPCARaster = RasterUtilities::createRasterElement(outputName, mNumRows, mNumColumns,
      mNumComponentsToUse, mOutputDataType, BIP, outLocation == IN_MEMORY, NULL);

   if (mpPCARaster == NULL)
   {
      mMessage = "Unable to create a new raster element!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   // copy classification from mpRaster since mpPCARaster was created with NULL parent
   mpPCARaster->copyClassification(mpRaster);

   // Bad values
   RasterDataDescriptor* pRdd = dynamic_cast<RasterDataDescriptor*>(mpPCARaster->getDataDescriptor());
   if (pRdd == NULL)
   {
      mMessage = "Unable to create a new raster element!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }
   vector<int>badValue(1);
   badValue[0] = 0;
   pRdd->setBadValues(badValue);

   // TODO: Units

   pStep->finalize(Message::Success);
   return true;
}

bool PCA::computePCAwhole()
{
   QString message;

   const RasterDataDescriptor* pPcaDesc = dynamic_cast<RasterDataDescriptor*>(mpPCARaster->getDataDescriptor());
   EncodingType pcaDataType = pPcaDesc->getDataType();
   unsigned int pcaNumRows = pPcaDesc->getRowCount();
   unsigned int pcaNumCols = pPcaDesc->getColumnCount();
   unsigned int pcaNumBands = pPcaDesc->getBandCount();
   if ((pcaNumRows != mNumRows) || (pcaNumCols != mNumColumns) || (pcaNumBands != mNumComponentsToUse))
   {
      mMessage = "The dimensions of the PCA RasterElement are not correct.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   FactoryResource<DataRequest> pBipRequest;
   pBipRequest->setInterleaveFormat(BIP);
   DataAccessor pcaAccessor = mpPCARaster->getDataAccessor(pBipRequest->copy());
   if (!pcaAccessor.isValid())
   {
      mMessage = "PCA could not obtain an accessor the PCA RasterElement";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   const RasterDataDescriptor* pOrigDescriptor = dynamic_cast<const RasterDataDescriptor*>
      (mpRaster->getDataDescriptor());
   if (pOrigDescriptor == NULL)
   {
      mMessage = "PCA received null pointer to the source data descriptor";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   EncodingType eDataType = pOrigDescriptor->getDataType();
   if (!eDataType.isValid())
   {
      mMessage = "PCA received invalid value for source data encoding type";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   // Initialize progress bar variables
   int currentProgress = 0;
   int count = 0;

   void* pPCAData = NULL;
   void* pOrigData = NULL;

   DataAccessor origAccessor = mpRaster->getDataAccessor(pBipRequest->copy());
   if (!origAccessor.isValid())
   {
      mMessage = "Could not get the pixels in the original cube!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, currentProgress, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   // Set the PCA cube data values
   double min = 0.0;
   double max = 0.0;
   double scalefactor = 0.0;
   int memsize = mNumRows * mNumColumns * sizeof(float);
   string compValuesName = "PcaComponentValues";
   RasterElement* pOldComponentValues = dynamic_cast<RasterElement*>(
      Service<ModelServices>()->getElement(compValuesName, TypeConverter::toString<RasterElement>(), mpRaster));
   if (pOldComponentValues != NULL)
   {
      VERIFY(Service<ModelServices>()->destroyElement(pOldComponentValues));
   }

   ModelResource<RasterElement> pComponentValues(RasterUtilities::createRasterElement(compValuesName,
      mNumRows, mNumColumns, FLT8BYTES, true, mpRaster));
   if (pComponentValues.get() == NULL)
   {
      mMessage = "Out of memory";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   FactoryResource<DataRequest> pBipWritableRequest;
   pBipWritableRequest->setWritable(true);
   pBipWritableRequest->setInterleaveFormat(BIP);

   unsigned int comp = 0;
   unsigned int row = 0;
   unsigned int band = 0;
   double* pValues = NULL;
   vector<double> coefficients(mNumBands, 0);
   double* pCoefficients = &coefficients.front();
   int progSave = 0;
   for (comp = 0; comp < mNumComponentsToUse; ++comp)
   {
      origAccessor = mpRaster->getDataAccessor(pBipRequest->copy());

      DataAccessor compValAccessor = pComponentValues->getDataAccessor(pBipWritableRequest->copy());
      for (band = 0; band < mNumBands; ++band)
      {
         pCoefficients[band] = mpMatrixValues[band][comp];
      }
      min = numeric_limits<double>::max();
      max = numeric_limits<double>::min();
      for (row = 0; row < mNumRows; ++row)
      {
         VERIFY(origAccessor.isValid());
         pOrigData = origAccessor->getRow();
         VERIFY(compValAccessor.isValid());
         pValues = reinterpret_cast<double*>(compValAccessor->getRow());
         switchOnEncoding(eDataType, ComputePcaRow, pOrigData, pValues,
            pCoefficients, mNumColumns, mNumBands, &min, &max);
         origAccessor->nextRow();
         compValAccessor->nextRow();
      }

      // check if aborted
      if (!isAborted())
      {
         // scale component values and save in pPCACube
         scalefactor = static_cast<double>(mMaxScaleValue) / (max - min);

         FactoryResource<DataRequest> pPcaRequest;
         pPcaRequest->setBands(pPcaDesc->getActiveBand(comp), pPcaDesc->getActiveBand(comp));
         pPcaRequest->setWritable(true);
         DataAccessor pcaAccessor = mpPCARaster->getDataAccessor(pPcaRequest.release());
         if (!pcaAccessor.isValid())
         {
            mMessage = "Could not get the pixels in the PCA cube!";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(mMessage, currentProgress, ERRORS);
            }

            mpStep->finalize(Message::Failure, mMessage);
            return false;
         }

         compValAccessor = pComponentValues->getDataAccessor(pBipRequest->copy());
         for (row = 0; row < mNumRows; ++row)
         {
            VERIFY(pcaAccessor.isValid());
            pPCAData = pcaAccessor->getRow();
            VERIFY(compValAccessor.isValid());
            pValues = reinterpret_cast<double*>(compValAccessor->getRow());
            switchOnEncoding(mOutputDataType, StorePcaRow, pPCAData, pValues, pcaNumCols, pcaNumBands,
               min, scalefactor);
            pcaAccessor->nextRow();
            compValAccessor->nextRow();

            if (isAborted())
            {
               break;
            }
         }

         currentProgress = 100 * (comp + 1) / mNumComponentsToUse;
         if (mpProgress != NULL && currentProgress != progSave)
         {
            progSave = currentProgress;
            mpProgress->updateProgress("Generating scaled PCA data cube...", currentProgress, NORMAL);
         }
      }
   }

   if (isAborted())
   {
      mpProgress->updateProgress("PCA aborted!", currentProgress, ABORT);
      mpStep->finalize(Message::Abort);
      return false;
   }
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("PCA computations complete!", 100, NORMAL);
   }

   return true;
}

bool PCA::computePCAaoi()
{
   const RasterDataDescriptor* pPcaDescriptor = dynamic_cast<const RasterDataDescriptor*>(
      mpPCARaster->getDataDescriptor());
   if (pPcaDescriptor == NULL)
   {
      mMessage = "PCA received null pointer to the PCA data RaterElement";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   const RasterDataDescriptor* pOrigDescriptor = dynamic_cast<const RasterDataDescriptor*>
      (mpRaster->getDataDescriptor());
   if (pOrigDescriptor == NULL)
   {
      mMessage = "PCA received null pointer to the source data RasterElement";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   EncodingType eDataType = pOrigDescriptor->getDataType();
   if (!eDataType.isValid())
   {
      mMessage = "PCA received invalid value for source data encoding type";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   // Initialize progress bar variables
   int currentProgress = 0;
   int count = 0;

   void* pPCAData = NULL;
   void* pOrigData = NULL;

   // Set the PCA cube data values
   double min = 0.0;
   double max = 0.0;
   double scalefactor = 0.0;
   int x1 = 0;
   int y1 = 0;
   int x2 = 0;
   int y2 = 0;
   BitMaskIterator it(mpAoiBitMask, mpRaster);
   it.getBoundingBox(x1, y1, x2, y2);

   int numRows = y2 - y1 + 1;
   int numCols = x2 - x1 + 1;

   string compValuesName = "PcaComponentValues";
   RasterElement* pOldComponentValues = dynamic_cast<RasterElement*>(
      Service<ModelServices>()->getElement(compValuesName, TypeConverter::toString<RasterElement>(), mpRaster));
   if (pOldComponentValues != NULL)
   {
      VERIFY(Service<ModelServices>()->destroyElement(pOldComponentValues));
   }

   ModelResource<RasterElement> pComponentValues(RasterUtilities::createRasterElement(compValuesName,
      numRows, numCols, FLT8BYTES, true, mpRaster));
   if (pComponentValues.get() == NULL)
   {
      mMessage = "Out of memory";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, currentProgress, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   int comp = 0;
   int band = 0;
   vector<double> coefficients(mNumBands, 0);
   double* pCoefficients = &coefficients.front();
   double* pTempVal = NULL;
   int progSave = 0;
   for (comp = 0; comp < static_cast<int>(mNumComponentsToUse); ++comp)
   {
      FactoryResource<DataRequest> pRequest;
      pRequest->setInterleaveFormat(BIP);
      pRequest->setRows(pOrigDescriptor->getActiveRow(y1), pOrigDescriptor->getActiveRow(y2));
      pRequest->setColumns(pOrigDescriptor->getActiveColumn(x1), pOrigDescriptor->getActiveColumn(x2));
      pRequest->setBands(pOrigDescriptor->getActiveBand(comp), pOrigDescriptor->getActiveBand(comp));
      DataAccessor accessor = mpRaster->getDataAccessor(pRequest.release());
      if (!accessor.isValid())
      {
         mMessage = "Could not get the pixels in the original cube!";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, currentProgress, ERRORS);
         }

         mpStep->finalize(Message::Failure, mMessage);
         return false;
      }

      FactoryResource<DataRequest> pWritableRequest;
      pWritableRequest->setWritable(true);
      DataAccessor compValAccessor = pComponentValues->getDataAccessor(pWritableRequest.release());

      for (band = 0; band < static_cast<int>(mNumBands); ++band)
      {
         pCoefficients[band] = mpMatrixValues[band][comp];
      }
      min = numeric_limits<double>::max();
      max = numeric_limits<double>::min();

      it.firstPixel();
      LocationType loc;

      while (it != it.end())
      {
         it.getPixelLocation(loc);
         accessor->toPixel(loc.mY, loc.mX);
         compValAccessor->toPixel(loc.mY - y1, loc.mX - x1);
         VERIFY(accessor.isValid());
         VERIFY(compValAccessor.isValid());

         pTempVal = reinterpret_cast<double*>(compValAccessor->getColumn());
         pOrigData = accessor->getColumn();
         if (pOrigData == NULL)
         {
               mMessage = "Could not get the pixels in the Original cube!";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(mMessage, currentProgress, ERRORS);
            }

            mpStep->finalize(Message::Failure, mMessage);
            return false;
         }

         switchOnEncoding(eDataType, ComputePcaValue, pOrigData, pTempVal, pCoefficients, mNumBands);

         if (*pTempVal > max)
         {
            max = *pTempVal;
         }
         if (*pTempVal < min)
         {
            min = *pTempVal;
         }
         if (isAborted())
         {
            break;
         }
          ++it;
      }

      if (!isAborted())
      {
         scalefactor = static_cast<double>(mMaxScaleValue) / (max - min);

         FactoryResource<DataRequest> pPcaRequest;
         pPcaRequest->setRows(pPcaDescriptor->getActiveRow(y1), pPcaDescriptor->getActiveRow(y2));
         pPcaRequest->setColumns(pPcaDescriptor->getActiveColumn(x1), pPcaDescriptor->getActiveColumn(x2));
         pPcaRequest->setBands(pPcaDescriptor->getActiveBand(comp), pPcaDescriptor->getActiveBand(comp));
         pPcaRequest->setWritable(true);
         DataAccessor pcaAccessor = mpPCARaster->getDataAccessor(pPcaRequest.release());

         if (!pcaAccessor.isValid())
         {
            mMessage = "Could not get the pixels in the PCA cube!";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(mMessage, currentProgress, ERRORS);
            }

            mpStep->finalize(Message::Failure, mMessage);
            return false;
         }

         it.firstPixel();
         compValAccessor = pComponentValues->getDataAccessor();
         while (it != it.end())
         {
            it.getPixelLocation(loc);
            pcaAccessor->toPixel(loc.mY, loc.mX);
            compValAccessor->toPixel(loc.mY - y1, loc.mX - x1);
            VERIFY(pcaAccessor.isValid());
            VERIFY(compValAccessor.isValid());
            pPCAData = pcaAccessor->getColumn();
            pTempVal = reinterpret_cast<double*>(compValAccessor->getColumn());
            if (pPCAData == NULL)
            {
               mMessage = "Could not get the pixels in the PCA cube!";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(mMessage, currentProgress, ERRORS);
               }

               mpStep->finalize(Message::Failure, mMessage);
               return false;
            }

            switchOnEncoding(mOutputDataType, StorePcaValue, pPCAData, pTempVal, &min, &scalefactor);
            ++it;
         }
         if (isAborted())
         {
            break;
         }
      }

      currentProgress = 100 * (comp + 1) / mNumComponentsToUse;
      if (mpProgress != NULL && currentProgress != progSave)
      {
         progSave = currentProgress;
         mpProgress->updateProgress("Generating scaled PCA data cube...", currentProgress, NORMAL);
      }
   }

   if (isAborted())
   {
      mpProgress->updateProgress("PCA aborted!", currentProgress, ABORT);
      mpStep->finalize(Message::Abort);
      return false;
   }

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("PCA computations complete!", 99, NORMAL);
   }

   return true;
}

bool PCA::createPCAView()
{
   if (mDisplayResults)
   {
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Creating view...", 0, NORMAL);
      }

      string filename = mpPCARaster->getName();

      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Creating view...", 25, NORMAL);
      }

      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(
         mpDesktop->createWindow(filename.c_str(), SPATIAL_DATA_WINDOW));
      if (pWindow == NULL)
      {
         mMessage = "Could not create new window!";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 25, ERRORS);
         }

         mpStep->finalize(Message::Failure, mMessage);
         return false;
      }

      if (pWindow != NULL)
      {
         mpView = pWindow->getSpatialDataView();
      }

      if (mpView == NULL)
      {
         mMessage = "Could not obtain new view!";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 25, ERRORS);
         }

         mpStep->finalize(Message::Failure, mMessage);
         return false;
      }

      mpView->setPrimaryRasterElement(mpPCARaster);

      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Creating view...", 50, NORMAL);
      }

      Layer* pLayer = NULL;
      {
         UndoLock lock(mpView);
         pLayer = mpView->createLayer(RASTER, mpPCARaster);
      }
      if (pLayer == NULL)
      {
         mpDesktop->deleteWindow(pWindow);
         mpPCARaster = NULL;
         mMessage = "Could not access raster properties for view!";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 50, ERRORS);
         }

         mpStep->finalize(Message::Failure, mMessage);
         return false;
      }

      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Creating view...", 75, NORMAL);
      }

      // Create a GCP layer if available
      if (mpRaster != NULL)
      {
         UndoLock lock(mpView);
         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            const RasterFileDescriptor* pFileDescriptor =
               dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
            if (pFileDescriptor != NULL)
            {
               Service<ModelServices> pModel;
               if (pModel.get() != NULL)
               {
                  list<GcpPoint> gcps;
                  if ((mNumRows == pFileDescriptor->getRowCount()) &&
                     (mNumColumns == pFileDescriptor->getColumnCount()))
                  {
                     gcps = pFileDescriptor->getGcps();
                  }

                  if (gcps.empty() == true)
                  {
                     if (mpRaster->isGeoreferenced())
                     {
                        GcpPoint gcp;

                        // Lower left
                        gcp.mPixel.mX = 0.0;
                        gcp.mPixel.mY = 0.0;
                        gcp.mCoordinate = mpRaster->convertPixelToGeocoord(gcp.mPixel);
                        gcps.push_back(gcp);

                        // Lower right
                        gcp.mPixel.mX = mNumColumns - 1;
                        gcp.mPixel.mY = 0.0;
                        gcp.mCoordinate = mpRaster->convertPixelToGeocoord(gcp.mPixel);
                        gcps.push_back(gcp);

                        // Upper left
                        gcp.mPixel.mX = 0.0;
                        gcp.mPixel.mY = mNumRows - 1;
                        gcp.mCoordinate = mpRaster->convertPixelToGeocoord(gcp.mPixel);
                        gcps.push_back(gcp);

                        // Upper right
                        gcp.mPixel.mX = mNumColumns - 1;
                        gcp.mPixel.mY = mNumRows - 1;
                        gcp.mCoordinate = mpRaster->convertPixelToGeocoord(gcp.mPixel);
                        gcps.push_back(gcp);

                        // Center
                        gcp.mPixel.mX = mNumColumns / 2.0;
                        gcp.mPixel.mY = mNumRows / 2.0;
                        gcp.mCoordinate = mpRaster->convertPixelToGeocoord(gcp.mPixel);
                        gcps.push_back(gcp);
                     }
                  }

                  if (gcps.empty() == false)
                  {
                     DataDescriptor* pGcpDescriptor = pModel->createDataDescriptor("Corner Coordinates",
                        "GcpList", mpPCARaster);
                     if (pGcpDescriptor != NULL)
                     {
                        GcpList* pGcpList = static_cast<GcpList*>(pModel->createElement(pGcpDescriptor));
                        if (pGcpList != NULL)
                        {
                           // Add the GCPs to the GCP list
                           pGcpList->addPoints(gcps);

                           // Create the GCP list layer
                           mpView->createLayer(GCP_LAYER, pGcpList);
                        }
                     }
                  }
                  else
                  {
                     string message = "Geocoordinates are not available and will not be added to the new PCA cube!";
                     if (mpProgress != NULL)
                     {
                        mpProgress->updateProgress(message, 0, WARNING);
                     }

                     if (mpStep != NULL)
                     {
                        mpStep->addMessage(message, "app", "FCD1A3B0-9CA3-41D5-A93B-D9D0DBE0222C", true);
                     }
                  }
               }
            }
         }
      }

      if (!isAborted())
      {
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress("Finished creating view...", 100, NORMAL);
         }
      }
      else
      {
         mpDesktop->deleteWindow(pWindow);
         mpPCARaster = NULL;
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress("Create view aborted", 100, NORMAL);
         }

         mpStep->finalize(Message::Abort);
         return false;
      }
   }

   return true;
}

void PCA::calculateEigenValues()
{
   StepResource pStep("Calculate Eigen Values", "app", "640DF72A-BBFC-4f17-877A-058C6B70B701");

   unsigned int lBandIndex;
   vector<double> eigenValues(mNumBands);
   double* pEigenValues = &eigenValues.front();

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Calculating Eigen Values...", 0, NORMAL);
   }

   // Get the eigenvalues and eigenvectors. Store the eigenvectors in mpMatrixValues for future use.
   if (MatrixFunctions::getEigenvalues(const_cast<const double**>(mpMatrixValues),
      pEigenValues, mpMatrixValues, mNumBands) == false)
   {
      pStep->finalize(Message::Failure, "Unable to calculate eigenvalues.");
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(pStep->getFailureMessage(), 100, ERRORS);
      }
   }

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Calculating Eigen Values...", 80, NORMAL);
   }

   double dEigen_Sum = 0.0;
   double dEigen_Current = 0.0;
   double dTemp = 0.0;
   int lNoise_Cutoff = 1;
   for (lBandIndex = 0; lBandIndex < mNumBands; ++lBandIndex)
   {
      dEigen_Sum += pEigenValues[lBandIndex];
   }

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Calculating Eigen Values...", 90, NORMAL);
   }

   for (lBandIndex = 0; lBandIndex < mNumBands; ++lBandIndex)
   {
      dEigen_Current += pEigenValues[lBandIndex];
      dTemp = 100.0 * dEigen_Current / dEigen_Sum;
      if (dTemp < 99.99)
      {
         ++lNoise_Cutoff;
      }
   }
   pStep->addProperty("Noise cutoff", lNoise_Cutoff);

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Calculation of Eigen Values completed", 100, NORMAL);
   }

   // check if user wanted to select num components based on Eigen value plot
   if (mUseEigenValPlot)
   {
      EigenPlotDlg plotDlg(mpDesktop->getMainWidget());
      plotDlg.setEigenValues(pEigenValues, mNumBands);
      if (plotDlg.exec() == QDialog::Rejected)
      {
         abort();
      }
      mNumComponentsToUse = plotDlg.getNumComponents();
   }
   pStep->finalize(Message::Success);
}

bool PCA::getStatistics(vector<string> aoiList)
{
   double* pdTemp = NULL;
   QString strFilename;

   string filename = mpRaster->getFilename();
   if (filename.empty() == false)
   {
      strFilename = QString::fromStdString(filename);
   }

   QString message;
   bool bLoadFromFile = false;
   bool success = false;
   switch (mCalcMethod)
   {
   case SECONDMOMENT:
      {
         if (mpSecondMomentMatrix == NULL)
         {
            ExecutableResource secondMoment("Second Moment", "", mpProgress);
            VERIFY(secondMoment->getPlugIn() != NULL);

            bool recalculate = true;
            bool computeInverse(false);
            secondMoment->getInArgList().setPlugInArgValue(DataElementArg(), mpRaster);
            secondMoment->getInArgList().setPlugInArgValue("Recalculate", &recalculate);
            secondMoment->getInArgList().setPlugInArgValue("ComputeInverse", &computeInverse);
            if (mUseAoi == true)
            {
               AoiElement* pAoi = getAoiElement(mRoiName.toStdString());
               secondMoment->getInArgList().setPlugInArgValue("AOI", pAoi);
            }
            mpSecondMoment = secondMoment;
            secondMoment->execute();
            mpSecondMomentMatrix = secondMoment->getOutArgList()
               .getPlugInArgValue<RasterElement>("Second Moment Matrix");
            mpSecondMoment = ExecutableResource();

            if (mpSecondMomentMatrix == NULL)
            {
               message = "Could not obtain Second Moment Matrix";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message.toStdString(), 0, ERRORS);
               }
               mpStep->finalize(Message::Failure, mMessage);
               return false;
            }
         }

         pdTemp = reinterpret_cast<double*>(mpSecondMomentMatrix->getRawData());
         if (pdTemp == NULL)
         {
            mMessage = "Unable to access data in Second Moment Matrix";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(mMessage, 0, ERRORS);
            }
            mpStep->finalize(Message::Failure, mMessage);
            return false;
         }

         int lOffset = 0;
         for (unsigned int row = 0; row < mNumBands; ++row)
         {
            memcpy(mpMatrixValues[row], &pdTemp[lOffset], mNumBands * sizeof(pdTemp[lOffset]));
            lOffset += mNumBands;

            mpProgress->updateProgress("Reordering Second Moment Matrix...", 100 * row / mNumBands, NORMAL);
            if (isAborted())
            {
               break;
            }
         }

         if (!isAborted())
         {
            mpProgress->updateProgress("Matrix retrieval complete", 100, NORMAL);
            success = true;
         }
         else
         {
            mpProgress->updateProgress("Matrix retrieval aborted by user", 100, NORMAL);
            success = false;
         }

         break;
      }

   case COVARIANCE:
      {
         if (mpCovarianceMatrix == NULL)
         {
            ExecutableResource covariance("Covariance", "", mpProgress);
            VERIFY(covariance->getPlugIn() != NULL);

            bool recalculate = true;
            bool computeInverse(false);
            covariance->getInArgList().setPlugInArgValue(DataElementArg(), mpRaster);
            covariance->getInArgList().setPlugInArgValue("Recalculate", &recalculate);
            covariance->getInArgList().setPlugInArgValue("ComputeInverse", &computeInverse);
            if (mUseAoi == true)
            {
               AoiElement* pAoi = getAoiElement(mRoiName.toStdString());
               covariance->getInArgList().setPlugInArgValue("AOI", pAoi);
            }
            mpCovariance = covariance;
            covariance->execute();
            mpCovarianceMatrix = covariance->getOutArgList()
               .getPlugInArgValue<RasterElement>("Covariance Matrix");
            mpCovariance = ExecutableResource();

            if (mpCovarianceMatrix == NULL)
            {
               message = "Could not obtain Covariance Matrix";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message.toStdString(), 0, ERRORS);
               }
               mpStep->finalize(Message::Failure, mMessage);
               return false;
            }
         }

         pdTemp = reinterpret_cast<double*>(mpCovarianceMatrix->getRawData());
         if (pdTemp == NULL)
         {
            mMessage = "Unable to access data in Covariance Matrix";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(mMessage, 0, ERRORS);
            }
            mpStep->finalize(Message::Failure, mMessage);
            return false;
         }

         int lOffset = 0;
         for (unsigned int row = 0; row < mNumBands; ++row)
         {
            memcpy(mpMatrixValues[row], &pdTemp[lOffset], mNumBands * sizeof(pdTemp[lOffset]));
            lOffset += mNumBands;

            mpProgress->updateProgress("Reordering Covariance Matrix...", 100 * row / mNumBands, NORMAL);
            if (isAborted())
            {
               break;
            }
         }

         if (!isAborted())
         {
            mpProgress->updateProgress("Matrix retrieval complete", 100, NORMAL);
            success = true;
         }
         else
         {
            mpProgress->updateProgress("Matrix retrieval aborted by user", 100, NORMAL);
            success = false;
         }

         break;
      }
   case CORRCOEF:
      {
         strFilename += ".ccm";
         if (!isBatch())
         {
            if (QFile::exists(strFilename))
            {
               bLoadFromFile = !QMessageBox::information(NULL, "Correlation Coefficient Algorithm",
                                              "A correlation coefficient matrix file has been found for this image.\n"
                                                      "Do you want to use it?",
                                              "Use",
                                              "Don't Use");
            }
         }
         else
         {
            bLoadFromFile = false;
         }

         if (bLoadFromFile)
         {
            if (!readMatrixFromFile(strFilename, mpMatrixValues, mNumBands, "Correlation Coefficient"))
            {
               return false; // error logged in mReadMatrixFromFile routine
            }
         }
         else
         {
            if (!isBatch())
            {
               StatisticsDlg sDlg("Correlation Coefficient", aoiList, mpDesktop->getMainWidget());
               if (sDlg.exec() == QDialog::Rejected)
               {
                  mpStep->finalize(Message::Abort);
                  return false;
               }

               QString strAoiName = sDlg.getAoiName();
               if (!strAoiName.isEmpty())
               {
                  success = computeCovarianceMatrix(strAoiName);
               }
               else // compute covariance over whole image using skip factors
               {
                  success = computeCovarianceMatrix(QString(), sDlg.getRowFactor(), sDlg.getColumnFactor());
               }
            }
            else // compute covariance over whole image using every pixel
            {
               success = computeCovarianceMatrix(QString(), 1, 1);
            }

            if (success)
            {
               // check if allocated
               vector<double> stdDev(mNumBands);
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress("Computing Correlation Coefficients from Covariances...", 0, NORMAL);
               }
               for (unsigned int band = 0; band < mNumBands; ++band)
               {
                  stdDev[band] = sqrt(mpMatrixValues[band][band]);
               }
               for (unsigned int band2 = 0; band2 < mNumBands; ++band2)
               {
                  for (unsigned int band1 = 0; band1 < mNumBands; ++band1)
                  {
                     if (band1 == band2)
                     {
                        mpMatrixValues[band2][band1] = 1.0;
                     }
                     else
                     {
                        mpMatrixValues[band2][band1] = mpMatrixValues[band2][band1] / (stdDev[band2] * stdDev[band1]);
                     }
                  }

                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress("Computing Correlation Coefficients from Covariances...",
                                                      100 * band2 / mNumBands, NORMAL);
                  }
               }

               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress("Finished computing Correlation Coefficients", 100, NORMAL);
               }

               if (!bLoadFromFile)
               {
                  writeMatrixToFile(strFilename, const_cast<const double**>(mpMatrixValues),
                     mNumBands, "Correlation Coefficient");
               }
            }
         }

         break;
      }

   default:
      break;
   }

   return success;
}

bool PCA::writeMatrixToFile(QString filename, const double **pData, int numBands, const string &caption)
{
   FileResource pFile(filename.toStdString().c_str(), "wt");
   if (pFile.get() == NULL)
   {
      mMessage = "Unable to save " + caption + " matrix to disk as " + filename.toStdString();
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 100, ERRORS);
      }

      mpStep->addMessage(mMessage, "app", "A0478959-21AF-4e64-B9DA-C17D7363F1BB", true);
   }
   else
   {
      fprintf(pFile, "%d\n", numBands);
      for (int row = 0; row < numBands; ++row)
      {
         for (int col = 0; col < numBands; ++col)
         {
            fprintf(pFile, "%.15e ", pData[row][col]);
         }
         fprintf(pFile, "\n");
      }

      mMessage = caption + " matrix saved to disk as " + filename.toStdString();
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 100, NORMAL);
      }
   }

   return true;
}

bool PCA::readMatrixFromFile(QString filename, double **pData, int numBands, const string &caption)
{
   FileResource pFile(filename.toStdString().c_str(), "rt");
   if (pFile.get() == NULL)
   {
      mMessage = "Unable to read " + caption + " matrix from file " + filename.toStdString();
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }
   mMessage = "Reading "  + caption + " matrix from file " + filename.toStdString();
   mpProgress->updateProgress(mMessage, 0, NORMAL);

   int lnumBands = 0;
   int numFieldsRead = fscanf(pFile, "%d\n", &lnumBands);
   if (numFieldsRead != 1)
   {
      mMessage = "Unable to read matrix file\n" + filename.toStdString();
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }
   if (lnumBands != numBands)
   {
      mMessage = "Mismatch between number of bands in cube and in matrix file.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }
   for (int row = 0; row < numBands; ++row)
   {
      for (int col = 0; col < numBands; ++col)
      {
         numFieldsRead = fscanf(pFile, "%lg ", &(pData[row][col]));
         if (numFieldsRead != 1)
         {
            mMessage = "Error reading " + caption + " matrix from disk.";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(mMessage, 0, ERRORS);
            }

            mpStep->finalize(Message::Failure, mMessage);
            return false;
         }
      }
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 100 * row / numBands, NORMAL);
      }
   }
   mMessage = caption + " matrix successfully read from disk";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(mMessage, 100, NORMAL);
   }

   return true;
}

bool PCA::computeCovarianceMatrix(QString aoiName, int rowSkip, int colSkip)
{
   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return false;
   }

   void* pData = NULL;

   if (aoiName.isEmpty())
   {
      if ((rowSkip < 1) || (colSkip < 1))
      {
         return false;
      }

      switchOnEncoding(pDescriptor->getDataType(), ComputeFactoredCov,
                        pData, mpRaster, mpMatrixValues, mpProgress,
                        &mAborted, rowSkip, colSkip);
   }
   else  // compute over AOI
   {
      MaskInput input;
      input.mpMatrix = mpMatrixValues;
      input.mpRaster = mpRaster;
      input.mpProgress = mpProgress;
      input.mpAbortFlag = &mAborted;
      AoiElement* pAoi = getAoiElement(aoiName.toStdString());
      if (pAoi == NULL)
      {
         mMessage = "Invalid AOI specified";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 0, ERRORS);
         }

         mpStep->finalize(Message::Failure, mMessage);
         return false;
      }
      input.mpMask = pAoi->getSelectedPoints();
      BitMaskIterator it(input.mpMask, mpRaster);

      // check if AOI has any points selected
      if (it.getCount() < 2)
      {
         mMessage = "Can't compute Covariance - not enough pixels were selected in " + aoiName.toStdString();
         if (mpProgress != NULL && it.getCount() != 0)
         {
            mpProgress->updateProgress(mMessage, 0, ERRORS);
         }

         mpStep->finalize(Message::Failure, mMessage);
         if (!isBatch())
         {
            QString message = "No pixels are currently selected in "+ aoiName + ".\nPCA is aborting.";
            QMessageBox::critical(NULL, "PCA", message);
         }
         return false;
      }
      switchOnEncoding(pDescriptor->getDataType(), ComputeMaskedCov, pData, &input);
   }

   if (isAborted())
   {
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Processing Covariance matrix aborted", 100, NORMAL);
      }

      mpStep->finalize(Message::Abort);
      return false;
   }

   return true;
}

bool PCA::readInPCAtransform(QString filename)
{
   FileResource pFile(filename.toStdString().c_str(), "rt");
   if (pFile.get() == NULL)
   {
      mMessage = "Unable to read PCA transform from file " + filename.toStdString();
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   mMessage = "Reading PCA transform from file " + filename.toStdString();
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(mMessage, 0, NORMAL);
   }

   unsigned int lnumBands = 0;
   unsigned int lnumComponents = 0;
   int numFieldsRead = fscanf(pFile, "%d\n", &lnumBands);
   if (numFieldsRead != 1)
   {
      mMessage = "Error reading number of bands from PCA transform file:\n" + filename.toStdString();
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   numFieldsRead = fscanf(pFile, "%d\n", &lnumComponents);
   if (numFieldsRead != 1)
   {
      mMessage = "Error reading number of components from PCA transform file:\n" + filename.toStdString();
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   if (lnumBands != mNumBands)
   {
      mMessage = "Mismatch between number of bands in cube and in PCA transform file.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }
   bool success = true;
   if (lnumComponents < mNumComponentsToUse)
   {
      if (!isBatch())
      {
         QString message;
         message.sprintf("This file only contains definitions for %d components, not %d",
                                    lnumComponents, mNumComponentsToUse);
         success = !QMessageBox::warning(NULL, "PCA", message, "Continue", "Cancel");
      }
   }

   if (success)
   {
      double junk = 0.0;
      for (unsigned int row = 0; row < mNumBands; ++row)
      {
         for (unsigned int col = 0; col < lnumComponents; ++col)
         {
            if (col < mNumComponentsToUse)
            {
               numFieldsRead = fscanf(pFile, "%lg ", &(mpMatrixValues[row][col]));
            }
            else
            {
               numFieldsRead = fscanf(pFile, "%lg ", &(junk));
            }

            if (numFieldsRead != 1)
            {
               success = false;
               break;
            }
         }
         if (!success)
         {
            break;
         }
         mpProgress->updateProgress(mMessage, 100 * row / mNumBands, NORMAL);
      }
      if (success)
      {
         mMessage = "PCA transform successfully read from disk";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 100, NORMAL);
         }
      }
      else
      {
         mMessage = "Error reading PCA transform from disk.";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 0, ERRORS);
         }

         mpStep->finalize(Message::Failure, mMessage);
      }
   }

   if (filename.contains("pcasmm"))
   {
      mCalcMethod = SECONDMOMENT;
   }
   else if (filename.contains("pcacvm"))
   {
      mCalcMethod = COVARIANCE;
   }
   else if (filename.contains("pcaccm"))
   {
      mCalcMethod = CORRCOEF;
   }

   return success;
}

bool PCA::writeOutPCAtransform(QString filename)
{
   FileResource pFile(filename.toStdString().c_str(), "wt");
   if (pFile.get() == NULL)
   {
      mMessage = "Unable to save PCA transform to disk as " + filename.toStdString();
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 100, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   fprintf(pFile, "%d\n", mNumBands);
   fprintf(pFile, "%d\n", mNumComponentsToUse);
   for (unsigned int row = 0; row < mNumBands; ++row)
   {
      for (unsigned int col = 0; col < mNumComponentsToUse; ++col)
      {
         fprintf(pFile, "%.15e ", mpMatrixValues[row][col]);
      }
      fprintf(pFile, "\n");
   }

   mMessage = "PCA transform saved to disk as " + filename.toStdString();
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(mMessage, 100, NORMAL);
   }

   return true;
}

AoiElement* PCA::getAoiElement(const std::string& aoiName)
{
   AoiElement* pAoi = dynamic_cast<AoiElement*>(mpModel->getElement(aoiName,
      TypeConverter::toString<AoiElement>(), mpRaster));
   if (pAoi == NULL)
   {
      pAoi = dynamic_cast<AoiElement*>(mpModel->getElement(aoiName,
         TypeConverter::toString<AoiElement>(), NULL));
   }

   return pAoi;
}
