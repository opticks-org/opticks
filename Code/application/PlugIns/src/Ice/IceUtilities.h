/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ICEUTILITIES_H
#define ICEUTILITIES_H

#include "MessageLogResource.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <hdf5.h>

class Hdf5Group;
class Message;

class Hdf5ErrorDesc
{
public:
   Hdf5ErrorDesc(H5E_error1_t* pErr) :
      mLineNumber(0)
   {
      if (pErr != NULL)
      {
         mErrorDesc = pErr->desc;
         mFunctionName = pErr->func_name;
         mFilename = pErr->file_name;
         mLineNumber = pErr->line;
         mMajorErrorMsg = H5Eget_major(pErr->maj_num);
         mMinorErrorMsg = H5Eget_minor(pErr->min_num);
      }
   }

   std::string mErrorDesc;
   std::string mFilename;
   std::string mFunctionName;
   unsigned int mLineNumber;
   std::string mMajorErrorMsg;
   std::string mMinorErrorMsg;
};

class IceException
{
public:
   IceException(const std::string& filename, unsigned int lineNumber, const std::string& failureMsg = "") :
      mFilename(filename), mLineNumber(lineNumber), mFailureMsg(failureMsg)
   {
      H5Ewalk1(H5E_WALK_DOWNWARD, &IceException::walk, this);
   }

   std::string hdfErrorToString() const
   {
      std::ostringstream output;
      std::vector<Hdf5ErrorDesc>::const_iterator iter;
      for (iter = mErrorList.begin(); iter != mErrorList.end(); ++iter)
      {
         output << iter->mErrorDesc << std::endl;
         output << "   " << iter->mFunctionName << std::endl;
         output << "   " << iter->mFilename << std::endl;
         output << "   " << iter->mLineNumber << std::endl;
         output << "   " << iter->mMajorErrorMsg << std::endl;
         output << "   " << iter->mMinorErrorMsg << std::endl;
      }
      return output.str();
   }

   void addToMessageLog() const
   {
      StepResource step("Ice Error", "app", "EE1AA170-2983-4A13-8613-A2E0D427D85E");
      MessageResource iceMsg("Ice Error Description", "app", "1A244959-F4B8-4C09-AFA9-EEC08A8B9F07");
      iceMsg->addProperty("Filename", mFilename);
      iceMsg->addProperty("Line number", mLineNumber);
      iceMsg->finalize();
      std::vector<Hdf5ErrorDesc>::const_iterator iter;
      for (iter = mErrorList.begin(); iter != mErrorList.end(); ++iter)
      {
         MessageResource msg("Hdf5 Error", "app", "EF243E5B-6D50-4E11-B95B-23B23ED80A04");
         msg->addProperty("Description", iter->mErrorDesc);
         msg->addProperty("Function Name", iter->mFunctionName);
         msg->addProperty("Filename", iter->mFilename);
         msg->addProperty("Line number", iter->mLineNumber);
         msg->addProperty("Major Error Msg", iter->mMajorErrorMsg);
         msg->addProperty("Minor Error Msg", iter->mMinorErrorMsg);
         msg->finalize();
      }
      step->finalize();
   }

   const std::string& getFailureMessage() const
   {
      return mFailureMsg;
   }

private:
   static herr_t walk(int n, H5E_error1_t *pErrDesc, void* pClientData)
   {
      IceException* pThisPtr = reinterpret_cast<IceException*>(pClientData);
      Hdf5ErrorDesc errorVal(pErrDesc);
      pThisPtr->mErrorList.push_back(errorVal);
      return 0;
   }

   std::vector<Hdf5ErrorDesc> mErrorList;
   std::string mFilename;
   unsigned int mLineNumber;
   std::string mFailureMsg;
};

class IceAbortException
{
public:
   IceAbortException() {}
   ~IceAbortException() {}
};

class IceVersion
{
public:
   IceVersion(unsigned int majorVersion, unsigned int minorVersion) :
      mMajorVersion(majorVersion),
      mMinorVersion(minorVersion)
   {
   }
   void setFileVersion(unsigned int fileVersion)
   {
      mMajorVersion = (fileVersion / 100);
      mMinorVersion = (fileVersion % 100);
   }
   unsigned int getFileVersion()
   {
      return (mMajorVersion * 100) + mMinorVersion;
   }
   bool operator==(const IceVersion& rhs) const
   {
      return mMajorVersion == rhs.mMajorVersion && mMinorVersion == rhs.mMinorVersion;
   }
   bool checkVersion(const std::vector<IceVersion>& versions)
   {
      if (std::find(versions.begin(), versions.end(), *this) != versions.end())
      {
         return true;
      }
      else
      {
         return false;
      }
   }

   unsigned int mMajorVersion;
   unsigned int mMinorVersion;
};

namespace IceUtilities
{
   enum FileTypeEnum
   {
      RASTER_ELEMENT,
      PSEUDOCOLOR_LAYER,
      THRESHOLD_LAYER
   };
   typedef EnumWrapper<FileTypeEnum> FileType;
};

class IceFormatDescriptor
{
public:
   enum FeatureTypeEnum
   {
      FILE_FORMAT_VERSION, /**< Is this version of the Ice file supported */
      FILE_FORMAT_VERSION_DEPRECATED, /**< Is this version of the Ice file deprecated,
                                      ie. soon to be removed so we should warn the user
                                      while loading the file. */
      ORIGINAL_NUMBERS_IN_DATASET, /**< Are the original numbers in 
                                  Hdf5 datasets within their own Hdf5 group or stored Hdf5 attributes */
      CUBE_CLASSIFICATION, /**< Is classification present and required for the raw cube data */
      CUBE_UNITS, /**< Is unit information present and required for the raw cube data */
      CUBE_DISPLAY_INFO, /**< Is display information present and required for the raw cube data */
      BAND_STATISTICS /**< Are Band Statistics present and required for the raw cube data */
   };
   typedef EnumWrapper<FeatureTypeEnum> FeatureType;

   IceFormatDescriptor();
   bool isValidIceFile();
   void setIsValidIceFile(bool isValid);
   bool getSupportedFeature(FeatureType feature);
   const IceVersion& getVersion();
   void setVersion(const IceVersion& version);
   IceUtilities::FileType getFileType();
   void setFileType(IceUtilities::FileType fileType);
   const std::string& getCreator();
   void setCreator(const std::string& creator);
   const std::string& getCreatorVersion();
   void setCreatorVersion(const std::string& creatorVersion);
   const std::string& getCreatorArch();
   void setCreatorArch(const std::string& creatorArch);
   const std::string& getCreatorOs();
   void setCreatorOs(const std::string& creator);

   void addToMessage(Message* pMsg);

private:
   bool mValidFile;
   IceVersion mVersion;
   IceUtilities::FileType mFileType;
   std::string mCreator;
   std::string mCreatorVersion;
   std::string mCreatorArch;
   std::string mCreatorOs;
};

namespace IceUtilities
{
   IceFormatDescriptor createIceFormatDescriptor(const Hdf5Group* pRootGroup);
};

#define ICEVERIFY_MSG(condition, msg) \
   if (!(condition)) \
   { \
      throw IceException(__FILE__, __LINE__, string(msg)); \
   }

#define ICEVERIFY(condition) \
   if (!(condition)) \
   { \
      throw IceException(__FILE__, __LINE__); \
   }

#endif
