/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLARGRIDLINES_H
#define POLARGRIDLINES_H

#include "Gridlines.h"

/**
 *  Circular gridlines for a PolarPlot.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The setRadialInterval() method is called.
 *  - Everything else documented in Gridlines.
 *
 *  @see        Gridlines
 */
class PolarGridlines : public Gridlines
{
public:
   /**
    *  Sets the angular interval of the lines extending out from the center of
    *  the polar plot.
    *
    *  @param   angle
    *           The angular interval for the lines extending out from the plot
    *           center in degrees.  The value should be between 0.0 and 360.0.
    *
    *  @notify  This method notifies Subject::signalModified() if the given
    *           interval is different than the current interval.
    */
   virtual void setRadialInterval(double angle) = 0;

   /**
    *  Returns the angular interval of the lines extending out from the center
    *  of the polar plot.
    *
    *  @return  The angular interval for the lines extending out from the plot
    *           center in degrees.  The value ranges from 0.0 to 360.0.
    */
   virtual double getRadialInterval() const = 0;

protected:
   /**
    *  A plug-in cannot create this object, it can only retrieve an already
    *  existing object by calling PolarPlot::getGridlines().  The polar plot
    *  manages its own instance of this object.
    */
   virtual ~PolarGridlines() {}
};

#endif
