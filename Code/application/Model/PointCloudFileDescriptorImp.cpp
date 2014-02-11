/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "FileDescriptor.h"
#include "PointCloudFileDescriptorImp.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

PointCloudFileDescriptorImp::PointCloudFileDescriptorImp() :
   mPointCount(0)
{
}

PointCloudFileDescriptorImp::~PointCloudFileDescriptorImp()
{
}

uint32_t PointCloudFileDescriptorImp::getPointCount() const
{
   return mPointCount;
}

void PointCloudFileDescriptorImp::setPointCount(uint32_t pointTotal)
{
   if (mPointCount != pointTotal)
   {
      mPointCount = pointTotal;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

PointCloudFileDescriptorImp& PointCloudFileDescriptorImp::operator =(const PointCloudFileDescriptorImp& descriptor)
{
   if (this != &descriptor)
   {
      FileDescriptorImp::clone(dynamic_cast<const FileDescriptor*>(&descriptor));

      mPointCount = descriptor.mPointCount;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

void PointCloudFileDescriptorImp::addToMessageLog(Message* pMessage) const
{
   FileDescriptorImp::addToMessageLog(pMessage);

   if (pMessage == NULL)
   {
      return;
   }
   pMessage->addProperty("Point Count", getPointCount());
}

bool PointCloudFileDescriptorImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   bool bSuccess = FileDescriptorImp::toXml(pXml);
   if (bSuccess == true)
   {
      pXml->addAttr("type", "PointCloudFileDescriptor");
      pXml->addAttr("pointCount", mPointCount);
   }

   return bSuccess;
}

bool PointCloudFileDescriptorImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   VERIFY(pDocument != NULL);

   bool success = FileDescriptorImp::fromXml(pDocument, version);
   if (!success)
   {
      return false;
   }

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);

   XmlReader::StringStreamAssigner<uint32_t> parser;
   mPointCount = parser(A(pElement->getAttribute(X("pointCount"))));

   notify(SIGNAL_NAME(Subject, Modified));
   return success;
}

const string& PointCloudFileDescriptorImp::getObjectType() const
{
   static string sType("PointCloudFileDescriptorImp");
   return sType;
}

bool PointCloudFileDescriptorImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PointCloudFileDescriptor"))
   {
      return true;
   }

   return FileDescriptorImp::isKindOf(className);
}

void PointCloudFileDescriptorImp::getFileDescriptorTypes(vector<string>& classList)
{
   classList.push_back("PointCloudFileDescriptor");
   FileDescriptorImp::getFileDescriptorTypes(classList);
}

bool PointCloudFileDescriptorImp::isKindOfFileDescriptor(const string& className)
{
   if ((className == "PointCloudFileDescriptorImp") || (className == "PointCloudFileDescriptor"))
   {
      return true;
   }

   return FileDescriptorImp::isKindOfFileDescriptor(className);
}
