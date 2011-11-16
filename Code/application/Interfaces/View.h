/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VIEW_H
#define VIEW_H

#include "ColorType.h"
#include "ConfigurationSettings.h"
#include "LocationType.h"
#include "SessionItem.h"
#include "Subject.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class AnimationController;
class Classification;
class MouseMode;
class QImage;
class QSize;
class QWidget;
class UndoAction;

/**
 *  A means to display data.
 *
 *  This class is a base class containing common functionality to display data to
 *  the user.  A view is actually a widget has a name and a type, and classification.
 *  Zooming and panning functionality is provided, along with capability to link
 *  other views to this view and to set other display features.
 *
 *  When the user right-clicks in the view, a \ref contextmenus "context menu"
 *  is invoked allowing the user to change the display characteristics of the
 *  view.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setMouseMode(), setName(),
 *    setClassificationText(), setBackgroundColor(), setDataOrigin().
 *  - The user right-clicks in the view to invoke a context menu.
 *  - Everything else documented in Subject.
 *
 *  @see     Window
 */
class View : public SessionItem, public Subject
{
public:
   SETTING(BackgroundColor, View, ColorType, ColorType())
   SETTING(DataOrigin, View, DataOrigin, UPPER_LEFT)
   SETTING(InsetShowCoordinates, View, bool, false)
   SETTING(InsetSize, View, unsigned int, 300)
   SETTING(InsetZoom, View, unsigned int, 5)
   SETTING(InsetZoomMode, View, InsetZoomMode, ABSOLUTE_MODE)
   SETTING(MousePanSensitivity, View, int, 100)
   SETTING(PixelValueMinimumFontSize, View, unsigned int, 5);
   SETTING(PixelValueMaximumFontSize, View, unsigned int, 20);
   SETTING(DisplayCrosshair, View, bool, false)
   SETTING(UseViewResolution, View, bool, false);
   SETTING(AspectRatioLock, View, bool, false);
   SETTING(OutputWidth, View, unsigned int, 0);
   SETTING(OutputHeight, View, unsigned int, 0);
   SETTING(LinkType, View, LinkType, AUTOMATIC_LINK);

   /**
    *  This class accesses portions of a view image.
    *
    *  If you need to grab a "screen shot" of an image which is
    *  larger than the physical display dimensions, this class can
    *  be used to access tiled sub-images.
    */
   class SubImageIterator
   {
   public:
      /**
       *  Destructor.
       *
       *  It is safe to delete a SubImageIterator.
       *  Ownership semantics are described in functions which
       *  return a SubImageIterator. Typically, the caller owns
       *  the SubImageIterator and must delete it when finished.
       *
       *  @see View::getSubImageIterator()
       */
      virtual ~SubImageIterator() {}

      /**
       *  Is there another sub-image?
       *  This should be checked before calling next().
       *
       *  @return True if there is another sub-image, false otherwise.
       */
      virtual bool hasNext() const = 0;

      /**
       *  Get the next sub-image.
       *  This will initialize the image to the size and encoding
       *  specified when the SubImageIterator was created.
       *
       *  @param image
       *         This will hold the resultant image. If this is
       *         an invalid QImage, it will be initialized by
       *         this method. Alternately, a QImage which was
       *         initialized by a previous call to next() may
       *         be passed. This prevents re-initialization of
       *         the QImage which is more efficient.
       *
       *  @return True if the image was successfully returned.
       *          False if an error occurred. If there are no
       *          tiles left (the iterator is at the end), false
       *          will be returned. If false is returned
       *          the state of image is undefined.
       */
      virtual bool next(QImage &image) = 0;

      /**
       *  Access the location of the current sub-image.
       *  This should be called after next(). It will return
       *  the sub-image location of the image returned by next().
       *
       *  @param x
       *         The zero indexed x-access index of the sub-image.
       *
       *  @param y
       *         The zero indexed y-access index of the sub-image.
       */
      virtual void location(int &x, int &y) const = 0;

      /**
       *  Access the total number of sub-images.
       *
       *  @param x
       *         The number of horizontal sub-images.
       *  @param y
       *         The number of vertical sub-images.
       */
      virtual void count(int &x, int &y) const = 0;
   };

