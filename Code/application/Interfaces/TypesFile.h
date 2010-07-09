/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef _TYPESFILE
#define _TYPESFILE

#include "DataVariantValidator.h"
#include "EnumWrapper.h"

/**
 *  Specifies animation behavior after the last frame in the animation controller is set.
 *
 *  @see     AnimationState, AnimationController
 */
enum AnimationCycleEnum
{
   PLAY_ONCE,  /**< The animation stops and the current frame is reset to the first frame.\   This is
                    the default cycle. */
   REPEAT,     /**< The current frame is reset to the first frame and the animation continues. */
   BOUNCE      /**< The current animation state changes to the opposite play direction and the animation
                    continues. */
};

/**
 * @EnumWrapper ::AnimationCycleEnum.
 */
typedef EnumWrapper<AnimationCycleEnum> AnimationCycle;

/**
 *  Identifies the current play state of the animation controller animation.
 *
 *  @see     AnimationCycle, AnimationController
 */
enum AnimationStateEnum
{
   STOP,          /**< The animation controller is not playing, and the current frame has been reset to the first
                       frame.\   This is the default state. */
   PLAY_FORWARD,  /**< The animation controller is playing and changing frames to the next frame in the frame
                       vector. */
   PLAY_BACKWARD, /**< The animation controller is playing and changing frames to the previous frame in the frame
                       vector. */
   PAUSE_FORWARD, /**< The animation controller is not playing, the current frame has not been reset to the
                       first frame and was running forward when paused. */
   PAUSE_BACKWARD /**< The animation controller is not playing, the current frame has not been reset to the
                       first frame and was running backward when paused. */
};

/**
 * @EnumWrapper ::AnimationStateEnum.
 */
typedef EnumWrapper<AnimationStateEnum> AnimationState;

/**
 *  Identifies the current AOI add mode on the AOI toolbar.
 */
enum AoiAddModeEnum
{
   APPEND_AOI, /**< New AOI shapes are appended to the current AOI. */
   REPLACE_AOI,  /**< The current AOI is erased before adding the new AOI shape. */
   NEW_AOI     /**< If the current AOI is empty, the shape is added. If it is
                             not empty, a new AOI is created and the shape is added to this. */
};

/**
 * @EnumWrapper ::AoiAddModeEnum.
 */
typedef EnumWrapper<AoiAddModeEnum> AoiAddMode;

/**
 *  Specifies the drawing format for annotation arc objects.
 */
enum ArcRegionEnum
{
   ARC_CENTER,    /**< The arc is filled and border line extends to the ellipical center of the arc. */
   ARC_CHORD,     /**< The arc is filled and the border extends from the start point to the stop point,
                       and back to the start point without passing through the center. */
   ARC_OPEN       /**< The arc is not filled, and the border line is not closed.\   A line is drawn
                       from the start point to the stop point. */
};

/**
 * @EnumWrapper ::ArcRegionEnum.
 */
typedef EnumWrapper<ArcRegionEnum> ArcRegion;

/**
 *  Available arrow styles
 *
 *  @see     Arrow
 */
enum ArrowStyleEnum { ARROW_NONE, ARROW_SMALL, ARROW_LARGE, ARROW_TRIANGLE_SMALL, ARROW_TRIANGLE_LARGE,
                  ARROW_TRIANGLE_SMALL_FILL, ARROW_TRIANGLE_LARGE_FILL, ARROW_DIAMOND, ARROW_DIAMOND_FILL };

/**
 * @EnumWrapper ::ArrowStyleEnum.
 */
typedef EnumWrapper<ArrowStyleEnum> ArrowStyle;

/**
 *  Available axes in a plot window.
 */
enum AxisPositionEnum { AXIS_LEFT = 0, AXIS_BOTTOM, AXIS_TOP, AXIS_RIGHT };

/**
 * @EnumWrapper ::AxisPositionEnum.
 */
typedef EnumWrapper<AxisPositionEnum> AxisPosition;

/**
 * Specifies the pixel origin (0, 0) location for the data cube.
 */
enum DataOriginEnum { LOWER_LEFT, UPPER_LEFT };

/**
 * @EnumWrapper ::DataOriginEnum.
 */
typedef EnumWrapper<DataOriginEnum> DataOrigin;

/**
 *  Specifies how a raster layer is drawn.
 */
