/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MRUFILE_H
#define MRUFILE_H

#include "DateTimeImp.h"

#include <string>
#include <vector>

class ImportDescriptor;

struct MruFile
{
   MruFile()
   {
   }

   MruFile(const std::string& name, const std::string& importerName,
      const std::vector<ImportDescriptor*>& descriptors, const DateTimeImp& modificationTime) :
      mName(name),
      mImporterName(importerName),
      mDescriptors(descriptors),
      mModificationTime(modificationTime)
   {
   }

   std::string mName;
   std::string mImporterName;
   std::vector<ImportDescriptor*> mDescriptors;
   DateTimeImp mModificationTime;
};

#endif
