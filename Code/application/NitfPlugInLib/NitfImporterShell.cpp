/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFile>
#include <QtCore/QString>

#include "AppAssert.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "CachedPager.h"
#include "Classification.h"
#include "DataDescriptor.h"
#include "DataVariant.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "GeoConversions.h"
#include "GeoPoint.h"
#include "ImportDescriptor.h"
#include "NitfConstants.h"
#include "NitfImporterShell.h"
#include "NitfMetadataParsing.h"
#include "NitfResource.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterPager.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"
#include "TypeConverter.h"
#include "TypesFile.h"

#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimNitfFile.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_X.h>
#include <ossim/support_data/ossimNitfImageHeaderV2_X.h>
#include <ossim/imaging/ossimNitfTileSource.h>

#include <list>
#include <sstream>
#include <vector>

using namespace std;

//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : TODO: The NULL pix value for OSSIM_PARTIAL " \
//   "is a bad value (leckels)")

Nitf::NitfImporterShell::NitfImporterShell()
{
   setExtensions("NITF Files (*.ntf *.NTF *.nitf *.NITF *.r0 *.R0)");
}

Nitf::NitfImporterShell::~NitfImporterShell()
{}

vector<ImportDescriptor*> Nitf::NitfImporterShell::getImportDescriptors(const string &filename)
{
   vector<ImportDescriptor*> retval;

   if (filename.empty())
   {
      return retval;
   }

   Nitf::OssimFileResource pFile(filename);
   if (pFile.get() == NULL)
   {
      return retval;
   }

   Nitf::OssimImageHandlerResource pHandler(filename);
   if (pHandler.get() == NULL || pHandler->canCastTo("ossimNitfTileSource") == false)
   {
      return retval;
   }

   ossimNitfFileHeaderV2_X* pFileHeader =
      PTR_CAST(ossimNitfFileHeaderV2_X, pFile->getHeader().get());
   if (pFileHeader == NULL)
   {
      return retval;
   }

   // Not all segments are importable.  This is generally due to the segment
   // using an unsupported compression format.  Only generate descriptors
   // for the importable segments.
   vector<ossim_uint32> importableImageSegments;
   pHandler->getEntryList(importableImageSegments);

   // Create map of TRE parsers.
   // The sole purpose of this map is to force DLLs to stay loaded while the metadata is being imported.
   std::map<std::string, TrePlugInResource> parsers;

   for (vector<ossim_uint32>::iterator segmentIter = importableImageSegments.begin();
        segmentIter != importableImageSegments.end(); ++segmentIter)
   {
      // Do not call pHandler->setCurrentEntry as it is a very expensive operation
      // which causes up to a several second delay on files with many large images.
      const ossim_uint32& currentIndex = *segmentIter;
      ossimNitfImageHeaderV2_X* pImgHeader =
         PTR_CAST(ossimNitfImageHeaderV2_X, pFile->getNewImageHeader(currentIndex));
      if (pImgHeader == NULL)
      {
         continue;
      }

      EncodingType dataType = ossimImageHeaderToEncodingType(pImgHeader);
      if (dataType.isValid() == false)
      {
         continue;
      }

      stringstream imageNameStream;
      imageNameStream << "I" << currentIndex + 1;
      string imageName = imageNameStream.str();

      ImportDescriptorResource pImportDescriptor(getImportDescriptor(filename, imageName,
         pFile.get(), pFileHeader, pImgHeader));
      if (pImportDescriptor.get() == NULL)
      {
         continue;
      }

      RasterDataDescriptor* pDd = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      VERIFYRV(pDd != NULL, retval);

      vector<DimensionDescriptor> bands = 
         RasterUtilities::generateDimensionVector(pImgHeader->getNumberOfBands(), true, false, true);
      pDd->setBands(bands);

      vector<DimensionDescriptor> rows = 
         RasterUtilities::generateDimensionVector(pImgHeader->getNumberOfRows(), true, false, true);
      pDd->setRows(rows);

      vector<DimensionDescriptor> cols = 
         RasterUtilities::generateDimensionVector(pImgHeader->getNumberOfCols(), true, false, true);
      pDd->setColumns(cols);

      if (pImgHeader->getIMode() == "P")
      {
         pDd->setInterleaveFormat(BIP);
      }
      else if (pImgHeader->getIMode() == "R")
      {
         pDd->setInterleaveFormat(BIL);
      }
      else
      {
         pDd->setInterleaveFormat(BSQ);
      }

      pDd->setDataType(dataType);
      pDd->setValidDataTypes(vector<EncodingType>(1, dataType));
      pDd->setProcessingLocation(IN_MEMORY);

      RasterFileDescriptor* pFd = dynamic_cast<RasterFileDescriptor*>(
         RasterUtilities::generateAndSetFileDescriptor(pDd, filename, imageName, LITTLE_ENDIAN_ORDER));
      VERIFYRV(pFd != NULL, retval);

      string errorMessage;
      if (Nitf::importMetadata(currentIndex + 1, pFile, pFileHeader, pImgHeader, pDd, parsers, errorMessage) == true)
      {
         // Populate specific fields in the data descriptor or file descriptor from the TREs
         const DynamicObject* pMeta = pDd->getMetadata();
         VERIFYRV(pMeta, retval);

         // Pixel size - This info is contained in multiple TREs, but there is no documentation on which
         // TRE contains the more precise value if multiple TREs containing the info are present.  Choosing
         // the order ACFTA, BANDSA, ACFTB, and BANDSB where the later "B" TREs will overwrite the values
         // contained in the earlier "A" TREs.  The BANDSB TRE contains GSD values for each band, which is
         // currently not supported, so only set the pixel size if the values in all bands are the same.
         double xGsd = 1.0;
         double yGsd = 1.0;

         const string acrftaPath[] =
         {
            Nitf::NITF_METADATA,
            Nitf::TRE_METADATA,
            "ACFTA",
            "0",
            END_METADATA_NAME
         };

         const DynamicObject* pAcrftA = dv_cast<DynamicObject>(&pMeta->getAttributeByPath(acrftaPath));
         if (pAcrftA != NULL)
         {
            // The ACFTA spec calls out specific spacing units for "SAR" and "EO-IR" data, but does not indicate how
            // this is determined.  It seems to be related to the ACFTB SENSOR_ID_TYPE field, but that field is not
            // present in the ACFTA TRE.  So just check for "SAR" data from the ICAT field in the image subheader
            // and assume every other data type is "EO-IR" data.
            const string imageCategory = pImgHeader->getCategory().trim();

            const DataVariant& rowSpacing = pAcrftA->getAttribute(Nitf::TRE::ACFTA::ROW_SPACING);
            if (rowSpacing.isValid() == true)
            {
               if (imageCategory == "SAR")
               {
                  yGsd = getGsd(rowSpacing, "f");  // Feet
               }
               else
               {
                  yGsd = getGsd(rowSpacing, "r");  // Micro-radians
               }
            }

            const DataVariant& columnSpacing = pAcrftA->getAttribute(Nitf::TRE::ACFTA::COL_SPACING);
            if (columnSpacing.isValid() == true)
            {
               if (imageCategory == "SAR")
               {
                  xGsd = getGsd(columnSpacing, "f");  // Feet
               }
               else
               {
                  xGsd = getGsd(columnSpacing, "r");  // Micro-radians
               }
            }
         }

         const string bandsaPath[] =
         {
            Nitf::NITF_METADATA,
            Nitf::TRE_METADATA,
            "BANDSA",
            "0",
            END_METADATA_NAME
         };

         const DynamicObject* pBandsA = dv_cast<DynamicObject>(&pMeta->getAttributeByPath(bandsaPath));
         if (pBandsA != NULL)
         {
            const DataVariant& rowSpacing = pBandsA->getAttribute(Nitf::TRE::BANDSA::ROW_SPACING);
            if (rowSpacing.isValid() == true)
            {
               const DataVariant& rowSpacingUnits = pBandsA->getAttribute(Nitf::TRE::BANDSA::ROW_SPACING_UNITS);
               if (rowSpacingUnits.isValid() == true)
               {
                  yGsd = getGsd(rowSpacing, rowSpacingUnits.toXmlString());
               }
            }

            const DataVariant& columnSpacing = pBandsA->getAttribute(Nitf::TRE::BANDSA::COL_SPACING);
            if (columnSpacing.isValid() == true)
            {
               const DataVariant& columnSpacingUnits = pBandsA->getAttribute(Nitf::TRE::BANDSA::COL_SPACING_UNITS);
               if (columnSpacingUnits.isValid() == true)
               {
                  xGsd = getGsd(columnSpacing, columnSpacingUnits.toXmlString());
               }
            }
         }

         const string acrftbPath[] =
         {
            Nitf::NITF_METADATA,
            Nitf::TRE_METADATA,
            "ACFTB",
            "0",
            END_METADATA_NAME
         };

         const DynamicObject* pAcrftB = dv_cast<DynamicObject>(&pMeta->getAttributeByPath(acrftbPath));
         if (pAcrftB != NULL)
         {
            const DataVariant& rowSpacing = pAcrftB->getAttribute(Nitf::TRE::ACFTB::ROW_SPACING);
            if (rowSpacing.isValid() == true)
            {
               const DataVariant& rowSpacingUnits = pAcrftB->getAttribute(Nitf::TRE::ACFTB::ROW_SPACING_UNITS);
               if (rowSpacingUnits.isValid() == true)
               {
                  yGsd = getGsd(rowSpacing, rowSpacingUnits.toXmlString());
               }
            }

            const DataVariant& columnSpacing = pAcrftB->getAttribute(Nitf::TRE::ACFTB::COL_SPACING);
            if (columnSpacing.isValid() == true)
            {
               const DataVariant& columnSpacingUnits = pAcrftB->getAttribute(Nitf::TRE::ACFTB::COL_SPACING_UNITS);
               if (columnSpacingUnits.isValid() == true)
               {
                  xGsd = getGsd(columnSpacing, columnSpacingUnits.toXmlString());
               }
            }
         }

         const string bandsbPath[] =
         {
            Nitf::NITF_METADATA,
            Nitf::TRE_METADATA,
            "BANDSB",
            "0",
            END_METADATA_NAME
         };

         const DynamicObject* pBandsB = dv_cast<DynamicObject>(&pMeta->getAttributeByPath(bandsbPath));
         if (pBandsB != NULL)
         {
            bool validRowGsd = false;

            const DataVariant& rowGsd = pBandsB->getAttribute(Nitf::TRE::BANDSB::ROW_GSD);
            if (rowGsd.isValid() == true)
            {
               const DataVariant& rowGsdUnits = pBandsB->getAttribute(Nitf::TRE::BANDSB::ROW_GSD_UNIT);
               if (rowGsdUnits.isValid() == true)
               {
                  yGsd = getGsd(rowGsd, rowGsdUnits.toXmlString());
                  validRowGsd = true;
               }
            }

            if (validRowGsd == false)
            {
               if (pBandsB->getAttribute(Nitf::TRE::BANDSB::ROW_GSD + "#0").isValid())
               {
                  double commonYGsd = -1.0;

                  unsigned int numBands = pDd->getBandCount();
                  for (unsigned int i = 0; i < numBands; ++i)
                  {
                     double bandYGsd = -1.0;
                     string bandPostfix = "#" + StringUtilities::toDisplayString(i);

                     const DataVariant& bandRowGsd = pBandsB->getAttribute(Nitf::TRE::BANDSB::ROW_GSD + bandPostfix);
                     if (bandRowGsd.isValid() == true)
                     {
                        const DataVariant& bandRowGsdUnits = pBandsB->getAttribute(Nitf::TRE::BANDSB::ROW_GSD_UNIT +
                           bandPostfix);
                        if (bandRowGsdUnits.isValid() == true)
                        {
                           bandYGsd = getGsd(bandRowGsd, bandRowGsdUnits.toXmlString());
                        }
                     }

                     if (bandYGsd == commonYGsd)
                     {
                        continue;
                     }

                     if (commonYGsd != -1.0)
                     {
                        commonYGsd = -1.0;
                        break;
                     }

                     commonYGsd = bandYGsd;
                  }

                  if (commonYGsd != 1.0)
                  {
                     yGsd = commonYGsd;
                  }
               }
            }

            bool validColumnGsd = false;

            const DataVariant& columnGsd = pBandsB->getAttribute(Nitf::TRE::BANDSB::COL_GSD);
            if (columnGsd.isValid() == true)
            {
               const DataVariant& columnGsdUnits = pBandsB->getAttribute(Nitf::TRE::BANDSB::COL_GSD_UNITS);
               if (columnGsdUnits.isValid() == true)
               {
                  xGsd = getGsd(columnGsd, columnGsdUnits.toXmlString());
                  validColumnGsd = true;
               }
            }

            if (validColumnGsd == false)
            {
               if (pBandsB->getAttribute(Nitf::TRE::BANDSB::COL_GSD + "#0").isValid())
               {
                  double commonXGsd = -1.0;

                  unsigned int numBands = pDd->getBandCount();
                  for (unsigned int i = 0; i < numBands; ++i)
                  {
                     double bandXGsd = -1.0;
                     string bandPostfix = "#" + StringUtilities::toDisplayString(i);

                     const DataVariant& bandRowGsd = pBandsB->getAttribute(Nitf::TRE::BANDSB::COL_GSD + bandPostfix);
                     if (bandRowGsd.isValid() == true)
                     {
                        const DataVariant& bandRowGsdUnits = pBandsB->getAttribute(Nitf::TRE::BANDSB::COL_GSD_UNIT +
                           bandPostfix);
                        if (bandRowGsdUnits.isValid() == true)
                        {
                           bandXGsd = getGsd(bandRowGsd, bandRowGsdUnits.toXmlString());
                        }
                     }

                     if (bandXGsd == commonXGsd)
                     {
                        continue;
                     }

                     if (commonXGsd != -1.0)
                     {
                        commonXGsd = -1.0;
                        break;
                     }

                     commonXGsd = bandXGsd;
                  }

                  if (commonXGsd != 1.0)
                  {
                     xGsd = commonXGsd;
                  }
               }
            }
         }

         double magFactor = 1.0;
         ossimString imag = pImgHeader->getImageMagnification().trim();
         if (imag.empty() == false)
         {
            // Need to multiply the GSD values by the image magnification (IMAG) value in the image subheader
            if (imag[0] == '/')
            {
               ossimString reciprocal = imag.substr(1);
               magFactor = 1.0 / reciprocal.toDouble();
            }
            else
            {
               magFactor = imag.toDouble();
            }

            xGsd *= magFactor;
            yGsd *= magFactor;
         }

         pDd->setXPixelSize(xGsd);
         pDd->setYPixelSize(yGsd);

         // Higher precision GCPs
         const string blockaPath[] =
         {
            Nitf::NITF_METADATA,
            Nitf::TRE_METADATA,
            "BLOCKA",
            "0",
            END_METADATA_NAME
         };

         const DynamicObject* pBlockA = dv_cast<DynamicObject>(&pMeta->getAttributeByPath(blockaPath));
         if (pBlockA != NULL)
         {
            const DataVariant& blockLines = pBlockA->getAttribute(Nitf::TRE::BLOCKA::L_LINES);
            if (blockLines.isValid() == true)
            {
               unsigned int numBlockRows = 0;
               if (blockLines.getValue<unsigned int>(numBlockRows) == true)
               {
                  // Need to multiply the number of rows by the image magnification (IMAG) value in the image subheader
                  numBlockRows = static_cast<unsigned int>(static_cast<double>(numBlockRows) * magFactor);
                  if (numBlockRows == pFd->getRowCount())
                  {
                     list<GcpPoint> updatedGcps;

                     list<GcpPoint> gcps = pFd->getGcps();
                     for (list<GcpPoint>::iterator iter = gcps.begin(); iter != gcps.end(); ++iter)
                     {
                        GcpPoint gcp = *iter;
                        string coordinateText;

                        list<GcpPoint>::size_type index = updatedGcps.size();
                        if (index == 0)
                        {
                           const DataVariant& gcp1 = pBlockA->getAttribute(Nitf::TRE::BLOCKA::FRFC_LOC);
                           if (gcp1.isValid() == true)
                           {
                              coordinateText = gcp1.toXmlString();
                           }
                        }
                        else if (index == 1)
                        {
                           const DataVariant& gcp2 = pBlockA->getAttribute(Nitf::TRE::BLOCKA::FRLC_LOC);
                           if (gcp2.isValid() == true)
                           {
                              coordinateText = gcp2.toXmlString();
                           }
                        }
                        else if (index == 2)
                        {
                           const DataVariant& gcp3 = pBlockA->getAttribute(Nitf::TRE::BLOCKA::LRLC_LOC);
                           if (gcp3.isValid() == true)
                           {
                              coordinateText = gcp3.toXmlString();
                           }
                        }
                        else if (index == 3)
                        {
                           const DataVariant& gcp4 = pBlockA->getAttribute(Nitf::TRE::BLOCKA::LRFC_LOC);
                           if (gcp4.isValid() == true)
                           {
                              coordinateText = gcp4.toXmlString();
                           }
                        }

                        if (StringUtilities::isAllBlank(coordinateText) == false)
                        {
                           coordinateText.insert(10, ", ");
                           LatLonPoint latLon(coordinateText);
                           gcp.mCoordinate = latLon.getCoordinates();
                        }

                        updatedGcps.push_back(gcp);
                     }

                     pFd->setGcps(updatedGcps);
                  }
               }
            }
         }

         // This only checks the first BANDSB. It is possible to have multiple BANDSB TREs.
         // If someone runs across real data where the bad band info is in another BANDSB TRE
         // this code will need to be modified.
         if (pBandsB != NULL && pBandsB->getAttribute(Nitf::TRE::BANDSB::BAD_BAND + "#0").isValid())
         {
            const std::vector<DimensionDescriptor>& curBands = pDd->getBands();
            std::vector<DimensionDescriptor> newBands;
            for (size_t idx = 0; idx < curBands.size(); ++idx)
            {
               const int* pVal = dv_cast<int>(&pBandsB->getAttribute(
                  Nitf::TRE::BANDSB::BAD_BAND + "#" + StringUtilities::toDisplayString(idx)));
               if (pVal == NULL || *pVal == 1) // 0 == invalid or suspect band, 1 = valid band
               {
                  newBands.push_back(curBands[idx]);
               }
            }
            pDd->setBands(newBands);
         }
         if (pImgHeader->hasTransparentCode() == true)
         {
            vector<int> badValues;
            badValues.push_back(static_cast<int>(pImgHeader->getTransparentCode()));
            pDd->setBadValues(badValues);
         }

         // If red, green, OR blue bands are valid, set the display mode to RGB.
         if (pDd->getDisplayBand(RED).isValid() == true ||
             pDd->getDisplayBand(GREEN).isValid() == true ||
             pDd->getDisplayBand(BLUE).isValid() == true)
         {
            pDd->setDisplayMode(RGB_MODE);
         }
         // Otherwise, if the gray band is valid, set the display mode to GRAYSCALE.
         else if (pDd->getDisplayBand(GRAY).isValid() == true)
         {
            pDd->setDisplayMode(GRAYSCALE_MODE);
         }
         // Otherwise, if at least 3 bands are available, set the display mode to RGB,
         // and set the first three bands to red, green, and blue respectively.
         else if (bands.size() >= 3)
         {
            pDd->setDisplayBand(RED, bands[0]);
            pDd->setDisplayBand(GREEN, bands[1]);
            pDd->setDisplayBand(BLUE, bands[2]);
            pDd->setDisplayMode(RGB_MODE);
         }
         // Otherwise, if at least 1 band is available, set the display mode to GRAYSCALE,
         // and set the first band to GRAY.
         else if (bands.empty() == false)
         {
            pDd->setDisplayBand(GRAY, bands[0]);
            pDd->setDisplayMode(GRAYSCALE_MODE);
         }
         else
         {
            continue;
         }

         mParseMessages[imageName] = errorMessage;
         retval.push_back(pImportDescriptor.release());
      }
   }

   return retval;
}

