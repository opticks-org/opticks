/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "assert.h"
#include "MatrixFunctions.h"
#include "ObjectResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "TestCase.h"
#include "TestSuiteNewSession.h"

#include <limits>
#include <math.h>
#include <string>
#include <vector>

using namespace std;

class MatrixResourceTestCase : public TestCase
{
public:
   MatrixResourceTestCase() : TestCase("MatrixResource") { }
   bool run()
   {
      bool success = true;

      // 3x3 matrix - double with known values
      {
         const int numRows = 3;
         const int numCols = 3;
         const double pMatrix[] = {
            1.0, 2.0, 3.0,
            4.0, 5.0, 6.0,
            7.0, 8.0, 9.0 };

         issearf(testMatrixResource<double>(numRows, numCols, pMatrix) == true);
      }

      // 4x5 matrix - int with known values
      {
         const int numRows = 4;
         const int numCols = 5;
         const int pMatrix[] = {
            0, 1, 2, 3, 4,
            5, 6, 7, 8, 9,
            0, 1, 2, 3, 4,
            5, 6, 7, 8, 9 };

         issearf(testMatrixResource<int>(numRows, numCols, pMatrix) == true);
      }

      // Several matrices of arbitrary size and type
      {
         issearf(testMatrixResource<char>(312, 1024) == true);
         issearf(testMatrixResource<unsigned char>(1024, 312) == true);

         issearf(testMatrixResource<short>(102, 1024) == true);
         issearf(testMatrixResource<unsigned short>(756, 567) == true);

         issearf(testMatrixResource<int>(567, 897) == true);
         issearf(testMatrixResource<unsigned int>(453, 1524) == true);

         issearf(testMatrixResource<long>(789, 23) == true);
         issearf(testMatrixResource<unsigned long>(765, 342) == true);

         issearf(testMatrixResource<int64_t>(564, 556) == true);
         issearf(testMatrixResource<uint64_t>(756, 468) == true);

         issearf(testMatrixResource<float>(27, 457) == true);
         issearf(testMatrixResource<double>(546, 5433) == true);
      }

      return success;
   }

private:
   template<typename T>
   bool testMatrixResource(const int& numRows, const int& numCols, const T* pOriginalMatrix = NULL)
   {
      bool success = true;

      // Create the matrix
      MatrixFunctions::MatrixResource<T> pNewMatrix(numRows, numCols, pOriginalMatrix);
      issearf(pNewMatrix.get() != NULL);

      // If the matrix had original values, check that they were all copied correctly
      if (pOriginalMatrix != NULL)
      {
         for (int row = 0; row < numRows; ++row)
         {
            isseab(memcmp(pOriginalMatrix + (row * numCols), pNewMatrix[row], numCols * sizeof(T)) == 0);
         }
      }
      // If the matrix did not have original values, check that all values are 0
      else
      {
         for (int row = 0; success && row < numRows; ++row)
         {
            for (int col = 0; col < numCols; ++col)
            {
               isseab(pNewMatrix[row][col] == static_cast<T>(0));
            }
         }
      }

      return success;
   }
};

