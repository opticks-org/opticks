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
   class FileHeader : public Header
   {
   public:
      FileHeader(const std::string &fileVersion);

      /**
       * Adds NITF File Header Metadata to the DynamicObject.
       */
      bool importMetadata(const ossimPropertyInterface *pHeader, RasterDataDescriptor *pDescriptor);

      std::string getMetadataPath() const;

      FactoryResource<DynamicObject> createDefaultsDynamicObject(
         const RasterDataDescriptor *pDescriptor);

   protected: // special import/export code
      static bool importClassificationString(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      static bool exportClassificationString(const RasterDataDescriptor *pDescriptor,
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      static bool importClassificationDate(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      static bool exportClassificationDate(const RasterDataDescriptor *pDescriptor,
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      static bool exportOSTAID(const RasterDataDescriptor *pDescriptor, 
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      static bool exportONAME(const RasterDataDescriptor *pDescriptor, 
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      static bool exportOPHONE(const RasterDataDescriptor *pDescriptor, 
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

   private:
      SETTING(OSTAID, FileHeader, std::string, APP_NAME);
      SETTING(ONAME, FileHeader, std::string, std::string());
      SETTING(OPHONE, FileHeader, std::string, std::string());
   };
}

#endif
