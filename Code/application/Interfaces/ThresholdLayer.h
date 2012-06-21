/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef THRESHOLDLAYER_H
#define THRESHOLDLAYER_H

#include "ColorType.h"
#include "ConfigurationSettings.h"
#include "Layer.h"
#include "TypesFile.h"

/**
 *  Adjusts the properties of a threshold layer.
 *
 *  A threshold layer consists of markers identifying pixels in a scene that fall in a
 *  defined range of values.  To identify the marked pixels, the following properties
 *  are present: pass area, region units, and one or two threshold values.  The pass
 *  area defines how many threshold values are used and how they define the marked
 *  pixels.  The pixel marker is drawn with a symbol based on the AOI properties.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setRegionUnits(), setPassArea(),
 *    setFirstThreshold(), setSecondThreshold(), setSymbol(), and setColor().
 *  - Everything else documented in AoiLayer.
 *
 *  @see     AoiLayer
 */
class ThresholdLayer : public Layer
{
public:
   SETTING(AutoColor, ThresholdLayer, bool, false)
   SETTING(FirstValue, ThresholdLayer, double, 0.0)
   SETTING(MarkerColor, ThresholdLayer, ColorType, ColorType())
   SETTING(MarkerSymbol, ThresholdLayer, SymbolType, SOLID)
   SETTING(PassArea, ThresholdLayer, PassArea, LOWER)
   SETTING(SecondValue, ThresholdLayer, double, 0.0)
   SETTING(RegionUnits, ThresholdLayer, RegionUnits, RAW_VALUE)

   /**
    *  Emitted with boost::any<RegionUnits> when the units are changed.
    */
   SIGNAL_METHOD(ThresholdLayer, UnitsChanged)

   /**
    *  Emitted with boost::any<PassArea> when the pass area is changed.
    */
   SIGNAL_METHOD(ThresholdLayer, PassAreaChanged)

   /**
    *  Emitted with boost::any<double> when the first threshold is changed.
    */
   SIGNAL_METHOD(ThresholdLayer, FirstThresholdChanged)

   /**
    *  Emitted with boost::any<double> when the second threshold is changed.
    */
   SIGNAL_METHOD(ThresholdLayer, SecondThresholdChanged)

   /**
    *  Emitted with boost::any<SymbolType> when the symbol is changed.
    */
   SIGNAL_METHOD(ThresholdLayer, SymbolChanged)

   /**
    *  Emitted with boost::any<ColorType> when the color is changed.
    */
   SIGNAL_METHOD(ThresholdLayer, ColorChanged)

   /**
    *  Sets the first threshold value for the current threshold layer.
    *
    *  The first threshold value is used as the sole threshold value for the LOWER
    *  and UPPER pass areas.  It is also used as the lower threshold value for the
    *  MIDDLE and OUTSIDE pass areas.
    *
    *  @param   dRawValue
    *           The new threshold value as a raw value.
    *
    *  @notify  This method will notify signalFirstThresholdChanged() with boost::any<double>.
    *
    *  @see     getFirstThreshold(), setSecondThreshold()
    */
   virtual void setFirstThreshold(double dRawValue) = 0;

   /**
    *  Returns the first threshold value for the current threshold layer as a raw value.
    *
    *  @return  The threshold value as a raw value.
    *
    *  @see     setFirstThreshold(), getSecondThreshold()
    */
   virtual double getFirstThreshold() const = 0;

   /**
    *  Sets the second threshold value for the current threshold layer.
    *
    *  The second threshold value is used as the upper threshold value for the
    *  MIDDLE and OUTSIDE pass areas.  It is ignored for the LOWER and UPPER pass
    *  areas.
    *
    *  @param   dRawValue
    *           The new threshold value as a raw value.
    *
    *  @notify  This method will notify signalSecondThresholdChanged() with boost::any<double>.
    *
    *  @see     getSecondThreshold(), setFirstThreshold()
    */
   virtual void setSecondThreshold(double dRawValue) = 0;

   /**
    *  Returns the second threshold value for the current threshold layer as a raw value.
    *
    *  @return  The threshold value as a raw value.
    *
    *  @see     getFirstThreshold(), setSecondThreshold()
    */
   virtual double getSecondThreshold() const = 0;

   /**
    *  Sets the pass area for the current threshold layer.
    *
    *  @param   eArea
    *           The new pass area.
    *
    *  @notify  This method will notify signalPassAreaChanged() with boost::any<PassArea>.
    *
    *  @see     getPassArea()
    */
   virtual void setPassArea(const PassArea& eArea) = 0;

   /**
    *  Returns the pass area of the current threshold layer.
    *
    *  @return  The current pass area.
    *
    *  @see     setPassArea()
    */
   virtual PassArea getPassArea() const = 0;

   /**
    *  Sets the threshold units for the current threshold layer.
    *
    *  @param   eUnits
    *           The new threshold units.
    *
    *  @notify  This method will notify signalTypeChanged() with boost::any<RegionUnits>.
    *
    *  @see     getRegionUnits()
    */
   virtual void setRegionUnits(const RegionUnits& eUnits) = 0;

   /**
    *  Returns the threshold units of the current threshold layer.
    *
    *  @return  The current threshold units.
    *
    *  @see     setRegionUnits()
    */
   virtual RegionUnits getRegionUnits() const = 0;

   /**
    *  Converts a threshold value from one threshold units to another.
    *
    *  @param   eUnits
    *           The initial threshold units.
    *  @param   dThreshold
    *           The threshold value to convert.
    *  @param   eNewUnits
    *           The threshold units for the converted value.
    *
    *  @return  The converted threshold value.  If an error occurred, 0.0 is returned.
    */
   virtual double convertThreshold(const RegionUnits& eUnits, double dThreshold,
      const RegionUnits& eNewUnits) = 0;

   /**
    *  Converts a threshold value from the current threshold units to another.
    *
    *  @param   dThreshold
    *           The threshold value to convert.
    *  @param   eNewUnits
    *           The threshold units for the converted value.
    *
    *  @return  The converted threshold value.  If an error occurred, 0.0 is returned.
    */
   virtual double convertThreshold(double dThreshold, const RegionUnits& eNewUnits) = 0;

   /**
    *  Retrieves the layer's current symbol style.
    *
    *  @return  The current symbol style.
    */
   virtual SymbolType getSymbol() const = 0;

   /**
    *  Sets the layer's current symbol style.
    *
    *  @param   symbol
    *           The new symbol style.
    *
    *  @notify  This method will notify signalSymbolChanged() with boost::any<SymbolType>.
    */
   virtual void setSymbol(SymbolType symbol) = 0;

   /**
    *  Retrieves the layer's current color.
    *
    *  @return  The layer's current color.
    */
   virtual ColorType getColor() const = 0;

   /**
    *  Sets the layer's color.
    *
    *  @param   color
    *           The new color.
    *
    *  @notify  This method will notify signalColorChanged() with boost::any<ColorType>.
    */
   virtual void setColor(const ColorType &color) = 0;

protected:
   /**
    * This should be destroyed by calling SpatialDataView::deleteLayer.
    */
   virtual ~ThresholdLayer() {}
};

#endif