unsigned char Nitf::NitfImporterShell::getFileAffinity(const string& filename)
{
   // Check that the file exists
   FileResource pFile(filename.c_str(), "rb");
   if (pFile.get() == NULL)
   {
      return Importer::CAN_NOT_LOAD;
   }

   // Check the file header and version (9 bytes plus a terminating NULL)
   char buffer[10];
   memset(buffer, NULL, sizeof(buffer));
   fgets(buffer, sizeof(buffer), pFile);

   if (buffer == Nitf::NITF_METADATA + Nitf::VERSION_02_00 ||
       buffer == Nitf::NITF_METADATA + Nitf::VERSION_02_10)
   {
      return Importer::CAN_LOAD;
   }

   return Importer::CAN_NOT_LOAD;
}

SpatialDataView* Nitf::NitfImporterShell::createView() const
{
   SpatialDataView* pView = RasterElementImporterShell::createView();
   VERIFYRV(pView != NULL, pView);

   RasterElement* pRaster = getRasterElement();
   VERIFYRV(pRaster != NULL, pView);

   const DynamicObject* pMetadata = pRaster->getMetadata();
   VERIFYRV(pMetadata != NULL, pView);

   const string backgroundColorPath[] = { Nitf::NITF_METADATA, Nitf::FILE_HEADER,
      Nitf::FileHeaderFieldNames::BACKGROUND_COLOR, END_METADATA_NAME };

   const DataVariant& dvBackground = pMetadata->getAttributeByPath(backgroundColorPath);
   if (dvBackground.isValid() == true)
   {
      ColorType ctBackground;
      VERIFY(dvBackground.getValue(ctBackground) == true);
      pView->setBackgroundColor(ctBackground);
      pView->refresh();
   }

   return pView;
}

