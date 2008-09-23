/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MOVEOBJECTIMP_H
#define MOVEOBJECTIMP_H

#include "GraphicObjectImp.h"

class UndoGroup;

/**
 * This class performs the following operations in a GraphicLayer:
 *  * Selection of a single object by clicking somewhere within it
 *  * Multiple selection by shift-clicking a second object
 *  * Multiple selection by dragging a bounding box
 *  * Duplicating an object by ctrl-clicking on it
 *  * Moving selected objects
 *  * Moving a handle on a selected object (MOVE_OBJECT)
 *  * Rotating when selecting a handle on a selected object (ROTATE_OBJECT)
 */
class MoveObjectImp : public GraphicObjectImp
{
public:
   MoveObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   virtual ~MoveObjectImp();

   void draw(double zoomFactor) const {}

   bool hit(LocationType) const { return false; }

   bool processMousePress(LocationType sceneCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseMove(LocationType sceneCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseRelease(LocationType sceneCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseDoubleClick(LocationType sceneCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   template<typename T>
   bool applyHandleChange(const T &functor);

   bool isVisible() const { return false; }
   bool hasCornerHandles() const { return false; }
   bool insertionUndoable() const { return false; }

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   LocationType mLastCoord;
   LocationType mPressCoord;
   int mSelectedHandle;
   GraphicObject *mpLastSelectedObject;
   UndoGroup *mpUndoGroup;
   bool mLeftPressed;
};

#define MOVEOBJECTADAPTEREXTENSION_CLASSES \
   GRAPHICOBJECTADAPTEREXTENSION_CLASSES

#define MOVEOBJECTADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass)

class MoveObjectAdapter : public GraphicObject, public MoveObjectImp
{
public:
   MoveObjectAdapter(const std::string& id, GraphicObjectType eType, GraphicLayer *pLayer, LocationType pixelCoord) :
      MoveObjectImp(id, eType, pLayer, pixelCoord)
   {
   }

   ~MoveObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("MoveObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "MoveObject"))
      {
         return true;
      }

      return MoveObjectImp::isKindOf(className);
   }

   MOVEOBJECTADAPTER_METHODS(MoveObjectImp)
};

#endif
