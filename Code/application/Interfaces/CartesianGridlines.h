/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CARTESIANGRIDLINES_H
#define CARTESIANGRIDLINES_H

#include "Gridlines.h"
#include "TypesFile.h"

/**
 *  Horizontal or vertical gridlines for a CartesianPlot.
 *
 *  @see     Gridlines
 */
class CartesianGridlines : public Gridlines
{
public:
   /**
    *  Returns the orientation of the gridlines.
    *
    *  @return  The gridline orientation.
    */
   virtual OrientationType getOrientation() const = 0;

protected:
   /**
    *  A plug-in cannot create this object, it can only retrieve an already
    *  existing object by calling CartesianPlot::getGridlines().  The Cartesian
    *  plot manages its own instances of this object.
    */
   virtual ~CartesianGridlines() {}
};

#endif
