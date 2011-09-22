/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "Hdf5Attribute.h"
#include "Hdf5Element.h"
#include "Hdf5File.h"

#include <string>

using namespace std;

Hdf5Element::Hdf5Element(Hdf5Node* pParent, const std::string& name) : 
Hdf5Node(pParent, name)
{
}

Hdf5Element::~Hdf5Element()
{
   // Delete the attributes
   AttributeContainer::iterator attributeIter = mAttributes.begin();
   while (attributeIter != mAttributes.end())
   {
      Hdf5Attribute* pAttribute = attributeIter->second;
      delete pAttribute; // delete handles NULL values OK
      ++attributeIter;
   }
   mAttributes.clear();
}

Hdf5Attribute* Hdf5Element::addAttribute(const string& name, const DataVariant& value)
{
   AttributeContainer::iterator found = mAttributes.find(name);
   Hdf5Attribute* pAttribute = NULL;
   if (found == mAttributes.end())
   {
      pAttribute = new Hdf5Attribute(this, name, value);
      if (pAttribute != NULL)
      {
         mAttributes.insert(AttributeContainer::value_type(name, pAttribute));
      }
   }
   return pAttribute;
}

const Hdf5Element::AttributeContainer& Hdf5Element::getAttributes() const
{
   return mAttributes;
}

Hdf5Attribute* Hdf5Element::getAttribute(const string& name) const
{
   if (name.empty() == true)
   {
      return NULL;
   }

   Hdf5Attribute* pAttribute = NULL;
   AttributeContainer::const_iterator it = mAttributes.find(name);
   if (it != mAttributes.end())
   {
      pAttribute = it->second;
   }
   return pAttribute;
}

size_t Hdf5Element::getNumAttributes() const
{
   return mAttributes.size();
}

bool Hdf5Element::removeAttribute(const string& name)
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
