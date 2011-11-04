/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef POINT_H
#define POINT_H

#include "ColorType.h"
#include "EnumWrapper.h"
#include "LocationType.h"
#include "PlotObject.h"
#include "TypesFile.h"

class PointSet;

/**
 *  A Point.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setLocation(), setSymbol(),
 *    setSymbolSize(), setColor(). 
 *  - Everything else documented in PlotObject.
 *
 *  @see     PlotObject
 */
class Point : public PlotObject
{
public:
   
   /**
   *  Pixel marker symbol for Point PlotObjects.
   *
   *  Selected pixels are displayed on the scene in various symbols so that the user can
   *  identify multiple selections on a single pixel.
   *

   */
   enum PointSymbolTypeEnum { 
      SOLID,                  /**< Filled box */
      X,                      /**< X */
      CROSS_HAIR,             /**< Crosshair */
      ASTERISK,               /**< Asterick */
      HORIZONTAL_LINE,        /**< Horizontal line */
      VERTICAL_LINE,          /**< Vertical line */
      FORWARD_SLASH,          /**< Forward slash */
      BACK_SLASH,             /**< Back slash */
      BOX,                    /**< Box */
      BOXED_X,                /**< Box with X */
      BOXED_CROSS_HAIR,       /**< Box with crosshair */
      BOXED_ASTERISK,         /**< Box with asterick */
      BOXED_HORIZONTAL_LINE,  /**< Box with horizontal line */
      BOXED_VERTICAL_LINE,    /**< Box with vertical line */
      BOXED_FORWARD_SLASH,    /**< Box with forward slash */
      BOXED_BACK_SLASH,       /**< Box with back slash */
      DIAMOND,                /**< Diamond */
      DIAMOND_FILLED,         /**< Filled Diamond */
      DIAMOND_CROSS_HAIR,     /**< Diamond with crosshair */
      TRIANGLE,               /**< Triangle */
      TRIANGLE_FILLED,        /**< Filled Triangle */
      RIGHT_TRIANGLE,         /**< Right Triangle */
      RIGHT_TRIANGLE_FILLED,  /**< Filled Right Triangle */
      LEFT_TRIANGLE,          /**< Left Triangle */
      LEFT_TRIANGLE_FILLED,   /**< Filled Left Triangle */
      DOWN_TRIANGLE,          /**< Down Triangle */
      DOWN_TRIANGLE_FILLED,   /**< Filled Down Triangle */
      CIRCLE,                 /**< Circle */
      CIRCLE_FILLED,          /**< Filled Circle */
      OCTAGON,                /**< Octagon */
      OCTAGON_FILLED,         /**< Filled Octagon */
      OCTAGON_CROSS_HAIR      /**< Octagon with crosshair */
   };

   /**
    * @EnumWrapper Point::PointSymbolTypeEnum.
    */
   typedef EnumWrapper<PointSymbolTypeEnum> PointSymbolType;

   /**
    *  Emitted with boost::any<LocationType> when the point's location changes.
    */
   SIGNAL_METHOD(Point, LocationChanged)
   /**
    *  Emitted with boost::any<PointSymbolType> when a point's symbol changes.
    */
   SIGNAL_METHOD(Point, SymbolChanged)
   /**
    *  Emitted with boost::any<int> when a point's symbol size changes.
    */
   SIGNAL_METHOD(Point, SymbolSizeChanged)
   /**
    *  Emitted with boost::any<ColorType> when a point's color changes.
    */
   SIGNAL_METHOD(Point, ColorChanged)

   /**
    *  Returns the x value
    *  
    *  @return  A double representation of the x value
    */
   virtual double getXLocation() const = 0;
   
   /**
    *  Returns the y value 
    *  
    *  @return  A double representation of the y value
    */
   virtual double getYLocation() const = 0;
   
   /**
    *  Returns the location of the point, both x and y in a LocationType structure
    *  
    *  @return  The LocationType structure
    *
    *  @see LocationType
    */
   virtual const LocationType& getLocation() const = 0;

   /**
    *  Returns the point symbol
    *  
    *  @return  The symbol type
    *
    *  @see PointSymbolType
    */
   virtual PointSymbolType getSymbol() const = 0;

   /**
    *  Returns the size of the symbol
    *  
    *  @return  The symbol size
    */
   virtual int getSymbolSize() const = 0;

   /**
    *  Returns the color of the point
    *  
    *  @return  The point color
    */
   virtual ColorType getColor() const = 0;

   /**
    *  Returns the PointSet that contains the point
    *  
    *  @return  The PointSet. Will return \c NULL if the Point is not in a PointSet.
    */
   virtual const PointSet* getPointSet() const = 0;

   /**
   *  Returns the PointSet that contains the point
   *  
   *  @return  The PointSet. Will return \c NULL if the Point is not in a PointSet.
   */
   virtual PointSet* getPointSet() = 0;

   /**
    *  Determines if this plot object resides at this point, most likely a mouse click
    *
    *  @param point  The location of the mouse click
    *  @return       True if the plot object is at this point
    */
   virtual bool hit(LocationType point) const = 0;

   /**
    *  Sets the location of the point
    *  
    *  @param location  The new location
    *
    *  @see LocationType
    *
    *  @notify  This method will notify signalLocationChanged with any<LocationType>.
    */
   virtual void setLocation(const LocationType& location) = 0;

   /**
    *  Sets the location of the point
    *  
    *  @param dX  The x value
    *  @param dY  The y value
    *
    *  @notify  This method will notify signalLocationChanged with any<LocationType>.
    */
   virtual void setLocation(double dX, double dY) = 0;

   /**
    *  Sets the point symbol
    *  
    *  @param eSymbol  The new symbol
    *
    *  @see PointSymbolType
    *
    *  @notify  This method will notify signalSymbolChanged() with boost::any<PointSymbolType>.
    */
   virtual void setSymbol(const PointSymbolType& eSymbol) = 0;

   /**
    *  Sets the point symbol size
    *  
    *  @param iSize  The new symbol size
    *
    *  @notify  This method will notify signalSymbolSizeChanged with any<int>.
    */
   virtual void setSymbolSize(int iSize) = 0;

   /**
    *  Sets the point symbol color
    *  
    *  @param clrSymbol  The new color
    *
    *  @notify  This method will notify signalColorChanged with any<ColorType>.
    */
   virtual void setColor(const ColorType& clrSymbol) = 0;

   /**
    *  Sets the PointSet of the Point
    *  
    *  @param pPointSet  The PointSet. Pass \c NULL to remove the Point from current PointSet.
    */
   virtual void setPointSet(PointSet* pPointSet) = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~Point() {}
};

/**
 * \cond INTERNAL
 * This template specialization is required to allow this type to be put into a DataVariant.
 */
template <> class VariantTypeValidator<Point::PointSymbolType> {};
/// \endcond

#endif
