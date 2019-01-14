/*
* Copyright (c) 2011, ITT Geospatial Systems, Ball Aerospace, and Booz Allen
* Hamilton
*
* $Id: topographic_attributes.hpp 108 2012-04-11 16:13:06Z blacktf $
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
* @author Tim Black
*/

#ifndef INCLUDE_TOPOGRAPHIC_ATTRIBUTES_HPP_
#define INCLUDE_TOPOGRAPHIC_ATTRIBUTES_HPP_

// std
#include <cassert>
#include <cmath>
#include <iostream>

// eigen
#include <Eigen/Core>

// camp
#include "Constants.h"

#include "ProgressTracker.h"

#define CAMP_EXPORTS

namespace topo {

   /**
   * Simplifies debug statements
   */
   /*void printDebug( std::string statement )
   {
      //std::cout << statement << std::endl;
   }*/

   /**
   * Shifts matrix A by n columns, using symmetric padding.
   *
   * @param m The input matrix A.
   * @param n The number of columns to shift. Values less than zero shift left,
   *          while values greather than zero shift right.
   * @returns The input matrix A, shifted by n columns.
   */
   template <typename Derived>
   Derived shiftX(Derived const& A, int n)
   {
      int rows = A.rows();
      int cols = A.cols();

      Derived output(rows, cols);

      if (n < 0) {
         n *= -1;
         output.block(0, 0, rows, cols - n) = A.block(0, n, rows, cols - n);
         output.col(cols - n) = A.col(cols - n);
      } else if (n > 0) {
         output.block(0, n, rows, cols - n) = A.block(0, 0, rows, cols - n);
         output.col(0) = A.col(0);
      } else {
         output = A;
      }

      return output;
   }

   /**
   * Shifts matrix A by n rows, using symmetric padding.
   *
   * @param m The input matrix A.
   * @param n The number of rows to shift. Values less than zero shift up, while
   *          values greather than zero shift down.
   * @returns The input matrix A, shifted by n rows.
   */
   template <typename Derived>
   Derived shiftY(Derived const& A, int n)
   {
      int rows = A.rows();
      int cols = A.cols();

      Derived output(rows, cols);

      if (n < 0) {
         n *= -1;
         output.block(0, 0, rows - n, cols) = A.block(n, 0, rows - n, cols);
         output.row(rows - n) = A.row(rows - n);
      } else if (n > 0) {
         output.block(n, 0, rows - n, cols) = A.block(0, 0, rows - n, cols);
         output.row(0) = A.row(0);
      } else {
         output = A;
      }

      return output;
   }

   /**
   * Computes first derivative in the x direction.
   *
   * @param m The input matrix A.
   * @param h The post spacing.
   * @returns The first derivative in the x direction.
   */
   template <typename Derived>
   inline Derived computeZx(Derived const& A, float h, int offset=1)
   {
      assert(h >= 0);

      int rows = A.rows();
      int cols = A.cols();

      Derived output(rows, cols);

      output = topo::shiftX<Derived>(A, offset) - topo::shiftX<Derived>(A, (-1*offset));
      output /= (2 * h);

      return output;
   }

   /**
   * Computes first derivative in the y direction.
   *
   * @param m The input matrix A.
   * @param h The post spacing.
   * @returns The first derivative in the y direction.
   */
   template <typename Derived>
   inline Derived computeZy(Derived const& A, float h, int offset=1)
   {
      assert(h >= 0);

      int rows = A.rows();
      int cols = A.cols();

      Derived output(rows, cols);

      output = topo::shiftY<Derived>(A, offset) - topo::shiftY<Derived>(A, (-1*offset));
      output /= (2 * h);

      return output;
   }

   /**
   * Computes the value p.
   *
   * @param zx The first derivative in the x direction.
   * @param zy The first derivative in the y direction.
   * @returns The value p.
   */
   template <typename Derived>
   inline Derived computeP(Derived const& zx, Derived const& zy)
   {
      int rows = zx.rows();
      int cols = zx.cols();

      Derived output(rows, cols);

      output = zx.array().pow(2) + zy.array().pow(2);

      return output;
   }

   /**
   * Computes the value p.
   *
   * @param m The input matrix A.
   * @param h The post spacing.
   * @returns The value p.
   */
   template <typename Derived>
   inline Derived computeP(Derived const& A, float h, int offset = 1)
   {
      assert(h >= 0);

      int rows = A.rows();
      int cols = A.cols();

      Derived zx(rows, cols);
      zx = topo::computeZx<Derived>(A, h, offset);

      Derived zy(rows, cols);
      zy = topo::computeZy<Derived>(A, h, offset);

      Derived output(rows, cols);
      output = topo::computeP<Derived>(zx, zy);

      return output;
   }

   /**
   * Computes the slope from the matrix using the finite distance method.
   *
   * ----------------
   * | z7 | z8 | z9 |
   * ----------------
   * | z6 | z1 | z2 |
   * ----------------
   * | z5 | z4 | z3 |
   * ----------------
   *
   * zx = (z2 - z6) / (2*h)
   * zy = (z8 - z4) / (2*h)
   * p = zx^2 + zy^2
   * slope = sqrt(p)
   *
   * @param dem_data the source matrix used to compute the slope
   * @return the Raster with the computed slope values.
   */
   template <typename Derived>
   inline CAMP_EXPORTS int ComputeSlope(
      Derived const& dem_data,
      Derived& slope,
      float const& h,
      int kernel_size = 3)
   {
      assert(h >= 0);

      int rows = dem_data.rows();
      int cols = dem_data.cols();
      int offset = kernel_size/2;

      Derived p(rows, cols);
      p = topo::computeP<Derived>(dem_data, h, offset);

      slope = p.array().sqrt();

      return camp::SUCCESS;
   }

