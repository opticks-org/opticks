/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "ProgressTracker.h"
#include "RasterDataDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialResampler.h"
#include "SpatialResamplerOptions.h"

#include <string>

#include <opencv2/imgproc/imgproc.hpp>

REGISTER_PLUGIN_BASIC(OpticksSpatialResampler, SpatialResampler);

namespace
{
   // Ensures output file can be created by removing an existing
   // one if necessary.
   void ensureOutput(const std::string& name)
   {
      DataElement* pExistingOutput =
         Service<ModelServices>()->getElement(name, TypeConverter::toString<RasterElement>(), NULL);
      if (pExistingOutput != NULL)
      {
         Service<ModelServices>()->destroyElement(pExistingOutput);
      }
   }

   template<typename T>
   void getOriginalAsType(T *pSource, cv::Mat sData, int row, int col)
   {
      sData.at<T>(row , col) = *pSource;
   }

   template<typename T>
   void putOriginalAsType(T *pSource, cv::Mat sData, int row, int col)
   {
      *pSource = sData.at<T>(row , col);
   }

   int convertEncodingTypeOpenCV(EncodingType encodingType)
   {
      switch (encodingType)
      {
      case INT1SBYTE:
         return CV_8S;
      case INT1UBYTE:
         return CV_8U;
      case INT2SBYTES:
         return CV_16S;
      case INT2UBYTES:
         return CV_16U;
      case INT4SBYTES:
         return CV_32S;
      case INT4UBYTES:         // OpenCV doesn't have unsigned int as a type it can work with.
         return CV_USRTYPE1;   // So substitute USERTYPE1.
      case FLT4BYTES:
         return CV_32F;
      case FLT8BYTES:
         return CV_64F;
      case INT4SCOMPLEX:
      case FLT8COMPLEX:        // OpenCV doesn't support complex data.
      default:
         return CV_USRTYPE1;   // So, substitute USRTYPE1.
      }
   }

   int convertInterpolationMethodOpenCV(InterpolationType interpolationMethod)
   {
      int cvInterpMethod;
      switch (interpolationMethod)
      {
      case INTERP_NEAREST_NEIGHBOR:
         cvInterpMethod = cv::INTER_NEAREST;
         break;
      case INTERP_BILINEAR:
         cvInterpMethod = cv::INTER_LINEAR;
         break;
      case INTERP_AREA:
         cvInterpMethod = cv::INTER_AREA;
         break;
      case INTERP_BICUBIC:
         cvInterpMethod = cv::INTER_CUBIC;
         break;
      case INTERP_LANCZOS4:
         cvInterpMethod = cv::INTER_LANCZOS4;
         break;
      default:
         cvInterpMethod = cv::INTER_CUBIC;
      }
      return cvInterpMethod;
   }
}

SpatialResampler::SpatialResampler()
{
   setName("Spatial Resampler");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("Perform Spatial Resampling");
   setDescription("Perform Spatial Resampling");
   setDescriptorId("{21600E6B-DB9F-4EF9-BBB4-1C707245BF0F}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setAbortSupported(true);
}

SpatialResampler::~SpatialResampler()
{}

bool SpatialResampler::setInteractive()
{
   AlgorithmShell::setInteractive();
   return false;
}

bool SpatialResampler::getInputSpecification(PlugInArgList*& pArgList)
{
   if (isBatch())
   {
      pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
      VERIFY(pArgList != NULL);
      VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), Executable::ProgressArgDescription()));
      VERIFY(pArgList->addArg<RasterElement>(Executable::DataElementArg(),
         "Element to be resampled."));
      double xFactor = SpatialResamplerOptions::getSettingXScaleFactor();
      VERIFY(pArgList->addArg<double>("X Scale Factor", xFactor, "Amount to increase resolution in the X direction."));
      double yFactor = SpatialResamplerOptions::getSettingYScaleFactor();
      VERIFY(pArgList->addArg<double>("Y Scale Factor", yFactor, "Amount to increase resolution in the Y direction."));
      InterpolationType interpolationMethod = SpatialResamplerOptions::getSettingInterpolationMethod();
      VERIFY(pArgList->addArg<InterpolationType>("Interpolation Type", interpolationMethod,
         "Type of interpolation used to determine new pixel values"));
      return true;
   }
   else
   {
      pArgList = NULL;
      return false;
   }
}

bool SpatialResampler::getOutputSpecification(PlugInArgList*& pArgList)
{
   if (isBatch())
   {
      pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
      VERIFY(pArgList != NULL);
      VERIFY(pArgList->addArg<RasterElement>(Executable::DataElementArg(),
         "Output element for the result of resampling."));
      return true;
   }
   else
   {
      pArgList = NULL;
      return false;
   }
}

