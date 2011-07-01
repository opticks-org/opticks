/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "GeoreferenceUtilities.h"
#include "LocationType.h"
#include "MatrixFunctions.h"

namespace GeoreferenceUtilities
{
void basisFunction(const LocationType& pixelCoord, double* pBasisValues, int numBasisValues)
{
   // numBasisValues = (order+1)*(order+2)/2, solve for order
   // add fudge factor to prevent x.9999999 being truncated to x
   int order = static_cast<int>((-3.0 + sqrt(9.0 + 8.0 *(numBasisValues-1)))/2.0 + 0.5);
   double yValue;

   int count = 0;
   for (int i = 0; i <= order; ++i)          // y power
   {
      yValue = pow(pixelCoord.mY, i);
      for (int j = 0; j <= order - i; ++j)   // x power
      {
         pBasisValues[count] = pow (pixelCoord.mX, j) * yValue;
         count++;
      }
   }
}

bool computeFit(const std::vector<LocationType>& points,
                const std::vector<LocationType>& values,
                int which,
                std::vector<double>& coefficients)
{
   // The number of GCP points represents the number of rows in the matrix to be solved.
   // The number of coefficients required for the calculation represents the number of columns in the matrix.
   const int numRows = values.size();
   const int numCols = coefficients.size();

   // Create and populate the matrix to be solved.
   MatrixFunctions::MatrixResource<double> pMatrix(numRows, numCols);
   VERIFY(pMatrix.get() != NULL);

   for (int row = 0; row < numRows; ++row)
   {
      basisFunction(points[row], pMatrix[row], numCols);
   }

   // Create a vector of latlon points corresponding to which (X or Y) set of values is to be used.
   // This vector represents the right-hand side of the equation to use when solving the equation.
   std::vector<double> latlon(numRows);
   for (int point = 0; point < numRows; ++point)
   {
      if (which == 0)
      {
         latlon[point] = values[point].mX;
      }
      else
      {
         latlon[point] = values[point].mY;
      }
   }

   // Solve the equation.
   return MatrixFunctions::solveLinearEquation(&coefficients.front(), pMatrix, &latlon.front(), numRows, numCols);
}

double computePolynomial(LocationType pixel, int order, std::vector<double>& coeffs)
{
   double yValue;

   int count = 0;
   double value = 0.0;
   for (int i = 0; i <= order; ++i)          // y power
   {
      yValue = pow(pixel.mY, i);
      for (int j = 0; j <= order - i; ++j)   // x power
      {
         value += coeffs[count] * pow (pixel.mX, j) * yValue;
         count++;
      }
   }

   return value;
}

LocationType evaluatePolynomial(LocationType position,
                                const std::vector<double>& pXCoeffs,
                                const std::vector<double>& pYCoeffs,
                                int order)
{
   double yValue;
   double xyValue;
   LocationType transformedPosition(0.0, 0.0);

   try
   {
      int count = 0;
      for (int i = 0; i <= order; ++i)          // y power
      {
         yValue = pow(position.mY, i);
         for (int j = 0; j <= order - i; ++j)   // x power
         {
            xyValue = pow(position.mX, j) * yValue;
            transformedPosition.mX += pXCoeffs[count] * xyValue;
            transformedPosition.mY += pYCoeffs[count] * xyValue;
            count++;
         }
      }
   }
   catch (const std::out_of_range&)
   {
      return LocationType(0.0,0.0);
   }

   return transformedPosition;
}

}
