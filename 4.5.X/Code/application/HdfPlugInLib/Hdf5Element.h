/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5ELEMENT_H
#define HDF5ELEMENT_H

#include "Hdf5Node.h"

#include <map>
#include <string>
#include <vector>

class DataVariant;
class Hdf5Attribute;
class Hdf5Node;

/**
 * An Hdf5Element is the most basic base class of an HDF component.
 *
 * It provides access to the name of the item, as well as its attributes.
 */
class Hdf5Element : public Hdf5Node
{
public:
   /**
    * The type of container that is used internally to store and manage Hdf5Attributes.
    */
   typedef std::map<std::string, Hdf5Attribute*> AttributeContainer;

   /**
    * Creates an attribute with a given type and number of elements.
    *
    * For supported types, see DynamicObject::set().
    *
    * @param  name
    *         The name of the attribute.
    * @param  value
    *         The current value of the attribute.
    * @return Returns a new Hdf5Attribute with the given name and value.
    */
   Hdf5Attribute* addAttribute(const std::string& name, const DataVariant& value);

   /**
    * Gets the element's attributes.
    *
    * @return Returns the member vector of attributes.
    *         NOTE: You cannot call delete on an Hdf5Attribute since its destructor is protected.
    */
   const AttributeContainer& getAttributes() const;

   /**
    * Gets an attribute based on its name.
    *
    * @param  name
    *         The name of the attribute to fetch.
    *
    * @return Returns a pointer to the Hdf5Attribute with the matching name. If multiple attributes
    *         with that name exist, the first one in the vector of attributes is returned.
    *         NOTE: You cannot call delete on this pointer since the Hdf5Attribute destructor is protected.
    */
   Hdf5Attribute* getAttribute(const std::string& name) const;

   /**
    * Gets the number of attributes.
    *
    * @return The number of attributes in the element. Equivalent to calling getAttributes().size().
    */
   size_t getNumAttributes() const;

   /**
    * Removes an attribute from the %Hdf5Element.
    *
    * @param  name
    *         The name of the attribute to remove from the %Hdf5Element.
    *         This is the only way to manually delete an attribute from this object
    *         since Hdf5Attribute::~Hdf5Attribute is protected.
    *
    * @return Returns true if the operation was successful. False, otherwise.
    */
   bool removeAttribute(const std::string& name);

   /**
    * Destroys an Hdf5Element and all of its attributes.
    */
   virtual ~Hdf5Element();

protected:
   /**
    * Creates an Hdf5Element object, which is the most basic type of HDF object.
    *
    * @param pParent
    *        The parent node of this element.
    * @param name
    *        The name of the HDF object.
    */
   Hdf5Element(Hdf5Node* pParent, const std::string& name);

private:
   AttributeContainer mAttributes;
};

#endif