bool Nitf::NitfImporterShell::createRasterPager(RasterElement *pRaster) const
{
   VERIFY(pRaster != NULL);

   DataDescriptor* pDd = pRaster->getDataDescriptor();
   VERIFY(pDd != NULL);
   FileDescriptor* pFd = pDd->getFileDescriptor();
   VERIFY(pFd != NULL);

   const string& datasetLocation = pFd->getDatasetLocation();
   if (datasetLocation.empty() == true)
   {
      return false;
   }

   stringstream imageNameStream(datasetLocation.substr(1));
   int imageSegment;
   imageNameStream >> imageSegment;

   FactoryResource<Filename> pFn;
   pFn->setFullPathAndName(pFd->getFilename());

   ExecutableResource pPlugIn("NitfPager");
   pPlugIn->getInArgList().setPlugInArgValue("Segment Number", &imageSegment);
   pPlugIn->getInArgList().setPlugInArgValue(CachedPager::PagedElementArg(), pRaster);
   pPlugIn->getInArgList().setPlugInArgValue(CachedPager::PagedFilenameArg(), pFn.get());

   if (pPlugIn->execute() == true)
   {
      RasterPager* pPager = dynamic_cast<RasterPager*>(pPlugIn->getPlugIn());
      if (pPager != NULL)
      {
         pRaster->setPager(pPager);
         pPlugIn->releasePlugIn();
         return true;
      }
   }

   return false;
}

