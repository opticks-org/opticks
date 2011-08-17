/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ICETHRESHOLDLAYEREXPORTER_H
#define ICETHRESHOLDLAYEREXPORTER_H

#include "IceExporterShell.h"
#include "TypesFile.h"

class FileDescriptor;
class RasterElement;
class Statistics;
class ThresholdLayer;

class IceThresholdLayerExporter : public IceExporterShell
{
public:
   IceThresholdLayerExporter();
   virtual ~IceThresholdLayerExporter();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);

protected:
   virtual void parseInputArgs(PlugInArgList* pInArgList);
   virtual void getOutputCubeAndFileDescriptor(RasterElement*& pOutputCube,
      RasterFileDescriptor*& pOutputFileDescriptor);
   virtual void finishWriting(IceWriter& writer);
   double convertThreshold(Statistics* pStatistics, RegionUnits regionUnits, double value) const;

private:
   ThresholdLayer* mpLayer;
   RasterElement* mpRaster;
   double mFirstThreshold;
   double mSecondThreshold;
   RegionUnits mRegionUnits;
   PassArea mPassArea;
   FileDescriptor* mpOutputDescriptor;
};

#endif