class GetEigenvaluesTestCase : public TestCase
{
public:
   GetEigenvaluesTestCase() : TestCase("GetEigenvalues") { }
   bool run()
   {
      bool success = true;

      // 1x1 matrix
      {
         const int numRows = 1;
         const double tolerance = 0.0;
         const double pOriginalMatrix[] = {17.0 };
         const double pExpectedEigenvalues[] = { 17.0 };
         const double pExpectedEigenvectors[] = { 1.0 };
         issearf(testEigenvalues(pOriginalMatrix, pExpectedEigenvalues, pExpectedEigenvectors, numRows, tolerance));
      }

      // 2x2 matrix
      {
         const int numRows = 2;
         const double pOriginalMatrix[] = {
            2.0, 2.0,
            2.0, 2.0};

         const double pExpectedEigenvalues[] = {
            4.0,
            0.0 };

         const double pExpectedEigenvectors[] = {
            0.70710678118654746, 0.70710678118654746,
            0.70710678118654746,-0.70710678118654746 };

         issearf(testEigenvalues(pOriginalMatrix, pExpectedEigenvalues, pExpectedEigenvectors, numRows));
      }

      // 3x3 matrix
      {
         const int numRows = 3;
         const double pOriginalMatrix[] = {
            -5.0, 8.0, -7.0,
            8.0, 2.0, -3.0,
            -7.0, -3.0, -10 };

         const double pExpectedEigenvalues[] = {
            9.406219706360574,
            -7.110208066499869,
            -15.296011639860705 };

         const double pExpectedEigenvectors[] = {
            -0.57464644687740718, -0.51116632484818070, 0.63913257578719540,
            -0.75175356361177748, 0.63838024909823554, -0.16533976278641541,
            0.32349349404302469, 0.57548209868116584, 0.75111404820397032 };

         issearf(testEigenvalues(pOriginalMatrix, pExpectedEigenvalues, pExpectedEigenvectors, numRows));
      }

      // 4x4 matrix
      {
         const int numRows = 4;
         const double pOriginalMatrix[] = {
           -9.0/534.0, -7.0/754.0, -9.0/234.0, 5.0/24.0,
           -7.0/754.0, 7.0/987.0, -8.0/324.0, -2.0/342.0,
           -9.0/234.0, -8.0/324.0, 0.0, -5.0/436.0,
            5.0/24.0, -2.0/342.0, -5.0/436.0, 22.0/7.0 };

         const double pExpectedEigenvalues[] = {
            3.156608766224691,
            0.03554303452690023,
            0.0035368777340949453,
            -0.06259326963125389 };

         const double pExpectedEigenvectors[] = {
            0.065565863666658203, -0.35700480358172532, 0.54152547873031776, 0.75828678191507370,
            -0.0020114695199988705, -0.54610626117767802, -0.78166036519093995, 0.30128222480338041,
            -0.0044082678565761542, 0.75740006793366754, -0.30704046321123601, 0.57623941050539895,
            0.99783647893155325, 0.025703284151773285, -0.038514715713560466, -0.046672466945693311 };

         issearf(testEigenvalues(pOriginalMatrix, pExpectedEigenvalues, pExpectedEigenvectors, numRows));
      }

      // 5x5 matrix
      {
         const int numRows = 5;
         const double pOriginalMatrix[] = {
            -4.0, -5.0, -1.0, 3.0, -1.0,
            -5.0, -9.0, -7.0, -5.0, -8.0,
            -1.0, -7.0, 1.0, -6.0, 0.0,
            3.0, -5.0, -6.0, -7.0, 6.0,
            -1.0, -8.0, 0.0, 6.0, 3.0 };

         const double pExpectedEigenvalues[] = {
            10.895295935358849,
            5.527034802634091,
            -1.5606657428893775,
            -12.261640796550118,
            -18.600024198553445 };

         const double pExpectedEigenvectors[] = {
            -0.18304335382926512, 0.072347490628551864, 0.84919359076375156, -0.43473397765033617, 0.22613620961882408,
            0.47901677568331597, 0.25106901709447105, -0.24035331565046270, -0.22709730897673153, 0.77341086957743987,
            -0.083065326338053247, -0.89123151752946372, 0.041505202130022192, 0.17904425918773922, 0.4062351599388475,
            -0.39135301497862518, 0.36746758112840477, 0.15659342800592230, 0.73313929875327921, 0.38703444474062398,
            -0.7595926727268941, 0.04903211434086854, -0.4414253567287395, -0.43575578344676835, 0.18940795899981375 };

         issearf(testEigenvalues(pOriginalMatrix, pExpectedEigenvalues, pExpectedEigenvectors, numRows));
      }

      return success;
   }

private:
   bool testEigenvalues(const double* pOriginalMatrix, const double* pExpectedEigenvalues,
      const double* pExpectedEigenvectors, const int& numRows, const double& tolerance = 1e-6)
   {
      bool success = true;
      const int numCols = numRows;

      MatrixFunctions::MatrixResource<double> ppOriginalMatrix(numRows, numCols, pOriginalMatrix);
      issearf(ppOriginalMatrix.get() != NULL);

      vector<double> eigenvalues(numRows);
      issea(MatrixFunctions::getEigenvalues(ppOriginalMatrix, &eigenvalues.front(), ppOriginalMatrix, numRows));
      for (int row = 0; success && row < numRows; ++row)
      {
         isseab(fabs(eigenvalues[row] - pExpectedEigenvalues[row]) <= tolerance);
         for (int col = 0; success && col < numCols; ++col)
         {
            isseab(fabs(ppOriginalMatrix[row][col] - pExpectedEigenvectors[row * numCols + col]) <= tolerance);
         }
      }

      return success;
   }
};

