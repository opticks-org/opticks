/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Value.h"

using namespace std;

Value::Value()
{
}

Value::Value(const string& itemName, const string& nodeName, const string& nodeType, const DataVariant& value) :
   mItemName(itemName),
   mNodeName(nodeName),
   mNodeType(nodeType),
   mValue(value)
{
}

Value::~Value()
{
}

void Value::setItemName(const string& itemName)
{
   mItemName = itemName;
}

const string& Value::getItemName() const
{
   return mItemName;
}

void Value::setNodeName(const string& nodeName)
{
   mNodeName = nodeName;
}

const string& Value::getNodeName() const
{
   return mNodeName;
}

void Value::setNodeType(const string& nodeType)
{
   mNodeType = nodeType;
}

const string& Value::getNodeType() const
{
   return mNodeType;
}

void Value::setValue(const DataVariant& value)
{
   mValue = value;
}

const DataVariant& Value::getValue() const
{
   return mValue;
}
