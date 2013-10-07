/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONTEXTMENUACTIONS_H
#define CONTEXTMENUACTIONS_H

/**
 *  Displays dialog for user to adjust the left and right bumper values.
 */
#define APP_ANIMATIONCONTROLLER_ADJUST_BUMPERS_ACTION "APP_ANIMATIONCONTROLLER_ADJUST_BUMPERS_ACTION"

/**
 *  Animation bumper actions separator.
 */
#define APP_ANIMATIONCONTROLLER_BUMPER_SEPARATOR_ACTION "APP_ANIMATIONCONTROLLER_BUMPER_SEPARATOR_ACTION"

/**
 *  Changes the direction of animation play.
 */
#define APP_ANIMATIONCONTROLLER_CHANGE_DIRECTION_ACTION "APP_ANIMATIONCONTROLLER_CHANGE_DIRECTION_ACTION"

/**
 *  Toggles whether frames should be dropped to preserve the frame speed.
 */
#define APP_ANIMATIONCONTROLLER_DROP_FRAMES_ACTION "APP_ANIMATIONCONTROLLER_DROP_FRAMES_ACTION"

/**
 *  Animation drop frames actions separator.
 */
#define APP_ANIMATIONCONTROLLER_DROP_FRAMES_SEPARATOR_ACTION "APP_ANIMATIONCONTROLLER_DROP_FRAMES_SEPARATOR_ACTION"

/**
 *  Enables the animation bumpers to limit playback to a subset of the animation.
 */
#define APP_ANIMATIONCONTROLLER_ENABLE_BUMPERS_ACTION "APP_ANIMATIONCONTROLLER_ENABLE_BUMPERS_ACTION"

/**
 *  Stops animation at the current frame.
 */
#define APP_ANIMATIONCONTROLLER_PAUSE_ACTION "APP_ANIMATIONCONTROLLER_PAUSE_ACTION"

/**
 *  Begins animation.
 */
#define APP_ANIMATIONCONTROLLER_PLAY_ACTION "APP_ANIMATIONCONTROLLER_PLAY_ACTION"

/**
 *  Resets the bumpers to the start and stop frames of the animation.
 */
#define APP_ANIMATIONCONTROLLER_RESET_BUMPERS_ACTION "APP_ANIMATIONCONTROLLER_RESET_BUMPERS_ACTION"

/**
 *  Toggles resetting the animation when the animation is stopped.
 */
#define APP_ANIMATIONCONTROLLER_RESET_ON_STOP_ACTION "APP_ANIMATIONCONTROLLER_RESET_ON_STOP_ACTION"

/**
 *  Restores the bumpers to the values in the user's configuration settings.
 */
#define APP_ANIMATIONCONTROLLER_RESTORE_BUMPERS_ACTION "APP_ANIMATIONCONTROLLER_RESTORE_BUMPERS_ACTION"

/**
 *  Sets the left (start) bumper to the current frame of the animation.
 */
#define APP_ANIMATIONCONTROLLER_SET_LEFT_BUMPER_ACTION "APP_ANIMATIONCONTROLLER_SET_LEFT_BUMPER_ACTION"

/**
 *  Sets the right (stop) bumper to the current frame of the animation.
 */
#define APP_ANIMATIONCONTROLLER_SET_RIGHT_BUMPER_ACTION "APP_ANIMATIONCONTROLLER_SET_RIGHT_BUMPER_ACTION"

/**
 *  Moves backward one frame in the animation.
 */
#define APP_ANIMATIONCONTROLLER_STEP_BACKWARD_ACTION "APP_ANIMATIONCONTROLLER_STEP_BACKWARD_ACTION"

/**
 *  Moves forward one frame in the animation.
 */
#define APP_ANIMATIONCONTROLLER_STEP_FORWARD_ACTION "APP_ANIMATIONCONTROLLER_STEP_FORWARD_ACTION"

/**
 *  Stops animation and sets the current frame to the first frame.
 */
#define APP_ANIMATIONCONTROLLER_STOP_ACTION "APP_ANIMATIONCONTROLLER_STOP_ACTION"

