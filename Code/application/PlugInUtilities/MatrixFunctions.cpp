/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "MatrixFunctions.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "Resource.h"
#include "TypesFile.h"

#include <limits>
#include <memory>
#include <string.h>
#include <ossim/matrix/newmat.h>
#include <ossim/matrix/newmatap.h>

using namespace NEWMAT;
using namespace std;

namespace
{
   /**
    *  Copies data from a source matrix into a smaller symmetric matrix.
    *
    *  @warning If \c pSource is not a symmetric matrix, then the behavior of this method is undefined.
    *
    *  @param   pDestination
    *           A pointer to the location to store the symmetric matrix.
    *           This location must be allocated and freed by the caller of this function.
    *           This parameter must be able to contain at least (numRows * (numRows + 1) / 2) doubles.
    *           This parameter cannot be the same as \c pSource.
    *           This parameter cannot be \b NULL.
    *
    *  @param   pSource
    *           A pointer to the location of the symmetric matrix to be copied.
    *           This parameter cannot be the same as \c pDestination.
    *           This parameter cannot be \b NULL.
    *
    *  @param   numRows
    *           The number of rows in \c pDestination and \c pSource.
    *           Since only square matrices can be symmetric, this value also represents the number of columns.
    *           This parameter cannot be less than or equal to 0.
    *
    *  @return True if the operation succeeded, false otherwise.
    */
   bool toSymmetricMatrix(double* pDestination, const double** pSource, const int& numRows)
   {
      if (pDestination == NULL || pSource == NULL || numRows <= 0)
      {
         return false;
      }

      double* pCurrentDestination = pDestination;
      for (int row = 0; row < numRows; ++row)
      {
         // Copy all values up to and including the diagonal in each row.
         const int numValuesToCopy = row + 1;
         memcpy(pCurrentDestination, pSource[row], numValuesToCopy * sizeof(double));
         pCurrentDestination += numValuesToCopy;
      }

      return true;
   }
}

bool MatrixFunctions::getEigenvalues(const double** pSymmetricMatrix,
   double* pEigenvalues, double** pEigenvectors, const int& numRows)
{
   if (pSymmetricMatrix == NULL || numRows <= 0 || isMatrixSymmetric(pSymmetricMatrix, numRows) == false)
   {
      return false;
   }

   // Copy pSymmetricMatrix into a SymmetricMatrix.
   SymmetricMatrix sourceMatrix(numRows);
   const int numValuesStored = sourceMatrix.Storage();
   if (numValuesStored <= 0)
   {
      return false;
   }

   auto_ptr<double> pSourceMatrixData(new double[numValuesStored]);
   if (toSymmetricMatrix(pSourceMatrixData.get(), pSymmetricMatrix, numRows) == false)
   {
      return false;
   }

   // The Store() method returns an RBD_COMMON::Real, which must be typedef'ed as a double for this code to work.
   double* pData = sourceMatrix.Store();
   VERIFY(pData != NULL);
   memcpy(pData, pSourceMatrixData.get(), numValuesStored * sizeof(pData[0]));

   // Compute the eigenvalues and eigenvectors.
   Matrix eigenvectors;
   DiagonalMatrix eigenvalues;
   EigenValues(sourceMatrix, eigenvalues, eigenvectors, false);

   // Copy eigenvalues to pEigenvalues.
   if (pEigenvalues != NULL)
   {
      pData = eigenvalues.Store();
      VERIFY(pData != NULL);
      memcpy(pEigenvalues, pData, numRows * sizeof(pData[0]));
   }

   // Copy eigenvectors to pEigenvectors.
   if (pEigenvectors != NULL)
   {
      pData = eigenvectors.Store();
      VERIFY(pData != NULL);
      for (int row = 0; row < numRows; ++row)
      {
         memcpy(pEigenvectors[row], pData + row * numRows, numRows * sizeof(pData[0]));
      }
   }

   return true;
}

