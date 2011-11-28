/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DESKTOPSERVICES_H
#define DESKTOPSERVICES_H

#include "Service.h"
#include "Subject.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class Axis;
class DataElement;
class DockWindow;
class FileDescriptor;
class MenuBar;
class MouseMode;
class PlotView;
class PlotWidget;
class PlugIn;
class PlugInCallback;
class ProductWindow;
class Progress;
class QAction;
class QCursor;
class QImage;
class QWidget;
class SessionItem;
class Signature;
class SpatialDataView;
class View;
class Window;
class WorkspaceWindow;

/**
 *  \ingroup ServiceModule
 *  Interface to manage windows and to provide access to widgets in the main
 *  application window.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - A window is added to the main application window.
 *  - A workspace window is activated within the main application window.
 *  - A window is removed from the main application window.
 *  - The main application window is closed.
 *  - Everything else documented in Subject.
 */
class DesktopServices : public Subject
{
public:
   /**
    *  Emitted with any<Window*> when a window is added to the main application
    *  window.
    */
   SIGNAL_METHOD(DesktopServices, WindowAdded)

   /**
    *  Emitted with any<Window*> when a window is removed from the main
    *  application window.
    */
   SIGNAL_METHOD(DesktopServices, WindowRemoved)

   /**
    *  Emitted with any<WorkspaceWindow*> when a workspace window is activated
    *  in the main application window.
    */
   SIGNAL_METHOD(DesktopServices, WindowActivated)

   /**
    *  Emitted when the properties dialog is about to be displayed with
    *  boost::any<std::pair<SessionItem*, std::vector<std::string>*> >
    *  containing the session item whose properties will be displayed and a
    *  pointer to a vector to which additional properties pages can be
    *  displayed.
    */
   SIGNAL_METHOD(DesktopServices, AboutToShowPropertiesDialog)

   /**
    *  Emitted with boost::any<ContextMenu*> when the user right-clicks to
    *  invoke a context menu.
    *
    *  This signal provides a means by which an object can be notified when a
    *  context menu is invoked when the user clicks on any session item.  To
    *  receive notification when the user clicks inside a specific session
    *  item, attach to the AboutToShowContextMenu signal of that particular
    *  session item instead.
    *
    *  This signal is emitted after SessionItem::getContextMenuActions() is
    *  called and before the session item's own signal is emitted and before
    *  the context menu is shown to give attached objects a chance to add or
    *  modify the context menu that will be displayed to the user.
    *
    *  The ContextMenu pointer value is guaranteed to be non-\c NULL.  The
    *  session items vector in the context menu contains the plot widget and
    *  the plot view.
    *
    *  @see     \ref callingsequence "Context menu calling sequence"
    */
   SIGNAL_METHOD(DesktopServices, AboutToShowContextMenu)

   /**
    *  Emitted when the main application window is closed.
    *
    *  The main application window is still fully created when this signal is
    *  emitted.
    */
   SIGNAL_METHOD(DesktopServices, ApplicationWindowClosed)

   /**
    *  Returns a pointer to the Qt widget for the main application window.
    *
    *  The Qt widget for the main application window can be used as the
    *  parent widget for dialogs and other GUIs in plug-ins.
    *
    *  @return  A pointer to the Qt widget for the main application window.  The
    *           widget should not be modified, but is provided for use as a parent
    *           widget in plug-in dialogs or other GUI components.
    */
   virtual QWidget* getMainWidget() const = 0;

   /**
    *  Returns the application menu bar located at the top of the main
    *  application window.
    *
    *  @return  A pointer to the appliation menu bar.
    */
   virtual MenuBar* getMainMenuBar() const = 0;

   /**
    *  Creates a window of a given type.
    *
    *  @param   windowName
    *           Name to be assigned to the new window.  Depending on the window
    *           type, the name may appear in the window's title bar.
    *  @param   windowType
    *           Type of the window to be created.
    *
    *  @return  A pointer to the window.  NULL is returned if the window already
    *           exists or an error occurred.
    *
    *  @see     WindowType, getWindow()
    */
   virtual Window* createWindow(const std::string& windowName, WindowType windowType) = 0;

   /**
    *  Returns a pointer to the window with a given name and type.
    *
    *  @param   windowName
    *           The window name.
    *  @param   windowType
    *           The window type.
    *
    *  @return  A pointer to the window or NULL if no window exists with the
    *           given name and type.
    *
    *  @see     getCurrentWorkspaceWindow()
    */
   virtual Window* getWindow(const std::string& windowName, WindowType windowType) const = 0;

   /**
   *  Returns a pointer to the window with a given unique identifier.
   *
   *  @param   uniqueId
   *           The unique identifier.
   *
   *  @return  A pointer to the window or \c NULL if no window exists with the
   *           given unique identifier.
   */
   virtual Window* getWindow(const std::string& uniqueId) const = 0;

   /**
    *  Retrieves all available windows of a given type.
    *
    *  @param   windowType
    *           The window type.
    *  @param   windows
    *           A reference to a vector that will be filled with pointers
    *           to the available windows of the given window type.
    */
   virtual void getWindows(WindowType windowType, std::vector<Window*>& windows) const = 0;

