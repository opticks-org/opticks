/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MATRIX_H
#define MATRIX_H

#define APPROXIMATELY_ZERO 1.0e-19

#include "Vector.h"
#include "FusionException.h"
#include "DataFusionTools.h"

#include <limits>

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Many methods in this class are duplicated " \
   "by MatrixFunctions.h (dadkins)")

template<class T>
class Matrix
{
   public:
      Matrix() {}
      Matrix(size_t rows, size_t columns);
      Matrix(const Matrix& srcMatrix);
      ~Matrix() {}

      inline const Vector<T>& operator[](const size_t row) const;
      inline Vector<T>& operator[](const size_t row);

      inline int getNumRows() const
      {
         return static_cast<int>(mMatrix.size());
      }

      inline int getNumColumns() const;

      void setToIdentity();
      void setToZero();

      void print() const;

      inline int count()
      {
         return (getNumRows() * getNumColumns());
      }

      T determinant() const;
      Matrix<T> invert() const;
      inline Matrix<T>& operator=(const Matrix<T>& rhs);

      inline Matrix<T> operator+(const Matrix<T>& rhs) const;
      inline Matrix<T> operator-(const Matrix<T>& rhs) const;
      inline Matrix<T> operator*(const Matrix<T>& rhs) const;
      inline Matrix<T> operator*(const double scalar) const;

      /**
       * Computes the transpose of the matrix.
       *
       * @return  A copy of the transposed matrix
       */
      inline Matrix<T> operator~() const;

      inline bool operator==(const Matrix<T>& rhs) const;
      inline bool operator!=(const Matrix<T>& rhs) const;

      inline Matrix<T>& operator+=(const Matrix<T>& opMatrix);
      inline Matrix<T>& operator-=(const Matrix<T>& opMatrix);
      inline Matrix<T>& operator*=(const double scalar);

   private:
      Vector<Vector<T> > mMatrix;

      /**
       * The LUDecompose function performs the LU decomposition of an n × n matrix M.
       *
       * WARNING: LuDecompose changes the vector index!
       *
       * @param     index
       *            A vector size n where the row permutation information can be stored.
       * @param     detVal
       *            The value of the matrix determinant
       * @exception FusionException
       *            Throws a FusionException with string text descibing why the operation
       *            failed. Most likely cause is a singular or non-square matrix.
       */
      void LuDecompose(Vector<int>& index, T& detVal);

      /**
       * Used privately to invert matrices. Can also be used to
       * uses solve a linear system using LuDecompose
       *
       * @param     index
       *            The row permutation information obtained by LuDecompose
       * @param     bx
       *            The solutions to the linear equations
       */
      void LuBackSubstitution(Vector<int> index, Vector<T>& bx) const;

      /**
       * Refines the numerical solution to a linear system previously calculated by LuDecompose
       */
      void LuImproveSolution(Matrix& luDecomp, int* index, double* b, double* x) const;
};

template<class T>
Matrix<T>::Matrix(size_t rows, size_t columns)
{
   mMatrix.reserve(rows);
   for (size_t i = 0; i < rows; i++)
   {
      mMatrix.push_back(Vector<T>(columns));
   }
   setToZero();
}

template<class T>
Matrix<T>::Matrix(const Matrix<T>& srcMatrix)
{
   mMatrix.resize(srcMatrix.mMatrix.size());

   for (size_t i = 0; i < srcMatrix.mMatrix.size(); i++)
   {
      mMatrix[i] = srcMatrix[i];
   }
}

template<class T>
inline const Vector<T>& Matrix<T>::operator[](const size_t row) const
{
   if (row < mMatrix.size())
   {
      return *(mMatrix.begin() + row);
   }
   else
   {
      throw FusionException(std::string("Out of bounds"), __LINE__, __FILE__);
   }
}

template<class T>
inline Vector<T>& Matrix<T>::operator[](const size_t row)
{
   if (row < mMatrix.size())
   {
      return *(mMatrix.begin() + row);
   }
   else
   {
      throw FusionException(std::string("Out of bounds"), __LINE__, __FILE__);
   }
}