enum DisplayModeEnum
{
   GRAYSCALE_MODE,   /**< A single data value is displayed through each of the red, green, and blue
                          color guns, thereby producing a gray appearance. */
   RGB_MODE          /**< A single data value is displayed through only one of the color guns.\   Three
                          data values can then be displayed simultaneously through each of the color
                          guns, creating a color composite. */
};

/**
 * @EnumWrapper ::DisplayModeEnum.
 */
typedef EnumWrapper<DisplayModeEnum> DisplayMode;

/**
 *  Specifies the distance units
 */
enum DistanceUnitsEnum
{
   NO_DISTANCE_UNIT,
   KILOMETER,
   MILE,
   NAUTICAL_MILE,
   METER,
   YARD,
   FOOT
};

/**
 * @EnumWrapper ::DistanceUnitsEnum.
 */
typedef EnumWrapper<DistanceUnitsEnum> DistanceUnits;

/**
 *  Specifies the display type for latitude/longitude values.
 */
enum DmsFormatTypeEnum
{
   DMS_FULL,            /**< Decimal seconds (DD° MM' SS.ss") */
   DMS_FULL_DECIMAL,    /**< Decimal degrees (DD.ddd) */
   DMS_MINUTES_DECIMAL  /**< Decimal minutes (DD° MM.mm') */
};

/**
 * @EnumWrapper ::DmsFormatTypeEnum.
 */
typedef EnumWrapper<DmsFormatTypeEnum> DmsFormatType;

/**
 *  The location of a DockWindow.
 */
enum DockWindowAreaTypeEnum
{
   DOCK_FLOATING,    /**< The window is not attached to the main application
                          window and can be positioned anywhere on the desktop. */
   DOCK_LEFT,        /**< The window is attached to the left side of the main
                          application window. */
   DOCK_RIGHT,       /**< The window is attached to the right side of the main
                          application window. */
   DOCK_TOP,         /**< The window is attached to the top of the main
                          application window. */
   DOCK_BOTTOM       /**< The window is attached to the bottom of the main
                          application window. */
};

/**
 * @EnumWrapper ::DockWindowAreaTypeEnum.
 */
typedef EnumWrapper<DockWindowAreaTypeEnum> DockWindowAreaType;

/**
 *  Data types for the data in a data set.
 *
 *  The EncodingType enumeration indicates the number of bytes in a single data
 *  value in addition to its positive/negative range.  Each enumerated value
 *  maps to a standard C++ data type.
 */
enum EncodingTypeEnum
{
   INT1SBYTE = 0,  /**< char */
   INT1UBYTE = 1,      /**< unsigned char */
   INT2SBYTES = 2,     /**< short */
   INT2UBYTES = 3,     /**< unsigned short */
   INT4SCOMPLEX = 4,   /**< complex short */
   INT4SBYTES = 5,     /**< int */
   INT4UBYTES = 6,     /**< unsigned int */
   FLT4BYTES = 7,      /**< float */
   FLT8COMPLEX = 8,    /**< complex float */
   FLT8BYTES = 9       /**< double */
};

/**
 * @EnumWrapper ::EncodingTypeEnum.
 */
typedef EnumWrapper<EncodingTypeEnum> EncodingType;

/**
 *  Specifies byte order for multi-byte data types.
 */
enum EndianTypeEnum
{
   BIG_ENDIAN_ORDER,       /**< The big endian type has the most significant byte first. */
   LITTLE_ENDIAN_ORDER     /**< The little endian type has the least significant byte first. */
};

/**
 * @EnumWrapper ::EndianTypeEnum.
 */
typedef EnumWrapper<EndianTypeEnum> EndianType;

/**
 *  Shading format for annotation objects.
 */
enum FillStyleEnum { SOLID_FILL, HATCH, EMPTY_FILL };

/**
 * @EnumWrapper ::FillStyleEnum.
 */
typedef EnumWrapper<FillStyleEnum> FillStyle;

/**
 *  Specifies the value of an animation frame that an animation controller cycles through.
 *
 *  @see     AnimationFrame, AnimationController, Animation
 */
enum FrameTypeEnum
{
   FRAME_ID,   /**< The animation is based on the frame number.\   Frame numbers typically range
                    from zero to one less than the number of total frames.\   Multiple frames in a
                    single animation should not contain the same frame number. */
   FRAME_TIME  /**< The animation is based on the time value.\   See the AnimationFrame documentation
                    for more details on the time value. */
};

