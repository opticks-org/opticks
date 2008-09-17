/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATA_VALUE_WRAPPER_H
#define DATA_VALUE_WRAPPER_H

#include <string>
#include <typeinfo>

#include "DataVariant.h"
#include "XercesIncludes.h"

using XERCES_CPP_NAMESPACE_QUALIFIER DOMNode;
class XMLWriter;

class DataValueWrapper
{
public:
   virtual ~DataValueWrapper()
   {
   }

   virtual void* getValue() = 0;
   virtual const std::type_info & getType() const = 0;
   virtual std::string getTypeName() const = 0;
   virtual std::string toXmlString(DataVariant::Status* pStatus) const = 0;
   virtual DataVariant::Status fromXmlString(const std::string &text) = 0;
   virtual std::string toDisplayString(DataVariant::Status* pStatus) const = 0;
   virtual DataVariant::Status fromDisplayString(const std::string &text) = 0;
   virtual bool toXml(XMLWriter* pWriter) const = 0;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version) = 0;
   virtual DataValueWrapper* copy() const = 0;
   virtual bool operator==(const DataValueWrapper &rhs) const = 0;
};

#endif
