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
#include "StringUtilities.h"
#include "WavelengthImporter.h"
#include "Wavelengths.h"

using namespace std;

WavelengthImporter::WavelengthImporter()
{
   setType(Wavelengths::WavelengthType());
   setAbortSupported(false);
   allowMultipleInstances(true);
}

WavelengthImporter::~WavelengthImporter()
{}

bool WavelengthImporter::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pManager;
   pArgList = pManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL));
   VERIFY(pArgList->addArg<Filename>(Wavelengths::WavelengthFileArg()));

   return true;
}

bool WavelengthImporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pManager;
   pArgList = pManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Wavelengths>(Wavelengths::WavelengthsArg()));
   return true;
}

bool WavelengthImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep(string("Execute ") + getName(), "app", "5675693B-AC25-4C98-9924-ED4E17DC1C30");
   if (pInArgList == NULL)
   {
      string message = "The input argument list is not present.";
      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Extract the input args
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());

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

   // Begin the import
   if (pProgress != NULL)
   {
      pProgress->updateProgress("Wavelength import started.", 0, NORMAL);
   }

   // Load the wavelength values
   FactoryResource<Wavelengths> pWavelengths;
   string errorMessage;

   bool bSuccess = loadWavelengths(pWavelengths.get(), errorMessage);
   if (bSuccess == false)
   {
      string message = "The wavelength values could not be read from the file.";
      if (errorMessage.empty() == false)
      {
         message = errorMessage;
      }

      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   if (pProgress != NULL)
   {
      if (errorMessage.empty() == false)
      {
         pProgress->updateProgress(errorMessage, 99, WARNING);
      }

      pProgress->updateProgress("Wavelength import completed successfully.", 100, NORMAL);
   }

   // Add the output values to the message log
   const vector<double>& startValues = pWavelengths->getStartValues();
   const vector<double>& centerValues = pWavelengths->getCenterValues();
   const vector<double>& endValues = pWavelengths->getEndValues();
   WavelengthUnitsType units = pWavelengths->getUnits();

   pStep->addProperty("Wavelength File", mFilename);
   pStep->addProperty("Wavelength Starts", startValues);
   pStep->addProperty("Wavelength Centers", centerValues);
   pStep->addProperty("Wavelength Ends", endValues);
   pStep->addProperty("Wavelength Units", StringUtilities::toDisplayString(units));

   // Populate the output arg list
   if (pOutArgList != NULL)
   {
      pOutArgList->setPlugInArgValue<Wavelengths>(Wavelengths::WavelengthsArg(), pWavelengths.release());
   }

   pStep->finalize(Message::Success);
   return true;
}

const string& WavelengthImporter::getFilename() const
{
   return mFilename;
}
