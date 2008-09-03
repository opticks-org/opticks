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

#include <string>

class ShapeFileExporter : public ExporterShell
{
public:
   ShapeFileExporter();
   ~ShapeFileExporter();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool setBatch();
   bool setInteractive();
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

private:
   bool mbInteractive;
   Service<PlugInManagerServices> mpPlugInManager;
};

#endif   // SHAPEFILEEXPORTER_H
