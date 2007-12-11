/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOCOORDLINKFUNCTOR_H
#define GEOCOORDLINKFUNCTOR_H

#include "LocationType.h"

#include <vector>
#include <complex>

class Layer;
class RasterElement;
class SpatialDataViewImp;
class ViewImp;

/**
 * This class forms a function object to perform geocoordinate
 * linking.
 */
class GeocoordLinkFunctor
{
public:
   /**
    * Construct the function object.
    *
    * @param pSrcView
    *        Source view to cause other views to match.
    *        This pointer must be valid for the lifetime
    *        of the object.
    */
   GeocoordLinkFunctor(const ViewImp *pSrcView);

   /**
    * Destroy the function object.
    */
   ~GeocoordLinkFunctor();

   /**
    * Perform the appropriate operations to cause pView
    * to geographically match the source view.
    *
    * If the destination is not georeferenced, this is a no-op.
    *
    * @param pDestView
    *        Destination view to cause to match.
    */
   void operator()(ViewImp *pDestView);

private:

   /**
    * Initialize the functor.
    *
    * This is not done in the constructor because it involves
    * a reasonable of math, and the functor may be created but never called.
    *
    * @return True if the object is now initialized, false otherwise.
    */
   bool initialize();

   /**
    * Get the relative geocoordinate-to-screen pixel resolution.
    *
    * @param pGeo
    *        The georeferenced element to use to find the resolution.
    * @param pLayer
    *        The layer to map between data and screen coordinates.
    * @return A value which is approximately proportional to the 
    *         geographic-to-screen of the given layer.  This value 
    *         should only be used to estimate the relative geographic
    *         scale between layers.
    */
   static double findResolution(const RasterElement *pGeo, 
      const Layer *pLayer, const LocationType &geocoord);

   /**
    * Number of points (not including the center) to use to calculate the link
    */
   static const int NUM_POINTS;

   /**
    * Tightness about the center point to calculate the link.
    */
   static const int LINK_TIGHTNESS;

   /**
    * The source view.
    */
   const SpatialDataViewImp *mpSrcView;

   /**
    * The source layer.
    */
   const Layer *mpSrcLayer;

   /**
    * The source georeferenced RasterElement.
    */
   const RasterElement *mpSrcGeo;

   /**
    * The tightness of this instance.  Divide by the result of findResolution() of
    * the destination to find how far around the center in data coordinates to calculate.
    */
   double mTightness;

   /**
    * Whether or not this object has been initialized.
    */
   bool mInitialized;

   /**
    * The center screen coordinate of the source view.
    */
   std::complex<double> mSrcScreenCenter;

   /**
    * The geocoord to place at the center
    */
   LocationType mCenterGeocoord;
};

#endif
