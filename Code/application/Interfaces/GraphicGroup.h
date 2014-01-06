/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICGROUP_H
#define GRAPHICGROUP_H

#include "GraphicObject.h"
#include "LocationType.h"

#include <list>

class Progress;

/**
 * GraphicGroup is a graphic object which contains zero or more GraphicObjects.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - An object is added to the group by calling addObject() or insertObject().
 *  - An object is removed from the group by calling removeObject().
 *  - All notifications documented in GraphicObject.
 */
class GraphicGroup : public GraphicObject
{
public:
   /**
    *  Emitted with any<GraphicObject*> when an object is added to the group.
    */
   SIGNAL_METHOD(GraphicGroup, ObjectAdded)

   /**
    *  Emitted with any<GraphicObject*> when an object is about to be removed from the
    *  group.
    */
   SIGNAL_METHOD(GraphicGroup, ObjectRemoved)

   /**
    *  Emitted with any<GraphicProperty*> when a property of an object is changed.
    */
   SIGNAL_METHOD(GraphicGroup, ObjectChanged)

   /**
    *  Creates and inserts an object of the specified type.
    *
    *  @param   objectType
    *           The type of object to add.
    *  @param   point
    *           The starting point of the object.
    *
    *  @return  The added object.  The group owns this object.
    *
    *  @notify  This method will notify signalObjectAdded() with
    *           boost::any<#GraphicObject*>.
    *
    *  @see     addObjects()
    */
   virtual GraphicObject* addObject(GraphicObjectType objectType, LocationType point = LocationType()) = 0;

   /**
    *  Creates and inserts multiple objects of the specified type.
    *
    *  @param   numObjects
    *           The number of objects to add.
    *  @param   objectType
    *           The type of object to add.
    *  @param   point
    *           The starting point for each object.
    *  @param   pProgress
    *           An optional pointer to a Progress object in which to report
    *           progress and error messages while the objects are being created.
    *
    *  @return  A list containing pointers to all added objects.  All objects in
    *           the list are contained in and owned by the group.
    *
    *  @notify  This method will notify signalObjectAdded() with
    *           boost::any<#GraphicObject*> for each object that is successfully
    *           created and added to the group.
    *
    *  @see     addObject()
    */
   virtual std::list<GraphicObject*> addObjects(unsigned int numObjects, GraphicObjectType objectType,
      LocationType point = LocationType(), Progress* pProgress = NULL) = 0;

   /**
    *  Returns a list of all objects within the group.
    *
    *  @return  A list containing all objects that the group contains.
    *
    *  @see     getObjects(GraphicObjectType) const
    */
   virtual const std::list<GraphicObject*>& getObjects() const = 0;

   /**
    *  Returns a list of all graphic objects of a given type within the group.
    *
    *  @param   objectType
    *           The type of graphic object to get.
    *
    *  @return  A list that is filled with pointers to all graphic objects of
    *           the given type in the group.
    *
    *  @see     getObjects() const
    */
   virtual std::list<GraphicObject*> getObjects(GraphicObjectType objectType) const = 0;

   /**
    *  Returns the number of graphic objects in the group.
    *
    *  @return  The total number of graphic objects in the group.
    *
    *  @see     getNumObjects(GraphicObjectType) const
    */
   virtual unsigned int getNumObjects() const = 0;

   /**
    *  Returns the number of graphic objects of a given type in the group.
    *
    *  @param   objectType
    *           The type of object to query for the number of objects contained
    *           in the group.
    *
    *  @return  The number of objects of the given type in the group.
    *
    *  @see     getNumObjects() const
    */
   virtual unsigned int getNumObjects(GraphicObjectType objectType) const = 0;

   /**
    *  Inserts an already existing object into the group.
    *
    *  The group takes ownership over this object.  It is the 
    *  caller's responsibility to ensure that no other objects
    *  claim ownership of the object.
    *
    *  @param   pObject
    *           The object to insert.
    *
    *  @notify  This method will notify signalObjectAdded() with
    *           any<GraphicObject*>.
    */
   virtual void insertObject(GraphicObject *pObject) = 0;

   /**
    *  Removes an object from the group.
    *
    *  @param   pObject
    *           The object to remove.
    *  @param   bDelete
    *           Whether or not to delete the object.  If \b true, the object
    *           will be deleted.  If \b false, ownership of the object
    *           is transfered to the caller.
    *
    *  @return  Returns \b true if the object was successfully removed from the
    *           group, otherwise returns \b false.
    *
    *  @notify  This method will notify signalObjectRemoved() with
    *           any<GraphicObject*>.
    */
   virtual bool removeObject(GraphicObject *pObject, bool bDelete) = 0;

   /**
    * Remove all objects from the group.
    *
    * @param bDelete
    *        Whether or not to delete the objects.  If true, the objects
    *        will be deleted.  If false, ownership of the object
    *        is transfered to the caller.
    */
   virtual void removeAllObjects(bool bDelete) = 0;

   /**
    *  Queries whether the object is contained by this group.
    *
    *  @param   pObject
    *           The object to find.
    *
    *  @return  Returns \b true if the object is contained, otherwise returns
    *           \b false.
    */
   virtual bool hasObject(GraphicObject *pObject) = 0;

protected:
   /**
    * This should be destroyed by calling GraphicLayer::removeObject.
    */
   virtual ~GraphicGroup() {}
};

#endif
