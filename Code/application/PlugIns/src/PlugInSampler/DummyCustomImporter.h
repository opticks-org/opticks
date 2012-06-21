/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DUMMYCUSTOMIMPORTER_H
#define DUMMYCUSTOMIMPORTER_H

#include "ImporterShell.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"

class DummyCustomImporter : public ImporterShell
{
public:
   DummyCustomImporter();
   ~DummyCustomImporter();

   bool getInputSpecification(PlugInArgList*&);
   bool getOutputSpecification(PlugInArgList*&);
   bool execute(PlugInArgList*, PlugInArgList*);
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   bool setBatch();
   bool setInteractive();
   bool hasAbort();
   bool abort();
   unsigned char getFileAffinity(const std::string& filename);

private:
   Service<PlugInManagerServices> mpPlugInManager;
   Service<ModelServices> mpModel;
};

#endif
