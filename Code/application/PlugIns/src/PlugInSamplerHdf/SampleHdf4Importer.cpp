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

#include <hdf.h>
#include <mfhdf.h>

#include "Classification.h"
#include "AppVerify.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "Hdf4Utilities.h"
#include "Hdf4Attribute.h"
#include "Hdf4Dataset.h"
#include "Hdf4File.h"
#include "Hdf4Group.h"
#include "ImportDescriptor.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "SampleHdf4Importer.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"

using namespace HdfUtilities;
using namespace std;

SampleHdf4Importer::SampleHdf4Importer()
{
   setName("Sample MODIS HDF4-EOS Importer");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setExtensions("MODIS Files (*.hdf *.HDF)");
   setDescriptorId("{1B1B1D52-B1DA-4247-92DF-324FE7EF8722}");
   setProductionStatus(false);
}

vector<ImportDescriptor*> SampleHdf4Importer::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;

   Hdf4File parsedFile(filename);
   bool bSuccess = getFileData(parsedFile);
   if (bSuccess == true)
   {
      const Hdf4Dataset* pDataset =
         dynamic_cast<const Hdf4Dataset*>(parsedFile.getRootGroup()->getElement("EV_500_RefSB"));
      if ((pDataset != NULL) && (mpModel.get() != NULL))
      {
         Hdf4FileResource pFile(filename.c_str());
         if (pFile.get() != NULL)
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
                     int32 numDims = 0;
                     int32 dataType = 0;
                     int32 numAttr = 0;

                     pFileDescriptor->setFilename(filename);

                     Hdf4DatasetResource pDataHandle(*pFile, pDataset->getName().c_str());
                     int32 dimSizes[MAX_VAR_DIMS] = {0};
                     if (pDataHandle != NULL && *pDataHandle != FAIL)
                     {
                        pFileDescriptor->setDatasetLocation(pDataset->getName());

                        int32 success = SDgetinfo(*pDataHandle, const_cast<char*>(pDataset->getName().c_str()),
                                                  &numDims, dimSizes, &dataType, &numAttr);
                        // find out what type this Dataset is
                        string strDataType = hdf4TypeToString(dataType, 1);
                        if (success == SUCCEED && numDims == 3 && strDataType == "unsigned short")
                        {
                           // Bands
                           vector<DimensionDescriptor> bands =
                              RasterUtilities::generateDimensionVector(dimSizes[0], true, false, true);

                           pDescriptor->setBands(bands);
                           pFileDescriptor->setBands(bands);

                           // Rows
                           vector<DimensionDescriptor> rows =
                              RasterUtilities::generateDimensionVector(dimSizes[1], true, false, true);

                           pDescriptor->setRows(rows);
                           pFileDescriptor->setRows(rows);

                           // Columns
                           vector<DimensionDescriptor> columns =
                              RasterUtilities::generateDimensionVector(dimSizes[2], true, false, true);

                           pDescriptor->setColumns(columns);
                           pFileDescriptor->setColumns(columns);
                        }
                     }

                     // Data type
                     EncodingType e = UNKNOWN;
                     pDataset->getDataEncoding(e);
                     pDescriptor->setDataType(e);
                     pFileDescriptor->setBitsPerElement(pDescriptor->getBytesPerElement() * 8);

                     // Interleave format
                     pDescriptor->setInterleaveFormat(BSQ);
                     pFileDescriptor->setInterleaveFormat(BSQ);

                     // Metadata
                     FactoryResource<DynamicObject> pMetadata;
                     if (pMetadata.get() != NULL)
                     {
                        const Hdf4Dataset::AttributeContainer& attributes = pDataset->getAttributes();
                        for (Hdf4Dataset::AttributeContainer::const_iterator it = attributes.begin();
                           it != attributes.end(); ++it)
                        {
                           Hdf4Attribute* pAttribute = it->second;
                           if (pAttribute != NULL)
                           {
                              const string& name = pAttribute->getName();
                              const DataVariant& var = pAttribute->getVariant();
                              const unsigned short* pValue = var.getPointerToValue<unsigned short>();
                              if (name == "_FillValue" && pValue != NULL)
                              {
                                 // Bad values
                                 vector<int> badValues;
                                 badValues.push_back(*pValue);

                                 pDescriptor->setBadValues(badValues);
                              }
                              else
                              {
                                 pMetadata->setAttribute(name, var);
                              }
                           }
                        }

                        pDescriptor->setMetadata(pMetadata.get());
                     }
                     pDescriptor->setFileDescriptor(pFileDescriptor.get());
                  }
               }

               descriptors.push_back(pImportDescriptor);
            }
         }
      }
   }

   return descriptors;
}

unsigned char SampleHdf4Importer::getFileAffinity(const std::string& filename)
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

#endif
