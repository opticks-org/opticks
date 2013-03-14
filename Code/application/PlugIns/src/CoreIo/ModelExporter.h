/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODELEXPORTER_H
#define MODELEXPORTER_H

#include "ExporterShell.h"

class PlugInManagerServices;

/**
 *  Model Exporter
 *
 *  This plug-in exports core model elements.
 */
class ModelExporter : public ExporterShell
{
public:
   ModelExporter(const std::string& dataElementSubclass);
   ~ModelExporter();

   bool getInputSpecification(PlugInArgList*& pInArgList);
   bool getOutputSpecification(PlugInArgList*& pOutArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   Service<PlugInManagerServices> mpPlugInManager;
   std::string mDataElementSubclass;
};

#endif
