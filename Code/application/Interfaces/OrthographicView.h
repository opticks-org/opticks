/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ORTHOGRAPHICVIEW_H
#define ORTHOGRAPHICVIEW_H

#include "LocationType.h"
#include "View.h"

/**
 *  A two-dimensional orthogonal view.
 *
 *  The orthographic view uses an orthonogal projection to draw its contents in two
 *  dimensions.  The data can be stretched or skewed to fit a desired display area.
 *  The aspect ratio of the data is not preserved by default, but can be made to do
 *  so by calling lockAspectRatio().
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The view is resized, panned, or zoomed.
 *  - Everything else documented in View.
 *
 *  @see     View, PerspectiveView
 */
class OrthographicView : public View
{
public:
   using View::getPixelSize;

   /**
    *  Emitted when the viewable area changes.
    */
   SIGNAL_METHOD(OrthographicView, DisplayAreaChanged)

   /**
    *  Locks the aspect ratio of the view.
    *
    *  This method toggles a lock of the view aspect ratio.  When locked, the
    *  values in the X dimension are displayed equally to the values in the Y
    *  dimension.  No distortion will occur when the viewport is resized.
    *
    *  @param   bLock
    *           Set this value to TRUE to lock the aspect ratio.  Set this value
    *           to FALSE to allow the view extents adjust freely with the view
    *           size.
    */
   virtual void lockAspectRatio(bool bLock) = 0;

   /**
    *  Queries whether the aspect ratio of the view is locked.
    *
    *  @return  TRUE if the aspect ratio is locked, otherwise FALSE.
    *
    *  @see     lockAspectRatio()
    */
   virtual bool isAspectRatioLocked() const = 0;

   /**
    *  Returns the number of screen pixels that covers one data pixel.
    *
    *  @return  The number of screen pixels covering one data pixel.  The
    *           x-value of the LocationType represents the ratio of screen
    *           pixels to data pixels in the x-dimension, and the y-value of
    *           the LocationType represents the ratio of screen pixels to data
    *           pixels in the y-dimension.
    */
   virtual LocationType getPixelSize() const = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteView.
    */
   virtual ~OrthographicView() {}
};

#endif