   template <typename Derived>
   inline CAMP_EXPORTS int ComputeD8Slope(
      Derived const& dem_data,
      Derived& slope,
      float const& h,
      int kernel_size = 3)
   {
      double tPhi1 = 1.0;
      double tPhi2 = sqrt(2.0f);
      double tH = h;
      int tXStart = 1, tXEnd = dem_data.cols() - 1;
      int tYStart = 1, tYEnd = dem_data.rows() - 1;

#pragma omp parallel for

      for (int tXOut = tXStart; tXOut < tXEnd; tXOut++) {
         int tXIn = tXOut;

         for (int tYOut = tYStart; tYOut < tYEnd; tYOut++) {
            // std::cout << "tYOut: " << tYOut << std::endl;
            double tVal, tN, tS, tE, tW, tNW, tNE, tSW, tSE, nextTVal;
            int tNextY, tNextX;
            double tSlopeVal, tSlopeValDegree;
            int tYIn = tYOut;
            tSlopeVal = camp::kNoData;

            // North
            tVal = dem_data(tYOut, tXOut);
            tNextY = tYOut - 1;
            tNextX = tXOut;
            nextTVal = dem_data(tNextY, tNextX);

            if (nextTVal != camp::kNoData) {
               tN = (tVal - nextTVal) / (tH * tPhi1);

               if (tN > tSlopeVal || tSlopeVal == camp::kNoData) {
                  tSlopeVal = tN;
               }
            }

            // South
            tNextY = tYOut + 1;
            tNextX = tXOut;
            nextTVal = dem_data(tNextY, tNextX);

            if (nextTVal != camp::kNoData) {
               tS = (tVal - nextTVal) / (tH * tPhi1);

               if (tS > tSlopeVal || tSlopeVal == camp::kNoData) {
                  tSlopeVal = tS;
               }
            }

            // East
            tNextY = tYOut;
            tNextX = tXOut + 1;
            nextTVal = dem_data(tNextY, tNextX);

            if (nextTVal != camp::kNoData) {
               tE = (tVal - nextTVal) / (tH * tPhi1);

               if (tE > tSlopeVal || tSlopeVal == camp::kNoData) {
                  tSlopeVal = tE;
               }
            }

            // West
            tNextY = tYOut;
            tNextX = tXOut - 1;
            nextTVal = dem_data(tNextY, tNextX);

            if (nextTVal != camp::kNoData) {
               tW = (tVal - nextTVal) / (tH * tPhi1);

               if (tW > tSlopeVal || tSlopeVal == camp::kNoData) {
                  tSlopeVal = tW;
               }
            }

            // NorthEast
            tNextY = tYOut - 1;
            tNextX = tXOut + 1;
            nextTVal = dem_data(tNextY, tNextX);

            if (nextTVal != camp::kNoData) {
               tNE = (tVal - nextTVal) / (tH * tPhi2);

               if (tNE > tSlopeVal || tSlopeVal == camp::kNoData) {
                  tSlopeVal = tNE;
               }
            }

            // NorthWest
            tNextY = tYOut - 1;
            tNextX = tXOut - 1;
            nextTVal = dem_data(tNextY, tNextX);

            if (nextTVal != camp::kNoData) {
               tNW = (tVal - nextTVal) / (tH * tPhi2);

               if (tNW > tSlopeVal || tSlopeVal == camp::kNoData) {
                  tSlopeVal = tNW;
               }
            }

            // SouthEast
            tNextY = tYOut + 1;
            tNextX = tXOut + 1;
            nextTVal = dem_data(tNextY, tNextX);

            if (nextTVal != camp::kNoData) {
               tSE = (tVal - nextTVal) / (tH * tPhi2);

               if (tSE > tSlopeVal || tSlopeVal == camp::kNoData) {
                  tSlopeVal = tSE;
               }
            }

            // SouthWest
            tNextY = tYOut + 1;
            tNextX = tXOut - 1;
            nextTVal = dem_data(tNextY, tNextX);

            if (nextTVal != camp::kNoData) {
               tSW = (tVal - nextTVal) / (tH * tPhi2);

               if (tSW > tSlopeVal || tSlopeVal == camp::kNoData) {
                  tSlopeVal = tSW;
               }
            }

            tSlopeValDegree = camp::kNoData;

            if (tSlopeVal != camp::kNoData) {
               tSlopeValDegree = atan(tSlopeVal) * camp::kPIDegree / camp::kPI;
            }

            slope(tYIn, tXIn) = tSlopeValDegree;
         }
      }

      return camp::SUCCESS;
   }

