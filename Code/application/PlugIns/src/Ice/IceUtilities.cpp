/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "IceUtilities.h"

#include "Hdf5Attribute.h"
#include "Hdf5Group.h"
#include "StringUtilitiesMacros.h"
#include <vector>

using namespace std;

namespace StringUtilities
{
BEGIN_ENUM_MAPPING_ALIAS(IceUtilities::FileType, IceUtilitiesFileType)
ADD_ENUM_MAPPING(IceUtilities::RASTER_ELEMENT, "Raster Element", "RasterElement")
ADD_ENUM_MAPPING(IceUtilities::PSEUDOCOLOR_LAYER, "Pseudocolor Layer", "PseudocolorLayer")
ADD_ENUM_MAPPING(IceUtilities::THRESHOLD_LAYER, "Threshold Layer", "ThresholdLayer")
END_ENUM_MAPPING()
}

herr_t ice_exception_walk(int n, H5E_error1_t *pErrDesc, void* pClientData)
{
   IceException* pThisPtr = reinterpret_cast<IceException*>(pClientData);
   Hdf5ErrorDesc errorVal(pErrDesc);
   pThisPtr->mErrorList.push_back(errorVal);
   return 0;
}

bool IceFormatDescriptor::getSupportedFeature(FeatureType feature)
{
   if (feature == FILE_FORMAT_VERSION || feature == FILE_FORMAT_VERSION_DEPRECATED)
   {
      vector<IceVersion> versions;
      //list of supported file format versions - but deprecated, ie. warn the user when they load the files.
      versions.push_back(IceVersion(0, 0));
      versions.push_back(IceVersion(0, 70));
      versions.push_back(IceVersion(0, 90));
      if (feature != FILE_FORMAT_VERSION_DEPRECATED)
      {
         //list of supported file format versions that are not deprecated.
         versions.push_back(IceVersion(1, 0));
         versions.push_back(IceVersion(1, 10));
         versions.push_back(IceVersion(1, 20));
      }
      return mVersion.checkVersion(versions);
   }
   if (feature == ORIGINAL_NUMBERS_IN_DATASET)
   {
      vector<IceVersion> versions;
      //original numbers were stored in attributes, ie. not Hdf5 datasets for these versions
      versions.push_back(IceVersion(0, 0));
      return !mVersion.checkVersion(versions);
   }
   if (feature == CUBE_CLASSIFICATION)
   {
      vector<IceVersion> versions;
      //classification was not present in the following versions
      versions.push_back(IceVersion(0, 0));
      versions.push_back(IceVersion(0, 70));
      return !mVersion.checkVersion(versions);
   }
   if (feature == CUBE_UNITS)
   {
      vector<IceVersion> versions;
      //units were not present in the following versions
      versions.push_back(IceVersion(0, 0));
      versions.push_back(IceVersion(0, 70));
      versions.push_back(IceVersion(0, 90));
      return !mVersion.checkVersion(versions);
   }
   if (feature == CUBE_DISPLAY_INFO)
   {
      vector<IceVersion> versions;
      //display information was not present in the following versions
      versions.push_back(IceVersion(0, 0));
      versions.push_back(IceVersion(0, 70));
      versions.push_back(IceVersion(0, 90));
      return !mVersion.checkVersion(versions);
   }
   if (feature == BAND_STATISTICS)
   {
      vector<IceVersion> versions;
      //statistics information was not present in the following versions
      versions.push_back(IceVersion(0, 0));
      versions.push_back(IceVersion(0, 70));
      versions.push_back(IceVersion(0, 90));
      return !mVersion.checkVersion(versions);
   }
   VERIFY_MSG(false, "Missing condition in the IceFormatDescriptor::getSupportedFeature function");
}

IceFormatDescriptor::IceFormatDescriptor() :
   mValidFile(false),
   mVersion(0, 0)
{
}

bool IceFormatDescriptor::isValidIceFile()
{
   return mValidFile;
}

void IceFormatDescriptor::setIsValidIceFile(bool isValid)
{
   mValidFile = isValid;
}

const IceVersion& IceFormatDescriptor::getVersion()
{
   return mVersion;
}

