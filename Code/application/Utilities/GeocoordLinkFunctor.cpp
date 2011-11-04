/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GeocoordLinkFunctor.h"

#include "AppVerify.h"
#include "GeoConversions.h"
#include "Layer.h"
#include "LayerList.h"
#include "RasterElement.h"
#include "SpatialDataViewImp.h"
#include "ViewImp.h"

#include <complex>

using namespace std;

const int GeocoordLinkFunctor::LINK_TIGHTNESS = 50;
const int GeocoordLinkFunctor::NUM_POINTS = 4;

GeocoordLinkFunctor::GeocoordLinkFunctor(const ViewImp *pSrcView) :
   mpSrcView(dynamic_cast<const SpatialDataViewImp*>(pSrcView)), 
   mpSrcLayer(NULL), mpSrcGeo(NULL), mTightness(LINK_TIGHTNESS), 
   mInitialized(false)
{
}

GeocoordLinkFunctor::~GeocoordLinkFunctor()
{
}

void GeocoordLinkFunctor::operator()(ViewImp *pDestView) const
{
   // lazily prepare the functor with the source information
   if (!const_cast<GeocoordLinkFunctor*>(this)->initialize())
   {
      return;
   }

   SpatialDataViewImp* pDstViewSpatial = dynamic_cast<SpatialDataViewImp*>(pDestView);
   if (pDstViewSpatial == NULL)
   {
      return;
   }

   LayerList* pDstLayerList = pDstViewSpatial->getLayerList();
   if (pDstLayerList == NULL)
   {
      return;
   }

   RasterElement* pDstGeo = pDstLayerList->getPrimaryRasterElement();
   if (pDstGeo == NULL || !pDstGeo->isGeoreferenced())
   {
      return;
   }

   Layer* pDstLayer = pDstLayerList->getLayer(RASTER, pDstGeo);
   if (pDstLayer == NULL)
   {
      return;
   }

   pDstViewSpatial->setPanLimit(NO_LIMIT);
   pDstViewSpatial->setMinimumZoom(0);
   pDstViewSpatial->setMaximumZoom(0);

   LocationType dstDataCenter = pDstGeo->convertGeocoordToPixel(mCenterGeocoord);
   LocationType dstWorldCenter;
   pDstLayer->translateDataToWorld(dstDataCenter.mX, dstDataCenter.mY, 
      dstWorldCenter.mX, dstWorldCenter.mY);
   LocationType dstScreenCenter(pDstViewSpatial->width() / 2.0, pDstViewSpatial->height() / 2.0);

   // Determined zoom and rotation are relative to the view's current state.
   // Reset the zoom and rotation to prevent cascading rounding errors
   pDstViewSpatial->resetOrientation();
   if (pDstViewSpatial->getDataOrigin() == UPPER_LEFT)
   {
      pDstViewSpatial->flipVertical();
   }
   pDstViewSpatial->panTo(dstWorldCenter);
   pDstViewSpatial->zoomTo(100);

   double localTightness = mTightness / findResolution(pDstGeo, pDstLayer, mCenterGeocoord);

   vector<LocationType> dstScreen(NUM_POINTS);
   dstScreen[0] = dstScreenCenter + LocationType(0, localTightness);
   dstScreen[1] = dstScreenCenter + LocationType(0, -localTightness);
   dstScreen[2] = dstScreenCenter + LocationType(localTightness, 0);
   dstScreen[3] = dstScreenCenter + LocationType(-localTightness, 0);

   // (Desired screen offset) / (world offset) yields zoom (magnitude) and rotation (phase).
   vector<complex<double> > transformVec(NUM_POINTS);
   for (int i = 0; i < NUM_POINTS; ++i)
   {
      // dst: screen -> world -> data -> geo
      LocationType dstWorld;
      pDstViewSpatial->translateScreenToWorld(dstScreen[i].mX, dstScreen[i].mY, dstWorld.mX, dstWorld.mY);
      LocationType dstData;
      pDstLayer->translateWorldToData(dstWorld.mX, dstWorld.mY, dstData.mX, dstData.mY);
      LocationType dstGeo = pDstGeo->convertPixelToGeocoord(dstData);

      // src: geo -> data -> screen
      LocationType srcData = mpSrcGeo->convertGeocoordToPixel(dstGeo);
      LocationType srcScreen;
      mpSrcLayer->translateDataToScreen(srcData.mX, srcData.mY, srcScreen.mX, srcScreen.mY);
      complex<double> srcScreenOffset = complex<double>(srcScreen.mX, srcScreen.mY) - mSrcScreenCenter;
      LocationType dstDataOffset = dstWorld - dstWorldCenter;
      transformVec[i] = srcScreenOffset / complex<double>(dstDataOffset.mX, dstDataOffset.mY);
   }

   // find average zoom and rotation for each axis
   complex<double> vertical = (transformVec[0] + transformVec[1]) / complex<double>(2);
   complex<double> horizontal = (transformVec[2] + transformVec[3]) / complex<double>(2);

   // find the aspects from the relative zoom, and correct the
   // vector to get a better rotation and zoom
   double aspect = abs(horizontal) / abs(vertical);
   if (aspect > 1)
   {
      horizontal /= aspect;
   }
   else
   {
      vertical *= aspect;
   }

   complex<double> total = (horizontal + vertical) / complex<double>(2);
   complex<double> totalFlipped = (horizontal - vertical) / complex<double>(2);
   if (abs(totalFlipped) > abs(total))
   {
      total = conj(totalFlipped);
      pDstViewSpatial->flipVertical();
   }

   double zoom = 100 * abs(total);
   double rot = 360 - (arg(total) * 180/PI);

   pDstViewSpatial->rotateTo(fmod(rot, 360));
   pDstViewSpatial->zoomTo(zoom);
   pDstViewSpatial->setPixelAspect(aspect);

}

