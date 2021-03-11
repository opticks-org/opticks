/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "ImportDescriptor.h"
#include "Mie4NitfImporter.h"
#include "ModelServices.h"
#include "NitfFileHeader.h"
#include "NitfMetadataParsing.h"
#include "NitfResource.h"
#include "NitfTreParser.h"
#include "NitfUtilities.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"

#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimNitfFile.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_X.h>
#include <ossim/support_data/ossimNitfImageHeader.h>
#include <ossim/support_data/ossimNitfImageHeaderV2_X.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>

#include <sstream>
#include <vector>

REGISTER_PLUGIN(OpticksNitf, Mie4NitfImporter, Nitf::Mie4NitfImporter);

namespace
{
   // Defined in NGA.STND.0044_1.3.1
   // Matched section indices are below.
   QRegExp mie4nitf_regexp("(.*)\\."      // 1:Match base portion of the name. Should probably limit to valid filename characters but we won't get an invalid filename into the methods
                           "(r[0-9]+)"    // 2:Spatial resolution factor
                           "(t[0-9]+)"    // 3:Temporal resolution factor
                           "(q[0-9]+)"    // 4:Compression quality
                           "(c[0-9]+)?"   // 5:Camera set. If there's only one camera set this may be missing
                           "(i[0-9]+)"    // 6:Index of the frame set. 0 for the manifest file.
                           "\\.ni?tf",    // 7:.nitf and .ntf are both valid.
      Qt::CaseInsensitive);
}