   /**
    *  Emitted with any<vector<LocationType> > when the selection box is changed.
    */
   SIGNAL_METHOD(View, SelectionBoxChanged)

   /**
    *  Emitted when the view name changes with boost::any<std::string>
    *  containing the new name.
    */
   SIGNAL_METHOD(View, Renamed)

   /**
    *  Emitted with any<const Classification*> when the classification is changed.
    */
   SIGNAL_METHOD(View, ClassificationChanged)

   /**
    *  Emitted with any<QFont> when the classification font is changed.
    */
   SIGNAL_METHOD(View, ClassificationFontChanged)

   /**
    *  Emitted with any<ColorType> when the classification color is changed.
    */
   SIGNAL_METHOD(View, ClassificationColorChanged)

   /**
    *  Emitted with any<ColorType> when the background color is changed.
    */
   SIGNAL_METHOD(View, BackgroundColorChanged)

   /**
    *  Emitted with any<DataOrigin> when the origin is changed.
    */
   SIGNAL_METHOD(View, OriginChanged)

   /**
    *  Emitted with any<const MouseMode*> when the mouse mode is changed.
    */
   SIGNAL_METHOD(View, MouseModeChanged)

   /**
    *  Emitted with boost::any<bool> when mouse pan is enabled or
    *  disabled by the user.
    */
   SIGNAL_METHOD(View, MousePanEnabled)

   /**
    *  Emitted with any<boost::tuple<double,double,double,double> > when the 
    *           view extents are changed. The values in the tuple are minX, minY,
    *           maxX and maxY respectively.
    */
   SIGNAL_METHOD(View, ExtentsChanged)

   /**
    *  Emitted with boost::any<ContextMenu*> when the user right-clicks to
    *  invoke a context menu.
    *
    *  This signal provides a means by which an object can be notified when a
    *  context menu is invoked by the user clicking inside a view.  To receive
    *  notification for when a context menu is invoked when the user clicks on
    *  any session item, attach to the
    *  DesktopServices::signalAboutToShowContextMenu() signal instead.
    *
    *  This signal is emitted after getContextMenuActions() is called and
    *  after the DesktopServices::signalAboutToShowContextMenu() signal is
    *  emitted, but before the context menu is shown to give attached objects a
    *  chance to add or modify the context menu that will be displayed to the
    *  user.
    *
    *  The ContextMenu pointer value is guaranteed to be non-\c NULL.  The
    *  session items vector in the context menu contains the view.
    *
    *  @see     \ref callingsequence "Context menu calling sequence"
    */
   SIGNAL_METHOD(View, AboutToShowContextMenu)

   /**
    *  Sets the view name.
    *
    *  @param   viewName
    *           The new name for the view.  Cannot be empty.
    *
    *  @notify  This method will notify signalRenamed() with any<std::string>.
    */
   virtual void setName(const std::string& viewName) = 0;

   /**
    *  Returns the view type.
    *
    *  @return  The view type.
    *
    *  @see     ViewType
    */
   virtual ViewType getViewType() const = 0;

   /**
    *  Returns the widget that displays the view.
    *
    *  This method returns the Qt widget that displays the view.  This can be used
    *  to add the view to a custom dialog or widget if it is not already contained
    *  in a ViewWindow.
    *
    *  @return  A pointer to the widget that displays the view.  \c NULL is
    *           returned if the widget cannot be accessed.
    */
   virtual QWidget* getWidget() const = 0;

   /**
    *  Sets the background color of the view.
    *
    *  This method does not call refresh() so that multiple calls to modify view
    *  settings can be made without refreshing the view after each modification.
    *
    *  @param   backgroundColor
    *           The new background color.  Must be a valid color.
    *
    *  @see     ColorType
    *
    *  @notify  This method will notify signalBackgroundColorChanged() with
    *           any<ColorType>.
    */
   virtual void setBackgroundColor(const ColorType& backgroundColor) = 0;

