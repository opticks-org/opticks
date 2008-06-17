/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Animation.h"
#include "AnimationController.h"
#include "AnimationServices.h"
#include "AppVersion.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "MultiLayerMovie.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "Undo.h"

#include <vector>

using namespace std;

MultiLayerMovie::MultiLayerMovie() : 
   mpRaster1(NULL), mpRaster2(NULL), mpRaster3(NULL),
   mpWindow(NULL),
   mpLayer1(NULL), mpLayer2(NULL), mpLayer3(NULL)
{
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright("Copyright (c) 2007 BATC");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(false);
   setMenuLocation("[Demo]\\Multi-Layer Movie");
   setName("Multi-Layer Movie");
   setDescription("Creates 3 data cubes in a single window");
   setShortDescription( "Creates 3 data cubes in a single window" );
   setDescriptorId("{56DEB209-768F-4d34-BC97-5186AC1F580C}");
}

bool MultiLayerMovie::getInputSpecification( PlugInArgList * &pArgList )
{
   pArgList = NULL;
   return true;
}

bool MultiLayerMovie::getOutputSpecification( PlugInArgList * &pArgList )
{
   pArgList = NULL;
   return true;
}

bool MultiLayerMovie::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{
   if (createRasterElements() == true)
   {
      ModelResource<RasterElement> pRes1(mpRaster1);
      ModelResource<RasterElement> pRes2(mpRaster2);
      ModelResource<RasterElement> pRes3(mpRaster3);

      bool bSuccess = populateRasterElements() && createWindow() && setupAnimations();
      if (bSuccess == true)
      {
         pRes1.release();
         pRes2.release();
         pRes3.release();
      }
      else if (mpWindow != NULL)
      {
         Service<DesktopServices>()->deleteWindow(mpWindow);
      }

      return bSuccess;
   }

   return false;
}

bool MultiLayerMovie::createRasterElements()
{
   mpRaster1 = RasterUtilities::createRasterElement("MultiLayerMovieCube1", mNumRows,
      mNumCols, mNumBands, INT2UBYTES, BIP, true, NULL);
   mpRaster2 = RasterUtilities::createRasterElement("MultiLayerMovieCube2", mNumRows/2,
      mNumCols/2, mNumBands, INT2UBYTES, BIP, true, NULL);
   mpRaster3 = RasterUtilities::createRasterElement("MultiLayerMovieCube3", mNumRows/4,
      mNumCols/4, mNumBands, INT2UBYTES, BIP, true, NULL);

   if (mpRaster1 == NULL || mpRaster2 == NULL || mpRaster3 == NULL)
   {
      return false;
   }
   else
   {
      return true;
   }
}

bool MultiLayerMovie::populateRasterElements()
{
   FactoryResource<DataRequest> pDataRequest1;
   pDataRequest1->setInterleaveFormat(BIP);
   pDataRequest1->setWritable(true);
   DataAccessor da1 = mpRaster1->getDataAccessor(pDataRequest1.release());
   if (!da1.isValid())
   {
      return false;
   }

   for (int row=0; row<mNumRows; ++row)
   {
      unsigned short *pData = static_cast<unsigned short*>(da1->getRow());
      for (int col=0; col<mNumCols; ++col)
      {
         for (int band=0; band<mNumBands; ++band)
         {
            *pData++ = band;
         }
      }
      da1->nextRow();
   }

   FactoryResource<DataRequest> pDataRequest2;
   pDataRequest2->setInterleaveFormat(BIP);
   pDataRequest2->setWritable(true);
   DataAccessor da2 = mpRaster2->getDataAccessor(pDataRequest2.release());
   if (!da2.isValid())
   {
      return false;
   }

   for (int row=0; row<mNumRows/2; ++row)
   {
      unsigned short *pData = static_cast<unsigned short*>(da2->getRow());
      for (int col=0; col<mNumCols/2; ++col)
      {
         for (int band=0; band<mNumBands; ++band)
         {
            *pData++ = mNumBands + band;
         }
      }
      da2->nextRow();
   }

   FactoryResource<DataRequest> pDataRequest3;
   pDataRequest3->setInterleaveFormat(BIP);
   pDataRequest3->setWritable(true);
   DataAccessor da3 = mpRaster3->getDataAccessor(pDataRequest3.release());
   if (!da3.isValid())
   {
      return false;
   }

   for (int row=0; row<mNumRows/4; ++row)
   {
      unsigned short *pData = static_cast<unsigned short*>(da3->getRow());
      for (int col=0; col<mNumCols/4; ++col)
      {
         for (int band=0; band<mNumBands; ++band)
         {
            *pData++ = 2*mNumBands + band;
         }
      }
      da3->nextRow();
   }

   return true;
}

