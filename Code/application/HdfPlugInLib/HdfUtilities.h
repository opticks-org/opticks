/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef HDF_UTILITIES_H
#define HDF_UTILITIES_H

#include <string>
#include <vector>

#include "TypesFile.h"

/**
 * A collection of generic utilities provided to simplify use of the application's HDF API.
 */
namespace HdfUtilities
{
   /**
    * Represents an unknown type found in the Hdf file.  The HdfPlugInLib will not parse data of this type.
    */
   static const std::string UNKNOWN_TYPE = "Unknown";

   /**
    * An %HdfUtilities::Exception is a simple exception that wraps a string.
    *
    * Various HdfUtilities throw an HdfUtilities::Exception when an error
    * condition occurrs and the function can't just 'return false.'
    */
   class Exception
   {
   public:
      /**
       * Creates an %HdfUtilities::Exception based on a string.
       *
       * @param  str
       *         Text to place into the exception. Should not be empty (but can be).
       */
      explicit Exception(const std::string &str) : mText(str.c_str()) {}

      /**
       * Gets the message from the exception.
       *
       * @return The string that was passed in to the constructor.
       */
      std::string getText() const { return mText; }
   private:
      std::string mText;
   };

   /**
    * Returns the size of each data element given an %EncodingType.
    *
    * Throws an HdfUtilities::Exception() if an invalid data type is encountered.
    * 
    * @param  dataType
    *         An %EncodingType that represents the data. Never returns 0.
    *
    * @return The size of the data type passed in. Returns 0 if the type is unknown or unsupported.
    */
   unsigned int getDataSize(EncodingType dataType);

   /**
    * Takes a void pointer and produces a wavelength vector.
    *
    * WARNING: This function cannot provide range checking! If passed invalid values, this method
    *          throws an HdfUtilities::Exception.
    *
    * @param  pData
    *         A pointer to memory (most frequently read in using HdfImporter::loadDatasetFromFile())
    *         that represents the wavelength vector.
    * @param  numElements
    *         The number of elements in the vector.
    * @param  type
    *         The data type of the wavelength vector. Valid values are INT1SBYTE, INT1UBYTE, INT2SBYTES,
    *         INT2UBYTES, INT4SBYTES, INT4UBYTES, INT4SCOMPLEX, FLT4BYTES, FLT8BYTES. If an invalid
    *         value is passed in for this argument (ie. UNKNOWN or INT4SCOMPLEX), an HdfUtilities::Exception
    *         is thrown.
    *
    * @return Returns a vector containing the wavelength data.
    */
   std::vector<double> createWavelengthVector(void* pData, size_t numElements, EncodingType type);
};

#endif
