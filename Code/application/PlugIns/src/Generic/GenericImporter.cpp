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
#include "GenericImporter.h"
#include "ImportDescriptor.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"

#include <string>
#include <vector>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksGeneric, GenericImporter);

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
{}

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

int GenericImporter::getValidationTest(const DataDescriptor* pDescriptor) const
{
   int validationTest = RasterElementImporterShell::getValidationTest(pDescriptor);
   if (pDescriptor != NULL)
   {
      const RasterFileDescriptor* pFileDescriptor = dynamic_cast<const RasterFileDescriptor*>(
         pDescriptor->getFileDescriptor());
      VERIFYRV(pFileDescriptor != NULL, validationTest);

      if (pFileDescriptor->getBandFiles().empty() == true)
      {
         validationTest |= FILE_SIZE;
      }
      else
      {
         validationTest |= BAND_FILE_SIZES;
      }
   }

   return validationTest;
}
