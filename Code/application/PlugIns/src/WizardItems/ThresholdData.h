/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef THRESHOLDDATA_H__
#define THRESHOLDDATA_H__

#include "DesktopItems.h"

class RasterElement;
class SpatialDataView;

class ThresholdData : public DesktopItems
{
public:
   ThresholdData();
   virtual ~ThresholdData();

   virtual bool setBatch();
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);

private:
   RasterElement* mpInputElement;
   SpatialDataView* mpView;
   double mFirstThreshold;
   double mSecondThreshold;
   PassArea mPassArea;
   RegionUnits mRegionUnits;
   unsigned int mDisplayBandNumber;
};

#endif
