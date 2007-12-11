/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "GeoAlgorithms.h"

#include <complex>
#include <math.h>
#include "GeoConversions.h"
#include "MgrsDatum.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"

using namespace std;

GeoAlgorithms::GeoAlgorithms()
{
   // Initialize Vincenty lat/lon update variables
   mVincentyLastLatStart         = 0;
   mVincentyLastLonStart         = 0;
   mVincentyLastLatEnd           = 0;
   mVincentyLastLonEnd           = 0;

   // Initialize Vincenty Calculation Variables
   mVincentyL             = 0;
   mVincentyLambda        = 0;
   mVincentyU1            = 0;
   mVincentyU2            = 0;
   mVincentySigma         = 0;
   mVincentyCosToSigmaM   = 0;
   mVincentyAlpha         = 0;
   mF                     = (GeoConversions::EQU_RAD_M-GeoConversions::POLAR_RAD_M)/GeoConversions::EQU_RAD_M;
   m1MinF                 = 1-mF;
}

double GeoAlgorithms::getPythagoreanDistance (double aXStart,
                                       double aYStart,
                                       double aXEnd,
                                       double aYEnd)
{
   double lDistA = abs(aXEnd-aXStart);
   double lDistB = abs(aYEnd-aYStart);
   double lDistC = sqrt((lDistA * lDistA) + (lDistB * lDistB));

   return lDistC;
}

double GeoAlgorithms::getPythagoreanDistance(double ax, double ay, double az,
                                             double bx, double by, double bz)
{
   return sqrt(pow(ax-bx,2) + pow(ay-by,2) + pow(az-bz,2));
}


