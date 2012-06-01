/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODELSERVICES_H
#define MODELSERVICES_H

#include "Any.h"
#include "AnyData.h"
#include "ComplexData.h"
#include "Service.h"
#include "Subject.h"
#include "switchOnEncoding.h"
#include "TypesFile.h"

#include <map>
#include <string>
#include <vector>

class DataDescriptor;
class DataElement;
class ImportDescriptor;

/**
 *  \ingroup ServiceModule
 *  Provides access for storing and managing data.
 *
 *  This interface allows for creation and manipulation of data, including
 *  access to a session within the application.  Data is stored in elements
 *  that contain descriptor classes to provide information related to the data.
 *
 *  Each element is stored internally according to a key that is comprised of
 *  a name, type, and a parent element.  The combination of these three
 *  identifiers must be unique when creating a new element, renaming an
 *  existing element, or reparenting an existing element.
 *
 *  The parent element allows for different elements of different types to
 *  be associated with each other.  For example, a common parent/child pair
 *  is an AoiElement element that has a RasterElement parent to indicate that
 *  the area of interest relates to a specific data set.
 *
 *  If an element has a non-\b NULL parent element, the element will be
 *  destroyed when its parent is destroyed.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: createElement(), destroyElement(),
 *    setElementParent(), and clear().
 *  - Everything else documented in Subject.
 */
class ModelServices : public Subject
{
public:
   /**
    *  Emitted with boost::any<DataElement*> when a DataElement is created.
    */
   SIGNAL_METHOD(ModelServices, ElementCreated)

   /**
    *  Emitted with boost::any<DataElement*> when a DataElement is destroyed.
    */
   SIGNAL_METHOD(ModelServices, ElementDestroyed)

   /**
    *  Emitted with boost::any<DataElement*> when the parent of a DataElement
    *  changes.
    */
   SIGNAL_METHOD(ModelServices, ElementReparented)

   /**
    *  Creates an empty data descriptor.
    *
    *  This method creates an empty data descriptor that can be populated so
    *  that a data element can be created.  The memory for the data is
    *  allocated by the studio but owned by the plug-in, so plug-in must 
    *  destroy the data descriptor by calling the destroyDataDescriptor()
    *  method.
    *
    *  @param   name
    *           The name for the data element.
    *  @param   type
    *           The type of the data element, which is used to determine the
    *           kind of element to create when createElement() is called.  To
    *           create a data element from the returned data descriptor, the
    *           type string must be one of the valid element types returned
    *           from getValidElementTypes().  For raster data types, a
    *           RasterDataDescriptor is created.  For all other types, a
    *           DataDescriptor is created.
    *  @param   pParent
    *           An optional parent element to which the new element will be
    *           associated.  Passing in \c NULL indicates that the new element
    *           is not associated with another element.  If the parent is
    *           non-\b NULL, the returned element is automatically destroyed
    *           when the parent element is destroyed. The new DataDescriptor
    *           inherits the parent's classification unless the parent is \c NULL,
    *           in which case the classification will be set to the system's
    *           highest level of classification.
    *
    *  @return  Returns a pointer to the newly created data descriptor.
    *
    *  @see     createElement()
    */
   virtual DataDescriptor* createDataDescriptor(const std::string& name, const std::string& type,
       DataElement* pParent) const = 0;

   /**
    *  Creates an empty data descriptor.
    *
    *  This method creates a DataDescriptor with a parent specified as a vector of strings.
    *  It should not be called if the parent data element already exists. It is to be used when the
    *  parent has not yet been created. Since the parent should not exist, the classification will
    *  be set to the highest level of the system. It is the responsibility of the caller to set
    *  the proper classification settings.
    *
    *  @param   name
    *           The name for the data element.
    *  @param   type
    *           The type of the data element, which is used to determine the
    *           kind of element to create when createElement() is called.  To
    *           create a data element from the returned data descriptor, the
    *           type string must be one of the valid element types returned
    *           from getValidElementTypes().  For raster data types, a
    *           RasterDataDescriptor is created.  For all other types, a
    *           DataDescriptor is created.
    *  @param   parent
    *           A parent element to which the new element will be associated.
    *           This parent is specified as a vector of names of elements. The
    *           first name in the vector is the top-level element, the next name
    *           is a child of that element, and so on. Passing an empty vector
    *           will create a top-level element.
    *
    *  @return  Returns a pointer to the newly created data descriptor.
    *
    *  @see     createElement()
    */
   virtual DataDescriptor* createDataDescriptor(const std::string& name, const std::string& type,
      const std::vector<std::string> &parent) const = 0;

