/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CGMIMPORTER_H
#define CGMIMPORTER_H

#include "DesktopServices.h"
#include "ImporterShell.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "PlugInManagerServices.h"

#include <vector>

class DataDescriptor;

/**
 *  Cgm Importer
 *
 *  This plug-in imports cgm elements.
 */
class CgmImporter : public ImporterShell
{
public:
   CgmImporter();
   ~CgmImporter();

   bool getInputSpecification(PlugInArgList *&pInArgList);
   bool getOutputSpecification(PlugInArgList *&pOutArgList);
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   bool execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList);

private:
   Service<DesktopServices> mpDesktop;
   Service<ModelServices> mpModel;
   Service<PlugInManagerServices> mpPlugInManager;
};

#endif
