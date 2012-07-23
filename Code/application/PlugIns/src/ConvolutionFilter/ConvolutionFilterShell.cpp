/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AoiLayer.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "BitMask.h"
#include "BitMaskIterator.h"
#include "ConfigurationSettings.h"
#include "ConvolutionFilterShell.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "LayerList.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "switchOnEncoding.h"
#include "Undo.h"

#include <QtCore/QStringList>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>

namespace
{
   template<typename T>
   void assignResult(T* pPixel, double value)
   {
      *pPixel = static_cast<T>(value);
   }
}

ConvolutionFilterShell::ConvolutionFilterShell() : mpAoi(NULL)
{
   setSubtype("Convolution Filter");
   setAbortSupported(true);
}

ConvolutionFilterShell::~ConvolutionFilterShell()
{
}

bool ConvolutionFilterShell::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   VERIFY(pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pInArgList->addArg<RasterElement>(Executable::DataElementArg(), NULL, 
      "The data element to be convolved.."));
   VERIFY(pInArgList->addArg<AoiElement>("AOI", NULL, "If not NULL, only the data in this AOI will be convolved and the "
      "new raster element will be the size of this AOI."));
   VERIFY(pInArgList->addArg<SpatialDataView>(Executable::ViewArg(), NULL, "The view to which the raster element to be convoluted is attached."));
   VERIFY(pInArgList->addArg<std::vector<unsigned int> >("Band Numbers", "The band numbers which will be convolved. Defaults to no bands. This argument takes precedence over the Band Number argument."));
   VERIFY(pInArgList->addArg<std::string>("Result Name", "The name of the new raster element. "
      "Defaults to the name of the input raster element with ' Convolved' appended."));
   VERIFY(pInArgList->addArg<double>("Offset", 0.0, "Optional offset value to add to each output pixel"));
   VERIFY(pInArgList->addArg<bool>("Force Float", false, "Optional flag to force output image to be 8-byte floating point."));
   return true;
}

bool ConvolutionFilterShell::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   VERIFY(pOutArgList->addArg<RasterElement>("Data Element", "The new raster element with the convolved data."));
   if (!Service<ApplicationServices>()->isBatch())
   {
      VERIFY(pOutArgList->addArg<SpatialDataView>("View", "The newly created view. "
                                            "This will be NULL if the application is non-interactive."));
   }
   return true;
}

