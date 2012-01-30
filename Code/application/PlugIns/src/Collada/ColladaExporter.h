/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COLLADAEXPORTER_H
#define COLLADAEXPORTER_H

#include "AppConfig.h"
#if defined(OPENCOLLADA_SUPPORT)

#include "ColladaStreamWriter.h"
#include "ExporterShell.h"

class ColladaExporter : public ExporterShell
{
public:
   ColladaExporter();
   virtual ~ColladaExporter();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

private:
   bool isGraphicObjectSupported(GraphicObjectType type);
};

#endif
#endif
