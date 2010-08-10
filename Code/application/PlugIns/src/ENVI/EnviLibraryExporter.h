/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ENVILIBRARYEXPORTER_H
#define ENVILIBRARYEXPORTER_H

#include "ExporterShell.h"

#include <string>
#include <vector>

class FileDescriptor;
class PlugInArgList;
class PlugInManagerServices;
class Progress;
class Signature;
class SignatureSet;
class Step;

class EnviLibraryExporter : public ExporterShell
{
public:
   EnviLibraryExporter();
   ~EnviLibraryExporter();

   bool setBatch();
   bool setInteractive();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool hasAbort();
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool abort();

protected:
   bool extractPlugInArgs(PlugInArgList* pArgList);
   bool extractSignatures(SignatureSet& signatureSet, std::vector<Signature*>& signatures);

private:
   bool mbAbort;
   Service<PlugInManagerServices> mpPlugInManager;

   Progress* mpProgress;
   SignatureSet* mpSignatureSet;
   FileDescriptor* mpFileDescriptor;

   Step* mpStep;
};

#endif
