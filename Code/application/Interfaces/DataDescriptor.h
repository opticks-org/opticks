/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATADESCRIPTOR_H
#define DATADESCRIPTOR_H

#include "Subject.h"
#include "Serializable.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class Classification;
class DataElement;
class DynamicObject;
class FileDescriptor;
class Message;

/**
 *  Describes a data element.
 *
 *  A data descriptor contains all ancillary data for a DataElement that is
 *  not part of the raw data.  ModelServices stores elements based on a key
 *  that is comprised of a name, type, and parent element, which are all stored
 *  here.  For this reason, the data descriptor must be created by calling
 *  ModelServices::createDataDescriptor() passing in the name, type, and parent
 *  element.
 *
 *  After the data descriptor is created the name, type, and parent element
 *  cannot be set directly on the data descriptor object to preserve the
 *  integrity of the identifying key in ModelServices.  After the data
 *  descriptor is created call ModelServices::setElementName() to change the
 *  name and ModelServices::setElementParent() to change the parent.
 *
 *  The data descriptor also contains the element classification and metadata,
 *  a processing location value that indicates how the data was imported and
 *  where it is accessed, and a FileDescriptor indicating how the data is
 *  stored in a file on disk.
 *
 *  The file descriptor is intended to indicate how a data element was imported
 *  and therefore is typically set only by importers and not as a result of
 *  creating a new data element or exporting an existing data element. The file
 *  descriptor is owned by the data descriptor class.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setClassification(), setMetadata(),
 *    setProcessingLocation(), setFileDescriptor(), and clone().
 *  - The descriptor is renamed.
 *  - The descriptor's parent data element is changed.
 *  - All notifications documented in Subject.
 *
 *  @see        DataElement, FileDescriptor
 */
class DataDescriptor : public Subject, public Serializable
{
public:
   /**
    *  Emitted when the descriptor name changes with boost::any<std::string>
    *  containing the new name.
    */
   SIGNAL_METHOD(DataDescriptor, Renamed)

   /**
    *  Emitted when the parent data element changes with
    *  boost::any<\link DataElement\endlink*> containing a pointer to the new
    *  parent element.
    */
   SIGNAL_METHOD(DataDescriptor, ParentChanged)

   /**
    *  Emitted when the processing location changes with
    *  boost::any<\link ::ProcessingLocation ProcessingLocation\endlink>
    *  containing the new processing location.
    *
    *  @see     setProcessingLocation()
    */
   SIGNAL_METHOD(DataDescriptor, ProcessingLocationChanged)

   /**
    *  Returns the name for the data.
    *
    *  This method returns the name that is used along with the type and parent
    *  element to uniquely identify a DataElement in ModelServices.  To set the
    *  name after the data descriptor is created, call
    *  ModelServices::setElementName().
    *
    *  @return  The data name.
    */
   virtual const std::string& getName() const = 0;

   /**
    *  Returns the type of the data.
    *
    *  The type is used by ModelServices to determine the kind of element to
    *  create and is used along with the name and parent element to uniquely
    *  identify a DataElement.  This method returns the type that was specified
    *  in ModelServices::createDataDescriptor().
    *
    *  @return  The data type.
    */
   virtual const std::string& getType() const = 0;

   /**
    *  Returns the parent element.
    *
    *  The parent element is typically the data set to which the element is
    *  associated.  This method returns the parent element that is used along
    *  with the name and type to uniquely identify a DataElement in
    *  ModelServices.  To set the parent after the data descriptor is created,
    *  call ModelServices::setElementParent().
    *
    *  @return  The element's associated parent element.  \b NULL is returned
    *           if the element does not have a parent.
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
    *  Sets the element's classification object.
    *
    *  The classification object documents how the data in element is to be
    *  handled and/or restricted.
    *
    *  This method performs a deep copy of the given classification object, so
    *  it is the responsibility of the calling object to delete the
    *  classification object when necessary.
    *
    *  @param   pClassification
    *           The classification for the element.  A deep copy is performed
    *           so it is the responsibility of the calling object to delete
    *           the given classifcation object when necessary.  This method
    *           does nothing if \b NULL is passed in.
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           classification is non-NULL and is different than the current
    *           classification.
    *
    *  @see     Classification
    */
   virtual void setClassification(const Classification* pClassification) = 0;

   /**
    *  Returns access to the element's classification object.
    *
    *  The classification object documents how the data in element is to be
    *  handled and/or restricted.
    *
    *  @return  A pointer to the element's classification object.
    *
    *  @see     Classification
    */
   virtual Classification* getClassification() = 0;

