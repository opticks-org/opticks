/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUD_ACCESSOR_IMPL_H
#define POINTCLOUD_ACCESSOR_IMPL_H

#include "AppConfig.h"
#include "ObjectResource.h"
#include "PointCloudDataDescriptor.h"
#include "PointCloudDataRequest.h"
#include "PointCloudElement.h"
#include "PointCloudPager.h"
#include "PointDataBlock.h"
#include "RasterUtilities.h"
#include "TypesFile.h"
#include <exception>
#include <stdexcept>

typedef double (*convertToDoublePC)(const void*, double scale, double offset);
typedef int64_t (*convertToIntegerPC)(const void*, double scale, double offset);

/**
 * Provides a generic interface to the dataset.
 *
 * This class provides an efficient, generic interface to the raw dataset.  To
 * provide efficient access to the raw data, range checking can not occur
 * within this class.  It is up to the user of the PointCloudAccessor to make
 * sure the bounds of the dataset are not exceeded.  This class provides
 * point ordered access to the data. Interleave is determined by the point cloud
 * pager or importer. Therefore, if a special ordering such as Kd-tree depth-first
 * is required, the raw data should be structured in this ordering.
 *
 * This class is not instantiated by a plug-in directly.  The plug-in developer
 * can get access to this class through the PointCloudElement::getPointCloudAccessor() 
 * method.
 *
 * @see      PointCloudElement::getPointCloudAccessor()
 */
class PointCloudAccessorImpl
{
public:
   /**
    * Constructor (not called by the developer directly, use PointCloudElement::getPointCloudAccessor() instead)
    *
    * @param pElement
    *        The associated PointCloudElement
    * @param pRequest
    *        The PointCloudDataRequest object. PointCloudAccessorImpl takes ownership of this object. Must not be \c NULL
    * @throws std::logic_error if pElement is \c NULL
    */
   PointCloudAccessorImpl(PointCloudElement* pElement, PointCloudDataRequest* pRequest) :
      mpRequest(pRequest),
      mbValid(false),
      mpRawData(NULL),
      mpPointElement(pElement),
      mpDataBlock(NULL),
      mNumPointsInOriginalRequest(300000), // For double x,y,z,intensity and 2-byte classification, this is just short of 10mb blocks
      mCurrentPoint(0),
      mPointsInBlock(0),
      mPointByteOffset(0),
      mCurrentPointOfBlockStart(0),
      mPointSize(0),
      mXOffset(0),
      mYOffset(0),
      mZOffset(0),
      mIdOffset(0),
      mValidPointOffset(0),
      mIntensityOffset(0),
      mClassificationOffset(0),
      mSpatialConvertToDoubleFunc(NULL),
      mSpatialConvertToIntegerFunc(NULL),
      mIConvertToDoubleFunc(NULL),
      mIConvertToIntegerFunc(NULL),
      mCConvertToDoubleFunc(NULL),
      mCConvertToIntegerFunc(NULL),
      mHdrXScale(0.),
      mHdrXOffset(0.),
      mHdrYScale(0.),
      mHdrYOffset(0.),
      mHdrZScale(0.),
      mHdrZOffset(0.)
   {
      if (mpPointElement == NULL)
      {
         throw std::logic_error("PointCloudAccessorImpl constructor received NULL pointer");
      }
      PointCloudDataDescriptor* pDescriptor = dynamic_cast<PointCloudDataDescriptor*>(mpPointElement->getDataDescriptor());
      if (pDescriptor == NULL)
      {
         return;
      }
      PointCloudPager* pPager = mpPointElement->getPager();
      if (pPager == NULL)
      {
         return;
      }
      mPointSize = static_cast<unsigned char>(pDescriptor->getPointSizeInBytes());
      mXOffset = 0; // only one supported interleave ordering right now
      mYOffset = RasterUtilities::bytesInEncoding(pDescriptor->getSpatialDataType());
      mZOffset = mYOffset + RasterUtilities::bytesInEncoding(pDescriptor->getSpatialDataType());
      mIdOffset = mZOffset + RasterUtilities::bytesInEncoding(pDescriptor->getSpatialDataType());
      mValidPointOffset = mIdOffset + sizeof(PointCloudElement::pointIdType);
      unsigned int optionalDataOffset = mValidPointOffset + sizeof(PointCloudElement::validPointType);
      if (pDescriptor->hasIntensityData())
      {
         mIntensityOffset = optionalDataOffset;
         optionalDataOffset = mIntensityOffset + RasterUtilities::bytesInEncoding(pDescriptor->getIntensityDataType());;
      }
      if (pDescriptor->hasClassificationData())
      {
         mClassificationOffset = optionalDataOffset;
      }
      uint32_t numPoints = mNumPointsInOriginalRequest;
      uint32_t maxPoints = mpPointElement->getArrayCount();
      if (numPoints > maxPoints)
      {
         numPoints = maxPoints;
      }
      if (pPager != NULL)
      {
         mpDataBlock = pPager->getPointBlock(mCurrentPointOfBlockStart, numPoints, mpRequest.get());
      }
      if (mpDataBlock != NULL)
      {
         mpRawData = reinterpret_cast<char*>(mpDataBlock->getRawData());
         mPointsInBlock = mpDataBlock->getNumPoints();
         mbValid = mpRawData != NULL;
      }
   }

