/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LATLONLAYER_H
#define LATLONLAYER_H

#include "ColorType.h"
#include "ConfigurationSettings.h"
#include "Layer.h"
#include "LocationType.h"
#include "TypesFile.h"

#include <string>

class Font;

/**
 *  Adjusts the properties of a latitude/longitude layer.
 *
 *  A latitude/longitude layer consists of gridlines marking the location of equal
 *  latitude and equal longitude values.  The gridlines have four properties: color,
 *  style, width and tick spacing.  This class also contains a flag where the tick spacing
 *  will be automatically calculated for the current layer.  This class provides the
 *  means to set the properties for the current lat/long layer and also the default
 *  properties for new lat/long layers.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setLatLonFormat(), setColor(),
 *    setFont(), setStyle(), setWidth(), setTickSpacing(),
 *    setAutoTickSpacing(), setGeocoordType(), 
 *  - Everything else documented in Layer.
 *
 *  @see     Layer
 */
class LatLonLayer : public Layer
{
public:
   SETTING(Font, LatLonLayer, std::string, "")
   SETTING(FontSize, LatLonLayer, unsigned int, 8)
   SETTING(GridlineColor, LatLonLayer, ColorType, ColorType())
   SETTING(GridlineStyle, LatLonLayer, LatLonStyle, LATLONSTYLE_SOLID)
   SETTING(GridlineWidth, LatLonLayer, unsigned int, 0)
   SETTING(AlwaysExtrapolate, LatLonLayer, bool, false)

   /**
    *  Emitted with any<DmsFormatType> when the numeric format changes.
    */
   SIGNAL_METHOD(LatLonLayer, FormatChanged)
   /**
    *  Emitted with any<ColorType> when the grid color changes.
    */
   SIGNAL_METHOD(LatLonLayer, ColorChanged)
   /**
    *  Emitted with any<QFont> when the grid font changes.
    */
   SIGNAL_METHOD(LatLonLayer, FontChanged)
   /**
    *  Emitted with any<LatLonStyle> when the grid style changes.
    */
   SIGNAL_METHOD(LatLonLayer, StyleChanged)
   /**
    *  Emitted with any<unsigned int> when the grid line width changes.
    */
   SIGNAL_METHOD(LatLonLayer, WidthChanged)
   /**
    *  Emitted with any<LocationType> when the tick spacing changes.
    */
   SIGNAL_METHOD(LatLonLayer, TickSpacingChanged)
   /**
    *  Emitted with any<bool> when autocomputing of tick spacing changes.
    */
   SIGNAL_METHOD(LatLonLayer, AutoTickSpacingChanged)
   /**
    *  Emitted with any<GeocoordType> when the coordinate system changes.
    */
   SIGNAL_METHOD(LatLonLayer, CoordTypeChanged)

   /**
    *  Sets the line color for the current lat/long layer.
    *
    *  @param   colorType
    *           The new line color, which must be valid
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     getColor(), ColorType::isValid()
    */
   virtual void setColor(const ColorType& colorType) = 0;

   /**
    *  Returns the line color of the current lat/long layer.
    *
    *  @return  The current line color.
    *
    *  @see     setColor()
    */
   virtual ColorType getColor() const = 0;

   /**
    *  Sets the line style for the current lat/long layer.
    *
    *  @param   eStyle
    *           The new line style.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     getStyle()
    */
   virtual void setStyle(const LatLonStyle& eStyle) = 0;

   /**
    *  Returns the line style of the current lat/long layer.
    *
    *  @return  The current line style.
    *
    *  @see     setStyle()
    */
   virtual LatLonStyle getStyle() const = 0;

   /**
    *  Sets the line width for the current lat/long layer.
    *
    *  @param   width
    *           The new line width.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     getWidth()
    */
   virtual void setWidth(unsigned int width) = 0;

   /**
    *  Returns the line width of the current lat/long layer.
    *
    *  @return  The current line width.
    *
    *  @see     setWidth()
    */
   virtual unsigned int getWidth() const = 0;

   /**
    *  Sets the gridline tick spacing of the current lat/long layer.
    *
    *  The tick spacing defines the distance separating the gridlines.  This method sets
    *  both the latitude and longitude (X and Y) spacing.  Calling this method disables
    *  auto tick spacing.
    *
    *  @param   spacing
    *           The tick spacing in the X and Y dimensions as a LocationType.
    *           The spacing should be in the geocoordinate system specified in
    *           setGeocoordType().
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     getTickSpacing()
    */
   virtual void setTickSpacing(const LocationType& spacing) = 0;

