/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConfigurationSettings.h"
#include "DMutex.h"
#include "FileResource.h"
#include "MemoryMappedArray.h"
#include "MemoryMappedArrayView.h"
#include "ModelServices.h"
#include "PlugInResource.h"
#include "PointCloudAccessor.h"
#include "PointCloudAccessorImpl.h"
#include "PointCloudDataDescriptorImp.h"
#include "PointCloudElement.h"
#include "PointCloudElementImp.h"
#include "PointCloudFileDescriptorImp.h"
#include "PointCloudInMemoryPager.h"
#include "PointCloudMemoryMappedPager.h"
#include "RasterUtilities.h"

namespace
{
   double convert_s1byte_to_double(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const signed char*>(pValue)) * scale + offset;
   }

   double convert_u1byte_to_double(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const unsigned char*>(pValue)) * scale + offset;
   }

   double convert_s2byte_to_double(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const signed short*>(pValue)) * scale + offset;
   }

   double convert_u2byte_to_double(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const unsigned short*>(pValue)) * scale + offset;
   }

   double convert_s4byte_to_double(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const signed int*>(pValue)) * scale + offset;
   }

   double convert_u4byte_to_double(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const unsigned int*>(pValue)) * scale + offset;
   }

   double convert_float_to_double(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const float*>(pValue)) * scale + offset;
   }

   double convert_double_to_double(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const double*>(pValue)) * scale + offset;
   }

   int64_t convert_s1byte_to_integer(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const signed char*>(pValue)) * scale + offset;
   }

   int64_t convert_u1byte_to_integer(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const unsigned char*>(pValue)) * scale + offset;
   }

   int64_t convert_s2byte_to_integer(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const signed short*>(pValue)) * scale + offset;
   }

   int64_t convert_u2byte_to_integer(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const unsigned short*>(pValue)) * scale + offset;
   }

   int64_t convert_s4byte_to_integer(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const signed int*>(pValue)) * scale + offset;
   }

   int64_t convert_u4byte_to_integer(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const unsigned int*>(pValue)) * scale + offset;
   }

   int64_t convert_float_to_integer(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const float*>(pValue)) * scale + offset;
   }

   int64_t convert_double_to_integer(const void* pValue, double scale, double offset)
   {
      return *(reinterpret_cast<const double*>(pValue)) * scale + offset;
   }
}

using namespace std;
XERCES_CPP_NAMESPACE_USE

void PointCloudElementImp::Deleter::operator()(PointCloudAccessorImpl* pCloudAccessor)
{
   delete pCloudAccessor;
   delete this;
}

PointCloudElementImp::PointCloudElementImp(const DataDescriptorImp& descriptor, const string& id) :
   DataElementImp(descriptor, id),
   mArrayCount(0),
   mModified(false),
   mpInMemoryData(NULL),
   mpPager(NULL)
{
   createData();
}

PointCloudElementImp::~PointCloudElementImp()
{
   Service<PlugInManagerServices> pPluginManager;
   if (mpPager != NULL)
   {
      pPluginManager->destroyPlugIn(dynamic_cast<PlugIn*>(mpPager));
   }
   if (mTempFilename.empty() == false)
   {
      remove(mTempFilename.c_str());
   }
}

uint32_t PointCloudElementImp::getArrayCount()
{
   return mArrayCount;
}

void PointCloudElementImp::createData()
{
   PointCloudDataDescriptor* pDescriptor = dynamic_cast<PointCloudDataDescriptor*>(getDataDescriptor());   
   if (pDescriptor == NULL)
   {
      return;
   }
   mArrayCount = 0;
   PointCloudArrangement arrangement = pDescriptor->getArrangement();
   if (arrangement == POINT_ARRAY)
   {
      mArrayCount = pDescriptor->getPointCount();
   }
   else if (arrangement == POINT_KDTREE_XY_ARRAY || POINT_KDTREE_XYZ_ARRAY)
   {
      uint32_t numElements = pDescriptor->getPointCount(); // compute the next highest power of 2 of 32-bit v
      numElements--;
      numElements |= numElements >> 1;
      numElements |= numElements >> 2;
      numElements |= numElements >> 4;
      numElements |= numElements >> 8;
      numElements |= numElements >> 16;
      numElements++;
      mArrayCount = numElements;
   }
}

