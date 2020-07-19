/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "Executable.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "TestBedTestUtilities.h"
#include "TestCase.h"
#include "TestSuiteNewSession.h"

#include <math.h>

using namespace std;

class SecondMomentMatrixTestCase : public TestCase
{
public:
   SecondMomentMatrixTestCase() : TestCase("SecondMomentMatrix") {}
   bool run()
   {
      bool success = true;
      RasterElement* pElement = TestUtilities::getStandardRasterElement(false, true);
      issearf(pElement != NULL);

      // Run Second Moment, calculating the results and loading from disk.
      issearf(runSecondMoment(pElement, true) == true);
      issearf(runSecondMoment(pElement, false) == true);

      return success;
   }

private:
   bool runSecondMoment(RasterElement* pElement, bool recalculate)
   {
      bool success = true;
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      issearf(pDescriptor != NULL);

      ExecutableResource pPlugIn("Second Moment");
      issearf(pPlugIn.get() != NULL);

      PlugInArgList& argsIn = pPlugIn->getInArgList();
      issearf(argsIn.setPlugInArgValue<bool>("Recalculate", &recalculate));
      issearf(argsIn.setPlugInArgValue<RasterElement>(Executable::DataElementArg(), pElement));
      issearf(pPlugIn->execute());

      PlugInArgList& argsOut = pPlugIn->getOutArgList();
      ModelResource<RasterElement> pElementOut(argsOut.getPlugInArgValue<RasterElement>("Second Moment Matrix"));
      issearf(pElementOut.get() != NULL);
      issearf(pElementOut->getParent() == pElement);
      ModelResource<RasterElement> pInvElementOut(
         argsOut.getPlugInArgValue<RasterElement>("Inverse Second Moment Matrix"));
      issearf(pInvElementOut.get() != NULL);
      issearf(pInvElementOut->getParent() == pElement);

      RasterDataDescriptor* pDescriptorOut =
         dynamic_cast<RasterDataDescriptor*>(pInvElementOut->getDataDescriptor());
      issearf(pDescriptorOut != NULL);
      issearf(pDescriptorOut->getRowCount() == pDescriptor->getBandCount());
      issearf(pDescriptorOut->getColumnCount() == pDescriptor->getBandCount());

      // This value is used to determine how close the computed result must be to its counterpart in pExpectedData.
      const double tolerance = 1e-6;

      // A sample of expected values which should have been computed. This sample includes only the first 3 rows.
      const double pExpectedData[] = {
         0.000166043, -2.9899e-005, -1.77223e-005, -1.72119e-005, -1.26652e-005, -1.66357e-005, -1.34303e-005,
         -7.57501e-006, -9.65545e-006, -2.25395e-006, -3.72119e-006, -7.7266e-007, -2.41062e-006, 2.94564e-006,
         9.11922e-007, -2.63895e-006, -1.32252e-005, -2.12275e-006, 6.06037e-006, 3.00735e-006, -6.1072e-007,
         -1.0488e-006, -7.74092e-006, -5.62641e-006, -8.73944e-006, 5.43698e-006, -3.50089e-006, 8.19713e-006,
         -1.63395e-006, 5.84713e-006, -7.40309e-006, 8.8169e-006, 5.54338e-006, -2.21671e-006, 1.86404e-006,
         -4.74112e-006, 2.76157e-006, -7.65263e-006, 5.87387e-006, -1.18174e-005, 3.62501e-006, -1.51888e-005,
         -1.26801e-005, 1.43538e-005, -1.63429e-005, 7.26224e-006, -1.8441e-005, 1.63278e-005, 5.66603e-006,
         2.0051e-005, -1.03603e-005, 1.12578e-005, -1.60639e-005, -6.17884e-007, -1.33717e-005, -1.7789e-005,
         -4.42868e-006, 2.73692e-006, 1.18744e-005, -2.83726e-006, 7.23106e-005, -1.26785e-005, -4.61003e-007,
         1.25508e-005, -1.92899e-005, -1.65281e-005, 4.26729e-005, -7.56699e-006, -6.97046e-005, -4.20885e-005,
         1.63123e-005, 5.22407e-005, -2.29662e-006, 3.42708e-005, -8.87318e-006, 2.5208e-006, 1.44119e-005,
         -2.18394e-005, -3.14121e-005, 2.75622e-005, 3.2084e-005, -5.02554e-006, 3.08154e-005, 7.84896e-005,
         -3.53783e-005, -3.36672e-005, 2.16426e-005, -1.53942e-005, -7.89073e-006, -3.22181e-005, 3.46034e-005,
         -1.30305e-005, 1.55531e-006, -5.60585e-005, 1.43874e-005, 1.5104e-005, -3.62533e-005, -4.96297e-005,
         -5.51749e-005, -3.64085e-005, -5.96542e-005, 8.02484e-005, 5.7902e-005, -0.000104525, 7.12703e-005,
         0.000131978, 0.000149503, 0.000176561, -0.000117289, -9.53063e-005, -1.10296e-005, -0.000208507,
         -5.45587e-005, -0.00010673, 7.59696e-005, -7.60181e-006, 0.000207874, 9.2218e-006, 7.60899e-007,
         -7.51802e-005, -4.35075e-005, -4.73887e-005, -0.000110114, 0.000333339, -0.000140805, 0.000105877,
         0.000406968, -0.000171736, 1.84011e-005, -3.45368e-005, 3.75232e-005, -0.000315975, 0.000199941,
         -0.000117066, -0.000144345, -6.86627e-005, -0.000126988, 9.41645e-005, 0.000245216, -2.8992e-005,
         0.000148961, 4.39918e-005, 9.28471e-005, -6.50959e-005, -0.000251514, 1.55543e-005, 0.000286022,
         0.000104939, -2.36384e-005, -0.00018182, 0.000115128, -9.19449e-005, -1.53909e-005, 0.000193553,
         -8.27103e-005, 1.0271e-005, 0.000448521, -0.000225863, -1.07934e-005, -0.000186375, 9.32742e-005,
         -9.94186e-005, -0.000187485, -7.55668e-005, -0.000229139, -0.000179596, -5.21521e-005, 1.20869e-006,
         -2.9899e-005, 0.000193986, -1.93118e-005, -1.88761e-005, -2.83146e-005, -1.79559e-005, -2.07253e-005,
         -1.02623e-005, -1.19958e-005, 6.06347e-006, -9.86282e-006, -3.52125e-006, -1.31272e-005, -5.61394e-007,
         -9.15088e-007, 3.28191e-006, -4.3944e-006, -2.19542e-006, -8.13659e-007, -4.34971e-007, -3.99912e-006,
         -6.70913e-006, -2.78537e-006, -6.42929e-006, -7.16015e-007, 5.1112e-006, 5.83679e-006, -6.82179e-006,
         4.7336e-006, 1.35504e-005, 9.62796e-006, 4.17152e-006, -3.5783e-007, 8.24351e-006, -9.85795e-006,
         -1.48037e-005, 5.40127e-006, -8.17139e-006, -1.62123e-005, 1.24377e-005, -1.035e-005, -8.4317e-006,
         -1.83669e-005, 7.82877e-006, -2.5087e-006, 7.62302e-006, 1.39714e-005, 1.68401e-005, -1.7971e-005,
         1.76358e-005, -2.06297e-005, 2.03178e-005, 1.33137e-005, -4.48137e-005, 1.84446e-005, 5.71514e-006,
         2.39756e-005, -1.05505e-005, -1.97401e-005, -2.25072e-008, 2.06611e-005, 2.49826e-005, -1.50381e-005,
         -2.92733e-005, -1.96509e-006, 4.08802e-005, 2.57101e-005, -9.48916e-006, 7.78849e-006, -2.40273e-005,
         -3.16362e-005, -1.56234e-005, -1.93041e-005, 1.99788e-005, -2.27573e-006, 4.09685e-005, -2.07558e-005,
         1.43337e-005, 2.05865e-006, -2.64091e-005, -7.55487e-006, -8.96625e-005, 2.96548e-005, 6.82514e-005,
         4.93142e-006, -6.56403e-005, -2.12508e-005, -6.66464e-006, -4.30254e-006, 9.38826e-006, 3.21424e-005,
         3.79067e-005, 2.79969e-006, -3.9933e-008, 3.32408e-005, -4.07388e-005, 6.19405e-005, -9.61183e-005,
         7.01841e-005, 3.82319e-005, 6.33349e-005, -1.86651e-005, 2.18877e-005, 3.1561e-005, -8.7484e-005,
         1.20601e-005, 8.03377e-005, 0.000149157, -7.26779e-005, -1.25696e-006, -0.000131122, -0.000189888,
         3.14088e-005, -0.000126876, 6.0927e-005, 1.62928e-006, 2.86056e-005, 0.000143142, 6.23068e-005,
         0.000135214, -6.41202e-005, -6.65412e-006, 0.000111412, 4.70262e-005, 0.00012443, 0.000108517,
         -1.37469e-005, 4.35621e-005, 4.95036e-005, -6.80236e-007, -7.08518e-005, -0.000326738, -0.000114146,
         1.56855e-005, -0.000222354, -8.9881e-005, 0.000190296, -6.5896e-005, 0.000275526, 0.000149371, -0.000133252,
         0.000142457, -1.02148e-005, -6.1321e-005, -1.49992e-005, 3.89233e-005, 5.53839e-005, -9.84011e-005,
         -0.000180023, 0.000237008, -3.30553e-005, 0.000146789, 0.000109855, -0.000115067, 6.61662e-005, 1.20826e-005,
         -7.62687e-005, -0.000110559, 4.04032e-005, 0.000106906, -1.51419e-005, -0.000238543, 5.74672e-005,
         -0.000130131, -5.16855e-005, -4.32562e-005, -9.63412e-005, -3.56175e-005, -1.77223e-005, -1.93118e-005,
         0.000229687, -3.16067e-005, -2.20919e-005, -1.99898e-005, -1.4683e-005, -1.9489e-005, -1.72762e-005,
         -1.10419e-005, -4.59736e-006, -8.67266e-006, -2.91743e-006, -1.00485e-005, -5.96899e-006, -1.61104e-006,
         -1.13952e-005, 1.32535e-006, -7.14485e-006, -3.26457e-006, -4.44849e-006, -1.19125e-005, 2.89796e-006,
         1.79553e-007, 2.53053e-006, 6.68043e-006, -6.83551e-006, 1.29854e-005, 1.864e-005, -5.59835e-006,
         2.64279e-006, 6.93782e-006, 1.35425e-005, -3.10092e-006, 3.22757e-006, -9.92498e-006, 1.87512e-006,
         -1.96127e-006, -1.98488e-005, -1.54029e-005, 4.97069e-006, -8.53519e-006, -1.51335e-005, 1.0871e-005,
         -7.16451e-006, -2.61834e-006, 1.21899e-005, 1.45134e-005, -3.87922e-006, 1.33256e-005, 4.12671e-006,
         -7.09979e-006, -8.35808e-006, -5.9728e-006, -3.24368e-005, 8.86731e-006, 1.11805e-005, -2.67717e-005,
         6.13489e-005, 2.50847e-005, -2.98767e-006, -1.95903e-005, 3.61466e-006, 1.76244e-006, 7.55983e-006,
         7.37014e-006, 3.49391e-005, -1.8336e-005, -5.10019e-005, 3.48671e-005, -1.76197e-005, -5.44616e-006,
         4.65892e-005, 4.7985e-006, -5.76139e-005, -1.20458e-006, -2.04706e-005, 4.39729e-005, 1.88132e-005,
         -2.29238e-005, 2.90165e-005, -4.64422e-005, -5.45682e-005, 1.0367e-005, 2.45773e-005, -5.5165e-005,
         3.92159e-005, -3.49013e-005, -6.05899e-006, -2.30711e-005, 4.24814e-005, -1.53109e-005, -5.09578e-005,
         2.37638e-005, 6.09445e-005, -4.20146e-005, 1.49961e-005, -9.15245e-005, 0.000202533, 7.65295e-007,
         1.33235e-005, 4.34992e-005, 6.20971e-005, 7.04149e-005, -5.76104e-005, 2.10277e-005, 6.79014e-005,
         -6.17723e-005, -5.16292e-005, -5.37911e-005, -2.98863e-008, -0.00017341, 3.55328e-005, -2.65718e-005,
         -8.53844e-005, -1.11833e-005, 0.000196693, -0.000147979, 0.000228913, 7.69587e-005, -3.32807e-005,
         8.8069e-005, 4.9677e-005, 0.000163026, 0.000183262, 0.000104657, -0.000204295, -2.81488e-005, 5.99783e-005,
         -0.000112251, 1.15524e-005, -0.000193885, -3.40298e-005, -0.000251797, 9.49819e-005, -0.000112043,
         -9.78818e-005, -3.73876e-005, 0.000169144, 0.000382726, -0.000142058, -0.000206008, 0.00026086, -4.37191e-005,
         -0.000138123, 4.60549e-005, -0.000200657, -0.000100198, 0.000118126, 4.80007e-005, 2.95763e-005, 8.29499e-005,
         0.000197878, -8.84475e-005, 0.000218547, 0.000155006, -0.000101533, -0.000130811, 9.58819e-005, 3.39938e-005,
         -1.1413e-05, -3.32962e-05, -0.000108349, 2.75437e-05, -0.000174813, 0.000151896, -0.000111081, -0.00038839 };

      // Scope the DataAccessor; it causes a crash if it is still in scope when pInvElementOut is destroyed.
      {
         DataAccessor dataAccessor = pInvElementOut->getDataAccessor();
         const unsigned int numElementsToTest = sizeof(pExpectedData) / sizeof(pExpectedData[0]);
         const unsigned int numColsToTest = pDescriptorOut->getColumnCount();
         const unsigned int numRowsToTest = numElementsToTest / numColsToTest;
         for (unsigned int row = 0; row < numRowsToTest; ++row)
         {
            const unsigned int rowOffset = row * numColsToTest;
            issearf(dataAccessor.isValid());
            double* pData = reinterpret_cast<double*>(dataAccessor->getRow());
            issearf(pData != NULL);

            for (unsigned int col = 0; col < numColsToTest; ++col)
            {
               issearf(fabs(pExpectedData[rowOffset + col] - pData[col]) < tolerance);
            }

            dataAccessor->nextRow();
         }
      }

      return success;
   }
};

class SecondMomentMatrixTestSuite : public TestSuiteNewSession
{
public:
   SecondMomentMatrixTestSuite() : TestSuiteNewSession("SecondMomentMatrix")
   {
      addTestCase(new SecondMomentMatrixTestCase);
   }
};

REGISTER_SUITE(SecondMomentMatrixTestSuite)
