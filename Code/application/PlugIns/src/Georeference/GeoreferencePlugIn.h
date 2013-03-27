/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOREFERENCEPLUGIN_H
#define GEOREFERENCEPLUGIN_H

#include "AlgorithmShell.h"
#include "TypesFile.h"

#include <string>

class PlugInArgList;
class Progress;
class RasterElement;
class SpatialDataView;

class GeoreferencePlugIn : public AlgorithmShell
{
public:
   GeoreferencePlugIn();
   virtual ~GeoreferencePlugIn();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInParam, PlugInArgList* pOutParam);

private:
   Progress* mpProgress;
   RasterElement* mpRaster;
   SpatialDataView* mpView;

   std::string mLayerName;
   bool mCreateLayer;
   bool mDisplayLayer;
   GeocoordType mGeocoordType;
   DmsFormatType mLatLonFormat;
};

#endif
