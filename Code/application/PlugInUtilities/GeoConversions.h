/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOCONVERSIONS_H
#define GEOCONVERSIONS_H

#include <math.h>
#include "AppConfig.h"

/**
 * The namespace contains methods to perform conversions common
 * to Geodesic Measurements.
 */
namespace GeoConversions
{
   // PI and Derivatives Constants
   const double PI_OVER_2 = PI/2.0;
   const double PI_OVER_4 = PI/4.0;
   const double TWO_PI = 2.0 * PI;
   const double THREE_PI_OVER_2 = 3.0 * PI / 2.0;
   const double DEG_TO_RAD = PI / 180.0;
   const double RAD_TO_DEG = 180.0 / PI;

   // WGS-84 Constants
   const double POLAR_RAD_M = 6356752.31424518; //!< Polar Radius-Km
   const double EQU_RAD_M = 6378137.0;          //!< Equatorial Radius-meters
   const double EQU_RAD_KM = 6378.137;          //!< Equatorial Radius-Km
   const double AVG_EQU_RAD_KM = 6371.0;        //!< Avg Equatorial Radius-Km
   const double FLATTENING = 1.000000/298.257223563;
   const double INV_FLATTENING = 298.257223563;
   const double EPS = 0.000000000005;

   // Common Trig Constants
   const double HALF_SQRT_TWO(sqrt(2.0)/2.0);
   const double SIN_45(HALF_SQRT_TWO);
   const double COS_45(HALF_SQRT_TWO);

   /**
    *   Converts degrees to radians.
    *
    *   @param aDegrees
    *     A value in Degrees that is to be converted to Radians
    *   @return 
    *     The input value (aDegrees) converted to radians.
    */
   inline double convertDegToRad(double aDegrees)
   {
      return aDegrees * DEG_TO_RAD;
   }

   /**
   *   Converts radians to degrees
   *
   *   @param aRadians
   *     A value in Radians that is to be converted to Degrees
   *   @return
   *     The input value (aRadians) converted to degrees
   */
   inline double convertRadToDeg(double aRadians)
   {
      return aRadians * RAD_TO_DEG;
   }

   /**
    *   Converts feet to meters.
    *  
    *   @param aFeet
    *     A value in Feet that is to be converted to Meters
    *   @return 
    *     The input value (aFeet) converted to meters
    */
   inline double convertFeetToMeters(double aFeet)
   {  
      return aFeet * 0.3048;
   }

   /**
    *   Converts meters to feet
    *
    *   @param aMeters
    *     A value in Meters that is to be converted to Feet
    *   @return 
    *     The input value (aMeters) converted to feet
    */
   inline double convertMetersToFeet(double aMeters)
   {
      return aMeters * 3.28083989501312335958;
   }
   
   /**
    *   Converts meters to nautical miles
    *   
    *   @param  aMeters
    *     A value in Meters that is to be converted to Nautical Miles
    *   @return 
    *     The input value (aMeters) converted to nautical miles
    */
   inline double convertMetersToNm(double aMeters)
   {
      return aMeters * 0.0005399568034557235;
   }

   /**
    *   Converts nautical miles to meters
    *
    *   @param aNm
    *     A value in Nautical Miles that is to be converted to Meters
    *   @return 
    *     The input value (aNm) converted to meters
    */
   inline double convertNmToMeters(double aNm)
   {
      return aNm * 1852.0000;
   }
   
   /**
    *   Converts meters to miles
    *   
    *   @param  aMeters
    *     A value in Meters that is to be converted to Statute Miles
    *   @return
    *     The input value (aMeters) converted to statute miles
    */
   inline double convertMetersToMiles(double aMeters)
   {
      return aMeters * 0.0006213699494949496;
   }

   
   /**
    *   Converts statute miles to meters
    *   
    *   @param aMiles
    *     A value in Statute Miles that is to be converted to Meters
    *   @return
    *     The input value (aMiles) converted to meters
    */
   inline double convertMilesToMeters(double aMiles)
   {
      return aMiles * 1609.3412196;
   }

   /** 
    *   Converts statute miles to feet.
    *
    *   @param miles
    *     A value in statute miles that is to be converted to feet.
    *   @return
    *     The input value converted to feet.
    */
   inline double convertMilesToFeet(double miles)
   {
      return miles * 5280.0f;
   }
   
   /**
    *   Converts nautical miles to radians
    *   
    *   @param aNm
    *     A value in Nautical Miles that is to be converted to Radians
    *   @return
    *     The input value (aNm) converted to radians
    */
   inline double convertNmToRad(double aNm)
   {
      return aNm * 0.00029088820866572159;
   }
   
   /**
   *   Converts radians to nautical miles
   *   
   *   @param aRad
   *     A value in Radians that is to be converted to Nautical Miles
   *   @return 
   *     The input value (aRad) converted to nautical miles
   */
   inline double convertRadToNm(double aRad)
   {
      return aRad * 3437.7467707849392526;
   }
};

#endif
