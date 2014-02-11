/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDELEMENTIMP_H
#define POINTCLOUDELEMENTIMP_H

#include "DataElementImp.h"
#include "PointCloudAccessor.h"
#include "PointCloudAccessorImpl.h"

class PointCloudPager;

class PointCloudElementImp : public DataElementImp
{
public:
   PointCloudElementImp(const DataDescriptorImp& descriptor, const std::string& id);
   ~PointCloudElementImp();

   virtual uint32_t getArrayCount();
   virtual void updateData(uint32_t updateMask);
   virtual bool setPager(PointCloudPager* pPager);
   virtual PointCloudPager* getPager() const;
   virtual bool createInMemoryPager() = 0;
   virtual bool createDefaultPager() = 0;
   virtual PointCloudAccessor getPointCloudAccessor(PointCloudDataRequest *pRequestIn=NULL);
   virtual PointCloudAccessor getPointCloudAccessor(PointCloudDataRequest *pRequestIn=NULL) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfElement(const std::string& className);
   static void getElementTypes(std::vector<std::string>& classList);

   class Deleter : public PointCloudAccessorDeleter
   {
      void operator()(PointCloudAccessorImpl* pCloudAccessor);
   };

   const void *getRawData() const;
   void *getRawData();

private:
   bool createMemoryMappedPagerForNewTempFile();

   uint32_t mArrayCount;
   void createData();
   std::string mTempFilename;

   char* mpInMemoryData;
   PointCloudPager* mpPager;

   mutable bool mModified;
};

#define POINTCLOUDELEMENTADAPTEREXTENSION_CLASSES \
   DATAELEMENTADAPTEREXTENSION_CLASSES

#define POINTCLOUDELEMENTADAPTER_METHODS(impClass) \
   DATAELEMENTADAPTER_METHODS(impClass) \
   uint32_t getArrayCount() \
   { \
      return impClass::getArrayCount(); \
   } \
   void updateData(uint32_t updateMask) \
   { \
      return impClass::updateData(updateMask); \
   } \
   bool setPager(PointCloudPager* pPager) \
   { \
      return impClass::setPager(pPager); \
   } \
   PointCloudPager* getPager() const \
   { \
      return impClass::getPager(); \
   } \
   bool createInMemoryPager() \
   { \
      return impClass::createInMemoryPager(); \
   } \
   bool createDefaultPager() \
   { \
      return impClass::createDefaultPager(); \
   } \
   PointCloudAccessor getPointCloudAccessor(PointCloudDataRequest *pRequest=NULL) \
   { \
      return impClass::getPointCloudAccessor(pRequest); \
   } \
   PointCloudAccessor getPointCloudAccessor(PointCloudDataRequest *pRequest=NULL) const \
   { \
      return impClass::getPointCloudAccessor(pRequest); \
   }
#endif