   /**
    *  Retrieves all available windows of a given type and its subtypes.
    *
    *  This method retrieves all available windows of a given type and all
    *  of its subtypes.  For example, passing in 'WorkspaceWindow' as the
    *  window type retrieves all available workspace windows, analysis
    *  windows, and product windows.
    *
    *  @param   windowType
    *           A string containing the base class name of the window type
    *           for which to retrieve the windows.  Cannot be empty.
    *  @param   windows
    *           A reference to a vector that will be filled with pointers
    *           to the available windows of the given window type and
    *           subtypes.
    */
   virtual void getWindows(const std::string& windowType, std::vector<Window*>& windows) const = 0;

   /**
    *  Retrieves all available windows of all types.
    *
    *  @param   windows
    *           A reference to a vector that will be filled with pointers
    *           to all available windows of all window types.
    */
   virtual void getWindows(std::vector<Window*>& windows) const = 0;

   /**
    *  Returns the number of available windows of a given type.
    *
    *  @param   windowType
    *           The window type for which to get the number of windows.
    *
    *  @return  The number of available windows of the given type.
    */
   virtual unsigned int getNumWindows(WindowType windowType) const = 0;

   /**
    *  Returns the number of available windows of a given type and its
    *  subtypes.
    *
    *  This method retrieves the number of available windows of a given
    *  type and all of its subtypes.  For example, passing in
    *  'WorkspaceWindow' as the window type retrieves the number of
    *  available workspace windows, analysis windows, and product windows.
    *
    *  @param   windowType
    *           The window type for which to get the number of windows.
    *
    *  @return  The number of available windows of the given type.
    */
   virtual unsigned int getNumWindows(const std::string& windowType) const = 0;

   /**
    *  Returns the total number of available windows of all types.
    *
    *  @return  The number of available windows of all types.
    */
   virtual unsigned int getNumWindows() const = 0;

   /**
    *  Deletes the given window.
    *
    *  This method deletes a given window; any view or widget contained in
    *  the window is also deleted.  
    *
    *  WARNING: Windows or Toolbars provided by the application cannot
    *  be deleted using this method, they will be cleaned up during
    *  application close.
    *
    *  @param   pWindow
    *           The window to delete.
    *
    *  @return  TRUE if the window was sucessfully deleted, otherwise FALSE.
    */
   virtual bool deleteWindow(Window* pWindow) = 0;

   /**
    *  Deletes all windows.
    *
    *  @warning Windows or toolbars provided by the application cannot be
    *           deleted using this method.  They will be cleaned up when the
    *           application is closed.
    */
   virtual void deleteAllWindows() = 0;

   /**
    *  Activates a workspace window.
    *
    *  @param   pWindow
    *           The workspace window to activate.  Cannot be NULL.
    *
    *  @return  TRUE if the window was successfully activated, otherwise
    *           FALSE.
    */
   virtual bool setCurrentWorkspaceWindow(WorkspaceWindow* pWindow) = 0;

   /**
    *  Gets a pointer to the currently active workspace window.
    *
    *  @return  A pointer to the active workspace window or NULL if no
    *           workspace windows exist.
    *
    *  @see     getCurrentWorkspaceWindowName()
    */
   virtual WorkspaceWindow* getCurrentWorkspaceWindow() const = 0;

   /**
    *  Retrieves the name of the active workspace window.
    *
    *  @param   windowName
    *           A string that is populated with the name of the active
    *           workspace window.  The string is emptied if no workspace
    *           window exists.
    *
    *  @return  TRUE if the current workspace window name was set
    *           successfully or FALSE if no workspace window exists.
    *
    *  @see     getCurrentWorkspaceWindow()
    */
   virtual bool getCurrentWorkspaceWindowName(std::string& windowName) const = 0;

   /**
    *  Returns the active view in the active workspace window.
    *
    *  This is a convenience method that calls getCurrentWorkspaceWindow() and
    *  then WorkspaceWindow::getActiveView().
    *
    *  @return  A pointer to the active View in the active workspace window.
    *           See the WorkspaceWindow::getActiveView() return value
    *           documentation for details on the returned View pointer based on
    *           the window type.
    *
    *  @see     WorkspaceWindow::getActiveView()
    */
   virtual View* getCurrentWorkspaceWindowView() const = 0;

   /**
    *  Arranges all workspace windows in a cascading fashion.
    *
    *  This method arranges all workspace windows in a cascading fashion
    *  within the main application workspace area.
    *
    *  @see     tileWorkspaceWindows()
    */
   virtual void cascadeWorkspaceWindows() = 0;

   /**
    *  Tiles all workspace windows within the main application workspace.
    *
    *  This method resizes and arranges all workspace windows to fill the
    *  entire main application workspace area.  Windows are arranged in a
    *  two row grid, horizontally in one row or vertically in one column based
    *  on the TilingType parameter.
    *
    *  @param   eType
    *           Orientation of the tiled windows
    *
    *  @see     cascadeWorkspaceWindows()
    */
   virtual void tileWorkspaceWindows(TilingType eType = TILE_GRID) = 0;

