/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef ANNOTATIONLAYER_H
#define ANNOTATIONLAYER_H

#include "GraphicLayer.h"
#include "LocationType.h"

#include <list>
#include <string>

/**
 *  Adjusts the properties of an Annotation layer.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - Everything else documented in GraphicLayer.
 *
 *  @see     Layer
 */
class AnnotationLayer : public GraphicLayer
{
public:
   /**
    * Correct the coordinate for whatever snapping may be required.
    *
    * AnnotationLayer will snap to gridlines if the user has selected this
    * option in the GUI.
    *
    * @param coord
    *        Coordinate to correct.
    *
    * @return The corrected coordinate.
    */
   virtual LocationType correctCoordinate(const LocationType &coord) const = 0;
   
   /**
    *  Copy a graphic object to the layer.
	*
	*  This method adds a graphic object to the layer which is a clone of the specified object.
	*
	*  @param pObject
	*         The object to clone.
	*
	*  @return A pointer to the new graphic object. NULL is returned if an error occured.
	*
	*  @notify This method will notify Subject::signalModified.
	*/
	virtual GraphicObject* cloneObject(const GraphicObject* pObject) = 0;

protected:
   /**
    * This should be destroyed by calling SpatialDataView::deleteLayer.
    */
   virtual ~AnnotationLayer() {}
};

#endif
