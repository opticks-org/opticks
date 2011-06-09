/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DynamicObject.h"
#include "SpecialMetadata.h"
#include "WavelengthsImp.h"

WavelengthsImp::WavelengthsImp() :
   mUnits(MICRONS)
{}

WavelengthsImp::~WavelengthsImp()
{}

bool WavelengthsImp::setStartValues(const std::vector<double>& startValues, WavelengthUnitsType valueUnits)
{
   // Check for error conditions
   if (startValues.empty() == false)
   {
      // Invalid units
      if (valueUnits.isValid() == false)
      {
         return false;
      }

      // Different number of wavelengths
      if ((mCenterValues.empty() == false && mCenterValues.size() != startValues.size()) ||
         (mEndValues.empty() == false && mEndValues.size() != startValues.size()))
      {
         return false;
      }

      // All zero or negative values
      bool invalidValues = true;
      for (std::vector<double>::const_iterator iter = startValues.begin(); iter != startValues.end(); ++iter)
      {
         if (*iter > 0.0)
         {
            invalidValues = false;
            break;
         }
      }

      if (invalidValues == true)
      {
         return false;
      }
   }

   // Set the start values
   if (startValues.empty() == true)
   {
      mStartValues.clear();
   }
   else if (valueUnits != mUnits)
   {
      // Convert the values to the member units
      mStartValues.resize(startValues.size());
      for (std::vector<double>::size_type i = 0; i < startValues.size(); ++i)
      {
         mStartValues[i] = Wavelengths::convertValue(startValues[i], valueUnits, mUnits);
      }
   }
   else
   {
      mStartValues = startValues;
   }

   return true;
}

const std::vector<double>& WavelengthsImp::getStartValues() const
{
   return mStartValues;
}

bool WavelengthsImp::hasStartValues() const
{
   return !(mStartValues.empty());
}

bool WavelengthsImp::setCenterValues(const std::vector<double>& centerValues, WavelengthUnitsType valueUnits)
{
   // Check for error conditions
   if (centerValues.empty() == false)
   {
      // Invalid units
      if (valueUnits.isValid() == false)
      {
         return false;
      }

      // Different number of wavelengths
      if ((mStartValues.empty() == false && mStartValues.size() != centerValues.size()) ||
         (mEndValues.empty() == false && mEndValues.size() != centerValues.size()))
      {
         return false;
      }

      // All zero or negative values
      bool invalidValues = true;
      for (std::vector<double>::const_iterator iter = centerValues.begin(); iter != centerValues.end(); ++iter)
      {
         if (*iter > 0.0)
         {
            invalidValues = false;
            break;
         }
      }

      if (invalidValues == true)
      {
         return false;
      }
   }

   // Set the center values
   if (centerValues.empty() == true)
   {
      mCenterValues.clear();
   }
   else if (valueUnits != mUnits)
   {
      // Convert the values to the member units
      mCenterValues.resize(centerValues.size());
      for (std::vector<double>::size_type i = 0; i < centerValues.size(); ++i)
      {
         mCenterValues[i] = Wavelengths::convertValue(centerValues[i], valueUnits, mUnits);
      }
   }
   else
   {
      mCenterValues = centerValues;
   }

   return true;
}

const std::vector<double>& WavelengthsImp::getCenterValues() const
{
   return mCenterValues;
}

bool WavelengthsImp::hasCenterValues() const
{
   return !(mCenterValues.empty());
}

bool WavelengthsImp::setEndValues(const std::vector<double>& endValues, WavelengthUnitsType valueUnits)
{
   // Check for error conditions
   if (endValues.empty() == false)
   {
      // Invalid units
      if (valueUnits.isValid() == false)
      {
         return false;
      }

      // Different number of wavelengths
      if ((mStartValues.empty() == false && mStartValues.size() != endValues.size()) ||
         (mCenterValues.empty() == false && mCenterValues.size() != endValues.size()))
      {
         return false;
      }

      // All zero or negative values
      bool invalidValues = true;
      for (std::vector<double>::const_iterator iter = endValues.begin(); iter != endValues.end(); ++iter)
      {
         if (*iter > 0.0)
         {
            invalidValues = false;
            break;
         }
      }

      if (invalidValues == true)
      {
         return false;
      }
   }

   // Set the end values
   if (endValues.empty() == true)
   {
      mEndValues.clear();
   }
   else if (valueUnits != mUnits)
   {
      // Convert the values to the member units
      mEndValues.resize(endValues.size());
      for (std::vector<double>::size_type i = 0; i < endValues.size(); ++i)
      {
         mEndValues[i] = Wavelengths::convertValue(endValues[i], valueUnits, mUnits);
      }
   }
   else
   {
      mEndValues = endValues;
   }

   return true;
}