   /**
   * Compute curvature product
   * Prerequisites: The input file name has been populated in the framework object.
   *
   * @param dem_data is the Eigen matrix storing the DEM data.
   * @param post_spacing is the pixel spacing of the matrix
   * @return A Raster<T>* pointer that contains the curvature values.
   */
   template <typename Derived>
   CAMP_EXPORTS int ComputeCurvature(
      Derived const& dem_data,
      Derived& curvature,
      Derived& zxx,
      Derived& zyy,
      float const& h,
      camp::Curvature const& curveType,
      camp::Rotation const& rotation = camp::EW)
   {
      return topo::ComputeCurvature( dem_data, curvature, zxx, zyy,
         h, 3, curveType, rotation );
      /*assert(h >= 0);

      int cols = dem_data.cols();
      int rows = dem_data.rows();

      Derived xshiftl(rows, cols);
      Derived xshiftr(rows, cols);
      Derived yshiftu(rows, cols);
      Derived yshiftd(rows, cols);
      Derived shiftdl(rows, cols);
      Derived shiftdr(rows, cols);
      Derived shiftul(rows, cols);
      Derived shiftur(rows, cols);
      Derived zx(rows, cols);
      Derived zy(rows, cols);
      Derived p(rows, cols);
      Derived q(rows, cols);
      Derived zxy(rows, cols);

      xshiftl = topo::shiftX<Derived>(dem_data, -1);
      xshiftr = topo::shiftX<Derived>(dem_data, 1);

      yshiftu = topo::shiftY<Derived>(dem_data, -1);
      yshiftd = topo::shiftY<Derived>(dem_data, 1);

      shiftdl.block(1, 0, rows - 1, cols - 1) = dem_data.block(0, 1, rows - 1, cols - 1);
      shiftdl.row(0) = shiftdl.row(1);
      shiftdl.col(cols - 1) = shiftdl.col(cols - 2);

      shiftdr.block(1, 1, rows - 1, cols - 1) = dem_data.block(0, 0, rows - 1, cols - 1);
      shiftdr.row(0) = shiftdr.row(1);
      shiftdr.col(0) = shiftdr.col(1);

      shiftul.block(0, 0, rows - 1, cols - 1) = dem_data.block(1, 1, rows - 1, cols - 1);
      shiftul.row(rows - 1) = shiftul.row(rows - 2);
      shiftul.col(cols - 1) = shiftul.col(cols - 2);

      shiftur.block(0, 1, rows - 1, cols - 1) = dem_data.block(1, 0, rows - 1, cols - 1);
      shiftur.row(rows - 1) = shiftur.row(rows - 2);
      shiftur.col(0) = shiftur.col(1);

      switch (rotation) {
      case camp::EW:
      zx = (xshiftr - xshiftl) / (2 * h);
      zy = (yshiftd - yshiftu) / (2 * h);
      p = topo::computeP<Derived>(zx, zy);
      q = p + Derived::Ones(rows, cols);
      zxx = (xshiftl.array() - 2 * dem_data.array() + xshiftr.array()) / (std::pow(h, 2));
      zyy = (yshiftd.array() - 2 * dem_data.array() + yshiftu.array()) / (std::pow(h, 2));
      zxy = (shiftdl + shiftur - shiftdr - shiftul) / (4 * std::pow(h, 2));

      break;

      case camp::NESW:
      zx = (shiftur - shiftdl) / (2 * h);
      zy = (shiftdr - shiftul) / (2 * h);
      p = topo::computeP<Derived>(zx, zy);
      q = p + Derived::Ones(rows, cols);
      zxx = (shiftdl.array() - 2 * dem_data.array() + shiftur.array()) / (std::pow(h, 2));
      zyy = (shiftdr.array() - 2 * dem_data.array() + shiftul.array()) / (std::pow(h, 2));
      zxy = (yshiftd + yshiftu - xshiftr - xshiftl) / (4 * std::pow(h, 2));

      break;

      case camp::NS:
      zx = (yshiftu - yshiftd) / (2 * h);
      zy = (xshiftr - xshiftl) / (2 * h);
      p = topo::computeP<Derived>(zx, zy);
      q = p + Derived::Ones(rows, cols);
      zxx = (yshiftd.array() - 2 * dem_data.array() + yshiftu.array()) / (std::pow(h, 2));
      zyy = (xshiftr.array() - 2 * dem_data.array() + xshiftl.array()) / (std::pow(h, 2));
      zxy = (shiftdr + shiftul - shiftur - shiftdl) / (4 * std::pow(h, 2));

      break;

      case camp::NWSE:
      zx = (shiftul - shiftdr) / (2 * h);
      zy = (shiftur - shiftdl) / (2 * h);
      p = topo::computeP<Derived>(zx, zy);
      q = p + Derived::Ones(rows, cols);
      zxx = (shiftdr.array() - 2 * dem_data.array() + shiftul.array()) / (std::pow(h, 2));
      zyy = (shiftur.array() - 2 * dem_data.array() + shiftdl.array()) / (std::pow(h, 2));
      zxy = (xshiftr + xshiftl - yshiftu - yshiftd) / (4 * std::pow(h, 2));

      break;
      }

      switch (curveType) {
      case camp::CONTOUR:
      curvature = 100 * ((zxx.array() * zx.array().pow(2)) +
      (2 * zxy.array() * zx.array() * zy.array()) +
      (zyy.array() * zy.array().pow(2))) /
      (p.array() * q.array().pow(3).sqrt());

      break;

      case camp::PROFILE:
      curvature = ((zxx.array() * zx.array().pow(2)) +
      (2 * zxy.array() * zx.array() * zy.array()) +
      (zyy.array() * zy.array().pow(2))) /
      (p.array() * q.array().pow(3).sqrt());

      break;

      case camp::TANGENTIAL:
      curvature = ((zxx.array() * zy.array().pow(2)) +
      (2 * zxy.array() * zx.array() * zy.array()) +
      (zyy.array() * zx.array().pow(2))) /
      (p.array() * q.array().sqrt());

      break;

      case camp::TOTAL:
      curvature = (zxx.array().pow(2)) +
      (2 * zxy.array().pow(2)) +
      (zyy.array().pow(2));

      break;
      }

      return camp::SUCCESS;*/
   }