   /**
    *  Destroys a data descriptor.
    *
    *  This destroys a data descriptor that was created with the
    *  createDataDescriptor() method.
    *
    *  @param   pDescriptor
    *           The data descriptor to destroy.
    */
   virtual void destroyDataDescriptor(DataDescriptor* pDescriptor) const = 0;

   /**
    *  Creates an import descriptor.
    *
    *  This method creates an import descriptor that contains a data descriptor
    *  that can be populated so that a data element can be imported.  Ownership
    *  of the import descriptor is transferred to the caller, so the
    *  destroyImportDescriptor() method should be called to destroy the object.
    *  The import descriptor maintains ownership of the created data
    *  descriptor.
    *
    *  @param   name
    *           The name for the data element.
    *  @param   type
    *           The type of the data element, which is used to determine the
    *           kind of element to create when createElement() is called.  To
    *           create a data element from the returned data descriptor, the
    *           type string must be one of the valid element types returned
    *           from getValidElementTypes().  For raster data types, a
    *           RasterDataDescriptor is created.  For all other types, a
    *           DataDescriptor is created.
    *  @param   pParent
    *           An optional parent element to which the new element will be
    *           associated.  Passing in \c NULL indicates that the new element
    *           is not associated with another element.  If the parent is
    *           non-\c NULL, the returned element is automatically destroyed
    *           when the parent element is destroyed. The created data descriptor
    *           in the new import descriptor inherits the parent's classification
    *           unless the parent is \c NULL, in which case the classification
    *           will be set to the system's highest level of classification.
    *  @param   bImported
    *           Set this parameter to \c true to import the element or to
    *           \c false to not create the data element on import.
    *
    *  @return  Returns a pointer to the newly created import descriptor.
    *
    *  @see     createImportDescriptor(DataDescriptor*, bool) const
    */
   virtual ImportDescriptor* createImportDescriptor(const std::string& name, const std::string& type,
      DataElement* pParent, bool bImported = true) const = 0;

   /**
    *  Creates an import descriptor.
    *
    *  This method creates an import descriptor whose parent is specified as a vector of names. 
    *  It should not be called if the parent data element already exists. It is to be used when the
    *  parent has not yet been created, e.g., in an importer to display the hierarchy of data elements
    *  to be imported in the import options widget. Since the parent should not exist, the classification
    *  will be set to the highest level of the system. It is the responsibility of the caller to set the
    *  proper classification settings.
    *
    *  @param   name
    *           The name for the data element.
    *  @param   type
    *           The type of the data element, which is used to determine the
    *           kind of element to create when createElement() is called.  To
    *           create a data element from the returned data descriptor, the
    *           type string must be one of the valid element types returned
    *           from getValidElementTypes().  For raster data types, a
    *           RasterDataDescriptor is created.  For all other types, a
    *           DataDescriptor is created.
    *  @param   parent
    *           A parent element to which the new element will be associated.
    *           This parent is specified as a vector of names of elements. The
    *           first name in the vector is the top-level element, the next name
    *           is a child of that element, and so on. Passing an empty vector will
    *           create a top-level element.
    *  @param   bImported
    *           Set this parameter to \c true to import the element or to
    *           \c false to not create the data element on import.
    *
    *  @return  Returns a pointer to the newly created import descriptor.
    *
    *  @see     createImportDescriptor(DataDescriptor*, bool) const,
    *           createDataDescriptor(const std::string& name, const std::string& type, const std::vector<std::string> &parent) const
    */
   virtual ImportDescriptor* createImportDescriptor(const std::string& name, const std::string& type,
      const std::vector<std::string> &parent, bool bImported = true) const = 0;

   /**
    *  Creates an import descriptor.
    *
    *  This method creates an import descriptor that contains a given data
    *  descriptor so that a data element can be imported.  Ownership of the
    *  import descriptor is transferred to the caller, so the
    *  destroyImportDescriptor() method should be called to destroy the object.
    *  Ownership of the given data descriptor is transferred to the created
    *  import descriptor.
    *
    *  @param   pDescriptor
    *           The data descriptor with which to create the data element on
    *           import.
    *  @param   bImported
    *           Set this parameter to \c true to create the data element and
    *           load the data on import or to \c false to not create the data
    *           element on import.
    *
    *  @return  Returns a pointer to the newly created import descriptor.
    *
    *  @see     createImportDescriptor(const std::string&, const std::string&, DataElement*, bool) const
    */
   virtual ImportDescriptor* createImportDescriptor(DataDescriptor* pDescriptor, bool bImported = true) const = 0;

   /**
    *  Destroys an import descriptor.
    *
    *  This destroys an import descriptor that was created with the
    *  createImportDescriptor() method.
    *
    *  @param   pImportDescriptor
    *           The import descriptor to destroy.
    */
   virtual void destroyImportDescriptor(ImportDescriptor* pImportDescriptor) const = 0;

