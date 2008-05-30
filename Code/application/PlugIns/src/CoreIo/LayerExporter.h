/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LAYEREXPORTER_H
#define LAYEREXPORTER_H

#include "ExporterShell.h"
#include "TypesFile.h"

class DesktopServices;
class PlugInManagerServices;
class UtilityServices;

/**
 *  Layer Exporter
 *
 *  This plug-in exports layers.
 */
class LayerExporter : public ExporterShell
{
public:
   LayerExporter(LayerType layerType);
   ~LayerExporter();

   bool setBatch()
   {
      mInteractive = false;
      return true;
   }
   bool setInteractive()
   {
      mInteractive = true;
      return true;
   }

   bool getInputSpecification(PlugInArgList*& pInArgList);
   bool getOutputSpecification(PlugInArgList*& pOutArgList);
   bool execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList);

private:
   Service<DesktopServices> mpDesktop;
   Service<PlugInManagerServices> mpPlugInManager;

   bool mInteractive;
   std::string mLayerSubclass;
};

#endif
