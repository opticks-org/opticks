/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */


#ifndef GEOALGORITHMS_H
#define GEOALGORITHMS_H

#include "LocationType.h"

class PerspectiveView;
class RasterElement;

/**
 * This class contains a set of algorithms used to compute
 * distance and azimuth of two coordinates.  The class also
 * contains methods to perform conversions common to distance
 * calculations.
 */
class GeoAlgorithms
{
public:
   /**
    * Initializes variables used by GeoAlgorithms
    */
   GeoAlgorithms();

   virtual ~GeoAlgorithms(){}

   /**
    *   Computes distance between two points assuming flat surface.
    *   
    * This method is useful for computing approximate distances when 
    * location data is not available.  The input values are not assumed 
    * to be in any particular units.  However, they must be for a flat 
    * grid reference system.  (ie. Lat/Lon coordinates won't work)
    *
    *   @param aXStart
    *     The X value of the first pair of coordinates.
    *   @param aXEnd
    *     The Y value of the first pair of coordinates.
    *   @param aYStart
    *     The X value of the end pair of coordinates.
    *   @param aYEnd
    *     The Y value of the end pair of coordinates.
    *   @return 
    *     The distance between the set of input coordinates.
    */
   static double getPythagoreanDistance(double aXStart, double aYStart,
                                 double aXEnd, double aYEnd);
   /**
    *   Computes distance between two points in 3D space.
    *
    *   @param ax
    *     The X value of the first pair of coordinates.
    *   @param ay
    *     The Y value of the first pair of coordinates.
    *   @param az
    *     The Z value of the first pair of coordinates.
    *   @param bx
    *     The X value of the second pair of coordinates.
    *   @param by
    *     The Y value of the second pair of coordinates.
    *   @param bz
    *     The Z value of the second pair of coordinates.
    *   @return 
    *     The distance between the set of input coordinates.
    */
   static double getPythagoreanDistance(double ax, double ay, double az,
                                        double bx, double by, double bz);

   static double getGCDistance(double aLat1, double aLon1,
                        double aLat2, double aLon2);

   /**
    * Returns the distance between two coordinates.
    *
    * This method calculates the distance between two lat/lon coordinates.
    * A WGS84 Ellipsoid earth is assumed
    *
    *   @param aLat1
    *     The latitude of the first coordinate
    *   @param aLon1
    *     The longitude of the first coordinate
    *   @param aLat2
    *     The latitude of the second coordinate
    *   @param aLon2
    *     The longitude of the second coordinate
    *   @return
    *     The distance between the two coordinates in meters
    */
   static double getEllipsoidDistance(double aLat1, double aLon1,
                               double aLat2, double aLon2);


   /**
    *   Compute distance between points on the WGS-84 ellipsoidal
    *   Earth to within a few millimeters of accuracy using
    *   Vincenty's algorithm.
    *   
    *   Calculates distance between two points using the Vincenty
    *   distance algorithm.  See description of InitVincenty for
    *   details of the VincentyAlgorithms and the use of the 
    *   InitVincenty method.

    *   @param aLat1
    *     GEODETIC latitude of first point (-90(S) to 90(N))
    *   @param aLon1
    *     Longitude of first point (-180(W) to 180(E))
    *   @param aLat2
    *     GEODETIC latitude of second point (-90(W) to 90(E))
    *   @param aLon2
    *     Longitude of second point (-180(W) to 180(E))
    *   @return
    *      Arc length distance in meters.  Returns -1 on error, otherwise,
    *      return value should be positive (>=0).
    */
   double getVincentyDistance(double aLat1, double aLon1,
                              double aLat2, double aLon2);

   /**
    *   Compute both the 3D Pythagorean and Vincenty distance between points
    *   on the WGS-84 ellipsoidal and return the maximum.
    *   
    *   @param aLat1
    *     GEODETIC latitude of first point (-90(S) to 90(N))
    *   @param aLon1
    *     Longitude of first point (-180(W) to 180(E))
    *   @param aLat2
    *     GEODETIC latitude of second point (-90(W) to 90(E))
    *   @param aLon2
    *     Longitude of second point (-180(W) to 180(E))
    *   @param elevation1
    *     The elevation of the first point in meters.
    *   @param elevation2
    *     The elevation of the second point in meters.
    *   @return
    *      Distance in meters.  Returns -1 on error, otherwise,
    *      return value should be positive (>=0).
    */
   double getPythagoreanOrVincentyDistance(double aLat1, double aLon1,
                                           double aLat2, double aLon2,
                                           double elevation1, double elevation2);


