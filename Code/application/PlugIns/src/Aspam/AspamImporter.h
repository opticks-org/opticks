/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ASPAMIMPORTER_H
#define ASPAMIMPORTER_H

#include "ImporterShell.h"

#include <stdio.h>
#include <string>
#include <vector>

class Aspam;

/**
 *  Plug-in for importing ASPAM and PAR files.
 */
class AspamImporter : public ImporterShell
{
public:
   AspamImporter();
   ~AspamImporter();

   bool getInputSpecification(PlugInArgList*& pInArgList);
   bool getOutputSpecification(PlugInArgList*& pOutArgList);
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

private:
   const char* parseParagraphA(const char* pStart);
   const char* parseParagraphB(const char* pStart);
   const char* parseParagraphD(const char* pStart);
   const char* parseParagraphF(const char* pStart);
   const char* parseParagraphG(const char* pStart);
   const char* parseParagraphH(const char* pStart);
   const char* parseParagraphJ(const char* pStart);
   bool deserialize(FILE* pFp);

   Aspam* mpAspam;
};

#endif
