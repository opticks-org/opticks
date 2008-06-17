/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DataDescriptor.h"
#include "IceImporterShell.h"
#include "Hdf5File.h"
#include "Hdf5Pager.h"
#include "Hdf5Resource.h"
#include "Hdf5Utilities.h"
#include "IceReader.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SessionManager.h"
using namespace std;

IceImporterShell::IceImporterShell(IceUtilities::FileType fileType) :
   mFileType(fileType)
{
}

IceImporterShell::~IceImporterShell()
{
}

bool IceImporterShell::validate(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   errorMessage = "";
   if (!mErrors.empty())
   {
      for (vector<string>::const_iterator iter = mErrors.begin(); iter != mErrors.end(); ++iter)
      {
         errorMessage += *iter + "\n";
      }
      return false;
   }
   string baseErrorMessage;
   bool bValidate = Hdf5ImporterShell::validate(pDescriptor, baseErrorMessage);
   if (!mWarnings.empty())
   {
      if (!baseErrorMessage.empty())
      {
         errorMessage += baseErrorMessage + "\n";
      }
      for (vector<string>::const_iterator iter = mWarnings.begin(); iter != mWarnings.end(); ++iter)
      {
         errorMessage += *iter + "\n";
      }
   }
   else
   {
      errorMessage = baseErrorMessage;
   }
   return bValidate;
}

unsigned char IceImporterShell::getFileAffinity(const string& filename)
{
   Hdf5File file(filename);
   if (file.readFileData("/IceFormatDescriptor") == true)
   {
      IceReader reader(file);
      if (reader.isValidIceFile() != IceReader::NOT_ICE && reader.getFileType() == mFileType)
      {
         return Importer::CAN_LOAD;
      }
   }
   return Importer::CAN_NOT_LOAD;
}

vector<ImportDescriptor*> IceImporterShell::getImportDescriptors(const string& filename)
{
   StepResource dbgStep("getDataDescriptors", "app", "31F1D43F-10D4-46CF-82CF-833B5D43F6B7", "", "Ice Importer");
   vector<ImportDescriptor*> descriptors;

   mErrors.clear();
   mWarnings.clear();

   // search the root group of the Hdf5File for applicable groups that correspond to data cubes
   Hdf5File file(filename);
   if (file.readFileData() == true)
   {
      IceReader reader(file);
      IceReader::ValidityType iceValid = reader.isValidIceFile();
      IceUtilities::FileType fileType = reader.getFileType();
      if (iceValid == IceReader::NOT_SUPPORTED || iceValid == IceReader::NOT_ICE || fileType != mFileType)
      {
         //we reported CAN_LOAD from getFileAffinity, create an empty data descriptor
         //so that no other importers will be invoked and so that validate() can return
         //false and provide an error message stating
         //the given version of the ice file is not supported
         ImportDescriptorResource pImportDescriptor(filename, "RasterElement");
         descriptors.push_back(pImportDescriptor.release());
      }
      else if (iceValid == IceReader::FULLY_SUPPORTED && fileType == mFileType)
      {
         descriptors = reader.getImportDescriptors();
      }
      mErrors = reader.getErrors();
      mWarnings = reader.getWarnings();
   }
   if (!descriptors.empty())
   {
      dbgStep->finalize(Message::Success);
   }
   return descriptors;
}

bool IceImporterShell::performImport() const
{
   Progress* pProgress = getProgress();
   if (Hdf5ImporterShell::performImport() == false)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to perform import.", 100, ERRORS);
      }

      return false;
   }

   hid_t fileHandle = -1;
   Hdf5FileResource fileResource;
   if (getFileHandle(fileHandle, fileResource) == false)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to get the file handle.", 100, ERRORS);
      }

      return NULL;
   }

   RasterElement* pElement = getRasterElement();
   if (pElement == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to get the RasterElement.", 100, ERRORS);
      }

      return false;
   }

   Hdf5File file(pElement->getFilename(), fileHandle);
   if (file.readFileData() == false)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to read statistics from file.", 100, WARNING);
      }
   }
   else
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
      IceReader reader(file);
      if (pDescriptor != NULL && 
         RasterUtilities::isSubcube(pDescriptor, false) == false &&
         Service<SessionManager>()->isSessionLoading() == false)
      {
         if (reader.loadCubeStatistics(pElement) == false)
         {
            if (pProgress != NULL)
            {
               pProgress->updateProgress("Unable to load statistics.", 100, WARNING);
            }
         }
      }
   }

   return true;
}