   /**
    *  PointCloudAccessorImpl destructor.
    */
   ~PointCloudAccessorImpl()
   {
      if (mpPointElement && mpDataBlock)
      {
         PointCloudPager* pPager = mpPointElement->getPager();
         if (pPager != NULL)
         {
            pPager->releasePointBlock(mpDataBlock);
         }
      }
   }

   /**
    *  Returns the PointCloudElement associated with this PointCloudAccessor.
    *
    *  @return  Returns the PointCloudElement associated with this class.
    */
   inline PointCloudElement* getAssociatedPointCloudElement()
   {
      return mpPointElement;
   }

   /**
    * Iterate to the next point in the underlying raw data order.
    *
    * If there is no next point, the accessor will become invalid.
    *
    * @throws std::logic_error if data pointers become corrupted.
    */
   inline void nextPoint()
   {
      ++mCurrentPoint;
      mPointByteOffset += mPointSize;
      updateIfNeeded();
   }

   /**
    * Iterate to the previous point in the underlying raw data order.
    *
    * If there is no previous point, the accessor will become invalid.
    *
    * @throws std::logic_error if data pointers become corrupted.
    */
   inline void previousPoint()
   {
      --mCurrentPoint;
      mPointByteOffset -= mPointSize;
      updateIfNeeded();
   }

   /**
    * Jump to the point at the specified index.
    *
    * Bounds checking is not performed on the index so the
    * calling routine should ensure it is a valid index.
    *
    * @param index
    *        The index (in the underlying raw data order) of the requested point.
    *
    * @throws std::logic_error if data pointers become corrupted.
    */
   inline void toIndex(uint32_t index)
   {
      mCurrentPoint = index - mCurrentPointOfBlockStart;
      mPointByteOffset = mCurrentPoint * mPointSize;
      updateIfNeeded();
   }

   /**
    * Iterate to the next valid point in the underlying raw data order.
    *
    * This will call nextPoint() until isPointValid() returns true.
    *
    * @throws std::logic_error if data pointers become corrupted.
    */
   inline void nextValidPoint()
   {
      nextPoint();
      while (isValid() && !isPointValid())
      {
         nextPoint();
      }
   }

   /**
    * Iterate to the previous valid point in the underlying raw data order.
    *
    * This will call previousPoint() until isPointValid() returns true.
    *
    * @throws std::logic_error if data pointers become corrupted.
    */
   inline void previousValidPoint()
   {
      previousPoint();
      while (isValid() && !isPointValid())
      {
         previousPoint();
      }
   }

   /**
    * Is the current point valid in the valid point data mask.
    * This also checks the validity of the current point offset by calling isValid().
    *
    * @return True if the point is valid, false otherwise.
    */
   inline bool isPointValid()
   {
      if (!isValid())
      {
         return false;
      }
      return *reinterpret_cast<bool*>(mpRawData + mPointByteOffset + mValidPointOffset);
   }

