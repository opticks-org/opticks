/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAELEMENT_H
#define DATAELEMENT_H

#include "SessionItem.h"
#include "Subject.h"
#include "Serializable.h"

#include <string>

class Classification;
class DataDescriptor;
class DynamicObject;

/**
 *  Base class for all data elements.
 *
 *  The DataElement is a base class for all objects that are managed by
 *  ModelServices within the application.  This class only stores anscillary
 *  data in a DataDescriptor object, where subclasses store or manage the
 *  actual raw data.  An instance of this class can be created to provide an
 *  element container for custom data not provided by the subclasses.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The Subject::signalModified notification is sent when the following items
 *    are modified in the data descriptor: classification, metadata, and file
 *    descriptor.
 *  - The data descriptor's name or parent element are changed through
 *    ModelServices.
 *
 *  @see    Subject, Serializable, DataDescriptor
 */
class DataElement : public SessionItem, public Subject, public Serializable
{
public:
   /**
    *  Returns all anscillary data for the element.
    *
    *  @return  A pointer to the DataDescriptor object containing the element anscillary
    *           data.
    */
   virtual DataDescriptor* getDataDescriptor() = 0;

   /**
    *  Returns read-only access to all anscillary data for the element.
    *
    *  @return  A const pointer to the DataDescriptor object containing the element
    *           anscillary data.  The data represented by the returned pointer should
    *           not be modified.  To modify the values, call the non-const version of
    *           getDataDescriptor().
    */
   virtual const DataDescriptor* getDataDescriptor() const = 0;

   /**
    *  Returns the identifying type for the element.
    *
    *  This is a convenience method that returns the element type that is stored in the
    *  DataDescriptor object.
    *
    *  The element type is one of the unique identifiers for a DataElement that is used
    *  to query elements in the ModelServices interface.
    *
    *  @return  The element type.  The returned type will be one of the valid element
    *           type strings returned by ModelServices::getValidElementTypes(), which
    *           may not be the same type returned by getObjectType().
    *
    *  @see     DataDescriptor::getType(), ModelServices::getElement()
    */
   virtual std::string getType() const = 0;

   /**
    *  Returns the filename where the element is saved on disk.
    *
    *  This is a convenience method that returns the filename that is stored in the
    *  FileDecriptor object contained in the element's DataDescriptor object.
    *
    *  @return  The element filename.  An empty string is returned if the element was not
    *           originally imported from a file on disk.
    *
    *  @see     FileDescriptor::getFilename()
    */
   virtual std::string getFilename() const = 0;

   /**
    *  Returns the parent element.
    *
    *  This is a convenience method that returns the parent element that is stored
    *  in the DataDescriptor object.  The element parent is typically the data set
    *  to which the element is associated.
    *
    *  The parent element is one of the unique identifiers for a DataElement that
    *  is used to query elements in the ModelServices interface.
    *
    *  @return  The element's associated parent element.  \b NULL is returned if the
    *           element does not have a parent.
    *
    *  @see     DataDescriptor::getParentElement(), ModelServices::getElement()
    */
   virtual DataElement* getParent() const = 0;

   /**
    *  Returns the parent element designator.
    *
    *  The parent element designator is a vector of strings such that the first
    *  string is the name of a top level element, the second is the name of a child
    *  of that element, and so on. This designator identifies the parent element.
    *
    *  @return A parent element designator
    *
    *  @see getParent(), ModelServices::createDataDescriptor(const std::string&,const std::string&, const std::vector<std::string>&) const
    */
   virtual std::vector<std::string> getParentDesignator() const = 0;

   /**
    *  Returns a pointer to the element's classification object.
    *
    *  The classification object documents how the data in the element is to be
    *  handled and/or restricted.
    *
    *  This is a convenience method that returns the classification object that
    *  is stored in the DataDescriptor object.
    *
    *  @return  A pointer to the element's classification object.
    *
    *  @see     DataDescriptor::getClassification(), Classification
    */
   virtual Classification* getClassification() = 0;

   /**
    *  Returns read-only access to the element's classification object.
    *
    *  The classification object documents how the data in the element is to be
    *  handled and/or restricted.
    *
    *  This is a convenience method that returns the classification object that
    *  is stored in the DataDescriptor object.
    *
    *  @return  A const pointer to the element's classification object.
    *           The classification represented by the returned pointer should
    *           not be modified.  To modify the values, call the non-const
    *           version of getClassification() instead.
    *
    *  @see     DataDescriptor::getClassification(), Classification
    */
   virtual const Classification* getClassification() const = 0;

   /**
    *  Sets the element's classification object.
    *
    *  The classification object documents how the data in element is to be handled and/or
    *  restricted.
    *
    *  This is a convenience method that makes a deep copy of the given classification object
    *  that is stored in its DataDescriptor object, so it is the responsibility of the calling object
    *  to delete the classification object when necessary.
    *
    *  @param   pClassification
    *           The classification for the element.  A deep copy is performed
    *           so it is the responsibility of the calling object to delete
    *           the given classification object when necessary.  This method
    *           does nothing if \c NULL is passed in.
    *
    *  @see     DataDescriptor::setClassification(), Classification
    */
   virtual void setClassification(const Classification* pClassification) = 0;

   /**
    *  Copies the classification settings from a DataElement.
    *
    *  This is a convenience method that deep copies the Classification object that is stored
    *  in the DataDescriptor object of another DataElement into the DataDescriptor object of
    *  this DataElement. The existing classification settings for this DataElement are replaced
    *  by the settings from the passed DataElement. No attempt is made to merge the settings.
    *
    *  @param   pElement
    *           The DataElement from which the Classification object will be copied.
    *
    *  @see     DataDescriptor::getClassification(), DataDescriptor::setClassification(), Classification
    */
   virtual void copyClassification(const DataElement* pElement) = 0;

   /**
    *  Returns a pointer to the element's metadata values.
    *
    *  This is a convenience method that returns the metadata object that is stored in the
    *  DataDescriptor object.
    *
    *  Please see \ref specialmetadata for details on
    *  special entries in the metadata that the application will
    *  attempt to use.
    *
    *  @return  A pointer to the element's metadata as a DynamicObject.
    *
    *  @see     DataDescriptor::getMetadata(), DynamicObject
    */
   virtual DynamicObject* getMetadata() = 0;

   /**
    *  Returns read-only access to the element's metadata values.
    *
    *  This is a convenience method that returns the metadata object that is
    *  stored in the DataDescriptor object.
    *
    *  Please see \ref specialmetadata for details on
    *  special entries in the metadata that the application will
    *  attempt to use.
    *
    *  @return  A const pointer to the element's metadata as a DynamicObject.
    *           The metadata represented by the returned pointer should not be
    *           modified.  To modify the values, call the non-const version of
    *           getMetadata().
    *
    *  @see     DataDescriptor::getMetadata(), DynamicObject
    */
   virtual const DynamicObject* getMetadata() const = 0;

   /**
    *  Creates a new data element with the same values as this element.
    *
    *  The method creates a new element based on the data contained in this
    *  element, which includes a copy of all data in the data descriptor.
    *
    *  @param   name
    *           The name for the created data element, which can be the same
    *           as this object's name if the parent is different than this
    *           object's parent.
    *  @param   pParent
    *           The parent element for the created data element, which can be
    *           the same as this object's parent if the name is different
    *           than this object's name.
    *
    *  @return  A pointer to the new data element.  \b NULL is returned if
    *           the element cannot be copied or if the given parent is the
    *           same as this object's parent.
    */
   virtual DataElement* copy(const std::string& name, DataElement* pParent) const = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~DataElement() {}
};

#endif