   /**
    *  Returns the background color of the view.
    *
    *  @return  The current background color.
    */
   virtual ColorType getBackgroundColor() const = 0;

   /**
    *  Sets the classification level of the view.
    *
    *  This method does not call refresh() so that multiple calls to modify view
    *  settings can be made without refreshing the view after each modification.
    *
    *  @param   pClassification
    *           A pointer to the classification object from which to set the view's
    *           text markings.  Cannot be \c NULL.
    *
    *  @see     Classification
    *
    *  @notify  This method will notify signalClassificationChanged() with
    *           boost::any<const Classification*>.
    */
   virtual void setClassification(const Classification* pClassification) = 0;

   /**
    *  Returns a pointer to the view's classification object.
    *
    *  The classification object documents how the data in the view is to be
    *  handled and/or restricted.
    *
    *  @return  A pointer to the view's classification object.
    *
    *  @see     Classification
    */
   virtual Classification* getClassification() = 0;

   /**
    *  Returns read-only access to the view's classification object.
    *
    *  The classification object documents how the data in the view is to be
    *  handled and/or restricted.
    *
    *  @return  A const pointer to the view's classification object. The
    *           classification represented by the returned pointer should not
    *           be modified.  To modify the values, call the non-const version
    *           of getClassification() instead.
    *
    *  @see     Classification
    */
   virtual const Classification* getClassification() const = 0;

   /**
    *  Retrieves a text string containing the classification markings.
    *
    *  @return  A string containing the complete classification text.
    */
   virtual std::string getClassificationText() const = 0;

   /**
    *  Enables/disables display of classification markings.
    *
    *  @param   enable
    *           If \c true (the default), classification markings are displayed
    *           on the view.  If \c false, classification markings are not
    *           displayed.
    */
   virtual void enableClassification(bool enable) = 0;

   /**
    *  Sets the data origin location in the view.
    *
    *  This method does not call refresh() so that multiple calls to modify view
    *  settings can be made without refreshing the view after each modification.
    *
    *  @param   dataOrigin
    *           The new origin location of the world coordinate system.
    *
    *  @notify  This method will notify signalOriginChanged() with
    *           any<DataOrigin>.
    */
   virtual void setDataOrigin(const DataOrigin& dataOrigin) = 0;

   /**
    *  Returns the data origin location.
    *
    *  @return  The current origin of the world coordinate system.
    */
   virtual DataOrigin getDataOrigin() const = 0;

   /**
    *  Registers a new mouse mode.
    *
    *  @param   pMouseMode
    *           The mouse mode to register with the view.  Cannot be \c NULL.
    *           The mouse mode name must be unique for all mouse modes
    *           registered with the view.
    *
    *  @return  TRUE if the mouse mode was successfully registered the view,
    *           otherwise FALSE.  FALSE is returned if the mouse mode cannot be
    *           registered, i.e. it has the same name as an existing mouse mode.
    */
   virtual bool addMouseMode(MouseMode* pMouseMode) = 0;

   /**
    *  Sets the current mouse mode.
    *
    *  @param   pMouseMode
    *           The new mouse mode.
    *
    *  @return  TRUE if the mouse mode was successfully set for the view,
    *           otherwise FALSE.  FALSE is returned if the view does not support
    *           the mouse mode.
    *
    *  @notify  This method will notify signalMouseModeChanged() with
    *           any<const MouseMode*>.
    *
    *  @see     MouseMode
    */
   virtual bool setMouseMode(MouseMode* pMouseMode) = 0;

   /**
    *  Sets the current mouse mode.
    *
    *  This method serves as a convenience function to set the mouse mode.  It
    *  sets the mouse mode based on the mode name.
    *
    *  @param   modeName
    *           The name for the mouse mode to set for the view.  Cannot be
    *           empty.
    *
    *  @return  TRUE if the mouse mode was successfully set for the view,
    *           otherwise FALSE.  FALSE is returned if the view does not contain
    *           a mouse mode with the given name.
    *
    *  @notify  This method will notify signalMouseModeChanged() with
    *           any<const MouseMode*>.
    */
   virtual bool setMouseMode(const std::string& modeName) = 0;