   /**
   *  Tiles given workspace windows within the main application workspace.
   *
   *  This method resizes and arranges the given workspace windows to fill
   *  the entire main application workspace area.  Windows are arranged in a 
   *  two row grid, horizontally in one row or vertically in one column based
   *  on the TilingType parameter.  The windows are displayed in the order they 
   *  are added to the vector. Workspace windows not in the given vector are 
   *  sent behind the tiled windows.
   *
   *  @param   windows
   *           The windows to tile in the main application workspace area.
   *  @param   maxFirst
   *           For an odd number of windows to tile, if this value is TRUE,
   *           the first window in the vector will have its height extend
   *           the full height of the workspace area.  If the value is
   *           FALSE, the last window in the vector will have its height
   *           extend the full height of the workspace area.  This parameter
   *           is ignored if the vector contains an even number of windows.
   *  @param   eType
   *           Orientation of the tiled windows.
   *
   *  @return  TRUE if the windows were successfully tiled, otherwise
   *           FALSE.
   *
   *  @see     TilingType
   */
   virtual bool tileWorkspaceWindows(const std::vector<WorkspaceWindow*>& windows, 
      bool maxFirst = true, TilingType eType = TILE_GRID) = 0;

   /**
    *  Creates a view that is not contained in a window.
    *
    *  This method creates a view that is not associated with a window.  The
    *  view is not managed in any way, so it should be deleted when necessary
    *  by calling the deleteView() method.
    *
    *  @param   viewName
    *           The view name.  Cannot be empty.
    *  @param   viewType
    *           The view type.  If ViewType::PLOT_VIEW is passed in, a plot of
    *           type PlotType::CARTESIAN_PLOT is created.
    *  @param   pParent
    *           The Qt widget to use as the parent for the view.
    *
    *  @return  A pointer to the newly created View.
    *
    *  @see     createPlot()
    */
   virtual View* createView(const std::string& viewName, ViewType viewType, QWidget* pParent = 0) = 0;

   /**
    *  Creates a plot view widget that is not contained in a window.
    *
    *  This method creates a plot that is not associated with a window.  The
    *  plot is not owned by another object, so it should be deleted when
    *  necessary by calling the deleteView() method.
    *
    *  @param   plotName
    *           The plot name.  Cannot be empty.
    *  @param   plotType
    *           The plot type.
    *  @param   pParent
    *           The Qt widget to use as the parent for the plot view widget.
    *
    *  @return  A pointer to the newly created plot view widget.
    *
    *  @see     createView()
    */
   virtual PlotView* createPlot(const std::string& plotName, PlotType plotType, QWidget* pParent = 0) = 0;

   /**
    *  Destroys a view.
    *
    *  This method is intended to destroy View objects created with the
    *  createView() method.  To destroy views displayed in a window, call
    *  deleteWindow() instead.
    *
    *  @param   pView
    *           The view to delete.  Cannot be NULL.  If the given view is
    *           displayed in a window, the window is also destroyed.
    */
   virtual void deleteView(View* pView) = 0;

   /**
    *  Queries a view to see if it is a kind of another view.
    *
    *  This method check type compatibility between two views.  This functionality
    *  is different than TypeAwareObject::isKindOf() in that only View-derived
    *  classes are checked.
    *
    *  @param   className
    *           The name of the View-derived class to query.
    *  @param   viewName
    *           The View-derived class name to check type compatibility.
    *
    *  @return  Returns true if the given view class name is a kind of the given
    *           class name; otherwise returns false.
    */
   virtual bool isKindOfView(const std::string& className, const std::string& viewName) = 0;

   /**
    *  Returns a list of inherited view class names for a given class name.
    *
    *  This method populates a vector with the class names of all inherited View
    *  class types.  The given class name is used as the initial class for
    *  populating the vector.  For example, passing in "SpatialDataView" as a
    *  class name populates a vector with the "SpatialDataView", "PerspectiveView",
    *  and "View" strings.
    *
    *  @param   className
    *           The view class name for which to get all view element types.
    *  @param   classList
    *           This vector is populated with the class names of all inherited view
    *           classes and the given class name.
    */
   virtual void getViewTypes(const std::string& className, std::vector<std::string>& classList) = 0;

   /**
    *  Queries a layer to see if it is a kind of another layer.
    *
    *  This method checks type compatibility between two layers.  This functionality
    *  is different than TypeAwareObject::isKindOf() in that only Layer-derived
    *  classes are checked.
    *
    *  @param   className
    *           The name of the Layer-derived class to query.
    *  @param   layerName
    *           The Layer-derived class name to check type compatibility.
    *
    *  @return  Returns true if the given layer class name is a kind of the given
    *           class name; otherwise returns false.
    */
   virtual bool isKindOfLayer(const std::string& className, const std::string& layerName) = 0;

   /**
    *  Returns a list of inherited layer class names for a given class name.
    *
    *  This method populates a vector with the class names of all inherited Layer
    *  class types.  The given class name is used as the initial class for
    *  populating the vector.  For example, passing in "RasterLayer" as a
    *  class name populates a vector with the "RasterLayer" and "Layer" strings.
    *
    *  @param   className
    *           The layer class name for which to get all layer element types.
    *  @param   classList
    *           This vector is populated with the class names of all inherited layer
    *           classes and the given class name.
    */
   virtual void getLayerTypes(const std::string& className, std::vector<std::string>& classList) = 0;

