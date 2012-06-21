/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef GCPLAYER_H
#define GCPLAYER_H

#include "ColorType.h"
#include "Layer.h"
#include "ConfigurationSettings.h"
#include "TypesFile.h"

/**
 *  Adjusts the properties of a GCP layer.
 *
 *  A GCP layer consists of markers identifying locations in a scene that are used as
 *  ground control points.  The marker has three properties: color, symbol, and symbol
 *  size.  The size of the marker represents the number of pixels in diameter that the
 *  marker covers.  This class provides the means to set the properties for the current
 *  GCP layer and also the default properties for new GCP layers.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setColor(), setSymbol(),
 *    setSymbolSize().
 *  - Everything else documented in Layer.
 *
 *  @see     Layer
 */
class GcpLayer : public Layer
{
public:
   SETTING(MarkerColor, GcpLayer, ColorType, ColorType())
   SETTING(MarkerSize, GcpLayer, unsigned int, 0)
   SETTING(MarkerSymbol, GcpLayer, GcpSymbol, GCP_X)

   /**
    *  Emitted with any<ColorType> when a member element is externally deleted.
    */
   SIGNAL_METHOD(GcpLayer, ColorChanged)
   /**
    *  Emitted with any<SymbolType> when a member element is externally deleted.
    */
   SIGNAL_METHOD(GcpLayer, SymbolChanged)
   /**
    *  Emitted with any<int> when a member element is externally deleted.
    */
   SIGNAL_METHOD(GcpLayer, SizeChanged)

   /**
    *  Sets the marker color for the current GCP layer.
    *
    *  @param   colorType
    *           The new marker color.
    *
    *  @notify  This method will notify signalColorChanged with any<ColorType>.
    *
    *  @see     getColor()
    */
   virtual void setColor(const ColorType& colorType) = 0;

   /**
    *  Returns the marker color of the current GCP layer.
    *
    *  @return  The current marker color.
    *
    *  @see     setColor()
    */
   virtual ColorType getColor() const = 0;

   /**
    *  Sets the marker symbol for the current GCP layer.
    *
    *  @param   eSymbol
    *           The new marker symbol.
    *
    *  @notify  This method will notify signalSymbolChanged with any<SymbolType>.
    *
    *  @see     getSymbol()
    */
   virtual void setSymbol(const GcpSymbol& eSymbol) = 0;

   /**
    *  Returns the marker symbol of the current GCP layer.
    *
    *  @return  The current marker symbol.
    *
    *  @see     setSymbol()
    */
   virtual GcpSymbol getSymbol() const = 0;

   /**
    *  Sets the marker symbol size for the current GCP layer.
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
    *  Returns the marker symbol size of the current GCP layer.
    *
    *  @return  The current marker symbol size.
    *
    *  @see     setSymbolSize()
    */
   virtual int getSymbolSize() const = 0;

protected:
   /**
    * This should be destroyed by calling SpatialDataView::deleteLayer.
    */
   virtual ~GcpLayer() {}
};

#endif