   /**
    * Mark the current point as valid or invalid in the valid point data mask.
    *
    * @param valid
    *        The new validity state for the current point.
    */
   inline void setPointValid(bool valid)
   {
      *reinterpret_cast<bool*>(mpRawData + mPointByteOffset + mValidPointOffset) = valid;
   }

   /**
    * Return the ID of the current point.
    *
    * @return the point's ID.
    */
   inline uint32_t getPointId()
   {
      return *reinterpret_cast<uint32_t*>(mpRawData + mPointByteOffset + mIdOffset);
   }

   /**
    * Set the ID of the current point.
    *
    * @param id
    *        The new point ID.
    */
   inline void setPointId(uint32_t id)
   {
      *reinterpret_cast<uint32_t*>(mpRawData + mPointByteOffset + mIdOffset) = id;
   }

   /**
    * Access a pointer to the raw X data for the current point.
    *
    * The data type can be determined by examining the PointCloudDataDescriptor.
    * No validity checking is performed. isValid() or isPointValid() should be
    * checked by the calling program.
    *
    * @return A pointer to the raw X data for the current point.
    */
   inline void* getRawX()
   {
      return mpRawData + mPointByteOffset + mXOffset;
   }

   /**
    * Access a pointer to the raw Y data for the current point.
    *
    * The data type can be determined by examining the PointCloudDataDescriptor.
    * No validity checking is performed. isValid() or isPointValid() should be
    * checked by the calling program.
    *
    * @return A pointer to the raw Y data for the current point.
    */
   inline void* getRawY()
   {
      return mpRawData + mPointByteOffset + mYOffset;
   }

   /**
    * Access a pointer to the raw Z data for the current point.
    *
    * The data type can be determined by examining the PointCloudDataDescriptor.
    * No validity checking is performed. isValid() or isPointValid() should be
    * checked by the calling program.
    *
    * @return A pointer to the raw Z data for the current point.
    */
   inline void* getRawZ()
   {
      return mpRawData + mPointByteOffset + mZOffset;
   }

   /**
    * Access a pointer to the raw intensity data for the current point.
    *
    * The data type can be determined by examining the PointCloudDataDescriptor.
    * No validity checking is performed. isValid() or isPointValid() should be
    * checked by the calling program.
    * If the data do not have intensity values, the returned value is undefined.
    *
    * @return A pointer to the raw intensity data for the current point.
    */
   inline void* getRawIntensity()
   {
      return mpRawData + mPointByteOffset + mIntensityOffset;
   }

   /**
    * Access a pointer to the raw classification data for the current point.
    *
    * The data type can be determined by examining the PointCloudDataDescriptor.
    * No validity checking is performed. isValid() or isPointValid() should be
    * checked by the calling program.
    * If the data do not have classification values, the returned value is undefined.
    *
    * @return A pointer to the raw classification data for the current point.
    */
   inline void* getRawClassification()
   {
      return mpRawData + mPointByteOffset + mClassificationOffset;
   }

   /**
    * Access the X datum for the current point as an integer.
    *
    * @param applyHeaderScaleOffset
    *        If true, the scale and offset defined in the PointCloudDataDescriptor will be applied to the returned value.
    * @return The X datum as an integer.
    */
   inline int64_t getXAsInteger(bool applyHeaderScaleOffset=false) const
   {
      return mSpatialConvertToIntegerFunc(mpRawData + mPointByteOffset + mXOffset,
                        applyHeaderScaleOffset ? mHdrXScale : 1.,
                        applyHeaderScaleOffset ? mHdrXOffset : 0.);
   }

   /**
    * Access the Y datum for the current point as an integer.
    *
    * @param applyHeaderScaleOffset
    *        If true, the scale and offset defined in the PointCloudDataDescriptor will be applied to the returned value.
    * @return The Y datum as an integer.
    */
   inline int64_t getYAsInteger(bool applyHeaderScaleOffset=false) const
   {
      return mSpatialConvertToIntegerFunc(mpRawData + mPointByteOffset + mYOffset,
                        applyHeaderScaleOffset ? mHdrYScale : 1.,
                        applyHeaderScaleOffset ? mHdrYOffset : 0.);
   }

