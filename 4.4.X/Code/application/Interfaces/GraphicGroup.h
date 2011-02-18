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
#include <list>

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
    *  Creates and inserts an object of the specified type.
    *
    *  @param   type
    *           The type of object to add.
    *  @param   point
    *           The starting point of the object.
    *
    *  @return  The added object.  The group owns this object.
    *
    *  @notify  This method will notify signalObjectAdded() with
    *           any<GraphicObject*>.
    */
   virtual GraphicObject *addObject(GraphicObjectType type,
      LocationType point = LocationType()) = 0;

   /**
    *  Returns a list of all objects within the group.
    *
    *  @return  A list containing all objects that the group contains.
    */
   virtual const std::list<GraphicObject*> &getObjects() const = 0;

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
