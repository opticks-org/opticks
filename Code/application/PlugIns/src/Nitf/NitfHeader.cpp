/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColorType.h"
#include "DateTime.h"
#include "NitfHeader.h"
#include "NitfConstants.h"
#include "RasterDataDescriptor.h"
#include "StringUtilities.h"

#include <ossim/base/ossimBinaryDataProperty.h>
#include <ossim/base/ossimColorProperty.h>
#include <ossim/base/ossimDateProperty.h>
#include <ossim/base/ossimPropertyInterface.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimRtti.h>

#include <vector>

using namespace std;


Nitf::Header::Element::Element(const string &appName, const string &ossimName,
   ImportFunction importFunction, ExportFunction exportFunction) :
   mAppName(appName), mOssimName(ossimName), mImportFunction(importFunction), mExportFunction(exportFunction)
{
   // nothing to do
}

Nitf::Header::Element::Element(ImportFunction importFunction, ExportFunction exportFunction) :
   mAppName(""), mOssimName(""), mImportFunction(importFunction), mExportFunction(exportFunction)
{
   // nothing to do
}

Nitf::Header::Header(const string &fileVersion) :
   mFileVersion(fileVersion)
{
   // nothing to do
}

Nitf::Header::~Header()
{
   // nothing to do
}

bool Nitf::Header::importMetadata(const ossimPropertyInterface *pProperties, 
                                  RasterDataDescriptor *pDescriptor,
                                  DynamicObject *pDynObj)
{
   for (vector<Element>::const_iterator iter = mElements.begin();
        iter != mElements.end(); ++iter)
   {
      const Element& element = *iter;
      if (element.mImportFunction != NULL)
      {
         element.mImportFunction(pProperties, pDescriptor, pDynObj, 
                                 element.mAppName, element.mOssimName);
      }
   }

   return true;
}

bool Nitf::Header::exportMetadata(const RasterDataDescriptor *pDescriptor,
                                  const DynamicObject *pDynObj, 
                                  ossimContainerProperty *pProperties)
{
   // Only export NITF 02.10 files.
   VERIFY(mFileVersion == Nitf::VERSION_02_10);

   string type;
   DataVariant variant;
   for (vector<Element>::const_iterator iter = mElements.begin();
        iter != mElements.end(); ++iter)
   {
      const Element& element = *iter;

      const DataVariant* pProp = &variant;
      if (pDynObj != NULL) // want to call mExportFunction even if the attribute doesn't exist
      {
         pProp = &pDynObj->getAttribute(element.mAppName);
      }
     
      if (element.mExportFunction != NULL)
      {
         element.mExportFunction(pDescriptor, *pProp, pProperties, 
                                 element.mAppName, element.mOssimName);
      }
   }

   return true;
}

bool Nitf::Header::exportMetadata(const RasterDataDescriptor *pDescriptor, 
                                  ossimContainerProperty *pExportHeader)
{
   VERIFY(pDescriptor != NULL);

   FactoryResource<DynamicObject> pDefaults = createDefaultsDynamicObject(pDescriptor);
   VERIFY(Header::exportMetadata(pDescriptor, pDefaults.get(), pExportHeader));

   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   VERIFY(pMetadata != NULL);

   const DynamicObject* pHeaderMetadata =
      pMetadata->getAttributeByPath(getMetadataPath()).getPointerToValue<DynamicObject>();

   VERIFY(Header::exportMetadata(pDescriptor, pHeaderMetadata, pExportHeader));
   return true;
}

bool Nitf::Header::importBinaryData(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor,
   DynamicObject *pDynObj, const string& appName,
   const string& ossimName)
{
   if (pPropertyInterface == NULL)
   {
      return false;
   }

   ossimRefPtr<ossimProperty> pProperty = pPropertyInterface->getProperty(ossimName);
   ossimBinaryDataProperty* pBinaryDataProperty = PTR_CAST(ossimBinaryDataProperty, pProperty.get());
   if (pBinaryDataProperty != NULL)
   {
      return pDynObj->setAttribute(appName, pBinaryDataProperty->getBinaryData());
   }

   return false;
}

bool Nitf::Header::exportBinaryData(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   const vector<unsigned char>* pData = prop.getPointerToValue<vector<unsigned char> >();
   if (pData == NULL)
   {
      return false;
   }

   ossimRefPtr<ossimProperty> pProperty = pProperties->getProperty(ossimName);
   ossimBinaryDataProperty* pBinaryDataProperty = PTR_CAST(ossimBinaryDataProperty, pProperty.get());
   if (pBinaryDataProperty != NULL)
   {
      pBinaryDataProperty->setBinaryData(*pData);
      return true;
   }

   return false;
}

bool Nitf::Header::importColor(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor,
   DynamicObject *pDynObj, const string& appName,
   const string& ossimName)
{
   if (pPropertyInterface == NULL)
   {
      return false;
   }

   ossimRefPtr<ossimProperty> pProperty = pPropertyInterface->getProperty(ossimName);
   ossimColorProperty* pColorProp = PTR_CAST(ossimColorProperty, pProperty.get());
   if (pColorProp != NULL)
   {
      ColorType colorType(static_cast<unsigned char>(pColorProp->getRed()),
                          static_cast<unsigned char>(pColorProp->getGreen()),
                          static_cast<unsigned char>(pColorProp->getBlue()));
      return pDynObj->setAttribute(appName, colorType);
   }

   return false;
}

