/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELENGTHS_H
#define WAVELENGTHS_H

#include "DynamicObject.h"
#include "SpecialMetadata.h"
#include "TypesFile.h"

#include <string>
#include <vector>

/**
 *  A convenience class for working with wavelength values.
 *
 *  This class provides a common object for working with wavelength values.  It
 *  contains start, center, and end wavelength values and the units associated
 *  with the values.  The values and units can be set directly or initialized
 *  from special metadata attributes in a DynamicObject or from another
 *  Wavelengths object.  The Wavelengths object can be used as an input or
 *  output plug-in arg.  Additional convenience static methods are provided to
 *  convert wavelength values between units and to query the number of
 *  wavelength values in a DynamicObject.
 *
 *  Wavelength objects should be created with an object factory resource as
 *  follows:
 *  @code
 *  FactoryResource<Wavelengths> pWavelengths;
 *  @endcode
 *
 *  When the Wavelengths object is created, the start, center, and end value
 *  vectors are empty (i.e. isEmpty() returns \c true), and the units are
 *  set to WavelengthUnitsType::MICRONS.
 *
 *  @see        \ref specialmetadata
 */
class Wavelengths
{
public:
   /**
    *  Sets starting wavelength values.
    *
    *  The number of starting values must equal the number of center and ending
    *  values if they are present.
    *
    *  @param   startValues
    *           The starting wavelength values.
    *  @param   valueUnits
    *           The units of the starting wavelength values.  If the units are
    *           not the same units returned by getUnits(), the values are
    *           automatically converted before they are set.
    *
    *  @return  Returns \c true if the start values are successfully set;
    *           returns \c false if any of the following occur:
    *           - \em startValues is not empty and \em valueUnits is invalid
    *           - \em startValues is not empty and the size is different than
    *             the number of center and ending values (if present)
    *           - \em startValues contains all zero or negative values
    *
    *  @see     setCenterValues(), setEndValues(), convertValue()
    */
   virtual bool setStartValues(const std::vector<double>& startValues, WavelengthUnitsType valueUnits) = 0;

   /**
    *  Returns the starting wavelength values.
    *
    *  @return  Returns the starting wavelength values.  The values are in the
    *           units as returned by getUnits().
    *
    *  @see     getCenterValues(), getEndValues()
    */
   virtual const std::vector<double>& getStartValues() const = 0;

   /**
    *  Queries whether starting wavelength values are present.
    *
    *  This is a convenience method that is equivalent to calling
    *  !getStartValues().empty().
    *
    *  @return  Returns \c true if starting wavelengths are present; otherwise
    *           returns \c false.
    *
    *  @see     getStartValues(), isEmpty()
    */
   virtual bool hasStartValues() const = 0;

   /**
    *  Sets center wavelength values.
    *
    *  The number of center values must equal the number of starting and ending
    *  values if they are present.
    *
    *  @param   centerValues
    *           The center wavelength values.
    *  @param   valueUnits
    *           The units of the center wavelength values.  If the units are
    *           not the same units returned by getUnits(), the values are
    *           automatically converted before they are set.
    *
    *  @return  Returns \c true if the center values are successfully set;
    *           returns \c false if any of the following occur:
    *           - \em centerValues is not empty and \em valueUnits is invalid
    *           - \em centerValues is not empty and the size is different than
    *             the number of starting and ending values (if present)
    *           - \em centerValues contains all zero or negative values
    *
    *  @see     setStartValues(), setEndValues(), convertValue()
    */
   virtual bool setCenterValues(const std::vector<double>& centerValues, WavelengthUnitsType valueUnits) = 0;

   /**
    *  Returns the center wavelength values.
    *
    *  @return  Returns the center wavelength values.  The values are in the
    *           units as returned by getUnits().
    *
    *  @see     getStartValues(), getEndValues()
    */
   virtual const std::vector<double>& getCenterValues() const = 0;

   /**
    *  Queries whether center wavelength values are present.
    *
    *  This is a convenience method that is equivalent to calling
    *  !getCenterValues().empty().
    *
    *  @return  Returns \c true if center wavelengths are present; otherwise
    *           returns \c false.
    *
    *  @see     getCenterValues(), isEmpty()
    */
   virtual bool hasCenterValues() const = 0;

   /**
    *  Sets ending wavelength values.
    *
    *  The number of ending values must equal the number of starting and center
    *  values if they are present.
    *
    *  @param   endValues
    *           The ending wavelength values.
    *  @param   valueUnits
    *           The units of the ending wavelength values.  If the units are
    *           not the same units returned by getUnits(), the values are
    *           automatically converted before they are set.
    *
    *  @return  Returns \c true if the ending values are successfully set;
    *           returns \c false if any of the following occur:
    *           - \em endValues is not empty and \em valueUnits is invalid
    *           - \em endValues is not empty and the size is different than
    *             the number of starting and center values (if present)
    *           - \em endValues contains all zero or negative values
    *
    *  @see     setStartValues(), setCenterValues(), convertValue()
    */
   virtual bool setEndValues(const std::vector<double>& endValues, WavelengthUnitsType valueUnits) = 0;

