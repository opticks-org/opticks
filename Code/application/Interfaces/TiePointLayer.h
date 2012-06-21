/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef TIE_POINT_LAYER_H
#define TIE_POINT_LAYER_H

#include "ColorType.h"
#include "Layer.h"
#include "ConfigurationSettings.h"

/**
 *  Adjusts the properties of a tie point layer.
 *
 *  A tie point layer consists of markers identifying locations in a scene that are used as
 *  tie points between one scene and another.  The marker has four properties: color, symbol,
 *  symbol size and id state.  The size of the marker represents the number of pixels in
 *  diameter that the marker covers.  This class provides the means to set the properties for
 *  the current tie point layer.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setColor(), setSymbolSize(), 
 *    enableLabels().
 *  - Everything else documented in Layer.
 *
 *  @see     Layer
 */
class TiePointLayer : public Layer
{
public:
   SETTING(AutoColor, TiePointLayer, bool, false)
   SETTING(LabelEnabled, TiePointLayer, bool, false)
   SETTING(MarkerColor, TiePointLayer, ColorType, ColorType())
   SETTING(MarkerSize, TiePointLayer, unsigned int, 0)

   /**
    *  Emitted with any<ColorType> when the color is changed.
    */
   SIGNAL_METHOD(TiePointLayer, ColorChanged)
   /**
    *  Emitted with any<int> when the symbol size is changed.
    */
   SIGNAL_METHOD(TiePointLayer, SizeChanged)
   /**
    *  Emitted with any<bool> when labels are enabled or disabled.
    */
   SIGNAL_METHOD(TiePointLayer, LabelsEnabled)

   /**
    *  Sets the marker color for the current tie point layer.
    *
    *  @param   colorType
    *           The new marker color.
    *
    *  @notify  This method will signalColorChanged with any<ColorType>.
    *
    *  @see     getColor()
    */
   virtual void setColor(const ColorType& colorType) = 0;

   /**
    *  Returns the marker color of the current tie point layer.
    *
    *  @return  The current marker color.
    *
    *  @see     setColor()
    */
   virtual ColorType getColor() const = 0;

   /**
    *  Sets the marker symbol size for the current tie point layer.
    *
    *  Sets the marker symbol size for the current tie point layer.
    *  %Setting the size to 0 causes the symbol to degenerate to
    *  a single pixel.
    *
    *  @param   iSize
    *           The new marker symbol size.
    *
    *  @notify  This method will notify signalSizeChanged with any<int>.
    *
    *  @see     getSymbolSize()
    */
   virtual void setSymbolSize(int iSize) = 0;

   /**
    *  Returns the marker symbol size of the current tie point layer.
    *
    *  @return  The current marker symbol size.
    *
    *  @see     setSymbolSize()
    */
   virtual int getSymbolSize() const = 0;

   /**
    *  Specifies whether the tie point labels are displayed or not.
    *
    *  Specifies whether the tie points are labeled with their 
    *  numeric index or not.
    *
    *  @param   enabled
    *           If true, the tie points will be drawn with their 
    *           numeric labels adjacent, otherwise the labels
    *           will not be drawn.
    *
    *  @notify  This method will notify signalLabelsEnabled with any<bool>.
    *
    *  @see     areLabelsEnabled()
    */
   virtual void enableLabels(bool enabled) = 0;

   /**
    *  Indicates if tie point labels are being displayed
    *
    *  Indicates if the tie points are currently being
    *  labeled with their numeric indices.
    *
    *  @return  True if the tie points are being drawn with
    *           their numeric indices; false otherwise.
    *
    *  @see     enableLabels()
    */
   virtual bool areLabelsEnabled() const = 0;

   /**
    *  Indicates if the layer is on a mission image. 
    *
    *  Indicates if the tie point layer is on a mission image. This will
    *  determine if the mission offset is applied to the tie point coordinates
    *  when they are being drawn.
    *
    *  @return  True if the tie point layer is being drawn on a 
    *           mission image; false otherwise.
    *
    *  @see     setIsMission()
    */
   virtual bool isMission() const = 0;

   /**
    *  Specifies whether the layer is on a mission image or not.
    *
    *  Specifies if the tie point layer is on a mission image. This will
    *  determine if the mission offset is applied to the tie point coordinates
    *  when they are being drawn.
    *
    *  @param   bIsMission
    *           Specifies if the layer is on a mission image.
    *
    *  @see     isMission()
    */
   virtual void setIsMission(bool bIsMission) = 0;

protected:
   /**
    * This should be destroyed by calling SpatialDataView::deleteLayer.
    */
   virtual ~TiePointLayer() {}
};

#endif