   /**
    *  Returns read-only access to the element's classification object.
    *
    *  The classification object documents how the data in element is to be
    *  handled and/or restricted.
    *
    *  @return  A const pointer to the element's classification object.  The
    *           classification represented by the returned pointer should not
    *           be modified.  To modify the values, use the non-const version
    *           of this method.
    *
    *  @see     Classification
    */
   virtual const Classification* getClassification() const = 0;

   /**
    *  Sets the element's metadata.
    *
    *  This method performs a deep copy of the given metadata dynamic object,
    *  so it is the responsibility of the calling object to delete the metadata
    *  object when necessary.
    *
    *  @param   pMetadata
    *           The element metadata.  Passing in \b NULL will clear all
    *           existing metadata.  A deep copy is performed so it is the
    *           responsibility of the calling object to delete the given
    *           metadata object when necessary.
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           metadata object is different than the current metadata object.
    */
   virtual void setMetadata(const DynamicObject* pMetadata) = 0;

   /**
    *  Returns a pointer to the element's metadata values.
    *  Please see \ref specialmetadata for details on
    *  special entries in the metadata that the application will
    *  attempt to use.
    *
    *  @return  A pointer to the element's metadata as a DynamicObject.
    */
   virtual DynamicObject* getMetadata() = 0;

   /**
    *  Returns read-only access to the element's metadata values.
    *  Please see \ref specialmetadata for details on
    *  special entries in the metadata that the application will
    *  attempt to use.
    *
    *  @return  A const pointer to the element's metadata as a DynamicObject.
    *           The metadata represented by the returned pointer should not be
    *           modified.  To modify the values, call the non-const version of
    *           getMetadata().
    */
   virtual const DynamicObject* getMetadata() const = 0;

   /**
    *  Specifies the location from where the element data will be accessed.
    *
    *  The processing location specifies how the element data should imported
    *  and how it will be accessed after importing.  This allows for processing
    *  large data sets that cannot be loaded entirely into memory.  The default
    *  location is \link ProcessingLocation::IN_MEMORY IN_MEMORY\endlink.
    *
    *  @param   processingLocation
    *           The processing location for the element data.
    *
    *  @notify  This method notifies signalProcessingLocationChanged() if the
    *           given processing location is different than the current
    *           processing location.
    */
   virtual void setProcessingLocation(ProcessingLocation processingLocation) = 0;

   /**
    *  Returns the location for where the element data is processed.
    *
    *  @return  The processing location of the element data.
    *
    *  @see     setProcessingLocation()
    */
   virtual ProcessingLocation getProcessingLocation() const = 0;

   /**
    *  Sets a file descriptor to indicate how the data is stored on disk.
    *
    *  By default, the data descriptor contains a \b NULL file descriptor,
    *  which indicates that the data element was not created as a result of an
    *  import.  By calling this method with a non-NULL value, the data element
    *  is marked as having a corresponding file on disk from which the data
    *  was imported.
    *
    *  The file descriptor is intended to indicate how a data element was
    *  imported and therefore this method should typically be called only by
    *  importers and not as a result of creating a new data element or
    *  exporting an existing data element.
    *
    *  This method performs a deep copy of the given file descriptor, so it is
    *  the responsibility of the calling object to delete the file descriptor
    *  when necessary.
    *
    *  @param   pFileDescriptor
    *           The file descriptor describing how the data is stored on disk.
    *           A deep copy is performed so the calling object is responsible
    *           for deleting the given file descriptor when necessary.  Passing
    *           in \b NULL indicates that the data element was not created as a
    *           result of an import.
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           file descriptor is different than the current file descriptor.
    */
   virtual void setFileDescriptor(const FileDescriptor* pFileDescriptor) = 0;

   /**
    *  Returns the file descriptor indicating how the data is stored on disk.
    *
    *  @return  A pointer to the file descriptor describing how the data is
    *           stored on disk.  Do not delete this pointer since it's owned by
    *           the data descriptor which will handle its deletion.
    *           If \b NULL is returned, this indicates that the data element 
    *           was not created as a result of an import.
    *
    *  @see     setFileDescriptor()
    */
   virtual FileDescriptor* getFileDescriptor() = 0;

   /**
    *  Returns read-only access to the file descriptor indicating how the data
    *  is stored on disk.
    *
    *  @return  The file descriptor describing how the data is stored on disk.
    *           The file descriptor represented by the returned pointer should
    *           not be modified.  To modify the values, call the non-const
    *           version of getFileDescriptor().  Do not delete this pointer 
    *           since it's owned by the data descriptor which will handle its 
    *           deletion. If \b NULL is returned, this indicates that the data 
    *           element was not created as a result of an import.
    *
    *  @see     setFileDescriptor()
    */
   virtual const FileDescriptor* getFileDescriptor() const = 0;