   /**
    *  Derive a new Product from an existing View.
    *
    *  This method creates a new ProductWindow and populates the view object with the specified View.
    *
    *  @param pView
    *         The View used to populate the new ProductWindow.
    *
    *  @return The new ProductWindow or NULL if there was an error.
    */
   virtual ProductWindow *deriveProduct(View *pView) = 0;

   /**
    *  Creates a plot widget that is not contained in a plot window.
    *
    *  This method creates a plot widget that is not associated with a plot
    *  window.  The widget is not owned by another object, so it should be
    *  deleted when necessary by calling the deletePlotWidget() method.
    *
    *  @param   plotName
    *           The plot name.  Cannot be empty.
    *  @param   plotType
    *           The plot type to create.
    *  @param   pParent
    *           The Qt widget to use as the parent for the plot widget.
    *
    *  @return  A pointer to the newly created PlotWidget.
    *
    *  @see     PlotType
    */
   virtual PlotWidget* createPlotWidget(const std::string& plotName, PlotType plotType, QWidget* pParent = 0) = 0;

   /**
    *  Destroys a plot widget.
    *
    *  This method is intended to destroy plot widgets created with the
    *  createPlotWidget() method.  To destroy plot widgets displayed in a plot
    *  window, get the plot set containing the plot widget and call
    *  PlotSet::deletePlot() instead.
    *
    *  @param   pPlot
    *           The plot widget to delete.  If the given plot widget is
    *           displayed in a plot window, the plot widget is removed from its
    *           parent plot set.
    */
   virtual void deletePlotWidget(PlotWidget* pPlot) = 0;

   /**
    *  Creates an axis widget that is not contained in a PlotWidget.
    *
    *  @param   position
    *           The position where the axis will be displayed relative to a
    *           plot view.  The position defines how the tick marks are drawn
    *           and how the text is oriented.
    *  @param   pParent
    *           The Qt widget to use as the parent for the axis widget.  The
    *           parent widget assumes ownership of the axis widget and will
    *           destroy the axis when it is destroyed.  If \b NULL is passed in
    *           the deleteAxis() method should be called to destroy the
    *           widget.
    *
    *  @return  A pointer to the newly created Axis widget.
    */
   virtual Axis* createAxis(AxisPosition position, QWidget* pParent = 0) = 0;

   /**
    *  Destroys an axis widget.
    *
    *  This method is intended to destroy axis widgets that were created with
    *  the createAxis() method.  This method should not be called if the axis
    *  widget was created with a valid Qt parent widget.
    *
    *  @param   pAxis
    *           The Axis widget to delete.
    */
   virtual void deleteAxis(Axis* pAxis) = 0;

   /**
    *  Initializes the keyboard shortcut in an action.
    *
    *  This method initializes the keyboard shortcut in a given action from the
    *  user-defined configuration settings.  If the shortcut for the action
    *  already exists in the configuration settings based on the given context,
    *  QAction::setShortcut() is called with that shortcut value.  If the
    *  shortcut for the action does not exist in the configuration settings, an
    *  entry is added with the value set to the current shortcut value in the
    *  action.
    *
    *  @param   pAction
    *           The action in which to initialize its keyboard shortcut.
    *  @param   shortcutContext
    *           The context name for the keyboard shortcut of the action.  When
    *           the user edits keyboard shortcuts, the shortcuts are grouped
    *           according to their context string.  Contexts can be nested by
    *           using a slash (/) between context levels.  If the top-level
    *           context name is the same as a plug-in name, the shortcut is
    *           grouped under a single Plug-Ins context.  This method does
    *           nothing if an empty string is passed in.
    *
    *  @see     ToolBar::addButton(QAction*, const std::string&),
    *           MenuBar::insertCommand()
    */
   virtual void initializeAction(QAction* pAction, const std::string& shortcutContext) = 0;

   /**
    *  Creates the mouse mode.
    *
    *  This method creates a mouse mode that is available to add to a view.
    *
    *  @param   modeName
    *           The name for the mouse mode.  Mouse mode names must be unique for
    *           the view type to which they are added.
    *  @param   mouseCursor
    *           An icon to use for the mouse cursor in XPM format.  Cursor icons
    *           can be any size, but typically cursors are 32 x 32 pixels in size
    *           or less.  Currently only black and white cursors are supported.
    *  @param   cursorMask
    *           An icon to use as a mask for the cursor icon.  The mask should
    *           act as a bit mask indicating which cursor icon pixels should be
    *           drawn and which pixels should be transparent.  The size of the
    *           icon should be the same as \e mouseCursor.
    *  @param   iHotX
    *           The x-coordinate for the cursor hot spot.
    *  @param   iHotY
    *           The y-coordinate for the cursor hot spot.
    *  @param   pAction
    *           A Qt action associated with the mouse mode.  When the mouse mode
    *           is added to a view, a non-\c NULL value will add the action to the
    *           main mouse mode action group containing the following mouse modes:
    *           @arg    "LayerMode"
    *           @arg    "MeasurementMode"
    *           @arg    "PanMode"
    *           @arg    "RotateMode"
    *           @arg    "ZoomInMode"
    *           @arg    "ZoomOutMode"
    *           @arg    "ZoomBoxMode"
    *
    *           When the mouse mode is activated in the view, the action is toggled
    *           on and the other actions in the group are toggled off.  So, this
    *           value should only be set if the action should be added to the action
    *           group.
    *
    *  @return  A pointer to the newly created mouse mode.
    *
    *  @see     createMouseMode(const std::string&, const QCursor&, QAction*) const,
    *           MouseMode, deleteMouseMode(), View::addMouseMode(), View::setMouseMode()
    */
   virtual MouseMode* createMouseMode(const std::string& modeName, const char* const mouseCursor[],
      const char* const cursorMask[], int iHotX = 0, int iHotY = 0, QAction* pAction = NULL) const = 0;

