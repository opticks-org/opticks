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
class ossimString;

namespace Nitf
{
   /**
    * This is a %Resource class for TRE parser plug-ins.
    *
    * @see TreParserShell
   */
   class TrePlugInResource : public PlugInResource
   {
   public:
      /**
       * Construct an object to represent a TRE parser.
       *
       * @param   plugInName
       *          The name of the plug-in to construct.
       */
      TrePlugInResource(std::string plugInName) : PlugInResource(plugInName)
      {}

      /**
       * Destruct the TRE parser plug-in.
       */
      TrePlugInResource() : PlugInResource()
      {}

      /**
       * Parse a TRE and store it in a DynamicObject and RasterDataDescriptor.
       *
       * @param input
       *        The ossimNitfRegisteredTag to read from.
       * @param output
       *        The DynamicObject to write to.
       * @param descriptor
       *        The RasterDataDescriptor which should be updated.
       * @param errorMessage
       *        If this is modified by the function, it will be displayed to the
       *        user as a warning that imported TREs might be incomplete, missing, etc.
       *
       * @return \c True on success, \c false otherwise.
       */
      bool parseTag(const ossimNitfRegisteredTag& input, DynamicObject& output,
         RasterDataDescriptor& descriptor, std::string& errorMessage) const;

      /**
       * Parse a TRE from a DynamicObject and store it in \c writer.
       *
       * @param input
       *        The DynamicObject to read from.
       * @param ownerIndex
       *        The index of the owner in \c writer.
       * @param tagType
       *        The type of tag in \c writer.
       * @param writer
       *        The ossimNitfWriter to write to.
       * @param errorMessage
       *        %Message that exported TREs might be incomplete, missing, etc.
       *
       * @return \c True on success or if the TRE was excluded from export, \c false otherwise.
       */
      bool writeTag(const DynamicObject& input, const ossim_uint32& ownerIndex, const ossimString& tagType,
         ossimNitfWriter& writer, std::string& errorMessage) const;

      /**
       * Exports TRE metadata.
       *
       * @param descriptor
       *        The RasterDataDescriptor to read from.
       * @param exportDescriptor
       *        The RasterDataDescriptor to write to.
       * @param writer
       *        The ossimNitfWriter to write to.
       * @param errorMessage
       *        %Message that exported TREs might be incomplete, missing, etc.
       *
       * @return \c True on success or if the TRE was excluded from export, \c false otherwise.
       */
      bool exportMetadata(const RasterDataDescriptor& descriptor,
         const RasterFileDescriptor& exportDescriptor, ossimNitfWriter& writer,
         std::string& errorMessage) const;

   private:
      SETTING(ExcludedTres, TrePlugInResource, std::vector<std::string>, std::vector<std::string>());
   };

  /**
   * Imports supported metadata for the specified image into a RasterDataDescriptor.
   *
   * @param currentImage
   *        The index of the image to import.
   * @param pFile
   *        The source file.
   * @param pFileHeader
   *        The header of the source file.
   * @param pImageSubheader
   *        The current image subheader.
   * @param pDescriptor
   *        The RasterDataDescriptor to populate.
   * @param parsers
   *        Contains TRE parsers which have already been loaded into memory -- included for improved performance.
   * @param errorMessage
   *        %Message for import errors, etc.
   *
   * @return \c True on success, \c false otherwise.
   */
   bool importMetadata(const unsigned int& currentImage, const Nitf::OssimFileResource& pFile,
      const ossimNitfFileHeaderV2_X* pFileHeader, const ossimNitfImageHeaderV2_X* pImageSubheader,
      RasterDataDescriptor* pDescriptor, std::map<std::string, TrePlugInResource>& parsers, std::string& errorMessage);

   /**
    * Adds a single TRE to a RasterDataDescriptor.
    *
    * @param ownerIndex
    *        The index of the owner of this TRE.
    * @param tagInfo
    *        Information about the TRE.
    * @param pDescriptor
    *        The RasterDataDescriptor to populate.
    * @param pTres
    *        The DynamicObject containing the TREs.
    * @param pTreInfo
    *        The DynamicObject containing the locations of the TREs.
    * @param parsers
    *        Contains TRE parsers which have already been loaded into memory -- included for performance.
    * @param errorMessage
    *        %Message for import errors, etc.
    *
    * @return \c True on success, \c false otherwise.
    */
   bool addTagToMetadata(const unsigned int& ownerIndex,
      const ossimNitfTagInformation& tagInfo, RasterDataDescriptor* pDescriptor, DynamicObject* pTres,
      DynamicObject* pTreInfo, std::map<std::string, TrePlugInResource>& parsers, std::string& errorMessage);

   /**
    * Exports supported metadata for the specified image into \c pNitf.
    *
    * @param pDescriptor
    *        The RasterDataDescriptor containing the metadata to export.
    * @param pExportDescriptor
    *        The RasterFileDescriptor for export.
    * @param pNitf
    *        The ossimNitfWriter to populate.
    * @param pProgress
    *        The Progress object to use.
    *
    * @return \c True on success, \c false otherwise.
    */
   bool exportMetadata(const RasterDataDescriptor* pDescriptor,
      const RasterFileDescriptor* pExportDescriptor,
      ossimNitfWriter* pNitf, Progress* pProgress);
}

#endif
