/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VALUE_H
#define VALUE_H

#include "DataVariant.h"

#include <string>

class Value
{
public:
   Value();
   Value(const std::string& itemName, const std::string& nodeName, const std::string& nodeType,
      const DataVariant& value);
   ~Value();

   void setItemName(const std::string& itemName);
   const std::string& getItemName() const;
   void setNodeName(const std::string& nodeName);
   const std::string& getNodeName() const;
   void setNodeType(const std::string& nodeType);
   const std::string& getNodeType() const;
   void setValue(const DataVariant& value);
   const DataVariant& getValue() const;

private:
   std::string mItemName;
   std::string mNodeName;
   std::string mNodeType;
   DataVariant mValue;
};

#endif
