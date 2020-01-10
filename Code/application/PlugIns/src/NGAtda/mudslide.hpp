/*
* Copyright (c) 2011, ITT Geospatial Systems, Ball Aerospace, and Booz Allen
* Hamilton
*
* $Id: mudslide.hpp 116 2012-06-01 14:20:01Z blacktf $
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
* @author Elizabeth Goldberg
* @author Brandon Cox
* @author Bradley J Chambers
*/

#ifndef INCLUDE_MUDSLIDE_H_
#define INCLUDE_MUDSLIDE_H_

// camp
//#include <camp/Core>

// boost
//#include <boost/filesystem.hpp>

//namespace fs = boost::filesystem;

// camp
#include "topographic_attributes.hpp"
#include "raster_io.hpp"

namespace mudslide {

   static const camp::RiskLevel RISK_TABLE[5][3] = {
      {camp::MUDSLIDE_LOW,    camp::MUDSLIDE_LOW,    camp::MUDSLIDE_LOW},
      {camp::MUDSLIDE_LOW,    camp::MUDSLIDE_LOW,    camp::MUDSLIDE_MEDIUM},
      {camp::MUDSLIDE_LOW,    camp::MUDSLIDE_LOW,    camp::MUDSLIDE_HIGH},
      {camp::MUDSLIDE_LOW,    camp::MUDSLIDE_MEDIUM, camp::MUDSLIDE_HIGH},
      {camp::MUDSLIDE_MEDIUM, camp::MUDSLIDE_HIGH,   camp::MUDSLIDE_HIGH}
   };

   /**
   * @class Mudslide
   */
   template <typename Derived>
   class Mudslide {
   public:
      /**
      * Executes the mudslide algorithm.
      *
      * @param none
      * @returns bool true if execution was successful
      */
#if 0
      int run(Derived const& data,
         int const& rows,
         int const& cols,
         float const& post_spacing,
         float const& buffer,
         float const& mudSize,
         fs::path const& out_file,
         fs::path const& in_file,
         GDALDataType const& datatype,
         Derived& risk) {
            Derived slope_raster(rows, cols);
            Derived curve_raster(rows, cols);

            topo::ComputeSlope<Derived>(data, slope_raster, post_spacing);
            topo::ComputeCurvature<Derived>(data, curve_raster, post_spacing,
               camp::PROFILE, 5);

            computeRiskRaster(slope_raster, curve_raster, data, rows, cols, risk);
            std::cout << "Buffer: " << buffer << std::endl;
            fs::path unfilteredPath( out_file.branch_path() /(out_file.stem() + "_MudUnfiltered.tif") );
            raster::writeRaster<Derived>( unfilteredPath, in_file, post_spacing, risk, datatype );
            GDALDatasetH hSrcDS = GDALOpen( unfilteredPath.file_string().c_str(), GA_Update);  // was GA_ReadOnly, perhaps provide an argument to choose
            GDALRasterBandH hSrcBand = GDALGetRasterBand(hSrcDS, 1);
            GDALSieveFilter(hSrcBand, NULL, hSrcBand, mudSize/post_spacing, 8, NULL, NULL, NULL);
            GDALClose( hSrcDS );
            raster::RasterIO filterMud;
            filterMud.getBandData( unfilteredPath, 1, risk );
            remove( unfilteredPath.file_string().c_str() );

            if ( buffer > 0 )
            {
               raster::bufferRaster<Derived>(risk, buffer/post_spacing);
            }
            fs::path slopePath( out_file.branch_path() /(out_file.stem() + "_MudSlope.tif") );
            fs::path curvePath( out_file.branch_path() /(out_file.stem() + "_MudCurve.tif") );
            //raster::writeRaster<Derived>(slopePath, in_file, post_spacing, slope_raster, datatype);
            //raster::writeRaster<Derived>(curvePath, in_file, post_spacing, curve_raster, datatype);
            int minSize = (mudSize + buffer*buffer*3.14159)/post_spacing;

            // calculate and/or place polygons on zones
            // remove small risk areas
            // buffer remaining risk areas
            // write out shape files and/or rasters

            return camp::SUCCESS;
      }
#endif

