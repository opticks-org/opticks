/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDDATADESCRIPTOR_H
#define POINTCLOUDDATADESCRIPTOR_H

#include "DataDescriptor.h"

/**
 * Enum representing the ordering of raw point cloud data.
 */
enum PointCloudArrangementEnum
{
   POINT_ARRAY=0,          /**< The points are represented as a 1-D array of points. The specific order is defined by the importer or creating plugin */
   POINT_KDTREE_XY_ARRAY,  /**< The points are represented as a K-d tree indexed by 2-D (x,y) location */
   POINT_KDTREE_XYZ_ARRAY, /**< The points are represented as a K-d tree indexed by 3-D (x,y,z) location */
   USER=64                 /**< The points use another, user-defined representation. For example: an R* tree representation.
                                User-defined representations may not be supported by PointCloudAccessor and other support classes
                                so care should be used when selecting such an arrangement. */
};

/**
 * @EnumWrapper ::PointCloudArrangementEnum.
 */
typedef EnumWrapper<PointCloudArrangementEnum> PointCloudArrangement;

/**
 * A description of point cloud data.
 */
class PointCloudDataDescriptor : public DataDescriptor
{
public:
   /**
    * Access the number of points in the element.
    *
    * @return the number of points.
    */
   virtual uint32_t getPointCount() const = 0;

   /**
    * Set the number of points in an element.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param pointTotal
    *        The new number of points.
    */
   virtual void setPointCount(uint32_t pointTotal) = 0;

   /**
    * Access the point arrangement in the element.
    *
    * @return the arrangement of points.
    */
   virtual PointCloudArrangement getArrangement() const = 0;

   /**
    * Set the arrangement of points in an element.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param arrangement
    *        The new arrangement of points.
    */
   virtual void setArrangement(PointCloudArrangement arrangement) = 0;

   /**
    * Access the scale to apply to raw X values.
    *
    * This is applied prior to X offset.
    *
    * @return the X scale.
    */
   virtual double getXScale() const = 0;

   /**
    * Set the scale to apply to raw X values.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param scale
    *        The new X scale.
    */
   virtual void setXScale(double scale) = 0;

   /**
    * Access the scale to apply to raw Y values.
    *
    * This is applied prior to Y offset.
    *
    * @return the Y scale.
    */
   virtual double getYScale() const = 0;

   /**
    * Set the scale to apply to raw Y values.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param scale
    *        The new Y scale.
    */
   virtual void setYScale(double scale) = 0;

   /**
    * Access the scale to apply to raw Z values.
    *
    * This is applied prior to Z offset.
    *
    * @return the Z scale.
    */
   virtual double getZScale() const = 0;

   /**
    * Set the scale to apply to raw Z values.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param scale
    *        The new Z scale.
    */
   virtual void setZScale(double scale) = 0;

   /**
    * Access the offset to apply to raw X values.
    *
    * This is applied after the X scale.
    *
    * @return the X offset.
    */
   virtual double getXOffset() const = 0;

   /**
    * Set the offset to apply to raw X values.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param offset
    *        The new X offset.
    */
   virtual void setXOffset(double offset) = 0;

   /**
    * Access the offset to apply to raw Y values.
    *
    * This is applied after the Y scale.
    *
    * @return the Y offset.
    */
   virtual double getYOffset() const = 0;

   /**
    * Set the offset to apply to raw Y values.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param offset
    *        The new Y offset.
    */
   virtual void setYOffset(double offset) = 0;

   /**
    * Access the offset to apply to raw Z values.
    *
    * This is applied after the Z scale.
    *
    * @return the Z offset.
    */
   virtual double getZOffset() const = 0;

   /**
    * Set the offset to apply to raw Z values.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param offset
    *        The new Z offset.
    */
   virtual void setZOffset(double offset) = 0;

   /**
    * Access the minimum X value.
    *
    * This is the raw minimum without scale and offset applied.
    *
    * @return the minimum X value in the data.
    */
   virtual double getXMin() const = 0;

