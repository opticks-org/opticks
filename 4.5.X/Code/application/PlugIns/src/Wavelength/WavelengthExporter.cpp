/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "Filename.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "Wavelengths.h"
#include "WavelengthExporter.h"

#include <sstream>
using namespace std;

WavelengthExporter::WavelengthExporter()
{
   setType(Wavelengths::WavelengthType());
   setAbortSupported(false);
   allowMultipleInstances(true);
}

WavelengthExporter::~WavelengthExporter()
{}

bool WavelengthExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pManager;
   pArgList = pManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL));
   VERIFY(pArgList->addArg<Wavelengths>(Wavelengths::WavelengthsArg()));
   VERIFY(pArgList->addArg<Filename>(Wavelengths::WavelengthFileArg()));

   return true;
}

bool WavelengthExporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool WavelengthExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }

   StepResource pStep(string("Execute ") + getName(), "app", "B6112093-AE6A-4319-85D8-B7A4C1E3DC5E");

   // Extract the input args
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());

   Wavelengths* pWavelengths = pInArgList->getPlugInArgValue<Wavelengths>(Wavelengths::WavelengthsArg());
   if (pWavelengths == NULL)
   {
      string message = "The " + Wavelengths::WavelengthsArg() + " input value is invalid.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   Filename* pFilename = pInArgList->getPlugInArgValue<Filename>(Wavelengths::WavelengthFileArg());
   if (pFilename == NULL)
   {
      string message = "The " + Wavelengths::WavelengthFileArg() + " input value is not present.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   mFilename = pFilename->getFullPathAndName();
   if (mFilename.empty() == true)
   {
      string message = "The " + Wavelengths::WavelengthFileArg() + " input value is invalid.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Save the wavelength values
   bool bSuccess = saveWavelengths(pWavelengths);
   if (bSuccess == false)
   {
      string message = "Could not save the wavelengths to the file.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Wavelength export completed successfully.", 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

const string& WavelengthExporter::getFilename() const
{
   return mFilename;
}