void PointCloudElementImp::updateData(uint32_t updateMask)
{
   mModified = true;
   notify(SIGNAL_NAME(PointCloudElement, DataModified), boost::any(updateMask));
}

bool PointCloudElementImp::setPager(PointCloudPager* pPager)
{
   if (mpPager != NULL || pPager == NULL)
   {
      return false;
   }
   mpPager = pPager;
   return true;
}

PointCloudPager* PointCloudElementImp::getPager() const
{
   return mpPager;
}

bool PointCloudElementImp::createInMemoryPager()
{
   if (getPager() != NULL)
   {
      return false;
   }
   ExecutableResource pPlugin("PointCloud In Memory Pager");
   VERIFY(pPlugin->getPlugIn() != NULL);

   PointCloudInMemoryPager* pPager = dynamic_cast<PointCloudInMemoryPager*>(pPlugin->getPlugIn());
   VERIFY(pPager != NULL);

   PointCloudDataDescriptor* pDescriptor = dynamic_cast<PointCloudDataDescriptor*>(getDataDescriptor());
   VERIFY(pDescriptor != NULL);
   if (!pPager->initialize(pDescriptor, mArrayCount))
   {
      return false;
   }

   if (!setPager(pPager))
   {
      return false;
   }

   pPlugin->releasePlugIn();

   return true;
}