bool GeocoordLinkFunctor::initialize()
{
   if (mInitialized)
   {
      return true;
   }

   if (mpSrcView != NULL)
   {
      LayerList* pSrcLayerList = mpSrcView->getLayerList();
      if (pSrcLayerList == NULL)
      {
         return false;
      }

      mpSrcGeo = pSrcLayerList->getPrimaryRasterElement();
      if (mpSrcGeo == NULL || !mpSrcGeo->isGeoreferenced())
      {
         return false;
      }

      mpSrcLayer = pSrcLayerList->getLayer(RASTER, mpSrcGeo);
      if (mpSrcLayer == NULL)
      {
         return false;
      }

      LocationType srcWorldCenter(mpSrcView->getVisibleCenter());

      LocationType srcScreenCenter;
      mpSrcView->translateWorldToScreen(srcWorldCenter.mX, srcWorldCenter.mY, 
         srcScreenCenter.mX, srcScreenCenter.mY);
      mSrcScreenCenter = complex<double>(srcScreenCenter.mX, srcScreenCenter.mY);

      LocationType srcDataCenter;
      mpSrcLayer->translateWorldToData(srcWorldCenter.mX, srcWorldCenter.mY, 
         srcDataCenter.mX, srcDataCenter.mY);
      mCenterGeocoord = mpSrcGeo->convertPixelToGeocoord(srcDataCenter);

      mTightness = findResolution(mpSrcGeo, mpSrcLayer, mCenterGeocoord) * LINK_TIGHTNESS;

      mInitialized = true;
      return true;
   }

   return false;
}

double GeocoordLinkFunctor::findResolution(const RasterElement* pGeo, const Layer* pLayer,
                                           const LocationType& geocoord)
{
   VERIFYRV(pLayer != NULL, 1.0);
   VERIFYRV(pGeo != NULL, 1.0);

   LocationType data1 = pGeo->convertGeocoordToPixel(geocoord);
   LocationType screen1;
   pLayer->translateDataToScreen(data1.mX, data1.mY, screen1.mX, screen1.mY);

   LocationType data2;
   LocationType screen2 = screen1 + LocationType(LINK_TIGHTNESS, LINK_TIGHTNESS);
   pLayer->translateScreenToData(screen2.mX, screen2.mY, data2.mX, data2.mY);

   LocationType geocoord2 = pGeo->convertPixelToGeocoord(data2);
   double res = hypot(geocoord2.mX - geocoord.mX, geocoord2.mY - geocoord.mY) / hypot(LINK_TIGHTNESS, LINK_TIGHTNESS);

   return res;
}
