/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MATRIXFUNCTIONS_H
#define MATRIXFUNCTIONS_H

#include "Resource.h"

#include <exception>
#include <math.h>
#include <memory>
#include <string.h>

class RasterElement;

/**
 * A collection of matrix functions.
 */
namespace MatrixFunctions
{
   //forward declarations only, see below for actual function definition
   template<typename T> T** createMatrix(const int& numRows, const int& numCols, const T* pInitialData = NULL);
   template<typename T> bool deleteMatrix(T**& pMatrix);

   /**
    * The %MatrixObject is a trait object for use with the %Resource template. 
    *
    * The %MatrixObject is a trait object for use with the %Resource template.
    * It provides capability for creating and destroying two-dimensional matrices.
    * 
    * @see MatrixResource
    */
   template<typename T>
   class MatrixObject
   {
   public:
      /**
      * This is an implementation detail of the %MatrixObject class. 
      *
      * This is an implementation detail of the MatrixObject class. It is used 
      * for passing the parameters required by createMatrix.
      */
      class Args
      {
      public:
         const int mNumRows;
         const int mNumCols;
         const T* const mpInitialData;
         Args(const int& numRows, const int& numCols, const T* pInitialData) :
            mNumRows(numRows), mNumCols(numCols), mpInitialData(pInitialData) {}
      };

      /**
       * Obtains an matrix using the createMatrix function.
       *
       * @param  args
       *         The arguments for obtaining the resource. Should be of type MatrixObject::Args.
       * @return Returns a pointer to the created Matrix.
       */
      T* obtainResource(const Args& args) const 
      {
         return reinterpret_cast<T*>(createMatrix<T>(args.mNumRows, args.mNumCols, args.mpInitialData));
      }

      /**
       * Releases a matrix using the deleteMatrix function.
       *
       * @param  args
       *         The arguments for releasing the resource. Should be of type MatrixObject::Args.
       * @param  pMatrix
       *         A pointer to the matrix to be freed.
       */
      void releaseResource(const Args& args, T* pMatrix) const
      {
         deleteMatrix<T>(reinterpret_cast<T**&>(pMatrix));
      }
   };

   /**
    *  This is a %Resource class that creates and destroys two-dimensional matrices.
    *
    *  This is a %Resource class that creates and destroys two-dimensional matrices. It has a conversion
    *  operator to allow a %MatrixResource object to be used whereever a double** would normally be used.
   */
   template<typename T>
   class MatrixResource : public Resource<T, MatrixObject<T> >
   {
   public:
      /**
       *  Constructs a Resource object that wraps a double**.
       *
       *  Constructs a Resource object that wraps a double**.
       *  Creates a matrix of the specified size with (optional) initial data.
       *
       *  @param   numRows
       *           The number of rows to allocate.
       *           This parameter cannot be less than or equal to 0.
       *
       *  @param   numCols
       *           The number of columns to allocate.
       *           This parameter cannot be less than or equal to 0.
       *
       *  @param   pInitialData
       *           The initial values to copy into the matrix.
       *           There should be \c numRows \c * \c numCols elements available in this buffer.
       *           If this parameter is \b NULL, the returned matrix will be set to 0 before returning.
       */
      MatrixResource(const int& numRows, const int& numCols, const T* pInitialData = NULL) :
         Resource<T, MatrixObject<T> >(typename MatrixObject<T>::Args(numRows, numCols, pInitialData)) {}

      /**
       *  Returns a pointer to the underlying T**.
       *
       *  Returns a pointer to the underlying T**. This operator
       *  allows the %MatrixResource object to be used whereever
       *  a T** would normally be used.
       *
       *  @return   A pointer to the underlying T**.
       */
      operator T**()
      {
         return reinterpret_cast<T**>(Resource<T,MatrixObject<T> >::get());
      }

      /**
       *  Returns a const pointer to the underlying T**.
       *
       *  Returns a const pointer to the underlying T**. This operator
       *  allows the %MatrixResource object to be used whereever
       *  a const T** would normally be used.
       *
       *  @return   A const pointer to the underlying T**.
       */
      operator const T**() const
      {
         return reinterpret_cast<const T**>(const_cast<T*>(Resource<T,MatrixObject<T> >::get()));
      }

