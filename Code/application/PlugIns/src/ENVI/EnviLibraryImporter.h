/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ENVILIBRARYIMPORTER_H
#define ENVILIBRARYIMPORTER_H

#include "AppVerify.h"
#include "Endian.h"
#include "EnviField.h"
#include "ImporterShell.h"

#include <stdio.h>
#include <string>
#include <vector>

class ModelServices;
class PlugInArgList;
class PlugInManagerServices;
class Progress;
class SignatureLibrary;
class Step;

class EnviLibraryImporter : public ImporterShell
{
public:
   EnviLibraryImporter();
   ~EnviLibraryImporter();

   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool hasAbort();
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool abort();

protected:
   bool extractPlugInArgs(PlugInArgList* pArgList);
   std::string findDataFile(const std::string& filename);
   std::string matchDataFile(const std::string& filename, const std::string& fileExtension,
      const std::string& openMode);
   bool parseHeader(const std::string& filename);

private:
   bool mbAbort;
   Service<PlugInManagerServices> mpPlugInManager;
   Service<ModelServices> mpModel;
   Step* mpStep;

   Progress* mpProgress;
   SignatureLibrary* mpSignatureLibrary;

   EnviField mFields;
};

#endif   // ENVILIBRARYIMPORTER_H
