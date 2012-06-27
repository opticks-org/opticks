/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "xmlreader.h"
#include "XmlUtilities.h"


void XmlUtilities::serializeDimensionDescriptor(const DimensionDescriptor& desc, XMLWriter* pXml)
{
   // This method is meant for a single DimensionDescriptor and does not serialize "count" or "skipFactor" attributes.
   VERIFYNRV(pXml != NULL);
   if (desc.isOriginalNumberValid())
   {
      pXml->addAttr("originalNumber", desc.getOriginalNumber());
   }
   if (desc.isOnDiskNumberValid())
   {
      pXml->addAttr("onDiskNumber", desc.getOnDiskNumber());
   }
   if (desc.isActiveNumberValid())
   {
      pXml->addAttr("activeNumber", desc.getActiveNumber());
   }
}

void XmlUtilities::deserializeDimensionDescriptor(DimensionDescriptor& desc,
   XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNode)
{
   // This method is meant for a single DimensionDescriptor and does not deserialize "count" or "skipFactor" attributes.
   VERIFYNRV(pNode != NULL);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pElement = static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pNode);
   XmlReader::StringStreamAssigner<unsigned int> valueParser;
   if (pElement->hasAttribute(X("originalNumber")))
   {
      desc.setOriginalNumber(valueParser(A(pElement->getAttribute(X("originalNumber")))));
   }
   if (pElement->hasAttribute(X("onDiskNumber")))
   {
      desc.setOnDiskNumber(valueParser(A(pElement->getAttribute(X("onDiskNumber")))));
   }
   if (pElement->hasAttribute(X("activeNumber")))
   {
      desc.setActiveNumber(valueParser(A(pElement->getAttribute(X("activeNumber")))));
   }
}

void XmlUtilities::serializeDimensionDescriptors(const std::string& elementName,
   const std::vector<DimensionDescriptor>& desc, XMLWriter* pXml)
{
   VERIFYNRV(pXml != NULL);
   for (std::vector<DimensionDescriptor>::const_iterator iter = desc.begin(); iter != desc.end(); ++iter)
   {
      pXml->pushAddPoint(pXml->addElement(elementName));
      XmlUtilities::serializeDimensionDescriptor(*iter, pXml);

      // Determine how many contiguous descriptors with a constant skip factor exist.
      unsigned int count = 1;
      unsigned int skipFactor = 0;
      bool skipFactorValid = false;
      for (std::vector<DimensionDescriptor>::const_iterator iter2 = iter + 1; iter2 != desc.end(); ++iter, ++iter2)
      {
         // Ensure that precisely the same set of numbers is defined.
         if ((iter->isOriginalNumberValid() != iter2->isOriginalNumberValid()) ||
            (iter->isOnDiskNumberValid() != iter2->isOnDiskNumberValid()) ||
            (iter->isActiveNumberValid() != iter2->isActiveNumberValid()))
         {
            break;
         }

         if (iter->isOriginalNumberValid())
         {
            // Ensure that the numbers are ascending.
            if (iter2->getOriginalNumber() <= iter->getOriginalNumber())
            {
               break;
            }

            // Ensure that the difference of the numbers matches the current skip factor.
            unsigned int delta = iter2->getOriginalNumber() - iter->getOriginalNumber() - 1;
            if (skipFactorValid == false)
            {
               skipFactor = delta;
               skipFactorValid = true;
            }
            else if (skipFactor != delta)
            {
               break;
            }
         }
         if (iter->isOnDiskNumberValid())
         {
            // Ensure that the numbers are ascending.
            if (iter2->getOnDiskNumber() <= iter->getOnDiskNumber())
            {
               break;
            }

            // Ensure that the difference of the numbers matches the current skip factor.
            unsigned int delta = iter2->getOnDiskNumber() - iter->getOnDiskNumber() - 1;
            if (skipFactorValid == false)
            {
               skipFactor = delta;
               skipFactorValid = true;
            }
            else if (skipFactor != delta)
            {
               break;
            }
         }
         if (iter->isActiveNumberValid())
         {
            // Ensure that the numbers are ascending.
            if (iter2->getActiveNumber() <= iter->getActiveNumber())
            {
               break;
            }

            // Ensure that the difference of the numbers matches the current skip factor.
            unsigned int delta = iter2->getActiveNumber() - iter->getActiveNumber() - 1;
            if (skipFactorValid == false)
            {
               skipFactor = delta;
               skipFactorValid = true;
            }
            else if (skipFactor != delta)
            {
               break;
            }
         }

         ++count;
      }

      if (count > 1)
      {
         pXml->addAttr("count", count);
         if (skipFactor > 0)
         {
            pXml->addAttr("skipFactor", skipFactor);
         }
      }

      pXml->popAddPoint();
   }
}

void XmlUtilities::deserializeDimensionDescriptors(const std::string& elementName,
   std::vector<DimensionDescriptor>& desc, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNode)
{
   VERIFYNRV(pNode != NULL);

   // Since this uses a vector, for performance, compute how many descriptors will be created and pre-allocate memory.
   unsigned int numDescriptors = 0;
   for (DOMNode* pChild = pNode->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XERCES_CPP_NAMESPACE_QUALIFIER XMLString::equals(pChild->getNodeName(), X(elementName.c_str())))
      {
         unsigned int count = 1;
         XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pElement =
            static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pChild);
         XmlReader::StringStreamAssigner<unsigned int> valueParser;
         if (pElement->hasAttribute(X("count")))
         {
            count = valueParser(A(pElement->getAttribute(X("count"))));
            if (count == 0)
            {
               count = 1;
            }
         }

         numDescriptors += count;
      }
   }
   desc.reserve(numDescriptors);

   for (DOMNode* pChild = pNode->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XERCES_CPP_NAMESPACE_QUALIFIER XMLString::equals(pChild->getNodeName(), X(elementName.c_str())))
      {
         DimensionDescriptor descriptor;
         XmlUtilities::deserializeDimensionDescriptor(descriptor, pChild);

         unsigned int count = 1;
         unsigned int skipFactor = 0;

         XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pElement =
            static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pChild);
         XmlReader::StringStreamAssigner<unsigned int> valueParser;
         if (pElement->hasAttribute(X("count")))
         {
            count = valueParser(A(pElement->getAttribute(X("count"))));
            if (count == 0)
            {
               count = 1;
            }

            if (count > 1 && pElement->hasAttribute(X("skipFactor")))
            {
               skipFactor = valueParser(A(pElement->getAttribute(X("skipFactor"))));
            }
         }

         unsigned int originalNumber = descriptor.getOriginalNumber();
         unsigned int onDiskNumber = descriptor.getOnDiskNumber();
         unsigned int activeNumber = descriptor.getActiveNumber();
         for (unsigned int i = 0; i < count; ++i)
         {
            desc.push_back(descriptor);
            if (descriptor.isOriginalNumberValid())
            {
               originalNumber += skipFactor + 1;
               descriptor.setOriginalNumber(originalNumber);
            }
            if (descriptor.isOnDiskNumberValid())
            {
               onDiskNumber += skipFactor + 1;
               descriptor.setOnDiskNumber(onDiskNumber);
            }
            if (descriptor.isActiveNumberValid())
            {
               activeNumber += skipFactor + 1;
               descriptor.setActiveNumber(activeNumber);
            }
         }
      }
   }

   // Sanity check -- if this verify is ever triggered, investigate the algorithm above for correctness.
   VERIFYNRV(numDescriptors == desc.size());
}