const std::vector<double>& WavelengthsImp::getEndValues() const
{
   return mEndValues;
}

bool WavelengthsImp::hasEndValues() const
{
   return !(mEndValues.empty());
}

void WavelengthsImp::setUnits(WavelengthUnitsType units, bool convertValues)
{
   if (units.isValid() == false)
   {
      return;
   }

   if (units != mUnits)
   {
      // Convert the values if necessary
      if (convertValues == true)
      {
         for (std::vector<double>::size_type i = 0; i < mStartValues.size(); ++i)
         {
            mStartValues[i] = Wavelengths::convertValue(mStartValues[i], mUnits, units);
         }

         for (std::vector<double>::size_type i = 0; i < mCenterValues.size(); ++i)
         {
            mCenterValues[i] = Wavelengths::convertValue(mCenterValues[i], mUnits, units);
         }

         for (std::vector<double>::size_type i = 0; i < mEndValues.size(); ++i)
         {
            mEndValues[i] = Wavelengths::convertValue(mEndValues[i], mUnits, units);
         }
      }

      // Set the units
      mUnits = units;
   }
}

WavelengthUnitsType WavelengthsImp::getUnits() const
{
   return mUnits;
}

void WavelengthsImp::clear()
{
   mStartValues.clear();
   mCenterValues.clear();
   mEndValues.clear();
   mUnits = MICRONS;
}

bool WavelengthsImp::isEmpty() const
{
   if ((hasStartValues() == false) && (hasCenterValues() == false) && (hasEndValues() == false))
   {
      return true;
   }

   return false;
}

unsigned int WavelengthsImp::getNumWavelengths() const
{
   if (hasStartValues() == true)
   {
      return static_cast<unsigned int>(mStartValues.size());
   }
   else if (hasCenterValues() == true)
   {
      return static_cast<unsigned int>(mCenterValues.size());
   }
   else if (hasEndValues() == true)
   {
      return static_cast<unsigned int>(mEndValues.size());
   }

   return 0;
}

void WavelengthsImp::scaleValues(double dScaleFactor)
{
   if (dScaleFactor <= 0.0)
   {
      return;
   }

   // Apply the scale factor to all wavelength values
   for (std::vector<double>::size_type i = 0; i < mStartValues.size(); ++i)
   {
      mStartValues[i] *= dScaleFactor;
   }

   for (std::vector<double>::size_type i = 0; i < mCenterValues.size(); ++i)
   {
      mCenterValues[i] *= dScaleFactor;
   }

   for (std::vector<double>::size_type i = 0; i < mEndValues.size(); ++i)
   {
      mEndValues[i] *= dScaleFactor;
   }
}

void WavelengthsImp::calculateFwhm(double dConstant)
{
   if (mCenterValues.size() < 2)
   {
      return;
   }

   mStartValues.clear();
   mEndValues.clear();

   // Calculate the FWHM values
   std::vector<double>::size_type index1 = 0;
   std::vector<double>::size_type index2 = 1;
   for (index1 = 0, index2 = 1; index1 < (mCenterValues.size() - 1), index2 < mCenterValues.size(); ++index1, ++index2)
   {
      // Start value
      double dStart = mCenterValues[index1] - ((mCenterValues[index2] - mCenterValues[index1]) * dConstant) / 2.0;
      if (dStart < 0.0)
      {
         dStart = 0.0;
      }

      mStartValues.push_back(dStart);

      // End value
      double dEnd = mCenterValues[index1] + ((mCenterValues[index2] - mCenterValues[index1]) * dConstant) / 2.0;
      if (dEnd < 0.0)
      {
         dEnd = 0.0;
      }

      mEndValues.push_back(dEnd);
   }

   // Calculate the FWHM values for the last wavelength
   std::vector<double>::size_type index = mCenterValues.size() - 1;

   double dStart = mCenterValues[index] - ((mCenterValues[index] - mCenterValues[index - 1]) * dConstant) / 2.0;
   if (dStart < 0.0)
   {
      dStart = 0.0;
   }

   mStartValues.push_back(dStart);

   double dEnd = mCenterValues[index] + ((mCenterValues[index] - mCenterValues[index - 1]) * dConstant) / 2.0;
   if (dEnd < 0.0)
   {
      dEnd = 0.0;
   }

   mEndValues.push_back(dEnd);
}

