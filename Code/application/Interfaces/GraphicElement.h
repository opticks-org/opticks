/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include "DataElement.h"

class GraphicGroup;
class RasterElement;

/**
 * GraphicElement is a model class used for vector-based graphical data.
 *
 * This subclass of Subject will notify upon the following conditions:
 *  * Any graphic object within the element is modified.
 *  * The geocentricity of the element is modified via setGeocentric().
 *  * Everything documented in DataElement.
 */
class GraphicElement : public DataElement
{
public:
   /**
    * Get the group which contains the all vector objects in the element.
    *
    * @return The associated GraphicGroup.
    */
   virtual GraphicGroup *getGroup() = 0;

   /**
    * Get the group which contains the all vector objects in the element.
    *
    * @return The associated GraphicGroup.
    */
   virtual const GraphicGroup *getGroup() const = 0;

   /**
    *  Sets whether modifications made are interactive or programmatic.
    *
    *  For efficiency, it is sometimes useful to prevent refreshes and updates
    *  when a large amount of modifications are made to the layer.
    *
    *  @param   interactive
    *           Determines whether to go into interactive or batch mode.  If
    *           a value of false is passed in, all refreshes are delayed until
    *           the method is called again with a true value.
    *
    *  @see getInteractive()
    */
   virtual void setInteractive(bool interactive) = 0;

   /**
    *  Gets whether modifications made are interactive or programmatic.
    *
    *  @return The current state of the interactive flag.
    *
    *  @see setInteractive()
    */
   virtual bool getInteractive() const = 0;

   /**
    *  Sets whether graphic objects within this graphic element are geocentric
    *  in nature.
    *
    *  This method obtains georeferencing information from the element's parent
    *  and updates the bounding box locations of all graphic objects in the
    *  element to include latitude/longitude coordinates in addition to pixel
    *  coordinates.
    *
    *  After calling this method GraphicObject::getLlCorner() and
    *  GraphicObject::getUrCorner() will continue to return pixel coordinate
    *  locations.
    *
    *  @param  geocentric
    *          Whether the GraphicObjects are geocentric.
    *
    *  @return Returns \c true if the operation succeeded; \c false otherwise.
    *          The operation will fail if this element's parent is not a
    *          georeferenced RasterElement.
    *
    *  @notify This method will notify with Subject::signalModified() if the
    *          graphic objects are successfully set as geocentric.
    *
    *  @see    getGeocentric()
    */
   virtual bool setGeocentric(bool geocentric) = 0;

   /**
    *  Gets whether graphic objects within this graphic element are geocentric
    *  in nature.
    *
    *  @return Returns \c true if the graphic objects is this element are
    *          geocentric; \c false otherwise.
    *
    *  @see    setGeocentric()
    */
   virtual bool getGeocentric() const = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~GraphicElement() {}
};

#endif