   /**
    *  Creates a duplicate data descriptor based on this data descriptor.
    *
    *  This is typically used to deep copy a data descriptor when a non-shared
    *  copy is required.  If the data element for this data descriptor already
    *  exists, the created data descriptor should not be used to create a new
    *  data element as the element already exists.
    *
    *  @return  A duplicate copy of this data descriptor or \c NULL if there was
    *           an error.  The returned data descriptor contains the same
    *           classification settings as this data descriptor.
    *
    *  @see     copy(const std::string&, DataElement*) const,
    *           copy(const std::string&, const std::vector<std::string>&) const,
    *           clone()
    */
   virtual DataDescriptor* copy() const = 0;

   /**
    *  Creates a new data descriptor based on this data descriptor.
    *
    *  This method creates a new data descriptor by calling
    *  ModelServices::createDataDescriptor() and sets the values of the created
    *  descriptor to this object's values.  Because ModelServices uses the
    *  name and parent element as unique identifiers, a new name and/or
    *  parent should be passed into this method for the call to
    *  ModelServices::createDataDescriptor() to succeed.  The type of this
    *  descriptor is used as the type for the new descriptor.
    *
    *  @param   name
    *           The name for the new element that would be created with the
    *           returned data descriptor.
    *  @param   pParent
    *           The parent element for the new element that would be created
    *           with the returned data descriptor.
    *
    *  @return  The new data descriptor containing values identical to the
    *           values in this data descriptor except for the given name and
    *           parent element.  This method will return a valid data
    *           descriptor even if a subsequent call to
    *           ModelServices::createElement() would fail because an element
    *           already exists with the same name, type, and parent as the
    *           created data descriptor.  The returned data descriptor will have
    *           the same classification settings as this data descriptor and not
    *           the classification settings of the given parent element.
    *
    *  @see     copy() const,
    *           copy(const std::string&, const std::vector<std::string>&) const,
    *           clone()
    */
   virtual DataDescriptor* copy(const std::string& name, DataElement* pParent) const = 0;

   /**
    *  Creates a new data descriptor based on this data descriptor.
    *
    *  This method creates a new data descriptor by calling
    *  ModelServices::createDataDescriptor() and sets the values of the created
    *  descriptor to this object's values.  Because ModelServices uses the
    *  name and parent element as unique identifiers, a new name and/or
    *  parent designator should be passed into this method for the call to
    *  ModelServices::createDataDescriptor() to succeed.  The type of this
    *  descriptor is used as the type for the new descriptor.
    *
    *  @param   name
    *           The name for the new element that would be created with the
    *           returned data descriptor.
    *  @param   parent
    *           The designator that identifies the parent element for the
    *           created data descriptor.  The designator is a vector of element
    *           names such that the first name is a top level element, the
    *           second name is a child of that element, and so on.
    *
    *  @return  The new data descriptor containing values identical to the
    *           values in this data descriptor except for the given name and
    *           parent element.  This method will return a valid data
    *           descriptor even if a subsequent call to
    *           ModelServices::createElement() would fail because an element
    *           already exists with the same name, type, and parent as the
    *           created data descriptor.  The returned data descriptor will have
    *           the same classification settings as this data descriptor and not
    *           the classification settings of the element represented by the
    *           given parent designator.
    *
    *  @see     copy() const, copy(const std::string&, DataElement*) const,
    *           clone(), getParentDesignator()
    */
   virtual DataDescriptor* copy(const std::string& name, const std::vector<std::string>& parent) const = 0;

   /**
    *  Sets all values in this data descriptor to those of another data
    *  descriptor.
    *
    *  @warning The type of given data descriptor must match the type of this
    *           data descriptor.
    *
    *  @param   pDescriptor
    *           The data descriptor from which to set all data values in this
    *           data descriptor.  No signal/slot attachments currently defined
    *           in \em pDescriptor are set into this descriptor.  This method
    *           does nothing and returns \c false if \c NULL is passed in.
    *
    *  @return  Returns \c true if all values in this data descriptor were
    *           successfully set to the values in the given data descriptor;
    *           otherwise returns \c false.  Returns \c false if the type of the
    *           given data descriptor does not match the type of this data
    *           descriptor.
    *
    *  @notify  This method notifies one or more signals defined in this class
    *           or its subclasses based on data values that are actually
    *           changed.
    *
    *  @see     copy(), getType()
    */
   virtual bool clone(const DataDescriptor* pDescriptor) = 0;

   /**
    *  Adds the current values in this data descriptor to a given message.
    *
    *  This convenience method adds the values in this data descriptor as
    *  properties to the given message in a message log.
    *
    *  @param   pMessage
    *           The message in the message log for which to add this data
    *           descriptor's values.
    */
   virtual void addToMessageLog(Message* pMessage) const = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyDataDescriptor.
    */
   virtual ~DataDescriptor() {}
};

#endif
