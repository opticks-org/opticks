/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ICEWRITER_H
#define ICEWRITER_H

#include "EnumWrapper.h"
#include "Hdf5Resource.h"
#include "IceUtilities.h"
#include "StringUtilities.h"

#include <hdf5.h>
#include <string>

class Classification;
class DynamicObject;
class Layer;
class PseudocolorLayer;
class RasterElement;
class RasterFileDescriptor;
class Progress;

enum IceCompressionTypeEnum
{
   NONE,
   GZIP,
   SHUFFLE_AND_GZIP
};

typedef EnumWrapper<IceCompressionTypeEnum> IceCompressionType;

class IceWriter
{
public:
   SETTING(CompressionType, IceWriter, std::string, std::string());
   SETTING(GzipCompressionLevel, IceWriter, int, 0);
   SETTING(ChunkSize, IceWriter, int, 0);

   IceWriter(hid_t fileHandle, IceUtilities::FileType fileType);
   void writeFileHeader();
   void writeCube(const std::string& hdfPath, RasterElement* pCube,
      const RasterFileDescriptor* pOutputFileDescriptor, Progress* pProgress);
   void writeLayer(const std::string& hdfPath, const std::string& datasetPath,
      const Layer* pLayer, Progress* pProgress);
   void abort();
   void setChunkSize(int chunkSize);
   void setCompressionType(IceCompressionType type);
   void setGzipCompressionLevel(int level);

   int getChunkSize() const;
   IceCompressionType getCompressionType() const;
   int getGzipCompressionLevel() const;

private:
   void writeBilCubeData(const std::string& hdfPath,
      RasterElement* pCube,
      const RasterFileDescriptor* pOutputFileDescriptor,
      Hdf5DataSetResource& dataId,
      Progress* pProgress);
   void writeBipCubeData(const std::string& hdfPath,
      RasterElement* pCube,
      const RasterFileDescriptor* pOutputFileDescriptor,
      Hdf5DataSetResource& dataId,
      Progress* pProgress);
   void writeBsqCubeData(const std::string& hdfPath,
      RasterElement* pCube,
      const RasterFileDescriptor* pOutputFileDescriptor,
      Hdf5DataSetResource& dataId,
      Progress* pProgress);
   void writeDynamicObject(const DynamicObject* pDynObj,
      const std::string& datasetName,
      const std::string& rootElementName);
   void writeClassification(const Classification* pClassification,
      const std::string& groupName,
      Progress* pProgress);
   void createDatasetForCube(hsize_t dimSpace[3], hsize_t chunkSpace[],
      EncodingType encoding, hid_t fd, const std::string& hdfPath,
      Hdf5DataSetResource& dataset);
   void writePseudocolorLayerProperties(const std::string& hdfPath,
      const PseudocolorLayer* pLayer, Progress*pProgress);
   void writeLayerProperties(const std::string& hdfPath,
      const Layer* pLayer, Progress*pProgress);

   void abortIfNecessary();

   hid_t mFileHandle;
   bool mAborted;
   IceUtilities::FileType mFileType;
   int mChunkSize;
   IceCompressionType mCompressionType;
   int mGzipCompressionLevel;
};

namespace StringUtilities
{
   template<>
   std::string toDisplayString(const IceCompressionType& value, bool* pError);
   template<>
   std::string toXmlString(const IceCompressionType& value, bool* pError);
   template<>
   IceCompressionType fromDisplayString<IceCompressionType>(std::string valueText, bool* pError);
   template<>
   IceCompressionType fromXmlString<IceCompressionType>(std::string valueText, bool* pError);
}

#endif
