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
 *  Stops animation at the current frame.
 */
#define APP_ANIMATIONCONTROLLER_PAUSE_ACTION "APP_ANIMATIONCONTROLLER_PAUSE_ACTION"

/**
 *  Begins animation.
 */
#define APP_ANIMATIONCONTROLLER_PLAY_ACTION "APP_ANIMATIONCONTROLLER_PLAY_ACTION"

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
 *  Default menu separator.
 */
#define APP_ANIMATIONSERVICES_SEPARATOR_ACTION "APP_ANIMATIONSERVICES_SEPARATOR_ACTION"

/**
 *  Destroys the selected DataElement.
 */
#define APP_APPLICATIONWINDOW_DATAELEMENT_DELETE_ACTION "APP_APPLICATIONWINDOW_DATAELEMENT_DELETE_ACTION"

/**
 *  Export the selected SessionItem.
 */
#define APP_APPLICATIONWINDOW_EXPORT_ACTION "APP_APPLICATIONWINDOW_EXPORT_ACTION"

/**
 *  Displays properties of the selected SessionItem.
 */
#define APP_APPLICATIONWINDOW_PROPERTIES_ACTION "APP_APPLICATIONWINDOW_PROPERTIES_ACTION"

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
 *  displayed.
 */
#define APP_HISTOGRAMPLOT_RASTER_SEPARATOR_ACTION "APP_HISTOGRAMPLOT_RASTER_SEPARATOR_ACTION"

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
 *  Changes the contrast stretch type of a RasterLayer.
 */
#define APP_HISTOGRAMPLOT_STRETCH_TYPE_MENU_ACTION "APP_HISTOGRAMPLOT_STRETCH_TYPE_MENU_ACTION"

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
 *  A toggle action indicating whether the layer is currently displayed in its
 *  view.
 */
#define APP_LAYER_DISPLAYED_ACTION "APP_LAYER_DISPLAYED_ACTION"

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
 *  Sets the security markings on the plot.
 */
#define APP_PLOTVIEW_SECURITY_MARKINGS_ACTION "APP_PLOTVIEW_SECURITY_MARKINGS_ACTION"

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
 *  Toggles the display of major and minor polar gridlines.
 */
#define APP_POLARPLOT_GRIDLINES_MENU_ACTION "APP_POLARPLOT_GRIDLINES_MENU_ACTION"

/**
 *  Activates the layer in the view.
 */
#define APP_PRODUCTVIEW_LAYER_ACTIVATE_ACTION "APP_PRODUCTVIEW_LAYER_ACTIVATE_ACTION"

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
 *  Sets the display mode to RGB and sets the displayed bands based on the
 *  wavelengths.
 */
#define APP_RASTERLAYER_TRUE_COLOR_ACTION "APP_RASTERLAYER_TRUE_COLOR_ACTION"

/**
 *  Collapses all nodes in the current tree view.
 */
#define APP_SESSIONEXPLORER_COLLAPSE_ALL_ACTION "APP_SESSIONEXPLORER_COLLAPSE_ALL_ACTION"

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
 *  Shows all selected layers in the Session Explorer.\   This action only
 *  appears when multiple layers in the same view are selected.
 */
#define APP_SPATIALDATAVIEW_SHOW_LAYERS_ACTION "APP_SPATIALDATAVIEW_SHOW_LAYERS_ACTION"

/**
 *  Toggles whether pixel edges in the image are smoothed together when drawn
 *  in the image.
 */
#define APP_SPATIALDATAVIEW_SMOOTH_ACTION "APP_SPATIALDATAVIEW_SMOOTH_ACTION"

#endif