std::vector<double> WavelengthsImp::getFwhm()
{
   if (!hasStartValues() || !hasEndValues())
   {
      calculateFwhm();
   }

   std::vector<double> fwhm;
   for (std::vector<double>::size_type i = 0; i < mStartValues.size() && i < mEndValues.size(); ++i)
   {
      fwhm.push_back(mEndValues[i] - mStartValues[i]);
   }

   return fwhm;
}

bool WavelengthsImp::initializeFromWavelengths(const Wavelengths* pWavelengths)
{
   if (pWavelengths == NULL)
   {
      return false;
   }

   mStartValues = pWavelengths->getStartValues();
   mCenterValues = pWavelengths->getCenterValues();
   mEndValues = pWavelengths->getEndValues();
   mUnits = pWavelengths->getUnits();

   return true;
}

bool WavelengthsImp::initializeFromDynamicObject(const DynamicObject* pData, bool convertToDisplayUnits)
{
   if (pData == NULL)
   {
      return false;
   }

   clear();

   // Get the band metadata
   const DynamicObject* pBand =
      dv_cast<DynamicObject>(&pData->getAttributeByPath(SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME));
   if (pBand == NULL)
   {
      return true;
   }

   // Units
   if (convertToDisplayUnits == true)
   {
      const WavelengthUnitsType* pUnits =
         dv_cast<WavelengthUnitsType>(&pBand->getAttribute(WAVELENGTH_DISPLAY_UNITS_METADATA_NAME));
      if (pUnits != NULL)
      {
         // Set the units first to potentially avoid multiple conversions
         setUnits(*pUnits);
      }
   }

   // Start values
   const std::vector<double>* pStartValues =
      dv_cast<std::vector<double> >(&pBand->getAttribute(START_WAVELENGTHS_METADATA_NAME));
   if (pStartValues != NULL)
   {
      setStartValues(*pStartValues, MICRONS);
   }

   // Center values
   const std::vector<double>* pCenterValues =
      dv_cast<std::vector<double> >(&pBand->getAttribute(CENTER_WAVELENGTHS_METADATA_NAME));
   if (pCenterValues != NULL)
   {
      setCenterValues(*pCenterValues, MICRONS);
   }

   // End values
   const std::vector<double>* pEndValues =
      dv_cast<std::vector<double> >(&pBand->getAttribute(END_WAVELENGTHS_METADATA_NAME));
   if (pEndValues != NULL)
   {
      setEndValues(*pEndValues, MICRONS);
   }

   return true;
}

bool WavelengthsImp::applyToDynamicObject(DynamicObject* pData)
{
   if (pData == NULL)
   {
      return false;
   }

   // Convert the units to microns to set into the dynamic object
   WavelengthUnitsType currentUnits = mUnits;
   setUnits(MICRONS);

   // Set the values in the dynamic object
   bool success = true;
   if (hasStartValues() == true)
   {
      success = pData->setAttributeByPath(START_WAVELENGTHS_METADATA_PATH, mStartValues);
   }
   else
   {
      pData->removeAttributeByPath(START_WAVELENGTHS_METADATA_PATH);
   }

   if (success == true)
   {
      if (hasCenterValues() == true)
      {
         success = pData->setAttributeByPath(CENTER_WAVELENGTHS_METADATA_PATH, mCenterValues);
      }
      else
      {
         pData->removeAttributeByPath(CENTER_WAVELENGTHS_METADATA_PATH);
      }
   }

   if (success == true)
   {
      if (hasEndValues() == true)
      {
         success = pData->setAttributeByPath(END_WAVELENGTHS_METADATA_PATH, mEndValues);
      }
      else
      {
         pData->removeAttributeByPath(END_WAVELENGTHS_METADATA_PATH);
      }
   }

   // Set the current display units
   if (success == true)
   {
      if (isEmpty() == false)
      {
         success = pData->setAttributeByPath(WAVELENGTH_DISPLAY_UNITS_METADATA_PATH, currentUnits);
      }
      else
      {
         pData->removeAttributeByPath(WAVELENGTH_DISPLAY_UNITS_METADATA_PATH);

         // Remove the special metadata attributes if necessary
         const DynamicObject* pBand =
            dv_cast<DynamicObject>(&pData->getAttributeByPath(SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME));
         if ((pBand != NULL) && (pBand->getNumAttributes() == 0))
         {
            pData->removeAttributeByPath(SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME);
         }

         const DynamicObject* pSpecial = dv_cast<DynamicObject>(&pData->getAttribute(SPECIAL_METADATA_NAME));
         if ((pSpecial != NULL) && (pSpecial->getNumAttributes() == 0))
         {
            pData->removeAttribute(SPECIAL_METADATA_NAME);
         }
      }
   }

   // Restore the current display units
   setUnits(currentUnits);

   return success;
}
