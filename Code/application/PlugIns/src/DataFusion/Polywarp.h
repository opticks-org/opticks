/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLYWARP_H
#define POLYWARP_H

/**
 * Polywarp
 *
 * This function creates the polynomial warp matrices used
 * to map the AOI in the primary image to the corresponding
 * area in the secondary image. These matrices are passed
 * into Poly2D to tranform the area in the original secondary
 * image to align with the AOI selected in the primary image.
 */

#include "DataFusionTools.h"

#include <iostream>
#include <vector>
#include <math.h>
#include "Matrix.h"
#include "Vector.h"
using namespace std;

const bool FILTER = false;

/**
 * This function creates the polynomial warp matrices used
 * to map the AOI in the primary image to the corresponding
 * area in the secondary image. These matrices are passed
 * into Poly2D to tranform the area in the original secondary
 * image to align with the AOI selected in the primary image.
 *
 * @param  XS
 *         The x components of the corner points of the ROI ij the primary image
 * @param  YS
 *         The y components of the corner points of the ROI ij the primary image
 * @param  XP
 *         The x components (found using RPC or other georeferencing data) of the
 *         corner point in the secondary image.
 * @param  YP
 *         The y components (found using RPC or other georeferencing data) of the
 *         corner point in the secondary image. * @param  
 * @param  KX
 *         An output of Polywarp. The X component of the warp vectors.
 * @param  KY
 *         An output of Polywarp. The Y component of the warp vectors.
 * @param  zoomFactor
 *         Always >= 1. An integer representing how much more resolution the secondary
 *         image has than the primary image. If both images are of comparable geo-resolution
 *         zoomFactor == 1.
 */
template<class T>
void polywarp( const Vector<T>& XS, const Vector<T>& YS, const Vector<T>& XP,
               const Vector<T>& YP, Vector<T> &KX, Vector<T> &KY,
               int zoomFactor,
               ProgressTracker &progressTracker)
{
   //--Out of 500,000 runs, the average approximate run time is 1.8 ms

   int i, j, k; //loop iterators
   int NumFitPoints = (int)XS.size(); //number of points used in the fit					
   int degree = 1; //degree of polynomial fit
   int ReqNumFitPoints = static_cast<int>(pow( static_cast<double>(degree + 1), 2 ));
   int GarbageFilterOM = 5; //order of magnitude (+ or -) considered garbage

   if (NumFitPoints == 0)
   {
      throw FusionException("Must supply more than 0 fit points!", __LINE__, __FILE__);
   }
   
   // -- EXCEPTIONS

   if ((NumFitPoints != (int)YS.size()) || (XP.size() != YP.size()) || (NumFitPoints != (int)XP.size()))
   {
      throw FusionException(
         std::string("Input Vectors must be the same size and the number of fit points must match!" ),
                     __LINE__,
                     __FILE__ );
   }

   //make sure there is at least one possible solution
   double MagXS, MagXP, MagYS, MagYP; //magnitudes of the input vectors
   MagXS = XS.magnitude();
   MagXP = XP.magnitude();
   MagYS = YS.magnitude();
   MagYP = YP.magnitude();

   if ((MagXP == 0.0 || MagYP == 0.0) && (MagXS != 0.0))
   {
      throw FusionException(std::string( "No possible solutions for KX" ), __LINE__, __FILE__ );
   }

   if ((MagXP == 0.0 || MagYP == 0.0) && (MagYS != 0.0))
   {
      throw FusionException(std::string( "No possible solutions for KY" ), __LINE__, __FILE__ );
   }

   //make sure there are not infinite solutions
   if (MagXS == 0.0 && (MagXP == 0.0 || MagYP == 0.0))
   {
      throw FusionException(std::string( "Infinite solutions for KX" ), __LINE__, __FILE__ );
   }

   if (MagYS == 0.0 && (MagXP == 0.0 || MagYP == 0.0))
   {
      throw FusionException(std::string( "Infinite solutions for KY" ), __LINE__, __FILE__ );
   }

   if (ReqNumFitPoints > NumFitPoints)
   {
      throw FusionException(std::string( "Number of points must be greater than (degree+1)^2" ), __LINE__, __FILE__ );
   }

   // -- START MATH
   //----- transpose of U
   Matrix<T> ut(NumFitPoints, ReqNumFitPoints);

   //----- [1,u2i,u2i^2,...]
   Vector<T> u2i(degree + 1);
   T yComponent; // y-component of current primary image point
   int column;

   for (i = 0; i < NumFitPoints; i++)
   {
      //----- init u2i
      u2i[0] = (T)1;
      yComponent = YP[i];
      for (j = 1; j <= degree; j++)
      {
         u2i[j] = u2i[j-1] * yComponent;
      }

      //----- evaluate 0 th power separately
      for (j = 0; j < (int)u2i.size(); j++)
      {
         ut[i][j] = u2i[j];
      }

      //----- fill ut=u0i^j * U2i
      for (j = 1; j <= degree; j++)
      {
         u2i *= (T)pow( XP[i], j );
         for (k = 0; k < (int)u2i.size(); k++)
         {
            column = ( j * (degree + 1) ) + k;
            ut[i][column] = u2i[k];
         }
      }
      int progress = int((100/NumFitPoints)*(i+1));
      progressTracker.report("Computing polynomial warp matrices", progress, NORMAL);
   }

   //----- big u
   Matrix<T> uu = ~ut * ut; 
   Matrix<T> kk = uu.invert(); 

   //----- solve equation
   kk = ut * kk;

   Matrix<T> XSMatrix( 1, XS.size() );
   Matrix<T> YSMatrix( 1, YS.size() );

   //read input vectors into matrix for multiplication
   for (i = 0; i < (int)XS.size(); i++)
   {
      XSMatrix[0][i] = XS[i];
      YSMatrix[0][i] = YS[i];
   }

   XSMatrix = XSMatrix * kk;
   YSMatrix = YSMatrix * kk;

   Vector<T> returnKX( static_cast<size_t>(pow( static_cast<double>(degree + 1), 2)) );
   Vector<T> returnKY( static_cast<size_t>(pow( static_cast<double>(degree + 1), 2)) );  

   //if you just set this to KX and KY, you get run-time errors
   for (i = 0; i < XSMatrix.getNumColumns(); i++)
   {
      returnKX[i] = XSMatrix[0][i];
      returnKY[i] = YSMatrix[0][i];
   }
   
   //filter out very small "garbage" values
   if (FILTER)
   {
      for (i = 0; i < XSMatrix.getNumColumns(); i++)
         {
            if ( abs(log10(abs(returnKX[i]))) >= (GarbageFilterOM - 1))
               returnKX[i]=(T)0;
            if ( abs(log10(abs(returnKY[i]))) >= (GarbageFilterOM - 1))
               returnKY[i]=(T)0;
         }
   }
   
   const int DIM = 2;
   unsigned int ui;
   for (ui = 0; ui < returnKX.size(); ui++)
   {
      returnKX[ui] = (returnKX[ui] / pow(static_cast<double>(zoomFactor), static_cast<int>(ui/DIM))) / 
                      pow(static_cast<double>(zoomFactor), static_cast<int>(ui%DIM));
   }
   for (ui = 0; ui < returnKY.size(); ui++)
   {
      returnKY[ui] = (returnKY[ui] / pow(static_cast<double>(zoomFactor), static_cast<int>(ui/DIM))) /
                      pow(static_cast<double>(zoomFactor), static_cast<int>(ui%DIM));
   }


   KX = returnKX;
   KY = returnKY;
}

#endif