   /**
    *  Adds a new element type to the list of valid types.
    *
    *  This method adds a new element type to the list of valid types and also
    *  adds the new type as a valid plug-in arg type in PlugInManagerServices.
    *
    *  New element types are added to distinguish between multiple custom data
    *  types stored in an Any element.  A new plug-in arg type is automatically
    *  registered with PlugInManagerServices, and when creating an arg with the
    *  new element type, the arg value must be set to the Any element and not
    *  the AnyData object contained in the Any element.
    *
    *  @warning If the new element type is going to be used as the type in a
    *           PlugInArg, this method should be called before setting the arg
    *           type in Executable::getInputSpecification() or
    *           Executable::getOutputSpecification().
    *
    *  @param   elementType
    *           The new element type.
    *
    *  @return  Returns \b true if the element type was successfully added to
    *           the valid types list.  Returns \b false if the element type
    *           already exists in the list, or if an empty string is passed in.
    *
    *  @see     getValidElementTypes()
    */
   virtual bool addElementType(const std::string& elementType) = 0;

   /**
    *  Queries whether a given type is a valid element type.
    *
    *  @param   elementType
    *           The type to query whether it is a valid element type.
    *
    *  @return  Returns \b true if the given type is a valid element type;
    *           otherwise returns \b false.
    *
    *  @see     getValidElementTypes()
    */
   virtual bool hasElementType(const std::string& elementType) const = 0;

   /**
    *  Returns the valid element types.
    *
    *  This method returns the names of each recognized element type.  Custom
    *  element types can be added to the list by calling the addElementType()
    *  method.  When a element with a custom type is created, an Any element
    *  is created.
    *
    *  The default element types are as follows:
    *  - AnnotationElement
    *  - Any
    *  - AoiElement
    *  - DataElement
    *  - DataElementGroup
    *  - GcpList
    *  - RasterElement
    *  - Signature
    *  - SignatureSet
    *  - TiePointList
    *
    *  @return  A vector of strings specifying the name of each valid element
    *           type.
    */
   virtual const std::vector<std::string>& getValidElementTypes() const = 0;

   /**
    *  Creates multiple data elements as a batch, ensuring proper parentage.
    *
    *  This method creates a data element for each DataDescriptor. When created,
    *  each element will have the parentage specified in the DataDescriptor. The
    *  group will be created in an order that guarantees a new element's parent exists
    *  before it is created as long as that parent exists before the call to createElements()
    *  or is part of the group of new elements. If a new element requests a parent that does
    *  not already exist and is not in the vector of DataDescriptors, that element will not be created.
    *
    *  @param descriptors
    *         A vector of DataDescriptors describing the elements to be created.
    *
    *  @return A vector of the new elements. If one or more elements can not be created, this method will
    *          not fail fast. A partial error can be detected by comparing the size of the return value
    *          with the size of descriptors. Only successfully created elements will be returned. If an element
    *          can not be created and has children in descriptors, those children will also fail to be created.
    *
    *  @see     createElement(const DataDescriptor*)
    */
   virtual std::vector<DataElement*> createElements(const std::vector<DataDescriptor*> &descriptors) = 0;

   /**
    *  Creates an empty data element with no data.
    *
    *  This method creates an element based on the given data descriptor.  The
    *  type in the data descriptor must be one of the valid element types
    *  returned from getValidElementTypes().
    *
    *  The memory for the element is created and maintained by the studio.  A
    *  deep copy of the given data descriptor is made so the calling object
    *  should destroy the data descriptor after calling this method. The new
    *  element inherits the classification settings of the passed in data descriptor.
    *
    *  @param   pDescriptor
    *           The data descriptor from which to create the data element.  The
    *           type in the data descriptor is used to determine the kind of
    *           element to create and must be one of the types returned from
    *           getValidElementTypes().  If a valid custom type is specified,
    *           an Any element is created.  This method does nothing if \c NULL
    *           is passed in.
    *
    *  @return  This method returns a pointer to the newly created data
    *           element.  \c NULL is returned in the following conditions:
    *           - The type in the data descriptor is not a valid element type
    *             returned from getValidElementTypes().
    *           - The parent element specified by the parent designator in the
    *             data descriptor (see DataDescriptor::getParentDesignator())
    *             does not exist.
    *           - The element could not be created (i.e. the element already
    *             exists).
    *           - The element type is RasterElement with a processing location of \link ProcessingLocation::IN_MEMORY IN_MEMORY \endlink
    *             and the required amount of memory could not be allocated.
    *
    *  @warning For data element type RasterElement with a processing location of \link ProcessingLocation::ON_DISK ON_DISK \endlink
    *           or \link ProcessingLocation::ON_DISK_READ_ONLY ON_DISK_READ_ONLY \endlink, this method will return a non-\c NULL pointer
    *           even if the temporary file required for the element can't be created. This behavior is necessary
    *           to prevent performance problems with importers. If you use this method to create an
    *           \link ProcessingLocation::ON_DISK ON_DISK \endlink or \link ProcessingLocation::ON_DISK_READ_ONLY ON_DISK_READ_ONLY \endlink
    *           raster element, a \c NULL check on the return from this method will not insure that the
    *           returned data element is usable. A better way is to check the return from a call to
    *           RasterElement::createDefaultPager() which will return \c false if the associated temporary file
    *           can not be created. For an even simpler approach, use RasterUtilities::createRasterElement() instead of
    *           this method to create the data element. RasterUtilities::createRasterElement() will return \c NULL
    *           if either the memory can't be allocated or the temporary file can't be created.
    *
    *  @notify  This method will notify signalElementCreated() with
    *           boost::any<DataElement*>.
    *
    *  @see     createElement(const std::string&, const std::string&, DataElement*),
    *           RasterUtilities::createRasterElement()
    */
   virtual DataElement* createElement(const DataDescriptor* pDescriptor) = 0;

