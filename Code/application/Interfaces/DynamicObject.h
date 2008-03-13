/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DYNAMICOBJECT_H
#define DYNAMICOBJECT_H

#include "DataVariantValidator.h"
#include "Subject.h"
#include "Serializable.h"

#include <string>
#include <vector>

class DataVariant;

/**
 *  Dynamic extension of class attributes
 *
 *  Dynamic Object refers to a class that allows attributes to be 
 *  added during runtime.  Many classes extend the Dynamic Object
 *  to provide future support for new attributes without changing
 *  the class specification.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setAttribute(), setAttributeByPath(),
 *    adoptAttribute(), adoptAttributeByPath(), merge(), removeAttribute(),
 *    removeAttributeByPath(), clear().
 *  - %Any of the objects within the DynamicObject post a notification.
 *    These notifications will be passed on to any observers of the
 *    DynamicObject
 *  - Everything else documented in Subject.
 */
class DynamicObject : public Subject, public Serializable
{
public:
   /**
    *  Emitted just after a new attribute is added with
    *  boost::any<std::pair<std::string, DataVariant*> > containing the
    *  attribute name and value.
    */
   SIGNAL_METHOD(DynamicObject, AttributeAdded)

   /**
    *  Emitted just after an existing attribute is modified with
    *  boost::any<std::pair<std::string, DataVariant*> > containing the
    *  attribute name and the new value.
    */
   SIGNAL_METHOD(DynamicObject, AttributeModified)

   /**
    *  Emitted just before an existing attribute is removed with
    *  boost::any<std::pair<std::string, DataVariant*> > containing the
    *  name and value of the attribute that will be removed.\   The attribute
    *  value is guaranteed to be valid when the signal is emitted.
    */
   SIGNAL_METHOD(DynamicObject, AttributeRemoved)

   /**
    *  Emitted just before the dynamic object is cleared.\   All existing
    *  attribute values are guaranteed to be valid when the signal is emitted.
    */
   SIGNAL_METHOD(DynamicObject, Cleared)

   /**
    *  Sets attributes from those of another dynamic object.
    *
    *  This method parses the given object's attributes, adding them to or
    *  setting them in this object.  Any existing attributes that are not
    *  contained in the given object are not removed.
    *
    *  @param   pObject
    *           The object from which to set this object's attributes.
    *           Cannot be NULL.
    *
    *  @notify  This method will notify once for each attribute which gets set.
    *           The parameters will depend on the attribute modified.
    *
    *  @see setAttribute()
    */
   virtual void merge(const DynamicObject* pObject) = 0;

   /**
    *  Creates a new attribute or sets an existing dynamic attribute.
    *
    * This method differs from adoptAttribute() in that it does perform a deep
    * copy of the DataVariant. Since a deep copy is performed, this method can
    * be much slower than adoptAttribute(), especially when dealing with DynamicObject values.
    *
    *  @param   name
    *           The name of the attribute to set the value.  If an attribute
    *           with the given name does not exist, an attribute is added.
    *  @param   value
    *           A variant holding the attribute's value.
    *
    *  @return  True if the attributes was successfully created or set,
    *           otherwise false.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see adoptAttribute()
    */
   virtual bool setAttribute(const std::string& name, const DataVariant &value) = 0;

   /**
    *  Creates a new attribute or sets an existing dynamic attribute.
    *
    * This method differs from setAttribute() in that it does \b not perform a deep
    * copy of the DataVariant. Since no deep copy is performed, this method can
    * be much faster than setAttribute(), especially when dealing with DynamicObject values.
    *
    *  @param   name
    *           The name of the attribute to set the value.  If an attribute
    *           with the given name does not exist, an attribute is added.
    *  @param   value
    *           A variant holding the attribute's value. On return, this will contain the value previously stored in the DynamicObject.
    *           If the value did not previously exist in the DynamicObject, then this will contain an invalid DataVariant.
    *
    *  @return  True if the attributes was successfully created or set,
    *           otherwise false.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see setAttribute()
    */
   virtual bool adoptAttribute(const std::string& name, DataVariant &value) = 0;

