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
#include <sstream>
#include <string>

#include "AppVerify.h"
#include "DataAccessorImpl.h"
#include "Hdf4ImporterShell.h"
#include "Hdf4Utilities.h"
#include "Hdf4Attribute.h"
#include "Hdf4Dataset.h"
#include "Hdf4File.h"
#include "Hdf4Group.h"
#include "Filename.h"
#include "ObjectResource.h"
#include "RasterElement.h"
#include "ProgressTracker.h"
#include "StringUtilities.h"

using namespace HdfUtilities;
using namespace std;

Hdf4ImporterShell::Hdf4ImporterShell()
{
   setExtensions("HDF Files (*.hdf)");
   addDependencyCopyright("Hdf4",
      "Copyright Notice and Statement for NCSA Hierarchical Data Format (HDF) "
      "Software Library and Utilities<br>"
      "<br>"
      "Copyright 1988-2005 The Board of Trustees of the University of Illinois<br>"
      "<br>"
      "All rights reserved.<br>"
      "<br>"
      "Contributors:   National Center for Supercomputing Applications "
      "(NCSA) at the University of Illinois, Fortner Software, Unidata "
      "Program Center (netCDF), The Independent JPEG Group (JPEG), "
      "Jean-loup Gailly and Mark Adler (gzip), and Digital Equipment "
      "Corporation (DEC).<br>"
      "<br>"
      "Redistribution and use in source and binary forms, with or without "
      "modification, are permitted for any purpose (including commercial "
      "purposes) provided that the following conditions are met:<br>"
      "<br>"
      "1. Redistributions of source code must retain the above copyright "
      "notice, this list of conditions, and the following disclaimer.<br>"
      "<br>"
      "2. Redistributions in binary form must reproduce the above copyright "
      "notice, this list of conditions, and the following disclaimer in the "
      "documentation and/or materials provided with the distribution.<br>"
      "<br>"
      "3. In addition, redistributions of modified forms of the source or "
      "binary code must carry prominent notices stating that the original "
      "code was changed and the date of the change.<br>"
      "<br>"
      "4. All publications or advertising materials mentioning features or use "
      "of this software are asked, but not required, to acknowledge that it was "
      "developed by the National Center for Supercomputing Applications at the "
      "University of Illinois at Urbana-Champaign and credit the contributors.<br>"
      "<br>"
      "5. Neither the name of the University nor the names of the Contributors "
      "may be used to endorse or promote products derived from this software "
      "without specific prior written permission from the University or the "
      "Contributors.<br>"
      "<br>"
      "DISCLAIMER<br>"
      "<br>"
      "THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND THE CONTRIBUTORS \"AS IS\" "
      "WITH NO WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED.  In no event "
      "shall the University or the Contributors be liable for any damages "
      "suffered by the users arising out of the use of this software, even if "
      "advised of the possibility of such damage.");
}