/**
 * @EnumWrapper ::FrameTypeEnum.
 */
typedef EnumWrapper<FrameTypeEnum> FrameType;

/**
 *  Available symbols for ground control points.
 *
 *  A ground control point is marked on the display with either an X or a + enclosed
 *  by a circle.  Next to the marker is the text name of the GCP. The ground control 
 *  point can be hidden by selecting no draw for the symbol.
 */
enum GcpSymbolEnum { GCP_X, GCP_PLUS, GCP_NODRAW };

/**
 * @EnumWrapper ::GcpSymbolEnum.
 */
typedef EnumWrapper<GcpSymbolEnum> GcpSymbol;

/**
 *  Type of geocoordinate.
 *
 *  Georeferencing can be performed in Latitude/Longitude or UTM coordinates.
 */
enum GeocoordTypeEnum { GEOCOORD_GENERAL, GEOCOORD_LATLON, GEOCOORD_UTM, GEOCOORD_MGRS };

/**
 * @EnumWrapper ::GeocoordTypeEnum.
 */
typedef EnumWrapper<GeocoordTypeEnum> GeocoordType;

/**
 *  Alignment format when aligning annotation objects.
 */
enum GraphicAlignmentEnum { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT, ALIGN_TOP, ALIGN_MIDDLE, ALIGN_BOTTOM };

/**
 * @EnumWrapper ::GraphicAlignmentEnum.
 */
typedef EnumWrapper<GraphicAlignmentEnum> GraphicAlignment;

/**
 *  Direction in which to evenly distribute selected annotation objects.
 */
enum GraphicDistributionEnum { DISTRIBUTE_HORIZONTALLY, DISTRIBUTE_VERTICALLY };

/**
 * @EnumWrapper ::GraphicDistributionEnum.
 */
typedef EnumWrapper<GraphicDistributionEnum> GraphicDistribution;

/**
 *  Specifies the mode types of annotation objects available.
 */
enum GraphicModeTypeEnum { MOVE, ROTATE };

/**
 * @EnumWrapper ::GraphicModeTypeEnum.
 */
typedef EnumWrapper<GraphicModeTypeEnum> GraphicModeType;

/**
 *  Specifies the types of annotation objects available.
 *
 *  NOTE: enum value TRAIL_OBJECT should not be used by plug-in code. It is for internal Core
 *  use only by the OverviewWindow.
 */
enum GraphicObjectTypeEnum { LINE_OBJECT, TEXT_OBJECT, FRAME_LABEL_OBJECT, ARROW_OBJECT,
   RECTANGLE_OBJECT, TRIANGLE_OBJECT, ELLIPSE_OBJECT, ROUNDEDRECTANGLE_OBJECT,
   ARC_OBJECT, MOVE_OBJECT, ROTATE_OBJECT, SCALEBAR_OBJECT, GROUP_OBJECT,
   POLYLINE_OBJECT, POLYGON_OBJECT, CGM_OBJECT, RAW_IMAGE_OBJECT, FILE_IMAGE_OBJECT,
   WIDGET_IMAGE_OBJECT, LATLONINSERT_OBJECT, NORTHARROW_OBJECT, EASTARROW_OBJECT,
   VIEW_OBJECT, MULTIPOINT_OBJECT, MEASUREMENT_OBJECT, BITMASK_OBJECT, HLINE_OBJECT,
   VLINE_OBJECT, ROW_OBJECT, COLUMN_OBJECT, TRAIL_OBJECT };

/**
 * @EnumWrapper ::GraphicObjectTypeEnum.
 */
typedef EnumWrapper<GraphicObjectTypeEnum> GraphicObjectType;

/**
 *  Specifies how the inset zoom percentage is calculated based on the current view zoom percentage.
 */
enum InsetZoomModeEnum { ABSOLUTE_MODE, RELATIVE_MODE };

/**
 * @EnumWrapper ::InsetZoomModeEnum.
 */
typedef EnumWrapper<InsetZoomModeEnum> InsetZoomMode;

/**
 *  Specifies the data storage format.
 *
 *  The interleave format indicates how data values are stored in a file,
 *  as imported, or how to access the data values.
 */
