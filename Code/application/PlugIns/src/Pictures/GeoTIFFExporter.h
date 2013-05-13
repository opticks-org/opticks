/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOTIFFEXPORTER_H
#define GEOTIFFEXPORTER_H

#include <xtiffio.h>
#include <memory>

#include "ExporterShell.h"
#include "Progress.h"

class GeoTiffExportOptionsWidget;
class RasterElement;
class RasterFileDescriptor;
class Step;

class GeoTIFFExporter : public ExporterShell
{
public:
   GeoTIFFExporter();
   virtual ~GeoTIFFExporter();

   bool abort();
   bool hasAbort();
   bool execute(PlugInArgList* pInParam, PlugInArgList* pOutParam);
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);

   QWidget* getExportOptionsWidget(const PlugInArgList* pInArgList);

private:
   bool CreateGeoTIFF(TIFF* pOut);
   bool applyWorldFile(TIFF* pOut);
   void updateProgress(int current, int total, const std::string& progressString, ReportingLevel level = NORMAL);
   bool writeCube(TIFF* pOut);

   Step* mpStep;
   std::auto_ptr<GeoTiffExportOptionsWidget> mpOptionWidget;

   Progress* mpProgress;
   RasterElement* mpRaster;
   RasterFileDescriptor* mpFileDescriptor;

   bool mAbortFlag;
   std::string mMessage;
   unsigned int mRowsPerStrip;
};

#endif