EncodingType Nitf::NitfImporterShell::ossimImageHeaderToEncodingType(ossimNitfImageHeaderV2_X* pImgHeader)
{
   VERIFYRV(pImgHeader != NULL, EncodingType());

   // Use NBPP not ABPP as is done in ossimNitfTileSource.cpp.
   const ossim_int32 bitsPerPixel = pImgHeader->getBitsPerPixelPerBand();
   const ossimString pixelValueType = pImgHeader->getPixelValueType().upcase();
   switch (bitsPerPixel)
   {
      case 8:
      {
         if (pixelValueType == "INT")
         {
            return INT1UBYTE;
         }

         if (pixelValueType == "SI")
         {
            return INT1SBYTE;
         }

         break;
      }
      case 16:
      {
         if (pixelValueType == "INT")
         {
            return INT2UBYTES;
         }

         if (pixelValueType == "SI")
         {
            return INT2SBYTES;
         }

         break;
      }
      case 32:
      {
         if(pixelValueType == "R")
         {
            return FLT4BYTES;
         }

         if (pixelValueType == "INT")
         {
            return INT4UBYTES;
         }

         if (pixelValueType == "SI")
         {
            return INT4SBYTES;
         }

         break;
      }
      case 64:
      {
         if (pixelValueType == "R")
         {
            return FLT8BYTES;
         }

         break;
      }
      default:
      {
         if (pImgHeader->isCompressed())
         {
            if (bitsPerPixel < 8)
            {
               return INT1UBYTE;
            }

            if (bitsPerPixel < 16)
            {
               return INT2UBYTES;
            }

            if (bitsPerPixel < 32)
            {
               return FLT4BYTES;
            }
         }

         break;
      }
   }

   return EncodingType();
}