bool MatrixFunctions::invertSquareMatrix1D(double* pDestination, const double* pSource, const int& numRows)
{
   if (pDestination == NULL || pSource == NULL || numRows <= 0)
   {
      return false;
   }

   // Copy pSource into a SquareMatrix.
   // The Store() method returns an RBD_COMMON::Real, which must be typedef'ed as a double for this code to work.
   SquareMatrix sourceMatrix(numRows);
   double* pData = sourceMatrix.Store();
   VERIFY(pData != NULL);
   const size_t matrixSizeBytes = numRows * numRows * sizeof(pData[0]);
   memcpy(pData, pSource, matrixSizeBytes);

   // Invert sourceMatrix and copy the calculated results into pDestination.
   try
   {
      // Throws an exception when trying to invert a non-invertible matrix.
      InvertedMatrix invertedMatrix = sourceMatrix.i();
      GeneralMatrix* pResultsMatrix = invertedMatrix.Evaluate();
      VERIFY(pResultsMatrix != NULL);
      pData = pResultsMatrix->Store();
      VERIFY(pData != NULL);
      memcpy(pDestination, pData, matrixSizeBytes);
   }
   catch (const RBD_COMMON::BaseException&)
   {
      return false;
   }

   return true;
}

bool MatrixFunctions::invertSquareMatrix2D(double** pDestination, const double** pSource, const int& numRows)
{
   if (pDestination == NULL || pSource == NULL || numRows <= 0)
   {
      return false;
   }

   // Copy pSource into a SquareMatrix.
   // The Store() method returns an RBD_COMMON::Real, which must be typedef'ed as a double for this code to work.
   SquareMatrix sourceMatrix(numRows);
   double* pData = sourceMatrix.Store();
   VERIFY(pData != NULL);
   for (int row = 0; row < numRows; ++row)
   {
      memcpy(pData + row * numRows, pSource[row], numRows * sizeof(pData[0]));
   }

   // Invert sourceMatrix and copy the calculated results into pDestination.
   try
   {
      // Throws an exception when trying to invert a non-invertible matrix.
      InvertedMatrix invertedMatrix = sourceMatrix.i();
      GeneralMatrix* pResultsMatrix = invertedMatrix.Evaluate();
      VERIFY(pResultsMatrix != NULL);
      pData = pResultsMatrix->Store();
      VERIFY(pData != NULL);
      for (int row = 0; row < numRows; ++row)
      {
         memcpy(pDestination[row], pData + row * numRows, numRows * sizeof(pData[0]));
      }
   }
   catch (const RBD_COMMON::BaseException&)
   {
      return false;
   }

   return true;
}

bool MatrixFunctions::invertRasterElement(RasterElement* pDestination, const RasterElement* pSource)
{
   if (pDestination == NULL || pSource == NULL)
   {
      return false;
   }

   const RasterDataDescriptor* pSourceDd =
      dynamic_cast<const RasterDataDescriptor*>(pSource->getDataDescriptor());
   const RasterDataDescriptor* pDestinationDd =
      dynamic_cast<const RasterDataDescriptor*>(pDestination->getDataDescriptor());
   if (pSourceDd == NULL || pDestinationDd == NULL)
   {
      return false;
   }

   // Only specific datasets are supported.
   const unsigned int numBands = pSourceDd->getBandCount();
   const unsigned int numRows = pSourceDd->getRowCount();
   const unsigned int numCols = pSourceDd->getColumnCount();
   const EncodingType dataType = pSourceDd->getDataType();
   if (numRows == 0 || numRows > static_cast<unsigned int>(numeric_limits<int>::max()) ||
      numRows != numCols || numBands != 1 || dataType != FLT8BYTES)
   {
      return false;
   }

   if (pDestinationDd->getBandCount() != numBands || pDestinationDd->getRowCount() != numRows ||
      pDestinationDd->getColumnCount() != numCols || pDestinationDd->getDataType() != dataType)
   {
      return false;
   }

   // All inputs are valid. Get pointers to the raw data and invert it.
   double* pDestinationData = reinterpret_cast<double*>(pDestination->getRawData());
   const double* pSourceData = reinterpret_cast<const double*>(pSource->getRawData());
   if (pDestinationData == NULL || pSourceData == NULL)
   {
      return false;
   }

   return invertSquareMatrix1D(pDestinationData, pSourceData, static_cast<int>(numRows));
}

