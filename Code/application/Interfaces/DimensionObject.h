/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DIMENSIONOBJECT_H
#define DIMENSIONOBJECT_H

#include "GraphicObject.h"

/**
 * This class provides access to the display properties for a dimension object.
 *
 * A dimension object selects full rows or columns of the scene.
 *
 * Possible GraphicObjectTypes: ROW_OBJECT, COLUMN_OBJECT.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - All notifications documented in GraphicObject.
 */
class DimensionObject : public GraphicObject
{
protected:
   /**
    * This should be destroyed by calling GraphicLayer::removeObject.
    */
   virtual ~DimensionObject() {}
};

#endif