class InvertMatrixTestCase : public TestCase
{
public:
   InvertMatrixTestCase(const string& name = "InvertMatrix") : TestCase(name) { }
   bool run()
   {
      bool success = true;

      // 1x1 matrix
      {
         // Positive test
         const int numRows = 1;
         const double pOriginalMatrix[] = {17.0};
         const double pInverseMatrix[] = {1.0 / 17.0};

         // Test that inversion is commutative. In other words, A * A' = A' * A = I.
         issearf(testInversion(pOriginalMatrix, pInverseMatrix, numRows));
         issearf(testInversion(pInverseMatrix, pOriginalMatrix, numRows));

         // Negative tests (invalid parameters)
         issearf(testInversionNegative(NULL, pInverseMatrix, numRows));
         issearf(testInversionNegative(pOriginalMatrix, pInverseMatrix, 0));
         issearf(testInversionNegative(pOriginalMatrix, pInverseMatrix, -1));
      }

      // 2x2 matrix
      {
         // Positive test
         const int numRows = 2;
         double pOriginalMatrix[] = {
            1.0, 2.0,
            3.0, 4.0 };

         const double pInverseMatrix[] = {
            -2.0, 1.0,
            1.5, -.5 };

         // Test that inversion is commutative. In other words, A * A' = A' * A = I.
         issearf(testInversion(pOriginalMatrix, pInverseMatrix, numRows));
         issearf(testInversion(pInverseMatrix, pOriginalMatrix, numRows));

         // Negative test (find the inverse of a matrix whose determinant is 0)
         pOriginalMatrix[3] = 6.0;
         issearf(testInversionNegative(pOriginalMatrix, pInverseMatrix, numRows));
      }

      // 3x3 matrix
      {
         // Positive test
         const int numRows = 3;
         const double pOriginalMatrix[] = {
            -4.0, -1.0, 4.0,
           11.0,  3.0, -4.0,
           -4.0, 0.0, -2.0 };

         double pInverseMatrix[] = {
            -3.0 / 17.0, -1.0 / 17.0, -4.0 / 17.0,
            19.0 / 17.0, 12.0 / 17.0, 14.0 / 17.0,
            6.0 / 17.0, 2.0 / 17.0, -1.0 / 34.0 };

         // Test that inversion is commutative. In other words, A * A' = A' * A = I.
         issearf(testInversion(pOriginalMatrix, pInverseMatrix, numRows));
         issearf(testInversion(pInverseMatrix, pOriginalMatrix, numRows));

         // Negative test (incorrect inverse matrix supplied)
         memset(pInverseMatrix, 0, sizeof(pInverseMatrix));
         issearf(testInversionNegative(pOriginalMatrix, pInverseMatrix, numRows));
      }

      // 4x4 matrix
      {
         // Positive test
         const int numRows = 4;
         double pOriginalMatrix[] = {
            -3.0, 1.0, 3.0, 2.0,
            -15.0, -1.0, -10.0, -5.0,
            -1.0, -5.0, 5.0, 3.0,
            3.0, -1.0, -1.0, -2.0 };

         const double pInverseMatrix[] = {
            -29.0 / 188.0, -13.0 / 282.0, -1.0 / 94.0, -31.0 / 564.0,
            35.0 / 188.0, -7.0 / 282.0, -15.0 / 94.0, 5.0 / 564.0,
            .5, 0.0, 0.0, .5,
            -27.0 / 47.0, -8.0 / 141.0, 3.0 / 47.0, -118.0 / 141.0 };

         // Test that inversion is commutative. In other words, A * A' = A' * A = I.
         issearf(testInversion(pOriginalMatrix, pInverseMatrix, numRows));
         issearf(testInversion(pInverseMatrix, pOriginalMatrix, numRows));

         // Negative test (incorrect inverse matrix supplied)
         memset(pOriginalMatrix, 0, sizeof(pOriginalMatrix));
         issearf(testInversionNegative(pOriginalMatrix, pInverseMatrix, numRows));
      }

      // 5x5 matrix
      {
         // Positive test
         const int numRows = 5;
         const double pOriginalMatrix[] = {
            -4.0, 5.0, 1.0, -1.0, 2.0,
             3.0, 1.0, 1.0, 4.0, 1.0,
            -1.0, 2.0, -3.0, 0.0, 3.0,
            4.0, 2.0, 4.0, 1.0, -2,
            4.0, 4.0, 2.0, -3.0, 0 };

         const double pInverseMatrix[] = {
            -10.0 / 91.0, 1.0 / 13.0, 2.0 / 91.0, -1.0 / 26.0, 23.0 / 182.0,
            -2.0 / 91.0, -5.0 / 13.0, 55.0 / 91.0, 9.0 / 13.0, -25.0 / 91.0,
            3.0 / 13.0, 7.0 / 13.0, -11.0 / 13.0, -10.0 / 13.0, 5.0 / 13.0,
            -2.0 / 91.0, -2.0 / 39.0, 74.0 / 273.0, 14.0 / 39.0, -25.0 / 91.0,
            19.0 / 91.0, 32.0 / 39.0, -248.0 / 273.0, -97.0 / 78.0, 111.0 / 182.0 };

         // Test that inversion is commutative. In other words, A * A' = A' * A = I.
         issearf(testInversion(pOriginalMatrix, pInverseMatrix, numRows));
         issearf(testInversion(pInverseMatrix, pOriginalMatrix, numRows));
      }

      // 6x6 matrix
      {
         // Positive test (identity matrix)
         const int numRows = 6;
         const double pMatrix[] = {
            1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 1.0 };

         // Test inversion for the identity matrix
         issearf(testInversion(pMatrix, pMatrix, numRows));
      }

      return success;
   }

protected:
   virtual bool testInversion(const double* pOriginalMatrix,
      const double* pExpectedInverseMatrix, const int& numRows, const double& tolerance = 1e-6)
   {
      bool success = true;
      issearf(testInversion1D(pOriginalMatrix, pExpectedInverseMatrix, numRows, tolerance));
      issearf(testInversion2D(pOriginalMatrix, pExpectedInverseMatrix, numRows, tolerance));
      return success;
   }

