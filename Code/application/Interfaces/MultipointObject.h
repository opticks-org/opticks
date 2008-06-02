/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MULTIPOINTOBJECT_H
#define MULTIPOINTOBJECT_H

#include "GraphicObject.h"

#include <vector>

/**
 * This class provides access to the display properties for a multipoint object.
 *
 * Possible GraphicObjectTypes: MULTIPOINT_OBJECT.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - All notifications documented in GraphicObject.
 */
class MultipointObject : public GraphicObject
{
public:
   /**
    * Get the verticies which define the object.
    *
    * @return A vector containing the verticies.
    */
   virtual const std::vector<LocationType> &getVertices() const = 0;

protected:
   /**
    * This should be destroyed by calling GraphicLayer::removeObject.
    */
   virtual ~MultipointObject() {}
};

#endif
