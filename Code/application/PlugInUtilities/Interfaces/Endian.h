/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ENDIAN_H
#define ENDIAN_H

#include "ComplexData.h"
#include "TypesFile.h"

/**
 *  A class to perform byte-swapping operations.
 *
 *  The Endian class provides a capability to perform byte-swapping of data
 *  values based on the endian type of the system.  An Endian object should be
 *  created with the target endian type.  The swapping methods can then be
 *  called to swap data values to the target type.  If the target type is the
 *  same as the system, no swapping is performed.
 *
 *  @see        EndianType
 */
class Endian
{
public:
   /**
    *  Creates an Endian object of a given endian type.
    *
    *  @param   endian
    *           The endian type for the object.  This type is used as the
    *           target endian type for the byte-swapping methods.  If the given
    *           type is the same as the system type the byte-swapping methods
    *           can still be called, but no swapping is performed.
    *
    *  @see     EndianType
    */
   Endian(EndianType endian);

   /**
    *  Creates an Endian object which will always swap.
    *
    *  @see     EndianType
    */
   Endian();

   /**
    *  Destroys the Endian object.
    */
   ~Endian();

   /**
    *  Returns the endian type of <em>this</em>.
    *
    *  @return  The endian type.
    *
    *  @see     getSystemEndian()
    */
   EndianType getEndian() const;

   /**
    *  Queries the endian type of <em>this</em> for EndianType::BIG_ENDIAN_ORDER.
    *
    *  @return  Returns true if the endian type is EndianType::BIG_ENDIAN_ORDER,
    *           otherwise returns false.
    */
   bool isBigEndian() const;

   /**
    *  Queries the endian type of <em>this</em> for EndianType::LITTLE_ENDIAN_ORDER.
    *
    *  @return  Returns true if the endian type is EndianType::LITTLE_ENDIAN_ORDER,
    *           otherwise returns false.
    */
   bool isLittleEndian() const;

   /**
    *  Swaps the bytes of a single data value.
    *
    *  This method performs in-place byte swapping of a single data value
    *  by modifying the given element.  If the original value should not be
    *  overwritten, use the swapValue() method instead.
    *
    *  @param   value
    *           A reference to the data value to swap.
    *
    *  @return  Returns true if byte swapping was actually performed.  Returns
    *           false if no byte swapping is necessary because the endian type
    *           is already the same as the system.
    *
    *  @see     swapValue(const T &amp;), swapBuffer(T *,size_t)
    */
   template <class T>
   bool swapValue(T& value) const
   {
      return swapBuffer(&value, 1);
   }

   /**
    *  Swaps the bytes of a single data value.
    *
    *  This method performs byte swapping of a single data value and returns
    *  the swapped value.  The original data element is not modified. For
    *  performance reasons, if the original data element can be overwritten,
    *  use the swapValue() method instead.
    *
    *  @param   value
    *           A data value to swap.  The value is not modified.
    *
    *  @return  Returns the swapped data value.  If no swapping is necessary, a
    *           copy of the original data value is returned.
    *
    *  @see     swapValue(T &amp;)
    */
   template <class T>
   T swapValue(const T& value) const
   {
      T newValue = value;
      swapValue(newValue);

      return newValue;
   }

   /**
    *  Swaps the bytes of data elements in an array.
    *
    *  This method swaps the bytes of data elements in an array if the endian
    *  type of the system is different than the endian type of <em>this</em>.
    *
    *  This method is provided as a convenience if the data type is known.
    *
    *  @param   pBuffer
    *           A pointer to the data to be byte swapped.
    *  @param   count
    *           The number of items in the array to byte swap.
    *
    *  @return  Returns true if byte swapping was actually performed.  Returns
    *           false if no byte swapping is necessary because the endian type
    *           is already the same as the system.
    *
    *  @see     swapBuffer(void *,size_t,size_t), swapValue(T &amp;)
    */
   template <class T>
   bool swapBuffer(T* pBuffer, size_t count) const
   {
      return swapBuffer(pBuffer, sizeof(T), count);
   }

   /**
    * This override has the same behavior as the templated
    * function of the same name.
    *
    * It is needed because the template definition is incorrect
    * for complex data.
    *
    * @param pBuffer
    *        The FloatComplex buffer to swap
    * @param count
    *        The number of elements in pBuffer to swap
    *
    *  @return  Returns true if byte swapping was actually performed.  Returns
    *           false if no byte swapping is necessary because the endian type
    *           is already the same as the system.
    */
   bool swapBuffer(FloatComplex* pBuffer, size_t count) const
   {
      return swapBuffer(pBuffer, sizeof(float), 2*count);
   }

   /**
    * This override has the same behavior as the templated
    * function of the same name.
    *
    * It is needed because the template definition is incorrect
    * for complex data.
    *
    * @param pBuffer
    *        The IntegerComplex buffer to swap
    * @param count
    *        The number of elements in pBuffer to swap
    *
    *  @return  Returns true if byte swapping was actually performed.  Returns
    *           false if no byte swapping is necessary because the endian type
    *           is already the same as the system.
    */
   bool swapBuffer(IntegerComplex* pBuffer, size_t count) const
   {
      return swapBuffer(pBuffer, sizeof(short), 2*count);
   }

   /**
    *  Swaps the bytes of data elements in an array.
    *
    *  This method swaps the bytes of data elements in an array if the endian
    *  type of the system is different than endian type of <em>this</em>.
    *
    *  @param   pBuffer
    *           A pointer to the data to be byte swapped.
    *  @param   dataSize
    *           The size of each element in the array.
    *  @param   count
    *           The number of items in the array to byte swap.
    *
    *  @return  Returns true if byte swapping was actually performed.  Returns
    *           false if no byte swapping is necessary because the endian type
    *           is already the same as the system.
    *
    *  @see     swapBuffer(T *,size_t), swapValue(T &amp;)
    */
   bool swapBuffer(void* pBuffer, size_t dataSize, size_t count) const
   {
      if ((pBuffer == NULL) || (mSystemType == mType))
      {
         return false;
      }

      for (size_t i = 0; i < count; ++i)
      {
         unsigned char* pData = reinterpret_cast<unsigned char*>(pBuffer) + (i * dataSize);
         for (size_t j = 0; j < dataSize / 2; ++j)
         {
            size_t index = dataSize - j - 1;

            unsigned char byteData = pData[j];
            pData[j] = pData[index];
            pData[index] = byteData;
         }
      }

      return true;
   }

   /**
    *  Returns the endian type of the system.
    *
    *  This value is used in the byte-swapping methods to determine whether
    *  the data values should be swapped or not.
    *
    *  @return  The system endian type.
    *
    *  @see     getEndian()
    */
   static EndianType getSystemEndian();

private:
   EndianType mType;
   EndianType mSystemType;
};

#endif
