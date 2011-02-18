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

#include "DataVariant.h"
#include "DataVariantValidator.h"
#include "Serializable.h"
#include "Subject.h"

#include <string>
#include <vector>

class DataVariant;
class QRegExp;

/**
 * Dynamic extension of class attributes
 *
 * Dynamic Object refers to a class that allows attributes to be 
 * added during runtime.  Many classes extend the Dynamic Object
 * to provide future support for new attributes without changing
 * the class specification.
 *
 * This subclass of Subject will notify upon the following conditions:
 * - The following methods are called: setAttribute(), setAttributeByPath(),
 *   adoptAttribute(), adoptAttributeByPath(), merge(), removeAttribute(),
 *   removeAttributeByPath(), clear().
 * - %Any of the objects within the DynamicObject post a notification.
 *   These notifications will be passed on to any observers of the
 *   DynamicObject
 * - Everything else documented in Subject.
 *
 * @see     DynamicObjectExt1
 */
class DynamicObject : public Subject, public Serializable
{
public:
   /**
    * Emitted just after a new attribute is added with
    * boost::any<std::pair<std::string, DataVariant*> > containing the
    * attribute name and value.
    */
   SIGNAL_METHOD(DynamicObject, AttributeAdded)

   /**
    * Emitted just after an existing attribute is modified with
    * boost::any<std::pair<std::string, DataVariant*> > containing the
    * attribute name and the new value.
    */
   SIGNAL_METHOD(DynamicObject, AttributeModified)

   /**
    * Emitted just before an existing attribute is removed with
    * boost::any<std::pair<std::string, DataVariant*> > containing the
    * name and value of the attribute that will be removed.\   The attribute
    * value is guaranteed to be valid when the signal is emitted.
    */
   SIGNAL_METHOD(DynamicObject, AttributeRemoved)

   /**
    * Emitted just before the dynamic object is cleared.\   All existing
    * attribute values are guaranteed to be valid when the signal is emitted.
    */
   SIGNAL_METHOD(DynamicObject, Cleared)

   /**
    * Sets attributes from those of another dynamic object.
    *
    * This method parses the given object's attributes, adding them to or
    * setting them in this object.  Any existing attributes that are not
    * contained in the given object are not removed.
    *
    * @param   pObject
    *          The object from which to set this object's attributes.
    *          Cannot be \c NULL.
    *
    * @notify  This method will notify Subject::signalModified once for each
    *          attribute which gets merged. The parameters will depend on
    *          the attribute modified.
    *
    * @see setAttribute()
    */
   virtual void merge(const DynamicObject* pObject) = 0;

   /**
    * Sets attributes from those of another dynamic object.
    *
    * This method parses the given object's attributes, adopting them into
    * the destination object.  Any existing attributes that are not
    * contained in the given object are not removed. The attributes of
    * the given dynamic object will be modified in an undefined manner.
    *
    * This method is intended for use when the given object will be discarded
    * after being merged into the destination object and can be much faster
    * than using the merge() method which sets the attributes rather than 
    * adopting them.
    *
    * @param   pObject
    *          The object from which to adopt into this object's attributes.
    *          Cannot be \c NULL.
    *
    * @notify  This method will notify Subject::signalModified once for each
    *          attribute which gets adopted. The parameters will depend on
    *          the attribute modified.
    *
    * @see adoptAttribute()
    */
   virtual void adoptiveMerge(DynamicObject* pObject) = 0;

   /**
    * Creates a new attribute or sets an existing dynamic attribute.
    *
    * This method is preferred to adoptAttribute() unless
    * you are passing an already constructed DataVariant in
    * which case, adoptAttribute() will be faster because
    * it avoids a deep copy.
    *
    * @param   name
    *          The name of the attribute to set the value.  If an attribute
    *          with the given name does not exist, an attribute is added.
    * @param   value
    *          The attribute's value.
    *
    * @return  Returns \c true if the attributes was successfully created or set,
    *          otherwise \c false.
    *
    * @notify  This method will notify Subject::signalModified.
    *
    * @see adoptAttribute()
    */
   template<class T>
   bool setAttribute(const std::string& name, const T& value)
   {
      DataVariant temp(value);
      return adoptAttribute(name, temp);
   }

