/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFMETADATAPARSING_H
#define NITFMETADATAPARSING_H

#include "ConfigurationSettings.h"
#include "PlugInResource.h"
#include "NitfResource.h"
#include "NitfTreParser.h"

#include <map>
#include <ostream>
#include <string>
#include <vector>

class DynamicObject;
class RasterDataDescriptor;

class ossimImageFileWriter;
class ossimNitfFileHeaderV2_X;
class ossimNitfImageHeaderV2_X;
class ossimNitfRegisteredTag;
class ossimNitfTagInformation;
class ossimNitfWriter;
class ossimContainerProperty;
class ossimProperty;

namespace Nitf
{
   class TrePlugInResource : public PlugInResource
   {
   public:
      TrePlugInResource(std::string plugInName) : PlugInResource(plugInName)
      {}

      TrePlugInResource() : PlugInResource()
      {}

      bool parseTag(const ossimNitfRegisteredTag& input, DynamicObject& output,
         RasterDataDescriptor& descriptor, std::string& errorMessage) const;
      bool writeTag(const DynamicObject& input, const ossim_uint32& ownerIndex, const ossimString& tagType,
         ossimNitfWriter& writer, std::string& errorMessage) const;

      bool exportMetadata(const RasterDataDescriptor& descriptor,
         const RasterFileDescriptor& exportDescriptor, ossimNitfWriter& writer,
         std::string& errorMessage) const;

      TreState validateTag(const DynamicObject& tag, std::ostream& reporter) const;

   private:
      SETTING(ExcludedTres, TrePlugInResource, std::vector<std::string>, std::vector<std::string>());
   };

  /**
   *  Adds tag data that is already pre-parsed by OSSIM.
   */
   bool importMetadata(const unsigned int& currentImage, const Nitf::OssimFileResource& pFile,
      const ossimNitfFileHeaderV2_X* pFileHeader, const ossimNitfImageHeaderV2_X* pImageSubheader,
      RasterDataDescriptor* pDescriptor, std::map<std::string, TrePlugInResource>& parsers, std::string& errorMessage);

   bool addTagToMetadata(const unsigned int& ownerIndex,
      const ossimNitfTagInformation& tagInfo, RasterDataDescriptor* pDescriptor, DynamicObject* pTres,
      DynamicObject* pTreInfo, std::map<std::string, TrePlugInResource>& parsers, std::string& errorMessage);

   bool exportMetadata(const RasterDataDescriptor *pDescriptor,
      const RasterFileDescriptor *pExportDescriptor,
      ossimNitfWriter *pNitf, Progress *pProgress);
}

#endif