bool GeoAlgorithms::initVincenty(double aLat1, double aLon1,
                                 double aLat2, double aLon2)
{

   // Verify that input latitudes are between -90 and 90 and longitudes are
   // between -180 and 180
   if ((abs(aLat1) > 90) || (abs(aLat2) > 90) || (abs(aLon1) > 180)
        || (abs(aLon2) > 180))
   {
       return false;
   }

   // convert inputs in degrees to radians:
   aLat1   = aLat1 * 0.0174532925199433;
   aLon1   = aLon1 * 0.0174532925199433;
   aLat2     = aLat2 * 0.0174532925199433;
   aLon2     = aLon2 * 0.0174532925199433;

   // correct for errors at exact poles by adjusting 0.6 millimeters:
   if (abs(GeoConversions::PI_OVER_2-abs(aLat1)) < (1e-10) )
   {
       aLat1 = getSign(aLat1) * (GeoConversions::PI_OVER_2-(1e-10));
   }
   if (abs(GeoConversions::PI_OVER_2-abs(aLat2)) < (1e-10))
   {
       aLat2 = getSign(aLat2) * (GeoConversions::PI_OVER_2-(1e-10));
   }

   // Ellipse CalcuaAltitudeions?
   mVincentyU1 = atan(m1MinF*tan(aLat1));
   mVincentyU2 = atan(m1MinF*tan(aLat2));
   aLon1   = getMod(aLon1, (GeoConversions::TWO_PI));
   aLon2     = getMod(aLon2, (GeoConversions::TWO_PI));
   mVincentyL  = abs(aLon2-aLon1);
   if (mVincentyL > PI)
   {
     mVincentyL = GeoConversions::TWO_PI - mVincentyL;
   }

   // Initialize Variables for Loop
   double sin_mVincentyU1     = sin(mVincentyU1);
   double cos_mVincentyU1     = cos(mVincentyU1);
   double sin_mVincentyU2     = sin(mVincentyU2);
   double cos_mVincentyU2     = cos(mVincentyU2);
   double sinU1_sinU2         = sin_mVincentyU1 * sin_mVincentyU2;
   double cosU1_sinU2         = cos_mVincentyU1 * sin_mVincentyU2;
   double sinU1_cosU2         = sin_mVincentyU1 * cos_mVincentyU2;
   double cosU1_cosU2         = cos_mVincentyU1 * cos_mVincentyU2;
   double sin_mVincentyLambda = 0;
   double cos_mVincentyLambda = 0;
   double cos_mVincentyAlpha  = 0;
   double sin_mVincentySigma  = 0;
   double cos_mVincentySigma  = 0;
   double lLambdaOld    = 0;
   long   lIterCount    = 0;
   double lSinSigma     = 0;
   double lCosSigma     = 0;
   double C             = 0;
   mVincentySigma       = 0;
   mVincentyAlpha       = 0;
   mVincentyCosToSigmaM = 0;
   mVincentyLambda      = mVincentyL;

   // ?
   while ((!lIterCount) || abs((mVincentyLambda-lLambdaOld) > (1e-12)))
   {
      lIterCount+= 1;
      if (lIterCount > 50)
      {
          mVincentyLambda = PI;
          break;
      }

      sin_mVincentyLambda    = sin(mVincentyLambda);
      cos_mVincentyLambda    = cos(mVincentyLambda);
      lLambdaOld             = mVincentyLambda;
      lSinSigma              = sqrt( pow(cos_mVincentyU2 * sin_mVincentyLambda,
                               2) + pow( cosU1_sinU2 - sinU1_cosU2
                               * cos_mVincentyLambda, 2 ) );
      lCosSigma              = sinU1_sinU2 + cosU1_cosU2 * cos_mVincentyLambda;
      mVincentySigma         = atan2(lSinSigma, lCosSigma);
      sin_mVincentySigma     = sin(mVincentySigma);
      cos_mVincentySigma     = cos(mVincentySigma);
      mVincentyAlpha         = asin( cosU1_cosU2 * sin_mVincentyLambda /
                                     sin_mVincentySigma );
      cos_mVincentyAlpha     = cos(mVincentyAlpha);
      mVincentyCosToSigmaM   = cos_mVincentySigma -  2 * sinU1_sinU2
                               / pow(cos_mVincentyAlpha, 2);
      C                      = mF/ 16 * pow(cos_mVincentyAlpha, 2) * (4 + mF
                               * (4 - 3 * pow(cos_mVincentyAlpha, 2)));
      mVincentyLambda        = mVincentyL + (1 - C) * mF * sin(mVincentyAlpha)
                               * (mVincentySigma + C * sin_mVincentySigma
                               * (mVincentyCosToSigmaM + C * cos_mVincentySigma
                               * (-1 + 2 * pow(mVincentyCosToSigmaM, 2))));

      // Correct for convergence failure in the case of essentially
      // antipodal points
      if (mVincentyLambda > PI)
      {
          mVincentyLambda = PI;
          break;
      }
   }

   return true;
}

double GeoAlgorithms::getVincentyDistance(double aLat1, double aLon1,
                                          double aLat2, double aLon2)
{

   // If coordinates have changed, InitVincenty needs to be run again
   if (updateVincentyCoords(aLat1, aLon1, aLat2, aLon2))
   {
      // Initialize variables for Vincenty algorithms
      if (!initVincenty(aLat1, aLon1, aLat2, aLon2))
         {
            return -1;
         }
   }

   // ?
   double sin_mVincentySigma        = sin(mVincentySigma);
   double pow_mVincentyCosToSigmaM2 = pow(mVincentyCosToSigmaM, 2);
   double u2            = pow(cos(mVincentyAlpha), 2) * (pow(GeoConversions::EQU_RAD_M,
                          2) - pow(GeoConversions::POLAR_RAD_M, 2)) / pow(GeoConversions::POLAR_RAD_M, 2);
   double A             = 1 + u2 / 16384 * ( 4096 + u2 * (-768 + u2 * (320 - 175 * u2)));
   double B             = u2/1024 * (256 + u2 * (-128 + u2 * (74 - 47 * u2)));

   double lDeltaSigma   = B * sin_mVincentySigma * (mVincentyCosToSigmaM + B
                          / 4 * (cos(mVincentySigma) * (-1 + 2 *
                          pow_mVincentyCosToSigmaM2) - B / 6 *
                          mVincentyCosToSigmaM * (-3 + 4 *
                          pow(sin_mVincentySigma, 2)) * (-3 + 4 *
                          pow_mVincentyCosToSigmaM2)));

   return GeoConversions::POLAR_RAD_M * A * (mVincentySigma - lDeltaSigma);
}


