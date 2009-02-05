/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "Hdf4Attribute.h"
#include "Hdf4Element.h"

using namespace std;

Hdf4Element::Hdf4Element(const std::string& name) : mName(name)
{
}

Hdf4Element::~Hdf4Element()
{
   // Delete the attributes
   AttributeContainer::iterator attributeIter = mAttributes.begin();
   while (attributeIter != mAttributes.end())
   {
      Hdf4Attribute* pAttribute = attributeIter->second;
      delete pAttribute; // delete handles NULL values OK
      ++attributeIter;
   }
   mAttributes.clear();
}

const std::string& Hdf4Element::getName() const
{
   return mName;
}

Hdf4Attribute* Hdf4Element::addAttribute(const string& name, const DataVariant& value)
{
   AttributeContainer::iterator found = mAttributes.find(name);
   Hdf4Attribute* pAttribute = NULL;
   if (found == mAttributes.end())
   {
      pAttribute = new Hdf4Attribute(name, value);
      if (pAttribute != NULL)
      {
         mAttributes.insert(AttributeContainer::value_type(name, pAttribute));
      }
   }
   return pAttribute;
}

const Hdf4Element::AttributeContainer& Hdf4Element::getAttributes() const
{
   return mAttributes;
}

Hdf4Attribute* Hdf4Element::getAttribute(const string& name) const
{
   if (name.empty() == true)
   {
      return NULL;
   }

   Hdf4Attribute* pAttribute = NULL;
   AttributeContainer::const_iterator it = mAttributes.find(name);
   if (it != mAttributes.end())
   {
      pAttribute = it->second;
   }
   return pAttribute;
}

size_t Hdf4Element::getNumAttributes() const
{
   return mAttributes.size();
}

bool Hdf4Element::removeAttribute(const string& name)
{
   AttributeContainer::iterator iter = mAttributes.find(name);
   bool found = iter != mAttributes.end();
   if (found == true)
   {
      delete iter->second;
      mAttributes.erase(iter);
   }
   return found;
}
