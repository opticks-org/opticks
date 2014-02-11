/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDVIEW_H
#define POINTCLOUDVIEW_H

#include "ColorMap.h"
#include "EnumWrapper.h"
#include "PerspectiveView.h"

/**
 * Enum representing the ordering of raw point cloud data.
 */
enum PointColorizationTypeEnum
{
   POINT_HEIGHT=0,       /**< The points are colored based on height/Z values. */
   POINT_INTENSITY,      /**< The points are colored based on intensity values. */
   POINT_CLASSIFICATION, /**< The points are colored based on classification values. */
   POINT_AUX,            /**< The points are colored based on auxiliary information. */
   POINT_USER=64         /**< The point colorization is based on user auxiliary data. This is for future expansion and is not currently supported. */
};

/**
 * @EnumWrapper ::PointColorizationTypeEnum.
 */
typedef EnumWrapper<PointColorizationTypeEnum> PointColorizationType;

class PointCloudElement;

/**
 * A view to display point clouds.
 *
 * A point cloud view displays point cloud data in 3 dimensions. A view
 * has an associated data set which is used to populate the display.
 *
 * The point cloud view defines the following mouse modes, where the name given is the
 * name populated by MouseMode::getName():
 *  - PanMode
 *  - RotateMode
 *  - ZoomInMode
 *  - ZoomOutMode
 *  - ZoomBoxMode
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - Everything else documented in PerspectiveView.
 *  - set* methods are called
 */
class PointCloudView : public PerspectiveView
{
public:
   /**
    * Mutator to attach a point cloud to the view.
    *
    * This should be called only once.
    *
    * @param pPointCloud
    *        The point cloud to display in the view.
    * @return True if successful, false if pPointCloud is \c NULL or there is
    *         already an attached point cloud.
    * @notify  This method will notify Subject::signalModified.
    */
   virtual bool setPrimaryPointCloud(PointCloudElement* pPointCloud) = 0;

   /**
    * Accessor for the displayed point cloud.
    *
    * @return The dislayed point cloud or \c NULL if none is displayed.
    */
   virtual PointCloudElement* getPrimaryPointCloud() = 0;

   /**
    * Set the colorization mode.
    *
    * @param type
    *        The method of colorization.
    * @notify  This method will notify Subject::signalModified.
    */
   virtual void setPointColorizationType(PointColorizationType type) = 0;

   /**
    * Access the colorization mode.
    * 
    * Defaults to height.
    *
    * @return the colorization mode.
    */
   virtual PointColorizationType getPointColorizationType() const = 0;

   /**
    * Set the decimation factor.
    *
    * @param decimation
    *        The new decimation factor.
    * @notify  This method will notify Subject::signalModified.
    */
   virtual void setDecimation(uint32_t decimation) = 0;

   /**
    * Access the decimation factor.
    *
    * This is the number of points to skip for every displayed point.
    * Defaults to 0 (display all points).
    *
    * @return the decimation factor.
    */
   virtual uint32_t getDecimation() const = 0;

   /**
    * Set the size of displayed points.
    *
    * @param pointsize
    *        The new point size. Must be >= 1
    * @notify  This method will notify Subject::signalModified.
    */
   virtual void setPointSize(float pointsize) = 0;

   /**
    * Access the size of displayed points.
    *
    * This is the number of screen pixels used to display
    * each point. If point smoothing is available on the
    * graphics device, these will be smoothed spheres. Otherwise
    * they will be cubes.
    * Defaults to 1
    *
    * @return the point size.
    */
   virtual float getPointSize() const = 0;

   /**
    * Set the lower stretch value.
    *
    * @param value
    *        The new lower stretch value.
    * @notify  This method will notify Subject::signalModified.
    */
   virtual void setLowerStretch(double value) = 0; 

   /**
    * Access the lower stretch value.
    *
    * Represents the lower end of the colormap or color ramp.
    * This should include the scale and offset. (i.e. it is not a raw value)
    * Defaults to the minimum data value.
    *
    * @return the lower stretch value.
    */
   virtual double getLowerStretch() const = 0;

   /**
    * Set the upper stretch value.
    *
    * @param value
    *        The new upper stretch value.
    * @notify  This method will notify Subject::signalModified.
    */
   virtual void setUpperStretch(double value) = 0;

   /**
    * Access the upper stretch value.
    *
    * Represents the upper end of the colormap or color ramp.
    * This should include the scale and offset. (i.e. it is not a raw value)
    * Defaults to the maximum data value.
    *
    * @return the upper stretch value.
    */
   virtual double getUpperStretch() const = 0;

   /**
    * Set the display color map.
    *
    * @param colorMap
    *        The new color map.
    * @notify  This method will notify Subject::signalModified.
    */
   virtual void setColorMap(const ColorMap& colorMap) = 0;
   
   /**
    * Access the display color map.
    *
    * This will be used to colorize the data points when the view
    * is in colormap mode.
    * Defaults to an empty/invalid color map.
    *
    * @return the display color map.
    */
   virtual const ColorMap& getColorMap() const = 0;

   /**
    * Set the color for the lower end of the color ramp.
    *
    * @param lower
    *        The color for the lower end of the color ramp.
    * @notify  This method will notify Subject::signalModified.
    */
   virtual void setLowerStretchColor(ColorType lower) = 0;

   /**
    * Set the color for the upper end of the color ramp.
    *
    * @param upper
    *        The color for the upper end of the color ramp.
    * @notify  This method will notify Subject::signalModified.
    */
   virtual void setUpperStretchColor(ColorType upper) = 0;

   /**
    * Set the color ramp colors.
    *
    * This is a single call which is equivalent to calling both
    * setLowerStretchColor() and setUpperStretchColor().
    *
    * When the display is in color stetch mode, a gradient
    * will be created between the lower and upper colors.
    *
    * @param lower
    *        The color for the lower end of the color ramp.
    * @param upper
    *        The color for the upper end of the color ramp.
    */
   virtual void setColorStretch(ColorType lower, ColorType upper) = 0;

   /**
    * Access the lower color ramp color.
    *
    * @return the lower color ramp color.
    */
   virtual ColorType getLowerStretchColor() const = 0;

   /**
    * Access the upper color ramp color.
    *
    * @return the upper color ramp color.
    */
   virtual ColorType getUpperStretchColor()  const = 0;

   /**
    * Set the colorization color mode.
    *
    * @param usingMap
    *        If true, a color map will be used else a color ramp will be used.
    * @notify  This method will notify Subject::signalModified.
    */
   virtual void setUsingColorMap(bool usingMap) = 0;

   /**
    * Accessor for the colorization color mode.
    *
    * @return true if a color map is being used else false if a color ramp is being used.
    */
   virtual bool isUsingColorMap() const = 0;

   /**
    * Set the height/Z exaggeration multiplier.
    *
    * @param value
    *        The Z exaggeration multiplier.
    * @notify  This method will notify Subject::signalModified.
    */
   virtual void setZExaggeration(double value) = 0;

   /**
    * Access the height/Z exaggeration multiplier.
    *
    * All height/Z values will be multiplied by this value when determining Z location.
    * This will exaggerate height values. When colorizing by height, the color values are
    * unaffected but this multiplier.
    *
    * @return the Z exaggeration multiplier.
    */
   virtual double getZExaggeration() = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteView.
    */
   virtual ~PointCloudView() {}
};

#endif