bool ConvolutionFilterShell::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   if (!extractInputArgs(pInArgList))
   {
      return false;
   }
   if (!populateKernel() || mInput.mKernel.Nrows() % 2 == 0 || mInput.mKernel.Ncols() % 2 == 0)
   {
      mProgress.report("Invalid kernel.", 0, ERRORS, true);
      return false;
   }
   BitMaskIterator iterChecker((mpAoi == NULL) ? NULL : mpAoi->getSelectedPoints(), 0, 0,
      mInput.mpDescriptor->getColumnCount() - 1, mInput.mpDescriptor->getRowCount() - 1);
   EncodingType resultType = mInput.mForceFloat ? FLT8BYTES : mInput.mpDescriptor->getDataType();
   if (resultType == INT4SCOMPLEX)
   {
      resultType = INT4SBYTES;
   }
   else if (resultType == FLT8COMPLEX)
   {
      resultType = FLT8BYTES;
   }
   if (!isBatch())
   {
      RasterElement* pResult = static_cast<RasterElement*>(
         Service<ModelServices>()->getElement(mResultName, TypeConverter::toString<RasterElement>(), NULL));
      if (pResult != NULL)
      {
         if (QMessageBox::question(Service<DesktopServices>()->getMainWidget(), "Result data set exists",
            "The result data set already exists. Would you like to replace it?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No)
         {
            return false;
         }
         Service<ModelServices>()->destroyElement(pResult);
      }
   }
   mProgress.report("Begin convolution matrix execution.", 0, NORMAL);

   ModelResource<RasterElement> pResult(RasterUtilities::createRasterElement(
      mResultName, iterChecker.getNumSelectedRows(), iterChecker.getNumSelectedColumns(),
      mInput.mBands.size(), resultType, mInput.mpDescriptor->getInterleaveFormat(),
      mInput.mpDescriptor->getProcessingLocation() == IN_MEMORY));
   pResult->copyClassification(mInput.mpRaster);
   pResult->getMetadata()->merge(mInput.mpDescriptor->getMetadata()); //copy original metadata
   //chip metadata by bands
   vector<DimensionDescriptor> orgBands = mInput.mpDescriptor->getBands();
   vector<DimensionDescriptor> newBands;
   newBands.reserve(mInput.mBands.size());
   for (unsigned int index = 0; index < mInput.mBands.size(); ++index)
   {
      unsigned int selectedBand = mInput.mBands[index];
      if (selectedBand < orgBands.size())
      {
         newBands.push_back(orgBands[selectedBand]);
      }
   }
   RasterUtilities::chipMetadata(pResult->getMetadata(), mInput.mpDescriptor->getRows(),
      mInput.mpDescriptor->getColumns(), newBands);
   mInput.mpResult = pResult.get();
   if (mInput.mpResult == NULL)
   {
      mProgress.report("Unable to create result data set.", 0, ERRORS, true);
      return false;
   }
   mInput.mpAbortFlag = &mAborted;
   mInput.mpIterCheck = &iterChecker;
   ConvolutionFilterThreadOutput outputData;
   mta::ProgressObjectReporter reporter("Convolving", mProgress.getCurrentProgress());
   mta::MultiThreadedAlgorithm<ConvolutionFilterThreadInput,
                               ConvolutionFilterThreadOutput,
                               ConvolutionFilterThread>
          alg(mta::getNumRequiredThreads(iterChecker.getNumSelectedRows()), mInput, outputData, &reporter);
   switch(alg.run())
   {
   case mta::SUCCESS:
      if (!isAborted())
      {
         mProgress.report("Convolution filter complete.", 100, NORMAL);
         SpatialDataView* pView = displayResult();
         if (Service<ApplicationServices>()->isInteractive() && pView == NULL)
         {
            return false;
         }
         pOutArgList->setPlugInArgValue("View", pView);

         pResult.release();
         mProgress.upALevel();
         return true;
      }
      // fall through
   case mta::ABORT:
      mProgress.report("Convolution filter aborted.", 0, ABORT, true);
      return false;
   case mta::FAILURE:
      mProgress.report("Convolution filter failed.", 0, ERRORS, true);
      return false;
   default:
      VERIFY(false); // can't happen
   }
}

bool ConvolutionFilterShell::extractInputArgs(PlugInArgList* pInArgList)
{
   VERIFY(pInArgList);
   mProgress = ProgressTracker(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Executing " + getName(), "app", "{64097F31-84D0-41bf-BBAF-F60DAF212836}");
   if ((mInput.mpRaster = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg())) == NULL)
   {
      mProgress.report("No raster element.", 0, ERRORS, true);
      return false;
   }
   mInput.mpDescriptor = static_cast<const RasterDataDescriptor*>(mInput.mpRaster->getDataDescriptor());
   mpAoi = pInArgList->getPlugInArgValue<AoiElement>("AOI");
   if (mpAoi == NULL && !isBatch())
   {
      SpatialDataView* pView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
      if (pView != NULL)
      {
         std::vector<Layer*> layers;
         pView->getLayerList()->getLayers(AOI_LAYER, layers);
         if (!layers.empty())
         {
            QStringList layerNames;
            layerNames << "<none>";
            for (std::vector<Layer*>::const_iterator layer = layers.begin(); layer != layers.end(); ++layer)
            {
               layerNames << QString::fromStdString((*layer)->getName());
            }
            bool ok = true;
            std::string selectedLayer = QInputDialog::getItem(
               Service<DesktopServices>()->getMainWidget(), "Choose an AOI",
               "Select an AOI or <none> to process the entire image", layerNames, 0, false, &ok).toStdString();
            if (!ok)
            {
               mProgress.report("User cancelled " + getName(), 0, ABORT, true);
               return false;
            }
            for (std::vector<Layer*>::const_iterator layer = layers.begin(); layer != layers.end(); ++layer)
            {
               if ((*layer)->getName() == selectedLayer)
               {
                  mpAoi = static_cast<AoiElement*>((*layer)->getDataElement());
                  break;
               }
            }
         }
      }
   }
   std::vector<unsigned int> bandNumbers;
   if (!pInArgList->getPlugInArgValue("Band Numbers", bandNumbers))
   {
      mProgress.report("Error getting band numbers.", 0, ERRORS, true);
      return false;
   }
   if (!bandNumbers.empty())
   {
      mInput.mBands = bandNumbers;
   }
   pInArgList->getPlugInArgValue("Result Name", mResultName);
   if (mResultName.empty())
   {
      mResultName = mInput.mpRaster->getName() + " Convolved";
      if (!isBatch())
      {
         bool ok = true;
         mResultName = QInputDialog::getText(Service<DesktopServices>()->getMainWidget(), "Output data name",
            "Choose a name for the output data cube", QLineEdit::Normal,
            QString::fromStdString(mResultName), &ok).toStdString();
         if (!ok)
         {
            mProgress.report("User cancelled " + getName(), 0, ABORT, true);
         }
      }
   }

   if (!pInArgList->getPlugInArgValue("Offset", mInput.mOffset))
   {
      mProgress.report("Error getting offset.", 0, ERRORS, true);
      return false;
   }

   if (!pInArgList->getPlugInArgValue("Force Float", mInput.mForceFloat))
   {
      mProgress.report("Error getting float output.", 0, ERRORS, true);
      return false;
   }
   return true;
}

SpatialDataView* ConvolutionFilterShell::displayResult()
{
   VERIFY(mInput.mpResult != NULL);
   if (Service<ApplicationServices>()->isBatch())
   {
      return NULL;
   }
   SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(
      Service<DesktopServices>()->createWindow(mInput.mpResult->getName(), SPATIAL_DATA_WINDOW));
   SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();
   if (pView == NULL)
   {
      Service<DesktopServices>()->deleteWindow(pWindow);
      mProgress.report("Unable to create view.", 0, ERRORS, true);
      return NULL;
   }
   pView->setPrimaryRasterElement(mInput.mpResult);

   RasterLayer* pLayer = NULL;
   { // scope
      UndoLock lock(pView);
      pLayer = static_cast<RasterLayer*>(pView->createLayer(RASTER, mInput.mpResult));
   }
   if (pLayer == NULL)
   {
//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This would be cleaner with a WindowResource. If one " \
//                                              "becomes available, use it instead. (tclarke)")
      Service<DesktopServices>()->deleteWindow(pWindow);
      mProgress.report("Unable to create layer.", 0, ERRORS, true);
      return NULL;
   }
   return pView;
}

ConvolutionFilterShell::ConvolutionFilterThread::ConvolutionFilterThread(const ConvolutionFilterThreadInput &input,
                                                                         int threadCount,
                                                                         int threadIndex,
                                                                         mta::ThreadReporter &reporter) :
               mta::AlgorithmThread(threadIndex, reporter),
               mInput(input),
               mRowRange(getThreadRange(threadCount, input.mpIterCheck->getNumSelectedRows()))
{
   if (input.mpIterCheck->useAllPixels())
   {
      mRowRange = getThreadRange(threadCount, input.mpDescriptor->getRowCount());
   }
}

void ConvolutionFilterShell::ConvolutionFilterThread::run()
{
   EncodingType encoding = static_cast<const RasterDataDescriptor*>(
         mInput.mpRaster->getDataDescriptor())->getDataType();
   switchOnComplexEncoding(encoding, convolve, NULL);
}

template<class T>
void ConvolutionFilterShell::ConvolutionFilterThread::convolve(const T*)
{
   int numResultsCols = mInput.mpIterCheck->getNumSelectedColumns();
   if (mInput.mpResult == NULL)
   {
      return;
   }

   const RasterDataDescriptor* pResultDescriptor = static_cast<const RasterDataDescriptor*>(
      mInput.mpResult->getDataDescriptor());

   // account for AOIs which extend outside the dataset
   int maxRowNum = static_cast<int>(mInput.mpDescriptor->getRowCount()) - 1;
   mRowRange.mFirst = std::max(0, mRowRange.mFirst);
   mRowRange.mLast = std::min(mRowRange.mLast, maxRowNum);

   unsigned int bandCount = mInput.mBands.size();
   for (unsigned int bandNum = 0; bandNum < bandCount; ++bandNum)
   {
      FactoryResource<DataRequest> pResultRequest;
      pResultRequest->setRows(pResultDescriptor->getActiveRow(mRowRange.mFirst),
         pResultDescriptor->getActiveRow(mRowRange.mLast));
      pResultRequest->setColumns(pResultDescriptor->getActiveColumn(0),
         pResultDescriptor->getActiveColumn(numResultsCols - 1));
      pResultRequest->setBands(pResultDescriptor->getActiveBand(bandNum),
         pResultDescriptor->getActiveBand(bandNum));
      pResultRequest->setWritable(true);
      DataAccessor resultAccessor = mInput.mpResult->getDataAccessor(pResultRequest.release());
      if (!resultAccessor.isValid())
      {
         return;
      }

      int oldPercentDone = -1;
      int rowOffset = static_cast<int>(mInput.mpIterCheck->getOffset().mY);
      int startRow = mRowRange.mFirst + rowOffset;
      int stopRow = mRowRange.mLast + rowOffset;

      int columnOffset = static_cast<int>(mInput.mpIterCheck->getOffset().mX);
      int startColumn = columnOffset;
      int stopColumn = numResultsCols + columnOffset - 1;

      int yshift = (mInput.mKernel.Nrows() - 1) / 2;
      int xshift = (mInput.mKernel.Ncols() - 1) / 2;

      FactoryResource<DataRequest> pRequest;
      pRequest->setRows(mInput.mpDescriptor->getActiveRow(std::max(0, startRow - yshift)),
         mInput.mpDescriptor->getActiveRow(std::min(maxRowNum, stopRow + mInput.mKernel.Nrows() - yshift)));
      pRequest->setColumns(mInput.mpDescriptor->getActiveColumn(startColumn),
         mInput.mpDescriptor->getActiveColumn(stopColumn));
      pRequest->setBands(mInput.mpDescriptor->getActiveBand(mInput.mBands[bandNum]),
         mInput.mpDescriptor->getActiveBand(mInput.mBands[bandNum]));
      DataAccessor accessor = mInput.mpRaster->getDataAccessor(pRequest.release());
      if (!accessor.isValid())
      {
         return;
      }

      Service<ModelServices> model;
      ModelServices* pModel = model.get();
      int numRows = stopRow - startRow + 1;
      for (int row_index = startRow; row_index <= stopRow; ++row_index)
      {
         int percentDone = 100 * ((bandNum * numRows) + (row_index - startRow)) / (numRows * bandCount); 
         if (percentDone > oldPercentDone)
         {
            oldPercentDone = percentDone;
            getReporter().reportProgress(getThreadIndex(), percentDone);
         }
         if (mInput.mpAbortFlag != NULL && *mInput.mpAbortFlag)
         {
            break;
         }

         for (int col_index = startColumn; col_index <= stopColumn; ++col_index)
         {
            double accum = 0.0;
            if (mInput.mpIterCheck->getPixel(col_index, row_index))
            {
               for (int kernelrow = 0; kernelrow < mInput.mKernel.Nrows(); kernelrow++)
               {
                  int neighbor_row = row_index - yshift + kernelrow;
                  int real_row = std::min(std::max(0, neighbor_row),
                     static_cast<int>(mInput.mpDescriptor->getRowCount()) - 1);
                  for (int kernelcol = 0; kernelcol < mInput.mKernel.Ncols(); kernelcol++)
                  {
                     int neighbor_col = col_index - xshift + kernelcol;
                     int real_col = std::min(std::max(0, neighbor_col),
                        static_cast<int>(mInput.mpDescriptor->getColumnCount()) - 1);
                     accessor->toPixel(real_row, real_col);
                     if (accessor.isValid() == false)
                     {
                        return;
                     }

                     double val = 0.0;
                     pModel->getDataValue<T>(reinterpret_cast<T*>(accessor->getColumn()), COMPLEX_MAGNITUDE, 0, val);
                     accum += mInput.mKernel(kernelrow+1, kernelcol+1) * val / mInput.mKernel.Storage();
                  }
               }
            }
            if (resultAccessor.isValid() == false)
            {
               return;
            }

            switchOnEncoding(pResultDescriptor->getDataType(), assignResult,
                             resultAccessor->getColumn(), accum + mInput.mOffset);
            resultAccessor->nextColumn();
         }
         resultAccessor->nextRow();
      }
   }
}

bool ConvolutionFilterShell::ConvolutionFilterThreadOutput::compileOverallResults(
   const std::vector<ConvolutionFilterThread*>& threads)
{
   return true;
}

REGISTER_PLUGIN_BASIC(OpticksConvolutionFilter, GenericConvolution);

GenericConvolution::GenericConvolution()
{
   setName("Generic Convolution");
   setDescriptorId("{7d4e2289-053a-48e3-a10f-8176706044fe}");
   setDescription("Convolve a data set with a specified kernel.");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setWizardSupported(false);
}

GenericConvolution::~GenericConvolution()
{
}

bool GenericConvolution::getInputSpecification(PlugInArgList*& pInArgList)
{
   if (!ConvolutionFilterShell::getInputSpecification(pInArgList))
   {
      return false;
   }
   PlugInArg* pArg = Service<PlugInManagerServices>()->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Kernel");
   pArg->setDescription("The convolution kernel as an ossim NEWMAT::Matrix.");
   pArg->setType("NEWMAT::Matrix");
   pInArgList->addArg(*pArg);
   return true;
}

bool GenericConvolution::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!ConvolutionFilterShell::extractInputArgs(pInArgList))
   {
      return false;
   }
   NEWMAT::Matrix* pMatrix = pInArgList->getPlugInArgValueUnsafe<NEWMAT::Matrix>("Kernel");
   if (pMatrix == NULL)
   {
      mProgress.report("No kernel specified.", 0, ERRORS, true);
      return false;
   }
   mInput.mKernel = *pMatrix;
   return mInput.mKernel.Storage() > 0;
}

bool GenericConvolution::populateKernel()
{
   // already did this in extractInputArgs()
   return true;
}