   virtual bool testInversionNegative(const double* pOriginalMatrix,
      const double* pExpectedInverseMatrix, const int& numRows, const double& tolerance = 1e-6)
   {
      bool success = true;
      issearf(testInversionNegative1D(pOriginalMatrix, pExpectedInverseMatrix, numRows, tolerance));
      issearf(testInversionNegative2D(pOriginalMatrix, pExpectedInverseMatrix, numRows, tolerance));
      return success;
   }

private:
   bool testInversion1D(const double* pOriginalMatrix,
      const double* pExpectedInverseMatrix, const int& numRows, const double& tolerance = 1e-6)
   {
      bool success = true;
      const int numCols = numRows;

      vector<double> pInverseMatrix(numRows * numCols);
      issea(MatrixFunctions::invertSquareMatrix1D(&pInverseMatrix.front(), pOriginalMatrix, numRows) == true);

      for (int row = 0; success && row < numRows; ++row)
      {
         const unsigned int rowOffset = row * numCols;
         for (int col = 0; success && col < numCols; ++col)
         {
            isseab(fabs(pInverseMatrix[rowOffset + col] - pExpectedInverseMatrix[rowOffset + col]) <= tolerance);
         }
      }

      return success;
   }

   bool testInversion2D(const double* pOriginalMatrix,
      const double* pExpectedInverseMatrix, const int& numRows, const double& tolerance = 1e-6)
   {
      bool success = true;
      const int numCols = numRows;

      MatrixFunctions::MatrixResource<double> ppOriginalMatrix(numRows, numCols, pOriginalMatrix);
      issearf(ppOriginalMatrix.get() != NULL);

      MatrixFunctions::MatrixResource<double> pInverseMatrix(numRows, numCols);
      issea(pInverseMatrix.get() != NULL);
      issea(MatrixFunctions::invertSquareMatrix2D(pInverseMatrix, ppOriginalMatrix, numRows) == true);

      for (int row = 0; row < numRows; ++row)
      {
         const unsigned int rowOffset = row * numCols;
         for (int col = 0; col < numCols; ++col)
         {
            isseab(fabs(pInverseMatrix[row][col] - pExpectedInverseMatrix[rowOffset + col]) <= tolerance);
         }
      }

      return success;
   }