/**
 *  Stores the current bumper values in the user's configuration settings.
 */
#define APP_ANIMATIONCONTROLLER_STORE_BUMPERS_ACTION "APP_ANIMATIONCONTROLLER_STORE_BUMPERS_ACTION"

/**
 *  Animation bumper store/restore actions separator.
 */
#define APP_ANIMATIONCONTROLLER_STORE_BUMPERS_SEPARATOR_ACTION "APP_ANIMATIONCONTROLLER_STORE_BUMPERS_SEPARATOR_ACTION"

/**
 *  Appearing in a context menu for an AnimationController, this action sets
 *  the controller widget to be the current controller on the %Animation
 *  toolbar.
 */
#define APP_ANIMATIONSERVICES_ACTIVATE_ACTION "APP_ANIMATIONSERVICES_ACTIVATE_ACTION"

/**
 *  Appearing in a context menu for an AnimationController, this action
 *  destroys the controller and all of its animations.
 */
#define APP_ANIMATIONSERVICES_DELETE_ACTION "APP_ANIMATIONSERVICES_DELETE_ACTION"

/**
 *  Appearing in the %Animation tab context menu in the Session Explorer when no
 *  items are selected, this action creates and activates a new animation
 *  controller, prompting the user for the controller name and type.
 */
#define APP_ANIMATIONSERVICES_NEW_CONTROLLER_ACTION "APP_ANIMATIONSERVICES_NEW_CONTROLLER_ACTION"

/**
 *  Default menu separator.
 */
#define APP_ANIMATIONSERVICES_SEPARATOR_ACTION "APP_ANIMATIONSERVICES_SEPARATOR_ACTION"

/**
 *  Copies an image of the active view to the clipboard.
 */
#define APP_APPLICATIONWINDOW_COPY_SNAPSHOT_ACTION "APP_APPLICATIONWINDOW_COPY_SNAPSHOT_ACTION"

/**
 *  Default menu separator.
 */
#define APP_APPLICATIONWINDOW_COPY_SNAPSHOT_SEPARATOR_ACTION "APP_APPLICATIONWINDOW_COPY_SNAPSHOT_SEPARATOR_ACTION"

/**
 *  Copies an image of the active view to the clipboard.\   The user is prompted
 *  to change the size of the copied image.
 */
#define APP_APPLICATIONWINDOW_COPY_SNAPSHOT_SIZED_ACTION "APP_APPLICATIONWINDOW_COPY_SNAPSHOT_SIZED_ACTION"

/**
 *  Destroys the selected DataElement.
 */
#define APP_APPLICATIONWINDOW_DATAELEMENT_DELETE_ACTION "APP_APPLICATIONWINDOW_DATAELEMENT_DELETE_ACTION"

/**
 *  Edits the classification markings for multiple selected ViewWindow or
 *  DataElement objects in the Session Explorer.
 */
#define APP_APPLICATIONWINDOW_EDIT_CLASSIFICATION_ACTION "APP_APPLICATIONWINDOW_EDIT_CLASSIFICATION_ACTION"

/**
 *  Export the selected SessionItem.
 */
#define APP_APPLICATIONWINDOW_EXPORT_ACTION "APP_APPLICATIONWINDOW_EXPORT_ACTION"

/**
 *  Links selected views in the Session Explorer.
 */
#define APP_APPLICATIONWINDOW_LINK_ACTION "APP_APPLICATIONWINDOW_LINK_ACTION"

/**
 *  Default menu separator.
 */
#define APP_APPLICATIONWINDOW_LINK_SEPARATOR_ACTION "APP_APPLICATIONWINDOW_LINK_SEPARATOR_ACTION"

/**
 *  Displays properties of the selected SessionItem.
 */
#define APP_APPLICATIONWINDOW_PROPERTIES_ACTION "APP_APPLICATIONWINDOW_PROPERTIES_ACTION"

/**
 *  Unlinks selected views in the Session Explorer.
 */
#define APP_APPLICATIONWINDOW_UNLINK_ACTION "APP_APPLICATIONWINDOW_UNLINK_ACTION"

