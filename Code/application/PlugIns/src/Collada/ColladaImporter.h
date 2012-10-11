/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COLLADAIMPORTER_H
#define COLLADAIMPORTER_H

#include "AppConfig.h"
#if defined(OPENCOLLADA_SUPPORT)

#include "Location.h"
#include "ColladaStreamReader.h"
#include "ImporterShell.h"
#include "Testable.h"

class ColladaImporter : public ImporterShell, public Testable
{
public:
   ColladaImporter();
   virtual ~ColladaImporter();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);

   virtual bool runOperationalTests(Progress* pProgress, std::ostream& failure);
   virtual bool runAllTests(Progress* pProgress, std::ostream& failure);
};

#endif
#endif
