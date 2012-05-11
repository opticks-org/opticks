/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFFILEHEADER_H
#define NITFFILEHEADER_H

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "NitfConstants.h"
#include "NitfHeader.h"

#include <string>

class ossimContainerProperty;
class ossimNitfFileHeader;
class RasterDataDescriptor;

class DynamicObject;

namespace Nitf
{
   /**
    * Represents the NITF file header.
    */
   class FileHeader : public Header
   {
   public:
      /**
       * Constructs the FileHeader.
       *
       * @param fileVersion
       *        Either Nitf::VERSION_02_00 or Nitf::VERSION_02_10.
       */
      FileHeader(const std::string& fileVersion);

      /**
       * Adds NITF File Header Metadata to the DynamicObject.
       *
       * @param pHeader
       *        The source of the metadata.
       *
       * @param pDescriptor
       *        The destination for the metadata.
       *
       * @return \c True on success, \c false otherwise.
       */
      bool importMetadata(const ossimPropertyInterface* pHeader, RasterDataDescriptor* pDescriptor);

      std::string getMetadataPath() const;

      FactoryResource<DynamicObject> createDefaultsDynamicObject(const RasterDataDescriptor* pDescriptor);

   protected: // special import/export code
      /**
       * @copydoc Nitf::Header::importMetadataValue()
       *
       * <b>This function is specialized for importing classification strings.</b>
       */
      static bool importClassificationString(const ossimPropertyInterface*pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::exportMetadataValue()
       *
       * <b>This function is specialized for exporting classification strings.</b>
       */
      static bool exportClassificationString(const RasterDataDescriptor* pDescriptor,
         const DataVariant& prop,
         ossimContainerProperty* pProperties, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::importMetadataValue()
       *
       * <b>This function is specialized for importing classification dates and times.</b>
       */
      static bool importClassificationDate(const ossimPropertyInterface* pPropertyInterface,
         RasterDataDescriptor* pDescriptor,
         DynamicObject* pDynObj, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::exportMetadataValue()
       *
       * <b>This function is specialized for exporting classification dates and times.</b>
       */
      static bool exportClassificationDate(const RasterDataDescriptor* pDescriptor,
         const DataVariant& prop,
         ossimContainerProperty* pProperties, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::exportMetadataValue()
       *
       * <b>This function is specialized for exporting Originating STAtion ID.</b>
       */
      static bool exportOSTAID(const RasterDataDescriptor* pDescriptor,
         const DataVariant& prop,
         ossimContainerProperty* pProperties, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::exportMetadataValue()
       *
       * <b>This function is specialized for exporting the originator's name.</b>
       */
      static bool exportONAME(const RasterDataDescriptor* pDescriptor,
         const DataVariant& prop,
         ossimContainerProperty* pProperties, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::exportMetadataValue()
       *
       * <b>This function is specialized for exporting the originator's phone number.</b>
       */
      static bool exportOPHONE(const RasterDataDescriptor* pDescriptor,
         const DataVariant& prop,
         ossimContainerProperty* pProperties, const std::string& appName,
         const std::string& ossimName);

   private:
      FileHeader& operator=(const FileHeader& rhs);

      SETTING(OSTAID, FileHeader, std::string, APP_NAME);
      SETTING(ONAME, FileHeader, std::string, std::string());
      SETTING(OPHONE, FileHeader, std::string, std::string());
   };
}

#endif