   /**
   * Compute curvature product
   * Prerequisites: The input file name has been populated in the framework object.
   *
   * @param dem_data is the Eigen matrix storing the DEM data.
   * @param post_spacing is the pixel spacing of the matrix
   * @return A Raster<T>* pointer that contains the curvature values.
   */
   template <typename Derived>
   CAMP_EXPORTS int ComputeCurvature(
      Derived const& dem_data,
      Derived& curvature,
      Derived& zxx,
      Derived& zyy,
      float const& h,
      int kernel_size,
      camp::Curvature const& curveType,
      camp::Rotation const& rotation = camp::EW)
   {
      assert(h >= 0);

      int cols = dem_data.cols();
      int rows = dem_data.rows();
      int windowOffset = kernel_size/2;

      Derived xshiftl(rows, cols);
      Derived xshiftr(rows, cols);
      Derived yshiftu(rows, cols);
      Derived yshiftd(rows, cols);
      Derived shiftdl(rows, cols);
      Derived shiftdr(rows, cols);
      Derived shiftul(rows, cols);
      Derived shiftur(rows, cols);
      Derived zx(rows, cols);
      Derived zy(rows, cols);
      Derived p(rows, cols);
      Derived q(rows, cols);
      Derived zxy(rows, cols);

      xshiftl = topo::shiftX<Derived>(dem_data, windowOffset * -1);
      xshiftr = topo::shiftX<Derived>(dem_data, windowOffset);

      yshiftu = topo::shiftY<Derived>(dem_data, windowOffset * -1);
      yshiftd = topo::shiftY<Derived>(dem_data, windowOffset);

      shiftdl.block(windowOffset, 0, rows - windowOffset, cols - windowOffset) 
         = dem_data.block(0, windowOffset, rows - windowOffset, cols - windowOffset);
      shiftdl.row(0) = shiftdl.row(windowOffset);
      shiftdl.col(cols - windowOffset) = shiftdl.col(cols - windowOffset - 1);

      shiftdr.block(windowOffset, windowOffset, rows - windowOffset, cols - windowOffset)
         = dem_data.block(0, 0, rows - windowOffset, cols - windowOffset);
      shiftdr.row(0) = shiftdr.row(windowOffset);
      shiftdr.col(0) = shiftdr.col(windowOffset);

      shiftul.block(0, 0, rows - windowOffset, cols - windowOffset)
         = dem_data.block(windowOffset, windowOffset, rows - windowOffset, cols - windowOffset);
      shiftul.row(rows - windowOffset) = shiftul.row(rows - windowOffset - 1);
      shiftul.col(cols - windowOffset) = shiftul.col(cols - windowOffset - 1);

      shiftur.block(0, windowOffset, rows - windowOffset, cols - windowOffset)
         = dem_data.block(windowOffset, 0, rows - windowOffset, cols - windowOffset);
      shiftur.row(rows - windowOffset) = shiftur.row(rows - windowOffset - 1);
      shiftur.col(0) = shiftur.col(windowOffset);

      switch (rotation) {
      case camp::EW:
         zx = (xshiftl - xshiftr) / (2 * h);
         zy = (yshiftd - yshiftu) / (2 * h);
         p = topo::computeP<Derived>(zx, zy);
         q = p + Derived::Ones(rows, cols);
         zxx = (xshiftl.array() - 2 * dem_data.array() + xshiftr.array()) / (std::pow(h, 2));
         zyy = (yshiftd.array() - 2 * dem_data.array() + yshiftu.array()) / (std::pow(h, 2));
         zxy = (shiftdl + shiftur - shiftdr - shiftul) / (4 * std::pow(h, 2));

         break;

      case camp::NESW:
         zx = (shiftdl - shiftur) / (2 * h);
         zy = (shiftdr - shiftul) / (2 * h);
         p = topo::computeP<Derived>(zx, zy);
         q = p + Derived::Ones(rows, cols);
         zxx = (shiftdl.array() - 2 * dem_data.array() + shiftur.array()) / (std::pow(h, 2));
         zyy = (shiftdr.array() - 2 * dem_data.array() + shiftul.array()) / (std::pow(h, 2));
         zxy = (yshiftd + yshiftu - xshiftr - xshiftl) / (4 * std::pow(h, 2));

         break;

      case camp::NS:
         zx = (yshiftd - yshiftu) / (2 * h);
         zy = (xshiftl - xshiftr) / (2 * h);
         p = topo::computeP<Derived>(zx, zy);
         q = p + Derived::Ones(rows, cols);
         zxx = (yshiftd.array() - 2 * dem_data.array() + yshiftu.array()) / (std::pow(h, 2));
         zyy = (xshiftr.array() - 2 * dem_data.array() + xshiftl.array()) / (std::pow(h, 2));
         zxy = (shiftdl + shiftur - shiftul - shiftdr) / (4 * std::pow(h, 2));

         break;

      case camp::NWSE:
         zx = (shiftdr - shiftul) / (2 * h);
         zy = (shiftur - shiftdl) / (2 * h);
         p = topo::computeP<Derived>(zx, zy);
         q = p + Derived::Ones(rows, cols);
         zxx = (shiftdr.array() - 2 * dem_data.array() + shiftul.array()) / (std::pow(h, 2));
         zyy = (shiftur.array() - 2 * dem_data.array() + shiftdl.array()) / (std::pow(h, 2));
         zxy = (xshiftr + xshiftl - yshiftu - yshiftd) / (4 * std::pow(h, 2));

         break;
      }

      switch (curveType) {
      case camp::CONTOUR:
         curvature = 100 * ((zxx.array() * zx.array().pow(2)) +
            (2 * zxy.array() * zx.array() * zy.array()) +
            (zyy.array() * zy.array().pow(2))) /
            (p.array() * q.array().pow(3).sqrt());

         break;

      case camp::PROFILE:
         curvature = ((zxx.array() * zx.array().pow(2)) +
            (2 * zxy.array() * zx.array() * zy.array()) +
            (zyy.array() * zy.array().pow(2))) /
            (p.array() * q.array().pow(3).sqrt());

         break;

      case camp::TANGENTIAL:
         curvature = ((zxx.array() * zy.array().pow(2)) +
            (2 * zxy.array() * zx.array() * zy.array()) +
            (zyy.array() * zx.array().pow(2))) /
            (p.array() * q.array().sqrt());

         break;

      case camp::TOTAL:
         curvature = (zxx.array().pow(2)) +
            (2 * zxy.array().pow(2)) +
            (zyy.array().pow(2));

         break;
      }

      return camp::SUCCESS;
   }
   /**
   * Compute curvature product
   * Prerequisites: The input file name has been populated in the framework object.
   *
   * @param dem_data is the Eigen matrix storing the DEM data.
   * @param post_spacing is the pixel spacing of the matrix
   * @return A Raster<T>* pointer that contains the curvature values.
   */
   template <typename Derived>
   CAMP_EXPORTS int ComputeCurvature(
      Derived const& dem_data,
      Derived& curvature,
      float const& h,
      camp::Curvature const& curveType,
      camp::Rotation const& rotation = camp::EW )
   {
      assert(h >= 0);

      int rows = dem_data.rows();
      int cols = dem_data.cols();

      Derived zxx(rows, cols);
      Derived zyy(rows, cols);

      if (!topo::ComputeCurvature(dem_data, curvature, zxx , zyy, h, 3, curveType, rotation)) {
         std::cerr << "Error computing curvature" << std::endl;
         return camp::FAILURE;
      }

      return camp::SUCCESS;
   }