void IceFormatDescriptor::setVersion(const IceVersion& version)
{
   mVersion = version;
}

IceUtilities::FileType IceFormatDescriptor::getFileType()
{
   return mFileType;
}

void IceFormatDescriptor::setFileType(IceUtilities::FileType fileType)
{
   mFileType = fileType;
}

const string& IceFormatDescriptor::getCreator()
{
   return mCreator;
}

void IceFormatDescriptor::setCreator(const string& creator)
{
   mCreator = creator;
}

const string& IceFormatDescriptor::getCreatorVersion()
{
   return mCreatorVersion;
}

void IceFormatDescriptor::setCreatorVersion(const string& creatorVersion)
{
   mCreatorVersion = creatorVersion;
}

const string& IceFormatDescriptor::getCreatorArch()
{
   return mCreatorArch;
}

void IceFormatDescriptor::setCreatorArch(const string& creatorArch)
{
   mCreatorArch = creatorArch;
}

const string& IceFormatDescriptor::getCreatorOs()
{
   return mCreatorOs;
}

void IceFormatDescriptor::setCreatorOs(const string& creator)
{
   mCreatorOs = creator;
}

void IceFormatDescriptor::addToMessage(Message* pMsg)
{
   DO_IF(pMsg == NULL, return);

   pMsg->addProperty("FormatMajorVersion", mVersion.mMajorVersion);
   pMsg->addProperty("FormatMinorVersion", mVersion.mMinorVersion);
   pMsg->addProperty("FileType", StringUtilities::toDisplayString(mFileType));
   pMsg->addProperty("Creator", mCreator.empty() ? "Unable to parse from file" : mCreator);
   pMsg->addProperty("CreatorVersion", mCreatorVersion.empty() ? "Unable to parse from file" : mCreatorVersion);
   pMsg->addProperty("CreatorOS", mCreatorOs.empty() ? "Unable to parse from file" : mCreatorOs);
   pMsg->addProperty("CreatorArch", mCreatorArch.empty() ? "Unable to parse from file" : mCreatorArch);
}

IceFormatDescriptor IceUtilities::createIceFormatDescriptor(const Hdf5Group* pRootGroup)
{
   IceFormatDescriptor desc;
   const Hdf5Element* pFormat = pRootGroup->getElement("IceFormatDescriptor");
   DO_IF(pFormat == NULL, return desc);

   Hdf5Attribute* pAttr = pFormat->getAttribute("FormatVersion");
   unsigned int formatVersion = 0;
   DO_IF(pAttr == NULL || !pAttr->getValueAs<unsigned int>(formatVersion), return desc);

   IceVersion version(0, 0);
   version.setFileVersion(formatVersion);
   desc.setVersion(version);
   desc.setIsValidIceFile(true);

   string temp;
   pAttr = pFormat->getAttribute("FileType");
   IceUtilities::FileType fileType = IceUtilities::RASTER_ELEMENT;
   if (pAttr != NULL)
   {
      if (pAttr->getValueAs<string>(temp))
      {
         bool error = false;
         IceUtilities::FileType tempFileType = StringUtilities::fromXmlString<IceUtilities::FileType>(temp, &error);
         if (!error)
         {
            fileType = tempFileType;
         }
      }
   }
   desc.setFileType(fileType);

   temp.clear();
   pAttr = pFormat->getAttribute("Creator");
   if (pAttr != NULL)
   {
      pAttr->getValueAs<string>(temp);
   }
   desc.setCreator(temp);

   temp.clear();
   pAttr = pFormat->getAttribute("CreatorVersion");
   if (pAttr != NULL)
   {
      pAttr->getValueAs<string>(temp);
   }
   desc.setCreatorVersion(temp);

   temp.clear();
   pAttr = pFormat->getAttribute("CreatorOS");
   if (pAttr != NULL)
   {
      pAttr->getValueAs<string>(temp);
   }
   desc.setCreatorOs(temp);

   temp.clear();
   pAttr = pFormat->getAttribute("CreatorArch");
   if (pAttr != NULL)
   {
      pAttr->getValueAs<string>(temp);
   }
   desc.setCreatorArch(temp);

   return desc;
}