double GeoAlgorithms::getPythagoreanOrVincentyDistance(double aLat1, double aLon1,
                                                       double aLat2, double aLon2,
                                                       double elevation1, double elevation2)
{
   double a_84, b_84, old_a, old_b, ax, ay, az, bx, by, bz, returnValue;
 
   Initialize_Ellipsoids();
   WGS84_Axes(&a_84, &b_84);
   Get_Geocentric_Parameters(&old_a, &old_b);
   Set_Geocentric_Parameters(a_84, b_84);

   if( Convert_Geodetic_To_Geocentric(aLat1*PI/180, aLon1*PI/180, elevation1, &ax, &ay, &az) == 0 &&
       Convert_Geodetic_To_Geocentric(aLat2*PI/180, aLon2*PI/180, elevation2, &bx, &by, &bz) == 0 )
   {
      double pythagoreanDistance = getPythagoreanDistance(ax, ay, az, bx, by, bz);
      double vincentyDistance = getVincentyDistance(aLat1, aLon1, aLat2, aLon2);
      //Note: if getVincentyDistance fails and returns < 0, pythagoreanDistance will be returned.
      returnValue = pythagoreanDistance > vincentyDistance ? pythagoreanDistance : vincentyDistance;
   }
   else
   {
      returnValue = getVincentyDistance(aLat1, aLon1, aLat2, aLon2);
   }
   Set_Geocentric_Parameters(old_a, old_b);
   return returnValue;
}


double GeoAlgorithms::getVincentyAzimuth(double aLat1, double aLon1,
                                         double aLat2, double aLon2)
{

   // If coordinates have changed, InitVincenty needs to be run again
   if (updateVincentyCoords(aLat1, aLon1, aLat2, aLon2))
   {
       // Initialize variables for Vincenty algorithms
       if (!initVincenty(aLat1, aLon1, aLat2, aLon2))
       {
           return -1;
       }
   }


   // From point #1 to point #2
   double lP1ToP2 = atan2(cos(mVincentyU2) * sin(mVincentyLambda),
                    cos(mVincentyU1) * sin(mVincentyU2) - sin(mVincentyU1)
                    * cos(mVincentyU2) * cos(mVincentyLambda));
   if (lP1ToP2 < 0)
   {
      lP1ToP2 = (lP1ToP2 + 2 * PI);
   }

   return lP1ToP2;
}

bool GeoAlgorithms::updateVincentyCoords(double aLat1, double aLon1,
                                          double aLat2, double aLon2)
{
   bool update = false;
   if (aLat1 != mVincentyLastLatStart)
   {
      mVincentyLastLatStart = aLat1;
      update = true;
   }
   if (aLon1 != mVincentyLastLonStart)
   {
         mVincentyLastLonStart = aLon1;
         update = true;
   }
   if (aLat2 != mVincentyLastLatEnd)
   {
         mVincentyLastLatEnd = aLat2;
         update = true;
   }
   if (aLon2 != mVincentyLastLonEnd)
   {
         mVincentyLastLonEnd = aLon2;
         update = true;
   }

   return update;
}

double GeoAlgorithms::getMod(double x, double y)
{
   if (y == 0) return x;
   return (x - (floor(x/y) * y));
}

double GeoAlgorithms::getSign(double aVal)
{
   if (aVal < 0.0) return -1;
   if (aVal > 0.0) return 1;
   return 0;
}