   /**
    * Creates a new attribute or sets an existing dynamic attribute.
    *
    * This method should not be used; generally setAttribute() is
    * preferred. This method and setAttribute() have identical
    * performance characteristics and setAttribute() is easier to
    * call.  This method is faster than setAttribute() though if
    * you have an already constructed DataVariant and is
    * preferred to setAttribute() in this case.
    *  
    * @param   name
    *          The name of the attribute to set the value.  If an attribute
    *          with the given name does not exist, an attribute is added.
    * @param   value
    *          A variant holding the attribute's value. On return, this will 
    *          contain the value previously stored in the DynamicObject.
    *          If the value did not previously exist in the DynamicObject, 
    *          then this will contain an invalid DataVariant.
    *
    * @return  Returns \c true if the attributes was successfully created or set,
    *          otherwise \c false.
    *
    * @notify  This method will notify Subject::signalModified.
    *
    * @see setAttribute()
    */
   virtual bool adoptAttribute(const std::string& name, DataVariant& value) = 0;
   
   /**
    * Sets the value within the DynamicObject hierarchy as described in the path.
    *
    * This method will create DynamicObjects as needed to set the value.
    * This method is preferred to adoptAttributeByPath() unless
    * you are passing an already constructed DataVariant in
    * which case, adoptAttributeByPath() will be faster because
    * it avoids a deep copy.
    *
    * @param   path
    *          The path of names within names for the DynamicObjects,
    *          separated by '/'.  A slash in the name can be represented by
    *          escaping the slash with another slash (e.g. '//').  If the path
    *          ends in a single slash, it will be ignored.
    * @param   value
    *          The value to set into the DynamicObject.
    *
    * @return  Returns \c true if the operation was a success, \c false otherwise.
    *          This method will fail if any object along the path already exists
    *          but is not a DynamicObject.
    *
    * @notify  This method will notify Subject::signalModified.
    *
    * @see adoptAttributeByPath()
    */
   template<class T>
   bool setAttributeByPath(const std::string& path, const T& value)
   {
      DataVariant temp(value);
      return adoptAttributeByPath(path, temp);
   }

   /**
    * Sets the value within the DynamicObject hierarchy as described in the path.
    *
    * This method should not be used; generally setAttributeByPath() is
    * preferred. This method and setAttributeByPath() have identical
    * performance characteristics and setAttributeByPath() is easier to
    * call.  This method is faster than setAttributeByPath() though if
    * you have an already constructed DataVariant and is
    * preferred to setAttributeByPath() in this case.
    *
    * @param   path
    *          The path of names within names for the DynamicObjects,
    *          separated by '/'.  A slash in the name can be represented by
    *          escaping the slash with another slash (e.g. '//').  If the path
    *          ends in a single slash, it will be ignored.
    * @param   value
    *          The value to set into the DynamicObject. On return, this will
    *          contain the value previously stored in the DynamicObject.  If
    *          the value did not previously exist in the DynamicObject, then
    *          this will contain an invalid DataVariant.
    *
    * @return  Returns \c true if the operation was a success, \c false otherwise.
    *          This method will fail if any object along the path already exists
    *          but is not a DynamicObject.
    *
    * @notify  This method will notify Subject::signalModified.
    *
    * @see setAttributeByPath()
    */
   virtual bool adoptAttributeByPath(const std::string& path, DataVariant& value) = 0;

   /**
    * Sets the value within the DynamicObject hierarchy as described in the path.
    *
    * This method will create DynamicObjects as needed to set the value.
    * This method is preferred to adoptAttributeByPath() unless
    * you are passing an already constructed DataVariant in
    * which case, adoptAttributeByPath() will be faster because
    * it avoids a deep copy.
    *
    * @param pComponents
    *        An array of path components to the desired object as std::strings.  Must end with
    *        END_METADATA_NAME.
    *
    * @param value
    *        The value to set into the DynamicObject.
    *
    * @return Returns \c true if the operation was a success, \c false otherwise.
    *         This method will fail if any object along the path already exists
    *         but is not a DynamicObject.
    *
    * @notify  This method will notify Subject::signalModified.
    *
    * @see adoptAttributeByPath()
    */
   template<class T>
   bool setAttributeByPath(const std::string pComponents[], const T& value)
   {
      DataVariant temp(value);
      return adoptAttributeByPath(pComponents, temp);
   }

