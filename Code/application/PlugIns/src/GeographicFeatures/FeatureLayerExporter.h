/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATURELAYEREXPORTER_H
#define FEATURELAYEREXPORTER_H

#include "ExporterShell.h"

class FeatureLayerExporter : public ExporterShell
{
public:
   FeatureLayerExporter();
   virtual ~FeatureLayerExporter();
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   virtual std::string getDefaultExtensions() const;
};

#endif