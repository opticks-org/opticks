/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GENERICIMPORTER_H
#define GENERICIMPORTER_H

#include "RasterElementImporterShell.h"

#include <string>
#include <vector>

class GenericImporter : public RasterElementImporterShell
{
public:
   GenericImporter();
   ~GenericImporter();

   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;
};

#endif