bool Nitf::NitfImporterShell::validate(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   if (RasterElementImporterShell::validate(pDescriptor, errorMessage) == false)
   {
      ValidationTest errorTest = getValidationError();
      if (errorTest == NO_BAND_FILES)
      {
         const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
         VERIFY(pRasterDescriptor != NULL);

         if (pRasterDescriptor->getInterleaveFormat() == BSQ)
         {
            errorMessage += "  Bands in multiple files are not supported with on-disk read-only processing.";
         }
      }
      else if ((errorTest == NO_ROW_SUBSETS) || (errorTest == NO_COLUMN_SUBSETS))
      {
         errorMessage = errorMessage.substr(0, errorMessage.length() - 1);
         errorMessage += " with on-disk read-only processing.";
      }

      return false;
   }

   VERIFY(pDescriptor != NULL);

   const FileDescriptor* const pFileDescriptor = pDescriptor->getFileDescriptor();
   VERIFY(pFileDescriptor != NULL);

   map<string, string>::const_iterator iter = mParseMessages.find(pFileDescriptor->getDatasetLocation());
   if (iter != mParseMessages.end() && iter->second.empty() == false)
   {
      errorMessage += iter->second;
   }

   const qint64 actualSize = QFile(QString::fromStdString(pFileDescriptor->getFilename().getFullPathAndName())).size();
   const qint64 maxSize = numeric_limits<ossim_uint64>::max() & numeric_limits<std::streamoff>::max();
   if (actualSize > maxSize)
   {
      errorMessage += "This file exceeds the maximum supported size for this platform.\n"
         "Data at the end of the file may be missing or incorrect.\n"
         "IMPORT DATA FROM THIS FILE WITH THE KNOWLEDGE THAT IT IS NOT FULLY SUPPORTED.\n";
   }

   // warn user if unsupported metadata is in the file
   // NITF 2.0: ICORDS values of U and C are unsupported
   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   VERIFY(pMetadata != NULL);

   const string versionPathName[] = { Nitf::NITF_METADATA, Nitf::FILE_HEADER,
      Nitf::FileHeaderFieldNames::FILE_VERSION, END_METADATA_NAME };
   if (pMetadata->getAttributeByPath(versionPathName).toDisplayString() == Nitf::VERSION_02_00)
   {
      const string iCordsPath[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
         Nitf::ImageSubheaderFieldNames::ICORDS, END_METADATA_NAME };
      string iCords = pMetadata->getAttributeByPath(iCordsPath).toDisplayString();
      if (iCords == Nitf::ImageSubheaderFieldValues::ICORDS_GEOCENTRIC)
      {
         errorMessage += "The ICORDS is not a supported value.\n";
      }
   }

   // LUTs are unsupported
   bool hasLut = false;
   const string irepPathName[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
      Nitf::ImageSubheaderFieldNames::REPRESENTATION, END_METADATA_NAME };

   string irep;
   const DataVariant& dvIrep = pMetadata->getAttributeByPath(irepPathName);
   if (dvIrep.getValue(irep) == true)
   {
      if (irep == Nitf::ImageSubheaderFieldValues::REPRESENTATION_LUT)
      {
         hasLut = true;
      }
   }

   if (hasLut == false)
   {
      vector<string> irepBands;
      const string irepBandsPathName[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
         Nitf::ImageSubheaderFieldNames::BAND_REPRESENTATIONS, END_METADATA_NAME };
      const DataVariant& dvIrepBands = pMetadata->getAttributeByPath(irepBandsPathName);
      if (dvIrepBands.getValue(irepBands) == true)
      {
         for (vector<string>::iterator iter = irepBands.begin(); iter != irepBands.end(); iter++)
         {
            if (*iter == Nitf::ImageSubheaderFieldValues::BAND_REPRESENTATIONS_LUT)
            {
               hasLut = true;
               break;
            }
         }
      }
   }

   if (hasLut == false)
   {
      vector<unsigned int> numLuts;
      const string numLutsPathName[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
         Nitf::ImageSubheaderFieldNames::NUMBER_OF_LUTS, END_METADATA_NAME };
      const DataVariant& dvNumLuts = pMetadata->getAttributeByPath(numLutsPathName);
      if (dvNumLuts.getValue(numLuts) == true)
      {
         for (vector<unsigned int>::iterator iter = numLuts.begin(); iter != numLuts.end(); iter++)
         {
            if (*iter > 0)
            {
               hasLut = true;
               break;
            }
         }
      }
   }

   if (hasLut == true)
   {
      errorMessage += "The lookup table will not be applied.\n";
   }

   // Check for valid Classification markings. If any level is higher than the file header, display a warning.
   FactoryResource<Classification> pClassification;
   const Classification* pOverallClassification = pDescriptor->getClassification();
   VERIFY(pOverallClassification != NULL);

   // Look in the image subheader.
   string imageClassLevel;
   const string imageClassLevelPathName[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
      Nitf::ImageSubheaderFieldNames::SECURITY_LEVEL, END_METADATA_NAME };
   const DataVariant& dvImageClassLevel = pMetadata->getAttributeByPath(imageClassLevelPathName);
   if (dvImageClassLevel.getValue(imageClassLevel) == true)
   {
      pClassification->setLevel(imageClassLevel);
      if (pClassification->hasGreaterLevel(pOverallClassification) == true)
      {
         errorMessage += "THIS FILE CONTAINS INVALID CLASSIFICATION INFORMATION!\n"
            "The image has a higher classification level than the file.\n"
            "Update the Classification information before proceeding.\n";
      }
   }

   // Look in each DES subheader.
   int numDes;
   const string numDesPathName[] = { Nitf::NITF_METADATA, Nitf::FILE_HEADER,
      Nitf::FileHeaderFieldNames::NUM_DES, END_METADATA_NAME };
   const DataVariant& dvNumDes = pMetadata->getAttributeByPath(numDesPathName);
   if (dvNumDes.getValue(numDes) == true)
   {
      for (int i = 0; i < numDes; ++i)
      {
         stringstream desStr;
         desStr << "DES_" << setw(3) << setfill('0') << i;

         string desClassLevel;
         const string desClassLevelPathName[] = { Nitf::NITF_METADATA, Nitf::DES_METADATA,
            desStr.str(), Nitf::DesSubheaderFieldNames::SECURITY_LEVEL, END_METADATA_NAME };
         const DataVariant& dvDesClassLevel = pMetadata->getAttributeByPath(desClassLevelPathName);
         if (dvDesClassLevel.getValue(desClassLevel) == true)
         {
            pClassification->setLevel(desClassLevel);
            if (pClassification->hasGreaterLevel(pOverallClassification) == true)
            {
               errorMessage += "THIS FILE CONTAINS INVALID CLASSIFICATION INFORMATION!\n" +
                  desStr.str() + " has a higher classification level than the file.\n"
                  "Update the Classification information before proceeding.\n";
            }
         }
      }
   }

   return true;
}

