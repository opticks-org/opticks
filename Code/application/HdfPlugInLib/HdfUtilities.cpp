/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include <sstream>

#include "HdfUtilities.h"

using namespace std;

unsigned int HdfUtilities::getDataSize(EncodingType dataType)
{
   unsigned int dataSize = 0;
   switch(dataType)
   {
   case INT1SBYTE: // fall through
   case INT1UBYTE:
      dataSize = 1;
      break;
   case INT2SBYTES: // fall through
   case INT2UBYTES:
      dataSize = 2;
      break;
   case INT4SBYTES: // fall through
   case INT4UBYTES: // fall through
   case INT4SCOMPLEX: // fall through
   case FLT4BYTES: // fall through
      dataSize = 4;
      break;
   case FLT8BYTES: // fall through
      dataSize = 8;
      break;
   default: // hit an invalid type
      throw Exception("Invalid data type encountered!");
   }
   return dataSize;
}

vector<double> HdfUtilities::createWavelengthVector(void* pData, size_t numElements, EncodingType type)
{
   if (pData == NULL)
   {
      throw HdfUtilities::Exception("Invalid pointer passed to HdfUtilities::createWavelengthVector()");
   }

   vector<double> wavelengths;
   bool bTruncated = false;
   for (size_t ui = 0; ui < numElements; ++ui)
   {
      switch(type)
      {
      case INT1SBYTE:
         wavelengths.push_back(reinterpret_cast<char*>(pData)[ui]);
         break;
      case INT1UBYTE:
         wavelengths.push_back(reinterpret_cast<unsigned char*>(pData)[ui]);
         break;
      case INT2SBYTES:
         wavelengths.push_back(reinterpret_cast<short*>(pData)[ui]);
         break;
      case INT2UBYTES:
         wavelengths.push_back(reinterpret_cast<unsigned short*>(pData)[ui]);
         break;
      case INT4SBYTES:
         wavelengths.push_back(reinterpret_cast<int*>(pData)[ui]);
         break;
      case INT4UBYTES:
         wavelengths.push_back(reinterpret_cast<unsigned int*>(pData)[ui]);
         break;
      case FLT4BYTES:
         wavelengths.push_back(reinterpret_cast<float*>(pData)[ui]);
         break;
      case FLT8BYTES:
         bTruncated = true;
         wavelengths.push_back(reinterpret_cast<double*>(pData)[ui]);
         break;
      default:
         throw HdfUtilities::Exception("Invalid wavelength type!");
      }
   }
   return wavelengths;
}