double GeoAlgorithms::getEllipsoidDistance(double aLat1, double aLon1,
                                           double aLat2, double aLon2)
{
   double distance = 0.0;
   double  faz, baz;
   double  r = 1.0 - GeoConversions::FLATTENING;
   double  tu1, tu2, cu1, su1, cu2, x, sx, cx, sy, cy, y, sa, c2a, cz, e, c, d;
   double  cosy1, cosy2;
   distance = 0.0;

   if((aLon1 == aLon2) && (aLat1 == aLat2)) return distance;
   aLon1 = GeoConversions::convertDegToRad(aLon1);
   aLon2 = GeoConversions::convertDegToRad(aLon2);
   aLat1 = GeoConversions::convertDegToRad(aLat1);
   aLat2 = GeoConversions::convertDegToRad(aLat2);

   cosy1 = cos(aLat1);
   cosy2 = cos(aLat2);

   if(cosy1 == 0.0) cosy1 = 0.0000000001;
   if(cosy2 == 0.0) cosy2 = 0.0000000001;

   tu1 = r * sin(aLat1) / cosy1;
   tu2 = r * sin(aLat2) / cosy2;
   cu1 = 1.0 / sqrt(tu1 * tu1 + 1.0);
   su1 = cu1 * tu1;
   cu2 = 1.0 / sqrt(tu2 * tu2 + 1.0);
   x = aLon2 - aLon1;

   distance = cu1 * cu2;
   baz = distance * tu2;
   faz = baz * tu1;

   do {
      sx = sin(x);
      cx = cos(x);
      tu1 = cu2 * sx;
      tu2 = baz - su1 * cu2 * cx;
      sy = sqrt(tu1 * tu1 + tu2 * tu2);
      cy = distance * cx + faz;
      y = atan2(sy, cy);
      sa = distance * sx / sy;
      c2a = -sa * sa + 1.0;
      cz = faz + faz;
      if(c2a > 0.0) cz = -cz / c2a + cy;
      e = cz * cz * 2. - 1.0;
      c = ((-3.0 * c2a + 4.0) * GeoConversions::FLATTENING + 4.0) * c2a * GeoConversions::FLATTENING / 16.0;
      d = x;
      x = ((e * cy * c + cz) * sy * c + y) * sa;
      x = (1.0 - c) * x * GeoConversions::FLATTENING + aLon2 - aLon1;
   } while(fabs(d - x) > GeoConversions::EPS);

   x = sqrt((1.0 / r / r - 1.0) * c2a + 1.0) + 1.0;
   x = (x - 2.0) / x;
   c = 1.0 - x;
   c = (x * x / 4.0 + 1.0) / c;
   d = (0.375 * x * x - 1.0) * x;
   x = e * cy;
   distance = 1.0 - e - e;
   distance = ((((sy * sy * 4.0 - 3.0) *
   distance * cz * d / 6.0 - x) * d / 4.0 + cz) * sy * d + y) * c * GeoConversions::EQU_RAD_KM * r;

   return distance;
}

double GeoAlgorithms::getGCDistance(double aLat1, double aLon1,
                                    double aLat2, double aLon2)
{
   aLat1 = GeoConversions::convertDegToRad(aLat1);
   aLon1 = GeoConversions::convertDegToRad(aLon1);
   aLat2 = GeoConversions::convertDegToRad(aLat2);
   aLon2 = GeoConversions::convertDegToRad(aLon2);

   double d = sin(aLat1)*sin(aLat2) + cos(aLat1)*cos(aLat2)*cos(aLon1 - aLon2);
   return (GeoConversions::AVG_EQU_RAD_KM * acos(d));
}