   /**
    * Set the minimum X value.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param min
    *        The new minmum X value in the data.
    */
   virtual void setXMin(double min) = 0;

   /**
    * Access the minimum Y value.
    *
    * This is the raw minimum without scale and offset applied.
    *
    * @return the minimum Y value in the data.
    */
   virtual double getYMin() const = 0;

   /**
    * Set the minimum Y value.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param min
    *        The new minmum Y value in the data.
    */
   virtual void setYMin(double min) = 0;

   /**
    * Access the minimum Z value.
    *
    * This is the raw minimum without scale and offset applied.
    *
    * @return the minimum Z value in the data.
    */
   virtual double getZMin() const = 0;

   /**
    * Set the minimum Z value.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param min
    *        The new minmum Z value in the data.
    */
   virtual void setZMin(double min) = 0;

   /**
    * Access the maximum X value.
    *
    * This is the raw maximum without scale and offset applied.
    *
    * @return the maximum X value in the data.
    */
   virtual double getXMax() const = 0;

   /**
    * Set the maximum X value.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param max
    *        The new maximum X value in the data.
    */
   virtual void setXMax(double max) = 0;

   /**
    * Access the maximum Y value.
    *
    * This is the raw maximum without scale and offset applied.
    *
    * @return the maximum Yvalue in the data.
    */
   virtual double getYMax() const = 0;

   /**
    * Set the maximum Y value.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param max
    *        The new maximum Y value in the data.
    */
   virtual void setYMax(double max) = 0;

   /**
    * Access the maximum Z value.
    *
    * This is the raw maximum without scale and offset applied.
    *
    * @return the maximum Z value in the data.
    */
   virtual double getZMax() const = 0;

   /**
    * Set the maximum Z value.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param max
    *        The new maximum Z value in the data.
    */
   virtual void setZMax(double max) = 0;

   /**
    * Access the data encoding for spatial location values. (x, y, z)
    *
    * @return the encoding type for spatial locations.
    */
   virtual EncodingType getSpatialDataType() const = 0;

   /**
    * Set the data encoding for spatial location values. (x, y, z)
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param type
    *        The new data encoding for spatial location values.
    */
   virtual void setSpatialDataType(EncodingType type) = 0;

   /**
    * Does the data contain intensity values?
    *
    * @return true if the data contains intensity values.
    */
   virtual bool hasIntensityData() const = 0;

   /**
    * Set the intensity data flag.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param intensityPresent
    *        The new intensity flag value.
    */
   virtual void setHasIntensityData(bool intensityPresent) = 0;

   /**
    * Access the data encoding for intensity data.
    *
    * @return the encoding type for intensity data. If intensity data
    *         is not present, this is undefined.
    */
   virtual EncodingType getIntensityDataType() const = 0;

   /**
    * Set the data encoding for intensity data
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param type
    *        The new data encoding for intensity values.
    */
   virtual void setIntensityDataType(EncodingType type) = 0;

   /**
    * Does the data contain classification values?
    *
    * @return true if the data contains classification values.
    */
   virtual bool hasClassificationData() const = 0;

   /**
    * Set the classification data flag.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param classificationPresent
    *        The new classification flag value.
    */
   virtual void setHasClassificationData(bool classificationPresent) = 0;

   /**
    * Access the data encoding for classification data.
    *
    * @return the encoding type for classification data. If classification data
    *         is not present, this is undefined.
    */
   virtual EncodingType getClassificationDataType() const = 0;

   /**
    * Set the data encoding for classification data
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param type
    *        The new data encoding for classification values.
    */
   virtual void setClassificationDataType(EncodingType type) = 0;

   /**
    * Access the total size in bytes of a point structure.
    *
    * @return the number of bytes in a single point structure.
    */
   virtual size_t getPointSizeInBytes() const = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyDataDescriptor.
    */
   virtual ~PointCloudDataDescriptor() {}
};

#endif
