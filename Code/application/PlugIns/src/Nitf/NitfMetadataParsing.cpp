/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DateTime.h"
#include "DynamicObject.h"
#include "MessageLogResource.h"
#include "NitfConstants.h"
#include "NitfDesSubheader.h"
#include "NitfFileHeader.h"
#include "NitfImageSubheader.h"
#include "NitfMetadataParsing.h"
#include "NitfTreParser.h"
#include "NitfUnknownTreParser.h"
#include "NitfUtilities.h"
#include "ObjectFactory.h"
#include "PlugInDescriptor.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"

#include <iostream>
#include <map>

#include <ossim/base/ossimRtti.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimColorProperty.h>
#include <ossim/base/ossimDateProperty.h>
#include <ossim/base/ossimContainerProperty.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/imaging/ossimNitfTileSource.h>
#include <ossim/imaging/ossimNitfWriter.h>

#include <ossim/support_data/ossimNitfDataExtensionSegment.h>
#include <ossim/support_data/ossimNitfImageHeaderV2_X.h>
#include <ossim/support_data/ossimNitfIchipbTag.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_X.h>
#include <ossim/support_data/ossimNitfRpcATag.h>
#include <ossim/support_data/ossimNitfFile.h>
#include <ossim/support_data/ossimNitfUnknownTag.h>
#include <ossim/support_data/ossimNitfTagFactoryRegistry.h>

using namespace Nitf;
using namespace Nitf::TRE;
using namespace std;

bool Nitf::TrePlugInResource::parseTag(const ossimNitfRegisteredTag& input, DynamicObject& output, 
                                       RasterDataDescriptor &descriptor, string &errorMessage) const
{
   bool parsed = false;
   const TreParser* pParser = dynamic_cast<const TreParser*>(get());
   if (pParser != NULL)
   {
      string errorMessage1;
      string errorMessage2;
      parsed = pParser->toDynamicObject(input, output, errorMessage1);
      if (!parsed)
      {
         stringstream strm;
         const_cast<ossimNitfRegisteredTag&>(input).writeStream(strm);
         parsed = pParser->toDynamicObject(strm, input.getSizeInBytes(), output, errorMessage1);
      }

      if (parsed)
      {
         pParser->importMetadata(output, descriptor, errorMessage2);
      }
      bool either = !errorMessage1.empty() || !errorMessage2.empty();
      bool both = !errorMessage1.empty() && !errorMessage2.empty();

      if (either)
      {
         errorMessage = getArgs().mPlugInName + ": ";

         if (!errorMessage1.empty())
         {
            errorMessage += errorMessage1;
         }

         if (both)
         {
            errorMessage += "\n";
         }

         if (!errorMessage2.empty())
         {
            errorMessage += errorMessage2;
         }
      }
   }
   return parsed;
}

bool Nitf::TrePlugInResource::writeTag(const DynamicObject &input, const ossim_uint32& ownerIndex,
   const ossimString& tagType, ossimNitfWriter &writer, string &errorMessage) const
{
   bool success = false;
   bool skipped = false;
   string tagName = getArgs().mPlugInName;

   // Check the list of excluded TREs to ensure that the config setting takes precedence over any existing plugins.
   const vector<string>& excluded = getSettingExcludedTres();
   if (find(excluded.begin(), excluded.end(), tagName) != excluded.end())
   {
      return true;
   }

   ossimNitfTagFactoryRegistry* pRegistry = ossimNitfTagFactoryRegistry::instance();
   ossimRefPtr<ossimNitfRegisteredTag> pTag = pRegistry->create(tagName);
   VERIFY(pTag != NULL);

   const TreParser* pParser = dynamic_cast<const TreParser*>(get());
   ossimNitfUnknownTag* pUnknown = NULL;
   if (pTag->canCastTo("ossimNitfUnknownTag"))
   {
      pUnknown = PTR_CAST(ossimNitfUnknownTag, pTag.get());
   }

   if (pUnknown == NULL && pParser != NULL && pParser->fromDynamicObject(input, *pTag, errorMessage))
   {
      success = true;
   }
   else
   {
      size_t numBytesWritten = 0;
      stringstream strm;
      if (pParser != NULL && pParser->fromDynamicObject(input, strm, numBytesWritten, errorMessage))
      {
         success = true;
      }
      else
      {
         PlugInResource pUnknownPlugin(UnknownTreParser::PLUGIN_NAME);
         const TreParser* pUnknownParser = dynamic_cast<const TreParser*>(pUnknownPlugin.get());
         VERIFY(pUnknownParser != NULL);
         success = pUnknownParser->fromDynamicObject(input, strm, numBytesWritten, errorMessage);
      }

      if (success)
      {
         if (pUnknown != NULL)
         {
            pUnknown->setTagName(tagName);
            pUnknown->setTagLength(numBytesWritten);
         }
         pTag->parseStream(strm);
      }
   }

   if (success)
   {
      writer.addRegisteredTag(pTag, false, ownerIndex, tagType);
      return true;
   }

   return false;
}