double GeoAlgorithms::getGCAzimuth(double aLat1, double aLon1,
                                   double aLat2, double aLon2)
{
   double result = 0.0;

   long iaLat1 = (long)(0.50 + aLat1 * 360000.0);
   long iaLat2 = (long)(0.50 + aLat2 * 360000.0);
   long iaLon1 = (long)(0.50 + aLon1 * 360000.0);
   long iaLon2 = (long)(0.50 + aLon2 * 360000.0);

   aLat1 = GeoConversions::convertDegToRad(aLat1);
   aLon1 = GeoConversions::convertDegToRad(aLon1);
   aLat2 = GeoConversions::convertDegToRad(aLat2);
   aLon2 = GeoConversions::convertDegToRad(aLon2);

   if ((iaLat1 == iaLat2) && (iaLon1 == iaLon2))
   {
      return result;
   }
   else if (iaLon1 == iaLon2)
   {
      if (iaLat1 > iaLat2)
         result = 180.0;
   }
   else
   {
      double c = acos(sin(aLat2)*sin(aLat1) + cos(aLat2)*cos(aLat1)*cos((aLon2-aLon1)));
      double A = asin(cos(aLat2)*sin((aLon2-aLon1))/sin(c));
      result = (GeoConversions::convertRadToDeg(A));

      if ((iaLat2 > iaLat1) && (iaLon2 > iaLon1))
      {
      }
      else if ((iaLat2 < iaLat1) && (iaLon2 < iaLon1))
      {
         result = 180.0 - result;
      }
      else if ((iaLat2 < iaLat1) && (iaLon2 > iaLon1))
      {
         result = 180.0 - result;
      }
      else if ((iaLat2 > iaLat1) && (iaLon2 < iaLon1))
      {
         result += 360.0;
      }
   }

   return result;
}

double GeoAlgorithms::getPythagoreanAzimuth(double aXStart, double aYStart,
                                            double aXEnd, double aYEnd)
{
   double lDist1 = aXEnd - aXStart;
   double lDist2 = aYEnd - aYStart;
   double lAzimuth = 0;

   if (lDist1 > 1e-10)
   {
      if (lDist2 > 1e-10)
      {
         lAzimuth = atan(lDist2/lDist1);
      }
      else if (lDist2 < -1e-10)
      {
         lAzimuth = atan(lDist1/abs(lDist2)) + GeoConversions::THREE_PI_OVER_2;
      }
   }
   else if (lDist1 < -1e-10)
   {
      if (lDist2 > 1e-10)
      {
         lAzimuth = atan(abs(lDist1)/lDist2) + GeoConversions::PI_OVER_2;
      }
      else if (lDist2 < -1e-10)
      {
         lAzimuth = atan(abs(lDist2)/abs(lDist1)) + PI;
      }
   }

   return lAzimuth;
}

double GeoAlgorithms::getXaxisGSD(const RasterElement *pRaster)
{
   if (pRaster == NULL)
   {
      return 0.0;
   }

   const RasterDataDescriptor* pDescriptor = static_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return 0.0;
   }

   LocationType origin(0.0, 0.0);
   LocationType point(0.0, 0.0);
   point.mX = pDescriptor->getColumnCount();

   LocationType originGeo = pRaster->convertPixelToGeocoord(origin);
   LocationType pointGeo = pRaster->convertPixelToGeocoord(point);
   
   GeoAlgorithms geoAlgorithms;
   double distance = geoAlgorithms.getVincentyDistance(originGeo.mX, originGeo.mY, pointGeo.mX, pointGeo.mY);
   return distance / point.mX;
}

double GeoAlgorithms::getYaxisGSD(const RasterElement *pRaster)
{
   if (pRaster == NULL || !pRaster->isGeoreferenced())
   {
      return 0.0;
   }

   const RasterDataDescriptor* pDescriptor = static_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return 0.0;
   }

   LocationType origin(0.0, 0.0);
   LocationType point(0.0, 0.0);
   point.mY = pDescriptor->getRowCount();

   LocationType originGeo = pRaster->convertPixelToGeocoord(origin);
   LocationType pointGeo = pRaster->convertPixelToGeocoord(point);

   GeoAlgorithms geoAlgorithms;
   double distance = geoAlgorithms.getVincentyDistance(originGeo.mX, originGeo.mY, pointGeo.mX, pointGeo.mY);

   return distance / point.mY;
}