Nitf::Mie4NitfImporter::Mie4NitfImporter()
{
   setName("MIE4NITF Importer");
   setDescriptorId("{98DFF415-793F-4084-9214-73D588F8FED5}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Nitf::Mie4NitfImporter::~Mie4NitfImporter()
{}

unsigned char Nitf::Mie4NitfImporter::getFileAffinity(const std::string& filename)
{
   auto rval = NitfImporterShell::getFileAffinity(filename);
   if (rval == Importer::CAN_LOAD)
   {
      rval = Importer::CAN_NOT_LOAD;
      // quick check on the filename, there's a specific format for MIE4NITF
      QFileInfo fname(QString::fromStdString(filename));
      if (mie4nitf_regexp.exactMatch(fname.fileName()) && mie4nitf_regexp.cap(6) == "i0")  // only use the manifest file
      {
         Nitf::OssimFileResource pNitfFile(filename);
         if (pNitfFile.get() == nullptr)
         {
            return rval;
         }
         const ossimNitfFileHeaderV2_X* pFileHeader =
            dynamic_cast<const ossimNitfFileHeaderV2_X*>(pNitfFile->getHeader());
         if (pFileHeader == nullptr)
         {
            return rval;
         }
         for (auto tagIdx = 0; tagIdx < pFileHeader->getNumberOfTags(); ++tagIdx)
         {
            ossimNitfTagInformation tagInfo;
            if (pFileHeader->getTagInformation(tagInfo, tagIdx))
            {
               if (tagInfo.getTagName() == "MIMCSA")
               {
                  rval = Importer::CAN_LOAD + 10;
                  break;
               }
            }
         }
      }
   }
   return rval;
}

std::vector<ImportDescriptor*> Nitf::Mie4NitfImporter::getImportDescriptors(const std::string &filename)
{
   std::vector<ImportDescriptor*> descriptors;
   Nitf::OssimFileResource pNitfFile(filename);
   VERIFYRV(pNitfFile.get() != nullptr, descriptors);
   const ossimNitfFileHeaderV2_X* pFileHeader =
      dynamic_cast<const ossimNitfFileHeaderV2_X*>(pNitfFile->getHeader());
   VERIFYRV(pFileHeader != nullptr, descriptors);
   // ossim does not currently support text headers in NITF 2.1 files but we don't really need to read it
   // since we can infer all the filenames stored in the segment. We can't verify that a segment is actually
   // the MIE4NITF index but there will only be one of those in the file. We make the following assumptions:
   //   The manifest file follows the filename convention.
   //   The manifest file has at least one text segment and it will be the correct type.
   //   The presence of any text segments in a file with the correct name indicates this is a manifest file.
   // Overall, these are reasonable assumption for compliant datasets.
   if (pFileHeader->getNumberOfTextSegments() > 0)
   {
      // Load all the file header TREs
      // We need a RDD and ImportDescriptor to call the parsing methods so we'll create
      // a dummy one. It'll get cleaned up when we leave the scope
      ImportDescriptorResource pImportDescriptor(filename, TypeConverter::toString<RasterElement>(), nullptr);
      VERIFYRV(pImportDescriptor.get() != nullptr, descriptors);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      VERIFYRV(pDescriptor != nullptr, descriptors);

      const std::string fileVersion = pFileHeader->getVersion();
      Nitf::FileHeader fileHeader(fileVersion);

      unsigned int layerNum = 0;
      const unsigned int numFileTags = pFileHeader->getNumberOfTags();
      for (unsigned int fileTag = 0; fileTag < numFileTags; ++fileTag)
      {
         ossimNitfTagInformation tagInfo;
         if (pFileHeader->getTagInformation(tagInfo, fileTag))
         {
            if (tagInfo.getTagName() == "MIMCSA")
            {
               FactoryResource<DynamicObject> pMimcsa;
               VERIFYRV(pMimcsa.get() != nullptr, descriptors);
               ossimRefPtr<ossimNitfRegisteredTag> pRegTag = tagInfo.getTagData();
               VERIFYRV(pRegTag.get() != nullptr, descriptors);
               TrePlugInResource pParser("MIMCSA");
               if (pParser.get() != nullptr)
               {
                  std::string errorMessage;
                  if (pParser.parseTag(*pRegTag.get(), *pMimcsa.get(), *pDescriptor, errorMessage))
                  {
                     // successfully parsed a MIMCSA, build an import descriptor for it
                     ImportDescriptor* pDesc = getImportDescriptor(filename, layerNum, pNitfFile, pFileHeader, *pMimcsa.get());
                     if (pDesc != nullptr)
                     {
                        descriptors.push_back(pDesc);
                        layerNum++;
                     }
                  }
               }
            }
         }
      }
   }

   return descriptors;
}

ImportDescriptor* Nitf::Mie4NitfImporter::getImportDescriptor(
   const std::string& filename,
   ossim_uint32 layerNum,
   const Nitf::OssimFileResource& pFile,
   const ossimNitfFileHeaderV2_X* pFileHeader,
   const DynamicObject& mimcsa)
{
   VERIFYRV(pFileHeader != nullptr, nullptr);

   std::string layerName = pFileHeader->getTitle().trim();
   if (layerName.empty())
   {
      layerName = filename;
   }
   layerName += " :: ";
   auto layerId = dv_cast<std::string>(mimcsa.getAttribute(TRE::MIMCSA::LAYER_ID));
   layerName += layerId;

   ImportDescriptorResource pImportDescriptor(layerName, TypeConverter::toString<RasterElement>(), nullptr);
   VERIFYRV(pImportDescriptor.get() != nullptr, nullptr);

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
   VERIFYRV(pDescriptor != nullptr, nullptr);

   // Populate the metadata and set applicable values in the data descriptor
   std::map<std::string, TrePlugInResource> parsers;
   std::string errorMessage;
   if (!Nitf::importMetadata(0, pFile, pFileHeader, nullptr, pDescriptor, parsers, errorMessage))
   {
      return nullptr;
   }
   // Populate specific fields in the data descriptor or file descriptor from the TREs
   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   VERIFYRV(pMetadata, nullptr);

   struct CamData
   {
      std::string cameraId;
      unsigned int idlvl;
      unsigned int lalvl;
      Opticks::PixelOffset iloc;
      unsigned int rows;
      unsigned int cols;
   };
   std::map<unsigned int, CamData> cameraData;
   const DynamicObject& camSdas = dv_cast<DynamicObject>(pMetadata->getAttributeByPath(Nitf::NITF_METADATA + "/" + Nitf::TRE_METADATA + "/" + Nitf::TRE::CAMSDA::TAG));
   for (auto camSdaIt = camSdas.begin(); camSdaIt != camSdas.end(); ++camSdaIt)
   {
      const DynamicObject& camSda = dv_cast<const DynamicObject>(camSdaIt->second);
      const DynamicObject& cameraSets = dv_cast<const DynamicObject>(camSda.getAttribute(Nitf::TRE::CAMSDA::CAMERA_SETS));
      for (auto cameraSetIt = cameraSets.begin(); cameraSetIt != cameraSets.end(); ++cameraSetIt)
      {
         const DynamicObject& cameras = dv_cast<const DynamicObject>(dv_cast<const DynamicObject>(cameraSetIt->second).getAttribute(Nitf::TRE::CAMSDA::CAMERAS));
         for (auto cameraIt = cameras.begin(); cameraIt != cameras.end(); ++cameraIt)
         {
            const DynamicObject& camera = dv_cast<const DynamicObject>(cameraIt->second);
            auto cameraLayerName = dv_cast<std::string>(camera.getAttribute(Nitf::TRE::CAMSDA::LAYER_ID));
            if (cameraLayerName == layerId)
            {
               // This camera is in this layer
               auto cameraId = dv_cast<std::string>(camera.getAttribute(Nitf::TRE::CAMSDA::CAMERA_ID));
               auto idlvl = dv_cast<unsigned int>(camera.getAttribute(Nitf::TRE::CAMSDA::IDLVL));
               auto ialvl = dv_cast<unsigned int>(camera.getAttribute(Nitf::TRE::CAMSDA::IALVL));
               const Opticks::PixelOffset& iloc = dv_cast<const Opticks::PixelOffset>(camera.getAttribute(Nitf::TRE::CAMSDA::ILOC));
               auto rows = dv_cast<unsigned int>(camera.getAttribute(Nitf::TRE::CAMSDA::NROWS));
               auto cols = dv_cast<unsigned int>(camera.getAttribute(Nitf::TRE::CAMSDA::NCOLS));
               cameraData.insert(std::make_pair(idlvl, CamData{cameraId, idlvl, ialvl, iloc, rows, cols}));
            }
         }
      }
   }
   // Now we have all the camera data we can determine the full size of the layer element
   if (cameraData.size() > 1)
   {
      // TODO: NOT YET SUPPORTED
      return nullptr;
   }

   loadFileInfo(filename, pDescriptor->getName(), layerId);

   unsigned int totalRows = cameraData[1].rows;
   unsigned int totalCols = cameraData[1].cols;

   std::vector<DimensionDescriptor> bands = RasterUtilities::generateDimensionVector(1, true, false, true);
   pDescriptor->setBands(bands);

   std::vector<DimensionDescriptor> rows = RasterUtilities::generateDimensionVector(totalRows, true, false, true);
   pDescriptor->setRows(rows);

   std::vector<DimensionDescriptor> cols = RasterUtilities::generateDimensionVector(totalCols, true, false, true);
   pDescriptor->setColumns(cols);

   pDescriptor->setInterleaveFormat(BSQ);

   EncodingType dataType(INT2UBYTES);
   pDescriptor->setDataType(dataType);
   pDescriptor->setValidDataTypes(std::vector<EncodingType>(1, dataType));
   pDescriptor->setProcessingLocation(IN_MEMORY);

   // Set the file descriptor
   RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(
      RasterUtilities::generateAndSetFileDescriptor(pDescriptor, filename, layerName, LITTLE_ENDIAN_ORDER));
   if (pFileDescriptor == nullptr)
   {
      return nullptr;
   }

   // Set the bits per element, which may be different than the data type in the data descriptor,
   // using NBPP instead of ABPP as is done in ossimNitfTileSource.cpp.
   unsigned int bitsPerPixel = static_cast<unsigned int>(16);
   pFileDescriptor->setBitsPerElement(bitsPerPixel);

   pDescriptor->setDisplayBand(GRAY, bands[0]);
   pDescriptor->setDisplayMode(GRAYSCALE_MODE);

   mParseMessages[layerNum] = errorMessage;

   return pImportDescriptor.release();
}

void Nitf::Mie4NitfImporter::loadFileInfo(const std::string& indexfile, const std::string& parentName, const std::string& layerId)
{
   QFileInfo fname(QString::fromStdString(indexfile));
   if (!mie4nitf_regexp.exactMatch(fname.fileName()))
   {
      return;
   }
   std::vector<std::string> parent{parentName};
   auto fdir = fname.dir();
   QString filt = QString("%1.%2%3%4i*.*tf").arg(mie4nitf_regexp.cap(1)).arg(mie4nitf_regexp.cap(2)).arg(mie4nitf_regexp.cap(3)).arg(mie4nitf_regexp.cap(4));
   auto files = fdir.entryInfoList(QStringList() << filt, QDir::Files, QDir::Name);
   for (auto file = files.begin(); file != files.end(); ++file)
   {
      if (file->fileName() == fname.fileName())
      {
         continue;
      }
      std::string fname(file->absoluteFilePath().toStdString());
      Nitf::OssimFileResource pNitfFile(file->absoluteFilePath().toStdString());
      VERIFYNRV(pNitfFile.get() != nullptr);
      const ossimNitfFileHeaderV2_X* pFileHeader =
         dynamic_cast<const ossimNitfFileHeaderV2_X*>(pNitfFile->getHeader());
      VERIFYNRV(pFileHeader != nullptr);
      for (int iidx = 0; iidx < pFileHeader->getNumberOfImages(); ++iidx)
      {
         DataDescriptorResource<RasterDataDescriptor> pDescriptor(dynamic_cast<RasterDataDescriptor*>(Service<ModelServices>()->createDataDescriptor(
            fname + "[" + StringUtilities::toDisplayString(iidx + 1) + "]", TypeConverter::toString<RasterElement>(), parent)));
         const ossimNitfImageHeaderV2_X* pImgHeader = dynamic_cast<const ossimNitfImageHeaderV2_X*>(pNitfFile->getNewImageHeader(iidx));
         VERIFYNRV(pImgHeader != nullptr);

         std::map<std::string, TrePlugInResource> parsers;
         std::string errorMessage;
         if (!Nitf::importMetadata(iidx, pNitfFile, pFileHeader, pImgHeader, pDescriptor.get(), parsers, errorMessage))
         {
            continue;
         }
         const DynamicObject* pMetadata = pDescriptor->getMetadata();
         VERIFYNRV(pMetadata);
         try
         {
            const DynamicObject& mtimsas = dv_cast<DynamicObject>(pMetadata->getAttributeByPath(Nitf::NITF_METADATA + "/" + Nitf::TRE_METADATA + "/" + Nitf::TRE::MTIMSA::TAG));
            const DynamicObject& mtimsa = dv_cast<DynamicObject>(mtimsas.getAttribute("0"));
            if (dv_cast<std::string>(mtimsa.getAttribute(Nitf::TRE::MTIMSA::LAYER_ID)) != layerId)
            {
               // TODO: Make this more efficient by loading and caching all layer info in one pass
               continue;
            }
            auto frame_count = dv_cast<unsigned int>(mtimsa.getAttribute(Nitf::TRE::MTIMSA::NUMBER_FRAMES));
         }
         catch (std::bad_cast&)
         {
            continue;  // image segment that isn't motion imagery so we'll just ignore it
         }
      }
   }
}