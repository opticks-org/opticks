/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AOILAYER_H
#define AOILAYER_H

#include "GraphicLayer.h"
#include "ConfigurationSettings.h"
#include "TypesFile.h"

class ColorType;

/**
 *  Adjusts the properties of an AOI layer.
 *
 *  An AOI layer consists of markers identifying pixels in a scene that are included
 *  in the area of interest.  The pixel marker has two properties: color and symbol.
 *  This class provides the means to set the color and symbol for the current layer
 *  and also the default color and symbol for new layers.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setColor(), setSymbol().
 *  - The AOI is manipulated through the GUI.
 *  - Everything else documented in GraphicLayer.
 *
 *  @see     Layer
 */
class AoiLayer : public GraphicLayer
{
public:
   SETTING(AutoColor, AoiLayer, bool, false)
   SETTING(MarkerColor, AoiLayer, ColorType, ColorType())
   SETTING(MarkerSymbol, AoiLayer, SymbolType, SOLID)

   /**
    *  Emitted with any<ColorType> when the color is changed.
    */
   SIGNAL_METHOD(AoiLayer, ColorChanged)
   /**
    *  Emitted with any<SymbolType> when the symbol is changed.
    */
   SIGNAL_METHOD(AoiLayer, SymbolChanged)

   /**
    *  Sets the pixel marker color for the current AOI layer.
    *
    *  @param   aoiColor
    *           The new pixel marker color.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     getColor()
    */
   virtual void setColor(const ColorType& aoiColor) = 0;

   /**
    *  Returns the pixel marker color of the current AOI layer.
    *
    *  @return  The current pixel marker color.
    *
    *  @see     setColor()
    */
   virtual ColorType getColor() const = 0;

   /**
    *  Sets the pixel marker symbol for the current AOI layer.
    *
    *  @param   aoiSymbol
    *           The new pixel marker symbol.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     getSymbol()
    */
   virtual void setSymbol(const SymbolType& aoiSymbol) = 0;

   /**
    *  Returns the pixel marker symbol of the current AOI layer.
    *
    *  @return  The current pixel marker symbol.
    *
    *  @see     setSymbol()
    */
   virtual SymbolType getSymbol() const = 0;

   /**
    *  Sets the initial drawing mode that is used when the user adds a new
    *  object to the layer with the mouse.
    *
    *  This method provides a means for objects to set the initial object
    *  drawing mode for only this layer.  This value is reset when the user
    *  selects a new mode on the AOI toolbar.
    *
    *  @param   mode
    *           The initial drawing mode to use when the user adds a new object
    *           to the layer with the mouse.
    *
    *  @see     DesktopServices::setAoiSelectionTool()
    */
   virtual void setMode(ModeType mode) = 0;

   /**
    *  Returns the initial drawing mode that is used when the user adds a new
    *  object to the layer with the mouse.
    *
    *  @return  The initial drawing mode used when the user added a new object
    *           to the layer with the mouse.  This value may be different than
    *           the return value of DesktopServices::getAoiSelectionMode() if
    *           the setMode() method was called and the user has not selected a
    *           new mode on the toolbar.
    */
   virtual ModeType getMode() const = 0;

   /**
    * Correct the coordinate for whatever snapping may be required.
    *
    * AoiLayer snaps mid-pixel points so that objects appear in the
    * center of the selected pixels.
    *
    * @param coord
    *        Coordinate to correct.
    *
    * @return The corrected coordinate.
    */
   virtual LocationType correctCoordinate(const LocationType &coord) const = 0;

protected:
   /**
    * This should be destroyed by calling SpatialDataView::deleteLayer.
    */
   virtual ~AoiLayer() {}
};

#endif
