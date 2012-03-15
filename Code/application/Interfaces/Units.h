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
#include "Subject.h"
#include "TypesFile.h"

#include <string>

/**
 * Provides unit and range information for data objects
 *
 * The units interface serves as a common mechanism for maintaining
 * unit and range information about other data structures.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setUnitName(), setUnitType(),
 *    setRangeMin(), setRangeMax(), setScaleFromStandard(), and setUnits().
 *  - All notifications documented in Subject.
 */
class Units : public Subject, public Serializable
{
public:
   /**
    *  Emitted when the unit name changes with boost::any<std::string>
    *  containing the new name.
    *
    *  @see     setUnitName()
    */
   SIGNAL_METHOD(Units, Renamed)

   /**
    *  Emitted when the unit type changes with
    *  boost::any<\link ::UnitType UnitType\endlink> containing the new type.
    *
    *  @see     setUnitType()
    */
   SIGNAL_METHOD(Units, TypeChanged)

   /**
    *  Emitted when the minimum or maximum range value changes.
    *
    *  No value is associated with this signal.
    *
    *  @see     setRangeMin(), setRangeMax()
    */
   SIGNAL_METHOD(Units, RangeChanged)

   /**
    *  Emitted when the scale value changes with boost::any<double> containing
    *  the new scale value.
    *
    *  @see     setScaleFromStandard()
    */
   SIGNAL_METHOD(Units, ScaleChanged)

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
    *  @notify  This method notifies signalRenamed() if the given unit name is
    *           different than the current unit name.
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
    *
    *  @notify  This method notifies signalTypeChanged() if the given unit type
    *           is different than the current unit type.
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
    *
    *  @notify  This method notifies signalRangeChanged() if the given range
    *           minimum is different than the current range minimum.
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
    *
    *  @notify  This method notifies signalRangeChanged() if the given range
    *           maximum is different than the current range maximum.
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
    *
    *  @notify  This method notifies signalScaleChanged() if the given scale
    *           value is different than the current scale value.
    */
   virtual void setScaleFromStandard(double myScaleFromStandard) = 0;

   /**
    *  Sets all values in this Units object to those of another Units object.
    *
    *  @param   pUnits
    *           The Units object from which to set all data values in this Units
    *           object.  This method does nothing if \c NULL is passed in.
    *
    *  @notify  This method notifies the following signals if the respective
    *           values in this Units object change:
    *           - signalRenamed()
    *           - signalTypeChanged()
    *           - signalRangeChanged()
    *           - signalScaleChanged()
    */
   virtual void setUnits(const Units* pUnits) = 0;

   /**
    *  Compares all values in this Units object with those of another Units
    *  object.
    *
    *  @param   pUnits
    *           The Units object with which to compare values in this Units
    *           object.  This method does nothing and returns \c false if
    *           \c NULL is passed in.
    *
    *  @return  Returns \c true if all values in \em pUnits are the same as the
    *           values in this Units object; otherwise returns \c false.
    */
   virtual bool compare(const Units* pUnits) const = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~Units() {}
};

#endif
