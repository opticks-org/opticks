/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>

#include "AppVersion.h"
#include "AppConfig.h"
#include "GenericImporter.h"
#include "FileResource.h"
#include "ImportDescriptor.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"

#include <sstream>
#include <string>
using namespace std;

GenericImporter::GenericImporter()
{
   setName("Generic Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("");
   setDescriptorId("{FFEBC264-3278-4fac-A39A-7D61280DE47B}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GenericImporter::~GenericImporter()
{
}

unsigned char GenericImporter::getFileAffinity(const string& filename)
{
   // If the file exists, it can be loaded (with user input).
   QString fileName = QString::fromStdString(filename);
   QFileInfo fileInfo(fileName);
   if ((fileInfo.exists() == true) && (fileInfo.isFile() == true))
   {
      return Importer::CAN_LOAD_WITH_USER_INPUT;
   }
   else
   {
      return Importer::CAN_NOT_LOAD;
   }
}

vector<ImportDescriptor*> GenericImporter::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;

   // Create a single empty import descriptor for the user to specify the values
   ImportDescriptor* pImportDescriptor = mpModel->createImportDescriptor(filename, "RasterElement", NULL);
   if (pImportDescriptor != NULL)
   {
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         FactoryResource<RasterFileDescriptor> pFileDescriptor;
         if (pFileDescriptor.get() != NULL)
         {
            // Set the filename
            pFileDescriptor->setFilename(filename);

            // Set the file descriptor in the data descriptor
            pDescriptor->setFileDescriptor(pFileDescriptor.get());
         }
      }

      descriptors.push_back(pImportDescriptor);
   }

   return descriptors;
}

bool GenericImporter::validate(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   if (pDescriptor == NULL)
   {
      errorMessage = "The data descriptor is invalid!";
      return false;
   }
   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      errorMessage = "The file descriptor is invalid!";
      return false;
   }
   const Filename& filename = pFileDescriptor->getFilename();
   if (filename.getFullPathAndName().empty() == true)
   {
      errorMessage = "The filename is invalid!";
      return false;
   }

   unsigned int numRows = pFileDescriptor->getRowCount();
   unsigned int numColumns = pFileDescriptor->getColumnCount();
   unsigned int numBands = pFileDescriptor->getBandCount();
   unsigned int bitsPerElement = pFileDescriptor->getBitsPerElement();

   if (numRows == 0 && numColumns == 0 && numBands == 0 && bitsPerElement == 0)
   {
      errorMessage = "This file was not recognized by an importer.\n"
         "Please enter the requested information to load this file using the Generic Importer.";
      return false;
   }

   if (numRows == 0)
   {
      errorMessage = "The number of rows is invalid!";
      return false;
   }
   if (numColumns == 0)
   {
      errorMessage = "The number of columns is invalid!";
      return false;
   }
   if (numBands == 0)
   {
      errorMessage = "The number of bands is invalid!";
      return false;
   }
   if (bitsPerElement == 0)
   {
      errorMessage = "The number of bits per element is invalid!";
      return false;
   }

   // check required size against file size/s
   int64_t requiredSize = RasterUtilities::calculateFileSize(pFileDescriptor);
   if (requiredSize < 0)
   {
      errorMessage = "Unable to determine data file size due to problem in RasterFileDescriptor.";
      return false;
   }

   const vector<const Filename*>& bandFiles = pFileDescriptor->getBandFiles();
   if (bandFiles.empty() == true)
   {
      LargeFileResource file;
      if (file.open(filename.getFullPathAndName(), O_RDONLY | O_BINARY, S_IREAD) == false)
      {
         errorMessage = "The file: " + string(filename) + " does not exist!";
         return false;
      }
      if (file.fileLength() < requiredSize)
      {
         errorMessage = "The size of the file does not match the current parameters!";
         return false;
      }
   }
   else
   {
      vector<const Filename*>::const_iterator bandIt;
      int band(1);
      for (bandIt = bandFiles.begin(); bandIt!=bandFiles.end(); ++bandIt)
      {
         const Filename* pFilename = *bandIt;
         VERIFYRV(pFilename != NULL, false);
         LargeFileResource bandFile;
         stringstream msg;
         if (bandFile.open(pFilename->getFullPathAndName(), O_RDONLY | O_BINARY, S_IREAD) == false)
         {
            msg << "Band filename " << band << " is invalid!";
            errorMessage = msg.str();
            return false;
         }
         if (bandFile.fileLength() < requiredSize)
         {
            msg << "Band filename " << band; 
            msg << " does not match required size for the current parameters!";
            errorMessage = msg.str();
            return false;
         }
         ++band;
      }
   }

   return RasterElementImporterShell::validate(pDescriptor, errorMessage);
}
