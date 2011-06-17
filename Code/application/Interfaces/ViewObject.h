/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VIEWOBJECT_H
#define VIEWOBJECT_H

#include "GraphicObject.h"

/**
 * This class provides access to the display properties for a view object.
 *
 * Possible GraphicObjectTypes: VIEW_OBJECT.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - A view is created to display in the object.  This may occur when calling
 *    setObjectView().
 *  - The displayed view is deleted. This may occur when calling
 *    setObjectView().
 *  - All notifications documented in GraphicObject.
 */
class ViewObject : public GraphicObject
{
public:
   /**
    *  Emitted with any<View*> when a view is created to display in the object.
    */
   SIGNAL_METHOD(ViewObject, ViewCreated)

   /**
    *  Emitted with any<View*> when the displayed view is deleted.
    */
   SIGNAL_METHOD(ViewObject, ViewDeleted)

protected:
   /**
    * This should be destroyed by calling GraphicLayer::removeObject.
    */
   virtual ~ViewObject() {}
};

#endif