/**
 *  Zooms the plot axes to user-defined extents.
 */
#define APP_CARTESIANPLOT_CUSTOM_ZOOM_ACTION "APP_CARTESIANPLOT_CUSTOM_ZOOM_ACTION"

/**
 *  Toggles the display of major and minor horizontal and vertical gridlines.
 */
#define APP_CARTESIANPLOT_GRIDLINES_MENU_ACTION "APP_CARTESIANPLOT_GRIDLINES_MENU_ACTION"

/**
 *  Attaches the dock window to the main application window.\   This action
 *  only appears in the context menu if the window is currently undocked.
 */
#define APP_DOCKWINDOW_DOCK_ACTION "APP_DOCKWINDOW_DOCK_ACTION"

/**
 *  Closes the dock window.\   This action only appears in the context menu if
 *  the window is currently shown.
 */
#define APP_DOCKWINDOW_HIDE_ACTION "APP_DOCKWINDOW_HIDE_ACTION"

/**
 *  Displays the dock window.\   This action only appears in the context menu
 *  if the window is currently hidden.
 */
#define APP_DOCKWINDOW_SHOW_ACTION "APP_DOCKWINDOW_SHOW_ACTION"

/**
 *  Floats the dock window on the desktop.\   This action only appears in the
 *  context menu if the window is currently docked.
 */
#define APP_DOCKWINDOW_UNDOCK_ACTION "APP_DOCKWINDOW_UNDOCK_ACTION"

/**
 *  Destroys the selected GraphicObject objects contained in a GraphicLayer.
 */
#define APP_GRAPHICLAYER_OBJECT_DELETE_ACTION "APP_GRAPHICLAYER_OBJECT_DELETE_ACTION"

/**
 *  Default menu separator.
 */
#define APP_GRAPHICLAYER_OBJECT_DELETE_SEPARATOR_ACTION "APP_GRAPHICLAYER_OBJECT_DELETE_SEPARATOR_ACTION"

/**
 *  Toggles the setting that automatically zooms the plot to the extents of the
 *  histogram data when the displayed band of a RasterLayer changes.
 */
#define APP_HISTOGRAMPLOT_AUTO_ZOOM_ACTION "APP_HISTOGRAMPLOT_AUTO_ZOOM_ACTION"

/**
 *  Toggles the histogram count axis scale between linear and logarithmic.
 */
#define APP_HISTOGRAMPLOT_AXIS_SCALE_MENU_ACTION "APP_HISTOGRAMPLOT_AXIS_SCALE_MENU_ACTION"

/**
 *  Invokes a dialog for users to filter data values out of the displayed
 *  histogram.
 */
#define APP_HISTOGRAMPLOT_BAD_VALUES_ACTION "APP_HISTOGRAMPLOT_BAD_VALUES_ACTION"

/**
 *  Changes the displayed band of a RasterLayer.
 */
#define APP_HISTOGRAMPLOT_BAND_MENU_ACTION "APP_HISTOGRAMPLOT_BAND_MENU_ACTION"

/**
 *  Applies a color map to a RasterLayer.
 */
#define APP_HISTOGRAMPLOT_COLOR_MAP_MENU_ACTION "APP_HISTOGRAMPLOT_COLOR_MAP_MENU_ACTION"

/**
 *  Changes the displayed histogram to a different component of complex data.
 */
#define APP_HISTOGRAMPLOT_COMPLEX_DATA_MENU_ACTION "APP_HISTOGRAMPLOT_COMPLEX_DATA_MENU_ACTION"

/**
 *  Changes the displayed element of a RasterLayer.
 */
#define APP_HISTOGRAMPLOT_ELEMENT_MENU_ACTION "APP_HISTOGRAMPLOT_ELEMENT_MENU_ACTION"

/**
 *  Default menu separator that is added at the end of the group of histogram
 *  plot actions added to the context menu.
 */
#define APP_HISTOGRAMPLOT_END_SEPARATOR_ACTION "APP_HISTOGRAMPLOT_END_SEPARATOR_ACTION"

