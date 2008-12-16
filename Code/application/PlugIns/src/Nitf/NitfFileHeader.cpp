/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Classification.h"
#include "DataVariant.h"
#include "NitfConstants.h"
#include "NitfFileHeader.h"
#include "NitfHeader.h"
#include "RasterDataDescriptor.h"
#include "ObjectResource.h"
#include "SpecialMetadata.h"
#include "TypeConverter.h"

#include <ossim/support_data/ossimNitfFileHeaderV2_X.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_0.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_1.h>

#include <string>
using namespace std;


using namespace Nitf::FileHeaderFieldNames;

Nitf::FileHeader::FileHeader(const string &fileVersion) :
   Header(fileVersion)
{
   // Elements specific to NITF 02.00
   if (mFileVersion == Nitf::VERSION_02_00)
   {
      mElements.push_back(Element(DATE_TIME, ossimNitfFileHeaderV2_X::FDT_KW,
         importDateDDHHMMSSZMONYY,
         NULL));

      // 02.00 security strings
      mElements.push_back(Element(SECURITY_DOWNGRADE_2_0, ossimNitfFileHeaderV2_0::FSDWNG_KW,
         importClassificationString,
         NULL));

      mElements.push_back(Element(SECURITY_CLASS_TEXT, ossimNitfFileHeaderV2_0::FSDEVT_KW,
         importClassificationString,
         NULL));
   }
   // Elements specific to NITF 02.10
   else if (mFileVersion == Nitf::VERSION_02_10)
   {
      mElements.push_back(Element(DATE_TIME, ossimNitfFileHeaderV2_X::FDT_KW,
      importDateCCYYMMDDhhmmss,
      exportDateCCYYMMDDhhmmss));

      // 02.10 security strings
      mElements.push_back(Element(SECURITY_SYSTEM, ossimNitfFileHeaderV2_1::FSCLASY_KW, 
         importClassificationString, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_DECLASS_TYPE, ossimNitfFileHeaderV2_1::FSDCTP_KW, 
         importClassificationString, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_DECLASS_EXEMPT, ossimNitfFileHeaderV2_1::FSDCXM_KW, 
         importClassificationString, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_DOWNGRADE, ossimNitfFileHeaderV2_1::FSDG_KW, 
         importClassificationString, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_CLASS_TEXT, ossimNitfFileHeaderV2_1::FSCLTX_KW, 
         importClassificationString, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_AUTH_TYPE, ossimNitfFileHeaderV2_1::FSCATP_KW, 
         importClassificationString, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_CLASS_REASON, ossimNitfFileHeaderV2_1::FSCRSN_KW, 
         importClassificationString, 
         exportClassificationString));

      // 02.10 Security dates
      mElements.push_back(Element(SECURITY_DECLASS_DATE, ossimNitfFileHeaderV2_1::FSDCDT_KW,
         importClassificationDate,
         exportClassificationDate));
      mElements.push_back(Element(SECURITY_DOWNGRADE_DATE, ossimNitfFileHeaderV2_1::FSDGDT_KW, 
         importClassificationDate,
         exportClassificationDate));
      mElements.push_back(Element(SECURITY_SOURCE_DATE, ossimNitfFileHeaderV2_1::FSSRDT_KW, 
         importClassificationDate,
         exportClassificationDate));
   }

   // Elements not specific to version
   mElements.push_back(Element(CLEVEL, ossimNitfFileHeaderV2_X::CLEVEL_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(ENCRYPTION, ossimNitfFileHeaderV2_X::ENCRYP_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(TITLE, ossimNitfFileHeaderV2_X::FTITLE_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(ONAME, ossimNitfFileHeaderV2_X::ONAME_KW, 
      importMetadataValue<string>, 
      exportONAME));
   mElements.push_back(Element(OPHONE, ossimNitfFileHeaderV2_X::OPHONE_KW, 
      importMetadataValue<string>, 
      exportOPHONE));
   mElements.push_back(Element(OSTAID, ossimNitfFileHeaderV2_X::OSTAID_KW, 
      importMetadataValue<string>, 
      exportOSTAID));
   mElements.push_back(Element(SYSTEM_TYPE, ossimNitfFileHeaderV2_X::STYPE_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));

   // At first glance, FBKGC *should* be specific to NITF version 2.1.
   // However, it was added to NITF 2.0 after its initial debut. See MIL-STD-2500B for more details.
   mElements.push_back(Element(BACKGROUND_COLOR, ossimNitfFileHeaderV2_1::FBKGC_KW, 
      importColor,
      exportColor));

   // Security strings
   mElements.push_back(Element(SECURITY_AUTH, ossimNitfFileHeaderV2_X::FSCAUT_KW, 
      importClassificationString, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_COPY_NUMBER, ossimNitfFileHeaderV2_X::FSCOP_KW, 
      importClassificationString, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_NUM_COPIES, ossimNitfFileHeaderV2_X::FSCPYS_KW, 
      importClassificationString, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_CTRL_AND_HANDL, ossimNitfFileHeaderV2_X::FSCTLH_KW, 
      importClassificationString, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_CTRL_NUM, ossimNitfFileHeaderV2_X::FSCTLN_KW, 
      importClassificationString, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_RELEASE_INSTRUCTIONS, ossimNitfFileHeaderV2_X::FSREL_KW, 
      importClassificationString, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_LEVEL, ossimNitfFileHeaderV2_X::FSCLAS_KW, 
      importClassificationString, 
      exportClassificationString));
   mElements.push_back(Element(SECURITY_CODEWORDS, ossimNitfFileHeaderV2_X::FSCODE_KW, 
      importClassificationString, 
      exportClassificationString));
}

string Nitf::FileHeader::getMetadataPath() const
{
   return Nitf::NITF_METADATA + "/" + Nitf::FILE_HEADER;
}

FactoryResource<DynamicObject> Nitf::FileHeader::createDefaultsDynamicObject(const RasterDataDescriptor *pDescriptor)
{
   FactoryResource<DynamicObject> pFileHeader;
   VERIFYRV(pFileHeader.get() != NULL, pFileHeader);

   // Create a default FTITLE with useful information.
   string defaultTitle = string("Generated by ") + APP_NAME + " (Version " + APP_VERSION_NUMBER + ")";
   pFileHeader->setAttribute(TITLE, defaultTitle);

   return pFileHeader;
}

bool Nitf::FileHeader::importMetadata(const ossimPropertyInterface *pHeader, RasterDataDescriptor *pDescriptor)
{
   VERIFY(pDescriptor != NULL && pHeader != NULL);

   ossimNitfFileHeader* pFileHeader = PTR_CAST(ossimNitfFileHeader, pHeader);
   VERIFY(pFileHeader != NULL && pFileHeader->getVersion() == mFileVersion);

   FactoryResource<DynamicObject> pFileHeaderMetadata;
   VERIFY(pFileHeaderMetadata.get() != NULL);

   VERIFY(Header::importMetadata(pFileHeader, pDescriptor, pFileHeaderMetadata.get()));
   pFileHeaderMetadata->setAttribute(FILE_VERSION, mFileVersion);

   int64_t fileLength = pFileHeader->getFileSize();
   pFileHeaderMetadata->setAttribute(LENGTH, fileLength);

   int headerLength = pFileHeader->getHeaderSize();
   pFileHeaderMetadata->setAttribute(HEADER_LENGTH, headerLength);

   int numDes = pFileHeader->getNumberOfDataExtSegments();
   pFileHeaderMetadata->setAttribute(NUM_DES, numDes);

   int numRes = pFileHeader->getNumberOfReservedExtSegments();
   pFileHeaderMetadata->setAttribute(NUM_RES, numRes);

   int numImages = pFileHeader->getNumberOfImages();
   pFileHeaderMetadata->setAttribute(NUM_IMAGE_SEGMENTS, numImages);

   int numTexts = pFileHeader->getNumberOfTextSegments();
   pFileHeaderMetadata->setAttribute(NUM_TEXT_SEGMENTS, numTexts);

   int numGraphics = pFileHeader->getNumberOfGraphics();
   pFileHeaderMetadata->setAttribute(NUM_GRAPHIC_SEGMENTS, numGraphics);

   if (mFileVersion == Nitf::VERSION_02_00)
   {
      int numLabels = pFileHeader->getNumberOfLabels();
      pFileHeaderMetadata->setAttribute(NUM_LABELS, numLabels);
   }

   DynamicObject* pMetadata = pDescriptor->getMetadata();
   VERIFY(pMetadata != NULL);
   VERIFY(pMetadata->setAttributeByPath(getMetadataPath(), *pFileHeaderMetadata.get()));
   return true;
}

bool Nitf::FileHeader::importClassificationString(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor,
   DynamicObject *pDynObj, const string& appName,
   const string& ossimName)
{
   VERIFY(pDescriptor != NULL);
   Classification* pClass = pDescriptor->getClassification();
   VERIFY(pClass != NULL);

   if (!importMetadataValue<string>(pPropertyInterface, pDescriptor, 
      pDynObj, appName, ossimName))
   {
      return false;
   }

   const DataVariant& attrValue = pDynObj->getAttribute(appName);
   string type = attrValue.getTypeName();
   const string* pValue = attrValue.getPointerToValue<string>();

   if (pValue == NULL)
   {
      // ok, empty string
      return true;
   }

   VERIFY(type == "string");

   if (appName == SECURITY_LEVEL)
   {
      pClass->setLevel(*pValue);
   }
   else if (appName == SECURITY_SYSTEM)
   {
      pClass->setSystem(*pValue);
      pClass->setCountryCode(*pValue);
   }
   else if (appName == SECURITY_CODEWORDS)
   {
      pClass->setCodewords(*pValue);
   }
   else if (appName == SECURITY_CTRL_AND_HANDL)
   {
      pClass->setFileControl(*pValue);
   }
   else if (appName == SECURITY_RELEASE_INSTRUCTIONS)
   {
      pClass->setFileReleasing(*pValue);
   }
   else if (appName == SECURITY_DECLASS_EXEMPT)
   {
      pClass->setDeclassificationExemption(*pValue);
   }
   else if (appName == SECURITY_DOWNGRADE)
   {
      pClass->setFileDowngrade(*pValue);
   }
   else if (appName == SECURITY_AUTH)
   {
      pClass->setAuthority(*pValue);
   }
   else if (appName == SECURITY_AUTH_TYPE)
   {
      pClass->setAuthorityType(*pValue);
   }
   else if (appName == SECURITY_CTRL_NUM)
   {
      pClass->setSecurityControlNumber(*pValue);
   }
   else if (appName == SECURITY_COPY_NUMBER)
   {
      pClass->setFileCopyNumber(*pValue);
   }
   else if (appName == SECURITY_NUM_COPIES)
   {
      pClass->setFileNumberOfCopies(*pValue);
   }
   else if (appName == SECURITY_CLASS_REASON)
   {
      pClass->setClassificationReason(*pValue);
   }
   else if (appName == SECURITY_CLASS_TEXT)
   {
      pClass->setDescription(*pValue);
   }
   else if (appName == SECURITY_DECLASS_TYPE)
   {
      pClass->setDeclassificationType(*pValue);
   }
   else if (appName == SECURITY_DOWNGRADE_2_0)
   {
      pClass->setDescription(pClass->getDescription() + " " + appName + ": " + *pValue);
   }
   else
   {
      return false;
   }

   return true;
}

bool Nitf::FileHeader::exportClassificationString(const RasterDataDescriptor *pDescriptor, 
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
   else if (appName == SECURITY_COPY_NUMBER)
   {
      value = pClass->getFileCopyNumber();
   }
   else if (appName == SECURITY_NUM_COPIES)
   {
      value = pClass->getFileNumberOfCopies();
   }
   else if (appName == SECURITY_CLASS_REASON)
   {
      value = pClass->getClassificationReason();
   }
   else if (appName == SECURITY_CLASS_TEXT)
   {
      value = pClass->getDescription();
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

bool Nitf::FileHeader::importClassificationDate(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor,
   DynamicObject *pDynObj, const string& appName,
   const string& ossimName)
{
   VERIFY(pDescriptor != NULL);
   Classification* pClass = pDescriptor->getClassification();
   VERIFY(pClass != NULL);

   // All FileHeader classification dates are stored in CCYYMMDD format.
   if (!importDateCCYYMMDD(pPropertyInterface, pDescriptor, 
                           pDynObj, appName, ossimName))
   {
      return false;
   }

   const DataVariant& attrValue = pDynObj->getAttribute(appName);
   string type = attrValue.getTypeName();
   const DateTime* pValue = attrValue.getPointerToValue<DateTime>();
   if (pValue == NULL)
   {
      // ok, empty string
      return true;
   }

   VERIFY(type == "DateTime");

   if (appName == SECURITY_DECLASS_DATE)
   {
      pClass->setDeclassificationDate(pValue);
   }
   else if (appName == SECURITY_DOWNGRADE_DATE)
   {
      pClass->setDowngradeDate(pValue);
   }
   else if (appName == SECURITY_SOURCE_DATE)
   {
      pClass->setSecuritySourceDate(pValue);
   }
   else
   {
      return false;
   }

   pDescriptor->setClassification(pClass);
   return true;
}

bool Nitf::FileHeader::exportClassificationDate(const RasterDataDescriptor *pDescriptor, 
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
      // All FileHeader dates are stored in CCYYMMDD format.
      return exportDateCCYYMMDD(pDescriptor, *pValue, pProperties, appName, ossimName);
   }

   return false;
}

bool Nitf::FileHeader::exportOSTAID(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   // OSTAID cannot be blank, so make sure that it has a reasonable value in it.
   if (getSettingOSTAID().empty() == true)
   {
      setSettingOSTAID(APP_NAME);
   }

   return exportMetadataValue<string>(pDescriptor, getSettingOSTAID(), pProperties, appName, ossimName);
}

bool Nitf::FileHeader::exportONAME(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   return exportMetadataValue<string>(pDescriptor, getSettingONAME(), pProperties, appName, ossimName);
}

bool Nitf::FileHeader::exportOPHONE(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   return exportMetadataValue<string>(pDescriptor, getSettingOPHONE(), pProperties, appName, ossimName);
}