bool SpatialResampler::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   VERIFY(pInArgList != NULL);

   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Execute " + getName(), "app", "{462314FD-0808-48D5-8093-72DA50024D9A}");

   RasterElement* pRasterElement = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   if (pRasterElement == NULL)
   {
      progress.report("No raster element provided.", 0, ERRORS, true);
      return false;
   }

   double xFactor;
   if (pInArgList->getPlugInArgValue<double>("X Scale Factor", xFactor) == false)
   {
      progress.report("No X scale factor provided.", 0, ERRORS, true);
      return false;
   }

   double yFactor;
   if (pInArgList->getPlugInArgValue<double>("Y Scale Factor", yFactor) == false)
   {
      progress.report("No Y scale factor provided.", 0, ERRORS, true);
      return false;
   }

   InterpolationType interpolationMethod;
   if (pInArgList->getPlugInArgValue<InterpolationType>("Interpolation Type", interpolationMethod) == false)
   {
      progress.report("No interpolation type provided.", 0, ERRORS, true);
      return false;
   }

   // Check for previous results and other incurable error conditions before displaying the dialog.
   const std::string outputName = pRasterElement->getDisplayName(true) + "_Resampling_Result";
   ensureOutput(outputName);
   RasterDataDescriptor* pSrcDesc = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
   VERIFY(pSrcDesc != NULL);

   EncodingType srcType = pSrcDesc->getDataType();
   if (srcType == INT4SCOMPLEX || srcType == FLT8COMPLEX)
   {
      progress.report("Spatial resampling cannot be performed on complex data.", 0, ERRORS, true);
      return false;
   }
   else if (srcType == INT4SBYTES || srcType == INT4UBYTES)
   {
      progress.report("Spatial resampling cannot be performed on 4-byte integer data.", 0, ERRORS, true);
      return false;
   }

   FactoryResource<DataRequest> pRequest;
   DataAccessor pSrcAcc = pRasterElement->getDataAccessor(pRequest.release());

   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(
      outputName, yFactor * pSrcDesc->getRowCount(),
      xFactor * pSrcDesc->getColumnCount(), pSrcDesc->getDataType()));
   if (pResultCube.get() == NULL)
   {
      progress.report("Unable to create output raster element.", 0, ERRORS, true);
      return false;
   }
   FactoryResource<DataRequest> pResultRequest;
   pResultRequest->setWritable(true);
   RasterDataDescriptor* pDestDesc = dynamic_cast<RasterDataDescriptor*>(pResultCube->getDataDescriptor());
   DataAccessor pDestAcc = pResultCube->getDataAccessor(pResultRequest.release());

   int matType;
   if (srcType == INT1SBYTE)
   {
      // Since cv::resize() doesn't handle signed byte data,
      // upconvert to signed short for processing.
      matType = convertEncodingTypeOpenCV(INT2SBYTES);
   }
   else
   {
      matType = convertEncodingTypeOpenCV(srcType);
   }
   try
   {
      cv::Mat testMat(pSrcDesc->getRowCount(), pSrcDesc->getColumnCount(), matType);
      int progressPercent = 0;
      progress.report("Reading source data", progressPercent, NORMAL);
      float totalRows = pSrcDesc->getRowCount();

      for (unsigned int row = 0; row < pSrcDesc->getRowCount(); ++row)
      {
         for (unsigned int col = 0; col < pSrcDesc->getColumnCount(); ++col)
         {
            VERIFY(pSrcAcc.isValid());
            switchOnEncoding(pSrcDesc->getDataType(), getOriginalAsType, pSrcAcc->getColumn(), testMat, row, col);
            pSrcAcc->nextColumn();
         }
         pSrcAcc->nextRow();
         if (isAborted() == true)
         {
            progress.report("Cancelled", 0, ABORT, true);
            return false;
         }
         progressPercent = row / totalRows * 33;
         progress.report("Reading source data", progressPercent, NORMAL);
      }

      int cvInterpMethod = convertInterpolationMethodOpenCV(interpolationMethod);
      cv::Mat resultsMat(yFactor * pSrcDesc->getRowCount(), xFactor * pSrcDesc->getColumnCount(), matType);
      cv::resize(testMat, resultsMat, resultsMat.size(), 0, 0, cvInterpMethod);

      totalRows = resultsMat.rows;
      for (int row = 0; row < resultsMat.rows; ++row)
      {
         for (int col = 0; col < resultsMat.cols; ++col)
         {
            switchOnEncoding(pSrcDesc->getDataType(), putOriginalAsType, pDestAcc->getColumn(), resultsMat, row, col);
            pDestAcc->nextColumn();
         }
         pDestAcc->nextRow();
         if (isAborted())
         {
            progress.report("Cancelled", 0, ABORT, true);
            return false;
         }
         progressPercent = row / totalRows * 33 + 64;
         progress.report("Writing result data", progressPercent, NORMAL);
      }
   }
   catch (cv::Exception ev)
   {
      progress.report(ev.msg, 0, ERRORS, true);
      return false;
   }

   progress.report(getName() + " complete.", 100, NORMAL);
   progress.upALevel();
   pOutArgList->setPlugInArgValue(Executable::DataElementArg(), pResultCube.release());
   return true;
}