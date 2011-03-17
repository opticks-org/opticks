/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef JPEG2000IMPORTER_H
#define JPEG2000IMPORTER_H

#include "AppConfig.h"
#if defined (JPEG2000_SUPPORT)

#include "RasterElementImporterShell.h"

#include <memory>
#include <vector>

class RasterDataDescriptor;

class Jpeg2000Importer : public RasterElementImporterShell
{
public:
   Jpeg2000Importer();
   virtual ~Jpeg2000Importer();

   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   bool execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList);
   bool isProcessingLocationSupported(ProcessingLocation location) const;

   QWidget* getPreview(const DataDescriptor* pDescriptor, Progress* pProgress);

   bool createRasterPager(RasterElement *pRasterElement) const;

protected:
   bool populateDataDescriptor(RasterDataDescriptor* pDescriptor);
};

#endif
#endif