enum InterleaveFormatTypeEnum
{
   BSQ,     /**< <b>Band Sequential</b> - The pixel values for an entire band are stored before the
                 pixel values for another band.\   This format also supports band values stored in
                 multiple files. */
   BIP,     /**< <b>Band Interleave by Pixel</b> - A single pixel value in one band is stored before
                 the value of the same pixel on another band is stored. */
   BIL      /**< <b>Band Interleave by Line</b> - A single row of pixel values in one band is stored
                 before the values of the same row on another band is stored. */
};

/**
 * @EnumWrapper ::InterleaveFormatTypeEnum.
 */
typedef EnumWrapper<InterleaveFormatTypeEnum> InterleaveFormatType;

/**
 *  The line style for a latitude/longitude layer.
 */
enum LatLonStyleEnum { LATLONSTYLE_SOLID, LATLONSTYLE_CROSS, LATLONSTYLE_NONE, LATLONSTYLE_DASHED };

/**
 * @EnumWrapper ::LatLonStyleEnum.
 */
typedef EnumWrapper<LatLonStyleEnum> LatLonStyle;

/**
 *  Supported layer types in the display.
 *
 *  The LayerType enumeration values list the layers available for display in a
 *  SpatialDataView.
 */
enum LayerTypeEnum { ANNOTATION, AOI_LAYER, CONTOUR_MAP, GCP_LAYER, GRAPHIC_LAYER, LAT_LONG,
                 RASTER, PSEUDOCOLOR, THRESHOLD, TIEPOINT_LAYER };

/**
 * @EnumWrapper ::LayerTypeEnum.
 */
typedef EnumWrapper<LayerTypeEnum> LayerType;

/**
 *  Line format for annotation objects.
 */
enum LineStyleEnum { SOLID_LINE, DASHED, DOT, DASH_DOT, DASH_DOT_DOT };

/**
 * @EnumWrapper ::LineStyleEnum.
 */
typedef EnumWrapper<LineStyleEnum> LineStyle;

/**
 * View link type.
 */
enum LinkTypeEnum
{
   NO_LINK,                /**< There is no link for this View. */
   AUTOMATIC_LINK,         /**< Linked views use GEOCOORD_LINK when georeferencing is available,
                                and MIRRORED_LINK when it is not. */
   MIRRORED_LINK,          /**< Linked views mirror actions.\   Panning, zooming, and rotating actions
                                are performed on each linked view. */
   GEOCOORD_LINK           /**< Linked views show the same geographic zoom and orientation. */
};

/**
 * @EnumWrapper ::LinkTypeEnum.
 */
typedef EnumWrapper<LinkTypeEnum> LinkType;

/**
 *  Identifies the reporting levels used by the message log manager.
 */
enum MessageReportingLevelEnum { ML_SILENT, ML_NORMAL, ML_VERBOSE };

/**
 * @EnumWrapper ::MessageReportingLevelEnum.
 */
typedef EnumWrapper<MessageReportingLevelEnum> MessageReportingLevel;

/**
 *  Identifies the type of message used in the suppressible message dialog.
 *
 *  @see    SuppressibleMsgDlg
 */
enum MessageTypeEnum 
{   
   MESSAGE_INFO,           /**< The message will tell the user some information. */
   MESSAGE_WARNING,        /**< The message will give the user a warning. */
   MESSAGE_ERROR           /**< The message will show the user an error. */
};

/**
 * @EnumWrapper ::MessageTypeEnum.
 */
typedef EnumWrapper<MessageTypeEnum> MessageType;

/**
 *  Pixel selection mode.
 *
 *  Used in combination with the ToolType pixel selection tools, the pixel mode defines
 *  the action when the user clicks a pixel in the scene.
 *
 *  @see     ToolType
 */
enum ModeTypeEnum
{
   DRAW,       /**< The selected pixel is always marked on the display. */
   ERASE,      /**< %Any marker on the selected pixel is always unmarked from the display. */
   TOGGLE,     /**< If the selected pixel is unmarked, it is marked.\   If the selected pixel
                    is marked, it is unmarked. */
   AOI_MOVE    /**< Move (and clone) vector AOI's. */
};

/**
 * @EnumWrapper ::ModeTypeEnum.
 */
typedef EnumWrapper<ModeTypeEnum> ModeType;

/**
 *  Specifies the direction in which an object is positioned.
 */
enum OrientationTypeEnum { HORIZONTAL, VERTICAL };

