/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ICE_READER_H
#define ICE_READER_H

#include "IceUtilities.h"

#include <hdf5.h>

#include <string>

class DataElement;
class DynamicObject;
class Hdf5Dataset;
class Hdf5File;
class Hdf5Group;
class ImportDescriptor;
class Layer;
class PseudocolorLayer;
class RasterDataDescriptor;
class RasterElement;
class SpatialDataView;

class IceReader
{
public:
   IceReader(Hdf5File& iceFile);

   enum ValidityTypeEnum
   {
      FULLY_SUPPORTED, /**< This is an ice file and a version of the file supported by this importer */
      NOT_SUPPORTED, /**< This is an ice file but not a version of the file supported by this importer */
      NOT_ICE, /**< This is not an ice file at all */
   };
   typedef EnumWrapper<ValidityTypeEnum> ValidityType;

   ValidityType isValidIceFile();
   IceUtilities::FileType getFileType();

   std::vector<ImportDescriptor*> getImportDescriptors();

   const std::vector<std::string>& getWarnings();
   const std::vector<std::string>& getErrors();

   bool loadCubeStatistics(RasterElement* pElement);
   bool createLayer(const std::string& hdfPath, SpatialDataView* pView, DataElement* pParent, std::string& warningMessage);

private:
   void readFormatDescriptor();
   ImportDescriptor* getImportDescriptor(const Hdf5Group* pCube);

   bool parseDimensionDescriptors(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor);
   bool parseMetadata(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor);
   bool parseClassification(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor);
   DynamicObject* parseDynamicObject(const Hdf5Dataset* pDynObjDs, const std::string& elementTag);
   bool parseGcps(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor);
   bool parseUnits(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor);
   bool parseDisplayInformation(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor);
   Layer* parseAndCreateLayer(const Hdf5Group* pLayerGroup, SpatialDataView* pView,
      DataElement* pParent, std::string& warningMessage);
   bool parsePseudocolorLayer(const Hdf5Group* pPseudocolorLayerGroup,
      PseudocolorLayer* pPseudocolorLayer, std::string& warningMessage);


   Hdf5File& mIceFile;
   ValidityType mIceFileValid;
   IceFormatDescriptor mIceDescriptor;
   std::vector<std::string> mWarnings;
   std::vector<std::string> mErrors;
};

#endif