   bool testInversionNegative1D(const double* pOriginalMatrix,
      const double* pExpectedInverseMatrix, const int& numRows, const double& tolerance = 1e-6)
   {
      bool success = true;

      const int numCols = numRows;
      double* pInverseMatrix = new double[numRows * numCols];
      success = success && (pInverseMatrix != NULL);
      success = success && (MatrixFunctions::invertSquareMatrix1D(pInverseMatrix, pOriginalMatrix, numRows) == true);

      if (pExpectedInverseMatrix != NULL)
      {
         for (int row = 0; success && row < numRows; ++row)
         {
            const unsigned int rowOffset = row * numCols;
            for (int col = 0; success && col < numCols; ++col)
            {
               success = fabs(pInverseMatrix[rowOffset + col] - pExpectedInverseMatrix[rowOffset + col]) <= tolerance;
            }
         }
      }

      delete[] pInverseMatrix;
      return (success == false);
   }

   bool testInversionNegative2D(const double* pOriginalMatrix,
      const double* pExpectedInverseMatrix, const int& numRows, const double& tolerance = 1e-6)
   {
      bool success = true;

      const int numCols = numRows;
      MatrixFunctions::MatrixResource<double> ppOriginalMatrix(numRows, numCols);
      success = success && (ppOriginalMatrix.get() != NULL);

      MatrixFunctions::MatrixResource<double> pInverseMatrix(numRows, numCols);
      issea(pInverseMatrix.get() != NULL);
      success = success && (MatrixFunctions::invertSquareMatrix2D(pInverseMatrix, ppOriginalMatrix, numRows) == true);

      if (pExpectedInverseMatrix != NULL)
      {
         for (int row = 0; success && row < numRows; ++row)
         {
            const unsigned int rowOffset = row * numCols;
            for (int col = 0; success && col < numCols; ++col)
            {
               success = (fabs(pInverseMatrix[row][col] - pExpectedInverseMatrix[rowOffset + col]) <= tolerance);
            }
         }
      }

      return (success == false);
   }
};

class InvertRasterElementTestCase : public InvertMatrixTestCase
{
public:
   InvertRasterElementTestCase() : InvertMatrixTestCase("InvertRasterElement") { }

private:
   bool testInversion(const double* pOriginalMatrix,
      const double* pExpectedInverseMatrix, const int& numRows, const double& tolerance = 1e-6)
   {
      bool success = true;
      const int numCols = numRows;

      ModelResource<RasterElement> pElement(RasterUtilities::createRasterElement(
         "InvertRasterElementTestCase-Normal", numRows, numCols, FLT8BYTES, true, NULL));
      issearf(pElement.get() != NULL);

      double* pData= reinterpret_cast<double*>(pElement->getRawData());
      issearf(pData != NULL);
      memcpy(pData, pOriginalMatrix, numRows * numCols * sizeof(double));

      ModelResource<RasterElement> pInvertedElement(RasterUtilities::createRasterElement(
         "InvertRasterElementTestCase-Inverted", numRows, numCols, FLT8BYTES, true, pElement.get()));
      issearf(pInvertedElement.get() != NULL);
      issearf(MatrixFunctions::invertRasterElement(pInvertedElement.get(), pElement.get()) == true);

      const double* pInverseMatrix= reinterpret_cast<double*>(pInvertedElement->getRawData());
      issearf(pInverseMatrix != NULL);

      for (int row = 0; row < numRows; ++row)
      {
         const unsigned int rowOffset = row * numCols;
         for (int col = 0; col < numCols; ++col)
         {
            issearf(fabs(pInverseMatrix[rowOffset + col] - pExpectedInverseMatrix[rowOffset + col]) <= tolerance);
         }
      }

      return success;
   }