   /**
    * Access the Z datum for the current point as an integer.
    *
    * @param applyHeaderScaleOffset
    *        If true, the scale and offset defined in the PointCloudDataDescriptor will be applied to the returned value.
    * @return The Z datum as an integer.
    */
   inline int64_t getZAsInteger(bool applyHeaderScaleOffset=false) const
   {
      return mSpatialConvertToIntegerFunc(mpRawData + mPointByteOffset + mZOffset,
                        applyHeaderScaleOffset ? mHdrZScale : 1.,
                        applyHeaderScaleOffset ? mHdrZOffset : 0.);
   }

   /**
    * Access the intensity datum for the current point as an integer.
    *
    * If the data do not have intensity values, the returned value is undefined.
    *
    * @return The intensity datum as an integer.
    */
   inline int64_t getIntensityAsInteger() const
   {
      return mIConvertToIntegerFunc(mpRawData + mPointByteOffset + mIntensityOffset, 1., 0.);
   }

   /**
    * Access the classification datum for the current point as an integer.
    *
    * If the data do not have classification values, the returned value is undefined.
    *
    * @return The classification datum as an integer.
    */
   inline int64_t getClassificationAsInteger() const
   {
      return mCConvertToIntegerFunc(mpRawData + mPointByteOffset + mClassificationOffset, 1., 0.);
   }

   /**
    * Access the X datum for the current point as a double.
    *
    * @param applyHeaderScaleOffset
    *        If true, the scale and offset defined in the PointCloudDataDescriptor will be applied to the returned value.
    * @return The X datum as a double.
    */
   inline double getXAsDouble(bool applyHeaderScaleOffset=false) const
   {
      return mSpatialConvertToDoubleFunc(mpRawData + mPointByteOffset + mXOffset,
                        applyHeaderScaleOffset ? mHdrXScale : 1.,
                        applyHeaderScaleOffset ? mHdrXOffset : 0.);
   }

   /**
    * Access the Y datum for the current point as a double.
    *
    * @param applyHeaderScaleOffset
    *        If true, the scale and offset defined in the PointCloudDataDescriptor will be applied to the returned value.
    * @return The Y datum as a double.
    */
   inline double getYAsDouble(bool applyHeaderScaleOffset=false) const
   {
      return mSpatialConvertToDoubleFunc(mpRawData + mPointByteOffset + mYOffset,
                        applyHeaderScaleOffset ? mHdrYScale : 1.,
                        applyHeaderScaleOffset ? mHdrYOffset : 0.);
   }

   /**
    * Access the Z datum for the current point as a double.
    *
    * @param applyHeaderScaleOffset
    *        If true, the scale and offset defined in the PointCloudDataDescriptor will be applied to the returned value.
    * @return The Z datum as a double.
    */
   inline double getZAsDouble(bool applyHeaderScaleOffset=false) const
   {
      return mSpatialConvertToDoubleFunc(mpRawData + mPointByteOffset + mZOffset,
                        applyHeaderScaleOffset ? mHdrZScale : 1.,
                        applyHeaderScaleOffset ? mHdrZOffset : 0.);
   }

   /**
    * Access the intensity datum for the current point as a double.
    *
    * If the data do not have intensity values, the returned value is undefined.
    *
    * @return The intensity datum as a double.
    */
   inline double getIntensityAsDouble() const
   {
      return mIConvertToDoubleFunc(mpRawData + mPointByteOffset + mIntensityOffset, 1., 0.);
   }

   /**
    * Access the classification datum for the current point as a double.
    *
    * If the data do not have classification values, the returned value is undefined.
    *
    * @return The classification datum as a double.
    */
   inline double getClassificationAsDouble() const
   {
      return mCConvertToDoubleFunc(mpRawData + mPointByteOffset + mClassificationOffset, 1., 0.);
   }