   /**
    *  Creates the mouse mode.
    *
    *  This overloaded method serves as a convenience to create a mouse mode
    *  that is available to add to a view.
    *
    *  @param   modeName
    *           The name for the mouse mode.  Mouse mode names must be unique for
    *           the view type to which they are added.
    *  @param   mouseCursor
    *           The cursor that should be displayed when the mouse mode is active.
    *           Cursors  can be any size, but typically they are 32 x 32 pixels
    *           in size or less.  Currently only black and white cursors are
    *           supported.
    *  @param   pAction
    *           A Qt action associated with the mouse mode.  When the mouse mode
    *           is added to a view, a non-\c NULL value will add the action to the
    *           main mouse mode action group containing the following mouse modes:
    *           @arg    "LayerMode"
    *           @arg    "MeasurementMode"
    *           @arg    "PanMode"
    *           @arg    "RotateMode"
    *           @arg    "ZoomInMode"
    *           @arg    "ZoomOutMode"
    *           @arg    "ZoomBoxMode"
    *
    *           When the mouse mode is activated in the view, the action is toggled
    *           on and the other actions in the group are toggled off.  So, this
    *           value should only be set if the action should be added to the action
    *           group.
    *
    *  @return  A pointer to the newly created mouse mode.
    *
    *  @see     MouseMode, deleteMouseMode(), View::addMouseMode(), View::setMouseMode()
    */
   virtual MouseMode* createMouseMode(const std::string& modeName, const QCursor& mouseCursor,
      QAction* pAction = NULL) const = 0;

   /**
    *  Deletes a mouse mode.
    *
    *  This method deletes a mouse mode.  The mode should have already been
    *  removed from all applicable views.  The MouseMode pointer is invalid after
    *  calling this method.
    *
    *  WARNING: Mouse modes provided by the application cannot be deleted using
    *  this method, they will be cleaned up during application close.
    *
    *  @param   pMouseMode
    *           The mouse mode to delete.
    *
    *  @see     MouseMode
    */
   virtual void deleteMouseMode(MouseMode* pMouseMode) const = 0;

   /**
    *  Sets the application pan mode.
    *
    *  This method sets the pan mode for the entire application.  All
    *  views in all windows use this pan mode when panning through the
    *  view.
    *
    *  @param   panMode
    *           The pan mode.
    *
    *  @see     PanMode
    */
   virtual void setPanMode(PanModeType panMode) = 0;

   /**
    *  Returns the application pan mode.
    *
    *  @return  The pan mode.
    */
   virtual PanModeType getPanMode() const = 0;

   /**
    *  Sets the current AOI pixel selection tool on the AOI toolbar.
    *
    *  This method sets the current object that is created when the user adds a
    *  new object to an AOI layer and the drawing mode that is used when the
    *  user edits the layer with the mouse.  The given object type and drawing
    *  mode are set into the current AOI layer of the current view and into
    *  each AOI layer that is activated in any view.  To set the graphic object
    *  type and drawing mode in the layer separately from the AOI toolbar, call
    *  GraphicLayer::setCurrentGraphicObjectType() or AoiLayer::setMode()
    *  instead.
    *
    *  @param   toolType
    *           The new pixel selection tool.
    *  @param   modeType
    *           The new pixel selection mode.
    *
    *  @deprecated
    *           This method is deprecated, and may be removed in a future
    *           version.\  Use AoiToolBar::setSelectionTool() instead.
    */
   virtual void setAoiSelectionTool(GraphicObjectType toolType, ModeType modeType) = 0;
   
   /**
    *  Sets the current AOI add mode on the AOI toolbar.
    *
    *  @param   mode
    *           The new add mode.
    *
    *  @deprecated
    *           This method is deprecated, and may be removed in a future
    *           version.\  Use AoiToolBar::setAddMode() instead.
    */
   virtual void setAoiAddMode(AoiAddMode mode) = 0;

   /**
    *  Returns the current AOI pixel selection tool on the AOI toolbar.
    *
    *  @return  The current pixel selection tool.
    *
    *  @deprecated
    *           This method is deprecated, and may be removed in a future
    *           version.\  Use AoiToolBar::getSelectionTool() instead.
    */
   virtual GraphicObjectType getAoiSelectionTool() const = 0;