   /**
    * Sets the value within the DynamicObject hierarchy as described in the path.
    *
    * This method will create DynamicObjects as needed to set the value.
    * This method differs from adoptAttributeByPath() in that it does perform a deep
    * copy of the DataVariant. Since a deep copy is performed, this method can
    * be much slower than adoptAttributeByPath(), especially when dealing with DynamicObject values.
    *
    * @param path
    *        The path of names within names for the DynamicObjects,
    *        separated by '/'.
    *
    * @param value
    *        The value to set into the DynamicObject.
    *
    * @return True if the operation was a success, false otherwise.
    *         This method will fail if there exists a non-DynamicObject
    *         as an intermediate path.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see adoptAttributeByPath()
    */
   virtual bool setAttributeByPath(const std::string& path, const DataVariant &value) = 0;

   /**
    * Sets the value within the DynamicObject hierarchy as described in the path.
    *
    * This method differs from setAttributeByPath() in that it does \b not perform a deep
    * copy of the DataVariant. Since no deep copy is performed, this method can
    * be much faster than setAttributeByPath(), especially when dealing with DynamicObject values.
    *
    * @param path
    *        The path of names within names for the DynamicObjects,
    *        separated by '/'.
    *  @param   value
    *        The value to set into the DynamicObject. On return, this will contain the value previously stored in the DynamicObject.
    *        If the value did not previously exist in the DynamicObject, then this will contain an invalid DataVariant.
    *
    * @return True if the operation was a success, false otherwise.
    *         This method will fail if there exists a non-DynamicObject
    *         as an intermediate path.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see setAttributeByPath()
    */
   virtual bool adoptAttributeByPath(const std::string& path, DataVariant &value) = 0;

   /**
    * Sets the value within the DynamicObject hierarchy as described in the path.
    *
    * This method will create DynamicObjects as needed to set the value.
    * This method differs from adoptAttributeByPath() in that it does perform a deep
    * copy of the DataVariant. Since a deep copy is performed, this method can
    * be much slower than adoptAttributeByPath(), especially when dealing with DynamicObject values.
    *
    * @param pComponents
    *        An array of path components to the desired object as std::strings.  Must end with
    *        END_METADATA_NAME.
    *
    * @param value
    *        The value to set into the DynamicObject.
    *
    * @return True if the operation was a success, false otherwise.
    *         This method will fail if there exists a non-DynamicObject
    *         as an intermediate path.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see adoptAttributeByPath()
    */
   virtual bool setAttributeByPath(const std::string pComponents[], const DataVariant &value) = 0;

   /**
    * Sets the value within the DynamicObject hierarchy as described in the path.
    *
    * This method will create DynamicObjects as needed to set the value.
    * This method differs from setAttributeByPath() in that it does \b not perform a deep
    * copy of the DataVariant. Since no deep copy is performed, this method can
    * be much faster than setAttributeByPath(), especially when dealing with DynamicObject values.
    *
    * @param pComponents
    *        An array of path components to the desired object as std::strings.  Must end with
    *        END_METADATA_NAME.
    *
    *  @param   value
    *        The value to set into the DynamicObject. On return, this will contain the value previously stored in the DynamicObject.
    *        If the value did not previously exist in the DynamicObject, then this will contain an invalid DataVariant.
    *
    * @return True if the operation was a success, false otherwise.
    *         This method will fail if there exists a non-DynamicObject
    *         as an intermediate path.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see setAttributeByPath()
    */
   virtual bool adoptAttributeByPath(const std::string pComponents[], DataVariant &value) = 0;

   /**
    *  Returns an attribute value.
    *
    *  @param   name
    *           The name of the attribute for which to get the value.
    *
    *  @return  A variant holding the attributes value. The variant will be
    *           invalid if the attribute did not exist.
    *
    *  @see     DataVariant::isValid()
    */
   virtual const DataVariant &getAttribute(const std::string& name) const = 0;

