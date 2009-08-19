/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef _UNITS
#define _UNITS

#include "Serializable.h"
#include "TypesFile.h"

#include <string>

/**
 * Provides unit and range information for data objects
 *
 * The units interface serves as a common mechanism for maintaining
 * unit and range information about other data structures.
 *
 * @see      Serializable
 */
class Units : public Serializable
{
public:
   /**
    *  Get the name of the current unit.
    *
    *  @return  The name of the unit.
    *
    *  @see     Units::setUnitName
    */
   virtual const std::string& getUnitName() const = 0;

   /**
    *  Sets the name for the current unit.
    *
    *  @param   unitName
    *           The new name for the current unit.
    *
    *  @see     Units::getUnitName
    */
   virtual void setUnitName(const std::string& unitName) = 0;

   /**
    *  Get the type of the units.
    *
    *  @return  An enumeration code indicating the type of the units.
    */
   virtual UnitType getUnitType() const = 0;

   /**
    *  Set the type of the units.
    *
    *  @param   myType
    *           An enumeration code indicating the type of the units.
    */
   virtual void setUnitType(UnitType myType) = 0;

   /**
    *  Get the minimum range of the units.
    *
    *  @return  The minimum range of the units.
    */
   virtual double getRangeMin() const = 0;

   /**
    *  Set the minimum range of the units.
    *
    *  @param   myRangeMin
    *           The minimum range of the units.
    */
   virtual void setRangeMin(double myRangeMin) = 0;

   /**
    *  Get the maximum range of the units.
    *
    *  @return  The maximum range of the units.
    */
   virtual double getRangeMax() const = 0;

   /**
    *  Set the maximum range of the units.
    *
    *  @param   myRangeMax
    *           The maximum range of the units.
    */
   virtual void setRangeMax(double myRangeMax) = 0;

   /**
    *  Get the quantity scaled from the standard units.
    *
    *  @return  The quantity scaled from the standard units.
    */
   virtual double getScaleFromStandard() const = 0;

   /**
    *  Set the quantity scaled from the standard units.
    *
    *  Example:
    *  For distance, the standard units will be meters. To convert distance
    *  values of meter to kilometer, call setScaleFromStandard(1e-3);
    *
    *  @param   myScaleFromStandard
    *           The quantity scaled from the standard units.
    */
   virtual void setScaleFromStandard(double myScaleFromStandard) = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~Units() {}
};

#endif
