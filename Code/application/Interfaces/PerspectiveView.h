/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PERSPECTIVEVIEW_H
#define PERSPECTIVEVIEW_H

#include "View.h"

/**
 *  A three-dimensional perspective view.
 *
 *  The perspective view uses a perspective projection to draw its contents in three
 *  dimensions.  The data can be zoomed, panned, and rotated in several different
 *  fashions.  The reset() and resetOrientation() functions return the view to its
 *  original display state.  The aspect ratio is preserved in all dimensions regardless
 *  of the display configuration.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: zoomTo(), rotateTo(), rotateBy(),
 *    zoomAboutPoint(), zoomBy(), zoomToPoint(), zoomToCenter(), flipBy(), flipTo(),
 *    flipHorizontal(), flipVertical(), resetZoom(), resetOrientation(), and reset().
 *  - The user causes the view to pan, rotate, flip, or zoom.
 *  - Everything else documented in View.
 *
 *  @see     View, OrthographicView
 */
class PerspectiveView : public View
{
public:
   SETTING(ZoomPercentage, PerspectiveView, unsigned int, 0)

   /**
    *  Emitted with any<double> when the zoom level changes.
    */
   SIGNAL_METHOD(PerspectiveView, ZoomChanged)
   /**
    *  Emitted  with any<double> when the scene rotation changes.
    */
   SIGNAL_METHOD(PerspectiveView, RotationChanged)
   /**
    *  Emitted  with any<double> when the scene pitch angle changes.
    */
   SIGNAL_METHOD(PerspectiveView, PitchChanged)
   /**
    *  Emitted when the viewable area changes.
    */
   SIGNAL_METHOD(PerspectiveView, DisplayAreaChanged)

   /**
    *  Adjusts the current zoom level by a given percentage.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   dPercent
    *           The percentage by which the zoom level is adjusted.
    *
    *  @see     zoomTo()
    *
    *  @notify  This method will notify signalZoomChanged with any<double>.
    */
   virtual void zoomBy(double dPercent) = 0;

   /**
    *  Sets the current zoom level to a given percentage.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   dPercent
    *           The percentage to which the zoom level is set.
    *
    *  @see     zoomBy()
    *
    *  @notify  This method will notify signalZoomChanged with any<double>.
    */
   virtual void zoomTo(double dPercent) = 0;

   /**
    *  Sets the current zoom level to a given percentage centered on a given point.
    *
    *  This method sets the view zoom level based on a given point.  The
    *  view is panned to center the point, the zoom level is set, and the
    *  view is panned back to the given point.  To not pan back to the
    *  given point, call zoomToPoint() instead.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   worldCoord
    *           The pixel coordinate about which zoom action is centered.
    *  @param   dPercent
    *           The percentage to which the zoom level is set.
    *
    *  @see     zoomToPoint()
    *
    *  @notify  This method will notify signalZoomChanged with any<double>.
    */
   virtual void zoomAboutPoint(const LocationType& worldCoord, double dPercent) = 0;

   /**
    *  Sets the current zoom level and pans to a given point.
    *
    *  This method sets the view zoom level based on a given point.  The
    *  view is panned to center the point and the zoom level is set.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   worldCoord
    *           The pixel coordinate for the new view center.
    *  @param   dPercent
    *           The percentage to which the zoom level is set.
    *
    *  @see     zoomAboutPoint()
    *
    *  @notify  This method will notify signalZoomChanged with any<double>.
    */
   virtual void zoomToPoint(const LocationType& worldCoord, double dPercent) = 0;

   /**
    *  Sets the current zoom level and pans to the center of the image.
    *
    *  This method first centers the image in the view and then zooms to the given
    *  percentage.  This is equivalent to calling panToCenter() and then zoomTo().
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   dPercent
    *           The percentage to which the zoom level is set.
    *
    *  @see     zoomToPoint()
    *
    *  @notify  This method will notify signalZoomChanged with any<double>.
    */
   virtual void zoomToCenter(double dPercent) = 0;

   /**
    *  Zooms the view to a custom area.
    *
    *  This method adjusts the zoom level of the view to the bounding box of the
    *  given world coordinate locations.  This method differs from zoomToBox()
    *  in that rotation of the view is taken into account when determining the
    *  bounding box of the given coordinates.
    *
    *  This method does not call refresh() so that multiple calls to modify view
    *  settings can be made without refreshing the view after each modification.
    *
    *  @param   worldCoords
    *           A vector of world coordinates defining the area to display in
    *           the view.
    *
    *  @notify  This method notifies signalZoomChanged() and
    *           signalDisplayAreaChanged() when the display area changes.
    *
    *  @see     zoomToBox()<br>zoomExtents()
    */
   virtual void zoomToArea(const std::vector<LocationType>& worldCoords) = 0;

