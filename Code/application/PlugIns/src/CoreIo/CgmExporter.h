/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CGMEXPORTER_H
#define CGMEXPORTER_H

#include "ExporterShell.h"
#include "PlugInManagerServices.h"

class CgmExporter : public ExporterShell
{
public:
   CgmExporter();
   ~CgmExporter();

   bool getInputSpecification(PlugInArgList*& pInArgList);
   bool getOutputSpecification(PlugInArgList*& pOutArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

private:
   Service<PlugInManagerServices> mpPlugInManager;
};

#endif
