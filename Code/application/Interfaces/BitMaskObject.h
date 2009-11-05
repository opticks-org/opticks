/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BITMASKOBJECT_H
#define BITMASKOBJECT_H

#include "GraphicObject.h"

/**
 * This class provides access to the display properties for a bitmask object.
 *
 * Possible GraphicObjectTypes: BITMASK_OBJECT.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - All notifications documented in GraphicObject.
 */
class BitMaskObject : public GraphicObject
{
public:
   /**
    * Sets the bitmask of the object to that in the parameter.
    *
    * @param pMask
    *        Bitmask to use for the object.
    * @param copy
    *        True if the object should make of copy of the mask.
    *        False if the object should reference the passed in mask.
    *        Note that using false does not pass ownership to
    *        this object.  It is the callers responsiblity to ensure
    *        that the mask remains in existance for as long as the
    *        bitmask object.
    *
    */
   virtual void setBitMask(const BitMask *pMask, bool copy = true) = 0;
   
protected:
   /**
    * This should be destroyed by calling GraphicLayer::removeObject.
    */
   virtual ~BitMaskObject() {}
};

#endif // BITMASKOBJECT_H