template<class T>
inline int Matrix<T>::getNumColumns() const
{
   try
   {
      return static_cast<int>(mMatrix[0].size());
   }
   catch (std::out_of_range exc)
   {
      throw FusionException(std::string("Out of bounds"), __LINE__, __FILE__);
   }
}

template<class T>
void Matrix<T>::setToIdentity()
{
   for (int i = 0; i < getNumRows(); i++)
   {
      for (int j = 0; j < getNumColumns(); j++)
      {
         mMatrix[i][j] = (T)(i == j);
      }
   }
}

template<class T>
void Matrix<T>::setToZero()
{
   for (int i = 0; i < getNumRows(); i++)
   {
      mMatrix[i] *= 0;
   }
}

template<class T>
void Matrix<T>::print() const
{
   Vector< Vector<T> >::const_iterator iter;

   for (iter = mMatrix.begin(); iter != mMatrix.end(); iter++)
   {
      iter->print();
   }
}

template<class T>
T Matrix<T>::determinant() const
{
   T detVal = (T)0;

   //----- Determinant only has meaning for square matrix
   if (getNumColumns() != getNumRows())
   {
      throw FusionException(std::string("Matrix is not square"), __LINE__, __FILE__);
   }

   //----- Create needed pivot array
   Vector<int> pivotInd(getNumRows());

   //----- Copy values into working matrix.  Need a copy because
   //----- the LuDecompose method alters the matrix.
   Matrix<T> detMatrix(*this);

   //----- Get determinant value from LuDecompose.
   //----- Perform lower-upper triangle decomposition
   detMatrix.LuDecompose(pivotInd, detVal);
   for (int i = 0; i < detMatrix.getNumRows(); i++)
   {
      detVal *= detMatrix[i][i];
   }

   return detVal;
}

template<class T>
Matrix<T> Matrix<T>::invert() const
{
   T detVal = (T)0;

   int i;
   int j;
   Vector<int> indexVector(getNumRows());
   Vector<T> columnVector(getNumRows());

   //----- Create copy of this Matrix
   Matrix<T> decMatrix(*this);
   //----- Decompose the copy.  Check for invalid values
   decMatrix.LuDecompose(indexVector, detVal);
   //----- Create return Matrix
   Matrix<T> transMatrix(getNumRows(), getNumColumns());
   //----- Find inverse by column.
   for (j = 0; j < getNumRows(); j++)
   {
      for (i = 0; i < getNumRows(); i++)
      {
         columnVector[i] = (T)0;
      }

      columnVector[j] = (T)1;
      decMatrix.LuBackSubstitution(indexVector, columnVector);
      for (i = 0; i < getNumRows(); i++)
      {
         transMatrix[i][j] = columnVector[i];
      }
   }
   return transMatrix;
}

template<class T>
inline Matrix<T>& Matrix<T>::operator=(const Matrix<T>& srcMatrix)
{
   Vector< Vector<T> >::const_iterator iter;
   if (this != &srcMatrix)
   {
      mMatrix.clear();
      mMatrix.reserve(getNumRows());

      // for every row in srcMatrix, make a copy and push_back onto mMatrix
      for (iter = srcMatrix.mMatrix.begin(); iter != srcMatrix.mMatrix.end(); iter++)
      {
         mMatrix.push_back(*iter);
      }
   }

   return *this;
}

template<class T>
inline Matrix<T> Matrix<T>::operator+(const Matrix<T>& opMatrix) const
{
   Matrix<T> returnMatrix(*this);
   returnMatrix += opMatrix;
   return returnMatrix;
}

template<class T>
inline Matrix<T> Matrix<T>::operator-(const Matrix<T>& opMatrix) const
{
   Matrix<T> returnMatrix(*this);
   returnMatrix -= opMatrix;
   return returnMatrix;
}

template<class T>
inline Matrix<T> Matrix<T>::operator*(double scalar) const
{
   Matrix<T> returnMatrix(*this);
   returnMatrix *= scalar;
   return returnMatrix;
}

