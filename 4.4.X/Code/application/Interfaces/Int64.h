/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INT64_H
#define INT64_H

#include "AppConfig.h"
#include "DataVariantValidator.h"

#include <vector>

/**
 * A signed 64-bit value.  This class exists because there is no
 * single primitive 64-bit type available in all of the C++ compilers
 * that can effectively be serialized/deserialized across
 * multiple platforms. This class does not provide a set() function and 
 * that is by design.
 */
class Int64
{
public:
   /**
    * Construct a default signed 64-bit value.  The value returned from get() will be undefined.
    * This constructor shouldn't normally be used, you should use the constructor that
    * takes an initial value.  This constructor is provided because in order to allow
    * this type inside a DataVariant, a default constructor is required.
    */
   Int64() {}

   /** 
    * Construct a signed 64-bit object with the given value.
    *
    * @param value
    *        The signed 64-bit value that this object should wrap.
    */
   explicit Int64(int64_t value) : mValue(value) {}

   /**
    * Assignment operator.
    * 
    * @param right
    *        The right hand of the assignment.
    *
    * @return A reference to *this, which has been changed to have a copy of the contents of right
    */
   BROKEN_INLINE_HINT Int64& operator=(const Int64& right)
   {
      if (this != &right)
      {
         mValue = right.mValue;
      }
      return *this;
   }

   /**
    * Compares two Int64 objects.
    *
    * @param right
    *        The object to compare with.
    *
    * @return Returns true if both Int64 objects wrap the same signed 64-bit integer value.
    */
   bool operator==(const Int64& right) const
   {
      return mValue == right.mValue;
   }

   /**
    * Access the signed 64-bit integer value.
    * This class does not provide a set() function and 
    * that is by design.  You must use the Int64 constructor to set a value into this object.
    *
    * @returns Returns the wrapped signed 64-bit integer value.
    */
   int64_t get() const
   {
      return mValue;
   }

   /**
    * Access the signed 64-bit integer value.
    *
    * @returns Returns the wrapped signed 64-bit integer value.
    */
   operator int64_t() const
   {
      return mValue;
   }

private:
   int64_t mValue;
};


/**
 * \cond INTERNAL
 * These template specializations are required to allow these types to be put into a DataVariant.
 */
template <> class VariantTypeValidator<Int64> {};
template <> class VariantTypeValidator<std::vector<Int64> > {};
/// \endcond

#endif
