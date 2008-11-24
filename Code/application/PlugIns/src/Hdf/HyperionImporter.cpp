/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#if defined(HDF4_SUPPORT)

#include "AppVersion.h"
#include "AppVerify.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "Endian.h"
#include "Filename.h"
#include "HyperionImporter.h"
#include "Hdf4Attribute.h"
#include "Hdf4Dataset.h"
#include "Hdf4File.h"
#include "Hdf4Group.h"
#include "Hdf4Utilities.h"
#include "ImportDescriptor.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "TestDataPath.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"
#include "switchOnEncoding.h"

#include <algorithm>
#include <vector>

using namespace std;

template <class T>
class ApplyScaleFactor
{
public:
   ApplyScaleFactor(const T& factor) : mFactor(factor)
   {
   }

   void operator()(T& element) const
   {
      element *= mFactor;
   }

private:
   T mFactor;
};

namespace
{
   const string NANO_MTR = "nanometers";

   template<typename T>
   void wavePushBack(T* pDummy, vector<double>& wavelengths, void* pData, size_t index)
   {
      wavelengths.push_back(reinterpret_cast<T*>(pData)[index]);
   }

   vector<double> createWavelengthVector(void* pData, size_t column, size_t colCount, size_t numElements,
                                         EncodingType type)
   {
      vector<double> wavelengths;
      bool bTruncated = false;
      for (size_t ui = 0; ui < numElements; ++ui)
      {
         size_t index = ui * colCount + column;
         if (type == UNKNOWN)
         {
            throw HdfUtilities::Exception("Invalid wavelength type!");
         }
         switchOnEncoding(type, wavePushBack, NULL, wavelengths, pData, index);
      }
      return wavelengths;
   }
}

