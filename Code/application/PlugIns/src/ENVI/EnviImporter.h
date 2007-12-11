/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ENVIIMPORTER_H
#define ENVIIMPORTER_H

#include "RasterElementImporterShell.h"
#include "EnviField.h"

#include <string>
#include <vector>

class EnviImporter : public RasterElementImporterShell
{
public:
   EnviImporter();
   ~EnviImporter();

   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);

protected:
   bool parseHeader(const std::string& filename);
   std::string findDataFile(const std::string& headerPath);
   std::string findHeaderFile(const std::string& dataPath);

private:
   EnviField mFields;
};

#endif
