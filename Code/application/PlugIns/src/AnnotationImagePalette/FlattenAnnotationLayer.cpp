/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayer.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "FileImageObject.h"
#include "FlattenAnnotationLayer.h"
#include "GraphicElement.h"
#include "GraphicObject.h"
#include "LayerList.h"
#include "PlugInArgList.h"
#include "PlugInFactory.h"
#include "PlugInManagerServices.h"
#include "ProgressTracker.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SpatialDataView.h"
#include "Statistics.h"
#include "switchOnEncoding.h"
#include "Undo.h"

#include <QtGui/QImage>

PLUGINFACTORY(FlattenAnnotationLayer);

FlattenAnnotationLayer::FlattenAnnotationLayer()
{
   setName("Flatten Annotation Layer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Flatten an annotation layer into a raster layer.");
   setDescriptorId("{25ff6fc8-e49e-43cd-9a71-96afc13e3ad6}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setMenuLocation("[General Algorithms]/Flatten Annotation Layer");
}

FlattenAnnotationLayer::~FlattenAnnotationLayer()
{
}

bool FlattenAnnotationLayer::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   VERIFY(pInArgList->addArg<Progress>(ProgressArg(), NULL));
   VERIFY(pInArgList->addArg<AnnotationLayer>(LayerArg(), "The annotation layer containing the image objects to be flattened."));
   VERIFY(pInArgList->addArg<RasterLayer>("Raster Layer", NULL, "The raster layer which will contain the flattened image objects. "
                                                                "The default is the top-most layer containing the primary raster "
                                                                "element of the annotation layer's view."));
   return true;
}