bool PointCloudElementImp::createMemoryMappedPagerForNewTempFile()
{
   if (getPager() != NULL)
   {
      return false;
   }
   ExecutableResource pPlugin("PointCloud MemoryMappedPager");
   VERIFY(pPlugin->getPlugIn() != NULL);

   PointCloudMemoryMappedPager* pPager = dynamic_cast<PointCloudMemoryMappedPager*>(pPlugin->getPlugIn());
   VERIFY(pPager != NULL);

   PointCloudDataDescriptor* pDescriptor = dynamic_cast<PointCloudDataDescriptor*>(getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   if (mTempFilename.empty() == false)
   {
      remove(mTempFilename.c_str());
      mTempFilename.erase();
   }

   const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
   string tempPath;
   if (pTempPath != NULL)
   {
      tempPath = pTempPath->getFullPathAndName();
   }

   char* pTempFilename = tempnam(tempPath.c_str(), "PC");
   if (pTempFilename == NULL)
   {
      return false;
   }
   mTempFilename = pTempFilename;
   free(pTempFilename);

   { 
      uint64_t pointSize = pDescriptor->getPointSizeInBytes();
      LargeFileResource tempFile;
      if (!tempFile.reserve(mTempFilename, pointSize * mArrayCount))
      {
         return false;
      }
   } //scoped block so that LargeFileResource closes when exiting here

   if (!pPager->initialize(mTempFilename, pDescriptor->getPointSizeInBytes(), true))
   {
      return false;
   }

   if (!setPager(pPager))
   {
      return false;
   }

   pPlugin->releasePlugIn();

   return true;
}

bool PointCloudElementImp::createDefaultPager()
{
   if (mpPager != NULL)
   {
      return true;
   }

   DataDescriptorImp* pDescriptor = getDataDescriptor();
   VERIFY(pDescriptor != NULL);

   switch (pDescriptor->getProcessingLocation())
   {
   case IN_MEMORY:
      return createInMemoryPager();
   case ON_DISK:
      return createMemoryMappedPagerForNewTempFile();
   case ON_DISK_READ_ONLY: // fall through
   default:
      return false;
   }
}

PointCloudAccessor PointCloudElementImp::getPointCloudAccessor(PointCloudDataRequest *pRequestIn)
{
   if (pRequestIn == NULL)
   {
      FactoryResource<PointCloudDataRequest> pRequestDefault;
      pRequestIn = pRequestDefault.release();
   }
   FactoryResource<PointCloudDataRequest> pRequest(pRequestIn);

   const PointCloudDataDescriptor* pDescriptor = dynamic_cast<const PointCloudDataDescriptor*>(getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return PointCloudAccessor(NULL, NULL);
   }

   pRequest->polish(pDescriptor);
   if (!pRequest->validate(pDescriptor))
   {
      return PointCloudAccessor(NULL, NULL);
   }

   PointCloudAccessorImpl* pImpl = new PointCloudAccessorImpl(dynamic_cast<PointCloudElement*>(this), pRequest.release());
   PointCloudAccessorDeleter* pDeleter = NULL;
   if (pImpl != NULL)
   {
      pDeleter = new PointCloudElementImp::Deleter;
   }
   const PointCloudDataDescriptorImp* pDesc = static_cast<const PointCloudDataDescriptorImp*>(getDataDescriptor());
   pImpl->mHdrXScale = pDesc->getXScale();
   pImpl->mHdrXOffset = pDesc->getXOffset();
   pImpl->mHdrYScale = pDesc->getYScale();
   pImpl->mHdrYOffset = pDesc->getYOffset();
   pImpl->mHdrZScale = pDesc->getZScale();
   pImpl->mHdrZOffset = pDesc->getZOffset();
   switch (pDesc->getSpatialDataType())
   {
   case INT1SBYTE:
      pImpl->mSpatialConvertToDoubleFunc = convert_s1byte_to_double;
      pImpl->mSpatialConvertToIntegerFunc = convert_s1byte_to_integer;
      break;
   case INT1UBYTE:
      pImpl->mSpatialConvertToDoubleFunc = convert_u1byte_to_double;
      pImpl->mSpatialConvertToIntegerFunc = convert_u1byte_to_integer;
      break;
   case INT2SBYTES:
      pImpl->mSpatialConvertToDoubleFunc = convert_s2byte_to_double;
      pImpl->mSpatialConvertToIntegerFunc = convert_s2byte_to_integer;
      break;
   case INT2UBYTES:
      pImpl->mSpatialConvertToDoubleFunc = convert_u2byte_to_double;
      pImpl->mSpatialConvertToIntegerFunc = convert_u2byte_to_integer;
      break;
   case INT4SBYTES:
      pImpl->mSpatialConvertToDoubleFunc = convert_s4byte_to_double;
      pImpl->mSpatialConvertToIntegerFunc = convert_s4byte_to_integer;
      break;
   case INT4UBYTES:
      pImpl->mSpatialConvertToDoubleFunc = convert_u4byte_to_double;
      pImpl->mSpatialConvertToIntegerFunc = convert_u4byte_to_integer;
      break;
   case FLT4BYTES:
      pImpl->mSpatialConvertToDoubleFunc = convert_float_to_double;
      pImpl->mSpatialConvertToIntegerFunc = convert_float_to_integer;
      break;
   case FLT8BYTES:
      pImpl->mSpatialConvertToDoubleFunc = convert_double_to_double;
      pImpl->mSpatialConvertToIntegerFunc = convert_double_to_integer;
      break;
   case INT4SCOMPLEX:
   case FLT8COMPLEX:
   default:
      delete pImpl;
      pImpl = NULL;
      break;
   }
   switch (pDesc->getIntensityDataType())
   {
   case INT1SBYTE:
      pImpl->mIConvertToDoubleFunc = convert_s1byte_to_double;
      pImpl->mIConvertToIntegerFunc = convert_s1byte_to_integer;
      break;
   case INT1UBYTE:
      pImpl->mIConvertToDoubleFunc = convert_u1byte_to_double;
      pImpl->mIConvertToIntegerFunc = convert_u1byte_to_integer;
      break;
   case INT2SBYTES:
      pImpl->mIConvertToDoubleFunc = convert_s2byte_to_double;
      pImpl->mIConvertToIntegerFunc = convert_s2byte_to_integer;
      break;
   case INT2UBYTES:
      pImpl->mIConvertToDoubleFunc = convert_u2byte_to_double;
      pImpl->mIConvertToIntegerFunc = convert_u2byte_to_integer;
      break;
   case INT4SBYTES:
      pImpl->mIConvertToDoubleFunc = convert_s4byte_to_double;
      pImpl->mIConvertToIntegerFunc = convert_s4byte_to_integer;
      break;
   case INT4UBYTES:
      pImpl->mIConvertToDoubleFunc = convert_u4byte_to_double;
      pImpl->mIConvertToIntegerFunc = convert_u4byte_to_integer;
      break;
   case FLT4BYTES:
      pImpl->mIConvertToDoubleFunc = convert_float_to_double;
      pImpl->mIConvertToIntegerFunc = convert_float_to_integer;
      break;
   case FLT8BYTES:
      pImpl->mIConvertToDoubleFunc = convert_double_to_double;
      pImpl->mIConvertToIntegerFunc = convert_double_to_integer;
      break;
   case INT4SCOMPLEX:
   case FLT8COMPLEX:
   default:
      delete pImpl;
      pImpl = NULL;
      break;
   }
   switch (pDesc->getClassificationDataType())
   {
   case INT1SBYTE:
      pImpl->mCConvertToDoubleFunc = convert_s1byte_to_double;
      pImpl->mCConvertToIntegerFunc = convert_s1byte_to_integer;
      break;
   case INT1UBYTE:
      pImpl->mCConvertToDoubleFunc = convert_u1byte_to_double;
      pImpl->mCConvertToIntegerFunc = convert_u1byte_to_integer;
      break;
   case INT2SBYTES:
      pImpl->mCConvertToDoubleFunc = convert_s2byte_to_double;
      pImpl->mCConvertToIntegerFunc = convert_s2byte_to_integer;
      break;
   case INT2UBYTES:
      pImpl->mCConvertToDoubleFunc = convert_u2byte_to_double;
      pImpl->mCConvertToIntegerFunc = convert_u2byte_to_integer;
      break;
   case INT4SBYTES:
      pImpl->mCConvertToDoubleFunc = convert_s4byte_to_double;
      pImpl->mCConvertToIntegerFunc = convert_s4byte_to_integer;
      break;
   case INT4UBYTES:
      pImpl->mCConvertToDoubleFunc = convert_u4byte_to_double;
      pImpl->mCConvertToIntegerFunc = convert_u4byte_to_integer;
      break;
   case FLT4BYTES:
      pImpl->mCConvertToDoubleFunc = convert_float_to_double;
      pImpl->mCConvertToIntegerFunc = convert_float_to_integer;
      break;
   case FLT8BYTES:
      pImpl->mCConvertToDoubleFunc = convert_double_to_double;
      pImpl->mCConvertToIntegerFunc = convert_double_to_integer;
      break;
   case INT4SCOMPLEX:
   case FLT8COMPLEX:
   default:
      delete pImpl;
      pImpl = NULL;
      break;
   }

   return PointCloudAccessor(pDeleter, pImpl);
}

PointCloudAccessor PointCloudElementImp::getPointCloudAccessor(PointCloudDataRequest *pRequestIn) const
{
   if (pRequestIn != NULL)
   {
      if (pRequestIn->getWritable())
      {
         // can't get a writable accessor to a const PointCloudElement
         FactoryResource<PointCloudDataRequest> pRequest(pRequestIn); // destroy the request
         return PointCloudAccessor(NULL, NULL);
      }
   }
   return const_cast<PointCloudElementImp*>(this)->getPointCloudAccessor(pRequestIn);
}

bool PointCloudElementImp::toXml(XMLWriter* pXml) const
{
   // Cannot be represented in XML format
   return false;
}

bool PointCloudElementImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   // Cannot be represented in XML format
   return false;
}

const string& PointCloudElementImp::getObjectType() const
{
   static string sType("PointCloudElementImp");
   return sType;
}

bool PointCloudElementImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PointCloudElement"))
   {
      return true;
   }

   return DataElementImp::isKindOf(className);
}

bool PointCloudElementImp::isKindOfElement(const string& className)
{
   if ((className == "PointCloudElementImp") || (className == "PointCloudElement"))
   {
      return true;
   }

   return DataElementImp::isKindOfElement(className);
}

void PointCloudElementImp::getElementTypes(vector<string>& classList)
{
   classList.push_back("PointCloudElement");
   DataElementImp::getElementTypes(classList);
}

const void* PointCloudElementImp::getRawData() const
{
   return const_cast<PointCloudElementImp*>(this)->getRawData();
}

void* PointCloudElementImp::getRawData()
{
   return mpInMemoryData;
}
