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

WavelengthExporter::WavelengthExporter() :
   mpStep(NULL),
   mpProgress(NULL),
   mpWavelengths(NULL)
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

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<Wavelengths>(Wavelengths::WavelengthsArg(), "Wavelengths to be exported."));
   VERIFY(pArgList->addArg<Filename>(Wavelengths::WavelengthFileArg(), "Location to export the wavelength data to."));

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
   mpStep = pStep.get();

   // Extract the input args
   if (extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   // Save the wavelength values
   bool bSuccess = saveWavelengths(mpWavelengths);
   if (bSuccess == false)
   {
      string message = "Could not save the wavelengths to the file.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Wavelength export completed successfully.", 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

bool WavelengthExporter::extractInputArgs(PlugInArgList* pArgList)
{
   VERIFY(pArgList != NULL);

   mpProgress = pArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());

   mpWavelengths = pArgList->getPlugInArgValue<Wavelengths>(Wavelengths::WavelengthsArg());
   if (mpWavelengths == NULL)
   {
      string message = "The " + Wavelengths::WavelengthsArg() + " input value is invalid.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }

   Filename* pFilename = pArgList->getPlugInArgValue<Filename>(Wavelengths::WavelengthFileArg());
   if (pFilename == NULL)
   {
      string message = "The " + Wavelengths::WavelengthFileArg() + " input value is not present.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }

   mFilename = pFilename->getFullPathAndName();
   if (mFilename.empty() == true)
   {
      string message = "The " + Wavelengths::WavelengthFileArg() + " input value is invalid.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }

   return true;
}

Progress* WavelengthExporter::getProgress() const
{
   return mpProgress;
}

const string& WavelengthExporter::getFilename() const
{
   return mFilename;
}
