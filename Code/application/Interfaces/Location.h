/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LOCATION_H
#define LOCATION_H

#include <math.h>
#include <utility>
#include "AppAssert.h"
#include "AppConfig.h"

namespace Opticks
{

/**
 * A point location.
 *
 * This can represent any two or three dimensional location either
 * integral or floating point. Usually, there will be typedefs for
 * specific uses such as geographic location and pixel location so this
 * class is not typically used directly.
 *
 * The template argument T is the data type used for each value in the location. This
 * is usually and int or double.
 * The template argument SZ must be either 2 or 3 and indicates the dimensionality
 * of the Location. If this value does not match the dimensionality of the contructor
 * used, an assertion exceptions will be thrown.
 *
 * @see LocationType, PixelLocation
 */
template<typename T, int SZ>
class Location
{
public:
   /**
    *  Creates a Location object with a given two dimensional position.
    *
    *  @param   x
    *           The first dimension's value.
    *  @param   y
    *           The second dimension's value.
    *  @throw   AssertException if the dimensionality of this Location is not 2.
    */
   Location(T x, T y) : mX(x), mY(y), mZ(0)
   {
      ENSURE(SZ == 2);
   }

   /**
    *  Creates a Location object with a given two dimensional position.
    *
    *  @param   point
    *           A point that is the source for the copy. point.first is
    *           used for mX, point.second for mY.
    *  @throw   AssertException if the dimensionality of this Location is not 2.
    */
   template<typename U>
   Location(std::pair<U, U> point) : mX(point.first), mY(point.second), mZ(0)
   {
      ENSURE(SZ == 2);
   }

   /**
    *  Creates a Location object with a given three dimensional position.
    *
    *  @param   x
    *           The first dimension's value.
    *  @param   y
    *           The second dimension's value.
    *  @param   z
    *           The third dimension's value.
    *  @throw   AssertException if the dimensionality of this Location is not 3.
    */
   Location(T x, T y, T z): mX(x), mY(y), mZ(z)
   {
      ENSURE(SZ == 3);
   }


  /**
   *  Creates a Location object with a given position.
   *
   *  The dimensionality of the two Locations must be the same.
   *
   *  @param   point
   *           A point that is the source for the copy.
   */
   Location(const Location<T,SZ> &point): mX(point.mX), mY(point.mY), mZ(point.mZ) {}

   /**
    *  Creates a default Location object.
    *
    *  A default location contains zero values for the X, Y, and Z
    *  positions.
    */
   Location(): mX(0), mY(0), mZ(0) {}

   /**
    *  Destructor
    */
   ~Location() {}

   /**
    *  Returns the dimensionality of the Location.
    *
    *  @return The dimensionality. Either 2 or 3.
    */
   static int dimensionality() { return SZ; }

   /**
    *  Returns the distance between the values comprising the Location.
    *
    *  The length will always be a floating point value regardless of the
    *  underlying data type of the Location.
    *
    *  @return  The length of the vector represented by the LocationType.
    */
   double length() const
   {
      if(SZ == 2)
      {
         return sqrt(static_cast<double>(mX * mX) + mY * mY);
      }
      else if(SZ == 3)
      {
         return sqrt(static_cast<double>(mX * mX) + mY * mY + mZ * mZ);
      }
      return 0.0; // not reached
   }

   /**
    *  Adds two Locations.
    *
    *  Both Locations must have the same dimensionality.
    *
    *  @param   point
    *           The other Location to add.
    *
    *  @return  A Location containing the sum of the coordinates
    */
   Location<T,SZ> operator+(const Location<T,SZ> &point) const
   {
      if(SZ == 2)
      {
         return Location<T,SZ>(mX + point.mX, mY + point.mY);
      }
      else if(SZ == 3)
      {
         return Location<T,SZ>(mX + point.mX, mY + point.mY, mZ + point.mZ);
      }
      return Location<T,SZ>(); // not reached
   }

   /**
    *  Adds a scalar to the values in the Location.
    *
    *  @param   scalar
    *           The scalar to add to the components of the Location
    *
    *  @return  A Location where each component has scalar added to it
    */
   Location<T,SZ> operator+(T scalar) const
   {
      if(SZ == 2)
      {
         return Location<T,SZ>(mX + scalar, mY + scalar);
      }
      else if(SZ == 3)
      {
         return Location<T,SZ>(mX + scalar, mY + scalar, mZ + scalar);
      }
      return Location<T,SZ>(); // not reached
   }

   /**
    *  Adds two Locations.
    *
    *  @param   point
    *           The other Location to add.
    *
    *  @return  A Location containing the sum of the coordinates
    */
   Location<T,SZ> &operator+=(const Location<T,SZ> &point)
   {
      mX += point.mX;
      mY += point.mY;
      if(SZ == 3)
      {
         mZ += point.mZ;
      }

      return *this;
   }

