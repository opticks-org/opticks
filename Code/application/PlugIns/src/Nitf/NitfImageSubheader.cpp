/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "NitfImageSubheader.h"

#include "Classification.h"
#include "DynamicObject.h"
#include "GeoPoint.h"
#include "LocationType.h"
#include "ModelServices.h"
#include "NitfConstants.h"
#include "NitfHeader.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "SpecialMetadata.h"

#include <boost/lexical_cast.hpp>

#include <ossim/support_data/ossimNitfImageHeaderV2_0.h>
#include <ossim/support_data/ossimNitfImageHeaderV2_1.h>
#include <string>

#include <QtCore/QString>
using namespace std;


using namespace Nitf::ImageSubheaderFieldNames;
using namespace Nitf::ImageSubheaderFieldValues;
using boost::lexical_cast;
namespace
{
   const static string dateTimePath[] = {SPECIAL_METADATA_NAME, COLLECTION_DATE_TIME_METADATA_NAME, END_METADATA_NAME};
   const static string iCordsPath[] = {Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER, ICORDS, END_METADATA_NAME};
   const static string iGeoloPath[] = {Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER, IGEOLO, END_METADATA_NAME};
}

Nitf::ImageSubheader::ImageSubheader(const string &fileVersion) :
   Header(fileVersion)
{
   // Elements specific to NITF 02.00
   if (mFileVersion == Nitf::VERSION_02_00)
   {
      mElements.push_back(Element(DATETIME, ossimNitfImageHeaderV2_X::IDATIM_KW,
         importIDATIM_2_0,
         NULL));

      mElements.push_back(Element(ID_2, ossimNitfImageHeaderV2_X::IID2_KW, 
         importMetadataValue<string>, 
         NULL));

      mElements.push_back(Element(importGeoInformation_2_0, NULL));

      // 02.00 security strings
      mElements.push_back(Element(SECURITY_CODEWORDS, ossimNitfImageHeaderV2_0::ISCODE_KW, 
         importMetadataValue<string>, 
         NULL));
      mElements.push_back(Element(SECURITY_CTRL_AND_HANDL, ossimNitfImageHeaderV2_0::ISCTLH_KW, 
         importMetadataValue<string>, 
         NULL));
      mElements.push_back(Element(SECURITY_RELEASE_INSTRUCTIONS, ossimNitfImageHeaderV2_0::ISREL_KW, 
         importMetadataValue<string>, 
         NULL));
      mElements.push_back(Element(SECURITY_AUTH, ossimNitfImageHeaderV2_0::ISCAUT_KW, 
         importMetadataValue<string>, 
         NULL));
      mElements.push_back(Element(SECURITY_CTRL_NUM, ossimNitfImageHeaderV2_0::CTLN_KW, 
         importMetadataValue<string>, 
         NULL));
      mElements.push_back(Element(SECURITY_DOWNGRADE, ossimNitfImageHeaderV2_0::ISDWNG_KW, 
         importDateYYMMDD, 
         NULL));
      mElements.push_back(Element(SECURITY_CLASS_TEXT, ossimNitfImageHeaderV2_0::ISDEVT_KW, 
         importMetadataValue<string>, 
         NULL));
   }
   // Elements specific to NITF 02.10
   else if (mFileVersion == Nitf::VERSION_02_10)
   {
      mElements.push_back(Element(DATETIME, ossimNitfImageHeaderV2_X::IDATIM_KW,
         importIDATIM,
         exportIDATIM));

      mElements.push_back(Element(ID_2, ossimNitfImageHeaderV2_X::IID2_KW, 
         importMetadataValue<string>, 
         exportMetadataValue<string>));

      mElements.push_back(Element(importGeoInformation, exportGeoInformation));

      // 02.10 Security strings
      mElements.push_back(Element(SECURITY_AUTH, ossimNitfImageHeaderV2_1::ISCAUT_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_CTRL_AND_HANDL, ossimNitfImageHeaderV2_1::ISCTLH_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_CTRL_NUM, ossimNitfImageHeaderV2_1::ISCTLN_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_RELEASE_INSTRUCTIONS, ossimNitfImageHeaderV2_1::ISREL_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_SYSTEM, ossimNitfImageHeaderV2_1::ISCLSY_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_CLASS_TEXT, ossimNitfImageHeaderV2_1::ISCLTX_KW,
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_CLASS_REASON, ossimNitfImageHeaderV2_1::ISCRSN_KW,
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_CODEWORDS, ossimNitfImageHeaderV2_1::ISCODE_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_DECLASS_EXEMPT, ossimNitfImageHeaderV2_1::ISDCXM_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_DOWNGRADE, ossimNitfImageHeaderV2_1::ISDG_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_AUTH_TYPE, ossimNitfImageHeaderV2_1::ISCATP_KW, 
         importMetadataValue<string>, 
         exportClassificationString));
      mElements.push_back(Element(SECURITY_DECLASS_TYPE, ossimNitfImageHeaderV2_1::ISDCTP_KW,
         importMetadataValue<string>, 
         exportClassificationString));

      // 02.10 Security dates
      mElements.push_back(Element(SECURITY_DECLASS_DATE, ossimNitfImageHeaderV2_1::ISDCDT_KW, 
         importDateCCYYMMDD, 
         exportClassificationDate));
      mElements.push_back(Element(SECURITY_DOWNGRADE_DATE, ossimNitfImageHeaderV2_1::ISDGDT_KW, 
         importDateCCYYMMDD, 
         exportClassificationDate));
      mElements.push_back(Element(SECURITY_SOURCE_DATE, ossimNitfImageHeaderV2_1::ISSRDT_KW, 
         importDateCCYYMMDD, 
         exportClassificationDate));
   }

   // Elements not specific to version
   mElements.push_back(Element(ID_1, ossimNitfImageHeaderV2_X::IID1_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(SOURCE, ossimNitfImageHeaderV2_X::ISORCE_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(TARGET_ID, ossimNitfImageHeaderV2_X::TGTID_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(REPRESENTATION, ossimNitfImageHeaderV2_X::IREP_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(CATEGORY, ossimNitfImageHeaderV2_X::ICAT_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(ENCRYPTION, ossimNitfImageHeaderV2_X::ENCRYP_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(MAGNIFICATION, ossimNitfImageHeaderV2_X::IMAG_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(MODE, ossimNitfImageHeaderV2_X::IMODE_KW, 
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(BLOCKS_PER_ROW, ossimNitfImageHeaderV2_X::NBPR_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(BLOCKS_PER_COLUMN, ossimNitfImageHeaderV2_X::NBPC_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(PIXELS_PER_BLOCK_HORIZONTAL, ossimNitfImageHeaderV2_X::NPPBH_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(PIXELS_PER_BLOCK_VERTICAL, ossimNitfImageHeaderV2_X::NPPBV_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(BITS_PER_PIXEL, ossimNitfImageHeaderV2_X::NBPP_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(ACTUAL_BITS_PER_PIXEL, ossimNitfImageHeaderV2_X::ABPP_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(DISPLAY_LEVEL, ossimNitfImageHeaderV2_X::IDLVL_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(ATTACHMENT_LEVEL, ossimNitfImageHeaderV2_X::IALVL_KW, 
      importMetadataValue<unsigned int>, 
      exportMetadataValue<unsigned int>));
   mElements.push_back(Element(COMPRESSION, ossimNitfImageHeaderV2_X::IC_KW,
      importMetadataValue<string>,
      exportMetadataValue<string>));
   mElements.push_back(Element(COMPRESSION_RATIO, ossimNitfImageHeaderV2_X::COMRAT_KW,
      importMetadataValue<string>,
      exportMetadataValue<string>));
   mElements.push_back(Element(PIXEL_VALUE_TYPE, ossimNitfImageHeaderV2_X::PVTYPE_KW,
      importMetadataValue<string>, 
      exportMetadataValue<string>));
   mElements.push_back(Element(PIXEL_JUSTIFICATION, ossimNitfImageHeaderV2_X::PJUST_KW,
      importMetadataValue<string>,
      exportMetadataValue<string>));

   // This element represents band information (IREPBANDn, ISUBCATn, NLUTSn).
   // Note that band information is not exported from the metadata.
   // This is done because ambiguities arise when exporting from the metadata and the view (which takes precedence?)
   mElements.push_back(Element(importBandInformation, NULL));

   mElements.push_back(Element(LOCATION, ossimNitfImageHeaderV2_X::ILOC_KW, 
      importILOC, 
      exportILOC));

   // Common security strings
   mElements.push_back(Element(SECURITY_LEVEL, ossimNitfImageHeaderV2_X::ISCLAS_KW, 
      importMetadataValue<string>, 
      exportClassificationString));
}

string Nitf::ImageSubheader::getMetadataPath() const
{
   return Nitf::NITF_METADATA + "/" + Nitf::IMAGE_SUBHEADER;
}

FactoryResource<DynamicObject> Nitf::ImageSubheader::createDefaultsDynamicObject(
   const RasterDataDescriptor *pDescriptor)
{
   FactoryResource<DynamicObject> pImageSubheader;
   return pImageSubheader;
}

bool Nitf::ImageSubheader::importIDATIM_2_0(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor,
   DynamicObject *pDynObj, const string& appName,
   const string& ossimName)
{
   if (pDescriptor == NULL || pDynObj == NULL)
   {
      return false;
   }

   DynamicObject* pMetadata = pDescriptor->getMetadata();
   if (pMetadata == NULL)
   {
      return false;
   }

   if (importDateDDHHMMSSZMONYY(pPropertyInterface, pDescriptor, pDynObj,
                                appName, ossimName) == false)
   {
      return false;
   }

   return pMetadata->setAttributeByPath(dateTimePath, pDynObj->getAttribute(appName));
}

bool Nitf::ImageSubheader::importIDATIM(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor,
   DynamicObject *pDynObj, const string& appName,
   const string& ossimName)
{
   if (pDescriptor == NULL || pDynObj == NULL)
   {
      return false;
   }

   DynamicObject* pMetadata = pDescriptor->getMetadata();
   if (pMetadata == NULL)
   {
      return false;
   }

   if (importDateCCYYMMDDhhmmss(pPropertyInterface, pDescriptor, pDynObj,
                                appName, ossimName) == false)
   {
      return false;
   }

   return pMetadata->setAttributeByPath(dateTimePath, pDynObj->getAttribute(appName));
}

bool Nitf::ImageSubheader::exportIDATIM(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   if (pDescriptor == NULL)
   {
      return false;
   }

   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   if (pMetadata == NULL)
   {
      return false;
   }

   return exportDateCCYYMMDDhhmmss(pDescriptor, pMetadata->getAttributeByPath(dateTimePath),
                                   pProperties, appName, ossimName);
}

bool Nitf::ImageSubheader::importMetadata(const ossimPropertyInterface *pHeader, RasterDataDescriptor *pDescriptor)
{
   VERIFY(pHeader != NULL && pDescriptor != NULL);
   FactoryResource<DynamicObject> pImageHeaderMetadata;
   VERIFY(pImageHeaderMetadata.get() != NULL);

   ossimNitfImageHeaderV2_X *pImageHeader = PTR_CAST(ossimNitfImageHeaderV2_X, pHeader);
   VERIFY(pImageHeader != NULL);
   VERIFY(Header::importMetadata(pImageHeader, pDescriptor, pImageHeaderMetadata.get()));

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : TODO: Fix OSSIM to parse comments and return the size of the vector (leckels)")

   VERIFY(pDescriptor != NULL);
   DynamicObject *pMetadata = pDescriptor->getMetadata();
   VERIFY(pMetadata != NULL);
   pMetadata->setAttributeByPath(getMetadataPath(), *pImageHeaderMetadata.get());
   return true;
}

bool Nitf::ImageSubheader::importBandInformation(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor,
   DynamicObject *pDynObj, const string& appName,
   const string& ossimName)
{
   ossimNitfImageHeaderV2_X *pImageHeader = PTR_CAST(ossimNitfImageHeaderV2_X, pPropertyInterface);
   if (pImageHeader == NULL || pDescriptor == NULL || pDynObj == NULL)
   {
      return false;
   }

   string numBandsStr;
   unsigned int numBands = pImageHeader->getNumberOfBands();
   if (numBands == 0)
   {
      return false;
   }

   if (numBands < 10)
   {
      numBandsStr = NBANDS;
   }
   else
   {
      numBandsStr = XBANDS;
   }

   vector<unsigned int> pNumLuts;
   vector<string> pBandRepresentations;
   vector<string> pBandSignificances;
   for (unsigned int i = 0; i < numBands; i++)
   {
      ossimRefPtr<ossimNitfImageBand> pBandInformation = pImageHeader->getBandInformation(i);

      ossimNitfImageBandV2_1* pImageBand = dynamic_cast<ossimNitfImageBandV2_1*>(pBandInformation.get());
      if (pImageBand != NULL)
      {
         unsigned int numLuts = static_cast<unsigned int>(pImageBand->getNumberOfLuts());
         pNumLuts.push_back(numLuts);

         string bandSignificance = pImageBand->getBandSignificance().trim();
         pBandSignificances.push_back(bandSignificance);

         string bandRepresentation = pImageBand->getBandRepresentation().trim();
         pBandRepresentations.push_back(bandRepresentation);

         RasterChannelType channelType;
         if (bandRepresentation == BAND_REPRESENTATIONS_RED)
         {
            channelType = RED;
         }
         else if (bandRepresentation == BAND_REPRESENTATIONS_GREEN)
         {
            channelType = GREEN;
         }
         else if (bandRepresentation == BAND_REPRESENTATIONS_BLUE)
         {
            channelType = BLUE;
         }
         else if (bandRepresentation == BAND_REPRESENTATIONS_MONO)
         {
            channelType = GRAY;
         }

         if (channelType.isValid() == true)
         {
            DimensionDescriptor band = pDescriptor->getDisplayBand(channelType);
            if (band.isValid() == false)
            {
               DimensionDescriptor dim = pDescriptor->getActiveBand(i);
               pDescriptor->setDisplayBand(channelType, dim);
            }
         }
      }
   }

   return pDynObj->setAttribute(NUMBER_OF_LUTS, pNumLuts) &&
      pDynObj->setAttribute(numBandsStr, numBands) &&
      pDynObj->setAttribute(BAND_REPRESENTATIONS, pBandRepresentations) &&
      pDynObj->setAttribute(BAND_SIGNIFICANCES, pBandSignificances);
}

bool Nitf::ImageSubheader::importGeoInformation_2_0(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor,
   DynamicObject *pDynObj, const string& appName,
   const string& ossimName)
{
   if (pPropertyInterface == NULL)
   {
      return false;
   }

   VERIFY(pDescriptor != NULL && pDynObj != NULL);
   RasterFileDescriptor *pFd =
      dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   VERIFY(pFd != NULL);
   if (Header::importMetadataValue<string>(pPropertyInterface, pDescriptor,
        pDynObj, ICORDS, ossimNitfImageHeaderV2_X::ICORDS_KW) == false)
   {
      return false;
   }

   if (Header::importMetadataValue<string>(pPropertyInterface, pDescriptor,
        pDynObj, IGEOLO, ossimNitfImageHeaderV2_X::IGEOLO_KW) == false)
   {
      return false;
   }

   const string& iCords = pDynObj->getAttribute(ICORDS).toXmlString();
   const string& iGeolo = pDynObj->getAttribute(IGEOLO).toXmlString();

   const LocationType pixelCorners[] = {
      LocationType(0, 0),
      LocationType(pFd->getColumnCount()-1, 0),
      LocationType(pFd->getColumnCount()-1, pFd->getRowCount()-1),
      LocationType(0, pFd->getRowCount()-1)
   };
   const unsigned int numPixelCorners = sizeof(pixelCorners) / sizeof(pixelCorners[0]);

   list<GcpPoint> gcps;
   if (iCords.empty() || iGeolo.empty() || iCords == ICORDS_NONE)
   {
      return false;
   }
   else if (iCords == ICORDS_GEOGRAPHIC)
   {
      if (getGCPsFromGeographic(iGeolo, pixelCorners, numPixelCorners, gcps) == false)
      {
         return false;
      }

      pFd->setGcps(gcps);
      return true;
   }
   else if (iCords == ICORDS_UTM_MGRS)
   {
      if (getGCPsFromUtmMgrs(iGeolo, pixelCorners, numPixelCorners, gcps) == false)
      {
         return false;
      }

      pFd->setGcps(gcps);
      return true;

   }
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : TODO: Add full NITF 02.00 compatibility for IGEOLO (GeoCentric). (dadkins)")

   return false;
}

bool Nitf::ImageSubheader::importGeoInformation(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor,
   DynamicObject *pDynObj, const string& appName,
   const string& ossimName)
{
   if (pPropertyInterface == NULL)
   {
      return false;
   }

   VERIFY(pDescriptor != NULL && pDynObj != NULL);
   RasterFileDescriptor *pFd =
      dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   VERIFY(pFd != NULL);
   if (Header::importMetadataValue<string>(pPropertyInterface, pDescriptor,
        pDynObj, ICORDS, ossimNitfImageHeaderV2_X::ICORDS_KW) == false)
   {
      return false;
   }

   if (Header::importMetadataValue<string>(pPropertyInterface, pDescriptor,
        pDynObj, IGEOLO, ossimNitfImageHeaderV2_X::IGEOLO_KW) == false)
   {
      return false;
   }

   const string& iCords = pDynObj->getAttribute(ICORDS).toXmlString();
   const string& iGeolo = pDynObj->getAttribute(IGEOLO).toXmlString();

   const LocationType pixelCorners[] = {
      LocationType(0, 0),
      LocationType(pFd->getColumnCount()-1, 0),
      LocationType(pFd->getColumnCount()-1, pFd->getRowCount()-1),
      LocationType(0, pFd->getRowCount()-1)
   };
   const unsigned int numPixelCorners = sizeof(pixelCorners) / sizeof(pixelCorners[0]);

   list<GcpPoint> gcps;
   if (iCords.empty() || iGeolo.empty())
   {
      return false;
   }
   else if (iCords == ICORDS_UTM_MGRS)
   {
      if (getGCPsFromUtmMgrs(iGeolo, pixelCorners, numPixelCorners, gcps) == false)
      {
         return false;
      }

      pFd->setGcps(gcps);
      return true;
   }
   else if (iCords == ICORDS_UTM_NORTH || iCords == ICORDS_UTM_SOUTH)
   {
      if (getGCPsFromUtm(iCords, iGeolo, pixelCorners, numPixelCorners, gcps) == false)
      {
         return false;
      }

      pFd->setGcps(gcps);
      return true;
   }
   else if (iCords == ICORDS_GEOGRAPHIC)
   {
      if (getGCPsFromGeographic(iGeolo, pixelCorners, numPixelCorners, gcps) == false)
      {
         return false;
      }

      pFd->setGcps(gcps);
      return true;
   }
   else if (iCords == ICORDS_DECIMAL_DEGREES)
   {
      if (getGCPsFromDecimalDegrees(iGeolo, pixelCorners, numPixelCorners, gcps) == false)
      {
         return false;
      }

      pFd->setGcps(gcps);
      return true;
   }

   return false;
}

bool Nitf::ImageSubheader::getGCPsFromUtmMgrs(const string& iGeolo, const LocationType gcpPixels[],
   const unsigned int& numGcpPixels, list<GcpPoint>& gcps)
{
   const unsigned int charsPerIGEOLO = 15;

   for (unsigned int i = 0; i < numGcpPixels; i++)
   {
      MgrsPoint mgrs(iGeolo.substr(i * charsPerIGEOLO, charsPerIGEOLO));
      LatLonPoint latLon = mgrs.getLatLonCoordinates();

      GcpPoint gcp;
      gcp.mCoordinate = latLon.getCoordinates();
      gcp.mPixel = gcpPixels[i];
      gcps.push_back(gcp);
   }

   return true;
}

bool Nitf::ImageSubheader::getGCPsFromUtm(const string& iCords, const string& iGeolo,
   const LocationType gcpPixels[], const unsigned int& numGcpPixels, list<GcpPoint>& gcps)
{
   if (iCords.empty() == true)
   {
      return false;
   }

   const char hemisphere = iCords[0];
   if (hemisphere != 'N' && hemisphere != 'S')
   {
      return false;
   }

   istringstream strm(iGeolo);
   const unsigned int charsPerZone = 2;
   const unsigned int charsPerEasting = 6;
   const unsigned int charsPerNorthing = 7;
   const unsigned int maxChars = charsPerNorthing;
   for (unsigned int i = 0; i < numGcpPixels; i++)
   {
      vector<char> buf(maxChars, 0);

      int zone;
      strm.read(&buf.front(), charsPerZone);
      stringstream strZone(&buf.front());
      strZone >> zone;

      double easting;
      strm.read(&buf.front(), charsPerEasting);
      stringstream strEasting(&buf.front());
      strEasting >> easting;

      double northing;
      strm.read(&buf.front(), charsPerNorthing);
      stringstream strNorthing(&buf.front());
      strNorthing >> northing;

      UtmPoint utmPoint(easting, northing, zone, hemisphere);
      LatLonPoint latLonPoint = utmPoint.getLatLonCoordinates();

      GcpPoint gcp;
      gcp.mCoordinate = latLonPoint.getCoordinates();
      gcp.mPixel = gcpPixels[i];

      gcps.push_back(gcp);
   }

   return true;
}

bool Nitf::ImageSubheader::getGCPsFromGeographic(const string& iGeolo,
   const LocationType gcpPixels[], const unsigned int& numGcpPixels, list<GcpPoint>& gcps)
{   
   istringstream strm(iGeolo);
   for (unsigned int ui = 0; ui < numGcpPixels; ++ui)
   {
      int n;
      vector<char> buf(3, 0);
      double latitude = 0, longitude = 0;
      char dir;
      for (int lat = 0; lat < 3; ++lat)
      {
         memset(&buf.front(), 0, buf.size());
         strm.read(&buf.front(), 2);
         stringstream numer(&buf.front());
         numer >> n;
         latitude += n * pow(60.0, -1.0*lat);
      }
      strm.read(&dir, 1);
      if (dir == 'S')
      {
         latitude *= -1;
      }

      strm.read(&buf.front(), 3);
      stringstream numer(&buf.front());
      numer >> longitude;
      for (int lon = 1; lon < 3; ++lon)
      {
         memset(&buf.front(), 0, buf.size());
         strm.read(&buf.front(), 2);
         stringstream numer(&buf.front());
         numer >> n;
         longitude += n * pow(60.0, -1.0*lon);
      }
      strm.read(&dir, 1);
      if (dir == 'W')
      {
         longitude *= -1;
      }

      GcpPoint gcp;
      gcp.mCoordinate.mX = latitude;
      gcp.mCoordinate.mY = longitude;
      gcp.mPixel = gcpPixels[ui];

      gcps.push_back(gcp);
   }

   return true;
}

bool Nitf::ImageSubheader::getGCPsFromDecimalDegrees(const string& iGeolo, const LocationType gcpPixels[],
   const unsigned int& numGcpPixels, list<GcpPoint>& gcps)
{
   istringstream strm(iGeolo);
   for (unsigned int i = 0; i < numGcpPixels; i++)
   {
      // Add this GCP to the list.
      GcpPoint gcp;
      strm >> gcp.mCoordinate.mX >> gcp.mCoordinate.mY;
      gcp.mPixel = gcpPixels[i];

      gcps.push_back(gcp);
   }

   return true;
}

bool Nitf::ImageSubheader::exportGeoInformation(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   VERIFY(pDescriptor != NULL);

   Service<ModelServices> pModel;
   RasterElement *pRaster = dynamic_cast<RasterElement*>(pModel->getElement(pDescriptor->getName(), 
      pDescriptor->getType(), pDescriptor->getParent()));
   VERIFY(pRaster != NULL);

   if (!pRaster->isGeoreferenced())
   {
      const DynamicObject* pMetadata = pDescriptor->getMetadata();
      if (pMetadata == NULL || pProperties == NULL)
      {
         return false;
      }

      // Attempt to export NITF 2.10 file geographic information even if the image was not georeferenced
      const string fileVersionPath[] = { Nitf::NITF_METADATA, Nitf::FILE_HEADER,
         Nitf::FileHeaderFieldNames::FILE_VERSION, END_METADATA_NAME };
      const DataVariant& fileVersion = pMetadata->getAttributeByPath(fileVersionPath);
      if (fileVersion.isValid() == false)
      {
         return false;
      }

      const DataVariant& iCords = pMetadata->getAttributeByPath(iCordsPath);
      const DataVariant& iGeolo = pMetadata->getAttributeByPath(iGeoloPath);
      if (iCords.isValid() == false || iGeolo.isValid() == false)
      {
         return false;
      }

      ossimRefPtr<ossimProperty> pOssimCords = pProperties->getProperty(ossimNitfImageHeaderV2_X::ICORDS_KW);
      ossimRefPtr<ossimProperty> pOssimGeolo = pProperties->getProperty(ossimNitfImageHeaderV2_X::IGEOLO_KW);
      if (pOssimCords.get() == NULL || pOssimGeolo.get() == NULL)
      {
         return false;
      }

      const string fileVersionStr = fileVersion.toXmlString();
      const string iCordsStr = iCords.toXmlString();
      const string iGeoloStr = iGeolo.toXmlString();
       if (fileVersionStr == Nitf::VERSION_02_10 || (fileVersionStr == Nitf::VERSION_02_00 &&
          (iCordsStr == ICORDS_UTM_MGRS || iCordsStr == ICORDS_GEOGRAPHIC)))
      {
         return pOssimCords->setValue(iCordsStr) && pOssimGeolo->setValue(iGeoloStr);
      }

       return false;
   }

   vector<LocationType> coords;
   coords.push_back(LocationType(0, 0));
   coords.push_back(LocationType(pDescriptor->getColumnCount(), 0));
   coords.push_back(LocationType(pDescriptor->getColumnCount(), 
      pDescriptor->getRowCount()));
   coords.push_back(LocationType(0, pDescriptor->getRowCount()));

   string igeolo;
   size_t index = 0;
   for (vector<LocationType>::iterator iter = coords.begin();
      iter != coords.end(); ++iter, index += 15)
   {
      LocationType geo = pRaster->convertPixelToGeocoord(*iter);
      double lat = abs(geo.mX);
      int degLat = lat;
      lat -= degLat;
      lat *= 60;
      int minLat = lat;
      lat -= minLat;
      lat *= 60;
      int secLat = lat+0.5;

      if (secLat >= 60)
      {
         minLat += (secLat / 60);
         secLat = (secLat % 60);
      }

      if (minLat >= 60)
      {
         degLat += (minLat / 60);
         minLat = (minLat % 60);
      }
      char dirLat = geo.mX < 0 ? 'S' : 'N';

      double lon = abs(geo.mY);
      int degLon = lon;
      lon -= degLon;
      lon *= 60;
      int minLon = lon;
      lon -= minLon;
      lon *= 60;
      int secLon = lon+0.5;

      if (secLon >= 60)
      {
         minLon += (secLon / 60);
         secLon = (secLon % 60);
      }

      if (minLon >= 60)
      {
         degLon += (minLon / 60);
         minLon = (minLon % 60);
      }
      char dirLon = geo.mY < 0 ? 'W' : 'E';

      QString geoStr;
      geoStr.sprintf("%02d%02d%02d%c%03d%02d%02d%c",
         degLat, minLat, secLat, dirLat, degLon, minLon, secLon, dirLon);

      VERIFY(geoStr.length() == 15);
      igeolo += geoStr.toStdString();
   }

   // Update IGEOLO
   VERIFY(exportMetadataValue<string>(pDescriptor, 
      igeolo, pProperties, IGEOLO, ossimNitfImageHeaderV2_X::IGEOLO_KW));

   // Update ICORDS
   string icords = "G";
   return exportMetadataValue<string>(pDescriptor, 
      icords, pProperties, ICORDS, ossimNitfImageHeaderV2_X::ICORDS_KW);
}

bool Nitf::ImageSubheader::importILOC(const ossimPropertyInterface *pPropertyInterface,
   RasterDataDescriptor *pDescriptor,
   DynamicObject *pDynObj, const string& appName,
   const string& ossimName)
{
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
      
   // ILOC is stored as RRRRRCCCCC where RRRRR and CCCCC represent row and columns offsets.
   // Valid values are [-9999, 99999].
   string rrrrrCCCCC = pProp->valueToString();
   LocationType iLoc;

   try
   {
      iLoc.mX = lexical_cast<double>(rrrrrCCCCC.substr(5, 5));
      iLoc.mY = lexical_cast<double>(rrrrrCCCCC.substr(0, 5));
   }
   catch (boost::bad_lexical_cast e)
   {
      return false;
   }

   return pDynObj->setAttribute(appName, iLoc);
}

bool Nitf::ImageSubheader::exportILOC(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   const LocationType* pLoc = prop.getPointerToValue<LocationType>();
   if (pLoc == NULL)
   {
      return false;
   }

   // ILOC is stored as RRRRRCCCCC where RRRRR and CCCCC represent row and columns offsets.
   // Valid values are [-9999, 99999].
   stringstream strm;
   strm << setw(5) << setfill('0') << pLoc->mY << setw(5) << setfill('0') << pLoc->mX;

   ossimRefPtr<ossimProperty> pProperty = pProperties->getProperty(ossimName);
   if (pProperty == NULL)
   {
      return false;
   }
   else
   {
      return pProperty->setValue(strm.str().c_str());
   }
}

bool Nitf::ImageSubheader::exportClassificationString(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   VERIFY(pDescriptor != NULL);
   const Classification *pClass = pDescriptor->getClassification();
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

bool Nitf::ImageSubheader::exportClassificationDate(const RasterDataDescriptor *pDescriptor, 
   const DataVariant &prop,
   ossimContainerProperty *pProperties, const string& appName,
   const string& ossimName)
{
   VERIFY(pDescriptor != NULL);
   const Classification *pClass = pDescriptor->getClassification();
   VERIFY(pClass != NULL);

   const DateTime *pValue = NULL;
   
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
      // All ImageSubheader classification dates are stored in CCYYMMDD format.
      return exportDateCCYYMMDD(pDescriptor, *pValue, pProperties, appName, ossimName);
   }

   return false;
}