bool Hdf4ImporterShell::getFileData(Hdf4File& parsedFile) const
{
   const string& filename = parsedFile.getName();
   if (filename.empty() == true)
   {
      return false;
   }

   Hdf4Group* pRootGroup = parsedFile.getRootGroup();
   VERIFY(pRootGroup != NULL);

   int iValid = Hishdf(filename.c_str());
   if (iValid <= 0)
   {
      return false;
   }

   Hdf4FileResource pFile(filename.c_str());
   if (pFile.get() == NULL || *pFile == FAIL)
   {
      return false;
   }

   bool bSuccess = false;
   int32 lNumDatasets = 0;
   int32 lNumFileAttributes = 0;

   int iSuccess = SDfileinfo(*pFile, &lNumDatasets, &lNumFileAttributes);
   if (iSuccess == SUCCEED)
   {
      // File attributes
      int32 i = 0;
      for (; i < lNumFileAttributes; i++)
      {
         char name[MAX_NC_NAME];
         int32 type = 0;
         int32 count = 0;
         iSuccess = SDattrinfo(*pFile, i, name, &type, &count);
         if (iSuccess == SUCCEED)
         {
            DataVariant var;
            if (HdfUtilities::readHdf4Attribute(*pFile, i, var))
            {
               parsedFile.addAttribute(name, var);
            }
         }
      }

      // Data sets
      for (i = 0; i < lNumDatasets; i++)
      {
         Hdf4DatasetResource pDatasetId(*pFile, i);
         if (pDatasetId.get() != NULL && *pDatasetId != FAIL)
         {
            char datasetName[MAX_NC_NAME];
            int32 lNumDimensions = 0;
            int32 lDimensionSizes[MAX_VAR_DIMS];
            int32 lDataType = 0;
            int32 lNumAttributes = 0;

            iSuccess = SDgetinfo(*pDatasetId, datasetName, &lNumDimensions,
               lDimensionSizes, &lDataType, &lNumAttributes);
            if (iSuccess == SUCCEED)
            {
               int iCoord = 0;
               iCoord = SDiscoordvar(*pDatasetId);
               if (iCoord == 0)
               {
                  // Every Hdf4File has a valid root group so we don't need to check the pointer
                  Hdf4Dataset* pDataset = pRootGroup->addDataset(datasetName);

                  Hdf4Dataset::HdfType encoding = Hdf4Dataset::UNSUPPORTED;
                  int bpp = 0;
                  if (pDataset != NULL)
                  {
                     // Data type
                     switch (lDataType)
                     {
                     case DFNT_CHAR: // fall through
                     case DFNT_INT8:
                        encoding = Hdf4Dataset::INT1SBYTE;
                        bpp = 1;
                        break;

                     case DFNT_UCHAR:  // fall through
                     case DFNT_UINT8:
                        encoding = Hdf4Dataset::INT1UBYTE;
                        bpp = 1;
                        break;

                     case DFNT_INT16:
                        encoding = Hdf4Dataset::INT2SBYTES;
                        bpp = 2;
                        break;

                     case DFNT_UINT16:
                        encoding = Hdf4Dataset::INT2UBYTES;
                        bpp = 2;
                        break;

                     case DFNT_INT32:
                        bpp = 4;
                        encoding = Hdf4Dataset::INT2SBYTES;
                        break;

                     case DFNT_UINT32:
                        bpp = 4;
                        encoding = Hdf4Dataset::INT4UBYTES;
                        break;

                     case DFNT_FLOAT32:
                        bpp = 4;
                        encoding = Hdf4Dataset::FLT4BYTES;
                        break;

                     case DFNT_FLOAT64:
                        bpp = 8;
                        encoding = Hdf4Dataset::FLT8BYTES;
                        break;

                     default:
                        break;
                     }

                     pDataset->setDataEncoding(encoding);
                     pDataset->setBytesPerElement(bpp);
                     
                     // Attributes
                     int32 j = 0;
                     for (j = 0; j < lNumAttributes; j++)
                     {
                        char name[MAX_NC_NAME];
                        int32 type = 0;
                        int32 count = 0;
                        iSuccess = SDattrinfo(*pDatasetId, j, name, &type, &count);
                        if (iSuccess == SUCCEED)
                        {
                           DataVariant var;
                           if (HdfUtilities::readHdf4Attribute(*pDatasetId, j, var))
                           {
                              pDataset->addAttribute(name, var);
                           }
                        }
                     }
                  }
               }
            }
            bSuccess = true;
         }
      }
   }

   return bSuccess;
}

bool Hdf4ImporterShell::createRasterPager(RasterElement *pRaster) const
{
   return HdfImporterShell::createRasterPagerPlugIn("Hdf4Pager", *pRaster);
}

void* Hdf4ImporterShell::loadDatasetFromFile(const Hdf4File& parsedFile, const Hdf4Dataset& dataset) const
{
   int32 numDims = 0;
   int32 dataType = 0;
   int32 numAttr = 0;
   int32 dimSizes[MAX_VAR_DIMS] = {0};

   char* pDatasetName = const_cast<char*>(dataset.getName().c_str());

   const string& filename = parsedFile.getName();

   ArrayResource<char> pBlock(NULL);

   Hdf4FileResource pFileHandle(filename.c_str());
   if (pFileHandle.get() != NULL && *pFileHandle!= FAIL)
   {
      Hdf4DatasetResource pDataHandle(*pFileHandle, pDatasetName);
      if (pDataHandle.get() == NULL || *pDataHandle != FAIL)
      {
         int32 success = SDgetinfo(*pDataHandle, pDatasetName, &numDims, dimSizes, &dataType, &numAttr);

         VERIFYRV(success != FAIL, NULL);

         double numBytes = 0;
         try
         {
            EncodingType e = UNKNOWN;
            dataset.getDataEncoding(e);

            numBytes = HdfUtilities::getDataSize(e);
            for (int i = 0; i < numDims; ++i)
            {
               numBytes *= dimSizes[i];
            }

            pBlock = ArrayResource<char>(static_cast<size_t>(numBytes));
         }
         catch (const HdfUtilities::Exception& exc)
         {
            stringstream msg;
            msg << "Error " << getName() << " 019: " << exc.getText();
            mProgressTracker.report(msg.str(), 0, ERRORS, true);
            return NULL;
         }
         VERIFYRV(pBlock.get() != NULL, NULL);

         int32 startValue[3] = {0};

         int iSuccess = SDreaddata(*pDataHandle, startValue, NULL, dimSizes, pBlock.get());
         if (iSuccess == FAIL)
         {
            stringstream msg;
            msg << "Error " << getName() << " 018: Failed to read from HDF4 file!";
            mProgressTracker.report(msg.str(), 0, ERRORS, true);
            pBlock = ArrayResource<char>(NULL);
         }
      }
      else
      {
         return NULL;
      }
   }
   return pBlock.release();
}

#endif
