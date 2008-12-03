/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PICTURESPLOTWIDGETEXPORTER_H
#define PICTURESPLOTWIDGETEXPORTER_H

#include "PicturesExporter.h"

class PlotWidget;

class PicturesPlotWidgetExporter : public PicturesExporter
{
public:
   PicturesPlotWidgetExporter(PicturesDetails *pDetails);
   ~PicturesPlotWidgetExporter();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool extractInputArgs(const PlugInArgList* pInArgList);
   bool generateImage(QImage &image);
};

#endif