   /**
   * Compute curvature product
   * Prerequisites: The input file name has been populated in the framework object.
   *
   * @param dem_data is the Eigen matrix storing the DEM data.
   * @param post_spacing is the pixel spacing of the matrix
   * @return A Raster<T>* pointer that contains the curvature values.
   */
   template <typename Derived>
   CAMP_EXPORTS int ComputeCurvature(
      Derived const& dem_data,
      Derived& curvature,
      float const& h,
      camp::Curvature const& curveType,
      int kernel_size = 3)
   {
      assert(h >= 0);

      int rows = dem_data.rows();
      int cols = dem_data.cols();

      Derived zxx(rows, cols);
      Derived zyy(rows, cols);

      if (!topo::ComputeCurvature(dem_data, curvature, zxx , zyy, h, kernel_size, curveType, camp::EW)) {
         std::cerr << "Error computing curvature" << std::endl;
         return camp::FAILURE;
      }

      return camp::SUCCESS;
   }

   /**
   * Tests to see if all four curves through the point are convex
   *
   * @param convexity An array of four convexity values.
   * @param curve_tolerance the minimum threshold for a curve to register.
   * @return true if the point is convex in all directions.
   */
   template <typename T>
   inline bool allConvex(T* xConvexity, T* yConvexity, T curve_tolerance)
   {
      bool allConvex = true;
      /*std::cout << "XConvex: ";
      for ( int i = 0; i < 4; i++ )
      {
      std::cout << xConvexity[i] << " ";
      }
      std::cout << std::endl;
      std::cout << "YConvex: ";
      for ( int i = 0; i < 4; i++ )
      {
      std::cout << yConvexity[i] << " ";
      }
      std::cout << std::endl;*/
      for (int i = 0; i < 4; i++) {
         if (abs(xConvexity[i]) < curve_tolerance
            || xConvexity[i] > 0
            || abs(yConvexity[i]) < curve_tolerance
            || yConvexity[i] > 0) {
               allConvex = false;
               break;
         }
      }

      return allConvex;
   }

   /**
   * Tests to see if all four curves through the point are concave
   *
   * @param convexity An array of four convexity values.
   * @param curve_tolerance the minimum threshold for a curve to register.
   * @return true if the point is concave in all directions.
   */
   template <typename T>
   inline  bool allConcave(T* xConvexity, T* yConvexity, T curve_tolerance)
   {
      bool allConcave = true;
      /*std::cout << "XConcave: ";
      for ( int i = 0; i < 4; i++ )
      {
      std::cout << xConvexity[i] << " ";
      }
      std::cout << std::endl;
      std::cout << "YConcave: ";
      for ( int i = 0; i < 4; i++ )
      {
      std::cout << yConvexity[i] << " ";
      }
      std::cout << std::endl;*/

      for (int i = 0; i < 4; i++) {
         if (abs(xConvexity[i]) < curve_tolerance
            || xConvexity[i] < 0
            || abs(yConvexity[i]) < curve_tolerance
            || yConvexity[i] < 0) {
               allConcave = false;
               break;
         }
      }

      return allConcave;
   }