/**
 * @EnumWrapper ::OrientationTypeEnum.
 */
typedef EnumWrapper<OrientationTypeEnum> OrientationType;

/**
 *  Refresh rate when panning the scene in a View.
 */
enum PanModeTypeEnum
{
   PAN_DELAY,     /**< The scene is not refreshed until the user releases the mouse button. */
   PAN_INSTANT    /**< The scene refreshed constantly while the user moves the mouse over the window. */
};

/**
 * @EnumWrapper ::PanModeTypeEnum.
 */
typedef EnumWrapper<PanModeTypeEnum> PanModeType;

/**
 *  Identifies a region of data values references against one or two threshold values.
 */
enum PassAreaEnum { LOWER, UPPER, MIDDLE, OUTSIDE };

/**
 * @EnumWrapper ::PassAreaEnum.
 */
typedef EnumWrapper<PassAreaEnum> PassArea;

/**
 *  Available plot widgets in which to display plot data.
 *
 *  @see     PlotWidget
 */
enum PlotTypeEnum { CARTESIAN_PLOT, HISTOGRAM_PLOT, SIGNATURE_PLOT, POLAR_PLOT };

/**
 * @EnumWrapper ::PlotTypeEnum.
 */
typedef EnumWrapper<PlotTypeEnum> PlotType;

/**
 *  Available objects to display in a plot view.
 *
 *  @see     PlotView
 */
enum PlotObjectTypeEnum { CURVE = 0, CURVE_COLLECTION, HISTOGRAM, LOCATOR, REGION, POINT_OBJECT, POINT_SET, ARROW,
                      AXIS, CARTESIAN_GRIDLINES, TEXT_OBJECT_TYPE, PLOT_GROUP, POLAR_GRIDLINES,
                      POLYGON_OBJECT_TYPE };

/**
 * @EnumWrapper ::PlotObjectTypeEnum.
 */
typedef EnumWrapper<PlotObjectTypeEnum> PlotObjectType;

/**
 * How selection should operate in a plot.
 */
enum PlotSelectionModeTypeEnum
{
   NORMAL_SELECTION, /**< Ordinary selection, where clicking on a plot object
                      *   selects the entire object. */
   DEEP_SELECTION    /**< Deep selection, where clicking on a compound plot 
                      *   object selects the portion clicked on.  DEEP_SELECTION
                      *   currently only effects PointSet. */
};

/**
 * @EnumWrapper ::PlotSelectionModeTypeEnum.
 */
typedef EnumWrapper<PlotSelectionModeTypeEnum> PlotSelectionModeType;

/**
 *  Available callbacks for plug-ins.
 */
enum PlugInCallbackTypeEnum { BACKGROUND_COMPLETE };

/**
 * @EnumWrapper ::PlugInCallbackTypeEnum.
 */
typedef EnumWrapper<PlugInCallbackTypeEnum> PlugInCallbackType;

/**
 *  How symbol selection should appear in a plot.
 */
enum PointSelectionDisplayTypeEnum { 
   BOX_SELECTION,      /**< The point selection draws a box around the selected points. */
   INVERT_SELECTION,   /**< The point selection draws an inverted color symbol behind
                        *   the selected points. */
   SYMBOL_SELECTION    /**< The point selection draws the selected points by 
                        *   increasing the symbol size. */
};

/**
 * @EnumWrapper ::PointSelectionDisplayTypeEnum.
 */
typedef EnumWrapper<PointSelectionDisplayTypeEnum> PointSelectionDisplayType;

/**
 *  Available locations for objects in a plot widget.
 *
 *  @see     PlotWidget
 */
enum PositionTypeEnum { TOP_LEFT_BOTTOM_LEFT, TOP_LEFT_BOTTOM_RIGHT, TOP_RIGHT_BOTTOM_LEFT,
                    TOP_RIGHT_BOTTOM_RIGHT, CENTER };

/**
 * @EnumWrapper ::PositionTypeEnum.
 */
typedef EnumWrapper<PositionTypeEnum> PositionType;

/**
 *  Specifies how the cube data should be handled on load and how it should
 *  be accessed after loading.
 *
 *  While the ProcessingLocation influences whether an algorithm can write to
 *  the data, various other factors can effect writability as well.  The correct
 *  way to determine whether a RasterElement can be written to is to call 
 *  RasterElement::getDataAccessor() with a writable request.  If the DataAccessor
 *  returned is valid, the data may be written to.
 *
 *  @see     InterleaveFormatType
 */
