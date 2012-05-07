/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLY2D_H
#define POLY2D_H

#include "AppAssert.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "FusionException.h"
#include "DimensionDescriptor.h"
#include "ModelServices.h"
#include "ProgressTracker.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "Statistics.h"
#include "Vector.h"

#include <math.h>
#include <string>

/**
 * Poly2D
 *
 * Poly2D warps an MxNxO image with an MxN image. Requires Polywarp to be
 * run to producte KX and KY, the warp matrices. dimX and dimY are the size
 * of the new image. xoff and yoff represent the X and Y offsets into the
 * primary image where the chip starts. zoomFactor (always > 1) represents
 * how many times greater the resolution of the secondary image is than the
 * primary image.
 *
 * @throw FusionException
 *        A FusionException is thrown when an unrecoverable error occurs.
 * @throw AssertException
 *        An AssertException is thrown when a bug occurs and the code is attempting to recover.
 *
 * All out-of-bounds values are 0. Uses bilinear interpolation.
 *
 * NOTE: Only the first 'band' of the secondary image is fused with the
 *       primary image!
 *
 * @param  pDummy
 *         Dummy parameter to allow for using switchOnEncoding.
 * @param  pRasterElement
 *         The secondary image
 * @param  pAssocDs
 *         The name of the dataset with which to associate the results.
 *         If NULL, becomes the Dataset Name of pRasterElement.
 * @param  KX
 *         The X warp vector output taken from the Polywarp call.
 * @param  KY
 *         The Y warp vector output taken from the Polywarp call.
 * @param  dimX
 *         The number of columns in S', the warped secondary image chip.
 *         This value already has zoomFactor taken into account.
 * @param  dimY
 *         The number of rows in S', the warped secondary image chip.
 *         This value already has zoomFactor taken into account.
 * @param  xoff
 *         The column offset into the primary image where the primary image
 *         chip starts.
 * @param  yoff
 *         The row offset into the primary image where the primary image
 *         chip starts.
 * @param  zoomFactor
 *         The integer respresenting how much more resolution the secondary
 *         image has than the primary image. It is the greatest integer that
 *         satisifes the following:
 *
 *         zoomFactor*geospatialresolution(primary) ~ geospatialresolution(secondary) and
 *         zoomFactor*geospatialresolution(primary) <= geospatialresolution(secondary)
 * @param  inMemory
 *         Whether the resulting RasterElement is created in memory or on-disk. Defaults
 *         to TRUE.
 * @return The warped image S' that corresponds to the data contained in the
 *         primary image chip.
 */