bool Nitf::TrePlugInResource::exportMetadata(const RasterDataDescriptor &descriptor, 
   const RasterFileDescriptor &exportDescriptor, ossimNitfWriter &writer, string &errorMessage) const
{
   const TreParser* pParser = dynamic_cast<const TreParser*>(get());
   VERIFY(pParser != NULL);

   string treErrorMessage;
   FactoryResource<DynamicObject> pTre;
   VERIFY(pTre.get() != NULL);

   unsigned int ownerIndex = 1;
   string tagType = "IXSHD";

   TreExportStatus status = pParser->exportMetadata(descriptor, exportDescriptor,
      *pTre.get(), ownerIndex, tagType, treErrorMessage);
   if (treErrorMessage.empty() == false)
   {
      errorMessage = getArgs().mPlugInName + ": " + treErrorMessage;
   }

   string writeErrorMessage;
   switch (status)
   {
   case REPLACE:
      writeTag(*pTre.get(), ownerIndex, ossimString(tagType), writer, writeErrorMessage);
      if (!writeErrorMessage.empty())
      {
         errorMessage += "," + writeErrorMessage;
      }
      return true;
   case REMOVE:
      return true;
   default:
      break;
   }
   
   return false;
}

TreState Nitf::TrePlugInResource::validateTag(const DynamicObject& tag, ostream& reporter) const
{
   const TreParser* pParser = dynamic_cast<const TreParser*>(get());
   VERIFYRV(pParser != NULL, INVALID);
   return pParser->isTreValid(tag, reporter);
}

namespace
{
   class SetStringFromStream
   {
   public:
      SetStringFromStream(string &str, ostringstream &stream) : mStr(str), mStream(stream)
      {
      }

      ~SetStringFromStream()
      {
         mStr = mStream.str();
      }
   private:
      string& mStr;
      ostringstream& mStream;
   };
}