   /**
   * Computes the aspect(slope direction) of the DEM passed into the method.
   */
   template <typename Derived>
   Derived ComputeAspect(Derived tDemData, double tPostSpacing)
   {
      //  float tPhi1 = 1.0f;
      //  float tPhi2 = sqrt(2.0f);
      int nDim = tDemData.rows();
      int mDim = tDemData.cols();
      double tH = tPostSpacing;
      int tXStart = 1, tXEnd = mDim - 1;
      int tYStart = 1, tYEnd = nDim - 1;
      Derived poRasterData(nDim, mDim);

      // Compute corners
      double tVal, tN, tS, tE, tW, zX, zY;
      //  float nextTVal;
      double tAspectVal, aspect;
      zX = (tDemData(0, 1) - tDemData(0, 0)) / tPostSpacing;
      zY = (tDemData(0, 0) - tDemData(1, 0)) / tPostSpacing;
      aspect = atan(zY / zX) * 180 / camp::kPI;
      poRasterData(0,0) = 180 - aspect + 90 * (zX / abs(zX));
      zX = (tDemData(0, tXEnd) - tDemData(0, tXEnd - 1)) / tPostSpacing;
      zY = (tDemData(0, tXEnd) - tDemData(1, tXEnd)) / tPostSpacing;
      aspect = atan(zY / zX) * 180 / camp::kPI;
      poRasterData(0, mDim - 1) = 180 - aspect + 90 * (zX / abs(zX));
      zX = (tDemData(tYEnd - 1, 1) - tDemData(tYEnd - 1, 0)) / tPostSpacing;
      zY = (tDemData(tYEnd - 2, 0) - tDemData(tYEnd - 1, 0)) / tPostSpacing;
      aspect = atan(zY / zX) * 180 / camp::kPI;
      poRasterData(nDim - 1, 0) = 180 - aspect + 90 * (zX / abs(zX));
      zX = (tDemData(tYEnd, tXEnd) - tDemData(tYEnd, tXEnd - 1)) / tPostSpacing;
      zY = (tDemData(tYEnd - 1, tXEnd) - tDemData(tYEnd, tXEnd)) / tPostSpacing;
      aspect = atan(zY / zX) * 180 / camp::kPI;
      poRasterData(nDim - 1, mDim - 1) = 180 - aspect + 90 * (zX / abs(zX));

      // Compute top and bottom edge cases
      for (int tXOut = tXStart; tXOut < tXEnd; tXOut++) {
         zX = (tDemData(0, tXOut + 1) - tDemData(0, tXOut)) / tPostSpacing;
         zY = (tDemData(0, tXOut) - tDemData(1, tXOut)) / tPostSpacing;
         aspect = atan(zY / zX) * 180 / camp::kPI;
         poRasterData(0, tXOut) = 180 - aspect + 90 * (zX / abs(zX));
         zX = (tDemData(tYEnd - 1, tXOut + 1) - tDemData(tYEnd - 1, tXOut)) / tPostSpacing;
         zY = (tDemData(tYEnd - 2, tXOut) - tDemData(tYEnd - 1, tXOut)) / tPostSpacing;
         aspect = atan(zY / zX) * 180 / camp::kPI;
         poRasterData(tYEnd, tXOut) = 180 - aspect + 90 * (zX / abs(zX));
      }

      // Compute left and right edge cases
      for (int tYOut = tYStart; tYOut < tYEnd; tYOut++) {
         zX = (tDemData(tYOut, 1) - tDemData(tYOut, 0)) / tPostSpacing;
         zY = (tDemData(tYOut, 1) - tDemData(tYOut - 1, 0)) / tPostSpacing;
         aspect = atan(zY / zX) * 180 / camp::kPI;
         poRasterData(tYOut ,0) = 180 - aspect + 90 * (zX / abs(zX));
         zX = (tDemData(tYOut, tXEnd - 1) - tDemData(tYOut, tXEnd - 2)) / tPostSpacing;
         zY = (tDemData(tYOut, tXEnd - 1) - tDemData(tYOut - 1, tXEnd - 2)) / tPostSpacing;
         aspect = atan(zY / zX) * 180 / camp::kPI;
         poRasterData(tYOut, tXEnd) = 180 - aspect + 90 * (zX / abs(zX));
      }

#pragma omp parallel for

      for (int tXOut = tXStart; tXOut < tXEnd; tXOut++) {
         int tXIn = tXOut;

         for (int tYOut = tYStart; tYOut < tYEnd; tYOut++) {
            // std::cout << "tYOut: " << tYOut << std::endl;
            // maxI tracks the Zi value corresponding to the max slope
            //      int tNextY, tNextX, maxI = 0;
            int tYIn = tYOut;

            // North
            tVal = static_cast<float>(tDemData(tYOut, tXOut));
            tN = static_cast<float>(tDemData(tYOut - 1, tXOut));
            // South
            tS = static_cast<float>(tDemData(tYOut + 1, tXOut));
            // East
            tE = static_cast<float>(tDemData(tYOut, tXOut + 1));
            // West
            tW = static_cast<float>(tDemData(tYOut, tXOut - 1));
            // Compute aspect
            zX = (tE - tW) / (2 * tH);
            zY = (tN - tS) / (2 * tH);
            float aspect = atan(zY / zX) * 180 / camp::kPI;
            tAspectVal = 180 - aspect + 90 * (zX / abs(zX));

            poRasterData(tYIn, tXIn) = tAspectVal;
         }
      }

      std::cout << "Finished computing aspect." << std::endl;

      return poRasterData;
   }

