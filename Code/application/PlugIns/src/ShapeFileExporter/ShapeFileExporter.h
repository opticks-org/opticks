/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SHAPEFILEEXPORTER_H
#define SHAPEFILEEXPORTER_H

#include "ExporterShell.h"
#include "PlugInManagerServices.h"
#include "ShapeFile.h"

#include <QtGui/QWidget>

#include <memory>

class AoiElement;
class LayerList;
class RasterElement;
class ShapeFileOptionsWidget;

class ShapeFileExporter : public ExporterShell
{
public:
   ShapeFileExporter();
   ~ShapeFileExporter();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   QWidget* getExportOptionsWidget(const PlugInArgList* pInArgList);

private:
   Service<PlugInManagerServices> mpPlugInManager;
   std::auto_ptr<ShapeFileOptionsWidget> mpOptionsWidget;
   ShapeFile mShapefile;
   AoiElement* mpAoi;
   RasterElement* mpGeoref;
   LayerList* mpLayers;
   bool extractInputs(const PlugInArgList* pInArgList, std::string& message);
};

#endif
