/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Classification.h"
#include "DateTime.h"
#include "NitfDesSubheader.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "SpecialMetadata.h"

#include <string>

#include <ossim/support_data/ossimNitfDataExtensionSegmentV2_0.h>
#include <ossim/support_data/ossimNitfDataExtensionSegmentV2_1.h>

using namespace Nitf::DesSubheaderFieldNames;
using namespace std;

Nitf::DesSubheader::DesSubheader(const string &fileVersion, unsigned int index) :
   Header(fileVersion),
   mIndex(index)
{
   // Elements specific to NITF 02.00
   if (mFileVersion == Nitf::VERSION_02_00)
   {
      mElements.push_back(Element(DESID, ossimNitfDataExtensionSegmentV2_0::DESTAG_KW, 
         importMetadataValue<string>, 
         NULL));

      // 02.00 security strings
      mElements.push_back(Element(SECURITY_DOWNGRADE, ossimNitfDataExtensionSegmentV2_0::DESDWNG_KW,
         importDateYYMMDD, 
         NULL));

      mElements.push_back(Element(SECURITY_CLASS_TEXT, ossimNitfDataExtensionSegmentV2_0::DESDEVT_KW,
         importMetadataValue<string>, 
         NULL));
   }
   //// Elements specific to NITF 02.10
   else if (mFileVersion == Nitf::VERSION_02_10)
   {
      mElements.push_back(Element(DESID, ossimNitfDataExtensionSegmentV2_1::DESID_KW, 
         importMetadataValue<string>, 
         exportMetadataValue<string>));

      // 02.10 Security strings
      mElements.push_back(Element(SECURITY_SYSTEM, ossimNitfDataExtensionSegmentV2_1::DESCLSY_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_DECLASS_TYPE, ossimNitfDataExtensionSegmentV2_1::DESDCTP_KW,
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_DECLASS_DATE, ossimNitfDataExtensionSegmentV2_1::DESDCDT_KW, 
         importDateCCYYMMDD, 
         exportClassificationDate));
      mElements.push_back(Element(SECURITY_DECLASS_EXEMPT, ossimNitfDataExtensionSegmentV2_1::DESDCXM_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_DOWNGRADE, ossimNitfDataExtensionSegmentV2_1::DESDG_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_DOWNGRADE_DATE, ossimNitfDataExtensionSegmentV2_1::DESDGDT_KW, 
         importDateCCYYMMDD, 
         exportClassificationDate));
      mElements.push_back(Element(SECURITY_CLASS_TEXT, ossimNitfDataExtensionSegmentV2_1::DESCLTX_KW,
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_AUTH_TYPE, ossimNitfDataExtensionSegmentV2_1::DESCATP_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_CLASS_REASON, ossimNitfDataExtensionSegmentV2_1::DESCRSN_KW,
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_SOURCE_DATE, ossimNitfDataExtensionSegmentV2_1::DESSRDT_KW, 
         importDateCCYYMMDD, 
         exportClassificationDate));
   }

   // Elements not specific to version
   mElements.push_back(Element(DE, ossimNitfDataExtensionSegment::DE_KW,
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(DESVER, ossimNitfDataExtensionSegment::DESVER_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(DESOFLW, ossimNitfDataExtensionSegment::DESOFLW_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(DESITEM, ossimNitfDataExtensionSegment::DESITEM_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(DESSHL, ossimNitfDataExtensionSegment::DESSHL_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(DESSHF, ossimNitfDataExtensionSegment::DESSHF_KW, 
      importBinaryData, 
      exportBinaryData));
   mElements.push_back(Element(DESDATA, ossimNitfDataExtensionSegment::DESDATA_KW, 
      importBinaryData,
      exportBinaryData));

   // Security strings
   mElements.push_back(Element(SECURITY_LEVEL, ossimNitfDataExtensionSegment::DECLAS_KW, 
      importMetadataValue<string>, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_CODEWORDS, ossimNitfDataExtensionSegment::DESCODE_KW, 
      importMetadataValue<string>, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_CTRL_AND_HANDL, ossimNitfDataExtensionSegment::DESCTLH_KW, 
      importMetadataValue<string>, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_RELEASE_INSTRUCTIONS, ossimNitfDataExtensionSegment::DESREL_KW, 
      importMetadataValue<string>, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_AUTH, ossimNitfDataExtensionSegment::DESCAUT_KW, 
      importMetadataValue<string>, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_CTRL_NUM, ossimNitfDataExtensionSegment::DESCTLN_KW, 
      importMetadataValue<string>, 
      exportClassificationString));
}

string Nitf::DesSubheader::getMetadataPath() const
{
   stringstream indexStr;
   indexStr << "DES_" << setw(3) << setfill('0') << mIndex;
   return Nitf::NITF_METADATA + "/" + Nitf::DES_METADATA + "/" + indexStr.str();
}

FactoryResource<DynamicObject> Nitf::DesSubheader::createDefaultsDynamicObject(
   const RasterDataDescriptor *pDescriptor)
{
   FactoryResource<DynamicObject> pDesSubheader;

   // Per the NITF spec, the field "DE" must contain the value "DE".
   pDesSubheader->setAttribute(DE, DE);
   return pDesSubheader;
}

bool Nitf::DesSubheader::importMetadata(const ossimPropertyInterface *pHeader, RasterDataDescriptor *pDescriptor)
{
   VERIFY(pHeader != NULL && pDescriptor != NULL);

   FactoryResource<DynamicObject> pDesSubheaderMetadata;
   VERIFY(pDesSubheaderMetadata.get() != NULL);

   VERIFY(Header::importMetadata(pHeader, pDescriptor, pDesSubheaderMetadata.get()));
   VERIFY(pDescriptor != NULL);

   DynamicObject* pMetadata = pDescriptor->getMetadata();
   VERIFY(pMetadata != NULL);

   pMetadata->setAttributeByPath(getMetadataPath(), *pDesSubheaderMetadata.get());
   return true;
}

bool Nitf::DesSubheader::exportClassificationString(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   VERIFY(pDescriptor != NULL);
   const Classification* pClass = pDescriptor->getClassification();
   VERIFY(pClass != NULL);

   string value;

   if (appName == SECURITY_LEVEL)
   {
      value = pClass->getLevel();
   }
   else if (appName == SECURITY_SYSTEM)
   {
      value = pClass->getSystem();
   }
   else if (appName == SECURITY_CODEWORDS)
   {
      value = pClass->getCodewords();
   }
   else if (appName == SECURITY_CTRL_AND_HANDL)
   {
      value = pClass->getFileControl();
   }
   else if (appName == SECURITY_RELEASE_INSTRUCTIONS)
   {
      value = pClass->getFileReleasing();
   }
   else if (appName == SECURITY_DECLASS_EXEMPT)
   {
      value = pClass->getDeclassificationExemption();
   }
   else if (appName == SECURITY_DOWNGRADE)
   {
      value = pClass->getFileDowngrade();
   }
   else if (appName == SECURITY_AUTH)
   {
      value = pClass->getAuthority();
   }
   else if (appName == SECURITY_AUTH_TYPE)
   {
      value = pClass->getAuthorityType();
   }
   else if (appName == SECURITY_CTRL_NUM)
   {
      value = pClass->getSecurityControlNumber();
   }
   else if (appName == SECURITY_RELEASE_INSTRUCTIONS)
   {
      value = pClass->getFileReleasing();
   }
   else if (appName == SECURITY_CLASS_TEXT)
   {
      value = pClass->getDescription();
   }
   else if (appName == SECURITY_CLASS_REASON)
   {
      value = pClass->getClassificationReason();
   }
   else if (appName == SECURITY_DECLASS_TYPE)
   {
      value = pClass->getDeclassificationType();
   }
   else
   {
      return false;
   }

   return exportMetadataValue<string>(pDescriptor, 
      value, pProperties, appName, ossimName);
}

bool Nitf::DesSubheader::exportClassificationDate(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   VERIFY(pDescriptor != NULL);
   const Classification* pClass = pDescriptor->getClassification();
   VERIFY(pClass != NULL);

   const DateTime* pValue = NULL;
   
   if (appName == SECURITY_DECLASS_DATE)
   {
      pValue = pClass->getDeclassificationDate();
   }
   else if (appName == SECURITY_DOWNGRADE_DATE)
   {
      pValue = pClass->getDowngradeDate();
   }
   else if (appName == SECURITY_SOURCE_DATE)
   {
      pValue = pClass->getSecuritySourceDate();
   }
   else
   {
      return false;
   }

   if (pValue != NULL && pValue->isValid() == true)
   {
      // All DesSubheader classification dates are stored in CCYYMMDD format.
      return exportDateCCYYMMDD(pDescriptor, *pValue, pProperties, appName, ossimName);
   }

   return false;
}
