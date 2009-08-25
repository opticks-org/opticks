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
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "MultiMovie.h"
#include "PlugInRegistration.h"
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

REGISTER_PLUGIN_BASIC(OpticksPlugInSamplerQt, MultiMovie);

MultiMovie::MultiMovie() : 
   mpRaster1(NULL), mpRaster2(NULL), mpRaster3(NULL),
   mpWindow1(NULL), mpWindow2(NULL), mpWindow3(NULL),
   mpLayer1(NULL), mpLayer2(NULL), mpLayer3(NULL)
{
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setMenuLocation("[Demo]\\Multi-Movie");
   setName("Multi Movie");
   setDescription("Creates 3 data cubes in separate windows, all attached to a single animation controller");
   setShortDescription( "Creates 3 data cubes in separate windows, all attached to a single animation controller" );
   setDescriptorId("{C2E2BF33-F1E7-46a2-ABA4-63BDD351283E}");
   setWizardSupported(false);
}

bool MultiMovie::getInputSpecification( PlugInArgList * &pArgList )
{
   pArgList = NULL;
   return true;
}

bool MultiMovie::getOutputSpecification( PlugInArgList * &pArgList )
{
   pArgList = NULL;
   return true;
}

bool MultiMovie::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{
   if (!createRasterElements())
   {
      Service<ModelServices>()->destroyElement(mpRaster1);
      Service<ModelServices>()->destroyElement(mpRaster2);
      Service<ModelServices>()->destroyElement(mpRaster3);
      return false;
   }

   if (!populateRasterElements())
   {
      Service<ModelServices>()->destroyElement(mpRaster1);
      Service<ModelServices>()->destroyElement(mpRaster2);
      Service<ModelServices>()->destroyElement(mpRaster3);
      return false;
   }

   if (!createWindow(mpRaster1, mpWindow1, mpLayer1))
   {
      Service<ModelServices>()->destroyElement(mpRaster1);
      Service<ModelServices>()->destroyElement(mpRaster2);
      Service<ModelServices>()->destroyElement(mpRaster3);
      return false;
   }

   if (!createWindow(mpRaster2, mpWindow2, mpLayer2))
   {
      Service<DesktopServices>()->deleteWindow(mpWindow1);
      Service<ModelServices>()->destroyElement(mpRaster2);
      Service<ModelServices>()->destroyElement(mpRaster3);
      return false;
   }

   if (!createWindow(mpRaster3, mpWindow3, mpLayer3))
   {
      Service<DesktopServices>()->deleteWindow(mpWindow1);
      Service<DesktopServices>()->deleteWindow(mpWindow2);
      Service<ModelServices>()->destroyElement(mpRaster3);
      return false;
   }

   if (!setupAnimations())
   {
      Service<DesktopServices>()->deleteWindow(mpWindow1);
      Service<DesktopServices>()->deleteWindow(mpWindow2);
      Service<DesktopServices>()->deleteWindow(mpWindow3);
      return false;
   }

   return true;
}

bool MultiMovie::createRasterElements()
{
   mpRaster1 = RasterUtilities::createRasterElement("MultiMovieCube1", mNumRows,
      mNumCols, mNumBands, INT2UBYTES, BIP, true, NULL);
   mpRaster2 = RasterUtilities::createRasterElement("MultiMovieCube2", mNumRows,
      mNumCols, mNumBands, INT2UBYTES, BIP, true, NULL);
   mpRaster3 = RasterUtilities::createRasterElement("MultiMovieCube3", mNumRows,
      mNumCols, mNumBands, INT2UBYTES, BIP, true, NULL);

   if (mpRaster1 == NULL || mpRaster2 == NULL || mpRaster3 == NULL)
   {
      return false;
   }
   else
   {
      return true;
   }
}

