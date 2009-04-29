/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"

// these must be before Hdf4Utilities so the symbols are properly compiled
#include <hdf.h>
#include <mfhdf.h>
#include <string>

#include "AppVerify.h"
#include "Hdf4Utilities.h"
#include "Hdf4Attribute.h"

using namespace std;

namespace
{
template<typename T>
bool readAttribute(int32 obj_id, int32 attr_index, int32 count, DataVariant& value)
{
   vector<T> temp(count);
   int iSuccess = SDreadattr(obj_id, attr_index, &temp.front());
   if (iSuccess != SUCCEED)
   {
      return false;
   }
   if (count > 1)
   {
      value = temp;
   }
   else
   {
      value = temp[0];
   }
   return value.isValid();
}
}

string HdfUtilities::hdf4TypeToString(long type, size_t count)
{
   string strType = UNKNOWN_TYPE;
   switch (type)
   {
      case DFNT_CHAR:  // fall through
      case DFNT_INT8:
         if (count > 1)
         {
            strType = "string";
         }
         else
         {
            strType = "char";
         }
         break;
      case DFNT_UCHAR:  // fall through
      case DFNT_UINT8:
         if (count > 1)
         {
            strType = "vector<unsigned char>";
         }
         else
         {
            strType = "unsigned char";
         }
         break;
      case DFNT_INT16:
         if (count > 1)
         {
            strType = "vector<short>";
         }
         else
         {
            strType = "short";
         }
         break;
      case DFNT_UINT16:
         if (count > 1)
         {
            strType = "vector<unsigned short>";
         }
         else
         {
            strType = "unsigned short";
         }
         break;
      case DFNT_INT32:
         if (count > 1)
         {
            strType = "vector<int>";
         }
         else
         {
            strType = "int";
         }
         break;
      case DFNT_UINT32:
         if (count > 1)
         {
            strType = "vector<unsigned int>";
         }
         else
         {
            strType = "unsigned int";
         }
         break;
      case DFNT_FLOAT32:
         if (count > 1)
         {
            strType = "vector<float>";
         }
         else
         {
            strType = "float";
         }
         break;
      case DFNT_FLOAT64:
         if (count > 1)
         {
            strType = "vector<double>";
         }
         else
         {
            strType = "double";
         }
         break;
      default:
         break;
   }
   return strType;
}

bool HdfUtilities::readHdf4Attribute(int32 obj_id, int32 attr_index, DataVariant& var)
{
   char name[MAX_NC_NAME];
   int32 type = 0;
   int32 count = 0;

   int iSuccess = SDattrinfo(obj_id, attr_index, name, &type, &count);
   if (iSuccess != SUCCEED)
   {
      return false;
   }

   bool success = false;
   switch (type)
   {
      case DFNT_CHAR:  // fall through
      case DFNT_INT8:
         if (count > 1)
         {
            string temp;
            temp.resize(count + 1);
            iSuccess = SDreadattr(obj_id, attr_index, &(temp[0]));
            if (iSuccess != SUCCEED)
            {
               return false;
            }
            var = temp;
            success = var.isValid();
         }
         else
         {
            char temp;
            iSuccess = SDreadattr(obj_id, attr_index, &temp);
            if (iSuccess != SUCCEED)
            {
               return false;
            }
            var = temp;
            success = var.isValid();
         }
         break;
      case DFNT_UCHAR:  // fall through
      case DFNT_UINT8:
         success = readAttribute<unsigned char>(obj_id, attr_index, count, var);
         break;
      case DFNT_INT16:
         success = readAttribute<short>(obj_id, attr_index, count, var);
         break;
      case DFNT_UINT16:
         success = readAttribute<unsigned short>(obj_id, attr_index, count, var);
         break;
      case DFNT_INT32:
         success = readAttribute<int>(obj_id, attr_index, count, var);
         break;
      case DFNT_UINT32:
         success = readAttribute<unsigned int>(obj_id, attr_index, count, var);
         break;
      case DFNT_FLOAT32:
         success = readAttribute<float>(obj_id, attr_index, count, var);
         break;
      case DFNT_FLOAT64:
         success = readAttribute<double>(obj_id, attr_index, count, var);
         break;
      default:
         break;
   }

   return success;
}

int32* HdfUtilities::Hdf4FileObject::obtainResource(const HdfUtilities::Hdf4FileObject::Args &args) const
{
   int iValid = Hishdf(args.mFilename.c_str());
   int32* pInt = new (nothrow) int32;
   VERIFYRV(pInt != NULL, NULL);
   if (iValid > 0 && pInt != NULL)
   {
      *pInt = SDstart(args.mFilename.c_str(), args.mAccess);
   }
   return pInt;
}

void HdfUtilities::Hdf4FileObject::releaseResource(const HdfUtilities::Hdf4FileObject::Args &args,
                                                   int32* pHandle) const
{ 
   if (pHandle != NULL && *pHandle != FAIL)
   {
      SDend(*pHandle);
      delete pHandle;
   }
}

int32* HdfUtilities::Hdf4DatasetObject::obtainResource(const HdfUtilities::Hdf4DatasetObject::Args &args) const
{
   int32* pInt = new (nothrow) int32;
   if (pInt != NULL)
   {
      *pInt = SDselect(args.mFileHandle, args.mIndex);
   }
   return pInt;
}

void HdfUtilities::Hdf4DatasetObject::releaseResource(const HdfUtilities::Hdf4DatasetObject::Args &args,
                                                      int32* pHandle) const
{ 
   if (pHandle != NULL && *pHandle != FAIL)
   {
      SDendaccess(*pHandle);
      delete pHandle;
   }
}