   /**
    *  Returns the gridline tick spacing of the current lat/long layer.
    *
    *  @return  The tick spacing in the X and Y dimensions as a LocationType.
    *
    *  @see     setTickSpacing()
    */
   virtual LocationType getTickSpacing() const = 0;

   /**
    *  Turns on or off auto computed tick spacing.
    *
    *  This method sets the current lat/long layer to automatically calculate the
    *  tick spacing.
    *
    *  @param   bAutoSpacing
    *           TRUE to automatically calculate the tick spacing.  FALSE to use the
    *           manually set values.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     getAutoTickSpacing()
    */
   virtual void setAutoTickSpacing(bool bAutoSpacing) = 0;

   /**
    *  Returns the current state of auto computed tick spacing.
    *
    *  @return  TRUE if the layer is set to automatically calculate the tick spacing.
    *           FALSE if the layer uses the manually set values.
    *
    *  @see     setAutoTickSpacing()
    */
   virtual bool getAutoTickSpacing() const = 0;

   /**
    *  Sets the font for the latitude/longitude text.
    *
    *  @param   font
    *           The new latitude/longitude font.
    *
    *  @notify  This method will notify Subject::signalModified().
    */
   virtual void setFont(const Font& font) = 0;

   /**
    *  Returns read-only access to the latitude/longitude text font.
    *
    *  @return  The current latitude/longitude text font.  To modify the font
    *           values, call setFont() instead.
    */
   virtual const Font& getFont() const = 0;

   /**
    *  Sets the displayed geocoordinate system.
    *
    *  If auto spacing is not enabled, setTickSpacing() should be called to
    *  specify the tick spacing in the geocoordinate system.
    *
    *  @param   system
    *           The geocoordinate system used for display.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     GeocoordType
    */
   virtual void setGeocoordType(const GeocoordType& system) = 0;

   /**
    *  Returns the geocoordinate system used for display.
    *
    *  @return  The geocoordinate system.
    *
    *  @see     GeocoordType
    */
   virtual GeocoordType getGeocoordType() const = 0;

   /**
    *  Sets the display format for latitude/longitude values.
    *
    *  This method sets the display format for latitude longitude values.
    *  It can still be called when the current geocoordinate system is
    *  not latitude/longitude, and will take effect when the user updates
    *  the view to display latitude/longitude values.
    *
    *  @param   eFormat
    *           The latitude/longitude display format.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     DmsFormatType, getGeocoordType()
    */
   virtual void setLatLonFormat(const DmsFormatType& eFormat) = 0;

   /**
    *  Returns the display format of latitude/longitude values.
    *
    *  @return  The latitude/longitude display format.  This value will be
    *           valid even if the currently displayed geocoordinate
    *           system is not latitude/longitude.
    *
    *  @see     DmsFormatType, getGeocoordType()
    */
   virtual DmsFormatType getLatLonFormat() const = 0;

   /**
    *  Sets whether or not to always extrapolate lat/long grid lines.
    *
    *  This method sets the extrapolation format for latitude/longitude values.
    *  If extrapolation is true, the lat/long grid lines will always be
    *  drawn to the edge of the view regardless of the validity of the
    *  geo coordinates from the raster layer.
    *
    *  @param   bExtrapolate
    *           The latitude/longitude extrapolation value.
    *           True = Always draw grid lines to edge of view.
    *           False = Only draw grid lines where geo reference is valid.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     getExtrapolation()
    */
   virtual void setExtrapolation(bool bExtrapolate) = 0;

   /**
    *  Returns the extrapolation value of latitude/longitude grid lines.
    *  Default is false.
    *
    *  @return  The latitude/longitude extrapolation value.
    *           True = Always draws grid lines to edge of view.
    *           False = Only draws grid lines where geo reference is valid.
    *
    *  @see     setGeocoordType()
    */
   virtual bool getExtrapolation() const = 0;

protected:
   /**
    * This should be destroyed by calling SpatialDataView::deleteLayer.
    */
   virtual ~LatLonLayer() {}
};

#endif
