/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */


#include "AppConfig.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SampleGeoref.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SpatialDataView.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <boost/any.hpp>

XERCES_CPP_NAMESPACE_USE

SampleGeoref::SampleGeoref() :
   mXSize(10),
   mYSize(5),
   mXScale(1.0),
   mYScale(1.0),
   mExtrapolate(false),
   mFrames(1.0),
   mCurrentFrame(1),
   mRotate(false),
   mpAnimation(SIGNAL_NAME(Animation, FrameChanged), Slot(this, &SampleGeoref::animationFrameChanged)),
   mpRaster(NULL),
   mpGui(NULL)
{
   setName("SampleGeoref");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Simple Georeference plugin");
   setProductionStatus(false);
   setDescriptorId("{57B86486-3B3B-465a-B74F-53A17290D982}");
   allowMultipleInstances(true);
   executeOnStartup(false);
   destroyAfterExecute(false);
}

SampleGeoref::~SampleGeoref()
{
}

bool SampleGeoref::setInteractive()
{
   ExecutableShell::setInteractive();
   return false; // Georeference plugins are always called as batch
}

bool SampleGeoref::getInputSpecification(PlugInArgList*& pArgList)
{
   bool success = GeoreferenceShell::getInputSpecification(pArgList);
   success = success && pArgList->addArg<int>("XSize", 10);
   success = success && pArgList->addArg<int>("YSize", 5);
   success = success && pArgList->addArg<bool>("Extrapolate", false);
   success = success && pArgList->addArg<bool>("Animated", false);
   success = success && pArgList->addArg<bool>("Rotate", false);
   success = success && pArgList->addArg<View>(ViewArg());
   return success;
}

bool SampleGeoref::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   // Do any kind of setup we need before converting coordinates.
   // In this case, get our X and Y factors.

   StepResource pStep("Run Sample Georef", "app", "CFCB8AA9-D504-42e9-86F0-547DF9B4798A");
   Progress *pProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());

   FAIL_IF(!isBatch(), "Interactive mode is not supported.", return false);

   // Default values
   bool animated = false;

   // get factors from pInArgList
   pInArgList->getPlugInArgValue("XSize", mXSize);
   pInArgList->getPlugInArgValue("YSize", mYSize);
   pInArgList->getPlugInArgValue("Extrapolate", mExtrapolate);
   pInArgList->getPlugInArgValue("Animated", animated);
   pInArgList->getPlugInArgValue("Rotate", mRotate);

   View *pView = pInArgList->getPlugInArgValue<View>(ViewArg());
   mpRaster = pInArgList->getPlugInArgValue<RasterElement>(DataElementArg());
   FAIL_IF(mpRaster == NULL, "Could not find raster element", return false);

   if (mpGui != NULL)
   {
      mXSize = mpGui->getXSize();
      mYSize = mpGui->getYSize();
      animated = mpGui->getAnimated();
      mRotate = mpGui->getRotate();
      mExtrapolate = mpGui->getExtrapolate();
   }

   if (animated)
   {
      SpatialDataView *pSpatialView = dynamic_cast<SpatialDataView*>(pView);
      FAIL_IF(pSpatialView == NULL, "Could not find spatial data view.", return false);

      LayerList *pLayerList = pSpatialView->getLayerList();
      FAIL_IF(pLayerList == NULL, "Could not find layer list.", return false);
      
      RasterLayer *pLayer = dynamic_cast<RasterLayer*>(pLayerList->getLayer(RASTER, mpRaster));
      FAIL_IF(pLayer == NULL, "Could not find raster layer", return false);

      Animation *pAnim = pLayer->getAnimation();
      FAIL_IF(pAnim == NULL, "Could not find animation", return false);

      const std::vector<AnimationFrame> &frames = pAnim->getFrames();
      FAIL_IF(frames.empty(), "No frames in animation.", return false);

      mpAnimation.reset(pAnim);
      mFrames = frames.size();
      mCurrentFrame = 0;
   }

   RasterDataDescriptor *pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   FAIL_IF(pDescriptor == NULL, "Could not get data descriptor.", return false);

   unsigned int rows = pDescriptor->getRowCount();
   unsigned int cols = pDescriptor->getColumnCount();
   unsigned int bands = pDescriptor->getBandCount();

   mXScale = static_cast<double>(mXSize) / rows;
   mYScale = static_cast<double>(mYSize) / cols;
   
   mpRaster->setGeoreferencePlugin(this);

   mpGui = NULL; // Pointer not guaranteed to be valid after execute() is called
   return true;
}