   /**
    *  Returns the ending wavelength values.
    *
    *  @return  Returns the ending wavelength values.  The values are in the
    *           units as returned by getUnits().
    *
    *  @see     getStartValues(), getCenterValues()
    */
   virtual const std::vector<double>& getEndValues() const = 0;

   /**
    *  Queries whether ending wavelength values are present.
    *
    *  This is a convenience method that is equivalent to calling
    *  !getEndValues().empty().
    *
    *  @return  Returns \c true if ending wavelengths are present; otherwise
    *           returns \c false.
    *
    *  @see     getEndValues(), isEmpty()
    */
   virtual bool hasEndValues() const = 0;

   /**
    *  Assigns units to the wavelength values.
    *
    *  @param   units
    *           The new wavelength units.
    *  @param   convertValues
    *           If set to \c true, all wavelength values are converted to the
    *           new units.  If set to \c false, no wavelength values are
    *           modified.
    *
    *  @see     getUnits(), convertValue()
    */
   virtual void setUnits(WavelengthUnitsType units, bool convertValues = true) = 0;

   /**
    *  Returns the units of the start, center, and end wavelength values.
    *
    *  @return  The wavelength units.
    *
    *  @see     getEndValues(), isEmpty()
    */
   virtual WavelengthUnitsType getUnits() const = 0;

   /**
    *  Clears the wavelength values and units.
    *
    *  This method clears the start, center, and end wavelength values, and
    *  sets the units to WavelengthUnitsType::MICRONS.  After calling this
    *  method, isEmpty() returns \c true.
    */
   virtual void clear() = 0;

   /**
    *  Queries whether start, center, or end wavelength values are present.
    *
    *  @return  Returns \c true if start, center, and end wavelengths are
    *           not present (i.e. all wavelength vectors are empty); otherwise
    *           returns \c false.
    *
    *  @see     getNumWavelengths()
    */
   virtual bool isEmpty() const = 0;

   /**
    *  Returns the number of wavelength values.
    *
    *  @return  Returns the number of wavelengths values if start, center, or
    *           end wavelength values are present; otherwise returns 0.
    *
    *  @see     isEmpty(), getNumWavelengths(const DynamicObject*)
    */
   virtual unsigned int getNumWavelengths() const = 0;

   /**
    *  Adjusts all wavelength values by a given scale factor.
    *
    *  This method multiplies the start, center, and end wavelengths by the
    *  given scale factor.  The units are not changed.
    *
    *  @param   dScaleFactor
    *           The scale factor by which to adjust the wavelength values.
    *           This method does nothing if the scale factor is zero or
    *           negative.
    *
    *  @see     calculateFwhm()
    */
   virtual void scaleValues(double dScaleFactor) = 0;

   /**
    *  Populates the start and end wavelengths by calculating the full width
    *  at half maximum (FWHM) of the center wavelength values.
    *
    *  This method does nothing if center wavelength values are not set.
    *
    *  @param   dConstant
    *           An optional constant that is multiplied by the wavelength width
    *           when calculating the FWHM.
    *
    *  @see     getFwhm(), scaleValues()
    */
   virtual void calculateFwhm(double dConstant = 1.0) = 0;

   /**
    *  Returns the calculated full width at half maximum (FWHM) values.
    *
    *  If start or end wavelengths are not set, this method calculates the FWHM
    *  and returns the values for each wavelength.
    *
    *  @return  The FWHM for each center wavelength value.
    *
    *  @see     calculateFwhm(), scaleValues()
    */
   virtual std::vector<double> getFwhm() = 0;

   /**
    *  Sets the start, center, and end wavelength values and units to those
    *  of another Wavelengths object.
    *
    *  @param   pWavelengths
    *           The Wavelengths object from which to set the wavelength values
    *           and units.
    *
    *  @return  Returns \c true if the start, center, and end values and units
    *           are successfully set from the Wavelengths object; otherwise
    *           returns \c false.
    *
    *  @see     initializeFromDynamicObject()
    */
   virtual bool initializeFromWavelengths(const Wavelengths* pWavelengths) = 0;

   /**
    *  Sets the start, center, and end wavelength values and units based on
    *  special metadata attributes.
    *
    *  @param   pData
    *           The DynamicObject containing the special metadata attributes
    *           from which to set the wavelength values and units.
    *  @param   convertToDisplayUnits
    *           If set to \c true, all wavelength values are converted to the
    *           units set in the #WAVELENGTH_DISPLAY_UNITS_METADATA_NAME
    *           special metadata attribute.  If set to \c false, the units will
    *           be set to WavelengthUnitsType::MICRONS, which is how the values
    *           are stored in the DynamicObject.
    *
    *  @return  Returns \c true if the start, center, and end values and units
    *           are successfully set from the special metadata attributes;
    *           otherwise returns \c false.
    *
    *  @see     initializeFromWavelengths(), \ref specialmetadata
    */
   virtual bool initializeFromDynamicObject(const DynamicObject* pData, bool convertToDisplayUnits) = 0;