int Nitf::NitfImporterShell::getValidationTest(const DataDescriptor* pDescriptor) const
{
   int validationTest = RasterElementImporterShell::getValidationTest(pDescriptor) | VALID_METADATA;
   if (pDescriptor != NULL)
   {
      if (pDescriptor->getProcessingLocation() == ON_DISK_READ_ONLY)
      {
         validationTest |= NO_BAND_FILES | NO_SUBSETS;
      }
   }

   return validationTest;
}

ImportDescriptor* Nitf::NitfImporterShell::getImportDescriptor(const string& filename, const string& imageName,
   const ossimNitfFile* pFile, const ossimNitfFileHeaderV2_X* pFileHeader,
   const ossimNitfImageHeaderV2_X* pImageSubheader)
{
   VERIFYRV(pImageSubheader != NULL, NULL);

   ImportDescriptorResource pImportDescriptor(filename + "-" + imageName,
      TypeConverter::toString<RasterElement>(), NULL);
   VERIFYRV(pImportDescriptor.get() != NULL, NULL);
   pImportDescriptor->setImported(pImageSubheader->getRepresentation() != "NODISPLY");
   return pImportDescriptor.release();
}

double Nitf::NitfImporterShell::getGsd(const DataVariant& spacing, const string& units) const
{
   if ((spacing.isValid() == false) || (units.empty() == true))
   {
      return 1.0;
   }

   double gsd = 1.0;
   if (spacing.getValue<double>(gsd) == false)
   {
      return 1.0;
   }

   // Convert the value to meters
   if (StringUtilities::toLower(units) == "f")        // Feet
   {
      gsd = GeoConversions::convertFeetToMeters(gsd);
   }
   else if (StringUtilities::toLower(units) == "r")   // Micro-radians
   {
      double nauticalMiles = GeoConversions::convertRadToNm(gsd * 1e-6);
      gsd = GeoConversions::convertNmToMeters(nauticalMiles);
   }

   return gsd;
}