LocationType SampleGeoref::geoToPixel(LocationType geo) const
{
   LocationType pixel;
   if (mRotate)
   {
      pixel.mX = geo.mY / mYScale;
      pixel.mY = geo.mX / mXScale;
      pixel = rotate(pixel, -2 * PI * mCurrentFrame/mFrames);
   }
   else
   {
      pixel.mX = (geo.mY - (mYSize * mCurrentFrame/mFrames)) / mYScale;
      pixel.mY = geo.mX / mXScale;
   }
   return pixel;
}

LocationType SampleGeoref::pixelToGeo(LocationType pixel) const
{
   LocationType geo;
   if (mRotate)
   {
      pixel = rotate(pixel, 2 * PI * mCurrentFrame/mFrames);
      geo.mX = pixel.mY * mXScale;
      geo.mY = pixel.mX * mYScale;
   }
   else
   {
      geo.mX = pixel.mY * mXScale;
      geo.mY = pixel.mX * mYScale + (mYSize * mCurrentFrame/mFrames);
   }
   return geo;
}

QWidget *SampleGeoref::getGui(RasterElement *pRaster)
{
   // we do not "own" any previous QWidgets generated, so we can safely 
   // overwrite the pointer, even if non-NULL
   mpGui = new SampleGeorefGui();
   return mpGui;
}

bool SampleGeoref::canHandleRasterElement(RasterElement *pRaster) const
{
   // This simple plugin can handle any RasterElement thrown at it.
   // A real plugin would probably need to check some parameter of the cube here.
   return true;
}

bool SampleGeoref::validateGuiInput() const
{
   // Our GUI input cannot be invalid.  Some validity check should go here.
   return true;
}

void SampleGeoref::animationFrameChanged(Subject &subject, const std::string &signal, const boost::any &data)
{
   const AnimationFrame *pFrame = boost::any_cast<AnimationFrame*>(data);
   if (pFrame != NULL)
   {
      mCurrentFrame = pFrame->mFrameNumber;

      mpRaster->updateGeoreferenceData();
   }
}

LocationType SampleGeoref::rotate(LocationType loc, double rad)
{
   LocationType ret;

   ret.mX = loc.mX * cos(rad) - loc.mY * sin(rad);
   ret.mY = loc.mX * sin(rad) + loc.mY * cos(rad); 

   return ret;
}

bool SampleGeoref::serialize(SessionItemSerializer &serializer) const
{
   XMLWriter writer("SampleGeoref");
   if (mpRaster != NULL)
   {
      writer.addAttr("rasterId", mpRaster->getId());
   }
   else
   {
      return false;
   }
   writer.addAttr("xSize", mXSize);
   writer.addAttr("ySize", mYSize);
   writer.addAttr("xScale", mXScale);
   writer.addAttr("yScale", mYScale);
   writer.addAttr("frames", mFrames);
   writer.addAttr("extrapolate", mExtrapolate);
   writer.addAttr("currentFrame", mCurrentFrame);
   writer.addAttr("rotate", mRotate);
   const Animation *pAnim = mpAnimation.get();
   if (pAnim)
   {
      writer.addAttr("animationId", pAnim->getId());
   }
   return serializer.serialize(writer);
}

bool SampleGeoref::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader reader(NULL, false);
   DOMElement *pRootElement = deserializer.deserialize(reader, "SampleGeoref");
   if (pRootElement)
   {
      std::string windowId = A(pRootElement->getAttribute(X("rasterId")));
      mpRaster = dynamic_cast<RasterElement*>(Service<SessionManager>()->getSessionItem(windowId));
      if (mpRaster != NULL)
      {
         mXSize = atoi(A(pRootElement->getAttribute(X("xSize"))));
         mYSize = atoi(A(pRootElement->getAttribute(X("ySize"))));
         mXScale = atof(A(pRootElement->getAttribute(X("xScale"))));
         mYScale = atof(A(pRootElement->getAttribute(X("yScale"))));
         mFrames = atof(A(pRootElement->getAttribute(X("frames"))));
         mExtrapolate = StringUtilities::fromXmlString<bool>(A(pRootElement->getAttribute(X("extrapolate"))));
         mCurrentFrame = atoi(A(pRootElement->getAttribute(X("currentFrame"))));
         mRotate = atoi(A(pRootElement->getAttribute(X("rotate"))));
         std::string animId = A(pRootElement->getAttribute(X("animationId")));
         if (animId.empty() == false)
         {
            Animation *pAnim = dynamic_cast<Animation*>(Service<SessionManager>()->getSessionItem(animId));
            if (pAnim)
            {
               mpAnimation.reset(pAnim);
            }
         }
         return true;
      }
   }
   return false;
}

bool SampleGeoref::canExtrapolate() const
{
   return mExtrapolate;
}