template<class T>
inline Matrix<T> Matrix<T>::operator*(const Matrix<T>& opMatrix) const
{
   Matrix<T> returnMatrix(getNumRows(), opMatrix.getNumColumns());

   //----- Can only multiply if LHS columns == RHS rows
   if (getNumColumns() != opMatrix.getNumRows())
   {
      throw FusionException(std::string("Size mismatch"), __LINE__, __FILE__);
   }

   for (int row = 0; row < getNumRows(); row++)
   {
      for (int col = 0; col < opMatrix.getNumColumns(); col++)
      {
         returnMatrix[row][col] = (T)0;
         for (int i = 0; i < getNumColumns(); i++)
         {
            returnMatrix[row][col] += mMatrix[row][i] * opMatrix[i][col];
         }
      }
   }

   return returnMatrix;
}

template<class T>
inline bool Matrix<T>::operator==(const Matrix<T>& rhs) const
{
   return !(*this != rhs);
}

template<class T>
inline bool Matrix<T>::operator!=(const Matrix<T>& rhs) const
{
   Vector< Vector<T> >::const_iterator iter;
   Vector< Vector<T> >::const_iterator rhsIter;

   if ((rhs.getNumRows() != getNumRows()) || (rhs.getNumColumns() != getNumColumns()))
   {
      throw FusionException(std::string("Size mismatch"), __LINE__, __FILE__);
   }

   for (iter = mMatrix.begin(), rhsIter = rhs.mMatrix.begin();
      iter != mMatrix.end() && rhsIter != rhs.mMatrix.end();
      iter++, rhsIter++)
   {
      if (*iter != *rhsIter)
      {
         return true;
      }
   }

   return false;
}

template<class T>
inline Matrix<T> Matrix<T>::operator~() const
{
   if (getNumColumns() != getNumRows())
   {
      throw FusionException(std::string("Matrix is not square"), __LINE__, __FILE__);
   }

   Matrix<T> returnMatrix(getNumRows(), getNumColumns());

   for (int i = 0; i < getNumRows(); i++)
   {
      for (int j = 0; j < getNumColumns(); j++)
      {
         returnMatrix[j][i] = mMatrix[i][j];
      }
   }

   return returnMatrix;
}

template<class T>
inline Matrix<T>& Matrix<T>::operator+=(const Matrix& srcMatrix)
{
   Vector< Vector<T> >::iterator iter;
   Vector< Vector<T> >::const_iterator srcIter;

   // size mismatch error
   if ((srcMatrix.getNumRows() != getNumRows()) || (srcMatrix.getNumColumns() != getNumColumns()))
   {
      throw FusionException(std::string("Size mismatch"), __LINE__, __FILE__);
   }

   for (iter = mMatrix.begin(), srcIter = srcMatrix.mMatrix.begin();
      iter != end() && srcIter != srcMatrix.mMatrix.end();
      iter++, srcIter++)
   {
      *iter += *srcIter;
   }

   return *this;
}

template<class T>
inline Matrix<T>& Matrix<T>::operator-=(const Matrix& srcMatrix)
{
   (*this) += (srcMatrix * -1);
   return *this;
}

template<class T>
inline Matrix<T>& Matrix<T>::operator*=(const double scalar)
{
   Matrix<T>::iterator iter;

   for (iter = begin(); iter != end(); iter++)
   {
      *iter *= scalar;
   }

   return *this;
}