   /**
    *  Creates an empty data element with no data.
    *
    *  This is a convenience method that creates a data element without needing
    *  to first create a data descriptor and then destroy the data descriptor
    *  after the element is created.  It is typically used to create simple
    *  data elements such as AOIs and GCP lists. The new element inherits the classification
    *  settings of the parent element unless the parent is \c NULL, in which case
    *  the new element's classification will be set to the system's highest level. 
    *
    *  @param   name
    *           The string name for the data element to create.
    *  @param   type
    *           The string name corresponding to the type of the data element
    *           to create.  The types must be one of the valid element types
    *           returned from getValidElementTypes().  If a valid custom type
    *           is passed in, an Any element is created.
    *  @param   pParent
    *           The parent element of the element to create.  Passing in
    *           \b NULL indicates that the element is not associated with
    *           another element.  If the parent is non-\b NULL, the returned
    *           element is automatically destroyed when the parent element is
    *           destroyed.
    *
    *  @return  This method returns a pointer to the newly created data
    *           element.  \b NULL is returned if the given type is not a valid
    *           element type returned from getValidElementTypes() or if the
    *           element could not be created (i.e. the element already exists).
    *
    *  @notify  This method will notify signalElementCreated with any<DataElement*>.
    *
    *  @see     createElement(const DataDescriptor*)
    */
   virtual DataElement* createElement(const std::string& name, const std::string& type, DataElement* pParent) = 0;

   /**
    *  Retrieves a data element.
    *
    *  @param   name
    *           The string name for the data element to retrieve.
    *  @param   type
    *           The string name corresponding to the type of the data element
    *           to retrieve. If this is an empty string, an element of any type is retrieved.
    *  @param   pParent
    *           The parent element of the element to get.  Passing in
    *           \b NULL indicates that the element is not associated with
    *           another element.
    *
    *  @return  This method returns a pointer to the data element with the
    *           given parameters.  If no element is found where all of the
    *           parameters match, \b NULL is returned.
    *
    *  @see     model_cast()
    */
   virtual DataElement* getElement(const std::string& name, const std::string& type,
      const DataElement* pParent) const = 0;

   /**
    *  Retrieves a data element.
    *
    *  @param   designator
    *           The element designator for the data element to retrieve.
    *  @param   type
    *           The string name corresponding to the type of the data element
    *           to retrieve. If this is an empty string, an element of any type is retrieved.
    *  @return  This method returns a pointer to the data element with the
    *           given parameters.  If no element is found where all of the
    *           parameters match, \b NULL is returned.
    *
    *  @see     model_cast(), DataElement::getParentDesignator()
    */
   virtual DataElement* getElement(const std::vector<std::string>& designator, const std::string& type) const = 0;

   /**
    *  Retrieves a data element.
    *
    *  @param   pDescriptor
    *           A data descriptor containing the name, type, and parent or
    *           parent designator for the element to retrieve.
    *
    *  @return  Returns a pointer to the data element with the name, type, and
    *           parent that are set in the given data descriptor.  If no element
    *           is found where all of the parameters match, \c NULL is returned.
    *
    *  @see     model_cast()
    */
   virtual DataElement* getElement(const DataDescriptor* pDescriptor) const = 0;

   /**
    *  Retrieves elements of a given type.
    *
    *  @param   type
    *           The string name corresponding to the element type to retrieve.
    *           If an empty string is passed in, all elements in the model are
    *           retrieved.
    *
    *  @return  A vector containing pointers to the data elements of the given
    *           type.
    *
    *  @see     getElements(const DataElement*, const std::string&) const,
    *           getElements(const std::string&, const std::string&) const
    */
   virtual std::vector<DataElement*> getElements(const std::string& type) const = 0;

