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
    *  Sets whether the GraphicObjects within this GraphicElement are
    *  geocentric in nature.  This object's parent will be used
    *  for georeferencing.
    *
    *  @param geocentric
    *         Whether the GraphicObjects are geocentric.
    *
    *  @return True if the operation succeeded, false otherwise.
    *          The operation will fail if this object's parent
    *          is not a georeferenced  RasterElement.
    *
    *  @notify  This method will notify with Subject::signalModified.
    *
    *  @see getGeocentric()
    */
   virtual bool setGeocentric(bool geocentric) = 0;

   /**
    *  Gets whether the GraphicObjects within this GraphicElement are
    *  geocentric in nature.
    *
    *  @return True if the objects are geocentric, false otherwise.
    *
    *  @see setGeocentric()
    */
   virtual bool getGeocentric() const = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~GraphicElement() {}
};

#endif