bool Nitf::Header::exportColor(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   const ColorType* pColor = prop.getPointerToValue<ColorType>();
   if (pColor == NULL || pColor->isValid() == false)
   {
      return false;
   }

   ossimRefPtr<ossimProperty> pProperty = pProperties->getProperty(ossimName);
   ossimColorProperty* pColorProperty = PTR_CAST(ossimColorProperty, pProperty.get());
   if (pColorProperty != NULL)
   {
      pColorProperty->setRed(static_cast<unsigned char>(pColor->mRed));
      pColorProperty->setBlue(static_cast<unsigned char>(pColor->mBlue));
      pColorProperty->setGreen(static_cast<unsigned char>(pColor->mGreen));
      return true;
   }

   return false;
}

bool Nitf::Header::importDateCCYYMMDDhhmmss(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor,
   DynamicObject *pDynObj, const string& appName,
   const string& ossimName)
{
   unsigned short year = 0;
   unsigned short month = 0;
   unsigned short day = 0;
   unsigned short hour = 0;
   unsigned short min = 0;
   unsigned short sec = 0;

   if (pPropertyInterface == NULL)
   {
      return false;
   }

   ossimRefPtr<ossimProperty> pProperty = pPropertyInterface->getProperty(ossimName);
   ossimProperty* pProp = pProperty.get();
   if (pProp == NULL)
   {
      return false;
   }

   FactoryResource<DateTime> pDateTime;
   VERIFY(pDateTime.get() != NULL && pDynObj != NULL);

   ossimString date;
   pProp->valueToString(date);

   // Purposely ignore the return value of DtgParseCCYYMMDDhhmmss so that a DateTime is
   // present in the metadata even if the value is invalid.
   DtgParseCCYYMMDDhhmmss(date, pDateTime.get());
   return pDynObj->setAttribute(appName, *pDateTime.get());
}

bool Nitf::Header::exportDateCCYYMMDDhhmmss(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   const DateTime* pDateTime = prop.getPointerToValue<DateTime>();
   if (pDateTime == NULL || pDateTime->isValid() == false)
   {
      return false;
   }

   if (pDateTime->isTimeValid() == true)
   {
      return Header::exportMetadataValue<string>(pDescriptor, pDateTime->getFormattedUtc("%Y%m%d%H%M%S"),
                                                      pProperties, appName, ossimName);
   }
   else
   {
      return Header::exportMetadataValue<string>(pDescriptor, pDateTime->getFormattedUtc("%Y%m%d------"),
                                                      pProperties, appName, ossimName);
   }
}

bool Nitf::Header::importDateCCYYMMDD(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor, 
   DynamicObject *pDynObj, const string& appName, 
   const string& ossimName)
{
   FactoryResource<DateTime> pDateTime;
   VERIFY(pDateTime.get() != NULL && pDynObj != NULL);

   if (pPropertyInterface == NULL)
   {
      return false;
   }

   ossimRefPtr<ossimProperty> pProperty = pPropertyInterface->getProperty(ossimName);
   ossimProperty* pProp = pProperty.get();
   if (pProp == NULL)
   {
      return false;
   }

   ossimString date;
   pProp->valueToString(date);

   // Purposely ignore the return value of DtgParseCCYYMMDD so that a DateTime is
   // present in the metadata even if the value is invalid.
   DtgParseCCYYMMDD(date, pDateTime.get());
   return pDynObj->setAttribute(appName, *pDateTime.get());
}

bool Nitf::Header::exportDateCCYYMMDD(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   const DateTime* pDateTime = prop.getPointerToValue<DateTime>();
   if (pDateTime == NULL || pDateTime->isValid() == false)
   {
      return false;
   }

   return Header::exportMetadataValue<string>(pDescriptor, pDateTime->getFormattedUtc("%Y%m%d"),
                                                   pProperties, appName, ossimName);
}

bool Nitf::Header::importDateDDHHMMSSZMONYY(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor, 
   DynamicObject *pDynObj, const string& appName, 
   const string& ossimName)
{
   FactoryResource<DateTime> pDateTime;
   VERIFY(pDateTime.get() != NULL && pDynObj != NULL);

   if (pPropertyInterface == NULL)
   {
      return false;
   }

   ossimRefPtr<ossimProperty> pProperty = pPropertyInterface->getProperty(ossimName);
   ossimProperty* pProp = pProperty.get();
   if (pProp == NULL)
   {
      return false;
   }

   ossimString date;
   pProp->valueToString(date);

   // Purposely ignore the return value of DtgParseCCYYMMDD so that a DateTime is
   // present in the metadata even if the value is invalid.
   DtgParseDDHHMMSSZMONYY(date, pDateTime.get());
   return pDynObj->setAttribute(appName, *pDateTime.get());
}

bool Nitf::Header::importDateYYMMDD(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor, 
   DynamicObject *pDynObj, const string& appName, 
   const string& ossimName)
{
   FactoryResource<DateTime> pDateTime;
   VERIFY(pDateTime.get() != NULL && pDynObj != NULL);

   if (pPropertyInterface == NULL)
   {
      return false;
   }

   ossimRefPtr<ossimProperty> pProperty = pPropertyInterface->getProperty(ossimName);
   ossimProperty* pProp = pProperty.get();
   if (pProp == NULL)
   {
      return false;
   }

   ossimString date;
   pProp->valueToString(date);

   // Purposely ignore the return value of DtgParseYYMMDD so that a DateTime is
   // present in the metadata even if the value is invalid.
   DtgParseYYMMDD(date, pDateTime.get());
   return pDynObj->setAttribute(appName, *pDateTime.get());
}