bool FlattenAnnotationLayer::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool FlattenAnnotationLayer::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }
   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(ProgressArg()), "Flattening layer.",
      "app", "f2582dbc-0003-472b-80c3-acffd34334b3");
   AnnotationLayer* pAnnotationLayer = pInArgList->getPlugInArgValue<AnnotationLayer>(LayerArg());
   if (pAnnotationLayer == NULL)
   {
      progress.report("No annotation layer specified.", 0, ERRORS, true);
      return false;
   }
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pAnnotationLayer->getView());
   if (pView == NULL)
   {
      progress.report("The annotation layer must be in a spatial data view.", 0, ERRORS, true);
      return false;
   }
   RasterLayer* pRasterLayer = pInArgList->getPlugInArgValue<RasterLayer>("Raster Layer");
   if (pRasterLayer == NULL)
   {
      pRasterLayer = static_cast<RasterLayer*>(
         pView->getLayerList()->getLayer(RASTER, pView->getLayerList()->getPrimaryRasterElement()));
   }
   if (pRasterLayer == NULL)
   {
      progress.report("No raster layer specified.", 0, ERRORS, true);
      return false;
   }
   RasterElement* pRaster = static_cast<RasterElement*>(pRasterLayer->getDataElement());
   VERIFY(pRaster);

   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
   VERIFY(pDesc);
   std::vector<ChannelInfo> channels;
   if (pRasterLayer->getDisplayMode() == GRAYSCALE_MODE)
   {
      ChannelInfo info;
      pRasterLayer->getStretchValues(GRAY, info.dataMin, info.dataMax);
      RegionUnits regionUnits = pRasterLayer->getStretchUnits(GRAY);
      if (regionUnits != RAW_VALUE)
      {
         info.dataMin = pRasterLayer->convertStretchValue(GRAY, info.dataMin, RAW_VALUE);
         info.dataMax = pRasterLayer->convertStretchValue(GRAY, info.dataMax, RAW_VALUE);
      }
      info.bandShift = 0;
      info.channel = GRAY;
      info.band = pRasterLayer->getDisplayedBand(GRAY);
      FactoryResource<DataRequest> pRequest;
      VERIFY(pRequest.get());
      if (pDesc->getInterleaveFormat() == BIP)
      {
         info.bandShift = info.band.getActiveNumber();
      }
      else
      {
         pRequest->setBands(info.band, info.band, 1);
      }
      pRequest->setWritable(true);
      info.acc = pRaster->getDataAccessor(pRequest.release());
      if (!info.acc.isValid())
      {
         progress.report("Unable to access the data cube.", 0, ERRORS, true);
         return false;
      }
      info.numRows = pDesc->getRowCount();
      info.numCols = pDesc->getColumnCount();
      channels.push_back(info);
   }
   else
   {
      ChannelInfo redInfo, greenInfo, blueInfo;
      pRasterLayer->getStretchValues(RED, redInfo.dataMin, redInfo.dataMax);
      if (pRasterLayer->getStretchUnits(RED) != RAW_VALUE)
      {
         redInfo.dataMin = pRasterLayer->convertStretchValue(RED, redInfo.dataMin, RAW_VALUE);
         redInfo.dataMax = pRasterLayer->convertStretchValue(RED, redInfo.dataMax, RAW_VALUE);
      }
      pRasterLayer->getStretchValues(GREEN, greenInfo.dataMin, greenInfo.dataMax);
      if (pRasterLayer->getStretchUnits(GREEN) != RAW_VALUE)
      {
         greenInfo.dataMin = pRasterLayer->convertStretchValue(GREEN, greenInfo.dataMin, RAW_VALUE);
         greenInfo.dataMax = pRasterLayer->convertStretchValue(GREEN, greenInfo.dataMax, RAW_VALUE);
      }
      pRasterLayer->getStretchValues(BLUE, blueInfo.dataMin, blueInfo.dataMax);
      if (pRasterLayer->getStretchUnits(BLUE) != RAW_VALUE)
      {
         blueInfo.dataMin = pRasterLayer->convertStretchValue(BLUE, blueInfo.dataMin, RAW_VALUE);
         blueInfo.dataMax = pRasterLayer->convertStretchValue(BLUE, blueInfo.dataMax, RAW_VALUE);
      }
      redInfo.bandShift = 0;
      greenInfo.bandShift = 0;
      blueInfo.bandShift = 0;
      redInfo.channel = RED;
      greenInfo.channel = GREEN;
      blueInfo.channel = BLUE;
      redInfo.band = pRasterLayer->getDisplayedBand(RED);
      greenInfo.band = pRasterLayer->getDisplayedBand(GREEN);
      blueInfo.band = pRasterLayer->getDisplayedBand(BLUE);
      if (pDesc->getInterleaveFormat() == BIP)
      {
         FactoryResource<DataRequest> pRequest;
         VERIFY(pRequest.get());
         pRequest->setWritable(true);
         redInfo.bandShift = redInfo.band.getActiveNumber();
         greenInfo.bandShift = greenInfo.band.getActiveNumber();
         blueInfo.bandShift = blueInfo.band.getActiveNumber();
         redInfo.acc = pRaster->getDataAccessor(pRequest.release());
         greenInfo.acc = redInfo.acc;
         blueInfo.acc = redInfo.acc;
      }
      else
      {
         FactoryResource<DataRequest> pRedRequest, pGreenRequest, pBlueRequest;
         VERIFY(pRedRequest.get() && pGreenRequest.get() && pBlueRequest.get());
         pRedRequest->setWritable(true);
         pGreenRequest->setWritable(true);
         pBlueRequest->setWritable(true);
         pRedRequest->setBands(redInfo.band, redInfo.band, 1);
         pGreenRequest->setBands(greenInfo.band, greenInfo.band, 1);
         pBlueRequest->setBands(blueInfo.band, blueInfo.band, 1);
         redInfo.acc = pRaster->getDataAccessor(pRedRequest.release());
         greenInfo.acc = pRaster->getDataAccessor(pGreenRequest.release());
         blueInfo.acc = pRaster->getDataAccessor(pBlueRequest.release());
      }
      if (!redInfo.acc.isValid() || !greenInfo.acc.isValid() || !blueInfo.acc.isValid())
      {
         progress.report("Unable to access the data cube.", 0, ERRORS, true);
         return false;
      }
      redInfo.numRows = greenInfo.numRows = blueInfo.numRows = pDesc->getRowCount();
      redInfo.numCols = greenInfo.numCols = blueInfo.numCols = pDesc->getColumnCount();
      channels.push_back(redInfo);
      channels.push_back(greenInfo);
      channels.push_back(blueInfo);
   }

   UndoLock undo(pView);
   std::list<GraphicObject*> objects;
   pAnnotationLayer->getObjects(FILE_IMAGE_OBJECT, objects);
   bool isGeocentric = static_cast<GraphicElement*>(pAnnotationLayer->getDataElement())->getGeocentric();
   float curProgress = 0.0;
   float progressInc = 99.0  / (objects.empty() ? 1 : objects.size());
   for (std::list<GraphicObject*>::iterator object = objects.begin(); object != objects.end(); ++object)
   {
      progress.report("Flattening image objects.", static_cast<int>(curProgress), NORMAL);
      curProgress += progressInc;
      int width = 0, height = 0;
      ColorType transparent;
      const unsigned int* pImageData = (*object)->getObjectImage(width, height, transparent);
      /**
       * This data is actually ABGR32 but QImage does not support that format. We'll pretend it's ARGB32 and
       * swap the red and blue components later.
       */
      QImage imageData(reinterpret_cast<const uchar*>(pImageData), width, height, width * 4, QImage::Format_ARGB32);
      if (imageData.isNull())
      {
         progress.report("No image data for " + (*object)->getName(), 0, WARNING);
         continue;
      }
      LocationType llCorner = (*object)->getLlCorner();
      LocationType urCorner = (*object)->getUrCorner();
      // check the data origin
      bool horizontalFlip(false);
      bool verticalFlip(false);
      pRasterLayer->isFlipped(llCorner, urCorner, horizontalFlip, verticalFlip);
      if (isGeocentric)
      {
         std::vector<LocationType> geos, pixels;
         geos.push_back(llCorner);
         geos.push_back(urCorner);
         pRaster->convertGeocoordsToPixels(geos);
         llCorner = pixels[0];
         urCorner = pixels[1];
      }
      int startRow = static_cast<int>(std::min(llCorner.mY, urCorner.mY));
      int startCol = static_cast<int>(std::min(llCorner.mX, urCorner.mX));
      int endRow = static_cast<int>(std::max(llCorner.mY, urCorner.mY) + 0.5);
      int endCol = static_cast<int>(std::max(llCorner.mX, urCorner.mX) + 0.5);
      if (horizontalFlip || verticalFlip)
      {
         imageData = imageData.mirrored(horizontalFlip, verticalFlip);
      }
      QSize initialSize(endCol - startCol, endRow - startRow);
      imageData = imageData.scaled(initialSize);
      double rotation = (*object)->getRotation();
      QTransform trans;
      trans.rotate(rotation);
      imageData = imageData.transformed(trans);
      if (initialSize != imageData.size())
      {
         initialSize = (imageData.size() - initialSize) / 2.0;
         startCol -= static_cast<int>(initialSize.width());
         endCol += static_cast<int>(initialSize.width() + 0.5);
         startRow -= static_cast<int>(initialSize.height());
         endRow += static_cast<int>(initialSize.height() + 0.5);
      }
      for (int row = 0; row < imageData.height(); ++row)
      {
         for (int col = 0; col < imageData.width(); ++col)
         {
            QRgb pixVal = imageData.pixel(col, row);
            for (std::vector<ChannelInfo>::iterator channel = channels.begin(); channel != channels.end(); ++channel)
            {
               unsigned int newRow = row + startRow;
               unsigned int newCol = col + startCol;
               if (newRow < 0 || newCol < 0 || newRow >= channel->numRows || newCol >= channel->numCols)
               {
                  // the row/col is outside the cube data...continue on
                  continue;
               }
               channel->acc->toPixel(row + startRow, col + startCol);
               VERIFY(channel->acc.isValid());
               switchOnComplexEncoding(pDesc->getDataType(), flattenData, channel->acc->getColumn(), *channel, pixVal);
            }
         }
      }
      pAnnotationLayer->removeObject(*object, true);
   }
   if (pAnnotationLayer->getNumObjects() > 0)
   {
      progress.report("The annotation layer still contains objects which were not flattened. "
         "The Layer will not be removed.", 0, WARNING, true);
   }
   else
   {
      pView->deleteLayer(pAnnotationLayer);
   }
   pRaster->updateData();

   progress.report("Flatten complete.", 100, NORMAL);
   progress.upALevel();
   return true;
}

