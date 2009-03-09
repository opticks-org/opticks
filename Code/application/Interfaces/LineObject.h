/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LINEOBJECT_H
#define LINEOBJECT_H

#include "GraphicObject.h"

/**
 * This class provides access to the display properties for a line object.
 *
 * Possible GraphicObjectTypes: LINE_OBJECT, HLINE_OBJECT, VLINE_OBJECT.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - All notifications documented in GraphicObject.
 */
class LineObject : public GraphicObject
{
protected:
   /**
    * This should be destroyed by calling GraphicLayer::removeObject.
    */
   virtual ~LineObject() {}
};

#endif