template<class T>
void Matrix<T>::LuDecompose(Vector<int>& index, T& detVal)
{
   T verySmallValue = numeric_limits<T>::epsilon();
   T currentVal = (T)0, detSum = (T)0, largestVal = (T)0, sum = (T)0;
   // Interim calculation values
   int pivotVal = 0;
   int rank = getNumRows(); // Calculated deterministic values
   int i;
   int j;
   int k;

   detVal = (T)1;

   //----- LuDecompose only has meaning for square matrix
   if (getNumColumns() != getNumRows())
   {
      throw FusionException(std::string("Size mismatch"), __LINE__, __FILE__);
   }

   //----- Create scaling array for column calculation
   Vector<T> scalingVector( getNumRows() );

   //----- Obtain the implicit scaling information.
   for (i = 0; i < rank; i++)
   {
      largestVal = (T)0;
      for (j = 0; j < rank; j++) 
      {
         currentVal = (T)fabs((double)mMatrix[i][j]);
         if (currentVal > largestVal)
         {
            largestVal = currentVal;
         }
      }
      //----- Check for singularity --> no non-zero largest element
      if (largestVal == (T)0)
      {
         throw FusionException(std::string("Matrix singular"), __LINE__, __FILE__);
      }
      else
      {
         //----- Save the scaling.
         scalingVector[i] = (T)((T)1 / largestVal);
      }
   }

   //----- Loop over columns using Crout's methods.
   for (j = 0; j < rank; j++)
   {
      for (i = 0; i < j; i++)
      {
         sum = mMatrix[i][j];
         for (k = 0; k < i; k++)
         {
            sum -= mMatrix[i][k] * mMatrix[k][j];
         }

         mMatrix[i][j] = sum;
      }
      //----- Reset for search for largest pivot element.
      largestVal = (T)0;
      for (i = j; i < rank; i++) 
      {
         sum = mMatrix[i][j];
         for (k = 0; k < j; k++)
         {
            sum -= mMatrix[i][k] * mMatrix[k][j];
         }

         mMatrix[i][j] = sum;
         //----- If the figure of merit for the pivot is better than that
         //----- found so far, then update.
         detSum = (T) scalingVector[i] * (T)fabs((double)sum);
         if (detSum >= largestVal)
         {
            largestVal = detSum;
            pivotVal = i;
         }
      }

      //----- Check to see if rows need to be interchanged.
      if (j != pivotVal)
      {
         for (k = 0; k < rank; k++)
         {
            detSum = mMatrix[pivotVal][k];
            mMatrix[pivotVal][k] = mMatrix[j][k];
            mMatrix[j][k] = detSum;
         }
         //----- Change the parity of the matrix and interchange the scale
         //----- factor.
         detVal *= (T)-1;
         scalingVector[pivotVal] = scalingVector[j];
      }
      index[j] = pivotVal;
      //----- If the pivot element is zero, the matrix is singular (to the
      //----- precision of the algorithm).  Set the pivot element to a very
      //----- small value.
      if (mMatrix[j][j] == (T)0)
      {
         mMatrix[j][j] = (T)verySmallValue;
      }

      if (j != (rank - 1))
      {
         detSum = (T)((T)1 / mMatrix[j][j]);
         for (i = j + 1; i < rank; i++)
         {
            mMatrix[i][j] *= detSum;
         }
      }
   }
}

template<class T>
void Matrix<T>::LuBackSubstitution(Vector<int> index, Vector<T>& bx) const
{
   int    intermed_ndx = 0, pivot_ndx = 0; // Indices used in calculation
   int    vect_size = getNumRows();        // Size of column to calculate
   T hold_val = (T)0;                  // Temporary hold value
   int i, j;
   //----- LuBackSubstitution only has meaning for square matrix
   if (getNumRows() != getNumColumns())
   {
      throw FusionException(std::string("Matrix is not square"), __LINE__, __FILE__);
   }
   //----- When intermed_ndx is set to a positive value, it will become the
   //----- index of the first nonvanishing element of bx.
   for (i = 0; i < vect_size; i++)
   {
      pivot_ndx = index[i];
      hold_val = bx[pivot_ndx];
      bx[pivot_ndx] = bx[i];
      if (intermed_ndx > 0)
      {
         for (j = intermed_ndx-1; j <= i-1; j++)
         {
            hold_val -= mMatrix[i][j] * bx[j]; 
         }
      }
      else if ( hold_val != (T)0 )
      {
         intermed_ndx = i + 1;
      }
      bx[i] = (T)hold_val;
   }
   for (i = vect_size - 1; i >= 0; i--)
   {
      hold_val = bx[i];
      for (j = i + 1; j < getNumRows(); j++)
      {
         hold_val -= mMatrix[i][j] * bx[j];
      }

      bx[i] = (T)(hold_val / mMatrix[i][i]);
   }
}

#endif