template<typename T>
void FlattenAnnotationLayer::flattenData(T* pRasterData,
                                         FlattenAnnotationLayer::ChannelInfo& channel,
                                         QRgb imageDatum)
{
   // scale the new data value to the data min/max
   double newScaled = 0.0;
   switch(channel.channel)
   {
   case GRAY:
      newScaled = qGray(imageDatum);
      break;
   /**
    * The data coming from the annotation image object is really in ABGR format
    * but QImage does not support this format. Since we are just copying components
    * we can do the switch from ARGB to ABGR here  by reversing the blue and red
    * components. If annoation image object is changed to use a format supported by
    * QImage, this can be changed.
    */
   case RED:
      newScaled = qBlue(imageDatum);
      break;
   case BLUE:
      newScaled = qRed(imageDatum);
      break;
   case GREEN:
      newScaled = qGreen(imageDatum);
      break;
   }
   newScaled = newScaled / 256.0 * (channel.dataMax - channel.dataMin) + channel.dataMin;

   // pre-multiply with the alpha channel
   double alpha = qAlpha(imageDatum) / 256.0;
   double curAlphaMult = pRasterData[channel.bandShift] * (1.0 - alpha);
   double newAlphaMult = newScaled * alpha;

   // set the new, alpha multiplied, scaled value
   pRasterData[channel.bandShift] = static_cast<T>(newAlphaMult + curAlphaMult);
}