      /**
       *  Returns a pointer to a row.
       *
       *  @param index
       *           The zero-based row to obtain.
       *
       *  @warning This overload is only valid for \c int variables.
       *
       *  @return   A pointer to the requested row.
       */
       T*& operator[] (const int& index)
      {
         T** pMatrix = reinterpret_cast<T**>(Resource<T,MatrixObject<T> >::get());
         return pMatrix[index];
      }

      /**
       *  Returns a const pointer to a row.
       *
       *  @param index
       *           The zero-based row to obtain.
       *
       *  @warning This overload is only valid for \c int variables.
       *
       *  @return   A const pointer to the requested row.
       */
      const T*& operator[] (const int& index) const
      {
         T** pMatrix = reinterpret_cast<T**>(Resource<T,MatrixObject<T> >::get());
         return pMatrix[index];
      }
   };

   /**
    *  Allocate memory for a two-dimensional matrix.
    *
    *  @param   numRows
    *           The number of rows to allocate.
    *           This parameter cannot be less than or equal to 0.
    *
    *  @param   numCols
    *           The number of columns to allocate.
    *           This parameter cannot be less than or equal to 0.
    *
    *  @param   pInitialData
    *           The initial values to copy into the matrix.
    *           There should be \c numRows \c * \c numCols elements available in this buffer.
    *           If this parameter is \b NULL, the returned matrix will be set to 0 before returning.
    *
    *  @return A pointer to the matrix if the operation succeeded, NULL otherwise.
    *
    *  @see deleteMatrix()
    */
   template<typename T>
   T** createMatrix(const int& numRows, const int& numCols, const T* pInitialData)
   {
      if (numRows <= 0 || numCols <= 0)
      {
         return NULL;
      }

      T** pMatrix = NULL;
      try
      {
         pMatrix = new T*[numRows];
         pMatrix[0] = new T[numRows * numCols];
      }
      catch (const std::bad_alloc&)
      {
         delete [] pMatrix[0];
         delete [] pMatrix;
         return NULL;
      }

      if (pInitialData == NULL)
      {
         memset(pMatrix[0], 0, numRows * numCols * sizeof(T));
      }
      else
      {
         memcpy(pMatrix[0], pInitialData, numRows * numCols * sizeof(T));
      }

      for (int row = 1; row < numRows; ++row)
      {
         pMatrix[row] = pMatrix[row - 1] + numCols;
      }

      return pMatrix;
   }

   /**
    *  Free memory for a two-dimensional matrix.
    *
    *  @param   pMatrix
    *           The value returned by createMatrix().
    *           On successful return, this will be set to \b NULL.
    *
    *  @return True if the operation succeeded, false otherwise.
    *
    *  @see createMatrix()
    */
   template<typename T>
   bool deleteMatrix(T**& pMatrix)
   {
      if (pMatrix == NULL)
      {
         return false;
      }

      delete [] pMatrix[0];
      delete [] pMatrix;
      pMatrix = NULL;
      return true;
   }

   /**
    *  Calculates the singular value decomposition (SVD) for the given matrix.
    *
    *  @param   pMatrix
    *           A pointer to the location of the matrix to use for computation.
    *           This location must be allocated and freed by the caller of this function.
    *           This parameter cannot be \b NULL.
    *
    *  @param   pSingularValues
    *           A pointer to the location to store the singular values.
    *           This location must be allocated and freed by the caller of this function.
    *           If this parameter is \b NULL, singular values will not be returned.
    *
    *  @param   pColumnMatrix
    *           A pointer to the location to store the numRows x numCols column matrix of the decomposition.
    *           This location must be allocated and freed by the caller of this function.
    *           If this parameter is \b NULL, the column matrix will not be returned.
    *
    *  @param   pOrthogonalMatrix
    *           A pointer to the location to store the numCols x numCols orthogonal matrix of the decomposition.
    *           This location must be allocated and freed by the caller of this function.
    *           If this parameter is \b NULL, the orthogonal matrix will not be returned.
    *
    *  @param   numRows
    *           The number of rows in \c pMatrix.
    *           This parameter cannot be less than \c numCols.
    *
    *  @param   numCols
    *           The number of columns in \c pMatrix.
    *           This parameter cannot be less than or equal to 0.
    *
    *  @return True if the operation succeeded, false otherwise.
    *
    *  @see isMatrixSymmetric()
    */
   bool computeSingularValueDecomposition(const double** pMatrix, double* pSingularValues,
      double** pColumnMatrix, double** pOrthogonalMatrix, const int& numRows, const int& numCols);