   /**
    *  Returns the current AOI pixel selection mode on the AOI toolbar.
    *
    *  @return  The current pixel selection mode.
    *
    *  @deprecated
    *           This method is deprecated, and may be removed in a future
    *           version.\  Use AoiToolBar::getSelectionMode() instead.
    */
   virtual ModeType getAoiSelectionMode() const = 0;

   /**
    *  Returns the current AOI add mode on the AOI toolbar.
    *
    *  @return  The current add mode.
    *
    *  @deprecated
    *           This method is deprecated, and may be removed in a future
    *           version.\  Use AoiToolBar::getAddMode() instead.
    */
   virtual AoiAddMode getAoiAddMode() const = 0;

   /**
    *  Sets the current annotation object on the Annotation toolbar.
    *
    *  This method sets the current object that is created when the user adds a
    *  new object to an annotation layer.  The given object is set into the
    *  current annotation layer of the current view and into each annotation
    *  layer that is activated in any view.  To set the graphic object in the
    *  layer separately from the Annotation toolbar, call
    *  GraphicLayer::setCurrentGraphicObjectType() instead.
    *
    *  @param   objectType
    *           The new annotation object.
    */
   virtual void setAnnotationObject(GraphicObjectType objectType) = 0;

   /**
    *  Retrieves the current annotation object on the Annotation toolbar.
    *
    *  @return  The current annotation object.
    */
   virtual GraphicObjectType getAnnotationObject() const = 0;

   /**
    *  Loads data from a file.
    *
    *  This method is used to open any type of data from a file.  A file selection
    *  dialog appears for the user to select a desired file and importer.  If a
    *  valid importer subtype is given, only the importers of that subtype are
    *  available to the user; otherwise all importers are available.
    *
    *  @param   importerSubtype
    *           The importer subtype for which to set the available importer in
    *           the file selection dialog.  Passing in an empty string allows all
    *           importers to be available.
    *  @param   pProgress
    *           The progress object to use when importing the data.  If \b NULL
    *           is passed in, a new progress object is created.
    *
    *  @return  Returns \b true if at least one or more data sets was successfully
    *           loaded from the file.
    *  @see importFile(const std::string&, Progress*, std::vector<DataElement*>&)
    */
   virtual bool importFile(const std::string& importerSubtype = std::string(), Progress* pProgress = NULL) = 0;


   /**
    *  Loads data from a file.
    *
    *  This method is used to open any type of data from a file.  A file selection
    *  dialog appears for the user to select a desired file and importer.  If a
    *  valid importer subtype is given, only the importers of that subtype are
    *  available to the user; otherwise all importers are available.
    *
    *  @param   importerSubtype
    *           The importer subtype for which to set the available importer in
    *           the file selection dialog.  Passing in an empty string allows all
    *           importers to be available.
    *  @param   pProgress
    *           The progress object to use when importing the data.  If \b NULL
    *           is passed in, a new progress object is created.
    *  @param   importedElements
    *           A vector that will be populated with the data elements that were
    *           successfully imported from the user selected file.  This vector
    *           will be empty if the user cancels the import operation or no data
    *           elements were successfully imported.
    *
    *  @return  Returns \b true if at least one or more data sets was successfully
    *           loaded from the file.
    *  @see importFile(const std::string&, Progress*)
    */
   virtual bool importFile(const std::string& importerSubtype, Progress* pProgress, std::vector<DataElement*>& importedElements) = 0;

   /**
    *  Saves a session item to disk.
    *
    *  This method is used to save data from a given session item to disk.  A file
    *  selection dialog is invoked for the user to select an exporter and a
    *  filename in which the data will be saved.
    *
    *  @param   pItem
    *           The session item to export.
    *  @param   pNewFileDescriptor
    *           The FileDescriptor to use for export.  If \c NULL is passed in,
    *           a default FileDescriptor is created.
    *  @param   pProgress
    *           The progress object to use when exporting the session item.  If
    *           \c NULL is passed in, a new progress object is created.
    *
    *  @return  Returns \c true if the session item was successfully saved to
    *           disk; otherwise returns \c false.
    */
   virtual bool exportSessionItem(SessionItem *pItem, FileDescriptor* pNewFileDescriptor = NULL,
      Progress* pProgress = NULL) = 0;

   /**
    *  Saves multiple session items to disk.
    *
    *  This method is used to save data from multiple session items to disk.  A
    *  dialog is invoked for the user to select a directory in which the data
    *  will be saved, the exporter, and the default file extension for the new
    *  files.
    *
    *  Each session item is saved to a separate file with a default name based
    *  on the session item name.
    *
    *  @param   items
    *           The session items to export.  Each session item in the vector
    *           must be of the same type returned by
    *           TypeAwareObject::getObjectType().
    *  @param   pProgress
    *           The progress object to use when exporting the session items.
    *           If \c NULL is passed in, a new progress object is created.
    *
    *  @return  Returns \c true if at least one session item was successfully
    *           saved to disk.  Returns \c false if no session items were
    *           successfully saved or if the given session items are not all of
    *           the same type.
    *
    *  @see     SessionItem::getName()
    */
   virtual bool exportSessionItems(const std::vector<SessionItem*>& items, Progress* pProgress = NULL) = 0;