template<>
void FlattenAnnotationLayer::flattenData<IntegerComplex>(IntegerComplex* pRasterData,
                                                         FlattenAnnotationLayer::ChannelInfo& channel,
                                                         QRgb imageDatum)
{
   // scale the new data value to the data min/max
   double newScaled = 0.0;
   switch(channel.channel)
   {
   case GRAY:
      newScaled = qGray(imageDatum);
      break;
   /**
    * The data coming from the annotation image object is really in ABGR format
    * but QImage does not support this format. Since we are just copying components
    * we can do the switch from ARGB to ABGR here  by reversing the blue and red
    * components. If annoation image object is changed to use a format supported by
    * QImage, this can be changed.
    */
   case RED:
      newScaled = qBlue(imageDatum);
      break;
   case GREEN:
      newScaled = qGreen(imageDatum);
      break;
   case BLUE:
      newScaled = qRed(imageDatum);
      break;
   }
   newScaled = newScaled / 256.0 * (channel.dataMax - channel.dataMin) + channel.dataMin;

   // pre-multiply with the alpha channel
   double alpha = qAlpha(imageDatum) / 256.0;
   double curAlphaMultReal = pRasterData[channel.bandShift].mReal * (1.0 - alpha);
   double curAlphaMultImag = pRasterData[channel.bandShift].mImaginary * (1.0 - alpha);
   double newAlphaMult = newScaled * alpha;

   // set the new, alpha multiplied, scaled value
   pRasterData[channel.bandShift].mReal = static_cast<short>(newAlphaMult + curAlphaMultReal);
   pRasterData[channel.bandShift].mImaginary = static_cast<short>(newAlphaMult + curAlphaMultImag);
}

template<>
void FlattenAnnotationLayer::flattenData<FloatComplex>(FloatComplex* pRasterData,
                                                       FlattenAnnotationLayer::ChannelInfo& channel,
                                                       QRgb imageDatum)
{
   // scale the new data value to the data min/max
   double newScaled = 0.0;
   switch(channel.channel)
   {
   case GRAY:
      newScaled = qGray(imageDatum);
      break;
   /**
    * The data coming from the annotation image object is really in ABGR format
    * but QImage does not support this format. Since we are just copying components
    * we can do the switch from ARGB to ABGR here  by reversing the blue and red
    * components. If annoation image object is changed to use a format supported by
    * QImage, this can be changed.
    */
   case RED:
      newScaled = qBlue(imageDatum);
      break;
   case GREEN:
      newScaled = qGreen(imageDatum);
      break;
   case BLUE:
      newScaled = qRed(imageDatum);
      break;
   }
   newScaled = newScaled / 256.0 * (channel.dataMax - channel.dataMin) + channel.dataMin;

   // pre-multiply with the alpha channel
   double alpha = qAlpha(imageDatum) / 256.0;
   double curAlphaMultReal = pRasterData[channel.bandShift].mReal * (1.0 - alpha);
   double curAlphaMultImag = pRasterData[channel.bandShift].mImaginary * (1.0 - alpha);
   double newAlphaMult = newScaled * alpha;

   // set the new, alpha multiplied, scaled value
   pRasterData[channel.bandShift].mReal = static_cast<float>(newAlphaMult + curAlphaMultReal);
   pRasterData[channel.bandShift].mImaginary = static_cast<float>(newAlphaMult + curAlphaMultImag);
}
