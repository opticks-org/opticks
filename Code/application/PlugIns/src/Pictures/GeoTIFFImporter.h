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

   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   bool validateDefaultOnDiskReadOnly(const DataDescriptor* pDescriptor, std::string& errorMessage) const;
   bool execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList);

   QWidget *getImportOptionsWidget(DataDescriptor *pDescriptor);

   bool createRasterPager(RasterElement *pRasterElement) const;
   bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

protected:
   bool populateDataDescriptor(RasterDataDescriptor* pDescriptor);
   void loadIsdMetadata(DataDescriptor *pDescriptor);

private:
   std::auto_ptr<OptionsTiffImporter> mImportOptionsWidget;
   std::string mMetadataMessage;
};

#endif