   /**
    * Returns the azimuth between two points.
    *
    * This method calculates the azimuth from point 1 to point 2.  A flat 
    * coordinate grid is assumed.  Points are assumed to be measured from 
    * an origin of 0.
    *
    *   @param aXStart
    *     The X value of the first point
    *   @param aYStart
    *     The Y value of the first point
    *   @param aXEnd
    *     The X value of the second point
    *   @param aYEnd
    *     The Y value of the second point
    *   @return
    *     The angle between the two points.  No units assumed.
    */
   static double getPythagoreanAzimuth(double aXStart, double aYStart,
                                double aXEnd, double aYEnd);

   /**
    * Returns the azimuth between two coordinates.
    *
    * This method calculates the azimuth between two lat/lon coordinates.
    * A Spherical earth is assumed.
    *
    *   @param aLat1
    *     The latitude of the first coordinate
    *   @param  aLon1
    *     The longitude of the first coordinate
    *   @param aLat2
    *     The latitude of the second coordinate
    *   @param aLon2
    *     The longitude of the second coordinate
    *   @return
    *     The azimuth from point 1 to point 2 in degrees.
    */
   static double getGCAzimuth(double aLat1, double aLon1, double aLat2, double aLon2);
   
   
   /**
    *   Calculate azimuth using Vincenty algorithm
    *   
    *   This method determines the azimuth from one point to another.
    *   Results are in radians.  See description of InitVincenty for details
    *   of the VincentyAlgorithms and the use of the InitVincenty method.
    *
    *   @param aLat1
    *     GEODETIC latitude of first point (-90(S) to 90(N))
    *   @param aLon1
    *     Longitude of first point (-180(W) to 180(E))
    *   @param aLat2
    *     GEODETIC latitude of second point (-90(W) to 90(E))
    *   @param aLon2
    *     Longitude of second point (-180(W) to 180(E))
    *   @return
    *     The azimuth from point 1 (start) to point 2 (end) in radians.  If an
    *     error occurs, -1 is returned.
    */
   double getVincentyAzimuth(double aLat1, double aLon1,
                             double aLat2, double aLon2);

   /**
    *  Calculates the distance between the origin and the number of columns in the cube. 
    *
    *  @param   pRaster
    *           The RasterElement to compute the GSD for
    *
    *  @return  This method returns the distance between the origin
    *           and the number of columns in the cube.
    */
   static double getXaxisGSD(const RasterElement *pRaster);

   /**
    *  Calculates the distance between the origin and the number of rows in the cube. 
    *
    *  @param   pRaster
    *           The RasterElement to compute the GSD for
    *
    *  @return  This method returns the distance between the origin
    *           and the number of rows in the cube.
    */
   static double getYaxisGSD(const RasterElement *pRaster);

   /**
    *  Calculates the angle to north.
    *
    *  This method only calculates the counterclockwise rotation to north from a pixel. It does not account for
    *  the view being upside down. The caller is responsible for any needed adjustments due to origin or orientation.
    *
    *  @param  pRaster
    *          The RasterElement to use for the calculation.
    *  @param  angle
    *          The counterclockwise angle to north in degrees. This is only valid when the method returns \c true.
    *  @param  pixelStart
    *          The coordinate from which to compute the angle to north. The angle to north is computed by comparing
    *          the latitude of this pixel to the latitude of the pixel directly above it.
    *
    *  @return \c True on success, \c false otherwise.
    *
    *  @see    getAngleToNorth(const RasterElement*, const PerspectiveView*, double&)
    */
   static bool getAngleToNorth(const RasterElement* pRaster, double& angle, LocationType pixelStart);

   /**
    *  Calculates the angle to north.
    *
    *  This method calculates the counterclockwise rotation to north from the visible center of the view. It checks if the
    *  view is upside down and adjusts the angle accordingly.
    *
    *  @param  pRaster
    *          The RasterElement to use for the calculation.
    *  @param  pView
    *          The PerspectiveView to use in checking if the image is flipped.
    *  @param  angle
    *          The counterclockwise angle to north in degrees. If the image in the view is flipped, the angle is
    *          adjusted by adding 180 degrees. The value is only valid when the method returns \c true.
    *
    *  @return \c True on success, \c false otherwise.
    *
    *  @see    getAngleToNorth(const RasterElement*, double&, LocationType)
    */
   static bool getAngleToNorth(const RasterElement* pRaster, const PerspectiveView* pView, double& angle);

private:
   // Calculation Variables used only by Vincenty Algorithms
   double mVincentyLastLatStart;
   double mVincentyLastLonStart;
   double mVincentyLastLatEnd;
   double mVincentyLastLonEnd;
   double mVincentyL;
   double mVincentyLambda;
   double mVincentyU1;
   double mVincentyU2;
   double mVincentySigma;
   double mVincentyCosToSigmaM;
   double mVincentyAlpha;
   double mF;   // Copied this from other code. Don't know what its for.
   double m1MinF;

