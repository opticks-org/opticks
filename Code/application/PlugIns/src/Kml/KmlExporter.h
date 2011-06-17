/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef KMLEXPORTER_H
#define KMLEXPORTER_H

#include "ExporterShell.h"

class KmlExporter : public ExporterShell
{
public:
   KmlExporter();
   ~KmlExporter();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

class KmlLayerExporter : public KmlExporter
{
public:
   KmlLayerExporter();
   ~KmlLayerExporter();

   bool getInputSpecification(PlugInArgList*& pArgList);
};

#endif
