/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GDALIMPORTER_H
#define GDALIMPORTER_H

#include "RasterElementImporterShell.h"

#include <string>
#include <vector>

class GdalImporter : public RasterElementImporterShell
{
public:
   GdalImporter();
   virtual ~GdalImporter();

   virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   virtual unsigned char getFileAffinity(const std::string& filename);
   virtual bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;
   virtual bool createRasterPager(RasterElement* pRaster) const;

protected:
   virtual int getValidationTest(const DataDescriptor* pDescriptor) const;

private:
   std::vector<std::string> mErrors;
   std::vector<std::string> mWarnings;
};

#endif
