/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataVariant.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "NitfChipConverter.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"

#include <vector>

using namespace std;

namespace
{
   struct FindFirstValidIchipB
   {
      bool operator()(const DynamicObject& dynObj)
      {
         return (dynObj.getAttribute(Nitf::TRE::ICHIPB::OP_COL_11).getPointerToValue<double>() != NULL);
      }
   };

   bool solve3LinearEq(double *pCoeffs, double *pXp, double *pYp, double *pOrig)
   {
      VERIFY((pCoeffs != NULL) && (pXp != NULL) && (pYp != NULL) && (pOrig != NULL));

      double epsilon = (pXp[1]-pXp[2])*(pYp[0]-pYp[1]) - (pXp[0]-pXp[1])*(pYp[1]-pYp[2]);
      if (fabs(epsilon) < 1e-6)
      {
         return false;
      }
      pCoeffs[0] = ((pYp[0]-pYp[1])*(pOrig[1]-pOrig[2]) - (pYp[1]-pYp[2])*(pOrig[0]-pOrig[1])) / epsilon;
      pCoeffs[1] = -((pXp[0]-pXp[1])*(pOrig[1]-pOrig[2]) - (pXp[1]-pXp[2])*(pOrig[0]-pOrig[1])) / epsilon;
      double alpha = pYp[1]*pXp[2] - pYp[2]*pXp[1];
      double beta = pYp[0]*pXp[1] - pYp[1]*pXp[0];
      double gamma = pOrig[1]*pXp[2] - pOrig[2]*pXp[1];
      double delta = pOrig[0]*pXp[1] - pOrig[1]*pXp[0];
      double psi = pXp[2]-pXp[1];
      double sigma = pXp[1]-pXp[0];
      double rho = beta*psi - alpha*sigma;
      if (fabs(rho) < 1e-6)
      {
         return false;
      }
      pCoeffs[2] = (beta*gamma - alpha*delta) / rho;
      return true;
   }

   double findOnDisk(const vector<DimensionDescriptor> &activeDims,
      double activeNumber)
   {
      // does not deal with skip factors
      DimensionDescriptor dimDesc;
      VERIFYRV(!activeDims.empty(), 0.0)

      DimensionDescriptor firstDesc = activeDims.front();
      DimensionDescriptor lastDesc = activeDims.back();

      if (activeNumber < firstDesc.getActiveNumber())
      {
         dimDesc = firstDesc;
      }
      else if (activeNumber > lastDesc.getActiveNumber())
      {
         dimDesc = lastDesc;
      }
      else if (activeNumber < activeDims.size())
      {
         dimDesc = activeDims[static_cast<size_t>(activeNumber)];
      }

      VERIFYRV(dimDesc.isValid(), 0.0);

      double offset = activeNumber - dimDesc.getActiveNumber();
      return dimDesc.getOnDiskNumber() + offset;
   }

