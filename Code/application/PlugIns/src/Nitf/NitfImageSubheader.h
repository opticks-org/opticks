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
   class ImageSubheader : public Header
   {
   public:
      ImageSubheader(const std::string &fileVersion);

      bool importMetadata(const ossimPropertyInterface *pHeader, 
         RasterDataDescriptor *pDescriptor);

      std::string getMetadataPath() const;

      FactoryResource<DynamicObject> createDefaultsDynamicObject(
         const RasterDataDescriptor *pDescriptor);

   protected: // special import/export code
      static bool importBandInformation(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      static bool importGeoInformation_2_0(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      static bool importGeoInformation(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      static bool exportGeoInformation(const RasterDataDescriptor *pDescriptor,
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      static bool importIDATIM_2_0(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      static bool importIDATIM(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      static bool exportIDATIM(const RasterDataDescriptor *pDescriptor,
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      static bool importILOC(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      static bool exportILOC(const RasterDataDescriptor *pDescriptor,
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      static bool exportClassificationString(const RasterDataDescriptor *pDescriptor,
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      static bool exportClassificationDate(const RasterDataDescriptor *pDescriptor,
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      static bool getGCPsFromUtmMgrs(const string& iGeolo, const LocationType gcpPixels[],
         const unsigned int& numGcpPixels, list<GcpPoint>& gcps);

      static bool getGCPsFromUtm(const string& iCords, const string& iGeolo,
         const LocationType gcpPixels[], const unsigned int& numGcpPixels, list<GcpPoint>& gcps);

      static bool getGCPsFromGeographic(const string& iGeolo, const LocationType gcpPixels[],
         const unsigned int& numGcpPixels, list<GcpPoint>& gcps);

      static bool getGCPsFromDecimalDegrees(const string& iGeolo, const LocationType gcpPixels[],
         const unsigned int& numGcpPixels, list<GcpPoint>& gcps);
   };
}

#endif