   /**
    *  Sets wavelength values and units special metadata attributes in a
    *  DynamicObject.
    *
    *  If starting, center, or ending values are not present, the existing
    *  special metadata attribute is removed.  If isEmpty() returns \c true,
    *  the units attribute is removed.
    *
    *  The wavelength values are set into the DynamicObject in microns,
    *  regardless of the units returned by getUnits().  The units returned by
    *  getUnits() are set into the #WAVELENGTH_DISPLAY_UNITS_METADATA_NAME
    *  attribute as the display units.
    *
    *  @param   pData
    *           The DynamicObject in which to set the wavelength values and
    *           units special metadata attributes.
    *
    *  @return  Returns \c true if the start, center, and end values and units
    *           special metadata attributes are successfully updated in the
    *           DynamicObject; otherwise returns \c false.
    *
    *  @see     initializeFromDynamicObject(), \ref specialmetadata
    */
   virtual bool applyToDynamicObject(DynamicObject* pData) = 0;

   /**
    *  Returns the number of wavelength values contained in the special metadata
    *  attributes of a DynamicObject.
    *
    *  This method is a convenience method to get the number of wavelength
    *  values in a DynamicObject without creating a Wavelengths instance and
    *  without making copies of its wavelength vectors.
    *
    *  @param   pData
    *           The DynamicObject from which to get the number of wavelength
    *           values contained in the special metadata attributes.  This
    *           should be the DynamicObject containing the "Special" attribute.
    *
    *  @return  Returns the number of wavelengths values in the DynamicObject
    *           if start, center, or end wavelength values are present;
    *           otherwise returns 0.
    *
    *  @see     getNumWavelengths(), isEmpty(), \ref specialmetadata
    */
   static unsigned int getNumWavelengths(const DynamicObject* pData)
   {
      if (pData != NULL)
      {
         // Center values
         const std::vector<double>* pCenterValues =
            dv_cast<std::vector<double> >(&pData->getAttributeByPath(CENTER_WAVELENGTHS_METADATA_PATH));
         if (pCenterValues != NULL)
         {
            return pCenterValues->size();
         }

         // Start values
         const std::vector<double>* pStartValues =
            dv_cast<std::vector<double> >(&pData->getAttributeByPath(START_WAVELENGTHS_METADATA_PATH));
         if (pStartValues != NULL)
         {
            return pStartValues->size();
         }

         // End values
         const std::vector<double>* pEndValues =
            dv_cast<std::vector<double> >(&pData->getAttributeByPath(END_WAVELENGTHS_METADATA_PATH));
         if (pEndValues != NULL)
         {
            return pEndValues->size();
         }
      }

      return 0;
   }

   /**
    *  Converts a wavelength value from one set of units to another.
    *
    *  @param   value
    *           The wavelength value to convert.
    *  @param   valueUnits
    *           The units of the given value.
    *  @param   newUnits
    *           The units to which the value should be converted.
    *
    *  @return  Returns the converted wavelength value in the new units.
    *           Returns \em value if either units parameter is invalid.
    */
   static double convertValue(double value, WavelengthUnitsType valueUnits, WavelengthUnitsType newUnits)
   {
      if ((valueUnits.isValid() == true) && (newUnits.isValid() == true) && (valueUnits != newUnits))
      {
         // Convert the value to microns
         switch (valueUnits)
         {
            case NANOMETERS:
               value *= 0.001;
               break;

            case INVERSE_CENTIMETERS:
               if (value != 0.0)
               {
                  value = (1.0 / value) * 10000.0;
               }
               break;

            default:
               break;
         }

         // Convert the value from microns to the new units
         switch (newUnits)
         {
            case NANOMETERS:
               value *= 1000.0;
               break;

            case INVERSE_CENTIMETERS:
               if (value != 0.0)
               {
                  value = 1.0 / (value * 0.0001);
               }
               break;

            default:
               break;
         }
      }

      return value;
   }

   /**
    *  The type string that should be returned from PlugIn::getType() for types
    *  of plug-ins that import or export wavelengths.
    *
    *  @return  Returns a string populated with "Wavelength".
    */
   static std::string WavelengthType()
   {
      return "Wavelength";
   }

   /**
    *  A default name for a Wavelengths plug-in argument.
    *
    *  @return  Returns a string populated with "Wavelengths".
    *
    *  @see     WavelengthFileArg()
    */
   static std::string WavelengthsArg()
   {
      return "Wavelengths";
   }

   /**
    *  A default name for a wavelengths file plug-in argument.
    *
    *  @return  Returns a string populated with "Wavelength File".
    *
    *  @see     WavelengthsArg()
    */
   static std::string WavelengthFileArg()
   {
      return "Wavelength File";
   }

protected:
   /**
    *  This should be destroyed by calling ObjectFactory::destroyObject().
    */
   virtual ~Wavelengths()
   {}
};

#endif