/**
 *  Changes the range of values to which the display threshold is applied for a
 *  ThresholdLayer.
 */
#define APP_HISTOGRAMPLOT_PASS_AREA_MENU_ACTION "APP_HISTOGRAMPLOT_PASS_AREA_MENU_ACTION"

/**
 *  Default menu separator that is used when the histogram of a RasterLayer is
 *  displayed.\   This separator appears before the raster layer submenus and
 *  after the auto-zoom action(s).
 */
#define APP_HISTOGRAMPLOT_RASTER_MENUS_SEPARATOR_ACTION "APP_HISTOGRAMPLOT_RASTER_MENUS_SEPARATOR_ACTION"

/**
 *  Default menu separator that is used when the histogram of a RasterLayer is
 *  displayed.\   This separator appears before the auto-zoom action(s).
 */
#define APP_HISTOGRAMPLOT_RASTER_SEPARATOR_ACTION "APP_HISTOGRAMPLOT_RASTER_SEPARATOR_ACTION"

/**
 *  Force recalculation of statistics.
 */
#define APP_HISTOGRAMPLOT_REFRESH_STATISTICS_ACTION "APP_HISTOGRAMPLOT_REFRESH_STATISTICS_ACTION"

/**
 *  Specifies the pixels that are used in calculating the histogram data for a
 *  layer.
 */
#define APP_HISTOGRAMPLOT_SAMPLING_ACTION "APP_HISTOGRAMPLOT_SAMPLING_ACTION"

/**
 *  Saves the raw histogram data to a text file.
 */
#define APP_HISTOGRAMPLOT_SAVE_ACTION "APP_HISTOGRAMPLOT_SAVE_ACTION"

/**
 *  Toggles the display of the histogram statistics.
 */
#define APP_HISTOGRAMPLOT_STATISTICS_ACTION "APP_HISTOGRAMPLOT_STATISTICS_ACTION"

/**
 *  Changes the contrast stretch to a preset user-defined value.
 */
#define APP_HISTOGRAMPLOT_STRETCH_FAVORITES_MENU_ACTION "APP_HISTOGRAMPLOT_STRETCH_FAVORITES_MENU_ACTION"

/**
 *  Resets the stretch properties of a RasterLayer.
 */
#define APP_HISTOGRAMPLOT_STRETCH_RESET_MENU_ACTION "APP_HISTOGRAMPLOT_STRETCH_RESET_MENU_ACTION"

/**
 *  Changes the contrast stretch type of a RasterLayer.
 */
#define APP_HISTOGRAMPLOT_STRETCH_TYPE_MENU_ACTION "APP_HISTOGRAMPLOT_STRETCH_TYPE_MENU_ACTION"

/**
 *  Sets the auto-zoom setting of the red, green, and blue histogram plots of a
 *  RasterLayer to that of the current RasterLayer histogram plot.
 */
#define APP_HISTOGRAMPLOT_SYNCHRONIZE_AUTO_ZOOM_ACTION "APP_HISTOGRAMPLOT_SYNCHRONIZE_AUTO_ZOOM_ACTION"

/**
 *  Default menu separator that is used when the histogram of a ThresholdLayer
 *  is displayed.
 */
#define APP_HISTOGRAMPLOT_THRESHOLD_SEPARATOR_ACTION "APP_HISTOGRAMPLOT_THRESHOLD_SEPARATOR_ACTION"

/**
 *  Changes the units of a RasterLayer or ThresholdLayer.
 */
#define APP_HISTOGRAMPLOT_UNITS_MENU_ACTION "APP_HISTOGRAMPLOT_UNITS_MENU_ACTION"

/**
 *  Calculate statistics over spatial/spectral subset and display in the histogram window.
 */
#define APP_LAYER_CALCULATE_SUBSET_STATISTICS_ACTION "APP_LAYER_CALCULATE_SUBSET_STATISTICS_ACTION"

/**
 *  A toggle action indicating whether the layer is currently displayed in its
 *  view.
 */
#define APP_LAYER_DISPLAYED_ACTION "APP_LAYER_DISPLAYED_ACTION"