   /**
    * Sets the value within the DynamicObject hierarchy as described in the path.
    *
    * This method should not be used; generally setAttributeByPath() is
    * preferred. This method and setAttributeByPath() have identical
    * performance characteristics and setAttributeByPath() is easier to
    * call.  This method is faster than setAttributeByPath() though if
    * you have an already constructed DataVariant and is
    * preferred to setAttributeByPath() in this case.
    *
    * @param pComponents
    *        An array of path components to the desired object as std::strings.  Must end with
    *        END_METADATA_NAME.
    *
    * @param value
    *        The value to set into the DynamicObject. On return, this will contain the value previously stored in the DynamicObject.
    *        If the value did not previously exist in the DynamicObject, then this will contain an invalid DataVariant.
    *
    * @return Returns \c true if the operation was a success, \c false otherwise.
    *         This method will fail if any object along the path already exists
    *         but is not a DynamicObject.
    *
    * @notify  This method will notify Subject::signalModified.
    *
    * @see setAttributeByPath()
    */
   virtual bool adoptAttributeByPath(const std::string pComponents[], DataVariant& value) = 0;

   /**
    * Returns an attribute value.
    *
    * @param   name
    *          The name of the attribute for which to get the value.
    *
    * @return  A variant holding the attributes value. The variant will be
    *          invalid if the attribute did not exist.
    *
    * @see     DataVariant::isValid()
    */
   virtual const DataVariant& getAttribute(const std::string& name) const = 0;

   /**
    * Returns an attribute value.
    *
    * @param   name
    *          The name of the attribute for which to get the value.
    *
    * @return  A variant holding the attributes value. The variant will be
    *          invalid if the attribute did not exist.
    *
    * @see     DataVariant::isValid()
    */
   virtual DataVariant& getAttribute(const std::string& name) = 0;

   /**
    * Get the value within the DynamicObject hierarchy
    * as described in the path.
    *
    * @param   path
    *          The path of names within names for the DynamicObjects,
    *          separated by '/'.  A slash in the name can be represented by
    *          escaping the slash with another slash (e.g. '//').  If the path
    *          ends in a single slash, it will be ignored.
    *
    * @return  A variant holding the attributes value. The variant will be
    *          empty if the attribute did not exist.
    */
   virtual const DataVariant& getAttributeByPath(const std::string& path) const = 0;

   /**
    * Get the value within the DynamicObject hierarchy
    * as described in the path.
    *
    * @param   path
    *          The path of names within names for the DynamicObjects,
    *          separated by '/'.  A slash in the name can be represented by
    *          escaping the slash with another slash (e.g. '//').  If the path
    *          ends in a single slash, it will be ignored.
    *
    * @return  A variant holding the attributes value. The variant will be
    *          empty if the attribute did not exist.
    */
   virtual DataVariant& getAttributeByPath(const std::string& path) = 0;

    /**
     * Get the value within the DynamicObject hierarchy
     * as described in the path.
     *
     * @param pComponents
     *        An array of path components to the desired object as std::strings.  Must end with
     *        END_METADATA_NAME.
     *
     * @return  A variant holding the attributes value. The variant will be
     *          empty if the attribute did not exist.
     */
   virtual const DataVariant& getAttributeByPath(const std::string pComponents[]) const = 0;

