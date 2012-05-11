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

#include "SerializableImp.h"
#include "SubjectImp.h"
#include "TypesFile.h"

#include <string>

class Units;

class UnitsImp : public SubjectImp
{
public:
   UnitsImp();
   virtual ~UnitsImp();

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

   void setUnits(const Units* pUnits);
   bool compare(const Units* pUnits) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   UnitType mUnitType;
   std::string mUnitName;
   double mRangeMin;
   double mRangeMax;
   double mScaleFromStandard;
};

#define UNITSADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES \
   SERIALIZABLEADAPTEREXTENSION_CLASSES

#define UNITSADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   SERIALIZABLEADAPTER_METHODS(impClass) \
   const std::string& getUnitName() const \
   { \
      return impClass::getUnitName(); \
   } \
   void setUnitName(const std::string& unitName) \
   { \
      impClass::setUnitName(unitName); \
   } \
   UnitType getUnitType() const \
   { \
      return impClass::getUnitType(); \
   } \
   void setUnitType(UnitType myType) \
   { \
      impClass::setUnitType(myType); \
   } \
   double getRangeMin() const \
   { \
      return impClass::getRangeMin(); \
   } \
   void setRangeMin(double myRangeMin) \
   { \
      impClass::setRangeMin(myRangeMin); \
   } \
   double getRangeMax() const \
   { \
      return impClass::getRangeMax(); \
   } \
   void setRangeMax(double myRangeMax) \
   { \
      impClass::setRangeMax(myRangeMax); \
   } \
   double getScaleFromStandard() const \
   { \
      return impClass::getScaleFromStandard(); \
   } \
   void setScaleFromStandard(double myScaleFromStandard) \
   { \
      impClass::setScaleFromStandard(myScaleFromStandard); \
   } \
   void setUnits(const Units* pUnits) \
   { \
      impClass::setUnits(pUnits); \
   } \
   bool compare(const Units* pUnits) const \
   { \
      return impClass::compare(pUnits); \
   }

#endif