   virtual bool testInversionNegative(const double* pOriginalMatrix,
      const double* pExpectedInverseMatrix, const int& numRows, const double& tolerance = 1e-6)
   {
      // There is no negative test for this TestCase
      // All negative tests for inversion are in the InvertMatrixTestCase
      return true;
   }
};

class SingularValueDecompositionTestCase : public TestCase
{
public:
   SingularValueDecompositionTestCase() : TestCase("SingularValueDecomposition") { }
   bool run()
   {
      bool success = true;

      // 1x1 matrix
      {
         const int numRows = 1;
         const int numCols = 1;
         const double pMatrix[] = { 13.0 };
         const double pExpectedSingularValues[] = { 13.0 };
         const double pExpectedColumnMatrix[] = { -1.0 };
         const double pExpectedOrthogonalMatrix[] = { -1.0 };

         issearf(testSingularValueDecomposition(pMatrix, pExpectedSingularValues,
            pExpectedColumnMatrix, pExpectedOrthogonalMatrix, numRows, numCols));
      }

     // 2x2 matrix
      {
         const int numRows = 2;
         const int numCols = 2;
         const double pMatrix[] = {
            1.0, 2.0,
            3.0, 4.0 };

         const double pExpectedSingularValues[] = {
            0.3659661906262578,
            5.464985704219045 };

         const double pExpectedColumnMatrix[] = {
            0.9145142956773046, 0.40455358483375736,
            -0.40455358483375714, 0.9145142956773046 };

         const double pExpectedOrthogonalMatrix[] = {
            -0.8174155604703635, 0.5760484367663207,
            0.5760484367663207, 0.8174155604703635 };

         issearf(testSingularValueDecomposition(pMatrix, pExpectedSingularValues,
            pExpectedColumnMatrix, pExpectedOrthogonalMatrix, numRows, numCols));
      }

     // 5x3 matrix
      {
         const int numRows = 5;
         const int numCols = 3;
         const double pMatrix[] = {
            1.0, 2.0, 3.0,
            4.0, 5.0, 6.0,
            7.0, 8.0, 9.0,
            10.0, 11.0, 12.0,
            13.0, 14.0, 15.0 };

         const double pExpectedSingularValues[] = {
            35.182648331894214,
            1.4769076999800883,
            7.195761392886296e-16 };

         issearf(testSingularValueDecomposition(pMatrix, pExpectedSingularValues, NULL, NULL, numRows, numCols));
      }

      // 6x4 matrix
      {
         const int numRows = 6;
         const int numCols = 4;
         const double pMatrix[] = {
            342.234, 324342.567, 346.768, .4258976324,
            345.85, 456.523, 4364.474, 78.675453,
            568346.75, 364347.734, 345346.634346, 564.546675,
            345625.35235, 52264.685546, 346346.56546, 453.546,
            12235.647587, 357.536785, 54685.856567, 876.789,
            45654.57856, 769.543324, 634436.634634, 543345.453 };

         const double pExpectedSingularValues[] = {
            1043005.808507220,
            657239.019536324,
            67892.4588499282,
            317680.412268427 };

         issearf(testSingularValueDecomposition(pMatrix, pExpectedSingularValues, NULL, NULL, numRows, numCols));
      }

      return success;
   }

private:
   bool SingularValueDecompositionTestCase::testSingularValueDecomposition(const double* pOriginalMatrix,
      const double* pExpectedSingularValues, const double* pExpectedColumnMatrix,
      const double* pExpectedOrthogonalMatrix, const int& numRows, const int& numCols, const double& tolerance = 1e-6)
   {
      bool success = true;

      // Create temporary matrices for holding the results of computeSingularValueDecomposition.
      MatrixFunctions::MatrixResource<double> ppOriginalMatrix(numRows, numCols, pOriginalMatrix);
      issearf(ppOriginalMatrix.get() != NULL);

      vector<double> pSingularValues(numCols);

      MatrixFunctions::MatrixResource<double> pColumnMatrix(numRows, numCols);
      issea(pColumnMatrix.get() != NULL);

      MatrixFunctions::MatrixResource<double> pOrthogonalMatrix(numCols, numCols);
      issea(pOrthogonalMatrix.get() != NULL);

      issea(MatrixFunctions::computeSingularValueDecomposition(ppOriginalMatrix,
         &pSingularValues.front(), pColumnMatrix, pOrthogonalMatrix, numRows, numCols));

      // Compare the results to the expected results.
      if (pExpectedColumnMatrix != NULL)
      {
         for (int row = 0; success && row < numRows; ++row)
         {
            int rowOffset = row * numCols;
            for (int col = 0; success && col < numCols; ++col)
            {
               isseab(fabs(pExpectedColumnMatrix[rowOffset + col] - pColumnMatrix[row][col]) <= tolerance);
            }
         }
      }

      if (pExpectedSingularValues != NULL || pExpectedOrthogonalMatrix != NULL)
      {
         // pOrthogonalMatrix is numCols x numCols, so use numCols instead of numRows.
         for (int row = 0; success && row < numCols; ++row)
         {
            if (pExpectedSingularValues != NULL)
            {
               isseab(fabs(pExpectedSingularValues[row] - pSingularValues[row]) <= tolerance);
            }

            if (pExpectedOrthogonalMatrix != NULL)
            {
               int rowOffset = row * numCols;
               for (int col = 0; success && col < numCols; ++col)
               {
                  isseab(fabs(pExpectedOrthogonalMatrix[rowOffset + col] - pOrthogonalMatrix[row][col]) <= tolerance);
               }
            }
         }
      }

      // Clean up.
      return success;
   }
};