   /**
    *  Retrieves elements with a given parent.
    *
    *  @param   pParent
    *           The parent element for which all child elements should be
    *           retrieved.  Passing in \b NULL retrieves all top-level
    *           elements.
    *  @param   type
    *           An optional type string for retreiving elements of a particular
    *           type from the given parent.  If an empty string is passed in,
    *           all elements with the given parent are retrieved.
    *
    *  @return  A vector containing pointers to the data elements with the
    *           given parent.
    *
    *  @see     getElements(const std::string&) const,
    *           getElements(const std::string&, const std::string&) const
    */
   virtual std::vector<DataElement*> getElements(const DataElement* pParent, const std::string& type) const = 0;

   /**
    *  Retrieves elements with a given file.
    *
    *  @param   filename
    *           The filename for which all elements with the same filename
    *           should be retrieved.  Passing in an empty string retrieves
    *           all elements that have no associated file.
    *  @param   type
    *           An optional type string for retreiving elements of a particular
    *           type within the given file.  If an empty string is passed in,
    *           all elements within the given file are retrieved.
    *
    *  @return  A vector containing pointers to the data elements within the
    *           given file.
    *
    *  @see     getElements(const std::string&) const,
    *           getElements(const DataElement*, const std::string&) const
    */
   virtual std::vector<DataElement*> getElements(const std::string& filename, const std::string& type) const = 0;

   /**
    *  Retrieves the names of elements of a given type.
    *
    *  @param   type
    *           The string name corresponding to the element type for which to
    *           retrieve the names.  If an empty string is passed in, the names
    *           of all elements in the model are retrieved.
    *
    *  @return  A vector containing the names of the data elements of the given
    *           type.
    *
    *  @see     getElementNames(const DataElement*, const std::string&) const,
    *           getElementNames(const std::string&, const std::string&) const
    */
   virtual std::vector<std::string> getElementNames(const std::string& type) const = 0;

   /**
    *  Retrieves the names of elements with a given parent.
    *
    *  @param   pParent
    *           The parent element for which the names of all child elements
    *           should be retrieved.  Passing in \b NULL retrieves the names
    *           of all top-level elements.
    *  @param   type
    *           An optional type string for retreiving the names of elements
    *           of a particular type from the given parent.  If an empty
    *           string is passed in, the names of all elements with the given
    *           parent are retrieved.
    *
    *  @return  A vector containing the names of the data elements with the
    *           given parent.
    *
    *  @see     getElementNames(const std::string&) const,
    *           getElementNames(const std::string&, const std::string&) const
    */
   virtual std::vector<std::string> getElementNames(const DataElement* pParent, const std::string& type) const = 0;

   /**
    *  Retrieves the names of elements with a given file.
    *
    *  @param   filename
    *           The filename for which the names of all elements with the same
    *           filename should be retrieved.  Passing in an empty string
    *           retrieves the name of all elements that have no associated file.
    *  @param   type
    *           An optional type string for retreiving the names of elements of
    *           a particular type within the given file.  If an empty string is
    *           passed in, the names of all elements within the given file are
    *           retrieved.
    *
    *  @return  A vector containing the names of the data elements with the
    *           given file.
    *
    *  @see     getElementNames(const std::string&) const,
    *           getElementNames(const DataElement*, const std::string&) const
    */
   virtual std::vector<std::string> getElementNames(const std::string& filename, const std::string& type) const = 0;

   /**
    *  Renames a data element.
    *
    *  @param   pElement
    *           The existing element in the session to rename.  This method
    *           does nothing if \b NULL is passed in.
    *  @param   name
    *           The new name for the element.  If an empty string is passed in,
    *           the element is not renamed.
    *
    *  @return  True if the rename was successful, false otherwise.  This
    *           operation will fail if a DataElement already exists with the new
    *           name, type, and parent as this element.
    */
   virtual bool setElementName(DataElement* pElement, const std::string& name) = 0;

   /**
    *  Reparents a data element. The classification settings of the reparented
    *  element will not be changed, i.e., they will not be set to the new parent's settings.
    *
    *  @param   pElement
    *           The existing element in the session to reparent.  This method
    *           does nothing if \b NULL is passed in.
    *  @param   pParent
    *           The new parent for the element.
    *
    *  @return  Returns \b true if the reparent was successful, otherwise
    *           returns \b false.  This operation will fail if the given parent
    *           element already contains a child element with the same name and
    *           type as the given element.
    *
    *  @notify  This method will notify signalElementReparented() with
    *           boost::any<DataElement*> if the element is successfully
    *           reparented.
    */
   virtual bool setElementParent(DataElement* pElement, DataElement *pParent) = 0;