   /**
    *  Returns whether this is a valid point cloud accessor.
    *
    *  A valid point cloud accessor means that calls to getRawX(), etc. will point
    *  to a valid location in memory.  This method can be called between nextPoint() or previousPoint()
    *  and the getters for the value types.
    *
    *  @return  Returns whether this point cloud accessor is valid.
    *
    *  @see     PointCloudAccessor::isValid()
    */
   inline bool isValid() const 
   { 
      return mbValid; 
   }

   /**
    *  Increases the number of users of this point cloud accessor.
    *
    *  This is simple reference counting mechanism to track the number
    *  of users of the point cloud accessor.
    *
    *  @return  Returns the current number of users of this point cloud accessor.
    */
   inline int incrementRefCount() 
   { 
      mRefCount++;
      return mRefCount; 
   }

   /**
    *  Decreases the number of users of this point cloud accessor.
    *
    *  This is simple reference counting mechanism to track the number
    *  of users of the point cloud accessor.
    *
    *  @return  Returns the current number of users of this point cloud accessor.
    */
   inline int decrementRefCount()
   { 
      mRefCount--; 
      return mRefCount; 
   }

private:
   inline void updateIfNeeded()
   {
      if (mCurrentPoint >= mPointsInBlock)
      {
         if (mpPointElement == NULL)
         {
            throw std::logic_error("PointCloudAccessor back-pointer to point cloud has become corrupted");
         }
         PointCloudPager* pPager = mpPointElement->getPager();
         if (pPager == NULL)
         {
            throw std::logic_error("PointCloudAccessor back-pointer to point cloud pager has become corrupted");
         }
         pPager->releasePointBlock(mpDataBlock);
         uint32_t newStart = mCurrentPointOfBlockStart + mCurrentPoint;
         uint32_t numPoints = mNumPointsInOriginalRequest;
         uint32_t maxPoints = mpPointElement->getArrayCount();
         if (newStart + numPoints > maxPoints)
         {
            numPoints = maxPoints - newStart;
         }
         mpDataBlock = pPager->getPointBlock(newStart, numPoints, mpRequest.get());
         if (mpDataBlock != NULL)
         {
            mCurrentPointOfBlockStart = newStart;
            mPointsInBlock = mpDataBlock->getNumPoints();
            mpRawData = reinterpret_cast<char*>(mpDataBlock->getRawData());
            mbValid = true;
         }
         else
         {
            mCurrentPointOfBlockStart = 0;
            mPointsInBlock = 0;
            mpRawData = NULL;
            mbValid = false;
         }
         mCurrentPoint = 0;
         mPointByteOffset = 0;
      }
   }

   FactoryResource<PointCloudDataRequest> mpRequest;

   bool mbValid;
   char* mpRawData;
   PointCloudElement* mpPointElement;
   PointDataBlock* mpDataBlock;
   uint32_t mNumPointsInOriginalRequest; // the number of points requested in a data block
   uint32_t mCurrentPoint;
   uint32_t mPointsInBlock;
   size_t mPointByteOffset;

   uint32_t mCurrentPointOfBlockStart;

   unsigned char mPointSize;
   unsigned int mXOffset;
   unsigned int mYOffset;
   unsigned int mZOffset;
   unsigned int mIdOffset;
   unsigned int mValidPointOffset;
   unsigned int mIntensityOffset;
   unsigned int mClassificationOffset;

   int mRefCount;
   convertToDoublePC mSpatialConvertToDoubleFunc;
   convertToIntegerPC mSpatialConvertToIntegerFunc;
   convertToDoublePC mIConvertToDoubleFunc;
   convertToIntegerPC mIConvertToIntegerFunc;
   convertToDoublePC mCConvertToDoubleFunc;
   convertToIntegerPC mCConvertToIntegerFunc;

   double mHdrXScale;
   double mHdrXOffset;
   double mHdrYScale;
   double mHdrYOffset;
   double mHdrZScale;
   double mHdrZOffset;

   friend class PointCloudElementImp;
};

#endif
