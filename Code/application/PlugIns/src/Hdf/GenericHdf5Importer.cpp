/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <hdf5.h>

#include "ApplicationServices.h"
#include "AppVerify.h"
#include "AppConfig.h"
#include "AppVersion.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "GenericHdf5Importer.h"
#include "Hdf5Dataset.h"
#include "Hdf5Group.h"
#include "Hdf5File.h"
#include "Hdf5Resource.h"
#include "Hdf5Utilities.h"
#include "ImportDescriptor.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "TestDataPath.h"
#include "TypesFile.h"
#include "UInt64.h"

#include <deque>
#include <sstream>
#include <string>

using namespace HdfUtilities;
using namespace std;

namespace
{
   herr_t populateMetadata(hid_t loc_id, const char *name, void* pParameter)
   {
      DynamicObject* pMetadata = static_cast<DynamicObject*>(pParameter);
      VERIFYRV(pMetadata != NULL, 1);

      Hdf5AttributeResource attrId(H5Aopen_name(loc_id, name));

      DataVariant var;
      if (HdfUtilities::readHdf5Attribute(*attrId, var) && var.isValid())
      {
         pMetadata->setAttribute(name, var);
      }

      return 0;
   }

   FactoryResource<DynamicObject> getDimensionData(hid_t dataDescriptor)
   {
      FactoryResource<DynamicObject> pDimensions;
      if (pDimensions.get() != NULL)
      {
         hid_t dataSpaceId = H5Dget_space(dataDescriptor);
         hid_t dataTypeId = H5Dget_type(dataDescriptor);

         int dimSize = H5Sget_simple_extent_ndims(dataSpaceId);
         if (dimSize > 0)
         {
            vector<hsize_t> dims(dimSize);
            vector<hsize_t> maxdims(dimSize);

            hsize_t numDims = H5Sget_simple_extent_dims(dataSpaceId, &dims.front(), &maxdims.front());
            for (hsize_t i = 0; i < numDims; ++i)
            {
               stringstream strm;
               strm << i;

               pDimensions->setAttribute(strm.str(), UInt64(static_cast<uint64_t>(dims[static_cast<size_t>(i)])));
            }
         }
      }

      return pDimensions;
   }
}

