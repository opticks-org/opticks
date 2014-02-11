/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */


#include "AppVerify.h"
#include "PointCloudDataDescriptor.h"
#include "PointCloudDataRequestImp.h"

#include <limits>

PointCloudDataRequestImp::PointCloudDataRequestImp() :
   mStartX(std::numeric_limits<double>::quiet_NaN()),
   mStopX(std::numeric_limits<double>::quiet_NaN()),
   mStartY(std::numeric_limits<double>::quiet_NaN()),
   mStopY(std::numeric_limits<double>::quiet_NaN()),
   mStartZ(std::numeric_limits<double>::quiet_NaN()),
   mStopZ(std::numeric_limits<double>::quiet_NaN()),
   mbWritable(false)
{
}

PointCloudDataRequestImp::~PointCloudDataRequestImp()
{
}

PointCloudDataRequestImp::PointCloudDataRequestImp(const PointCloudDataRequestImp& rhs) :
   mStartX(rhs.mStartX),
   mStopX(rhs.mStopX),
   mStartY(rhs.mStartY),
   mStopY(rhs.mStopY),
   mStartZ(rhs.mStartZ),
   mStopZ(rhs.mStopZ),
   mbWritable(rhs.mbWritable)
{
}

PointCloudDataRequest *PointCloudDataRequestImp::copy() const
{
   return new PointCloudDataRequestImp(*this);
}

bool PointCloudDataRequestImp::validate(const PointCloudDataDescriptor *pDescriptor) const
{
   if (pDescriptor == NULL)
   {
      return false;
   }

   if (mStartX < pDescriptor->getXMin() ||
       mStopX > pDescriptor->getXMax())
   {
      return false;
   }
   if (mStartY < pDescriptor->getYMin() ||
       mStopY > pDescriptor->getYMax())
   {
      return false;
   }
   if (mStartZ < pDescriptor->getZMin() ||
       mStopZ > pDescriptor->getZMax())
   {
      return false;
   }

   return true;
}

bool PointCloudDataRequestImp::polish(const PointCloudDataDescriptor *pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return false;
   }

   // NOTE: doube x; x != x   is true iff x is NaN
   // X
   if (mStartX != mStartX)
   {
      mStartX = pDescriptor->getXMin();
   }
   if (mStopX != mStopX)
   {
      mStopX = pDescriptor->getXMax();
   }

   // Y
   if (mStartY != mStartY)
   {
      mStartY = pDescriptor->getYMin();
   }
   if (mStopY != mStopY)
   {
      mStopY = pDescriptor->getYMax();
   }

   // Z
   if (mStartZ != mStartZ)
   {
      mStartZ = pDescriptor->getZMin();
   }
   if (mStopZ != mStopZ)
   {
      mStopZ = pDescriptor->getZMax();
   }

   return true;
}

int PointCloudDataRequestImp::getRequestVersion(const PointCloudDataDescriptor *pDescriptor) const
{
   return 1;
}

double PointCloudDataRequestImp::getStartX() const
{
   return mStartX;
}

double PointCloudDataRequestImp::getStopX() const
{
   return mStopX;
}

double PointCloudDataRequestImp::getStartY() const
{
   return mStartY;
}

double PointCloudDataRequestImp::getStopY() const
{
   return mStopY;
}

double PointCloudDataRequestImp::getStartZ() const
{
   return mStartZ;
}

double PointCloudDataRequestImp::getStopZ() const
{
   return mStopZ;
}

void PointCloudDataRequestImp::setBoundingBox(double startX, double stopX, double startY, double stopY, double startZ, double stopZ)
{
   mStartX = startX;
   mStopX = stopX;
   mStartY = startY;
   mStopY = stopY;
   mStartZ = startZ;
   mStopZ = stopZ;
}

void PointCloudDataRequestImp::setStartX(double val)
{
   mStartX = val;
}

void PointCloudDataRequestImp::setStopX(double val)
{
   mStopX = val;
}

void PointCloudDataRequestImp::setStartY(double val)
{
   mStartY = val;
}

void PointCloudDataRequestImp::setStopY(double val)
{
   mStopY = val;
}

void PointCloudDataRequestImp::setStartZ(double val)
{
   mStartZ = val;
}

void PointCloudDataRequestImp::setStopZ(double val)
{
   mStopZ = val;
}

bool PointCloudDataRequestImp::getWritable() const
{
   return mbWritable;
}

void PointCloudDataRequestImp::setWritable(bool writable)
{
   mbWritable = writable;
}