   /**
   * Classifies the topographic features found in the matrix using
   * the technique described in "The Geomorphological Characterisation
   * of Digital Elevation Models" by Jo Wood. The original paper can
   * be found at: http://www.soi.city.ac.uk/~jwo/phd/05feat.php
   *
   * @param dem_data the source matrix to be classified.
   * @param curve_tolerance the tolerance for the curvature measurements.
   * @param slope_tolerance the tolerance for the slope calculations.
   * @param cols the width of the DEM data.
   * @param rows the height of the DEM data.
   * @param post_spacing the resolution of the matrix
   * @return the Raster with the computed classifications.
   */
   template <typename ClassMatTyp, typename Derived>
   CAMP_EXPORTS int ClassifyTopography(
      Derived const& dem,
      ClassMatTyp& class_matrix,
      float const& h, int kernel_size,
      float curve_tolerance, float slope_tolerance, ProgressTracker& progress)
   {
      assert(h >= 0);
      assert(curve_tolerance >= 0);
      assert(slope_tolerance >= 0);

      int rows = dem.rows();
      int cols = dem.cols();

      Derived slope(rows, cols);
      Derived curve_ew(rows, cols);
      Derived zxx_ew(rows, cols);
      Derived zyy_ew(rows, cols);
      Derived curve_nesw(rows, cols);
      Derived zxx_nesw(rows, cols);
      Derived zyy_nesw(rows, cols);
      Derived curve_ns(rows, cols);
      Derived zxx_ns(rows, cols);
      Derived zyy_ns(rows, cols);
      Derived curve_nwse(rows, cols);
      Derived zxx_nwse(rows, cols);
      Derived zyy_nwse(rows, cols);
      Derived min_convexity(rows, cols);
      Derived max_convexity(rows, cols);
      Eigen::MatrixXi convex(rows, cols);

      //class_matrix.setConstant(rows, cols, static_cast<int>(camp::UNCLASSIFIED));

      progress.report("Compute slope", 10, NORMAL);
      slope.setConstant(rows, cols, 0);
      if (!topo::ComputeD8Slope(dem, slope, h, kernel_size ) ) {
         std::cerr << "Error computing slope" << std::endl;
         return camp::FAILURE;
      }
      //raster::writeRaster<Derived>("c:\\projects\\datafiles\\camp\\output2\\topo_slope.tif", 
      //  "c:\\projects\\datafiles\\camp\\ftmeade1.las", h, slope, GDT_Float32);

      progress.report("Compute aspect", 20, NORMAL);
      Derived aspect = topo::ComputeAspect( dem, h );
      //raster::writeRaster<Derived>("c:\\projects\\datafiles\\camp\\output2\\topo_aspect.tif", 
      //  "c:\\projects\\datafiles\\camp\\ftmeade1.las", h, aspect, GDT_Float32);

      progress.report("Compute curvature", 30, NORMAL);
      if (!topo::ComputeCurvature(dem, curve_ew, zxx_ew, zyy_ew, h, kernel_size, camp::PROFILE, camp::EW)) {
         std::cerr << "Error computing EW profile curvature" << std::endl;
         return camp::FAILURE;
      }

      progress.report("Compute curvature", 40, NORMAL);
      if (!topo::ComputeCurvature(dem, curve_nesw, zxx_nesw, zyy_nesw, h, kernel_size, camp::PROFILE, camp::NESW)) {
         std::cerr << "Error computing NESW profile curvature" << std::endl;
         return camp::FAILURE;
      }

      progress.report("Compute curvature", 50, NORMAL);
      if (!topo::ComputeCurvature(dem, curve_ns, zxx_ns, zyy_ns, h, kernel_size, camp::PROFILE, camp::NS)) {
         std::cerr << "Error computing NS profile curvature" << std::endl;
         return camp::FAILURE;
      }

      progress.report("Compute curvature", 60, NORMAL);
      if (!topo::ComputeCurvature(dem, curve_nwse, zxx_nwse, zyy_nwse, h, kernel_size, camp::PROFILE, camp::NWSE)) {
         std::cerr << "Error computing NWSE profile curvature" << std::endl;
         return camp::FAILURE;
      }
      progress.report("Compute classification", 60, NORMAL);
#pragma omp parallel for
      for (int i = 0; i < rows; i++) {
         for (int j = 0; j < cols; j++) {
            //float slopeDegrees = atan(slope(i,j)) * 180 / camp::kPI;
            if ( dem(i,j) == camp::kNoData )
            {
               class_matrix(i,j) = camp::kNoData;
            }
            else if ( abs(slope(i,j)) > 90 )
            {
               class_matrix(i,j) = camp::UNCLASSIFIED;
            }
            else
            {
               Eigen::ArrayXf::Index min_convex, max_convex;
               Eigen::ArrayXf temp(4);
               temp << curve_ew(i, j), curve_nesw(i, j), curve_ns(i, j), curve_nwse(i, j);
               int aspect_direction = 0;
               if ( aspect(i,j) >= 180 )
               {
                  aspect_direction = (( 180 - ( 360 - (int)aspect(i,j)) )/45 + 2) % 4;
               }
               else
               {
                  aspect_direction = (((int)aspect(i,j) / 45) + 2) % 4;
               }
               min_convexity(i, j) =  temp.maxCoeff(&max_convex);
               max_convexity(i, j) =  temp.minCoeff(&min_convex);
               if ( slope(i,j) > slope_tolerance )
               {
                  if ( temp[aspect_direction] < (-1*curve_tolerance) )//(aspect_direction + 2) % 4] > curve_tolerance )
                  {
                     class_matrix(i,j) = camp::RIDGE;
                  }
                  else if ( temp[aspect_direction] > curve_tolerance )//(aspect_direction + 2) % 4] < (-1*curve_tolerance) )
                  {
                     class_matrix(i,j) = camp::CHANNEL;
                  }
                  else
                  {
                     class_matrix(i,j) = camp::PLANE;
                  }
               }
               else if ( max_convexity(i,j) < (-1*curve_tolerance) )
               {
                  if ( min_convexity(i,j) < (-1* curve_tolerance) )
                  {
                     //std::cout << "max_convex: " << max_convexity(i,j) << std::endl;
                     //std::cout << "min_convex: " << min_convexity(i,j) << std::endl;
                     class_matrix(i,j) = camp::PEAK;
                  }
                  else if ( min_convexity(i,j) > curve_tolerance )
                  {
                     class_matrix(i,j) = camp::PASS;
                  }
                  else
                  {
                     class_matrix(i,j) = camp::RIDGE;
                  }
               }
               else if ( min_convexity(i,j) > curve_tolerance )
               {
                  if ( max_convexity(i,j) > curve_tolerance )
                  {
                     class_matrix(i,j) = camp::PIT;
                  }
                  else
                  {
                     class_matrix(i,j) = camp::CHANNEL;
                  }
               }
               else
               {
                  class_matrix(i,j) = camp::PLANE;
               }
               /*#pragma omp critical
               {
               if ( (i%25) == 0 && (j%25) == 0 )
               {
               std::cout << "Slope: " << slopeDegrees << std::endl;
               std::cout << "Crosc: " << temp[aspect_direction] << std::endl;
               std::cout << "Min convexity: " << min_convexity(i,j) << std::endl;
               std::cout << "Max convexity: " << max_convexity(i,j) << std::endl;
               std::cout << "Classification: " << class_matrix(i,j) << std::endl;
               }
               }*/
               /*if ( (abs(slope(i, j)) < slope_tolerance //) {
               && abs(max_convexity(i,j)) < curve_tolerance
               && abs(min_convexity(i,j)) < curve_tolerance )
               || ( slope(i,j) > slope_tolerance
               && abs(temp[(aspect_direction + 2) % 4]) < curve_tolerance ) ) {
               //std::cout << "Plane found" << std::endl;
               class_matrix(i, j) = camp::PLANE;
               }
               //else if (convex(i, j) && abs(xConvexity[max_convex]) > curve_tolerance) {
               else if ( ( slope(i,j) < slope_tolerance && max_convexity(i,j) > curve_tolerance
               && min_convexity(i,j) > curve_tolerance )) {

               class_matrix(i, j) = camp::PEAK;
               //std::cout << "Peak found" << std::endl;
               }
               else if ( abs((slope(i,j)) < slope_tolerance && max_convexity(i,j) > curve_tolerance
               && abs(min_convexity(i,j)) < curve_tolerance)
               || (slope(i,j) > slope_tolerance && temp[(aspect_direction + 2) % 4] > curve_tolerance) ){
               class_matrix(i, j) = camp::RIDGE;
               //std::cout << "Ridge found" << std::endl;
               }
               else if ( abs(slope(i,j)) < slope_tolerance && max_convexity(i,j) > curve_tolerance
               && min_convexity(i,j) < 0 && abs(min_convexity(i,j)) > curve_tolerance ) {
               class_matrix(i, j) = camp::PASS;
               //std::cout << "Pass found" << std::endl;
               } 
               else if ( (abs(slope(i,j)) < slope_tolerance && abs(max_convexity(i,j))< curve_tolerance
               && min_convexity(i,j) < 0 && abs(min_convexity(i,j)) > curve_tolerance )
               || (slope(i,j) > slope_tolerance && abs(temp[(aspect_direction + 2)%4]) > curve_tolerance
               && temp[(aspect_direction + 2)%4] < 0) ) {
               class_matrix(i, j) = camp::CHANNEL;

               }
               else if ( abs(slope(i,j)) < slope_tolerance && max_convexity(i,j) < 0
               && abs(max_convexity(i,j)) > curve_tolerance && min_convexity(i,j) < 0
               && abs(min_convexity(i,j)) > curve_tolerance) {
               class_matrix(i, j) = camp::PIT;
               //std::cout << "Pit found" << std::endl;
               }
               else  {
               class_matrix(i, j) = camp::UNCLASSIFIED;
               //std::cout << "X Convexity: " << xConvexity[max_convex] << std::endl;
               //std::cout << "Y Convexity: " << yConvexity[min_convex] << std::endl;
               }*/

               //delete[] xConvexity;
               //delete[] yConvexity;
            }
         }
      }

      return camp::SUCCESS;
   }
}  // namespace topo

#endif  // INCLUDE_TOPOGRAPHIC_ATTRIBUTES_HPP_
