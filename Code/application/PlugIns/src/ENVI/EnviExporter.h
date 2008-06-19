/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ENVIEXPORTER_H
#define ENVIEXPORTER_H

#include "ExporterShell.h"
#include "PlugInManagerServices.h"

class Progress;
class RasterElement;
class RasterFileDescriptor;
class Step;

class EnviExporter : public ExporterShell
{
public:
   EnviExporter();
   ~EnviExporter();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool hasAbort();
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool abort();

protected:
   bool extractInputArgs(PlugInArgList* pArgList);

private:
   bool mAbortFlag;
   Service<PlugInManagerServices> mpPlugInManager;

   Progress* mpProgress;
   RasterElement* mpRaster;
   RasterFileDescriptor* mpFileDescriptor;

   Step *mpStep;
};

#endif
