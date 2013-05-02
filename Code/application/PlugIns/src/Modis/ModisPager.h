/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODISPAGER_H
#define MODISPAGER_H

#include "CachedPager.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "ModisUtilities.h"
#include "ObjectResource.h"
#include "TypesFile.h"

#include <hdfi.h>
#include <string>

class DimensionDescriptor;

class ModisPager : public CachedPager
{
public:
   ModisPager();
   virtual ~ModisPager();

   static std::string MetadataArg();
   static std::string DatasetNameArg();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool parseInputArgs(PlugInArgList* pArgList);

   virtual bool openFile(const std::string& filename);
   virtual CachedPage::UnitPtr fetchUnit(DataRequest* pOriginalRequest);

protected:
   template <typename Out>
   CachedPage::UnitPtr loadBand(int bandIndex, EncodingType inputDataType, const DimensionDescriptor& startRow,
      const DimensionDescriptor& startColumn, const DimensionDescriptor& startBand, unsigned int concurrentRows,
      unsigned int concurrentColumns) const;

   template <typename In, typename Out>
   bool populateBandData(In* pInData, Out* pOutData, unsigned int numPixels, int bandIndex) const;

   DataVariant getMetadataValue(const std::string& attributeName) const;

private:
   int32 mFileHandle;
   int32 mDatasetHandle;
   std::string mDatasetName;

   FactoryResource<DynamicObject> mpMetadata;
   ModisUtilities::RasterConversionType mRasterConversion;
};

#endif
