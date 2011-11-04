/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFIMAGESUBHEADER_H
#define NITFIMAGESUBHEADER_H

#include "LocationType.h"
#include "NitfHeader.h"
#include <memory>

class GcpPoint;
class ossimNitfImageHeaderV2_X;
class RasterDataDescriptor;
class ossimContainerProperty;

namespace Nitf
{
   /**
    * Represents the NITF image subheader.
    */
   class ImageSubheader : public Header
   {
   public:
      /**
       * Constructs the ImageSubheader.
       *
       * @param fileVersion
       *        Either Nitf::VERSION_02_00 or Nitf::VERSION_02_10.
       */
      ImageSubheader(const std::string& fileVersion);

      /**
       * Adds NITF ImageSubheader metadata to the DynamicObject.
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
       * @copydoc Nitf::Header::importMetadataValue()
       *
       * <b>This function is specialized for importing the band information.</b>
       */
      static bool importBandInformation(const ossimPropertyInterface* pPropertyInterface,
         RasterDataDescriptor* pDescriptor,
         DynamicObject* pDynObj, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::importMetadataValue()
       *
       * <b>This function is specialized for importing the IGEOLO from NITF 2.0 files.</b>
       */
      static bool importGeoInformation_2_0(const ossimPropertyInterface* pPropertyInterface,
         RasterDataDescriptor* pDescriptor,
         DynamicObject* pDynObj, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::importMetadataValue()
       *
       * <b>This function is specialized for importing the IGEOLO from NITF 2.1 files.</b>
       */
      static bool importGeoInformation(const ossimPropertyInterface* pPropertyInterface,
         RasterDataDescriptor* pDescriptor,
         DynamicObject* pDynObj, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::exportMetadataValue()
       *
       * <b>This function is specialized for exporting the IGEOLO to NITF 2.1 files.</b>
       */
      static bool exportGeoInformation(const RasterDataDescriptor* pDescriptor,
         const DataVariant& prop,
         ossimContainerProperty* pProperties, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::importMetadataValue()
       *
       * <b>This function is specialized for importing the IDATIM from NITF 2.0 files.</b>
       */
      static bool importIDATIM_2_0(const ossimPropertyInterface* pPropertyInterface,
         RasterDataDescriptor* pDescriptor,
         DynamicObject* pDynObj, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::importMetadataValue()
       *
       * <b>This function is specialized for importing the IDATIM from NITF 2.1 files.</b>
       */
      static bool importIDATIM(const ossimPropertyInterface* pPropertyInterface,
         RasterDataDescriptor* pDescriptor,
         DynamicObject* pDynObj, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::exportMetadataValue()
       *
       * <b>This function is specialized for exporting the IDATIM to NITF 2.1 files.</b>
       */
      static bool exportIDATIM(const RasterDataDescriptor* pDescriptor,
         const DataVariant& prop,
         ossimContainerProperty* pProperties, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::importMetadataValue()
       *
       * <b>This function is specialized for importing the ILOC.</b>
       */
      static bool importILOC(const ossimPropertyInterface* pPropertyInterface,
         RasterDataDescriptor* pDescriptor,
         DynamicObject* pDynObj, const std::string& appName,
         const std::string& ossimName);

      /**
       * @copydoc Nitf::Header::exportMetadataValue()
       *
       * <b>This function is specialized for exporting the ILOC.</b>
       */
      static bool exportILOC(const RasterDataDescriptor* pDescriptor,
         const DataVariant& prop,
         ossimContainerProperty* pProperties, const std::string& appName,
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
       * @copydoc Nitf::Header::exportMetadataValue()
       *
       * <b>This function is specialized for exporting classification dates and times.</b>
       */
      static bool exportClassificationDate(const RasterDataDescriptor* pDescriptor,
         const DataVariant& prop,
         ossimContainerProperty* pProperties, const std::string& appName,
         const std::string& ossimName);

   private:
      ImageSubheader& operator=(const ImageSubheader& rhs);

      static bool getGCPsFromUtmMgrs(const string& iGeolo, const LocationType gcpPixels[],
         unsigned int numGcpPixels, list<GcpPoint>& gcps);

      static bool getGCPsFromUtm(const string& iCords, const string& iGeolo,
         const LocationType gcpPixels[], unsigned int numGcpPixels, list<GcpPoint>& gcps);

      static bool getGCPsFromGeographic(const string& iGeolo, const LocationType gcpPixels[],
         unsigned int numGcpPixels, list<GcpPoint>& gcps);

      static bool getGCPsFromDecimalDegrees(const string& iGeolo, const LocationType gcpPixels[],
         unsigned int numGcpPixels, list<GcpPoint>& gcps);
   };
}

#endif