   /**
    *  Removes a data element from the session.
    *
    *  This method removes a data element from the session but does not
    *  delete it.  To remove and delete the element, call the destroyElement()
    *  method instead.
    *
    *  @param   pElement
    *           The data element to be removed.
    *
    *  @return  Returns \b true if element was successfully removed; otherwise
    *           returns \b false.
    */
   virtual bool removeElement(const DataElement* pElement) = 0;

   /**
    *  Removes a data element from the session and deletes it.
    *
    *  This method removes a data element from the session and deletes it and
    *  all of its child elements.  To remove the element without deleting it
    *  and its children, call the removeElement() method instead.
    *
    *  @param   pElement
    *           The element to remove and delete, along with its child
    *           elements.  This method does nothing if \b NULL is passed in.
    *
    *  @return  Returns \b true if the element was successfully removed and
    *           deleted; otherwise returns \b false.
    *
    *  @notify  This method will notify signalElementDestroyed with 
    *           any<DataElement*>. The DataElement pointer is still valid at 
    *           the time of notification.
    */
   virtual bool destroyElement(DataElement* pElement) = 0;

   /**
    *  Removes and destroys all elements in the session.
    *
    *  @notify  This method will notify signalElementDestroyed with 
    *           any<DataElement*> for each element destroyed. The DataElement 
    *           pointer is still valid at the time of notification.
    */
   virtual void clear() = 0;

   /**
    *  Allocates a large block of memory.
    *
    *  This method allocates a contiguous block of memory of a given size and
    *  returns a pointer to the memory block.  This method can be used to
    *  create space for a dataset in the studio's memory space that it can be
    *  accessed when a plug-in has been unloaded.
    *
    *  NOTE: On a 64-bit platform, the maximum available bytes to allocate is
    *  2^64, which is well over 18 million GB.  On a 32-bit platform, the
    *  maximum available bytes to allocate is 4 GB.
    *
    *  @param   size
    *           The size in bytes of the requested memory block.
    *  @return  A pointer to the new memory block.  \b NULL is returned if
    *           a contiguous block of memory of the given size cannot be
    *           allocated.  On Windows systems, \b NULL may be returned
    *           even if the amount of free memory is greater than <em>size</em>
    *           if the location of any system DLLs fragments the memory space
    *           so that a contiguous block is not available.
    *
    *  @see     deleteMemoryBlock()
    */
   virtual char* getMemoryBlock(size_t size) = 0;

   /**
    *  Deletes a block of memory allocated by the studio.
    *
    *  This method is used to delete a block of memory created by the
    *  getMemoryBlock() method.
    *
    *  @param   memory
    *           A pointer to the memory block to be deleted.
    */
   virtual void deleteMemoryBlock(char* memory) = 0; 

   /**
    *  This static method retrieves an individual data value from a block of memory.
    *
    *  @param   type
    *           The data type of the memory block.
    *  @param   pData
    *           A pointer to the block of memory from which to get the data
    *           value.
    *  @param   iIndex
    *           The index into the memory block for the data value.
    *
    *  @return  The data value.  A value of 0.0 is returned if the given type
    *           is not recognized.
    */
   static double getDataValue(EncodingType type, const void* pData, int iIndex)
   {
      return getDataValue(type, pData, COMPLEX_MAGNITUDE, iIndex);
   }

   /**
    *  This static method retrieves an individual data value from a block of memory.
    *
    *  @param   type
    *           The data type of the memory block.
    *  @param   pData
    *           A pointer to the block of memory from which to get the data
    *           value.
    *  @param   component
    *           For complex data, this specifies the component of the complex
    *           data that should be returned.  For non-complex data, this
    *           value is ignored.
    *  @param   iIndex
    *           The index into the memory block for the data value.
    *
    *  @return  The data value.  A value of 0.0 is returned if the given type
    *           is not recognized.
    *
    *  @see     ComplexComponent
    */
   static double getDataValue(EncodingType type, const void* pData, ComplexComponent component, int iIndex)
   {
      double dValue = 0.0;
      switchOnComplexEncoding(type, ModelServices::getDataValue, pData, component, iIndex, dValue);
      return dValue;
   }

   /**
    *  Retrieves an individual data value from a block of memory independent
    *  of the data type.
    *
    *  This static method provides the same functionality as the corresponding
    *  virtual method, but can be used inside of other template methods.  This
    *  can provide a significant performance improvement if getting numerous
    *  data values.
    *
    *  @param   pData
    *           A pointer to the block of memory from which to get the data
    *           value.
    *  @param   component
    *           For complex data, this specifies the component of the complex
    *           data that should be returned.  For non-complex data, this value
    *           is ignored.
    *  @param   iIndex
    *           The index into the memory block for the data value.
    *  @param   dValue
    *           Populated with the data value.
    */
   template<class T>
   static inline void getDataValue(const T* pData, ComplexComponent component, int iIndex, double& dValue)
   {
      if (pData != NULL)
      {
         dValue = ModelServices::getDataValue(pData[iIndex], component);
      }
   }