   /**
    *  Returns the current zoom level as a percentage.
    *
    *  @return  The current zoom percentage.  The percentage is the relationship
    *           between screen pixel and scene pixel.  A 100% zoom indicates one scene
    *           pixel is displayed in one screen pixel.
    */
   virtual double getZoomPercentage() const = 0;

   /**
    *  Rotates the image from its current position by a given angle.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   dDegrees
    *           The angle in degrees by which the view should be rotated.  The value
    *           must be between -360.0 and 360.0 degrees.  A positive value indicates a
    *           counterclockwise rotation.
    *
    *  @see     rotateTo()
    *
    *  @notify  This method will notify signalRotationChanged with any<double>.
    */
   virtual void rotateBy(double dDegrees) = 0;

   /**
    *  Rotates the image to a given angle.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   dDegrees
    *           The angle in degrees to which the view should be rotated.  The value
    *           must be between 0.0 and 360.0 degrees.
    *
    *  @see     rotateBy()
    *
    *  @notify  This method will notify signalRotationChanged with any<double>.
    */
   virtual void rotateTo(double dDegrees) = 0;

   /**
    *  Returns the current rotation angle.
    *
    *  @return  The angle in degrees by which the view is rotated, ranging from 0.0 to
    *           360.0 degrees.
    *
    *  @see     rotateTo()
    */
   virtual double getRotation() const = 0;

   /**
    *  Flips the image from its current position by a given angle.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   dDegrees
    *           The angle in degrees by which the view should be flipped.  The value
    *           must be between -360.0 and 360.0 degrees.
    *
    *  @see     flipTo(), flipHorizontal(), flipVertical()
    *
    *  @notify  This method will notify signalPitchChanged with any<double>.
    */
   virtual void flipBy(double dDegrees) = 0;

   /**
    *  Flips the image to a given angle.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   dDegrees
    *           The angle in degrees to which the view should be flipped.  The value
    *           must be between 0.0 and 360.0 degrees.
    *
    *  @notify  This method will notify signalPitchChanged with any<double>.
    *
    *  @see     flipBy(), flipHorizontal(), flipVertical()
    */
   virtual void flipTo(double dDegrees) = 0;

   /**
    *  Flips the image across the horizontal axis.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @see     flipVertical(), flipTo(), flipBy()
    *
    *  @notify  This method will notify signalPitchChanged with any<double> and 
    *              signalRotationChanged with any<double>.
    */
   virtual void flipHorizontal() = 0;

   /**
    *  Flips the image across the vertical axis.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @see     flipHorizontal(), flipTo(), flipBy()
    *
    *  @notify  This method will notify signalPitchChanged with any<double>.
    */
   virtual void flipVertical() = 0;

   /**
    *  Returns the current pitch angle.
    *
    *  @return  The angle in degrees by which the view is pitched, ranging from 0.0 to
    *           360.0 degrees.
    *
    *  @see     flipTo()
    */
   virtual double getPitch() const = 0;
  
   /**
    *  Resets the zoom level to the user-defined default value.
    *
    *  This method adjusts the zoom level based on the getSettingZoomPercentage()
    *  value.  If the default zoom percentage is zero, zoomExtents() is called.
    *  Otherwise, zoomTo() is called and the data origin is aligned to the corner of
    *  the view.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @see     reset()
    *
    *  @notify  This method will notify signalZoomChanged with any<double>.
    */
   virtual void resetZoom() = 0;

   /**
    *  Resets the image rotatation and flip angles.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @see     reset()
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void resetOrientation() = 0;

   /**
    *  Resets the image zoom and rotation.
    *
    *  This method zooms the image to the data extents and resets the rotation
    *  and flip angles to 0.0.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @see     zoomExtents(), resetOrientation()
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void reset() = 0;

   /**
    *  Toggles whether to show pixel coordinates or pixel values
    *  when sufficiently zoomed.
    */
   virtual void toggleShowCoordinates() = 0;

   /**
    * Gets the pixel aspect ratio.
    *
    * Values greater than 1 will cause a stretch in the x direction, 
    * values less than 1 will cause a stretch in the y direction.
    *
    * @return The ratio of x as compared to y.
    */
   virtual double getPixelAspect() const = 0;

   /**
    * Sets the pixel aspect ratio.
    *
    * Values greater than 1 will cause a stretch in the x direction, 
    * values less than 1 will cause a stretch in the y direction.
    *
    * @param aspect
    *        The ratio of x as compared to y.
    */
   virtual void setPixelAspect(double aspect) = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteView.
    */
   virtual ~PerspectiveView() {}
};

#endif