   /**
    *  Queries whether a given mouse mode is registered with the view.
    *
    *  @param   pMouseMode
    *           The mouse mode to query.
    *
    *  @return  TRUE if the mouse mode is registered for the view, otherwise
    *           FALSE.  FALSE is returned if the view does not support the
    *           mouse mode.
    *
    *  @see     MouseMode
    */
   virtual bool containsMouseMode(MouseMode* pMouseMode) const = 0;

   /**
    *  Returns the mouse mode with a given name.
    *
    *  @param   modeName
    *           The name for the mouse mode to get.  Cannot be empty.
    *
    *  @return  A pointer to the mouse mode with the given name.  \c NULL is
    *           returned if the view does not contain a mouse mode with the
    *           given name.
    */
   virtual MouseMode* getMouseMode(const std::string& modeName) const = 0;

   /**
    *  Returns the current mouse mode.
    *
    *  @return  A pointer to the current mouse mode.  \c NULL is returned if
    *           the view does not contain any mouse modes, or if no mode is
    *           considered current.
    */
   virtual MouseMode* getCurrentMouseMode() const = 0;

   /**
    *  Retrieves all mouse modes registered with the view.
    *
    *  @param   modes
    *           Populated with pointers to the registered mouse modes.
    */
   virtual void getMouseModes(std::vector<MouseMode*>& modes) const = 0;

   /**
    *  Retrieves the animation controller associated with the view.
    *
    *  @return  A pointer to the associated AnimationController or \c NULL
    *           if there is no associated AnimationController
    *
    *  @see  setAnimationController()
    */
   virtual AnimationController *getAnimationController() const = 0;

   /**
    *  Associates an animation controller with the view.
    *
    *  @param  pPlayer
    *          The AnimationController to associate with the view.
    *
    *  @see  getAnimationController()
    */
   virtual void setAnimationController(AnimationController *pPlayer) = 0;

   /**
    *  Returns the number of registered mouse mode.
    *
    *  @return  The number of mouse modes currently available in the view.
    */
   virtual unsigned int getNumMouseModes() const = 0;

