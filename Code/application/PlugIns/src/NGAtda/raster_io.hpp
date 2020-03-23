/*
 * Copyright (c) 2011, ITT Geospatial Systems, Ball Aerospace, and Booz Allen
 * Hamilton
 *
 * $Id: raster_io.hpp 114 2012-05-31 13:33:23Z blacktf $
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any
 * later version.
 *
 * Alternatively, you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License and
 * a copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 *
 * @author Bradley J Chambers
 */

#ifndef INCLUDE_RASTER_IO_H_
#define INCLUDE_RASTER_IO_H_

//#include <camp/Core>
//#include <camp/Raster>

// std
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

// eigen
#include <Eigen/Core>

namespace raster {

/**
* This adds a buffer zone around the input raster, out to the requested number of pixels
* It assumes that the input raster contains only 0, 1, and the noData values.
*
* @param dem_raster the input raster that has the locations that need a buffer zone.
* @param bufferPixels the number of pixels to buffer.(Caller should ensure that this number
* has been adjusted to the spacing of the raster.)
* @return camp::SUCCESS if successful.
*/
   template <typename Derived>
   int CAMP_EXPORTS bufferRaster(Derived& dem_raster, int bufferPixels)
   {

      // should bufferPixels actually be passed as meters and then converted here?

      //int rows = dem_raster.rows();
      //int cols = dem_raster.cols();

      // x, y are counter-intuitive, consider switcing
      int rows = dem_raster.rows();
      int cols = dem_raster.cols();
#pragma omp parallel for
      for (int x = 0; x < rows; x++) {

         for (int y = 0; y < cols; y++) {
            int b = bufferPixels;
            int b2 = bufferPixels;

            if (dem_raster(x, y) == 0) {
               while (b > -(bufferPixels + 1)) {
                  b2 = bufferPixels;

                  while (b2 > -(bufferPixels + 1))
                  {
                     if (x - b > -1 && y - b2 > -1 && (x - b) < rows && (y - b2) < cols   ) // dont go out of bounds
                     { 
                        double diagonal_sum = b*b + b2*b2;
                        double diagonal = sqrt(diagonal_sum);

                        if (dem_raster(x - b, y - b2) == 1 && dem_raster(x,y)!=camp::kNoData && diagonal<=bufferPixels)
                        {

                           dem_raster(x, y) = 9;
                        }
                     }

                     b2--;
                  }

                  b--;
               }
            }
         }
      }

      for (int r = 0; r < rows; r++) {
         for (int c = 0; c < cols; c++) {
            if (dem_raster(r, c) == 9) {
               dem_raster(r, c) = 1;
            }
         }
      }

      return camp::SUCCESS;
   }

}  // namespace raster

#endif  // INCLUDE_RASTER_IO_H_