HyperionImporter::HyperionImporter()
{
   setName("Hyperion Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{1C4BCE03-8DB4-4d1a-8E6D-0394304D7E8C}");
   setExtensions("Hyperion Files (*.L1*)");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

HyperionImporter::~HyperionImporter()
{
}

vector<ImportDescriptor*> HyperionImporter::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;

   Hdf4File parsedFile(filename);
   bool bSuccess = getFileData(parsedFile);
   if (bSuccess == true)
   {
      FactoryResource<Filename> pFn;
      VERIFYRV(pFn.get() != NULL, descriptors);

      pFn->setFullPathAndName(filename);
      const string& baseName = pFn->getFileName();

      const Hdf4Dataset* pPrimaryDataset =
         dynamic_cast<const Hdf4Dataset*>(parsedFile.getRootGroup()->getElement(baseName));
      if ((pPrimaryDataset != NULL) && (mpModel.get() != NULL))
      {
         ImportDescriptor* pImportDescriptor = mpModel->createImportDescriptor(filename, "RasterElement", NULL);
         if (pImportDescriptor != NULL)
         {
            RasterDataDescriptor* pDescriptor =
               dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
            if (pDescriptor != NULL)
            {
               FactoryResource<RasterFileDescriptor> pFileDescriptor;
               if (pFileDescriptor.get() != NULL)
               {
                  pFileDescriptor->setFilename(filename);
                  pFileDescriptor->setDatasetLocation(pPrimaryDataset->getName());

                  // File attributes
                  FactoryResource<DynamicObject> pMetadata;
                  Hdf4File::AttributeContainer attributes = parsedFile.getAttributes();

                  Hdf4File::AttributeContainer::const_iterator it;
                  for (it = attributes.begin(); it != attributes.end(); ++it)
                  {
                     Hdf4Attribute* pAttribute = it->second;
                     if (pAttribute != NULL)
                     {
                        const string& name = pAttribute->getName();
                        const DataVariant& var = pAttribute->getVariant();

                        if (var.isValid())
                        {
                           if ((name == "File Byte Order") && (var.getTypeName() == "string"))
                           {
                              /* Since the field for 'byte order' is a fixed length string, when the byte order string
                              is set to 'Big endian', it is actually 'Big endian\000'. The roundabout method below
                              is to eliminate the trailing NULL characters so the std::string == comparison passes.
                              */
                              string value;
                              var.getValue(value);
                              if (value == "Big endian")
                              {
                                 pFileDescriptor->setEndian(BIG_ENDIAN);
                              }
                              else if (value == "Little endian")
                              {
                                 pFileDescriptor->setEndian(LITTLE_ENDIAN);
                              }
                           }
                           else if (name == "Interleave Format")
                           {
                              string value;
                              var.getValue(value);
                              value = StringUtilities::stripWhitespace(value);
                              InterleaveFormatType interleave = BIL;
                              if (value == "BIP")
                              {
                                 interleave = BIP;
                              }
                              else if (value == "BSQ")
                              {
                                 interleave = BSQ;
                              }

                              // HS data is most often manipulated as BIP, so set it as such
                              pDescriptor->setInterleaveFormat(BIP);
                              pFileDescriptor->setInterleaveFormat(interleave);
                           }
                           else
                           {
                              pMetadata->setAttribute(name, var);
                           }
                        }
                     }
                  }

                  // Data set attributes
                  attributes = pPrimaryDataset->getAttributes();
                  for (it = attributes.begin(); it != attributes.end(); ++it)
                  {
                     Hdf4Attribute* pAttribute = it->second;
                     if (pAttribute != NULL)
                     {
                        const string& name = pAttribute->getName();
                        const DataVariant& var = pAttribute->getVariant();

                        if (var.isValid())
                        {
                           if ((name == "Number of Along Track Pixels") &&
                              (var.getTypeName() == TypeConverter::toString<int>()))
                           {
                              int numRows = 0;
                              var.getValue(numRows);

                              vector<DimensionDescriptor> rows =
                                 RasterUtilities::generateDimensionVector(numRows, true, false, true);
                              pDescriptor->setRows(rows);
                              pFileDescriptor->setRows(rows);
                           }
                           else if ((name == "Number of Cross Track Pixels") &&
                              (var.getTypeName() == TypeConverter::toString<int>()))
                           {
                              int numColumns = 0;
                              var.getValue(numColumns);

                              vector<DimensionDescriptor> columns =
                                 RasterUtilities::generateDimensionVector(numColumns, true, false, true);
                              pDescriptor->setColumns(columns);
                              pFileDescriptor->setColumns(columns);
                           }
                           else if ((name == "Number of Bands") &&
                              (var.getTypeName() == TypeConverter::toString<int>()))
                           {
                              int numBands = 0;
                              var.getValue(numBands);

                              vector<DimensionDescriptor> bands =
                                 RasterUtilities::generateDimensionVector(numBands, true, false, true);
                              pDescriptor->setBands(bands);
                              pFileDescriptor->setBands(bands);
                           }
                           else
                           {
                              pMetadata->setAttribute(name, var);
                           }
                        }
                     }
                  }

                  // Data type
                  EncodingType e = UNKNOWN;
                  pPrimaryDataset->getDataEncoding(e);
                  pDescriptor->setDataType(e);
                  pFileDescriptor->setBitsPerElement(pDescriptor->getBytesPerElement() * 8);

                  // Center wavelengths
                  vector<double> wavelengthCenters;

                  const Hdf4Dataset* pCenterDataset = NULL;

                  Hdf4Group* pRootGroup = parsedFile.getRootGroup();
                  if (pRootGroup != NULL)
                  {
                     pCenterDataset =
                        dynamic_cast<const Hdf4Dataset*>(pRootGroup->getElement("Spectral Center Wavelengths"));
                  }

                  if (pCenterDataset != NULL)
                  {
                     void* pData = loadDatasetFromFile(parsedFile, *pCenterDataset);
                     if (pData != NULL)
                     {
                        EncodingType dataType = UNKNOWN;
                        pCenterDataset->getDataEncoding(dataType);
                        unsigned int wavelengthSize = HdfUtilities::getDataSize(dataType);

                        // Get the wavelength units
                        bool inMicrons = true;

                        string units = "";
                        if (HdfUtilities::getAttribute<string>(*pCenterDataset, "Data Units", units) == true)
                        {
                           // call c_str() to remove trailing spaces in the string that fail this comparison
                           if (NANO_MTR == units.c_str())
                           {
                              inMicrons = false;
                           }
                        }

                        wavelengthCenters = createWavelengthVector(pData, 0, pDescriptor->getColumnCount(),
                           pDescriptor->getBandCount(), dataType);
                        if (inMicrons == false)
                        {
                           for_each(wavelengthCenters.begin(), wavelengthCenters.end(),
                              ApplyScaleFactor<double>(1e-3f));
                        }

                        string pCenterPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, 
                           CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
                        pMetadata->setAttributeByPath(pCenterPath, wavelengthCenters);
                     }
                  }

                  // Start and end wavelengths
                  if (wavelengthCenters.empty() == false)
                  {
                     const Hdf4Dataset* pWidthDataset = NULL;
                     if (pRootGroup != NULL)
                     {
                        pWidthDataset = dynamic_cast<const Hdf4Dataset*>(pRootGroup->getElement("Spectral Bandwidths"));
                     }

                     if (pWidthDataset != NULL)
                     {
                        void* pData = loadDatasetFromFile(parsedFile, *pWidthDataset);
                        if (pData != NULL)
                        {
                           EncodingType dataType = UNKNOWN;
                           pWidthDataset->getDataEncoding(dataType);
                           unsigned int wavelengthSize = HdfUtilities::getDataSize(dataType);

                           // Get the wavelength units
                           bool inMicrons = true;

                           string units = "";
                           if (HdfUtilities::getAttribute<string>(*pWidthDataset, "Data Units", units) == true)
                           {
                              // call c_str() to remove trailing spaces in the string that fail this comparison
                              if (NANO_MTR == units.c_str())
                              {
                                 inMicrons = false;
                              }
                           }

                           vector<double> waveWidths = createWavelengthVector(pData, 0, pDescriptor->getColumnCount(),
                              pDescriptor->getBandCount(), dataType);
                           if (inMicrons == false)
                           {
                              for_each(waveWidths.begin(), waveWidths.end(), ApplyScaleFactor<double>(1e-3f));
                           }


                           vector<double> startWavelengths;
                           vector<double> endWavelengths;
                           const vector<DimensionDescriptor>& bands = pFileDescriptor->getBands();
                           for (unsigned int i = 0; i < bands.size(); ++i)
                           {
                              if (i < wavelengthCenters.size())
                              {
                                 double center = wavelengthCenters[i];
                                 double fwhm = waveWidths[i];

                                 double dStartWavelength = center - fwhm / 2;
                                 double dEndWavelength = center + fwhm / 2;

                                 startWavelengths.push_back(dStartWavelength);
                                 endWavelengths.push_back(dEndWavelength);
                              }
                           }

                           if (!startWavelengths.empty() && !endWavelengths.empty())
                           {
                              string pStartPath[] = { SPECIAL_METADATA_NAME, 
                                 BAND_METADATA_NAME, START_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
                              pMetadata->setAttributeByPath(pStartPath, startWavelengths);
                              string pEndPath[] = { SPECIAL_METADATA_NAME, 
                                 BAND_METADATA_NAME, END_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
                              pMetadata->setAttributeByPath(pEndPath, endWavelengths);
                           }
                        }
                     }
                  }

                  // Metadata
                  pDescriptor->setMetadata(pMetadata.get());
                  pDescriptor->setFileDescriptor(pFileDescriptor.get());
               }
            }

            descriptors.push_back(pImportDescriptor);
         }
      }
   }

   return descriptors;
}

unsigned char HyperionImporter::getFileAffinity(const std::string& filename)
{
   if (getImportDescriptors(filename).empty())
   {
      return Importer::CAN_NOT_LOAD;
   }
   else
   {
      return Importer::CAN_LOAD;
   }
}

bool HyperionImporter::runOperationalTests(Progress* pProgress, ostream& failure)
{
   return false;
}

bool HyperionImporter::runAllTests(Progress* pProgress, ostream& failure)
{
   // test accessing an HDF4/HDF4-EOS file
   const string testFile = getTestDataPath() + "EO1H1690362003147110PM.L1R";

   Hdf4File file(testFile);
   VERIFY(getFileData(file));

   VERIFY(file.getAttributes().size() == 14);
   Hdf4Attribute* pAttribute = file.getAttribute("Interleave Format");
   VERIFY(pAttribute != NULL);
   VERIFY(pAttribute->getVariant().getPointerToValue<string>() != NULL);
   VERIFY(pAttribute->getVariant().getPointerToValue<string>()->c_str() == string("BIL"));

   Hdf4Group* pRoot = file.getRootGroup();
   VERIFY(pRoot != NULL);

   const vector<Hdf4Element*>& elements = pRoot->getElements();
   VERIFY(elements.size() == 5);

   const Hdf4Dataset* pCube = dynamic_cast<const Hdf4Dataset*>(pRoot->getElement("EO1H1690362003147110PM.L1R"));
   VERIFY(pCube != NULL);

   // columns
   pAttribute = pCube->getAttribute("Number of Cross Track Pixels");
   VERIFY(pAttribute != NULL);
   VERIFY(pAttribute->getVariant().getPointerToValue<int>() != NULL);
   VERIFY(*pAttribute->getVariant().getPointerToValue<int>() == 256);

   // rows
   pAttribute = pCube->getAttribute("Number of Along Track Pixels");
   VERIFY(pAttribute != NULL);
   VERIFY(pAttribute->getVariant().getPointerToValue<int>() != NULL);
   VERIFY(*pAttribute->getVariant().getPointerToValue<int>() == 2914);

   pAttribute = pCube->getAttribute("Number of Bands");
   VERIFY(pAttribute != NULL);
   VERIFY(pAttribute->getVariant().getPointerToValue<int>() != NULL);
   VERIFY(*pAttribute->getVariant().getPointerToValue<int>() == 242);

   pAttribute = pCube->getAttribute("Data units");
   VERIFY(pAttribute != NULL);
   VERIFY(pAttribute->getVariant().getPointerToValue<string>() != NULL);
   VERIFY(pAttribute->getVariant().getPointerToValue<string>()->c_str() == string("w/m^2/sr/micron"));

   const Hdf4Dataset* pDataset = dynamic_cast<const Hdf4Dataset*>(pRoot->getElement("Flag Mask"));
   VERIFY(pDataset != NULL);

   pDataset = dynamic_cast<const Hdf4Dataset*>(pRoot->getElement("Gain Coefficients"));
   VERIFY(pDataset != NULL);

   pDataset = dynamic_cast<const Hdf4Dataset*>(pRoot->getElement("Spectral Bandwidths"));
   VERIFY(pDataset != NULL);

   pDataset = dynamic_cast<const Hdf4Dataset*>(pRoot->getElement("Spectral Center Wavelengths"));
   VERIFY(pDataset != NULL);

   return true;
}

#endif