   /**
    *  Returns an attribute value.
    *
    *  @param   name
    *           The name of the attribute for which to get the value.
    *
    *  @return  A variant holding the attributes value. The variant will be
    *           invalid if the attribute did not exist.
    *
    *  @see     DataVariant::isValid()
    */
   virtual DataVariant &getAttribute(const std::string& name) = 0;

   /**
    * Get the value within the DynamicObject hierarchy
    * as described in the path.
    *
    * @param path
    *        The path of names within names for the DynamicObject,
    *        separated by '/'.
    *
    *  @return  A variant holding the attributes value. The variant will be
    *           empty if the attribute did not exist.
    */
   virtual const DataVariant &getAttributeByPath(const std::string &path) const = 0;

   /**
    * Get the value within the DynamicObject hierarchy
    * as described in the path.
    *
    * @param path
    *        The path of names within names for the DynamicObject,
    *        separated by '/'.
    *
    *  @return  A variant holding the attributes value. The variant will be
    *           empty if the attribute did not exist.
    */
   virtual DataVariant &getAttributeByPath(const std::string &path) = 0;

    /**
    * Get the value within the DynamicObject hierarchy
    * as described in the path.
    *
    * @param pComponents
    *        An array of path components to the desired object as std::strings.  Must end with
    *        END_METADATA_NAME.
    *
    *  @return  A variant holding the attributes value. The variant will be
    *           empty if the attribute did not exist.
    */
   virtual const DataVariant &getAttributeByPath(const std::string pComponents[]) const = 0;

   /**
    * Get the value within the DynamicObject hierarchy
    * as described in the path.
    *
    * @param pComponents
    *        An array of path components to the desired object as std::strings.  Must end with
    *        END_METADATA_NAME.
    *
    *  @return  A variant holding the attributes value. The variant will be
    *           empty if the attribute did not exist.
    */
   virtual DataVariant &getAttributeByPath(const std::string pComponents[]) = 0;

   /**
    * Retrieves the names of all attributes in the object.
    *
    * @param   attributeNames
    *          A vector of strings that is populated with the attribute names.
    *
    * @see     getNumAttributes()
    * @see     getAttribute()
    */
   virtual void getAttributeNames(std::vector<std::string>& attributeNames) const = 0;

   /**
    * Retrieves the total number of attributes in the object.
    *
    * @return  The number of attributes in the object.
    *
    * @see     getAttributeNames()
    */
   virtual unsigned int getNumAttributes() const = 0;

   /**
    * Removes an attribute from the object.
    *
    * @param   name
    *          The name of the attribute to remove.
    *
    * @return  True if the attribute was was successfully removed from the object,
    *          otherwise false.
    *
    * @notify This method will notify Subject::signalModified.
    *
    * @see     clear()
    */
   virtual bool removeAttribute(const std::string& name) = 0;

   /**
    * Removes an attribute from the object.
    *
    * @param pComponents
    *        An array of path components to the desired object as std::strings.  Must end with
    *        END_METADATA_NAME.
    *
    * @return  True if the attribute was was successfully removed from the object,
    *          otherwise false.
    *
    * @notify This method will notify Subject::signalModified.
    *
    * @see     clear()
    */
   virtual bool removeAttributeByPath(const std::string pComponents[]) = 0;

   /**
    * Removes an attribute from the object.
    *
    * @param path
    *        The path of names within names for the DynamicObject,
    *        separated by '/'.
    *
    * @return  True if the attribute was was successfully removed from the object,
    *          otherwise false.
    *
    * @notify This method will notify Subject::signalModified.
    *
    * @see     clear()
    */
   virtual bool removeAttributeByPath(const std::string& path) = 0;

   /**
    *  Erases all attributes in the object.
    *
    *  @notify This method will notify Subject::signalModified.
    *
    *  @see     remove()
    */
   virtual void clear() = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~DynamicObject() {}
};

/**
 * \cond INTERNAL
 * These template specialization are required to allow these types to be put into a DataVariant.
 */
template <> class VariantTypeValidator<DynamicObject> {};
template <> class VariantTypeValidator<const DynamicObject> {};
/// \endcond

#endif   // DYNAMICOBJECT_H