enum ProcessingLocationEnum
{
   IN_MEMORY = 0,       /**< The cube data is loaded entirely into memory, and the data can be accessed
                             directly. */
   ON_DISK_READ_ONLY,   /**< The cube data is not loaded into memory.\   It is accessed by parsing the
                             original data from the disk.\   This can decrease processing performance,
                             but has the advantage of not using memory or additional disk space.\   The
                             cube data however is accessed in a read-only fashion.  Please see above for
                             information on how to determine if the data may be overwritten. */
   ON_DISK              /**< The cube data is not loaded into memory.\   It is accessed by parsing the
                             original data from the disk.\   This can decrease processing performance,
                             but has the advantage of not using memory or additional disk space.\   The
                             cube data however is accessed in a read/write fashion, so algorithms can
                             change the raw cube data.\   Please be aware that even though the cube is
                             opened in read/write fashion from disk, the original data file on the disk
                             will in no way be changed.\   Only the in memory representation of the data
                             being accessed from disk will be changed, and those changes must be kept the
                             entire time the cube is loaded into the application.\   It is up to the RasterPager
                             implementations to uphold this requirement.  Please see above for
                             information on how to determine if the data may be overwritten. */
};

/**
 * @EnumWrapper ::ProcessingLocationEnum.
 */
typedef EnumWrapper<ProcessingLocationEnum> ProcessingLocation;

/** 
 *  Possible raster layer display colors.
 *
 *  This type specifies an individual display color channel when displaying a raster layer.
 */
enum RasterChannelTypeEnum { GRAY, RED, GREEN, BLUE };

/**
 * @EnumWrapper ::RasterChannelTypeEnum.
 */
typedef EnumWrapper<RasterChannelTypeEnum> RasterChannelType;

/**
 *  Specifies the units of the upper and lower values for a region used for threshold layers
 *  and histogram stretch.
 */
enum RegionUnitsEnum { RAW_VALUE, PERCENTAGE, PERCENTILE, STD_DEV };

/**
 * @EnumWrapper ::RegionUnitsEnum.
 */
typedef EnumWrapper<RegionUnitsEnum> RegionUnits;

/**
 *  Application release configuration mode.
 */
enum ReleaseTypeEnum
{
   RT_NORMAL,     /**< The application is running in Normal mode. */
   RT_DEMO,       /**< The application is running in Demo mode (Not for Production Use). */
   RT_TRAINING,   /**< The application is running in Training mode (Not for Production Use). */
   RT_TEST,       /**< The application is running in Test mode (Not for Production Use). */
   RT_PROTO,      /**< The application is running in Prototype mode. */
   RT_RD          /**< The application is running in R&D mode. */
};

/**
 * @EnumWrapper ::ReleaseTypeEnum.
 */
typedef EnumWrapper<ReleaseTypeEnum> ReleaseType;

/**
 *  Specifies under what circumstances a session should be saved.
 */
enum SessionSaveTypeEnum
{
   SESSION_AUTO_SAVE = 0,  /**< The session is auto saved on a periodic basis. */
   SESSION_DONT_AUTO_SAVE, /**< The session is only saved when the user specifies that it should be. */
   SESSION_QUERY_SAVE      /**< The user is queried re: saving whenever a session is closed. */
};

/**
 * @EnumWrapper ::SessionSaveTypeEnum.
 */
typedef EnumWrapper<SessionSaveTypeEnum> SessionSaveType;

/**
 *  Specifies the type of stretch for raster layer display.
 */
enum StretchTypeEnum { LINEAR, LOGARITHMIC, EXPONENTIAL, EQUALIZATION };

/**
 * @EnumWrapper ::StretchTypeEnum.
 */
typedef EnumWrapper<StretchTypeEnum> StretchType;

/**
 *  Pixel marker symbol.
 *
 *  Selected pixels are displayed on the scene in various symbols so that the user can
 *  identify multiple selections on a single pixel.
 *
 *  NOTE: these are oredered so there is a relationship between boxed
 *        and unboxed such that X = BOXED_X - BOXED
 */