   /**
    *  Provided for convenience for template methods, this method simply returns the given value.
    *
    *  @param    data
    *            The data value from which to extract the complex component value.
    *  @param    component
    *            This value is ignored since the given data value is non-complex.
    *
    *  @return   The given data value.
    */
   static inline unsigned char getDataValue(const unsigned char& data, ComplexComponent component)
   {
      return data;
   }

   /**
    *  Provided for convenience for template methods, this method simply returns the given value.
    *
    *  @param    data
    *            The data value from which to extract the complex component value.
    *  @param    component
    *            This value is ignored since the given data value is non-complex.
    *
    *  @return   The given data value.
    */
   static inline signed char getDataValue(const signed char& data, ComplexComponent component)
   {
      return data;
   }

   /**
    *  Provided for convenience for template methods, this method simply returns the given value.
    *
    *  @param    data
    *            The data value from which to extract the complex component value.
    *  @param    component
    *            This value is ignored since the given data value is non-complex.
    *
    *  @return   The given data value.
    */
   static inline unsigned short getDataValue(const unsigned short& data, ComplexComponent component)
   {
      return data;
   }

   /**
    *  Provided for convenience for template methods, this method simply returns the given value.
    *
    *  @param    data
    *            The data value from which to extract the complex component value.
    *  @param    component
    *            This value is ignored since the given data value is non-complex.
    *
    *  @return   The given data value.
    */
   static inline signed short getDataValue(const signed short& data, ComplexComponent component)
   {
      return data;
   }

   /**
    *  Returns a data value according to a given complex data component.
    *
    *  @param    data
    *            The data value from which to extract the complex component value.
    *  @param    component
    *            For complex data, this specifies the component of the complex data whose
    *            value should be returned.
    *
    *  @return   The data value corresponding to the given complex data component.
    */
   static inline double getDataValue(const IntegerComplex& data, ComplexComponent component)
   {
      return data[component];
   }

   /**
    *  Provided for convenience for template methods, this method simply returns the given value.
    *
    *  @param    data
    *            The data value from which to extract the complex component value.
    *  @param    component
    *            This value is ignored since the given data value is non-complex.
    *
    *  @return   The given data value.
    */
   static inline unsigned int getDataValue(const unsigned int& data, ComplexComponent component)
   {
      return data;
   }

   /**
    *  Provided for convenience for template methods, this method simply returns the given value.
    *
    *  @param    data
    *            The data value from which to extract the complex component value.
    *  @param    component
    *            This value is ignored since the given data value is non-complex.
    *
    *  @return   The given data value.
    */
   static inline signed int getDataValue(const signed int& data, ComplexComponent component)
   {
      return data;
   }

   /**
    *  Provided for convenience for template methods, this method simply returns the given value.
    *
    *  @param    data
    *            The data value from which to extract the complex component value.
    *  @param    component
    *            This value is ignored since the given data value is non-complex.
    *
    *  @return   The given data value.
    */
   static inline float getDataValue(const float& data, ComplexComponent component)
   {
      return data;
   }

   /**
    *  Returns a data value according to a given complex data component.
    *
    *  @param    data
    *            The data value from which to extract the complex component value.
    *  @param    component
    *            For complex data, this specifies the component of the complex data whose
    *            value should be returned.
    *
    *  @return   The data value corresponding to the given complex data component.
    */
   static inline double getDataValue(const FloatComplex& data, ComplexComponent component)
   {
      return data[component];
   }

   /**
    *  Provided for convenience for template methods, this method simply returns the given value.
    *
    *  @param    data
    *            The data value from which to extract the complex component value.
    *  @param    component
    *            This value is ignored since the given data value is non-complex.
    *
    *  @return   The given data value.
    */
   static inline double getDataValue(const double& data, ComplexComponent component)
   {
      return data;
   }

   /**
    *  Queries a element to see if it is a kind of another element.
    *
    *  This method check type compatibility between two data elements.  This
    *  functionality is different than TypeAwareObject::isKindOf() in that only
    *  DataElement-derived classes are checked.
    *
    *  @param   className
    *           The name of the DataElement-derived class to query.
    *  @param   elementName
    *           The DataElement-derived class name to check type compatibility.
    *
    *  @return  Returns true if the given element name is a kind of the given class
    *           name, otherwise false.
    */
   virtual bool isKindOfElement(const std::string& className, const std::string& elementName) const = 0;