   /**
    *  Displays the main application online help page.
    *
    *  @return  TRUE if the browser was successfully launched with the main help page
    *           displayed, otherwise FALSE.
    */
   virtual bool displayHelpHome() const = 0;

   /**
    *  Displays a given help file.
    *
    *  This method displays the given HTML help file by launching the default browser.
    *  A specific target within the file can be displayed by appending the # operator
    *  followed by the target name.
    *
    *  @param   filename
    *           The existing HTML help file.
    *
    *  @return  TRUE if the browser was successfully launched with the file displayed,
    *           otherwise FALSE.
    */
   virtual bool displayHelp(const std::string& filename) const = 0;

   /**
    *  Invokes the properties dialog for a given session item.
    *
    *  @param   pItem
    *           The session item for which to display its properties.  This
    *           method does nothing if \c NULL is passed in.
    *
    *  @notify  This method emits the signalAboutToShowPropertiesDialog()
    *           signal just before the properties dialog is about to be
    *           displayed.
    *
    *  @see     displayProperties(SessionItem*, const std::vector<std::string>&, bool) const
    */
   virtual void displayProperties(SessionItem* pItem) const = 0;

   /**
    *  Invokes the properties dialog containing only certain page widgets for a
    *  given session item.
    *
    *  This method provides a means to hide default properties page widgets for
    *  the given session item.  Only the properties pages given in the vector
    *  will be displayed in the properties dialog.
    *
    *  @param   pItem
    *           The session item for which to display its properties.  This
    *           method does nothing if \c NULL is passed in.
    *  @param   displayedPages
    *           The names of Properties plug-in page widgets to display in the
    *           properties dialog.
    *  @param   bEmitSignal
    *           Set this parameter to \c true to emit the
    *           signalAboutToShowPropertiesDialog() signal just before the
    *           dialog is shown.  Set the parameter to \c false to not emit the
    *           signal and prevent other objects from adding pages not
    *           contained in \em displayedPages.
    *
    *  @notify  This method emits the signalAboutToShowPropertiesDialog()
    *           signal just before the properties dialog is about to be
    *           displayed if the \em bEmitSignal parameter is set to \c true.
    *
    *  @see     displayProperties(SessionItem*) const
    */
   virtual void displayProperties(SessionItem* pItem, const std::vector<std::string>& displayedPages,
      bool bEmitSignal = true) const = 0;

   /**
    *  Displays a message on the main application status bar.
    *
    *  This method displays a message on the status bar for five seconds.  %Any message
    *  currently displayed in the status bar is replaced.
    *
    *  @param   messageText
    *           The message to display.  Can be empty.
    */
   virtual void setStatusBarMessage(const std::string& messageText) const = 0;

   /**
    *   Sets the state of the check box inside a suppressible message dialog. 
    *
    *   The check box state is stored in ConfigurationSettings.
    *   
    *   Please see \ref suppressiblemsgdlg for more details.
    *
    *   @param   id
    *            The suppressible message dialog key that will be passed into ConfigurationSettings::getSetting() to set
    *            the dialog's check box state.
    *            
    *   @param   bState
    *            The state of the check box inside the suppressible message dialog.
    *
    *   @see     getSuppressibleMsgDlgState(), showSuppressibleMsgDlg(), SuppressibleMsgDlg
    */
   virtual void setSuppressibleMsgDlgState(const std::string& id, bool bState) = 0;

   /**
    *  Gets the state of the check box inside a suppressible message dialog.
    *
    *  Please see \ref suppressiblemsgdlg for more details.
    *
    *  @param   id
    *           This ID will be used to get the suppressible message dialog check box state from ConfigurationSettings::getSetting().
    *
    *  @return  This method will return \c true if the state of the check box inside the suppressible message dialog is checked,
    *           meaning that the dialog will not be shown again. If the dialog check box is not checked, then the method will
    *           return \c false. 
    *
    *  @see     setSuppressibleMsgDlgState(), showSuppressibleMsgDlg(), SuppressibleMsgDlg
    */
   virtual bool getSuppressibleMsgDlgState(const std::string& id) = 0;

   /**
    *  Displays a suppressible message dialog that the user can disable by clicking on the check box inside the dialog.
    *
    *  This method will display a suppressible message dialog based on its ID as long as its "don't show this again" 
    *  check box has not been checked. While this method shows the dialog, it's the dialog itself that 
    *  handles both showing and reading of the "don't show this again" check box state.
    *  The icon that will be shown next to the message in the dialog will depend on the message type.
    *
    *  Each suppressible message dialog must have its own unique ID. This ID will be passed into 
    *  ConfigurationSettings::getSetting() in order to get the dialog's current check box state.
    *
    *  If a plug-in developer wanted to give the user more control on when the message dialog 
    *  gets displayed, the developer will have to write an option plug-in himself. For a example on how 
    *  this would be done, take a look at the OptionsSuppressibleMsgDlg class located in PlugInSamplerQt.
    *
    *  Please see \ref suppressiblemsgdlg for more details.
    *
    *  @param   dialogTitle
    *           The text that will appear on the suppressible message dialog title bar.
    *  @param   dialogMsg
    *           The text that will be displayed inside the suppressible message dialog.
    *  @param   type
    *           The type of message.
    *  @param   id
    *           The message dialog ID that will be used to get the check box state from ConfigurationSettings::getSetting().
    *  @param   pParent
    *           The message box parent. If \em pParent is NULL, the main application window will be the parent.
    *
    *  @see     getSuppressibleMsgDlgState(), setSuppressibleMsgDlgState(), SuppressibleMsgDlg
    */
   virtual void showSuppressibleMsgDlg(const std::string& dialogTitle, const std::string& dialogMsg, MessageType type, 
      const std::string& id, QWidget* pParent = NULL) = 0;