/**
 *  Zooms the view displaying the layer to the extents of the layer.
 */
#define APP_LAYER_ZOOM_TO_LAYER_ACTION "APP_LAYER_ZOOM_TO_LAYER_ACTION"

/**
 *  Appearing only in a context menu for a PlotWidget, this action sets the
 *  plot widget to be the current plot widget in the plot set.
 */
#define APP_PLOTSET_ACTIVATE_ACTION "APP_PLOTSET_ACTIVATE_ACTION"

/**
 *  Destroys the current plot widget in the plot set.\   This action can also
 *  appear in a context menu for a specific PlotWidget.
 */
#define APP_PLOTSET_DELETE_ACTION "APP_PLOTSET_DELETE_ACTION"

/**
 *  Default menu separator that appears if other actions are displayed in
 *  addition to those of the plot set.
 */
#define APP_PLOTSET_SEPARATOR_ACTION "APP_PLOTSET_SEPARATOR_ACTION"

/**
 *  Indicates the current mouse mode for the plot view.
 */
#define APP_PLOTVIEW_MOUSE_MODE_MENU_ACTION "APP_PLOTVIEW_MOUSE_MODE_MENU_ACTION"

/**
 *  Default menu separator.
 */
#define APP_PLOTVIEW_MOUSE_MODE_SEPARATOR_ACTION "APP_PLOTVIEW_MOUSE_MODE_SEPARATOR_ACTION"

/**
 *  Restores the plot axes to the original scale.
 */
#define APP_PLOTVIEW_RESCALE_AXES_ACTION "APP_PLOTVIEW_RESCALE_AXES_ACTION"

/**
 *  Default menu separator.
 */
#define APP_PLOTVIEW_RESCALE_AXES_SEPARATOR_ACTION "APP_PLOTVIEW_RESCALE_AXES_SEPARATOR_ACTION"

/**
 *  Toggles the display of the plot legend.
 */
#define APP_PLOTWIDGET_LEGEND_ACTION "APP_PLOTWIDGET_LEGEND_ACTION"

/**
 *  Default menu separator separating these actions from the PlotView actions.
 */
#define APP_PLOTWIDGET_LEGEND_SEPARATOR_ACTION "APP_PLOTWIDGET_LEGEND_SEPARATOR_ACTION"

/**
 *  Sends an image of the plot widget to the printer.\   The image is a
 *  snapshot of the current widget, so only the current display area of the
 *  plot will be captured.
 */
#define APP_PLOTWIDGET_PRINT_ACTION "APP_PLOTWIDGET_PRINT_ACTION"

/**
 *  Default menu separator.
 */
#define APP_PLOTWIDGET_PRINT_SEPARATOR_ACTION "APP_PLOTWIDGET_PRINT_SEPARATOR_ACTION"

/**
 *  Toggles the display of plot widget toolbars.
 */
#define APP_PLOTWIDGET_TOOLBARS_MENU_ACTION "APP_PLOTWIDGET_TOOLBARS_MENU_ACTION"

/**
 *  Toggles the display of major and minor polar gridlines.
 */
#define APP_POLARPLOT_GRIDLINES_MENU_ACTION "APP_POLARPLOT_GRIDLINES_MENU_ACTION"

/**
 *  Activates the layer in the view.
 */
#define APP_PRODUCTVIEW_LAYER_ACTIVATE_ACTION "APP_PRODUCTVIEW_LAYER_ACTIVATE_ACTION"

/** 
 *  Default menu separator.
 */
#define APP_PRODUCTVIEW_PROPERTIES_SEPARATOR_ACTION "APP_PRODUCTVIEW_PROPERTIES_SEPARATOR_ACTION"

/**
 *  Sets the display mode to RGB and sets the displayed bands for the raster layer.
 */
#define APP_RASTERLAYER_DISPLAY_AS_MENU_ACTION "APP_RASTERLAYER_DISPLAY_AS_MENU_ACTION"

/**
 *  Sets the current display mode for the raster layer.
 */
