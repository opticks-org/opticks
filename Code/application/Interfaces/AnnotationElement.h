/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANNOTATIONELEMENT_H
#define ANNOTATIONELEMENT_H

#include "GraphicElement.h"

/**
 * AnnotationElement is a class used to contain vector data
 * for annotating a scene.
 *
 * This subclass of Subject will notify upon the following conditions:
 *  * Everything documented in GraphicElement.
 */
class AnnotationElement : public GraphicElement
{
protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~AnnotationElement() {}
};

#endif // ANNOTATIONELEMENT_H