   /**
    *  Displays a message box to the user.
    *
    *  In interactive mode, this method displays a QMessageBox to the user. 
    *  It will not return until the user presses one of the buttons on the 
    *  message box. In batch mode, this method writes the caption and text to 
    *  standard out and returns the default button value.
    *
    *  @param   caption
    *           The text to put on the window title bar
    *  @param   text
    *           The message text for the dialog
    *  @param   button0
    *           The label for the first button. If this string is empty, the 
    *           button will not be displayed.
    *  @param   button1
    *           The label for the second button. If this string is empty, the 
    *           button will not be displayed.
    *  @param   button2
    *           The label for the third button. If this string is empty, the 
    *           button will not be displayed.
    *  @param   defaultButton
    *           The button that will be selected if the user simply presses
    *           the 'Enter' key. If this value does not correspond to any
    *           button, the 'Enter' key will be ignored.
    *  @param   escapeButton
    *           The button that will be selected if the user simply presses
    *           the 'Escape' key. If this value does not correspond to any
    *           button, pressing the 'Escape' key will cause the dialog to be
    *           dismissed and the value specified will be returned.
    *
    *  @return  The number of the button the user selected.
    */
   virtual int showMessageBox(const std::string &caption, const std::string &text, 
      const std::string &button0=std::string(), const std::string &button1=std::string(), 
      const std::string &button2=std::string(), int defaultButton=0, 
      int escapeButton=-1) const = 0;

   /**
    *  Adds a new background plug-in to the Background Plug-In %Window.
    *
    *  This method adds an item to the background plug-in window that allows
    *  the user to view progress and abort the plug-in if possible.  The
    *  method should be called after executing a background plug-in in
    *  interactive mode.
    *
    *  @param   pPlugIn
    *           The background plug-in to add to the Background Plug-In %Window.
    *  @param   pProgress
    *           The (thread-safe) Progress object that the plug-in uses.
    */
   virtual void addBackgroundPlugIn(PlugIn* pPlugIn, Progress* pProgress) const = 0;

   /**
    *  Registers a callback to the core.
    *
    *  Valid callback types are enumerated in TypesFile.h.  Currently, only
    *  callbacks signaling background processing are supported.
    *
    *  @param   eType
    *           The type of callback to register.
    *  @param   pCallback
    *           The callback being registered.  The application assumes
    *           ownership of the callback object and deletes it after executing
    *           the callback.  The plug-in should not attempt to delete the
    *           callback object itself.
    *
    *  @return  Returns \c true if the callback was registered properly; returns
    *           \c false if there was an error registering or if an invalid
    *           callback type was specified.
    *
    *  @see     PlugInCallbackType, Executable::isBackground()
    */
   virtual bool registerCallback(PlugInCallbackType eType, PlugInCallback* pCallback) const = 0;

   /**
    *  Query the dock location of the specified DockWindow
    *
    *  @param dockWindow
    *         The DockWindow whose location is being queried
    *
    *  @return the location of the DockWindow
    */
   virtual DockWindowAreaType getDockWindowArea(const DockWindow &dockWindow) const = 0;

   /**
    *  Sets a DockWindow to the given dock area.
    *
    *  @param   pDockWindow
    *           DockWindow to move.
    *
    *  @param   dockArea
    *           Area to dock with.
    *
    *  @return  True if the operation was successful, false otherwise.
    */
  virtual bool setDockWindowArea(DockWindow *pDockWindow, 
     DockWindowAreaType dockArea) = 0;

   /**
    *  Attach a Progress object to an appropriate dialog.
    *
    *  @param caption
    *         A caption for the view.
    *
    *  @param pProgress
    *         The Progress object to attach.
    *
    *  @returns True if successful, false otherwise.
    */
   virtual bool createProgressDialog(const std::string &caption, Progress *pProgress) const = 0;

   /**
    * Get a list of all available symbols for MultipointObjects.
    *
    * The list is formed from all supported files within the Annotation directory.
    * Only CGM files are currently supported.
    *
    * @returns A vector of strings containing the names of available symbols.
    */
   virtual const std::vector<std::string> &getAvailableSymbolNames() const = 0;

   /**
    * Get a QImage of the given symbol, for use with MultipointObjects.
    *
    * @param symbol
    *        The name of the symbol to get the image of.  This name
    *        should be contained in the vector gotten from getAvailableSymbolNames().
    *
    * @return A QImage containing the drawn symbol.
    */
   virtual const QImage &getSymbolImage(const std::string &symbol) const = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~DesktopServices() {}
};

#endif
