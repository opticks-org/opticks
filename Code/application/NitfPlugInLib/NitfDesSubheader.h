/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFDESSUBHEADER_H
#define NITFDESSUBHEADER_H

#include "NitfHeader.h"
#include <memory>
#include <string>

class ossimNitfDesHeaderV2_X;
class RasterDataDescriptor;
class ossimContainerProperty;

namespace Nitf
{
   /**
    * Represents the NITF DES subheader.
    */
   class DesSubheader : public Header
   {
   public:
      /**
       * Constructs the DesSubheader.
       *
       * @param fileVersion
       *        Either Nitf::VERSION_02_00 or Nitf::VERSION_02_10.
       * @param index
       *        The index of this DES.
       */
      DesSubheader(const std::string& fileVersion, unsigned int index);

      /**
       * Adds NITF DesSubheader metadata to the DynamicObject.
       *
       * @param pHeader
       *        The source of the metadata.
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
       * @copydoc Nitf::Header::exportMetadataValue()
       *
       * <b>This function is specialized for exporting classification strings.</b>
       */
      static bool exportClassificationString(const RasterDataDescriptor* pDescriptor,
         const DataVariant& prop,
         ossimContainerProperty* pProperties, const std::string& appName,
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

   private:
      DesSubheader& operator=(const DesSubheader& rhs);

      const unsigned int mIndex;
   };
}

#endif