   /**
    *  Solves a linear equation of the form Ax = b, where A is represented by \c pLhs,
    *  b is represented by \c pRhs, and x is represented by \c pResult.
    *
    *  @param   pResult
    *           A pointer to the location of the vector to use for results.
    *           This vector is assumed to be 1 column wide and contain \c numRows entries.
    *           This location must be allocated and freed by the caller of this function.
    *           This parameter cannot be \b NULL.
    *
    *  @param   pLhs
    *           A pointer to the location of the matrix to use for solving the equation.
    *           This location must be allocated and freed by the caller of this function.
    *           This parameter cannot be \b NULL.
    *
    *  @param   pRhs
    *           A pointer to the location of the vector to use for solving the equation.
    *           This vector is assumed to be 1 column wide and contain \c numRows entries.
    *           This location must be allocated and freed by the caller of this function.
    *           This parameter cannot be \b NULL.
    *
    *  @param   numRows
    *           The number of rows in \c pLhs, \c pRhs and \c pResult.
    *           This parameter cannot be less than \c numColsLhs.
    *
    *  @param   numColsLhs
    *           The number of columns in \c pLhs.
    *           This parameter cannot be less than or equal to 0.
    *
    *  @return True if the operation succeeded, false otherwise.
    */
   bool solveLinearEquation(double* pResult, const double** pLhs, const double* pRhs,
      const int& numRows, const int& numColsLhs);

   /**
    *  Calculates eigenvalues and eigenvectors for the given symmetric matrix.
    *
    *  @param   pSymmetricMatrix
    *           A pointer to the location of the symmetric matrix to use for computation.
    *           This location must be allocated and freed by the caller of this function.
    *           This parameter cannot be \b NULL.
    *
    *  @param   pEigenvalues
    *           A pointer to the location to store the sorted eigenvalues.
    *           This location must be allocated and freed by the caller of this function.
    *           If this parameter is \b NULL, eigenvalues will not be returned.
    *
    *  @param   pEigenvectors
    *           A pointer to the location to store the sorted eigenvectors.
    *           This location must be allocated and freed by the caller of this function.
    *           If this parameter is \b NULL, eigenvectors will not be returned.
    *
    *  @param   numRows
    *           The number of rows in \c pSymmetricMatrix.
    *           Since only square matrices have eigenvalues, this value also represents the number of columns.
    *           This parameter cannot be less than or equal to 0.
    *
    *  @return True if the operation succeeded, false otherwise.
    *
    *  @see isMatrixSymmetric()
    */
   bool getEigenvalues(const double** pSymmetricMatrix, double* pEigenvalues, double** pEigenvectors,
      const int& numRows);

   /**
    *  Inverts a square matrix.
    *
    *  @param   pDestination
    *           A pointer to the location to store the inverted matrix.
    *           This location must be allocated and freed by the caller of this function.
    *           This parameter can be the same as \c pSource.
    *           This parameter cannot be \b NULL.
    *
    *  @param   pSource
    *           A pointer to the location of the matrix to be inverted.
    *           This location must be allocated and freed by the caller of this function.
    *           This parameter can be the same as \c pDestination.
    *           This parameter cannot be \b NULL.
    *
    *  @param   numRows
    *           The number of rows in \c pDestination and \c pSource.
    *           Since only square matrices are invertible, this value also represents the number of columns.
    *           This parameter cannot be less than or equal to 0.
    *
    *  @return True if the operation succeeded, false otherwise.
    */
   bool invertSquareMatrix1D(double* pDestination, const double* pSource, const int& numRows);

   /**
    *  This method is similar to invertSquareMatrix1D, except \c pSource and \c pDestination refer
    *  to two-dimensional pointers, such as those returned by createMatrix().
    *
    *  @copydoc invertSquareMatrix1D()
    *
    *  @see createMatrix()
    */
   bool invertSquareMatrix2D(double** pDestination, const double** pSource, const int& numRows);