template<class T>
RasterElement* poly_2D(T* pDummy, RasterElement* pRasterElement,
                     const Vector<double>& KX, const Vector<double>& KY, 
                     unsigned int dimX, unsigned int dimY,
                     unsigned int xoff, unsigned int yoff, int zoomFactor,
                     ProgressTracker& progressTracker, bool inMemory = true)
{
   double x1 = 0, y1 = 0;
   double x_prime = 0, y_prime = 0;
   double u = 0, v = 0;

   const T BAD_VALUE = 0;
   const double THRESHOLD = 0.10; // if 10% of pixels are 'bad', throw up a warning later

   double badValues = 0;

   REQUIRE(pRasterElement != NULL);

   const RasterDataDescriptor* pOrigDescriptor =
      dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());

   REQUIRE(pOrigDescriptor != NULL);
   
   unsigned int nx = pOrigDescriptor->getColumnCount();
   unsigned int ny = pOrigDescriptor->getRowCount();

   if (dimX == 0)
   {
      dimX = nx;
   }

   if (dimY == 0)
   {
      dimY = ny;
   }
   REQUIRE(dimX > 0 && dimY > 0);
   REQUIRE(nx > 0 && ny > 0);

   std::string msg = "Warping image...";

   RasterDataDescriptor* pNewDescriptor = RasterUtilities::generateRasterDataDescriptor("SecondaryPrime",
      pRasterElement, dimY, dimX, 1, BSQ, pOrigDescriptor->getDataType(), inMemory ? IN_MEMORY : ON_DISK);
   pNewDescriptor->setClassification(pOrigDescriptor->getClassification());
   pNewDescriptor->setUnits(pOrigDescriptor->getUnits());

   ModelResource<RasterElement> pNewRaster(pNewDescriptor);
   if (pNewRaster.get() == NULL)
   {
      throw FusionException("Cannot allocate memory for warped image!", __LINE__, __FILE__);
   }

   pNewDescriptor = NULL; // ModelResource deletes it

   FactoryResource<DataRequest> pNewRequest;
   pNewRequest->setWritable(true);
   DataAccessor rmda = pNewRaster->getDataAccessor(pNewRequest.release());
   REQUIRE(rmda.isValid() == true);

   DataAccessor originalData = pRasterElement->getDataAccessor();
   REQUIRE(originalData.isValid() == true);

   unsigned int x2 = 0, y2 = 0;
   float rowProgress = 100.0f/dimY;
   for (unsigned int y = 0; y < dimY; y++)
   {
      if (DataFusionTools::getAbortFlag())
      {
         return NULL;
      }

      T* results = (T*) rmda->getRow();
      for (unsigned int x = 0; x < dimX; ++x) // for every column in s'
      {
         /* Let xoff = offset of ROI in primary image
            x2=x+xoff;
            Let yoff = offset of ROI in primary
            y2=y+yoff
            x_prime = KX[0] + KX[1]*y2 + KX[2]*x2 + KX[3]*x2*y2
            y_prime = KY[0] + KY[1]*y2 + KY[2]*x2 + KY[3]*x2*y2
          */

         const int XNEW = x+(zoomFactor*xoff), YNEW = y+(zoomFactor*yoff);

         x_prime = KX[0] + KX[1]*YNEW + KX[2]*XNEW + KX[3]*XNEW*YNEW;
         y_prime = KY[0] + KY[1]*YNEW + KY[2]*XNEW + KY[3]*XNEW*YNEW;

         x1 = floor(x_prime);
         y1 = floor(y_prime);

         // Handle out of bounds case
         if ((x1 > (nx-1)) || (y1 > (ny-1)) || (x1 < 0) || (y1 < 0))
         {
            badValues++;
            results[x] = BAD_VALUE;
         }
         else
         {
            // bilinear interpolation 
            if ((x1+1) > (double(nx)-1))
            {
               x2 = nx-1;
            }
            else
            {
               x2 = static_cast<unsigned int>(x1+1);
            }
            if ((y1+1) > (double(ny)-1))
            {
               y2 = ny-1;
            }
            else
            {
               y2 = static_cast<unsigned int>(y1+1);
            }
            u = x_prime - x1;
            v = y_prime - y1;

            // the "out of bounds case" above guarnatees that the toPixel() operations will succeed

            originalData->toPixel(static_cast<int>(y1), static_cast<int>(x1));
            const T minXminY = *reinterpret_cast<T*>(originalData->getColumn());

            originalData->toPixel(static_cast<int>(y1), static_cast<int>(x2));
            const T maxXminY = *reinterpret_cast<T*>(originalData->getColumn());

            originalData->toPixel(static_cast<int>(y2), static_cast<int>(x1));
            const T minXmaxY = *reinterpret_cast<T*>(originalData->getColumn());
            
            originalData->toPixel(static_cast<int>(y2), static_cast<int>(x2));
            const T maxXmaxY = *reinterpret_cast<T*>(originalData->getColumn());

            results[x] = static_cast<T>((minXminY * ((1.0 - u) * (1.0 - v))
                                       + maxXminY * (u * (1.0 - v))
                                       + minXmaxY * ((1.0 - u) * v)
                                       + maxXmaxY * (u * v)));
         }
      }
      rmda->nextRow();
      // report 1 row's worth of progress
      int workDone = static_cast<int>(rowProgress * (y + 1));
      progressTracker.report(msg.c_str(), workDone, NORMAL);
   }

   if ((badValues / (dimX * dimY)) > THRESHOLD) 
   {
      std::string txt = "Warning: Too many values in the primary data set are not in the secondary data set! "
         "Possible causes: you selected a region in the primary image that is not in the secondary image, "
         "or the georeferencing is bad!";
      progressTracker.report(txt, 99, WARNING, true);
   }

   Statistics* pStatistics = pNewRaster->getStatistics();
   if (pStatistics != NULL)
   {
      std::vector<int> badVals;
      badVals.push_back((const int)BAD_VALUE);

      pStatistics->setBadValues(badVals);
   }

   progressTracker.report(msg.c_str(), 100, NORMAL);
   return pNewRaster.release();
}

#endif
