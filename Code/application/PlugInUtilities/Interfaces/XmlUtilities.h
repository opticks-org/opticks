/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef XMLUTILITIES_H
#define XMLUTILITIES_H

#include "DimensionDescriptor.h"
#include "XercesIncludes.h"
#include "xmlwriter.h"

#include <string>
#include <vector>

/**
 * This namespace contains convenience functions for dealing with serialization/deserialization of
 * dimension descriptors.  This includes serialization/deserialization of vectors of dimension descriptors.
 */
namespace XmlUtilities
{
   /**
    * Serializes a DimensionDescriptor.
    *
    * @param desc
    *        The DimensionDescriptor to be serialized.
    * @param pXml
    *        Pointer to XMLWriter.
    */
   void serializeDimensionDescriptor(const DimensionDescriptor& desc, XMLWriter* pXml);

   /**
    * Deserializes a DimensionDescriptor.
    *
    * @param desc
    *        The DimensionDescriptor to be populated during the deserialization.
    * @param pNode
    *        Pointer to XML node containing the dimension descriptor information to be deserialized.
    */
   void deserializeDimensionDescriptor(DimensionDescriptor& desc, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNode);

   /**
    * Serializes a vector of DimensionDescriptors.
    *
    * @param elementName
    *        The name of the XML element to contain information for the dimension descriptors.
    * @param desc
    *        The vector of DimensionDescriptors to be serialized.
    * @param pXml
    *        Pointer to XMLWriter.
    */
   void serializeDimensionDescriptors(const std::string& elementName,
      const std::vector<DimensionDescriptor>& desc, XMLWriter* pXml);

   /**
    * Deserializes a vector of DimensionDescriptors.
    *
    * @param elementName
    *        The name of the XML element containing the information for the dimension descriptors.
    * @param desc
    *        The vector of DimensionDescriptors to be populated during the deserialization.
    * @param pNode
    *        Pointer to XML node containing the dimension descriptor information to be deserialized.
    */
   void deserializeDimensionDescriptors(const std::string& elementName,
      std::vector<DimensionDescriptor>& desc, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNode);
};

#endif
