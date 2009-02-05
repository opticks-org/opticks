/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef UINT64_H
#define UINT64_H

#include "AppConfig.h"

/**
 * A unsigned 64-bit value.  This class exists because there is no
 * single primitive 64-bit type available in all of the C++ compilers
 * that can effectively be serialized/deserialized across
 * multiple platforms. This class does not provide a set() function and 
 * that is by design.
 */
class UInt64
{
public:
   /**
    * Construct a default unsigned 64-bit value.  The value returned from get() will be undefined.
    * This constructor shouldn't normally be used, you should use the constructor that
    * takes an initial value.  This constructor is provided because in order to allow
    * this type inside a DataVariant, a default constructor is required.
    */
   UInt64() {}

   /** 
    * Construct a unsigned 64-bit object with the given value.
    *
    * @param value
    *        The unsigned 64-bit value that this object should wrap.
    */
   explicit UInt64(uint64_t value) : mValue(value) {}

   /**
    * Assignment operator.
    * 
    * @param right
    *        The right hand of the assignment.
    *
    * @return A reference to *this, which has been changed to have a copy of the contents of right
    */
   BROKEN_INLINE_HINT UInt64& operator=(const UInt64& right)
   {
      if (this != &right)
      {
         mValue = right.mValue;
      }
      return *this;
   }

   /**
    * Compares two UInt64 objects.
    *
    * @param right
    *        The object to compare with.
    *
    * @return Returns true if both UInt64 objects wrap the same unsigned 64-bit integer value.
    */
   bool operator==(const UInt64& right) const
   {
      return mValue == right.mValue;
   }

   /**
    * Access the unsigned 64-bit integer value.
    * This class does not provide a set() function and 
    * that is by design.  You must use the UInt64 constructor to set a value into this object.
    *
    * @returns Returns the wrapped unsigned 64-bit integer value.
    */
   uint64_t get() const
   {
      return mValue;
   }

   /**
    * Access the unsigned 64-bit integer value.
    *
    * @return Returns the wrapped unsigned 64-bit integer value.
    */
   operator uint64_t() const
   {
      return mValue;
   }

private:
   uint64_t mValue;
};

/**
 * \cond INTERNAL
 * These template specializations are required to allow these types to be put into a DataVariant.
 */
template <> class VariantTypeValidator<UInt64> {};
/// \endcond

#endif