   /**
    *  Inverts a RasterElement representing a square matrix.
    *
    *  @param   pDestination
    *           A pointer to the RasterElement to store the inverted matrix.
    *           This parameter must be the same size (rows, columns, and bands) and EncodingType as \c pSource.
    *           This parameter can be the same as \c pSource.
    *           This parameter cannot be \b NULL.
    *
    *  @param   pSource
    *           A pointer to the RasterElement containing the data to be inverted.
    *           The number of rows must not be greater than \c numeric_limits<int>::max().
    *           This parameter must contain the same number of rows and columns.
    *           This parameter must contain single band data.
    *           This parameter must have an EncodingType of FLT8BYTES.
    *           This parameter can be the same as \c pDestination.
    *           This parameter cannot be \b NULL.
    *
    *  @return True if the operation succeeded, false otherwise.
    */
   bool invertRasterElement(RasterElement* pDestination, const RasterElement* pSource);

   /**
    *  Compares two matrices for equality.
    *
    *  @param   pLhsMatrix
    *           A pointer to the location of the left-hand side matrix to be compared.
    *           This location must be allocated and freed by the caller of this function.
    *           This parameter cannot be \b NULL.
    *
    *  @param   pRhsMatrix
    *           A pointer to the location of the right-hand side matrix to be compared.
    *           This location must be allocated and freed by the caller of this function.
    *           This parameter cannot be \b NULL.
    *
    *  @param   numRows
    *           The number of rows in \c pLhsMatrix and \c pRhsMatrix.
    *           This parameter cannot be less than or equal to 0.
    *
    *  @param   numCols
    *           The number of columns in \c pLhsMatrix and \c pRhsMatrix.
    *           This parameter cannot be less than or equal to 0.
    *
    *  @param   tolerance
    *           The maximum allowable difference between any single element of pLhsMatrix and pRhsMatrix.
    *           This parameter cannot be less than 0.
    *
    *  @return True if the operation succeeded, false otherwise.
    */
   template<typename T>
   bool areMatricesEqual(const T** pLhsMatrix, const T** pRhsMatrix,
      const int& numRows, const int& numCols, const double& tolerance = 1e-6)
   {
      if (pLhsMatrix == NULL || pRhsMatrix == NULL || numRows <= 0 || numCols <= 0 || tolerance < 0.0)
      {
         return false;
      }

      if (pLhsMatrix == pRhsMatrix)
      {
         return true;
      }

      if (tolerance == 0.0)
      {
         if (memcmp(pLhsMatrix, pRhsMatrix, numRows * numCols * sizeof(T)) != 0)
         {
            return false;
         }
      }
      else
      {
         for (int row = 0; row < numRows; ++row)
         {
            for (int col = 0; col < numCols; ++col)
            {
               if (fabs(static_cast<double>(pLhsMatrix[row][col] - pRhsMatrix[row][col])) > tolerance)
               {
                  return false;
               }
            }
         }
      }

      return true;
   }

   /**
    *  Determines whether a given matrix is symmetric.
    *
    *  @param   pMatrix
    *           A pointer to the location of the matrix to be checked.
    *           This location must be allocated and freed by the caller of this function.
    *           This parameter cannot be \b NULL.
    *
    *  @param   numRows
    *           The number of rows in \c pMatrix.
    *           Since only square matrices can be symmetric, this value also represents the number of columns.
    *           This parameter cannot be less than or equal to 0.
    *
    *  @param   tolerance
    *           The maximum allowable difference between pMatrix[row][col] and pMatrix[col][row].
    *           This parameter cannot be less than 0.
    *
    *  @return True if the operation succeeded, false otherwise.
    */
   template<typename T>
   bool isMatrixSymmetric(const T** pMatrix, const int& numRows, const double& tolerance = 1e-3)
   {
      if (pMatrix == NULL || numRows <=0 || tolerance < 0.0)
      {
         return false;
      }

      const int numCols = numRows;
      for (int row = 0; row < numRows; ++row)
      {
         for (int col = row + 1; col < numCols; ++col)
         {
            if (fabs(static_cast<double>(pMatrix[row][col] - pMatrix[col][row])) > tolerance)
            {
               return false;
            }
         }
      }

      return true;
   }
}

#endif