#define APP_RASTERLAYER_DISPLAY_MODE_MENU_ACTION "APP_RASTERLAYER_DISPLAY_MODE_MENU_ACTION"

/**
 *  Default menu separator separating these actions from the Layer actions.
 */
#define APP_RASTERLAYER_SEPARATOR_ACTION "APP_RASTERLAYER_SEPARATOR_ACTION"

/**
 *  Indicates the current contrast stretch type for the raster layer.
 */
#define APP_RASTERLAYER_STRETCH_MENU_ACTION "APP_RASTERLAYER_STRETCH_MENU_ACTION"

/**
 *  Collapses all nodes in the current tree view.
 */
#define APP_SESSIONEXPLORER_COLLAPSE_ALL_ACTION "APP_SESSIONEXPLORER_COLLAPSE_ALL_ACTION"

/**
 *  Copy the name of the current item in the current SessionExplorer tab to the clipboard.
 */
#define APP_SESSIONEXPLORER_COPY_NAME_TO_CLIPBOARD_ACTION "APP_SESSIONEXPLORER_COPY_NAME_TO_CLIPBOARD_ACTION"

/**
 *  Expands all nodes in the current tree view.
 */
#define APP_SESSIONEXPLORER_EXPAND_ALL_ACTION "APP_SESSIONEXPLORER_EXPAND_ALL_ACTION"

/**
 *  Default menu separator separating these actions from the DockWindow
 *  actions.
 */
#define APP_SESSIONEXPLORER_SEPARATOR_ACTION "APP_SESSIONEXPLORER_SEPARATOR_ACTION"

/**
 *  Rename the current item in the current SessionExplorer tab.
 */
#define APP_SESSIONEXPLORER_RENAME_ACTION "APP_SESSIONEXPLORER_RENAME_ACTION"

/**
 *  Toggles the display of the orientation axis in the upper left corner of the view.
 */
#define APP_SPATIALDATAVIEW_AXIS_ACTION "APP_SPATIALDATAVIEW_AXIS_ACTION"

/**
 *  Allows the user to change the background color of the view.
 */
#define APP_SPATIALDATAVIEW_BACKGROUND_COLOR_ACTION "APP_SPATIALDATAVIEW_BACKGROUND_COLOR_ACTION"

/**
 *  Toggles the display of the crosshair marker in the center of the view.
 */
#define APP_SPATIALDATAVIEW_CROSSHAIR_ACTION "APP_SPATIALDATAVIEW_CROSSHAIR_ACTION"

/**
 *  Default menu separator.
 */
#define APP_SPATIALDATAVIEW_CROSSHAIR_SEPARATOR_ACTION "APP_SPATIALDATAVIEW_CROSSHAIR_SEPARATOR_ACTION"

/**
 *  Hides all selected layers in the Session Explorer.\   This action only
 *  appears when multiple layers in the same view are selected.
 */
#define APP_SPATIALDATAVIEW_HIDE_LAYERS_ACTION "APP_SPATIALDATAVIEW_HIDE_LAYERS_ACTION"

/**
 *  Activates the layer in the view.
 */
#define APP_SPATIALDATAVIEW_LAYER_ACTIVATE_ACTION "APP_SPATIALDATAVIEW_LAYER_ACTIVATE_ACTION"

/**
 *  Default menu separator.
 */
#define APP_SPATIALDATAVIEW_LAYER_ACTIVATE_SEPARATOR_ACTION "APP_SPATIALDATAVIEW_LAYER_ACTIVATE_SEPARATOR_ACTION"

/**
 *  Creates a new layer of a user-defined type and deletes the original layer.
 */
#define APP_SPATIALDATAVIEW_LAYER_CONVERT_ACTION "APP_SPATIALDATAVIEW_LAYER_CONVERT_ACTION"

/**
 *  Copies the layer in the view and queries the user for a new name for the
 *  layer.
 */
#define APP_SPATIALDATAVIEW_LAYER_COPY_ACTION "APP_SPATIALDATAVIEW_LAYER_COPY_ACTION"

/**
 *  Removes the layer from the view and destroys it.
 */
