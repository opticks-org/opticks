/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef __SERILZBLIMP_H
#define __SERILZBLIMP_H

#include "Serializable.h"

#include <stdio.h>
#include <string>
#include <vector>

bool SerializeString(FILE* fp, const std::string& source);
bool DeserializeString(FILE* fp, std::string& target);
bool SerializeStringVector(FILE* fp, std::vector<std::string>& aList);
bool DeserializeStringVector(FILE* fp, std::vector<std::string>& theList);

#define SERIALIZABLEADAPTEREXTENSION_CLASSES

#define SERIALIZABLEADAPTER_METHODS(impClass) \
   bool toXml(XMLWriter* pXml) const \
   { \
      return impClass::toXml(pXml); \
   } \
   bool fromXml(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pDocument, unsigned int version) \
   { \
      return impClass::fromXml(pDocument, version); \
   } 

#endif
