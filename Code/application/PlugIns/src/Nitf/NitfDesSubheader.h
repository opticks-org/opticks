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
   class DesSubheader : public Header
   {
   public:
      DesSubheader(const std::string &fileVersion, unsigned int index);

      bool importMetadata(const ossimPropertyInterface *pHeader, 
         RasterDataDescriptor *pDescriptor);

      std::string getMetadataPath() const;

      FactoryResource<DynamicObject> createDefaultsDynamicObject(
         const RasterDataDescriptor *pDescriptor);

   protected: // special import/export code
      static bool exportClassificationString(const RasterDataDescriptor *pDescriptor,
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      static bool exportClassificationDate(const RasterDataDescriptor *pDescriptor,
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

   private:
      const unsigned int mIndex;
   };
}

#endif
