/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SCALEBAROBJECT_H
#define SCALEBAROBJECT_H

#include "GraphicObject.h"

/**
 * This class provides access to the display properties for a scalebar object.
 *
 * Possible GraphicObjectTypes: SCALEBAR_OBJECT.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - All notifications documented in GraphicObject.
 */
class ScaleBarObject : public GraphicObject
{
protected:
   /**
    * This should be destroyed by calling GraphicLayer::removeObject.
    */
   virtual ~ScaleBarObject() {}
};

#endif
