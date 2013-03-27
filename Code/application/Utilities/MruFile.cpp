/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ImportDescriptorImp.h"
#include "ModelServices.h"
#include "MruFile.h"
#include "xmlreader.h"
#include "xmlwriter.h"

MruFile::MruFile()
{}

MruFile::MruFile(const std::string& name, const std::string& importerName,
   const std::vector<ImportDescriptor*>& descriptors, const DateTimeImp& modificationTime) :
   mName(name),
   mImporterName(importerName),
   mDescriptors(descriptors),
   mModificationTime(modificationTime)
{}

MruFile::~MruFile()
{
   for (std::vector<ImportDescriptor*>::iterator iter = mDescriptors.begin(); iter != mDescriptors.end(); ++iter)
   {
      // Equivalent to calling "ModelServices::destroyImportDescriptor(*iter)", which is not called
      // because this method can be called from ConfigurationSettingsImp::~ConfigurationSettingsImp()
      // which is destroyed after ModelServices and causes a std::logic_error to be thrown.
      ImportDescriptorImp* pDescriptorImp = dynamic_cast<ImportDescriptorImp*>(*iter);
      if (pDescriptorImp != NULL)
      {
         delete pDescriptorImp;
      }
   }
}

bool MruFile::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL || mName.empty() || mImporterName.empty() || mDescriptors.empty())
   {
      return false;
   }
   pXml->addAttr("name", mName);
   pXml->addAttr("importer", mImporterName);
   pXml->addAttr("version", XmlBase::VERSION);

   for (std::vector<ImportDescriptor*>::const_iterator descriptorIter = mDescriptors.begin();
      descriptorIter != mDescriptors.end();
      ++descriptorIter)
   {
      ImportDescriptor* pImportDescriptor = *descriptorIter;
      if (pImportDescriptor != NULL)
      {
         DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
         if (pDescriptor != NULL)
         {
            pXml->pushAddPoint(pXml->addElement("DataDescriptor"));
            pDescriptor->toXml(pXml);
            pXml->popAddPoint();
         }
      }
   }

   const DateTime* pDateTime = &(mModificationTime);
   std::string modificationText = StringUtilities::toXmlString(pDateTime);
   pXml->addAttr("modification_time", modificationText);

   return true;
}

bool MruFile::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   // Clear descriptors.
   Service<ModelServices> pModel;
   for (std::vector<ImportDescriptor*>::iterator iter = mDescriptors.begin(); iter != mDescriptors.end(); ++iter)
   {
      pModel->destroyImportDescriptor(dynamic_cast<ImportDescriptorImp*>(*iter));
   }
   mDescriptors.clear();

   for (XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pDescriptorNode = pDocument->getFirstChild();
      pDescriptorNode != NULL;
      pDescriptorNode = pDescriptorNode->getNextSibling())
   {
      if (XERCES_CPP_NAMESPACE_QUALIFIER XMLString::equals(pDescriptorNode->getNodeName(),
         X("DataDescriptor")))
      {
         XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pDescriptorElement =
            static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pDescriptorNode);
         if (!(pDescriptorElement->hasAttribute(X("type")) && pDescriptorElement->hasAttribute(X("version"))))
         {
            continue;
         }
         std::string mruName = A(pDescriptorElement->getAttribute(X("name")));
         std::string type = A(pDescriptorElement->getAttribute(X("type")));
         DataDescriptor* pDescriptor = pModel->createDataDescriptor(mruName, type, NULL);
         if (pDescriptor == NULL)
         {
            continue;
         }
         try
         {
            if (pDescriptor->fromXml(pDescriptorElement, version))
            {
               ImportDescriptor* pImportDescriptor = pModel->createImportDescriptor(pDescriptor);
               if (pImportDescriptor != NULL)
               {
                  mDescriptors.push_back(pImportDescriptor);
               }
            }
         }
         catch (const XmlBase::XmlException&)
         {}
      }
   }

   XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pElement =
      static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pDocument);
   mName = A(pElement->getAttribute(X("name")));
   mImporterName = A(pElement->getAttribute(X("importer")));
   return mModificationTime.set(A(pElement->getAttribute(X("modification_time"))));
}

const std::string& MruFile::getName() const
{
   return mName;
}

const std::string& MruFile::getImporterName() const
{
   return mImporterName;
}

const std::vector<ImportDescriptor*>& MruFile::getDescriptors() const
{
   return mDescriptors;
}

const DateTimeImp& MruFile::getModificationTime() const
{
   return mModificationTime;
}