   double findLoaded(const vector<DimensionDescriptor> &onDiskDims,
      const vector<DimensionDescriptor> &activeDims,
      double onDiskNumber)
   {
      // does not deal with skip factors
      VERIFYRV(!onDiskDims.empty() && !activeDims.empty(), 0.0);
      DimensionDescriptor dimDesc;

      DimensionDescriptor firstDesc = activeDims.front();
      DimensionDescriptor lastDesc = activeDims.back();

      if (onDiskNumber < firstDesc.getOnDiskNumber())
      {
         dimDesc = firstDesc;
      }
      else if (onDiskNumber > lastDesc.getOnDiskNumber())
      {
         dimDesc = lastDesc;
      }
      else if (onDiskNumber < onDiskDims.size())
      {
         dimDesc = onDiskDims[static_cast<size_t>(onDiskNumber)];
      }

      VERIFYRV(dimDesc.isValid(), 0.0);

      double offset = onDiskNumber - dimDesc.getOnDiskNumber();
      return dimDesc.getActiveNumber() + offset;
   }

}
Nitf::ChipConverter::ChipConverter(const RasterDataDescriptor &descriptor) : 
   mDescriptor(descriptor)
{
   mChipCoefficients.push_back(1.0);
   mChipCoefficients.push_back(0.0);
   mChipCoefficients.push_back(0.0);
   mChipCoefficients.push_back(0.0);
   mChipCoefficients.push_back(1.0);
   mChipCoefficients.push_back(0.0);

   /* Try to get chipping info from ICHIPB
   1. Get ICHIPB TRE
   2. Get first three corners
   3. If first three corners are colinear, trade one for the fourth
   4. Create 2D 1st order fit from chip coords to pOrig coords
   */
   double pXp[4];
   double pYp[4];
   double x[4];
   double y[4];

   // look for a string that includes
   // one of the items that we know is in the ICHIPB TRE.  Get its pPrefix so we can prepend it
   // to the names we know to get the full names of the keys we want

   const DynamicObject* pMetadata = mDescriptor.getMetadata();
   if (pMetadata == NULL)
   {
      throw string("Could not find metadata.");
   }

   const DynamicObject* pNitfMetadata = pMetadata->getAttribute(Nitf::NITF_METADATA).getPointerToValue<DynamicObject>();

   if (pNitfMetadata == NULL)
   {
      throw string("Could not find NITF metadata.");
   }
   const DynamicObject* pIchipB = Nitf::getTagHandle<FindFirstValidIchipB>(*pNitfMetadata, "ICHIPB");
   if (pIchipB != NULL)
   {
      // Get coordinates from the ICHIPB TRE
      try
      {
         pXp[0] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::OP_COL_11));
         pXp[1] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::OP_COL_12));
         pXp[2] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::OP_COL_21));
         pXp[3] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::OP_COL_22));
         pYp[0] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::OP_ROW_11));
         pYp[1] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::OP_ROW_12));
         pYp[2] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::OP_ROW_21));
         pYp[3] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::OP_ROW_22));
         x[0] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::FI_COL_11));
         x[1] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::FI_COL_12));
         x[2] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::FI_COL_21));
         x[3] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::FI_COL_22));
         y[0] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::FI_ROW_11));
         y[1] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::FI_ROW_12));
         y[2] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::FI_ROW_21));
         y[3] = dv_cast<double>(pIchipB->getAttribute(TRE::ICHIPB::FI_ROW_22));
         
         // We are trying to create two equations of the form:
         // FI_x = a*OP_x + b*OP_y + c and
         // FI_y = d*OP_x + e*OP_y + f
         // In order to solve for a-f, we need 6 equations in these 6 unknowns.
         // We create 3 for a-c and 3 for d-f by plugging in the known OP/FI
         // coordinates in the above equations. We try first using 11, 12 and 21.
         // If those fail due to a divide by 0 (indicating that all 3 are in a line)
         // then we try 12, 21 and 22. If those fail then all 4 points are in a line
         // and we quit, indicating that the ICHIPB is bad.

         // Try 11, 12, and 21
         bool success = solve3LinearEq(&mChipCoefficients[0], pXp, pYp, x) &&
                        solve3LinearEq(&mChipCoefficients[3], pXp, pYp, y);
         if (!success)
         {
            // Try 12, 21 and 22
            success = solve3LinearEq(&mChipCoefficients[0], &pXp[1], &pYp[1], &x[1]);
            success = success && solve3LinearEq(&mChipCoefficients[3], &pXp[1], &pYp[1], &y[1]);
         }
         if (!success)
         {
            throw string("ICHIPB TRE is invalid!");
         }
      }
      catch (const bad_cast&)
      {
         throw string("Not all ICHIPB fields are present!");
      }
   }

}

Nitf::ChipConverter::ChipConverter(const RasterDataDescriptor &descriptor, const vector<double> &coefficients) :
   mChipCoefficients(coefficients), 
   mDescriptor(descriptor)
{
   if (mChipCoefficients.size() != 6)
   {
      throw string("Not all coefficients are present");
   }
}


Nitf::ChipConverter::~ChipConverter()
{
}

LocationType Nitf::ChipConverter::originalToActive(LocationType original) const
{
   // add the pixel offset created by chipping
   LocationType onDiskPixel;
   onDiskPixel.mX = ((mChipCoefficients[4]*original.mX - mChipCoefficients[1]*original.mY) - 
      (mChipCoefficients[2]*mChipCoefficients[4] - mChipCoefficients[1]*mChipCoefficients[5])) / 
      (mChipCoefficients[0]*mChipCoefficients[4] - mChipCoefficients[1]*mChipCoefficients[3]);
   onDiskPixel.mY = ((mChipCoefficients[3]*original.mX - mChipCoefficients[0]*original.mY) - 
      (mChipCoefficients[2]*mChipCoefficients[3] - mChipCoefficients[0]*mChipCoefficients[5])) / 
      (mChipCoefficients[1]*mChipCoefficients[3] - mChipCoefficients[0]*mChipCoefficients[4]);

   const RasterFileDescriptor* pFd = static_cast<const RasterFileDescriptor*>(
      mDescriptor.getFileDescriptor());
   VERIFYRV(pFd != NULL, LocationType(0, 0));

   LocationType active;
   active.mY = findLoaded(pFd->getRows(), mDescriptor.getRows(), onDiskPixel.mY);
   active.mX = findLoaded(pFd->getColumns(), mDescriptor.getColumns(), onDiskPixel.mX);

   return active;
}

LocationType Nitf::ChipConverter::activeToOriginal(LocationType active) const
{
   LocationType onDisk;
   onDisk.mY = findOnDisk(mDescriptor.getRows(), active.mY);
   onDisk.mX = findOnDisk(mDescriptor.getColumns(), active.mX);

   // add the pixel offset created by chipping
   LocationType original;
   original.mX = onDisk.mX*mChipCoefficients[0] + onDisk.mY*mChipCoefficients[1] + mChipCoefficients[2];
   original.mY = onDisk.mX*mChipCoefficients[3] + onDisk.mY*mChipCoefficients[4] + mChipCoefficients[5];
   return original;
}

const vector<double> &Nitf::ChipConverter::getChipCoefficients() const
{
   return mChipCoefficients;
}
