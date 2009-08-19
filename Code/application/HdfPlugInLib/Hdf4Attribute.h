/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF4ATTRIBUTE_H
#define HDF4ATTRIBUTE_H

#include <string>

#include "DataVariant.h"
#include "TypeConverter.h"

/**
 * The HDF Attribute class is designed to provide an abstraction for the HDF Attribute data type.
 * Attributes are small metadata elements that are associated with an Hdf4Dataset.
 *
 * The Hdf4Attribute class CANNOT be created by itself. Only an Hdf4Element can create or destroy
 * them.
  * Currently supported are all basic types and vectors of basic types. Any other types must have their
 * memory allocated manually. Examples:
 *
 * @code
 * // assume pDataset exists and is a member of some Hdf4Group
 * // create a complex attribute
 * Hdf4Attribute* pAttr = pDataset->addAttribute("complex", 2);
 * IntegerComplex* pCompl = new IntegerComplex[2];
 * // set pCompl
 * pAttr->setValue(pCompl); // pAttr now owns this memory
 * @endcode
 *
 * @code
 * // assume pDataset exists and is a member of some Hdf4Group
 * // create a vector<int> attribute
 * // since this is a built-in type, the attribute allocates the memory for us
 * Hdf4Attribute* pAttr = pDataset->addAttribute("vector<int>", 2);
 * int* pVector = pAttr->getFirstValue(); // get a pointer to the first value in the vector and start writing
 * fread(pVector, sizeof(int), 2, pInputFile); // read directly from file into attribute
 * @endcode
 *
 * @see Hdf4Dataset::addAttribute(), Hdf4File::addAttribute()
 */
class Hdf4Attribute
{
public:
   /**
    * Gets the attribute name.
    *
    * @return  Returns the name of the attribute.
    */
   virtual const std::string& getName() const;

   /**
    * Sets the value of the attribute.
    *
    * @param  var
    *         A variant holding the attribute value.
    */
   virtual void setVariant(const DataVariant &var);

   /**
    * Returns a reference to the object.
    *
    * @return Returns a reference to the member value representing the attribute.
    */
   virtual const DataVariant &getVariant() const;

   /**
    * Returns a reference to the object.
    *
    * @return Returns a reference to the member value representing the attribute.
    */
   virtual DataVariant &getVariant();

   /**
    * Retrieves the value.
    *
    * @return true if the value was successfully retrieved or valse otherwise.
    */
   template<class T>
   bool getValueAs(T& value) const
   {
      return getVariant().getValue(value);
   }

   /**
    * Returns a pointer to the value or NULL if the type does not match.
    *
    * @return Returns the pointer value representing the attribute.
    */
   template<class T>
   const T* getValueAsPointer() const
   {
      return getVariant().getPointerToValue<T*>();
   }

protected:
   /**
    * Creates an attribute with a given type and number of elements.
    *
    * This method cannot be called directly. Use Hdf4Element::addAttribute()
    * to add an attribute to an Hdf4Element.
    *
    * @param  name
    *         The name of the attribute.
    * @param  value
    *         The current value of the attribute.
    */
   Hdf4Attribute(const std::string& name, const DataVariant& value);

   /**
    * Destroys an Hdf4Attribute object.
    *
    * Use Hdf4Element::removeAttribute() to delete an Hdf4Attribute from an Hdf4Element.
    */
   ~Hdf4Attribute();

   /* Only Hdf4Element can create or destroy Hdf4Attribute types.
    */
   friend class Hdf4Element;

private:
   std::string mName;
   DataVariant mValue;
};

#endif