GenericHdf5Importer::GenericHdf5Importer()
{
   Hdf5ImporterShell::setName("Generic HDF5 Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setExtensions("HDF5 Files (*.h5)");
   setDescriptorId("{34B85DC0-7A0B-44f2-945C-7B06749F57F0}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GenericHdf5Importer::~GenericHdf5Importer() 
{
}

unsigned char GenericHdf5Importer::getFileAffinity(const string& filename)
{
   herr_t status = H5Fis_hdf5(filename.c_str());
   if (status > 0)
   {
      return Importer::CAN_LOAD_FILE_TYPE;
   }
   return Importer::CAN_NOT_LOAD;
}

vector<ImportDescriptor*> GenericHdf5Importer::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;

   Hdf5FileResource fileId(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
   VERIFYRV(fileId.get() != NULL, descriptors);

   Hdf5File parsedFile(filename);
   bool bSuccess = parsedFile.readFileData();
   if (bSuccess == true)
   {
      FactoryResource<RasterFileDescriptor> pFileDescriptor;
      VERIFYRV(pFileDescriptor.get() != NULL, descriptors);

      pFileDescriptor->setFilename(filename);

      const Hdf5Group* pRoot = parsedFile.getRootGroup();
      if (pRoot != NULL)
      {
         const vector<Hdf5Element*>& elements = pRoot->getElements();
         deque<Hdf5Element*> elementsToProcess;
         std::copy(elements.begin(), elements.end(), std::back_inserter(elementsToProcess));

         while (elementsToProcess.empty() == false)
         {
            Hdf5Element* pFirst = elementsToProcess.front();
            elementsToProcess.pop_front();
            // the element extracted is a group, add its elements to the end
            if (dynamic_cast<Hdf5Group*>(pFirst) != NULL)
            {
               const vector<Hdf5Element*>& subElements = static_cast<Hdf5Group*>(pFirst)->getElements();
               std::copy(subElements.begin(), subElements.end(), std::back_inserter(elementsToProcess));
            }
            else
            {
               Hdf5Dataset* pDataset = dynamic_cast<Hdf5Dataset*>(pFirst);
               if (pDataset != NULL)
               {
                  const string fpan = pDataset->getFullPathAndName();
                  pFileDescriptor->setDatasetLocation(fpan);

                  FactoryResource<DynamicObject> pMetadata;
                  VERIFYRV(pMetadata.get() != NULL, descriptors);

                  FactoryResource<DynamicObject> pAttributes;
                  VERIFYRV(pAttributes.get() != NULL, descriptors);

                  Hdf5DataSetResource dataId(*fileId, fpan);
                  VERIFYRV(dataId.get() != NULL, descriptors);

                  Service<ModelServices> pModel;

                  ImportDescriptor* pImportDescriptor =
                     pModel->createImportDescriptor(fpan, "RasterElement", NULL, false);
                  if (pImportDescriptor != NULL)
                  {
                     RasterDataDescriptor* pDescriptor =
                        dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
                     if (pDescriptor != NULL)
                     {
                        EncodingType encoding;
                        pDataset->getDataEncoding(encoding);
                        pDescriptor->setDataType(encoding);
                        pFileDescriptor->setBitsPerElement(RasterUtilities::bytesInEncoding(encoding) * 8);

                        FactoryResource<DynamicObject> pDimensions = getDimensionData(*dataId);
                        herr_t status = H5Aiterate(*dataId, NULL, populateMetadata, pAttributes.get());
                        if (status == 0)
                        {
                           pMetadata->setAttribute("Attributes", *pAttributes.get());
                        }
                        pMetadata->setAttribute("Dimensions", *pDimensions.get());
                        pDescriptor->setMetadata(pMetadata.get());

                        pDescriptor->setFileDescriptor(pFileDescriptor.get());
                     }

                     descriptors.push_back(pImportDescriptor);
                  }
               }
            }
         }
      }
   }

   return descriptors;
}

bool GenericHdf5Importer::runOperationalTests(Progress* pProgress, ostream& failure)
{
   return runAllTests(pProgress, failure);
}

bool GenericHdf5Importer::runAllTests(Progress* pProgress, std::ostream& failure)
{
   Service<ModelServices> pModel;
   VERIFY(pModel.get() != NULL);

   // test accessing an HDF4/HDF4-EOS file
   string testFile = getTestDataPath() + "EO1H1690362003147110PM.L1R";

   auto_ptr<Hdf5File> pFile(new Hdf5File(testFile));
   VERIFY(pFile.get() != NULL);
   VERIFY(pFile->readFileData() == false); // not HDF5!

   Hdf5FileResource h5f(testFile);
   VERIFY(h5f.get() != NULL && *h5f == -1); // not HDF5!

   testFile = getTestDataPath() + "Hdf/small.h5";

   Hdf5File* pHdfFile = new Hdf5File(testFile);
   pFile = auto_ptr<Hdf5File>(pHdfFile);
   VERIFY(pFile.get() != NULL);
   
   h5f = Hdf5FileResource(testFile);
   VERIFY(h5f.get() != NULL && *h5f != -1); // is HDF5

   VERIFY(pFile->readFileData()); // is HDF5

   const Hdf5Group* pRoot = pFile->getRootGroup();
   VERIFY(pRoot != NULL);

   string datasetLocation = "/Data/Frame/FPA1/0000000001/Frame Data";
   const Hdf5Dataset* pDatasetUsingFullPath =
      dynamic_cast<const Hdf5Dataset*>(pRoot->getElementByPath(datasetLocation));
   // now try it piecemeal to see if opening Data --> Frame --> FPA1 --> XXXX --> 'Frame Data' works
   const Hdf5Group* pGroup = dynamic_cast<const Hdf5Group*>(pRoot->getElement("Data"));
   VERIFY(pGroup != NULL);

   pGroup = dynamic_cast<const Hdf5Group*>(pGroup->getElement("Frame"));
   VERIFY(pGroup != NULL);

   pGroup = dynamic_cast<const Hdf5Group*>(pGroup->getElement("FPA1"));
   VERIFY(pGroup != NULL);

   VERIFY(pRoot->getElement("103513") == NULL); // no such element in file

   pGroup = dynamic_cast<const Hdf5Group*>(pGroup->getElement("0000000001"));
   VERIFY(pGroup != NULL);

   const Hdf5Dataset* pPiecemeal = dynamic_cast<const Hdf5Dataset*>(pGroup->getElement("Frame Data"));
   VERIFY(pPiecemeal != NULL);

   VERIFY(pPiecemeal == pDatasetUsingFullPath);

   VERIFY(pDatasetUsingFullPath != NULL);
   EncodingType type;
   pDatasetUsingFullPath->getDataEncoding(type);
   VERIFY(type == INT2UBYTES);
   VERIFY(RasterUtilities::bytesInEncoding(type) == 2);

   // now try loading datasets into memory and make sure to blast them when done
   ImporterResource hdfImporter("Generic HDF5 Importer", testFile);
   vector<ImportDescriptor*> descriptorsToImport;

   vector<ImportDescriptor*> descriptors = hdfImporter->getImportDescriptors();
   for (vector<ImportDescriptor*>::iterator iter = descriptors.begin(); iter != descriptors.end(); ++iter)
   {
      ImportDescriptor* pImportDescriptor = *iter;
      if (pImportDescriptor != NULL)
      {
         RasterDataDescriptor* pDescriptor =
            dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            RasterFileDescriptor* pFileDescriptor =
               dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
            VERIFY(pFileDescriptor != NULL);

            if (pFileDescriptor->getDatasetLocation() == datasetLocation)
            {
               vector<DimensionDescriptor> rows = RasterUtilities::generateDimensionVector(256, true, false, true);
               vector<DimensionDescriptor> cols = RasterUtilities::generateDimensionVector(256, true, false, true);
               vector<DimensionDescriptor> bands = RasterUtilities::generateDimensionVector(4, true, false, true);

               pDescriptor->setRows(rows);
               pFileDescriptor->setRows(rows);
               pDescriptor->setColumns(cols);
               pFileDescriptor->setColumns(cols);
               pDescriptor->setBands(bands);
               pFileDescriptor->setBands(bands);
               pDescriptor->setInterleaveFormat(BSQ);
               pFileDescriptor->setInterleaveFormat(BSQ);

               pImportDescriptor->setImported(true);
               descriptorsToImport.push_back(pImportDescriptor);
            }
         }
      }
   }

   hdfImporter->setImportDescriptors(descriptorsToImport);
   hdfImporter->execute();

   vector<DataElement*> elements = hdfImporter->getImportedElements();
   VERIFY(elements.size() == 1);

   ModelResource<RasterElement> pRaster(dynamic_cast<RasterElement*>(elements.front()));
   VERIFY(pRaster.get() != NULL);

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   const vector<DimensionDescriptor>& importedRows = pDescriptor->getRows();
   const vector<DimensionDescriptor>& importedCols = pDescriptor->getColumns();
   const vector<DimensionDescriptor>& importedBands = pDescriptor->getBands();

   VERIFY(pRaster->getPixelValue(importedCols[2], importedRows[0], importedBands[0]) == 31);
   VERIFY(pRaster->getPixelValue(importedCols[15], importedRows[8], importedBands[0]) == 71);
   VERIFY(pRaster->getPixelValue(importedCols[145], importedRows[88], importedBands[3]) == 56);

   return true;
}