#define APP_SPATIALDATAVIEW_LAYER_DELETE_ACTION "APP_SPATIALDATAVIEW_LAYER_DELETE_ACTION"

/**
 *  Default menu separator.
 */
#define APP_SPATIALDATAVIEW_LAYER_DELETE_SEPARATOR_ACTION "APP_SPATIALDATAVIEW_LAYER_DELETE_SEPARATOR_ACTION"

/**
 *  A separate submenu action is created for each layer in the view.\   Using
 *  the \#define generates a unique ID string for the specific layer.
 */
#define APP_SPATIALDATAVIEW_LAYER_MENU_ACTION(pLayer) \
   pLayer != NULL ? std::string("APP_SPATIALDATAVIEW_LAYER_MENU_ACTION : ") + pLayer->getId() : \
   "APP_SPATIALDATAVIEW_LAYER_MENU_ACTION"

/**
 *  Default menu separator.
 */
#define APP_SPATIALDATAVIEW_LAYER_SEPARATOR_ACTION "APP_SPATIALDATAVIEW_LAYER_SEPARATOR_ACTION"

/**
 *  Invokes the properties dialog to edit the Measurements layer properties.
 */
#define APP_SPATIALDATAVIEW_MEASUREMENTS_PROPERTIES_ACTION "APP_SPATIALDATAVIEW_MEASUREMENTS_PROPERTIES_ACTION"

/**
 *  Creates a new, empty layer in the view.
 */
#define APP_SPATIALDATAVIEW_NEW_LAYER_MENU_ACTION "APP_SPATIALDATAVIEW_NEW_LAYER_MENU_ACTION"

/**
 *  Toggles the display of the origin location marker in the view.
 */
#define APP_SPATIALDATAVIEW_ORIGIN_ACTION "APP_SPATIALDATAVIEW_ORIGIN_ACTION"

/**
 *  Indicates the image corner location of the data origin.
 */
#define APP_SPATIALDATAVIEW_ORIGIN_MENU_ACTION "APP_SPATIALDATAVIEW_ORIGIN_MENU_ACTION"

/**
 *  Default menu separator.
 */
#define APP_SPATIALDATAVIEW_ORIGIN_SEPARATOR_ACTION "APP_SPATIALDATAVIEW_ORIGIN_SEPARATOR_ACTION"

/**
 *  Default menu separator.
 */
#define APP_SPATIALDATAVIEW_PROPERTIES_SEPARATOR_ACTION "APP_SPATIALDATAVIEW_PROPERTIES_SEPARATOR_ACTION"

/**
 *  Select all layers in the Session Explorer which are of the same type as any currently
 *  selected layer.
 */
#define APP_SPATIALDATAVIEW_SELECT_LAYERS_OF_TYPE_ACTION "APP_SPATIALDATAVIEW_SELECT_LAYERS_OF_TYPE_ACTION"

/**
 *  Shows all selected layers in the Session Explorer.\   This action only
 *  appears when multiple layers in the same view are selected.
 */
#define APP_SPATIALDATAVIEW_SHOW_LAYERS_ACTION "APP_SPATIALDATAVIEW_SHOW_LAYERS_ACTION"

/**
 *  Toggles whether pixel edges in the image are smoothed together when drawn
 *  in the image.
 */
#define APP_SPATIALDATAVIEW_SMOOTH_ACTION "APP_SPATIALDATAVIEW_SMOOTH_ACTION"

/**
 *  Hides the toolbar.\   This action only appears in the context menu
 *  if the toolbar is shown.
 */
#define APP_TOOLBAR_HIDE_ACTION "APP_TOOLBAR_HIDE_ACTION"

/**
 *  Displays the toolbar.\   This action only appears in the context menu
 *  if the toolbar is hidden.
 */
#define APP_TOOLBAR_SHOW_ACTION "APP_TOOLBAR_SHOW_ACTION"

/**
 *  Display the window on top of all other workspace windows.
 */
#define APP_WORKSPACEWINDOW_ACTIVATE_ACTION "APP_WORKSPACEWINDOW_ACTIVATE_ACTION"

#endif