   /**
    *  Returns a list of inherited data element class names for a given class name.
    *
    *  This method populates a vector with the class names of all inherited DataElement
    *  class types.  The given class name is used as the initial class for populating the
    *  vector.  For example, passing in "RasterElement" as a class name populates a
    *  vector with the "RasterElement" and "DataElement"
    *  strings.
    *
    *  @param   className
    *           The data element class name for which to get all inherited element types.
    *  @param   classList
    *           This vector is populated with the class names of all inherited data
    *           element classes and the given class name.
    */
   virtual void getElementTypes(const std::string& className, std::vector<std::string>& classList) const = 0;

   /**
    *  Queries a data descriptor to see if it is a kind of another data descriptor.
    *
    *  This method check type compatibility between two data descriptors.  This
    *  functionality is different than TypeAwareObject::isKindOf() in that only
    *  DataDescriptor-derived classes are checked.
    *
    *  @param   className
    *           The name of the DataDescriptor-derived class to query.
    *  @param   descriptorName
    *           The DataDescriptor-derived class name to check type compatibility.
    *
    *  @return  Returns true if the given data descriptor name is a kind of the
    *           given class name, otherwise false.
    */
   virtual bool isKindOfDataDescriptor(const std::string& className, const std::string& descriptorName) const = 0;

   /**
    *  Returns a list of inherited data descriptor class names for a given class name.
    *
    *  This method populates a vector with the class names of all inherited DataDescriptor
    *  class types.  The given class name is used as the initial class for populating the
    *  vector.  For example, passing in "RasterDataDescriptor" as a class name populates a
    *  vector with the "RasterDataDescriptor" and "DataDescriptor" strings.
    *
    *  @param   className
    *           The data descriptor class name for which to get all inherited descriptor types.
    *  @param   classList
    *           This vector is populated with the class names of all inherited data
    *           descriptor classes and the given class name.
    */
   virtual void getDataDescriptorTypes(const std::string& className, std::vector<std::string>& classList) const = 0;

   /**
    *  Queries a file descriptor to see if it is a kind of another file descriptor.
    *
    *  This method check type compatibility between two file descriptors.  This
    *  functionality is different than TypeAwareObject::isKindOf() in that only
    *  FileDescriptor-derived classes are checked.
    *
    *  @param   className
    *           The name of the FileDescriptor-derived class to query.
    *  @param   descriptorName
    *           The FileDescriptor-derived class name to check type compatibility.
    *
    *  @return  Returns true if the given file descriptor name is a kind of the
    *           given class name, otherwise false.
    */
   virtual bool isKindOfFileDescriptor(const std::string& className, const std::string& descriptorName) const = 0;

   /**
    *  Returns a list of inherited file descriptor class names for a given class name.
    *
    *  This method populates a vector with the class names of all inherited FileDescriptor
    *  class types.  The given class name is used as the initial class for populating the
    *  vector.  For example, passing in "RasterFileDescriptor" as a class name
    *  populates a vector with the "RasterFileDescriptor"  and "FileDescriptor", strings.
    *
    *  @param   className
    *           The file descriptor class name for which to get all inherited descriptor types.
    *  @param   classList
    *           This vector is populated with the class names of all inherited file
    *           descriptor classes and the given class name.
    */
   virtual void getFileDescriptorTypes(const std::string& className, std::vector<std::string>& classList) const = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~ModelServices() {}
};

/**
 *  Converts a pointer of one type to a pointer of another type.
 *
 *  This cast is provided as a convenience to directly cast from a DataElement
 *  as returned by ModelServices::getElement() to the custom AnyData object
 *  stored in an Any element.  If the requested type is registered with
 *  ModelServices::addElementType() and stored in an Any element, this cast
 *  returns a pointer to the custom data type; otherwise this cast is simply
 *  a wrapper around a dynamic_cast.
 *
 *  @param   pSourcePtr
 *           The pointer to convert to a pointer of the requested type.
 *
 *  @return  A pointer of the requested type.  \b NULL is returned if the
 *           given pointer cannot be dynamic_cast to the requested type or if
 *           the given Any element pointer does not contain data of the
 *           requested type.
 *
 *  @see     DataElement::isKindOf()
 *
 *  @relates ModelServices
 */
template<typename Destination, typename Source>
Destination model_cast(Source pSourcePtr)
{
   if (pSourcePtr == NULL)
   {
      return NULL;
   }

   Destination pDestPtr = dynamic_cast<Destination>(pSourcePtr);
   if (pDestPtr == NULL)
   {
      Any* pAny = dynamic_cast<Any*>(pSourcePtr);
      if (pAny != NULL)
      {
         return dynamic_cast<Destination>(pAny->getData());
      }
   }

   return pDestPtr;
}

#endif