   /**
    * Get the value within the DynamicObject hierarchy
    * as described in the path.
    *
    * @param pComponents
    *        An array of path components to the desired object as std::strings.  Must end with
    *        END_METADATA_NAME.
    *
    * @return  A variant holding the attributes value. The variant will be
    *          empty if the attribute did not exist.
    */
   virtual DataVariant& getAttributeByPath(const std::string pComponents[]) = 0;

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
    * @return  Returns \c true if the attribute was was successfully removed from the object,
    *          otherwise \c false.
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
    * @return  Returns \c true if the attribute was was successfully removed from the object,
    *          otherwise \c false.
    *
    * @notify This method will notify Subject::signalModified.
    *
    * @see     clear()
    */
   virtual bool removeAttributeByPath(const std::string pComponents[]) = 0;

   /**
    * Removes an attribute from the object.
    *
    * @param   path
    *          The path of names within names for the DynamicObjects,
    *          separated by '/'.  A slash in the name can be represented by
    *          escaping the slash with another slash (e.g. '//').  If the path
    *          ends in a single slash, it will be ignored.
    *
    * @return  Returns \c true if the attribute was was successfully removed from the object,
    *          otherwise \c false.
    *
    * @notify  This method will notify Subject::signalModified.
    *
    * @see     clear()
    */
   virtual bool removeAttributeByPath(const std::string& path) = 0;

   /**
    * Erases all attributes in the object.
    *
    * @notify This method will notify Subject::signalModified.
    *
    * @see     remove()
    */
   virtual void clear() = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~DynamicObject() {}
};

/**
 * Extends capability of the DynamicObject interface.
 *
 * This class provides additional capability for the DynamicObject interface class.
 * A pointer to this class can be obtained by performing a dynamic cast on a
 * pointer to DynamicObject or any of its subclasses.
 *
 * @warning A pointer to this class can only be used to call methods contained
 *           in this extension class and cannot be used to call any methods in
 *           DynamicObject or its subclasses.
 */
class DynamicObjectExt1
{
public:
   /**
    * Searches the object for an attribute with a given name and/or value.
    *
    * @warning This method only searches this object's attributes and not
    *          any attributes of a child DynamicObject.
    *
    * @param   name
    *          The name of the attribute for which to search, which can be a
    *          fixed string or any regular expression supported by QRegExp.  If
    *          \em name is empty, then the first attribute found with the given
    *          value is returned.
    * @param   value
    *          The value of the attribute for which to search, which can be a
    *          fixed string or any regular expression supported by QRegExp.  If
    *          \em value is empty, then the first attribute found with the given
    *          name is returned.
    *
    * @return  Returns a const reference to the first attribute found with the
    *          given name and/or value.  A const reference to an invalid value
    *          is returned if both \em name and \em value are empty, or if this
    *          object does not have an attribute with the given name and/or
    *          value.
    *
    * @see     DataVariant& findFirstOf(const QRegExp&, const QRegExp&)
    */
   virtual const DataVariant& findFirstOf(const QRegExp& name, const QRegExp& value) const = 0;

   /**
    * Searches the object for an attribute with a given name and/or value.
    *
    * @warning This method only searches this object's attributes and not
    *          any attributes of a child DynamicObject.
    *
    * @param   name
    *          The name of the attribute for which to search, which can be a
    *          fixed string or any regular expression supported by QRegExp.  If
    *          \em name is empty, then the first attribute found with the given
    *          value is returned.
    * @param   value
    *          The value of the attribute for which to search, which can be a
    *          fixed string or any regular expression supported by QRegExp.  If
    *          \em value is empty, then the first attribute found with the given
    *          name is returned.
    *
    * @return  Returns a non-const reference to the first attribute found with
    *          the given name and/or value.  A non-const reference to an invalid
    *          value is returned if both \em name and \em value are empty, or if
    *          this object does not have an attribute with the given name and/or
    *          value.
    *
    * @see     const DataVariant& findFirstOf(const QRegExp&, const QRegExp&) const
    */
   virtual DataVariant& findFirstOf(const QRegExp& name, const QRegExp& value) = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject().
    */
   virtual ~DynamicObjectExt1()
   {}
};

/**
 * \cond INTERNAL
 * These template specialization are required to allow these types to be put into a DataVariant.
 */
template <> class VariantTypeValidator<DynamicObject> {};
template <> class VariantTypeValidator<const DynamicObject> {};
/// \endcond

#endif   // DYNAMICOBJECT_H
