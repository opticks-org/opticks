/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELENGTHSIMP_H
#define WAVELENGTHSIMP_H

#include "TypesFile.h"
#include "Wavelengths.h"

#include <vector>

class DynamicObject;

class WavelengthsImp : public Wavelengths
{
public:
   WavelengthsImp();
   virtual ~WavelengthsImp();

   bool setStartValues(const std::vector<double>& startValues, WavelengthUnitsType valueUnits);
   const std::vector<double>& getStartValues() const;
   bool hasStartValues() const;

   bool setCenterValues(const std::vector<double>& centerValues, WavelengthUnitsType valueUnits);
   const std::vector<double>& getCenterValues() const;
   bool hasCenterValues() const;

   bool setEndValues(const std::vector<double>& endValues, WavelengthUnitsType valueUnits);
   const std::vector<double>& getEndValues() const;
   bool hasEndValues() const;

   void setUnits(WavelengthUnitsType units, bool convertValues = true);
   WavelengthUnitsType getUnits() const;

   void clear();
   bool isEmpty() const;
   unsigned int getNumWavelengths() const;

   void scaleValues(double dScaleFactor);
   void calculateFwhm(double dConstant = 1.0);
   std::vector<double> getFwhm();

   bool initializeFromWavelengths(const Wavelengths* pWavelengths);
   bool initializeFromDynamicObject(const DynamicObject* pData, bool convertToDisplayUnits);
   bool applyToDynamicObject(DynamicObject* pData);

private:
   std::vector<double> mStartValues;
   std::vector<double> mCenterValues;
   std::vector<double> mEndValues;
   WavelengthUnitsType mUnits;
};

#endif