bool Nitf::importMetadata(const unsigned int& currentImage, const Nitf::OssimFileResource& pFile,
   const ossimNitfFileHeaderV2_X* pFileHeader, const ossimNitfImageHeaderV2_X* pImageSubheader,
   RasterDataDescriptor *pDescriptor, string &errorMessage)
{
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Separate the file header parsing " \
   "from the subheader parsing (dadkins)")

   VERIFY(pFileHeader != NULL && pImageSubheader != NULL && pDescriptor != NULL);

   DynamicObject* pMetadata = pDescriptor->getMetadata();
   VERIFY(pMetadata != NULL);

   ostringstream errorStream;
   SetStringFromStream stringSetter(errorMessage, errorStream);

   // Add metadata for the NITF File Header and the supported subheaders
   const string fileVersion = pFileHeader->getVersion();
   Nitf::FileHeader fileHeader(fileVersion);
   if (fileHeader.importMetadata(pFileHeader, pDescriptor) == false)
   {
      return false;
   }

   unsigned int numDes = pFileHeader->getNumberOfDataExtSegments();
   for (unsigned int i = 0; i < numDes; ++i)
   {
      ossimNitfDataExtensionSegment* pDes = pFile->getNewDataExtensionSegment(i);
      if (pDes != NULL)
      {
         Nitf::DesSubheader desSubheader(fileVersion, i);
         if (desSubheader.importMetadata(pDes, pDescriptor) == false)
         {
            delete pDes;
            return false;
         }

         delete pDes;
      }
   }

   Nitf::ImageSubheader imageSubheader(fileVersion);
   if (imageSubheader.importMetadata(pImageSubheader, pDescriptor) == false)
   {
      return false;
   }

   // Now do the TREs
   FactoryResource<DynamicObject> pTres;
   FactoryResource<DynamicObject> pTreInfo;

   const unsigned int numImageTags = pImageSubheader->getNumberOfTags();
   for (unsigned int imageTag = 0; imageTag < numImageTags; ++imageTag)
   {
      ossimNitfTagInformation tagInfo;
      if (pImageSubheader->getTagInformation(tagInfo, imageTag) == false)
      {
         errorStream << "Unable to retrieve tag #" << imageTag << " from the image subheader." << endl;
      }
      else
      {
         addTagToMetadata(currentImage, tagInfo, pDescriptor, pTres.get(), pTreInfo.get(), errorMessage);
      }
   }

   const unsigned int numFileTags = pFileHeader->getNumberOfTags();
   for (unsigned int fileTag = 0; fileTag < numFileTags; ++fileTag)
   {
      ossimNitfTagInformation tagInfo;
      if (pFileHeader->getTagInformation(tagInfo, fileTag) == false)
      {
         errorStream << "Unable to retrieve tag #" << fileTag << " from the file header." << endl;
      }
      else
      {
         // For file headers, currentImage is always 0.
         addTagToMetadata(0, tagInfo, pDescriptor, pTres.get(), pTreInfo.get(), errorMessage);
      }
   }

   // FTITLE parsing requires the metadata object to be populated first
   pMetadata->setAttributeByPath(Nitf::NITF_METADATA + "/" + Nitf::TRE_METADATA, *pTres.get());
   pMetadata->setAttributeByPath(Nitf::NITF_METADATA + "/" + Nitf::TRE_INFO_METADATA, *pTreInfo.get());

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Consider moving this into ChipConverter (leckels)")
   // We have two equations of the form:
   // FI_x = a*OP_x + b*OP_y + c and
   // FI_y = d*OP_x + e*OP_y + f
   //
   // The coefficients a-f are as follows (from Todd's implementation of FTITLE -> ICHIPB Coefficients
   // a = 1.0;
   // b = 0.0
   // c = (row-1)*x_pixel_block_size, where row comes from FTITLE
   // d = 0.0
   // e = 1.0
   // f = (column-1)*y_pixel_block_size, where column comes from FTITLE
   //
   // Solving for OP_x, OP_y, and substituting for a-f, we get:
   // FI_x - c = OP_x
   // FI_y - f = OP_y
   // Building the ICHIPB requires solving for OP_x and OP_y, since a-f can be derived from the FTITLE

   // Special case if NTM: build and insert an ICHIPB if it is missing
   // trust the ICHIPB more than the other tags; only create an ICHIPB if none exist
   if (pTres->getAttribute("STDIDB").isValid() == true && pTres->getAttribute("ICHIPB").isValid() == false)
   {
      int x_pixel_block_size = pImageSubheader->getNumberOfPixelsPerBlockHoriz();
      int y_pixel_block_size = pImageSubheader->getNumberOfPixelsPerBlockVert();

      unsigned int imgRows = pImageSubheader->getNumberOfRows();
      unsigned int imgCols = pImageSubheader->getNumberOfCols();

      int blockRow = 1;
      int blockCol = 1;
      //the ftitle we only need a small portion of, we can extract
      //the displayed blocks included in this image from a properly
      //named ftitle.  

      string ftitle = pFileHeader->getTitle();
      string::size_type index = ftitle.find_first_of('_');
      if (index != string::npos)
      {
         string fubstring = ftitle.substr(index+1);
         if (fubstring.size() > 16)
         {
            char tempRow[3] = {0};
            char tempCol[6] = {0};
            istringstream strm(fubstring);
            if (strm.good())
            {
               strm.read(tempRow, 2);
            }

            stringstream t(tempRow);
            t >> blockRow;

            if (strm.good())
            {
               strm.read(tempCol, 5);
            }

            t.clear();
            t << tempCol;
            t >> blockCol;
         }
      }
      // blockRow, blockCol are 1-based
      --blockRow;
      --blockCol;

      // compute the start coordinates of the scene using original numbers to produce FI_X, FI_Y numbers
      double startRow = 0.5 + y_pixel_block_size * blockRow;
      double startCol = 0.5 + x_pixel_block_size * blockCol;

      LocationType fi_11(startCol, startRow);
      LocationType fi_12(startCol + imgCols-1, startRow);
      LocationType fi_21(startCol, startRow + imgRows-1);
      LocationType fi_22(startCol + imgCols-1, startRow + imgRows-1);

      LocationType op_11;
      LocationType op_12;
      LocationType op_21;
      LocationType op_22;
      op_11.mX = fi_11.mX - blockCol*x_pixel_block_size;
      op_11.mY = fi_11.mY - blockRow*y_pixel_block_size;
      op_12.mX = fi_12.mX - blockCol*x_pixel_block_size;
      op_12.mY = fi_12.mY - blockRow*y_pixel_block_size;
      op_21.mX = fi_21.mX - blockCol*x_pixel_block_size;
      op_21.mY = fi_21.mY - blockRow*y_pixel_block_size;
      op_22.mX = fi_22.mX - blockCol*x_pixel_block_size;
      op_22.mY = fi_22.mY - blockRow*y_pixel_block_size;

      FactoryResource<DynamicObject> pIchipb;
      VERIFY(pIchipb.get() != NULL);

      FactoryResource<DynamicObject> pInstance;
      VERIFY(pInstance.get() != NULL);

      if (pInstance->setAttribute(ICHIPB::XFRM_FLAG, 0) && pInstance->setAttribute(ICHIPB::SCALE_FACTOR, 1) &&
          pInstance->setAttribute(ICHIPB::ANAMRPH_CORR, 0) && pInstance->setAttribute(ICHIPB::SCANBLK_NUM, 0) &&
          pInstance->setAttribute(ICHIPB::FI_COL_11, fi_11.mX) &&
          pInstance->setAttribute(ICHIPB::FI_COL_12, fi_12.mX) &&
          pInstance->setAttribute(ICHIPB::FI_COL_21, fi_21.mX) &&
          pInstance->setAttribute(ICHIPB::FI_COL_22, fi_22.mX) &&
          pInstance->setAttribute(ICHIPB::FI_ROW_11, fi_11.mY) &&
          pInstance->setAttribute(ICHIPB::FI_ROW_12, fi_12.mY) &&
          pInstance->setAttribute(ICHIPB::FI_ROW_21, fi_21.mY) &&
          pInstance->setAttribute(ICHIPB::FI_ROW_22, fi_22.mY) &&
          pInstance->setAttribute(ICHIPB::OP_COL_11, op_11.mX) &&
          pInstance->setAttribute(ICHIPB::OP_COL_12, op_12.mX) &&
          pInstance->setAttribute(ICHIPB::OP_COL_21, op_21.mX) &&
          pInstance->setAttribute(ICHIPB::OP_COL_22, op_22.mX) &&
          pInstance->setAttribute(ICHIPB::OP_ROW_11, op_11.mY) &&
          pInstance->setAttribute(ICHIPB::OP_ROW_12, op_12.mY) &&
          pInstance->setAttribute(ICHIPB::OP_ROW_21, op_21.mY) &&
          pInstance->setAttribute(ICHIPB::OP_ROW_22, op_22.mY))
      {
      }
      else
      {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : TODO: Error - unable to set values (leckels)")
         return false;
      }

      pIchipb->setAttribute("0", *pInstance.get()); // first instance of ICHIPB

      pTres->setAttribute("ICHIPB", *pIchipb.get());
   }
   return true;
}