enum SymbolTypeEnum { SOLID, X, CROSS_HAIR, ASTERISK, HORIZONTAL_LINE,
                  VERTICAL_LINE, FORWARD_SLASH, BACK_SLASH,
                  BOX, BOXED_X, BOXED_CROSS_HAIR, BOXED_ASTERISK, BOXED_HORIZONTAL_LINE,
                  BOXED_VERTICAL_LINE, BOXED_FORWARD_SLASH, BOXED_BACK_SLASH};

/**
 * @EnumWrapper ::SymbolTypeEnum.
 */
typedef EnumWrapper<SymbolTypeEnum> SymbolType;

/**
 *  Available scale types for a plot.
 *
 *  A plot can either display in a linear or log scale.
 */
enum ScaleTypeEnum
{
   SCALE_LINEAR = 0,
   SCALE_LOG = 1
};

/**
 * @EnumWrapper ::ScaleTypeEnum.
 */
typedef EnumWrapper<ScaleTypeEnum> ScaleType;

/**
 * Specifies how textures should be drawn when a single texture
 * element is larger than a single screen pixel.
 */
enum TextureModeEnum {
   TEXTURE_LINEAR, /**< An interpolation calculation is performed,
                        please see the documentation for GL_LINEAR for
                        more details. */
   TEXTURE_NEAREST_NEIGHBOR /**< A nearest neighbor calcution is peformed,
                                 please see the documentation of GL_NEAREST
                                 for more details. */
};

/**
* @EnumWrapper ::TextureModeEnum.
*/
typedef EnumWrapper<TextureModeEnum> TextureMode;

/**
*  Specifies the type of tiling to use for workspace windows.
*/
enum TilingTypeEnum 
{ 
   TILE_GRID,        /**< arrange windows to fill the view in a two row grid.\   For an odd number of windows,
                          the odd window will span both rows in its column. */
   TILE_HORIZONTAL,  /**< arrange windows in one row with a full view height column for each window */
   TILE_VERTICAL     /**< arrange windows in one column with a full view width row for each window */
};

/**
 * @EnumWrapper ::TilingTypeEnum.
 */
typedef EnumWrapper<TilingTypeEnum> TilingType;

/**
 *  %System of units to use.
 *
 *  The UnitSystem enumeration indicates the system of units to use when displaying values
 *  to the user.
 *
 *  @see     Units
 */
enum UnitSystemEnum
{
   UNIT_KM = 100, /**< Use meters or kilometers as appropriate */
   UNIT_KFT, /**< Use feet or kilofeet (1000 feet) as appropriate */
   UNIT_MI /**< Use feet or miles as appropriate */
};

/**
 * @EnumWrapper ::UnitTypeEnum.
 */
typedef EnumWrapper<UnitSystemEnum> UnitSystem;

/**
 *  %Units of the values in a data set.
 *
 *  The UnitType enumeration indicates the format of values ins a data set.
 *  In addition to the available formats, custom types can be further
 *  distinguished by a units name.
 *
 *  @see     Units
 */
enum UnitTypeEnum { RADIANCE, REFLECTANCE, EMISSIVITY, DIGITAL_NO, CUSTOM_UNIT, REFLECTANCE_FACTOR,
                TRANSMITTANCE, ABSORPTANCE, ABSORBANCE, DISTANCE };

/**
 * @EnumWrapper ::UnitTypeEnum.
 */
typedef EnumWrapper<UnitTypeEnum> UnitType;

/**
 *  Result of plug-in validate() calls.
 */
enum ValidationResultTypeEnum
{
   VALIDATE_SUCCESS,       /**< Validation was successfull */
   VALIDATE_FAILURE,       /**< Validation was a complete failure. */
   VALIDATE_INFO,          /**< Information needs to be presented to the user. */
   VALIDATE_INPUT_REQUIRED /**< Success is possible with additional user input. */
};

/**
 * @EnumWrapper ::ValidationResultTypeEnum.
 */
typedef EnumWrapper<ValidationResultTypeEnum> ValidationResultType;

/**
 *  Available view types.
 */
enum ViewTypeEnum
{
   SPATIAL_DATA_VIEW,   /**< Displays one or more layers from a LayerList. */
   PRODUCT_VIEW,        /**< Provides a paper layout where annotation objects can be added to construct
                             a finished product. */
   PLOT_VIEW            /**< Displays a plot with two axes and an optional legend. */
};