      /**
      * Given slope angle in degrees, returns the class of the slope, with the
      * class being defined based on the Lahir gold mine slope risk study. This
      * document did not explicitly assign classes to slopes, but instead
      * provided ranges of degrees by which to group points into implied risk
      * classes.
      *
      *                        Plan Curvature Class
      *    Mudslide Class    Slope Angle   Convex    Planar    Concave
      *    I                 0 <= D < 6    Low       Low       Low
      *    II                6 <= D < 12   Low       Low       Medium
      *    III               12 <= D < 18  Low       Low       High
      *    IV                18 <= D < 25  Low       Medium    High
      *    V                 25 <= D       Medium    High      High
      *
      * This table allows the assignment of risk based on slope angle and
      * curvature.
      *
      * @param slopeAngleDegrees angle of slope IN DEGREES
      * @returns slope angle classification
      */
      template <typename T>
      inline const camp::SlopeAngleClass classifyAngle(const T& slopeAngleDegrees) {
         if (slopeAngleDegrees <= 6.0) {
            return camp::ClassI;
         } else if (slopeAngleDegrees <= 12.0) {
            return camp::ClassII;
         } else if (slopeAngleDegrees <= 18.0) {
            return camp::ClassIII;
         } else if (slopeAngleDegrees <= 25.0) {
            return camp::ClassIV;
         } else {
            return camp::ClassV;
         }
      }

      /**
      * Given the plan curvature of a point, classify the curvature as Convex,
      * Planar, or Concave. The convention used in the Lahir gold mine study
      * document is as follows:
      *
      * Convex: curvature <= -.01
      * Concave: curvature >= .01
      * Planar:  -.01 < curvature < .01
      * (units of meters^-1)
      *
      * @param curvature plan curvature in units of m^-1
      * @returns curvature classification
      */
      template <typename T>
      inline const camp::CurvatureClass classifyCurvature(const T& curvature) {
         if (curvature <= -.01) {
            return camp::CONVEX;
         } else if (curvature < .01) {
            return camp::PLANAR;
         } else {
            return camp::CONCAVE;
         }
      }

      /**
      *
      * Return a risk level from the above table based on the slope and curvature
      * class.
      *
      * @param slopeDegrees slope angle in degrees
      * @param curvature the plan curvature in 1/meters
      * @returns mudslide risk level
      */
      template <typename T>
      inline const camp::RiskLevel classifyRisk(const T& slopeDegrees,
         const T& curvature) {
            camp::CurvatureClass curvClass = classifyCurvature<T>(curvature);
            camp::SlopeAngleClass slopeClass = classifyAngle<T>(slopeDegrees);

            return RISK_TABLE[slopeClass][curvClass];
      }

      /**
      * Populates the riskRaster using the slope and curvature rasters and
      * classifying based on the Lahir gold mine study parameters
      */
      int computeRiskRaster(Derived const& slope_raster,
         Derived const& curve_raster,
         Derived const& data,
         int const& rows,
         int const& cols,
         Derived& risk_raster) {
            for (int r = 0; r < rows; r++) {
               for (int c = 0; c < cols; c++) {			
                  if ( data(r,c) == camp::kNoData )
                  {
                     risk_raster(r,c) = camp::kNoData;
                  }
                  else if ( abs(slope_raster(r,c)) > 90 )
                  {
                     risk_raster(r,c) = 0;
                  }
                  else
                  {
                     float slopeDegrees = atan(slope_raster(r, c)) * 180 / camp::kPI;
                     switch (classifyRisk<float>(slopeDegrees,
                        curve_raster(r, c))) {
                     case camp::MUDSLIDE_HIGH:
                        risk_raster(r, c) = 3;
                        break;
                     case camp::MUDSLIDE_MEDIUM:
                        risk_raster(r, c) = 2;
                        break;
                     case camp::MUDSLIDE_LOW:
                        risk_raster(r, c) = 1;
                        break;
                     default:
                        risk_raster(r,c) = 0;
                        break;
                     }
                  }
               }
            }

            return camp::SUCCESS;
      }
   };
}  // namespace mudslide

#endif  // INCLUDE_MUDSLIDE_H_