bool Nitf::addTagToMetadata(const unsigned int& ownerIndex, const ossimNitfTagInformation& tagInfo,
   RasterDataDescriptor* pDescriptor, DynamicObject* pTres, DynamicObject* pTreInfo, string& errorMessage)
{
   // Verify that input is valid
   ossimString tagName = tagInfo.getTagName();
   VERIFY(tagName.empty() == false);
   VERIFY(pTres != NULL);
   VERIFY(pTreInfo != NULL);

   // Try to parse the TRE with a specialized parser.
   FactoryResource<DynamicObject> pTag;
   VERIFY(pTag.get() != NULL);

   ossimRefPtr<ossimNitfRegisteredTag> pRegTag = tagInfo.getTagData();
   VERIFY(pRegTag.get() != NULL);

   TrePlugInResource pPlugIn(tagName);
   if (pPlugIn.parseTag(*pRegTag.get(), *pTag.get(), *pDescriptor, errorMessage) == false)
   {
      // Failing that, use UnknownTreParser.
      pTag->clear();
      TrePlugInResource pUnknownPlugIn(UnknownTreParser::PLUGIN_NAME);
      VERIFY(pUnknownPlugIn.get() != NULL);

      if (pUnknownPlugIn.parseTag(*pRegTag.get(), *pTag.get(), *pDescriptor, errorMessage) == false)
      {
         errorMessage += tagName + " has not been imported.\n";
         return false;
      }
   }

   // Parse the TRE info.
   FactoryResource<DynamicObject> pTagInfo;
   VERIFY(pTagInfo.get() != NULL);
   VERIFY(pTagInfo->setAttribute("Tag Type", string(tagInfo.getTagType())));
   VERIFY(pTagInfo->setAttribute("Owner Index", ownerIndex));

   // Determine how many (if any) tags of this name already exist.
   // Use this value to generate a unique name for this tag.
   unsigned int instance = 0;
   DynamicObject* pParentDynObj = pTres->getAttribute(tagName).getPointerToValue<DynamicObject>();
   if (pParentDynObj != NULL)
   {
      instance = pParentDynObj->getNumAttributes();
   }

   // Build the TRE and the TRE_INFO entries
   stringstream strm;
   strm << tagName << "/" << instance;

   VERIFY(pTres->setAttributeByPath(strm.str(), *pTag.get()));
   VERIFY(pTreInfo->setAttributeByPath(strm.str(), *pTagInfo.get()));
   return true;
}