bool MatrixFunctions::computeSingularValueDecomposition(const double** pMatrix, double* pSingularValues,
   double** pColumnMatrix, double** pOrthogonalMatrix, const int& numRows, const int& numCols)
{
   if (pMatrix == NULL || numRows < numCols || numCols <= 0)
   {
      return false;
   }

   // Copy pMatrix into a Matrix.
   // The Store() method returns an RBD_COMMON::Real, which must be typedef'ed as a double for this code to work.
   Matrix inputMatrix(numRows, numCols);
   double* pData = inputMatrix.Store();
   VERIFY(pData != NULL);
   for (int row = 0; row < numRows; ++row)
   {
      memcpy(pData + row * numCols, pMatrix[row], numCols * sizeof(pData[0]));
   }

   // Perform the SVD.
   Matrix columnMatrix;
   Matrix orthogonalMatrix;
   const bool sortResults = false;
   DiagonalMatrix outputMatrix(numCols);
   const bool computeColumnMatrix = (pColumnMatrix != NULL);
   const bool computeOrthogonalMatrix = (pOrthogonalMatrix != NULL);

   try
   {
      // Solve the equation. Throws an exception on invalid input or when unable to converge.
      SVD(inputMatrix, outputMatrix, columnMatrix, orthogonalMatrix,
         computeColumnMatrix, computeOrthogonalMatrix, sortResults);
   }
   catch (const RBD_COMMON::BaseException&)
   {
      return false;
   }

   // Copy outputMatrix into pSingularValues.
   if (pSingularValues != NULL)
   {
      // outputMatrix is an array of size numCols , so use numCols instead of numRows.
      pData = outputMatrix.Store();
      VERIFY(pData != NULL);
      memcpy(pSingularValues, pData, numCols * sizeof(pData[0]));
   }

   // Copy columnMatrix into pColumnMatrix.
   if (pColumnMatrix != NULL)
   {
      pData = columnMatrix.Store();
      VERIFY(pData != NULL);
      for (int row = 0; row < numRows; ++row)
      {
         memcpy(pColumnMatrix[row], pData + row * numCols, numCols * sizeof(pData[0]));
      }
   }

   // Copy orthogonalMatrix into pOrthogonalMatrix.
   if (pOrthogonalMatrix != NULL)
   {
      // orthogonalMatrix is numCols x numCols, so use numCols instead of numRows.
      pData = orthogonalMatrix.Store();
      VERIFY(pData != NULL);
      for (int row = 0; row < numCols; ++row)
      {
         memcpy(pOrthogonalMatrix[row], pData + row * numCols, numCols * sizeof(pData[0]));
      }
   }

   return true;
}

bool MatrixFunctions::solveLinearEquation(double* pResult, const double** pLhs, const double* pRhs,
   const int& numRows, const int& numColsLhs)
{
   if (pResult == NULL || pLhs == NULL || pRhs == NULL || numRows < numColsLhs || numColsLhs <= 0)
   {
      return false;
   }

   // Copy pLhs into a Matrix.
   // The Store() method returns an RBD_COMMON::Real, which must be typedef'ed as a double for this code to work.
   Matrix lhsMatrix(numRows, numColsLhs);
   double* pData = lhsMatrix.Store();
   VERIFY(pData != NULL);
   for (int row = 0; row < numRows; ++row)
   {
      memcpy(pData + row * numColsLhs, pLhs[row], numColsLhs * sizeof(pData[0]));
   }

   // Copy pRhs into a ColumnVector.
   ColumnVector rhsVector(numRows);
   pData = rhsVector.Store();
   VERIFY(pData != NULL);
   memcpy(pData, pRhs, rhsVector.Storage() * sizeof(pData[0]));

   // Create a ColumnVector to hold the results.
   ColumnVector resultVector(numColsLhs);

   try
   {
      // Solve the equation. Throws an exception on invalid input or when unable to converge.
      SVDSolve(resultVector, lhsMatrix, rhsVector);
   }
   catch (const RBD_COMMON::BaseException&)
   {
      return false;
   }

   pData = resultVector.Store();
   VERIFY(pData != NULL);
   memcpy(pResult, pData, resultVector.Storage() * sizeof(pData[0]));
   return true;
}
