/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDFILEDESCRIPTORIMP_H
#define POINTCLOUDFILEDESCRIPTORIMP_H

#include "FileDescriptorImp.h"

class PointCloudFileDescriptorImp : public FileDescriptorImp
{
public:
   PointCloudFileDescriptorImp();
   ~PointCloudFileDescriptorImp();

   virtual uint32_t getPointCount() const;
   virtual void setPointCount(uint32_t pointTotal);

   PointCloudFileDescriptorImp& operator =(const PointCloudFileDescriptorImp& descriptor);

   void addToMessageLog(Message* pMessage) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getFileDescriptorTypes(std::vector<std::string>& classList);
   static bool isKindOfFileDescriptor(const std::string& className);

private:
   uint32_t mPointCount;
};

#define POINTCLOUDFILEDESCRIPTORADAPTEREXTENSION_CLASSES \
   FILEDESCRIPTORADAPTEREXTENSION_CLASSES

#define POINTCLOUDFILEDESCRIPTORADAPTER_METHODS(impClass) \
   FILEDESCRIPTORADAPTER_METHODS(impClass) \
   uint32_t getPointCount() const \
   { \
       return impClass::getPointCount(); \
   } \
   void setPointCount(uint32_t pointTotal) \
   { \
       impClass::setPointCount(pointTotal); \
   } \

#endif