   /**
    *  Retrieves the data extents in world coordinates.
    *
    *  @param   dMinX
    *           The minimum data value in the X dimension.
    *  @param   dMinY
    *           The minimum data value in the Y dimension.
    *  @param   dMaxX
    *           The maximum data value in the X dimension.
    *  @param   dMaxY
    *           The maximum data value in the Y dimension.
    */
   virtual void getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY) const = 0;

   /**
    *  Retrieves the corner coordinates of the visible view area.
    *
    *  @param   lowerLeft
    *           A LocationType that is populated with the lower left coordinate.
    *  @param   upperLeft
    *           A LocationType that is populated with the upper left coordinate.
    *  @param   upperRight
    *           A LocationType that is populated with the upper right coordinate.
    *  @param   lowerRight
    *           A LocationType that is populated with the lower right coordinate.
    *
    *  @see     getVisibleCenter()
    */
   virtual void getVisibleCorners(LocationType& lowerLeft, LocationType& upperLeft, LocationType& upperRight,
      LocationType& lowerRight) const = 0;

   /**
    *  Returns the coordinate at the current visible center of the view.
    *
    *  @return  The current center coordinate.
    */
   virtual LocationType getVisibleCenter() const = 0;

   /**
    *  Zooms the image to the data extents.
    *
    *  This method zooms the view out so that the entire extents of the data are
    *  displayed.
    *
    *  This method does not call refresh() so that multiple calls to modify view
    *  settings can be made without refreshing the view after each modification.
    *
    *  @see     zoomToBox()
    */
   virtual void zoomExtents() = 0;

   /**
    *  Zooms the image to a defined rectangular area.
    *
    *  This method does not call refresh() so that multiple calls to modify view
    *  settings can be made without refreshing the view after each modification.
    *
    *  @param   lowerLeft
    *           The lower left location of the view in world coordinates.
    *  @param   upperRight
    *           The upper right location of the view in world coordinates.
    *
    *  @see     zoomExtents()
    */
   virtual void zoomToBox(const LocationType& lowerLeft, const LocationType& upperRight) = 0;

   /**
    *  Centers the image on a given point.
    *
    *  This method does not call refresh() so that multiple calls to modify view
    *  settings can be made without refreshing the view after each modification.
    *
    *  @param   worldCoord
    *           The point to pan to the center of the view in world coordinates.
    *
    *  @see     panToCenter(), panBy()
    */
   virtual void panTo(const LocationType& worldCoord) = 0;

   /**
    *  Centers the image in the view.
    *
    *  This method does not call refresh() so that multiple calls to modify view
    *  settings can be made without refreshing the view after each modification.
    *
    *  @see     panTo(), panBy()
    */
   virtual void panToCenter() = 0;

   /**
    *  Moves the image by a given distance.
    *
    *  This method does not call refresh() so that multiple calls to modify view
    *  settings can be made without refreshing the view after each modification.
    *
    *  @param   dDeltaX
    *           The X distance from which to pan the view from its current location
    *           in world coordinates.
    *  @param   dDeltaY
    *           The Y distance from which to pan the view from its current location
    *           in world coordinates.
    *
    *  @see     panTo()
    */
   virtual void panBy(double dDeltaX, double dDeltaY) = 0;

   /**
    *  Toggles the display of a dynamic inset window.
    *
    *  This methods toggles the display of a dynamic inset window, which appears
    *  on top of the view.  The window displays a portion of the view at a
    *  different zoom and level than the view.  The default size and zoom level
    *  of the inset window are specified in the Options.
    *
    *  @param   bEnable
    *           Set this value to TRUE to display the inset window or to FALSE
    *           to hide the inset window.
    *
    *  @return  TRUE if the inset window was successfully enabled or disabled, or
    *           FALSE if the inset window is not supported by the view.
    *
    *  @see     setInsetPoint()
    */
   virtual bool enableInset(bool bEnable) = 0;

   /**
    *  Queries whether the dynamic inset window is enabled.
    *
    *  @return  TRUE if the inset window is enabled, whereby it is drawn on top
    *           of the view, or FALSE if the inset window is disabled.
    */
   virtual bool isInsetEnabled() const = 0;

   /**
    *  Sets the center of the dynamic inset window.
    *
    *  @param   worldCoord
    *           The new inset center point in world coordinates.
    */
   virtual void setInsetPoint(const LocationType& worldCoord) = 0;

   /**
    *  Toggles the display of the view crosshair.
    *
    *  This methods toggles the display of a crosshair mark in the center of
    *  the view.
    *
    *  @param   bEnable
    *           Set this value to TRUE to display the crosshair in the view or
    *           to FALSE to hide the crosshair.
    *
    *  @return  TRUE if the crosshair was successfully enabled or disabled, or
    *           FALSE if the crosshair is not supported by the view.
    */
   virtual bool enableCrossHair(bool bEnable) = 0;

   /**
    *  Queries whether the view crosshair is enabled.
    *
    *  @return  TRUE if the crosshair is enabled, whereby it is drawn in the
    *           center of the view, or FALSE if the crosshair is disabled.
    */
   virtual bool isCrossHairEnabled() const = 0;

   /**
    *  Redraws the current contents of the view.
    */
   virtual void refresh() = 0;

   /**
    *  Retrieves an image of the current view state.
    *
    *  This method retrieves the data for the current image as displayed in the view.
    *  This is similar to taking a screen snapshot of the view.
    *
    *  @param   image
    *           A Qt image reference that is populated with the current view
    *           image.  If a null image is passed in, the size of the image
    *           will be equal to the size of the view in screen pixels.
    *           Otherwise, the view will be scaled to the image size when
    *           generating the image data. If a size is specified, the QImage
    *           should use QImage::Format_ARGB32. If the format is different,
    *           the QImage will be converted to QImage::Format_ARGB32.
    *
    *  @return  Returns \c true if a valid image was successfully retrieved,
    *           otherwise returns \c false.
    */
   virtual bool getCurrentImage(QImage &image) = 0;

   /**
    *  Retrieves an iterator over sub-images of the current view state.
    *
    *  This method is similar to getCurrentImage(QImage&) except it obtains
    *  an iterator which accesses sub-sections of the view. This is used for "pan and scan"
    *  image generation. The caller takes ownership of the SubImageIterator and MUST delete
    *  the SubImageIterator when it is no longer needed. When a SubImageIterator exists for a
    *  view, another can not be created and no other view operations should be performed.
    *
    *  @param totalSize
    *         This is the size of the entire image. If all the generated sub-images are
    *         stitched together, they will make an image this size.
    *
    *  @param subImageSize
    *         This is the size of each sub-image.
    *
    *  @return A SubImageIterator over the view. The iterator will immediatly return false
    *          to SubImageIterator::hasNext() calls if an error occurs. If a SubImageIterator
    *          already exists for this View, that iterator will be returned.
    */
   virtual SubImageIterator *getSubImageIterator(const QSize &totalSize, const QSize &subImageSize) = 0;

   /**
    *  Links a view with this view.
    *
    *  This method establishes a link between this view and another view.  Linked views
    *  are linked depending on the link type specified.
    *
    *  @param   pView
    *           The view to link with this view.  Cannot be \c NULL.
    *  @param   type
    *           The type of link to create.  Specifying NO_LINK
    *           is equivilent to calling unlinkView().
    *  @return  TRUE if the view was successfully linked with this view, otherwise
    *           FALSE.
    */
   virtual bool linkView(View* pView, LinkType type) = 0;

   /**
    *  Retrieves views linked with this view.
    *
    *  @param   linkedViews
    *           A reference to a vector that is populated with pointers to views linked
    *           with this view and the associated link type.  If no views are linked,
    *           the vector is emptied.
    */
   virtual void getLinkedViews(std::vector<std::pair<View*, LinkType> > &linkedViews) const = 0;

   /**
    *  Queries what kind of link a view has with this view.
    *
    *  @param   pView
    *           The view to query for linkage with this view.
    *
    *  @return  The type of link that exists.  NO_LINK indicated
    *           that there is no linking relationship.
    */
   virtual LinkType getViewLinkType(View* pView) const = 0;

   /**
    *  Breaks the link between this view and a given view.
    *
    *  @param   pView
    *           The view to unlink with this view.  Cannot be \c NULL.
    *
    *  @return  TRUE if the view was successfully unlinked with this view, otherwise
    *           FALSE.
    */
   virtual bool unlinkView(View* pView) = 0;

   /**
    *  Creates a new view with the same settings and properties as this view.
    *
    *  This is designed to copy an existing view into a product view object.
    *  This method will not create a new window.
    *
    *  @return  A pointer to the new view.  \c NULL is returned if an error
    *           occurs.
    */
   virtual View* copy() const = 0;

   /**
    *  Copies view settings and properties to an existing view.
    *
    *  This performs a shallow copy into an existing view. The common use case
    *  is to create a new window/view and copy the settings of an existing view.
    *  This creates a duplicate but independent view into the same data.
    *
    *  @param pView
    *         The target view.
    *  @return True if successful, false if there was an error.
    */
   virtual bool copy(View *pView) const = 0;

   /**
    *  Removes an existing mouse mode.
    *
    *  This method removes the given mouse mode from the view regardless of whether
    *  the mode is enabled or disabled.  The mouse mode is not deleted.
    *
    *  @param   pMouseMode
    *           The mouse mode to remove from the view.
    *
    *  @return  True if the mouse mode was successfully removed from the view.  False
    *           is returned if the mouse mode is not contained in this view.
    *
    *  @see     isMouseModeEnabled()
    */
   virtual bool removeMouseMode(const MouseMode* pMouseMode) = 0;

   /**
    *  Enables or disables a mouse mode.
    *
    *  This method enables or disabled the given mouse mode from the view.  A disabled
    *  mouse mode cannot be made current with the setMouseMode() method.
    *
    *  @param   pMouseMode
    *           The mouse mode to enable or disable.  This method does nothing if the
    *           view does not contain the given mouse mode.
    *  @param   bEnable
    *           Set this parameter to true to enable the mouse mode or to false to
    *           disable the mouse mode.
    *
    *  @see     setMouseMode(), isMouseModeEnabled()
    */
   virtual void enableMouseMode(const MouseMode* pMouseMode, bool bEnable) = 0;

   /**
    *  Queries whether a mouse mode is currently enabled or disabled.
    *
    *  This method removes the given mouse mode from the view regardless of whether
    *  the mode is enabled or disabled.  The mouse mode is not deleted.
    *
    *  @param   pMouseMode
    *           The mouse mode to query whether it is enabled or disabled.
    *
    *  @return  True if the mouse mode is currently enabled or false if the mode is
    *           disabled.  False is also returned if the mouse mode is not contained
    *           in this view.
    *
    *  @see     enableMouseMode()
    */
   virtual bool isMouseModeEnabled(const MouseMode* pMouseMode) const = 0;

   /**
    *  Retrieves a screen coordinate for a given world coordinate.
    *
    *  This method converts a world coordinate into a screen coordinate.  The world
    *  coordinate is the linear coordinate system used internally to draw the data,
    *  which is typically the same coodinate system as the data itself.  The screen
    *  coordinate is the location relative to the view widget with the origin being
    *  in the lower left corner.
    *
    *  @param   dWorldX
    *           The x-position of the world coordinate.  The value does not need to
    *           be within the view extents.
    *  @param   dWorldY
    *           The y-position of the world coordinate.  The value does not need to
    *           be within the view extents.
    *  @param   dScreenX
    *           Populated with the screen x-coordinate of the given world coordinate.
    *           A negative value indicates that the world coordinate is not currently
    *           displayed in the view widget.
    *  @param   dScreenY
    *           Populated with the screen y-coordinate of the given world coordinate.
    *           A negative value indicates that the world coordinate is not currently
    *           displayed in the view widget.
    *  @param   pVisible
    *           If this value is non-\c NULL, the pointer is populated with a value
    *           indicating whether the given world coordinate is visible with the
    *           current view widget.
    *
    *  @see     translateScreenToWorld(), getPixelSize()
    */
   virtual void translateWorldToScreen(double dWorldX, double dWorldY, double& dScreenX, double& dScreenY,
      bool* pVisible = NULL) const = 0;

   /**
    *  Retrieves a world coordinate for a given screen coordinate.
    *
    *  This method converts an OpenGL screen coordinate into a world coordinate.
    *  The OpenGL screen coordinate is the location relative to the view widget 
    *  with the origin being in the lower left corner. To determine the correct 
    *  dScreenY value, the mouse event y-coordinate must be subtracted from the 
    *  view height,e.g., dScreenY = pViewWidget.height() - pMouseEvent.pos().y().
    *
    *  The world coordinate is the linear coordinate system used 
    *  internally to draw the data, which is typically the same coodinate system 
    *  as the data itself.
    *
    *  @param   dScreenX
    *           The x-position of the screen coordinate.  The value does not need to be
    *           within the view extents.
    *  @param   dScreenY
    *           The y-position of the screen coordinate.  The value does not need to be
    *           within the view extents.
    *  @param   dWorldX
    *           Populated with the world x-coordinate of the given screen coordinate.
    *  @param   dWorldY
    *           Populated with the world y-coordinate of the given screen coordinate.
    *
    *  @see     translateWorldToScreen(), getPixelSize()
    */
   virtual void translateScreenToWorld(double dScreenX, double dScreenY, double& dWorldX, double& dWorldY) const = 0;

   /**
    *  Returns the number of screen pixels that cover a given range of world pixels.
    *
    *  @warning This method assumes that the scene pixel aspect ratio is one,
    *           and therefore should not be called on an orthographic view.
    *           For orthographic views, call OrthographicView::getPixelSize()
    *           instead.  When calling this method on a perspective view, if
    *           PerspectiveView::getPixelAspect() returns a value other than
    *           one, the return value should be multiplied by the pixel aspect
    *           to get the correct pixel size for the x-dimension, as shown in
    *           the code sample below:
    *  @code
    *  double pixelSize = pPerspectiveView->getPixelSize(llCorner, urCorner);
    *  double pixelSizeX = pixelSize * pPerspectiveView->getPixelAspect();
    *  double pixelSizeY = pixelSize;
    *  @endcode
    *
    *  @param   lowerLeft
    *           The world coordinate closest to the origin.
    *  @param   upperRight
    *           The world coordinate furthest from the origin.
    *
    *  @return  The number of screen pixels covering the given range of world coordinates.
    *
    *  @see     translateWorldToScreen(), translateScreenToWorld()
    */
   virtual double getPixelSize(const LocationType& lowerLeft, const LocationType& upperRight) const = 0;

   /**
    *  Queries whether undo actions are able to be added to the view's undo
    *  stack.
    *
    *  @return  Returns \c true if undo actions cannot be added to the undo
    *           stack, or \c false if undo actions are added to the stack.
    *
    *  @see     UndoLock, addUndoAction()
    */
   virtual bool isUndoBlocked() const = 0;

   /**
    *  Queries whether undo actions are currently added to an undo group.
    *
    *  @return  Returns \c true if undo actions are added to an undo group, or
    *           \c false if undo actions are added directly to the stack.
    *
    *  @see     UndoGroup, addUndoAction()
    */
   virtual bool inUndoGroup() const = 0;

   /**
    *  Adds an action to the undo stack.
    *
    *  This method add the given action to the undo stack.  If startUndoGroup()
    *  has been called without a corresponding call to endUndoGroup() the
    *  action is added to the group.
    *
    *  This method only needs to be called from a plug-in if adding custom undo
    *  actions defined in the plug-in.
    *
    *  @warning If a plug-in defines its own UndoAction subclass, the plug-in
    *           must remain loaded until the view is deleted to ensure the
    *           action can be properly deleted.
    *
    *  @param   pAction
    *           The action to add to the undo stack.  The view assumes
    *           ownership of the action and will delete it when necessary.  If
    *           isUndoBlocked() returns \c true, this method deletes the given
    *           action immediately.
    */
   virtual void addUndoAction(UndoAction* pAction) = 0;

   /**
    *  Clears the undo stack.
    *
    *  This method removes all undo actions from the undo stack regardless of
    *  whether adding undo actions is blocked.  If inUndoGroup() returns
    *  \c true, any actions existing in the group are destroyed.
    */
   virtual void clearUndo() = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteView.
    */
   virtual ~View() {}

   friend class UndoLock;
   friend class UndoGroup;

   /**
    *  Prevents undo actions from being added to the view's undo stack.
    *
    *  @warning This method exists for implementation purposes and cannot be
    *           used directly.  Instead, use the UndoLock object to prevent
    *           undo actions from being added to the view.
    *
    *  @see     UndoLock, isUndoBlocked()
    */
   virtual void blockUndo() = 0;

   /**
    *  Allows undo actions to be added to the view's undo stack.
    *
    *  @warning This method exists for implementation purposes and cannot be
    *           used directly.  Instead, use the UndoLock object to prevent
    *           undo actions from being added to the view.
    *
    *  @see     UndoLock, isUndoBlocked()
    */
   virtual void unblockUndo() = 0;

   /**
    *  Instructs the undo stack to add all future undo actions to a group.
    *
    *  @warning This method exists for implementation purposes and cannot be
    *           used directly.  Instead, use the UndoGroup object to group
    *           future undo actions into a single undoable operation.
    *
    *  @param   text
    *           The name for the undo action group that will appear to the
    *           user.
    *
    *  @see     UndoGroup, inUndoGroup(), addUndoAction()
    */
   virtual void startUndoGroup(const std::string& text) = 0;

   /**
    *  Returns the undo stack to normal operations when adding undo actions.
    *
    *  @warning This method exists for implementation purposes and cannot be
    *           used directly.  Instead, use the UndoGroup object to group
    *           future undo actions into a single undoable operation.
    *
    *  @see     UndoGroup, inUndoGroup(), addUndoAction()
    */
   virtual void endUndoGroup() = 0;
};

#endif
