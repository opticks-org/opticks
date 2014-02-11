/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDDATAREQUESTIMP_H
#define POINTCLOUDDATAREQUESTIMP_H

#include "PointCloudDataRequest.h"
#include "TypesFile.h"

class PointCloudDataRequestImp : public PointCloudDataRequest
{
public:
   PointCloudDataRequestImp();
   PointCloudDataRequestImp(const PointCloudDataRequestImp& rhs);
   ~PointCloudDataRequestImp();

   PointCloudDataRequest *copy() const;

   bool validate(const PointCloudDataDescriptor *pDescriptor) const;
   bool polish(const PointCloudDataDescriptor *pDescriptor);

   int getRequestVersion(const PointCloudDataDescriptor *pDescriptor) const;

   virtual double getStartX() const;
   virtual double getStopX() const;
   virtual double getStartY() const;
   virtual double getStopY() const;
   virtual double getStartZ() const;
   virtual double getStopZ() const;
   virtual void setStartX(double val);
   virtual void setStopX(double val);
   virtual void setStartY(double val);
   virtual void setStopY(double val);
   virtual void setStartZ(double val);
   virtual void setStopZ(double val);
   virtual void setBoundingBox(double startX, double stopX, double startY, double stopY, double startZ, double stopZ);

   virtual bool getWritable() const;
   virtual void setWritable(bool writable);

private:
   double mStartX;
   double mStopX;
   double mStartY;
   double mStopY;
   double mStartZ;
   double mStopZ;

   bool mbWritable;
};

#endif