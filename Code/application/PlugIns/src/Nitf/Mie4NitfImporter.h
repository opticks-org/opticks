/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MIE4NITFIMPORTER_H
#define MIE4NITFIMPORTER_H

#include "NitfImporterShell.h"

namespace Nitf
{
   class Mie4NitfImporter : public Nitf::NitfImporterShell
   {
   public:
      Mie4NitfImporter();
      virtual ~Mie4NitfImporter();

      virtual unsigned char getFileAffinity(const std::string& filename);
      virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string &filename);
      virtual ImportDescriptor* getImportDescriptor(const std::string& filename, ossim_uint32 layerNum,
                                                    const Nitf::OssimFileResource& pFile, const ossimNitfFileHeaderV2_X* pFileHeader,
                                                    const DynamicObject& mimcsa);
      virtual bool createRasterPager(RasterElement* pRaster) const;
      bool validate(const DataDescriptor* pDescriptor, const std::vector<const DataDescriptor*>& importedDescriptors, std::string& errorMessage) const;

   private:
      unsigned int loadFileInfo(const std::string& indexfile, const std::string& parentName, const std::string& layerId, DynamicObject& manifestMetadata);
   };
}
#endif