   // Vincenty Methods
   /**
    *   Verify and update last known Vincenty coordinates.
    *   
    *   This method will check the input coordinates against the
    *   last know coordinates member variables and determine if 
    *   an update is necessary.  If an update is needed, the passed 
    *   in coordinates will replace the stored coordinates and true 
    *   will be returned.  If an update is not needed (coordinates 
    *   are the same) nothing is changed and false is returned.
    *
    *   @param aLat1
    *     GEODETIC latitude of first point (-90(S) to 90(N))
    *   @param aLon1
    *     Longitude of first point (-180(W) to 180(E))
    *   @param aLat2
    *     GEODETIC latitude of second point (-90(W) to 90(E))
    *   @param aLon2
    *     Longitude of second point (-180(W) to 180(E))
    *   @return
    *     true if update was performed, otherwise false.
    */
   bool   updateVincentyCoords(double aLat1, double aLon1,
                               double aLat2, double aLon2);
   
   /**
    *   Performs calculations for Vincenty algorithms
    *   
    *   Performs initial calculations for Vincenty algorithms.  Most of the
    *   processing for the Vincenty algorithms occur in this method.
    *   InitVincenty is run anytime coordinates have changed.  It is used by all
    *   of the Vincenty algorithms.  So, if one Vincenty algorithm is run, it
    *   will not be run again for the rest of the Vincenty algorithms, as long as
    *   the coordinates remain the same.
    *
    *   Original algorithm source:
    *     T. Vincenty, "Direct and Inverse Solutions of Geodesics on the
    *     Ellipsoid with Application of Nested Equations", Survey Review, vol.
    *     23, no. 176, April 1975, pp 88-93
    *
    *   Transcription and Translation:
    *     The Vincenty procedure was transcribed verbatim by Peter Cederholm,
    *     August 12, 2003. It was modified and translated to English by Michael
    *     Kleder.  Mr. Cederholm's website is http://www.plan.aau.dk/~pce/
    *
    *   Implementation:
    *     Original Code was modified and converted to C++ by Nick Bays, BATC.
    *
    *   Notes:
    *     -# Error correcting code, convergence failure traps, antipodal
    *        corrections, polar error corrections, and WGS84 ellipsoid
    *        parameters written by Michael Kleder, 2004.
    *     -# Vincenty describes his original algorithm as precise to within
    *        0.01 millimeters, subject to the ellipsoidal model.
    *     -# Essentially antipodal points are treated as exactly
    *        antipodal, potentially reducing accuracy by a small amount.
    *     -# Failures for points exactly at the poles are eliminated by
    *        moving the points by 0.6 millimeters.
    *     -# This algorithm uses an ellipsoidal earth model and is,
    *        therefore, more accurate than a spherical Earth model. The
    *        maximum differences between this algorithm and the spherical
    *        earth algorithm is:
    *        - Max absolute difference: 38 kilometers
    *        - Max fractional difference: 0.56 percent
    * 
    *   @param aLat1
    *     GEODETIC latitude of first point (-90(S) to 90(N))
    *   @param aLon1
    *     Longitude of first point (-180(W) to 180(E))
    *   @param aLat2
    *     GEODETIC latitude of second point (-90(W) to 90(E))
    *   @param aLon2
    *     Longitude of second point (-180(W) to 180(E))
    *   @return
    *     true if succesful, false if not
    */
   bool initVincenty(double aLat1, double aLon1, double aLat2, double aLon2);

   /**
    * Computes modulus of two double values
    * 
    * This method allows the computation of modulus for double values.
    * C++'s native % operator will not accept double values as input.  
    * This was necessary to convert a matlab routine to C++.
    *
    *   @param x
    *     The dividend
    *   @param y
    *     The divisor
    *   @return
    *     The quotient of the inputs.
    */
   double getMod(double x, double y);

   /**
    * Determines whether input value is positive, negative, or 0.
    *
    * This method looks at the input value and returns its sign.
    *
    *   @param aVal
    *     The value to evaluate
    *   @return
    *     1 if aVal is positive, -1 if aVal is negative, 0 if aVal is 0.0
    */
   double getSign(double aVal);
};

#endif