   /**
    *  Subtracts two Locations.
    *
    *  @param   point
    *           The other Location to subtract.
    *
    *  @return  A Location containing the difference of the coordinates
    */
   Location<T,SZ> operator-(const Location<T,SZ> &point) const
   {
      if(SZ == 2)
      {
         return Location<T,SZ>(mX - point.mX, mY - point.mY);
      }
      else if(SZ == 3)
      {
         return Location<T,SZ>(mX - point.mX, mY - point.mY, mZ - point.mZ);
      }
      return Location<T,SZ>(); // not reached
   }

   /**
    *  Subtracts a scalar from the values in the Location.
    *
    *  @param   scalar
    *           The scalar to subtract from the components of the Location
    *
    *  @return  A Location where each component has scalar subtracted from it
    */
   Location<T,SZ> operator-(T scalar) const
   {
      if(SZ == 2)
      {
         return Location<T,SZ>(mX - scalar, mY - scalar);
      }
      else if(SZ == 3)
      {
         return Location<T,SZ>(mX - scalar, mY - scalar, mZ - scalar);
      }
      return Location<T,SZ>(); // not reached
   }

   /**
    *  Subtracts two Locations.
    *
    *  @param   point
    *           The other Location to subtract.
    *
    *  @return  A Location containing the difference of the coordinates
    */
   Location<T,SZ> &operator-=(const Location<T,SZ> &point)
   {
      mX -= point.mX;
      mY -= point.mY;
      if(SZ == 3)
      {
         mZ -= point.mZ;
      }

      return *this;
   }

   /**
    *  Multiplies the values of the Location by a scalar.
    *
    *  @param   scalar
    *           The scalar to multiply the components of the Location
    *
    *  @return  A Location where each component has scalar multiplied by it.
    */
   Location<T,SZ> operator*(T scalar) const
   {
      if(SZ == 2)
      {
         return Location<T,SZ>(mX * scalar, mY * scalar);
      }
      if(SZ == 3)
      {
         return Location<T,SZ>(mX * scalar, mY * scalar, mZ * scalar);
      }
      return Location<T,SZ>(); // not reached
   }

   /**
    *  Assigns another Location to this.
    *
    *  @param   point
    *           The source Location.
    *
    *  @return  A reference to *this, which has been changed to have a copy of the contents of point.
    */
   BROKEN_INLINE_HINT Location<T,SZ> &operator=(const Location<T,SZ> &point)
   {
      if(this != &point)
      {
         mX = point.mX;
         mY = point.mY;
         mZ = point.mZ;
      }

      return *this;
   }

   /**
    *  Compares two Locations.
    *
    *  @param   point
    *           The other Location.
    *
    *  @return  Returns true if the coordinates of *this are equal to those of point.
    */
   bool operator==(const Location<T,SZ> &point) const
   {
      return (mX == point.mX) && (mY == point.mY) && (SZ == 2 || (mZ == point.mZ));
   }

   /**
    *  Compares two Locations.
    *
    *  @param   point
    *           The other Location.
    *
    *  @return  Returns true if the coordinates of *this are NOT equal to those of point.
    */
   bool operator!=(const Location<T,SZ> &point) const
   {
      return !(*this == point);
   }

   /**
    *  Compares two Locations.
    *
    *  @param   point
    *           The other Location.
    *
    *  @return  Returns true if all coordinates of *this are smaller than those of point.
    */
   bool operator<(const Location<T,SZ> &point) const
   {
      return (mX < point.mX) && (mY < point.mY) && (SZ == 2 || mZ < point.mZ);
   }

   /**
    *  Ensures that both coordinates of this Location are not smaller than another.
    *
    *  @param   point
    *           The coordinates of the smallest allowable Location
    */
   void clampMinimum(const Location<T,SZ> &point)
   {
      mX = std::max(mX, point.mX);
      mY = std::max(mY, point.mY);
      if(SZ == 3)
      {
         mZ = std::max(mZ, point.mZ);
      }
   }

   /**
    *  Ensures that both coordinates of this Location are not larger than another.
    *
    *  @param   point
    *           The coordinates of the largest allowable Location.
    */
   void clampMaximum(const Location<T,SZ> &point)
   {
      mX = std::min(mX, point.mX);
      mY = std::min(mY, point.mY);
      if(SZ == 3)
      {
         mZ = std::min(mZ, point.mZ);
      }
   }

   T mX;
   T mY;
   T mZ;
};

/**
 * This represents a pixel location in cube or screen coordinates.
 */
typedef Location<int,2> PixelLocation;

/**
 * This represents a pixel offset in cube or screen coordinates.
 */
typedef Location<int,2> PixelOffset;

}

#endif