bool MultiMovie::populateRasterElements()
{
   FactoryResource<DataRequest> pDataRequest1;
   pDataRequest1->setInterleaveFormat(BIP);
   pDataRequest1->setWritable(true);
   DataAccessor da1 = mpRaster1->getDataAccessor(pDataRequest1.release());

   FactoryResource<DataRequest> pDataRequest2;
   pDataRequest2->setInterleaveFormat(BIP);
   pDataRequest2->setWritable(true);
   DataAccessor da2 = mpRaster2->getDataAccessor(pDataRequest2.release());

   FactoryResource<DataRequest> pDataRequest3;
   pDataRequest3->setInterleaveFormat(BIP);
   pDataRequest3->setWritable(true);
   DataAccessor da3 = mpRaster3->getDataAccessor(pDataRequest3.release());
   if (!da1.isValid() || !da2.isValid() || !da3.isValid())
   {
      return false;
   }

   for (int row = 0; row < mNumRows; ++row)
   {
      unsigned short* pData1 = static_cast<unsigned short*>(da1->getRow());
      unsigned short* pData2 = static_cast<unsigned short*>(da2->getRow());
      unsigned short* pData3 = static_cast<unsigned short*>(da3->getRow());
      for (int col = 0; col < mNumCols; ++col)
      {
         for (int band = 0; band < mNumBands; ++band)
         {
            *pData1++ = band;
            *pData2++ = mNumBands + band;
            *pData3++ = 2*mNumBands + band;
         }
      }
      da1->nextRow();
      da2->nextRow();
      da3->nextRow();
   }

   return true;
}

bool MultiMovie::createWindow(RasterElement *pElement, SpatialDataWindow *&pWindow, RasterLayer *&pLayer)
{
   bool bSuccess = false;

   VERIFY(pElement != NULL);

   pWindow = static_cast<SpatialDataWindow*>(
      Service<DesktopServices>()->createWindow(
      pElement->getName().c_str(),
      SPATIAL_DATA_WINDOW));

   VERIFY(pWindow != NULL);
   
   SpatialDataView* pView = pWindow->getSpatialDataView();
   VERIFY(pView != NULL);

   UndoLock lock(pView);
   pView->setPrimaryRasterElement(pElement);

   pLayer = dynamic_cast<RasterLayer*>(pView->createLayer(RASTER, pElement));
   if (pLayer != NULL)
   {
      pLayer->enableGpuImage(true);
      pLayer->setStretchUnits(GRAYSCALE_MODE, RAW_VALUE);
      pLayer->setStretchValues(GRAY, 0, 3*mNumBands-1);
      pLayer->enableFilter("ByPass");
      bSuccess = true;
   }

   return bSuccess;
}

bool MultiMovie::setupAnimations()
{
   // Create the controller
   Service<AnimationServices> pServices;

   AnimationController* pController = pServices->createAnimationController("MultiMovie", FRAME_TIME);
   if (pController == NULL)
   {
      return false;
   }
   pController->setCanDropFrames(false);

   // Set the controller into each of the views
   SpatialDataView* pView1 = dynamic_cast<SpatialDataView*>(mpWindow1->getView());
   SpatialDataView* pView2 = dynamic_cast<SpatialDataView*>(mpWindow2->getView());
   SpatialDataView* pView3 = dynamic_cast<SpatialDataView*>(mpWindow3->getView());
   if (pView1 == NULL || pView2 == NULL || pView3 == NULL)
   {
      pServices->destroyAnimationController(pController);
      return false;
   }

   pView1->setAnimationController(pController);
   pView2->setAnimationController(pController);
   pView3->setAnimationController(pController);

   // Create the animations for each layer
   Animation* pAnimation1 = pController->createAnimation("MultiMovie1");
   Animation* pAnimation2 = pController->createAnimation("MultiMovie2");
   Animation* pAnimation3 = pController->createAnimation("MultiMovie3");
   if (pAnimation1 == NULL || pAnimation2 == NULL || pAnimation3 == NULL)
   {
      pServices->destroyAnimationController(pController);
      return false;
   }

   // Set up the frames for each animation
   const int timeOffset = mNumBands/4;
   vector<AnimationFrame> frames1;
   vector<AnimationFrame> frames2;
   vector<AnimationFrame> frames3;
   for (int i = 0; i < mNumBands; ++i)
   {
      AnimationFrame frame1("frame", i, static_cast<double>(i)/mNumBands);
      AnimationFrame frame2("frame", i, static_cast<double>(i+timeOffset)/(mNumBands+timeOffset));
      AnimationFrame frame3("frame", i, static_cast<double>(i+timeOffset)/(mNumBands+timeOffset));
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
