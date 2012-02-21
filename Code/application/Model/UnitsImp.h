/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef UNITSIMP_H
#define UNITSIMP_H

#include "Units.h"

#include <string>

class UnitsImp : public Units
{
public:
   UnitsImp();
   ~UnitsImp();

   UnitType getUnitType() const;
   void setUnitType(UnitType myType);
   const std::string& getUnitName() const;
   void setUnitName(const std::string& unitName);
   double getRangeMin() const;
   void setRangeMin(double myRangeMin);
   double getRangeMax() const;
   void setRangeMax(double myRangeMax);
   double getScaleFromStandard() const;
   void setScaleFromStandard(double myScaleFromStandard);

   UnitsImp& operator =(const UnitsImp& units);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   bool operator==(const UnitsImp& rhs) const;
   bool operator!=(const UnitsImp& rhs) const;

private:
   UnitType mUnitType;
   std::string mUnitName;
   double mRangeMin;
   double mRangeMax;
   double mScaleFromStandard;
};

#endif
