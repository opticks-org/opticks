/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOTIFFIMPORTER_H
#define GEOTIFFIMPORTER_H

#include "RasterElementImporterShell.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class OptionsTiffImporter;
class RasterDataDescriptor;

class GeoTIFFImporter : public RasterElementImporterShell
{
public:
   GeoTIFFImporter();
   virtual ~GeoTIFFImporter();

   unsigned char getFileAffinity(const std::string& filename);
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   QWidget* getImportOptionsWidget(DataDescriptor* pDescriptor);
   bool validate(const DataDescriptor* pDescriptor, const std::vector<const DataDescriptor*>& importedDescriptors,
      std::string& errorMessage) const;
   bool createRasterPager(RasterElement* pRasterElement) const;

protected:
   bool populateDataDescriptor(RasterDataDescriptor* pDescriptor);
   virtual int getValidationTest(const DataDescriptor* pDescriptor) const;

private:
   std::auto_ptr<OptionsTiffImporter> mpImportOptionsWidget;
   std::map<std::string, std::string> mMetadataMessages;
};

#endif