bool Nitf::exportMetadata(const RasterDataDescriptor *pDescriptor, 
   const RasterFileDescriptor *pExportDescriptor, ossimNitfWriter *pNitf, Progress *pProgress)
{
   VERIFY(pDescriptor != NULL && pExportDescriptor != NULL && pNitf != NULL);

   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   VERIFY(pMetadata != NULL);

   ossimRefPtr<ossimProperty> pFileProp = pNitf->getProperty("file_header");
   VERIFY(pFileProp.get() != NULL && pFileProp->canCastTo("ossimContainerProperty"));

   Nitf::FileHeader fileHeader(Nitf::VERSION_02_10);
   if (fileHeader.exportMetadata(pDescriptor, PTR_CAST(ossimContainerProperty, pFileProp.get())) == false)
   {
      return false;
   }
   pNitf->setProperty(pFileProp.get());

   string pDesPath[] = { NITF_METADATA, DES_METADATA, END_METADATA_NAME };
   const DynamicObject* pDesMetadata = pMetadata->getAttributeByPath(pDesPath).getPointerToValue<DynamicObject>();
   if (pDesMetadata != NULL)
   {
      unsigned int numDes = pDesMetadata->getNumAttributes();
      for (unsigned int des = 0; des < numDes; des++)
      {
         ossimRefPtr<ossimProperty> pDesProp = pNitf->getProperty("des_header");
         ossimContainerProperty* pDesContainer = PTR_CAST(ossimContainerProperty, pDesProp.get());
         VERIFY(pDesContainer != NULL);

         Nitf::DesSubheader desSubheader(Nitf::VERSION_02_10, des);
         if (desSubheader.exportMetadata(pDescriptor, pDesContainer) == false)
         {
            return false;
         }

         vector<ossimRefPtr<ossimProperty> > propertyList;
         pDesContainer->getPropertyList(propertyList);

         ossimNitfDataExtensionSegmentV2_1 newDes;
         newDes.setProperties(propertyList);
         pNitf->addDataExtensionSegment(newDes, false);
      }
   }

   ossimRefPtr<ossimProperty> pImageProp = pNitf->getProperty("image_header");
   VERIFY(pImageProp != NULL && pImageProp->canCastTo("ossimContainerProperty"));

   Nitf::ImageSubheader imageSubheader(Nitf::VERSION_02_10);
   if (imageSubheader.exportMetadata(pDescriptor, PTR_CAST(ossimContainerProperty, pImageProp.get())) == false)
   {
      return false;
   }

   pNitf->setProperty(pImageProp.get());

   Service<PlugInManagerServices> pPlugInMgr;
   vector<PlugInDescriptor*> treParserDescriptors = pPlugInMgr->getPlugInDescriptors(TreParser::Type());

   set<string> skipTreNames;
   for (vector<PlugInDescriptor*>::iterator iter = treParserDescriptors.begin();
      iter != treParserDescriptors.end(); ++iter)
   {
      if ((*iter != NULL) && (*iter)->getSubtype() == Nitf::TreParser::CreateOnExportSubtype())
      {
         string name = (*iter)->getName();
         TrePlugInResource pTrePlugIn(name);
         LOG_IF(pTrePlugIn.get() == NULL, continue); 

         string errorMessage;
         if (pTrePlugIn.exportMetadata(*pDescriptor, *pExportDescriptor, *pNitf, errorMessage))
         {
            skipTreNames.insert(name);
         }

         if (!errorMessage.empty() && pProgress != NULL)
         {
            pProgress->updateProgress(errorMessage, 0, WARNING);
         }
      }
   }

   string pTrePath[] = { NITF_METADATA, TRE_METADATA, END_METADATA_NAME };
   const DynamicObject* pTres = pMetadata->getAttributeByPath(pTrePath).getPointerToValue<DynamicObject>();

   string pTreInfoPath[] = { NITF_METADATA, TRE_INFO_METADATA, END_METADATA_NAME };
   const DynamicObject* pTreInfos = pMetadata->getAttributeByPath(pTreInfoPath).getPointerToValue<DynamicObject>();
   if (pTres != NULL && pTreInfos != NULL)
   {
      vector<string> names;
      pTres->getAttributeNames(names);

      for (vector<string>::const_iterator nameIter = names.begin(); nameIter != names.end(); ++nameIter)
      {
         if (skipTreNames.find(*nameIter) != skipTreNames.end())
         {
            continue;
         }
         const DynamicObject* pTreInstances = pTres->getAttribute(*nameIter).getPointerToValue<DynamicObject>();
         if (!NN(pTreInstances))
         {
            continue;
         }

         const DynamicObject* pTreInfoInstances = pTreInfos->getAttribute(*nameIter).getPointerToValue<DynamicObject>();
         if (!NN(pTreInfoInstances))
         {
            continue;
         }

         TrePlugInResource pPlugIn(*nameIter);

         vector<string> treInstanceNames;
         pTreInstances->getAttributeNames(treInstanceNames);
         for (vector<string>::const_iterator instanceIter = treInstanceNames.begin();
            instanceIter != treInstanceNames.end(); ++instanceIter)
         {
            const DynamicObject* pTre = pTreInstances->getAttribute(*instanceIter).getPointerToValue<DynamicObject>();
            if (!NN(pTre))
            {
               continue;
            }

            const DynamicObject* pTreInfo = pTreInfoInstances->getAttribute(
               *instanceIter).getPointerToValue<DynamicObject>();
            if (!NN(pTreInfo))
            {
               continue;
            }

            unsigned int ownerIndex;
            if (pTreInfo->getAttribute("Owner Index").getValue(ownerIndex) == false)
            {
               continue;
            }

            string tagType;
            if (pTreInfo->getAttribute("Tag Type").getValue(tagType) == false)
            {
               continue;
            }

            string errorMessage;
            if (!pPlugIn.writeTag(*pTre, ownerIndex, ossimString(tagType), *pNitf, errorMessage))
            {
               if (pProgress != NULL)
               {
                  pProgress->updateProgress(*nameIter + " failed to export: " + errorMessage, 0, WARNING);
               }
            }
            else if (!errorMessage.empty())
            {
               if (pProgress != NULL)
               {
                  pProgress->updateProgress(*nameIter + ": " + errorMessage, 0, WARNING);
               }
            }
         }
      }
   }

   return true;
}