/**
 * @EnumWrapper ::ViewTypeEnum.
 */
typedef EnumWrapper<ViewTypeEnum> ViewType;

/**
 *  %Units for spectral data wavelengths.
 *
 *  Wavelength values are defined and stored as microns in special metadata
 *  attributes in a DynamicObject.  However, they can be displayed to the user
 *  in several different units.
 *
 *  @see        \ref specialmetadata
 */
enum WavelengthUnitsTypeEnum
{
   MICRONS,
   NANOMETERS,
   INVERSE_CENTIMETERS
};

/**
 * @EnumWrapper ::WavelengthUnitsTypeEnum.
 */
typedef EnumWrapper<WavelengthUnitsTypeEnum> WavelengthUnitsType;

/**
 *  Specifies the default size for a WorkspaceWindow when it is displayed.
 */
enum WindowSizeTypeEnum
{
   FIXED_SIZE,             /**< The window is displayed at a specified size in
                                number of screen pixels. */
   MAXIMIZED,              /**< The window is maximized to fill the entire
                                workspace area in the main application window. */
   WORKSPACE_PERCENTAGE    /**< The window size is calculated as a percentage
                                of the total current workspace area in the main
                                application window. */
};

/**
 * @EnumWrapper ::WindowSizeTypeEnum.
 */
typedef EnumWrapper<WindowSizeTypeEnum> WindowSizeType;

/**
 *  Available window types.
 */
enum WindowTypeEnum
{
   WORKSPACE_WINDOW,       /**< A window that displays one or more views and can be minimized, maximized,
                                or float within the main application workspace area. */
   SPATIAL_DATA_WINDOW,    /**< A workspace window that contains an analysis view. */
   PRODUCT_WINDOW,         /**< A workspace window that contains a product view. */
   DOCK_WINDOW,            /**< A window that can contain a QWidget that can float or dock along the edges
                                of the main application window. */
   PLOT_WINDOW,            /**< A dock window providing the capability to create plots. */
   TOOLBAR                 /**< A dock window containing buttons and other widgets to perform certain actions. */
};

/**
 * @EnumWrapper ::WindowTypeEnum.
 */
typedef EnumWrapper<WindowTypeEnum> WindowType;

/**
 * \cond INTERNAL
 * These template specializations are required to allow these types to be put into a DataVariant.
 */
template <> class VariantTypeValidator<AnimationCycle> {};
template <> class VariantTypeValidator<AnimationState> {};
template <> class VariantTypeValidator<ArcRegion> {};
template <> class VariantTypeValidator<DataOrigin> {};
template <> class VariantTypeValidator<DisplayMode> {};
template <> class VariantTypeValidator<DistanceUnits> {};
template <> class VariantTypeValidator<DmsFormatType> {};
template <> class VariantTypeValidator<EncodingType> {};
template <> class VariantTypeValidator<EndianType> {};
template <> class VariantTypeValidator<FillStyle> {};
template <> class VariantTypeValidator<GcpSymbol> {};
template <> class VariantTypeValidator<GeocoordType> {};
template <> class VariantTypeValidator<GraphicObjectType> {};
template <> class VariantTypeValidator<InsetZoomMode> {};
template <> class VariantTypeValidator<InterleaveFormatType> {};
template <> class VariantTypeValidator<LatLonStyle> {};
template <> class VariantTypeValidator<LayerType> {};
template <> class VariantTypeValidator<LineStyle> {};
template <> class VariantTypeValidator<PassArea> {};
template <> class VariantTypeValidator<PlotObjectType> {};
template <> class VariantTypeValidator<PositionType> {};
template <> class VariantTypeValidator<ProcessingLocation> {};
template <> class VariantTypeValidator<SessionSaveType> {};
template <> class VariantTypeValidator<RasterChannelType> {};
template <> class VariantTypeValidator<RegionUnits> {};
template <> class VariantTypeValidator<ReleaseType> {};
template <> class VariantTypeValidator<StretchType> {};
template <> class VariantTypeValidator<SymbolType> {};
template <> class VariantTypeValidator<UnitSystem> {};
template <> class VariantTypeValidator<UnitType> {};
template <> class VariantTypeValidator<WavelengthUnitsType> {};
template <> class VariantTypeValidator<WindowSizeType> {};
template <> class VariantTypeValidator<WindowType> {};
/// \endcond

#endif
