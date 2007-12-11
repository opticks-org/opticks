/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LAYERIMPORTER_H
#define LAYERIMPORTER_H

#include "ImporterShell.h"

class DesktopServices;
class PlugInManagerServices;
class UtilityServices;

/**
 *  Layer Importer
 *
 *  This plug-in imports layers.
 */
class LayerImporter : public ImporterShell
{
public:
   LayerImporter();
   ~LayerImporter();

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
   bool getInputSpecification(PlugInArgList *&pInArgList);
   bool getOutputSpecification(PlugInArgList *&pOutArgList);
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   bool execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList);

private:
   bool mInteractive;

   Service<DesktopServices> mpDesktop;
   Service<PlugInManagerServices> mpPlugInManager;
};

#endif
