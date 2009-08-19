/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATA_ELEMENT_GROUP
#define DATA_ELEMENT_GROUP

#include "DataElement.h"

#include <vector>

/**
 *  Provides for associations of DataElements.
 *  
 *  A data element group stores a list of DataElements. It assumes 
 *  ownership of the elements added to it, meaning that when the group is
 *  destroyed, all elements currently contained in the group will also be
 *  destroyed.
 *
 *  If a DataElement that has been added to the group is destroyed via
 *  ModelServices, the group will be notified and will remove the element
 *  from its internal tracking. This will trigger a notification of a change to the
 *  group.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *    - An element or group of elements is added to the group
 *    - An element or group of elements is removed from the group
 *    - Other notifications documented in the DataElement and Subject classes.
 *
 *  @see     DataElement
 */
class DataElementGroup : public DataElement
{
public:
   /**
    *  Emitted with any<DataElement*> when a member element is externally deleted.
    */
   SIGNAL_METHOD(DataElementGroup, ElementDeleted)

   /**
    *  Adds an element to the group. It is possible to add the same element to
    *  the group multiple times. If uniformity is being enforced and the element
    *  is of a different type from the elements already in the group, the method
    *  will fail. The method will also fail if pElement is NULL.
    *
    *  @param   pElement
    *           A pointer to the element to add to the group
    *
    *  @return  true if the element was added to the group, otherwise false.
    *
    *  @notify  Subject::signalModified
    *
    *  @see     enforceUniformity()
    */
   virtual bool insertElement(DataElement* pElement) = 0;

   /**
    *  Add several elements to the group. It is possible to add the same element
    *  to the group multiple times.
    *
    *  This method adds several elements to the group. If any of the elements to
    *  add are NULL, the method will fail, having done nothing. If uniformity is
    *  being enforced and any of the elements to add are of a different type
    *  from the elements in the group or each other, the method will fail,
    *  having done nothing.
    *
    *  @param   elements
    *           A vector of elements to add to the group
    *
    *  @return  true if the elements were added, otherwise false.
    *
    *  @notify  Subject::signalModified after the 
    *           elements are done being added to the list. Only one notification 
    *           will be done. No notification will be done if elements is empty.
    *
    *  @see     enforceUniformity()
    */
   virtual bool insertElements(const std::vector<DataElement*>& elements) = 0;

   /**
    *  Checks to see if an element is in the group.
    *
    *  @param   pElement
    *           The element to check for.
    *
    *  @return  true if the specified element is in the group, otherwise false.
    */
   virtual bool hasElement(DataElement* pElement) const = 0;

   /**
    *  Counts the elements in the group.
    *
    *  @return  the number of elements in the group.
    */
   virtual unsigned int getNumElements() const = 0;

   /**
    *  Gets the list of elements in the group. The elements are not guaranteed
    *  to be in any particular order.
    *
    *  @return  the list of elements in the group.
    */
   virtual const std::vector<DataElement*>& getElements() const = 0;

   /**
    *  Removes an element from the group, optionally deleting it as well.
    *
    *  @param   pElement
    *           A pointer to the element to remove from the group
    *
    *  @param   bDelete
    *           Specifies if the element should be destroyed as well as being 
    *           removed from the group. If the element is not found in the 
    *           group it will not be deleted, regardless of the value of this 
    *           parameter.
    *
    *  @return  true if the element was in the group, otherwise false.
    *
    *  @notify  Subject::signalModified after the 
    *           element is removed from the list.
    */
   virtual bool removeElement(DataElement* pElement, bool bDelete = false) = 0;

   /**
    *  Removes several elements from the group.
    *
    *  This method removes several elements from the group, optionally deleting
    *  them in the process. If the list of elements to remove contains elements
    *  not in the group, those elements will be ignored.
    *
    *  @param   elements
    *           A vector of elements to remove from the group
    *
    *  @param   bDelete
    *           Specifies if the elements should be destroyed as well as being 
    *           removed from the group. Elements not found in the group will
    *           not be deleted regardless of the value of this parameter.
    *
    *  @return  true if the the list of elements to remove was non-empty and 
    *           all of the specified elements were found in the group, 
    *           otherwise false.
    *
    *  @notify  Subject::signalModified after the 
    *           elements are done being removed from the list. Only one 
    *           notification will be done.
    */
   virtual bool removeElements(const std::vector<DataElement*>& elements, bool bDelete = false) = 0;

   /**
    *  Removes all of the elements from the group.
    *
    *  This method removes all of the elements from the group, optionally deleting
    *  them in the process.
    *
    *  @param   bDelete
    *           Specifies if the elements should be destroyed as well as being 
    *           removed from the group.
    *
    *  @notify  Subject::signalModified after the elements are
    *           done being removed from the group if the group was not empty.
    *           At most one notification will be done.
    */
   virtual void clear(bool bDelete = false) = 0;

   /**
    *  Specifies whether all elements added to a group must be of the same type.
    *
    *  This method specifies whether all elements in a group must be of the same
    *  type. If the group already has elements of varying types in it when this 
    *  method is called to turn on enforcement, it will fail to turn on enforcement. 
    *  When uniformity is enforced, addElement and addElements will fail to 
    *  add elements of types different from the type of the elements already 
    *  in the group. The default for a new DataElementGroup is for enforcement
    *  of uniformity to be off.
    *
    *  @param   enforce
    *           Specifies if enforcement is being turned on or off
    *
    *  @return  true if the enforcement setting was successfully set. It will
    *           always succeed in turning uniformity enforcement off. If 
    *           enforcement is being turned on and there are already elements
    *           of varying types in the group, it will fail and return false.
    */
   virtual bool enforceUniformity(bool enforce) = 0;

   /**
    *  Indicates if all of the elements in the group are of the same type.
    *
    *  @return  true if the group is empty or all elements in the group are of
    *           the same type, false otherwise.
    */
   virtual bool isUniform() const = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~DataElementGroup() {}
};

#endif
