/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef REGIONOBJECT_H
#define REGIONOBJECT_H

#include "ColorType.h"
#include "PlotObject.h"
#include "TypesFile.h"

#include <vector>

/**
 *  A shaded rectangular object.
 *
 *  The region plot object provides a shaded rectanuglar area with a
 *  transparency value.  The shading can be solid with a single color, or
 *  a gradient with multiple colors.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setRegion(), setColor(), setColors(),
 *    setTransparency(), setDrawBorder().
 *  - Everything else documented in PlotObject.
 *
 *  @see     PlotObject
 */
class RegionObject : public PlotObject
{
public:
   /**
    *  Emitted with any<boost::tuple<double,double,double,double> > when the 
    *          region boundary is changed. The values in the tuple are minX,
    *          minY, maxX and maxY respectively.
    */
   SIGNAL_METHOD(RegionObject, RegionChanged)
   /**
    *  Emitted with any<vector<ColorType> when the colors of the region are changed.
    */
   SIGNAL_METHOD(RegionObject, ColorsChanged)
   /**
    *  Emitted with any<int> when the opacity is changed. The value represents the
    *          opacity, ranging from 0 to 255.
    */
   SIGNAL_METHOD(RegionObject, TransparencyChanged)
   /**
    *  Emitted with any<bool> when the border is enabled or disabled.
    */
   SIGNAL_METHOD(RegionObject, BorderToggled)

   /**
    *  Queries whether the region has valid values.
    *
    *  A region, when it is first created, is invalid.  A region becomes valid
    *  when its values are set such that both the width and height are not zero.
    *
    *  @return  True if the region has valid values, or false if the values
    *           are invalid.
    *
    *  @see     setRegion()
    */
   virtual bool isValid() const = 0;

   /**
    *  Sets the region location.
    *
    *  This method sets the region location by specifying  the minimum and maximum
    *  coordinate extents.  The region can be obtained by getting the extents of
    *  the object.
    *
    *  @param   dMinX
    *           The new minimum x-coordinate value.
    *  @param   dMinY
    *           The new minimum y-coordinate value.
    *  @param   dMaxX
    *           The new maximum x-coordinate value.
    *  @param   dMaxY
    *           The new maximum y-coordinate value.
    *
    *  @notify  This method will notify signalRegionChanged with any<boost::tuple<double,double,double,double> >.
    *
    *  @see     PlotObject::getExtents()
    */
   virtual void setRegion(double dMinX, double dMinY, double dMaxX, double dMaxY) = 0;

   /**
    * Get the data coordinates for the region.
    *
    * @param minX
    *        The minimum x-coordinate.  This will set by this method.
    * @param minY
    *        The minimum y-coordinate.  This will set by this method.
    * @param maxX
    *        The maximum x-coordinate.  This will set by this method.
    * @param maxY
    *        The maximum y-coordinate.  This will set by this method.
    *
    * @return True if the operation was a success, false otherwise.
    */
   virtual bool getRegion(double &minX, double &minY, 
      double &maxX, double &maxY) const = 0;

   /**
    *  Queries whether a single, solid color is used.
    *
    *  @return  True if the region is using a single color, otherwise false.
    *
    *  @see     getColor()
    */
   virtual bool isSolidColor() const = 0;

   /**
    *  Sets the region to have a single, solid color.
    *
    *  @param   regionColor
    *           The new region color.  Must be a valid color.
    *
    *  @notify  This method will notify signalColorsChanged with vector<ColorType>.
    *           The vector will have only one color.
    *
    *  @see     ColorType::isValid()
    */
   virtual void setColor(const ColorType& regionColor) = 0;

   /**
    *  Set the region colors.
    *
    *  The colors will be drawn in order from the minimum x-coordinate extent
    *  to the maximum x-coordinate extent.
    *
    *  @param   colors
    *           The colors to be drawn, in order.
    *
    *  @notify  This method will notify signalColorsChanged with vector<ColorType>.
    */
   virtual void setColors(const std::vector<ColorType>& colors) = 0;

   /**
    *  Returns the region color.
    *
    *  This method returns the region color if the region is displayed as a
    *  single, solid color.
    *
    *  @return  The current region color.  The color is invalid if the region
    *           uses multiple colors.
    *
    *  @see     ColorType::isValid(), getColors()
    */
   virtual ColorType getColor() const = 0;

   /**
    *  Returns the region colors.
    *
    *  @return  A reference to vector containing the region colors, in order.
    *           The vector is valid if the region uses a single color.
    */
   virtual const std::vector<ColorType>& getColors() const = 0;

   /**
    *  Sets the region transparency value.
    *
    *  The region can be set to be transparent or semi-transparent by setting
    *  its transparency value.  Valid values range from 0 to 255, with the 0
    *  value being fully transparent and the 255 value being fully opaque.
    *  The default value is 255. When the region is displaying a colormap, this
    *  value will be applied as a multiplier on the transparency values of
    *  each value in the colormap.
    *
    *  @param   iTransparency
    *           The new transparency value.  Valid values range from 0 to 255.
    *
    *  @notify  This method will notify signalTransparencyChanged with any<int>.
    *
    *  @see   setColors()
    */
   virtual void setTransparency(int iTransparency) = 0;

   /**
    *  Returns the region transparency value.
    *
    *  @return  The current region transparency value.
    *
    *  @see     setTransparency()
    */
   virtual int getTransparency() const = 0;

   /**
    *  Toggles the region border.
    *
    *  This method sets whether a black border is drawn around the region.  The
    *  object defaults to not draw the border.
    *
    *  @param   bBorder
    *           Set this value to true to draw a black border around the object.
    *
    *  @notify  This method will notify signalBorderToggled with any<bool>.
    */
   virtual void setDrawBorder(bool bBorder) = 0;

   /**
    *  Queries whether the region border is drawn.
    *
    *  @return  True if the border is drawn, otherwise false.
    *
    *  @see     setDrawBorder()
    */
   virtual bool getDrawBorder() const = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~RegionObject() {}
};

#endif