class SolveLinearEquationTestCase : public TestCase
{
public:
   SolveLinearEquationTestCase() : TestCase("SolveLinearEquation") { }
   bool run()
   {
      bool success = true;

      // 1x1
      {
         const int numRows = 1;
         const int numColsLhs = 1;
         const double pLhs[] = { 19.0 };
         const double pRhs[] = { 29.0 };
         const double pExpectedResult[] = { 1.526315789473680 };
         issearf(testSolveLinearEquation(pExpectedResult, pLhs, pRhs, numRows, numColsLhs));
      }

      // 2x2
      {
         const int numRows = 2;
         const int numColsLhs = 2;
         const double pLhs[] = {
            325.19, 785.563,
            234.765, 534.536 };

         const double pRhs[] = {
            765.290,
            564657.5465420 };

         const double pExpectedResult[] = {
             41820.107929560900000,
            -17310.789341674600000 };

         issearf(testSolveLinearEquation(pExpectedResult, pLhs, pRhs, numRows, numColsLhs));
      }

      return success;
   }

private:
   bool testSolveLinearEquation(const double* pExpectedResult, const double* pLhs, const double* pRhs,
      const int& numRows, const int& numColsLhs, const double& tolerance = 1e-6)
   {
      bool success = true;

      MatrixFunctions::MatrixResource<double> ppLhs(numRows, numColsLhs, pLhs);
      issearf(ppLhs.get() != NULL);

      vector<double> pResult(numRows);
      issea(MatrixFunctions::solveLinearEquation(&pResult.front(), ppLhs, pRhs, numRows, numColsLhs) == true);

      for (int row = 0; success && row < numRows; ++row)
      {
         isseab(fabs(pExpectedResult[row] - pResult[row]) <= tolerance);
      }

      return success;
   }
};

class MatrixFunctionsTestSuite : public TestSuiteNewSession
{
public:
   MatrixFunctionsTestSuite() : TestSuiteNewSession( "MatrixFunctions" )
   {
      addTestCase(new MatrixResourceTestCase);
      addTestCase(new GetEigenvaluesTestCase);
      addTestCase(new InvertMatrixTestCase);
      addTestCase(new InvertRasterElementTestCase);
      addTestCase(new SingularValueDecompositionTestCase);
      addTestCase(new SolveLinearEquationTestCase);
   }
};

REGISTER_SUITE(MatrixFunctionsTestSuite)
