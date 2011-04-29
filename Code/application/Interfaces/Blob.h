/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BLOB_H
#define BLOB_H

#include "AppConfig.h"
#include "DataVariantValidator.h"

#include <vector>

/**
 * A Binary Large OBject (BLOB).  This class exists because the serialization/deserialization of
 * std::vector<unsigned char> is not recommended for large quantities of data even though this is a natural
 * representation of a large quantity of data. XML representations of this class are not generally human readable.
 * This class does not provide a set() function and that is by design. The StringUtilities::toDisplayString() and
 * StringUtilities::fromDisplayString() specializations of this class are not guaranteed to return useful information.
 */
class Blob
{
public:
   /**
    * Construct a default object.  The value returned from get() will be an empty vector.
    * This constructor shouldn't normally be used; you should use the constructor that
    * takes an initial value.  This constructor is provided because in order to allow
    * this type inside a DataVariant, a default constructor is required.
    */
   Blob() {}

   /** 
    * Construct an object from a std::vector of unsigned char data.
    *
    * @param value
    *        An array containing the data.
    */
   explicit Blob(const std::vector<unsigned char>& value) : mValue(value) {}

   /**
    * Construct a blob from a C array of octets.
    *
    * @param pValue
    *        C array of octets with len bytes. If this is NULL, an empty blob is created.
    * @param len
    *        The number of octets in the array.
    */
   explicit Blob(const void* pValue, size_t len)
   {
      if (pValue == NULL)
      {
         mValue.clear();
      }
      else
      {
         mValue.resize(len);
         memcpy(&mValue.front(), pValue, len);
      }
   }

   /**
    * Assignment operator.
    * 
    * @param right
    *        The right hand of the assignment.
    *
    * @return A reference to \c *this, which has been changed to have a copy of the contents of \em right.
    */
   BROKEN_INLINE_HINT Blob& operator=(const Blob& right)
   {
      if (this != &right)
      {
         mValue = right.mValue;
      }
      return *this;
   }

   /**
    * Compares two Blob objects.
    *
    * @param right
    *        The object to compare with.
    *
    * @return Returns \c true if both Blob objects wrap equivalent data.
    */
   bool operator==(const Blob& right) const
   {
      return mValue == right.mValue;
   }

   /**
    * Access the data.
    * This class does not provide a set() function and
    * that is by design.  You must use the Blob constructor to set a value into this object.
    *
    * @returns Returns the wrapped data.
    */
   const std::vector<unsigned char>& get() const
   {
      return mValue;
   }

   /**
    * Access the data.
    *
    * @returns Returns the data.
    */
   operator const std::vector<unsigned char>&() const
   {
      return mValue;
   }

private:
   std::vector<unsigned char> mValue;
};


/**
 * \cond INTERNAL
 * These template specializations are required to allow these types to be put into a DataVariant.
 */
template <> class VariantTypeValidator<Blob> {};
/// \endcond

#endif