bool MultiLayerMovie::createWindow()
{
   bool bSuccess = false;

   mpWindow = static_cast<SpatialDataWindow*>(
      Service<DesktopServices>()->createWindow(
      "Multi-Layer Movie",
      SPATIAL_DATA_WINDOW));

   VERIFY(mpWindow != NULL);
   
   SpatialDataView *pView = mpWindow->getSpatialDataView();
   VERIFY(pView != NULL);

   UndoLock lock(pView);

   pView->setPrimaryRasterElement(mpRaster1);
   bSuccess = createLayer(pView, mpRaster1, mpLayer1) &&
      createLayer(pView, mpRaster2, mpLayer2, 0.5, 5, 0.5, 5)  &&
      createLayer(pView, mpRaster3, mpLayer3, 2, -5, 2, -5);

   return bSuccess;
}

bool MultiLayerMovie::createLayer(SpatialDataView* pView, RasterElement* pElement, RasterLayer*& pLayer,
   double xScale, double xOffset, double yScale, double yOffset)
{
   VERIFY(pView != NULL);
   pLayer = dynamic_cast<RasterLayer*>(pView->createLayer(RASTER, pElement));

   if (pLayer != NULL)
   {
      pLayer->enableGpuImage(true);
      pLayer->setStretchUnits(GRAYSCALE_MODE, RAW_VALUE);
      pLayer->setStretchValues(GRAY, 0, 3*mNumBands-1);
      pLayer->enableFilter("ByPass");
      pLayer->setXScaleFactor(xScale);
      pLayer->setXOffset(xOffset);
      pLayer->setYScaleFactor(yScale);
      pLayer->setYOffset(yOffset);
      return true;
   }

   return false;
}

bool MultiLayerMovie::setupAnimations()
{
   // Create the controller
   AnimationController *pController =
      Service<AnimationServices>()->createAnimationController("MultiLayerMovie", FRAME_TIME);
   if (pController == NULL)
   {
      return false;
   }
   pController->setCanDropFrames(false);

   // Set the controller into each of the views
   SpatialDataView *pView = dynamic_cast<SpatialDataView*>(mpWindow->getView());
   if (pView == NULL)
   {
      Service<AnimationServices>()->destroyAnimationController(pController);
      return false;
   }

   pView->setAnimationController(pController);

   // Create the animations for each layer
   Animation *pAnimation1 = pController->createAnimation("MultiLayerMovie1");
   Animation *pAnimation2 = pController->createAnimation("MultiLayerMovie2");
   Animation *pAnimation3 = pController->createAnimation("MultiLayerMovie3");
   if (pAnimation1 == NULL || pAnimation2 == NULL || pAnimation3 == NULL)
   {
      Service<AnimationServices>()->destroyAnimationController(pController);
      return false;
   }

   // Set up the frames for each animation
   const int timeOffset = mNumBands/4;
   vector<AnimationFrame> frames1;
   vector<AnimationFrame> frames2;
   vector<AnimationFrame> frames3;
   for (int i=0; i<mNumBands; ++i)
   {
      AnimationFrame frame1("frame", i, static_cast<double>(i)/mNumBands);
      AnimationFrame frame2("frame", i, static_cast<double>(i+timeOffset)/(mNumBands+timeOffset));
      AnimationFrame frame3("frame", i, static_cast<double>(i+2*timeOffset)/(mNumBands+timeOffset));
      frames1.push_back(frame1);
      frames2.push_back(frame2);
      frames3.push_back(frame3);
   }

   // Set the frames into the animations
   pAnimation1->setFrames(frames1);
   pAnimation2->setFrames(frames2);
   pAnimation3->setFrames(frames3);

   // Assign the animations to the layers
   mpLayer1->setAnimation(pAnimation1);
   mpLayer2->setAnimation(pAnimation2);
   mpLayer3->setAnimation(pAnimation3);

   return true;
}
