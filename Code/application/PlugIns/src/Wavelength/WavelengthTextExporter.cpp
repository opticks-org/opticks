/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "FileResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "StringUtilities.h"
#include "Wavelengths.h"
#include "WavelengthTextExporter.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWavelength, WavelengthTextExporter);

WavelengthTextExporter::WavelengthTextExporter() :
   mBandNumbers(false)
{
   setName("Wavelength Text Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Saves wavelength values to a file");
   setDescriptorId("{F2B48F8D-C78F-4FB0-9C21-52A2CB635BB3}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

WavelengthTextExporter::~WavelengthTextExporter()
{}

bool WavelengthTextExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   if (WavelengthExporter::getInputSpecification(pArgList) == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<bool>("Export Band Numbers", mBandNumbers, "Flag that specifies whether band numbers "
      "should be exported.  If set to true, a one-based band number will be saved in the first column for each "
      "wavelength value.  The default value is false."));

   return true;
}

bool WavelengthTextExporter::extractInputArgs(PlugInArgList* pArgList)
{
   if (WavelengthExporter::extractInputArgs(pArgList) == false)
   {
      return false;
   }

   VERIFY(pArgList->getPlugInArgValue<bool>("Export Band Numbers", mBandNumbers));
   return true;
}

bool WavelengthTextExporter::saveWavelengths(const Wavelengths* pWavelengths) const
{
   if (pWavelengths == NULL)
   {
      return false;
   }

   if (pWavelengths->hasCenterValues() == false)
   {
      return false;
   }

   // Get the filename
   string filename = getFilename();
   if (filename.empty() == true)
   {
      return false;
   }

   // Open the file for writing
   FileResource pFile(filename.c_str(), "wt");
   if (pFile.get() == NULL)
   {
      return false;
   }

   // Save the wavelengths
   vector<double> startValues = pWavelengths->getStartValues();
   vector<double> centerValues = pWavelengths->getCenterValues();
   vector<double> endValues = pWavelengths->getEndValues();

   if (pWavelengths->getUnits() != MICRONS)
   {
      // Convert the wavelength values to microns since units are not saved in the file
      FactoryResource<Wavelengths> pSaveWavelengths;
      pSaveWavelengths->initializeFromWavelengths(pWavelengths);
      pSaveWavelengths->setUnits(MICRONS);

      startValues = pSaveWavelengths->getStartValues();
      centerValues = pSaveWavelengths->getCenterValues();
      endValues = pSaveWavelengths->getEndValues();
   }

   bool centerOnly = true;
   if ((startValues.empty() == false) && (endValues.empty() == false))
   {
      centerOnly = false;
   }

   for (vector<double>::size_type i = 0; i < centerValues.size(); ++i)
   {
      if (centerOnly == false)
      {
         string startValue = StringUtilities::toXmlString(startValues[i]);
         string centerValue = StringUtilities::toXmlString(centerValues[i]);
         string endValue = StringUtilities::toXmlString(endValues[i]);
         if (mBandNumbers == true)
         {
            fprintf(pFile.get(), "%u\t%s\t%s\t%s\n", i + 1, centerValue.c_str(), startValue.c_str(), endValue.c_str());
         }
         else
         {
            fprintf(pFile.get(), "%s\t%s\t%s\n", centerValue.c_str(), startValue.c_str(), endValue.c_str());
         }
      }
      else
      {
         string centerValue = StringUtilities::toXmlString(centerValues[i]);
         if (mBandNumbers == true)
         {
            fprintf(pFile.get(), "%u\t%s\n", i + 1, centerValue.c_str());
         }
         else
         {
            fprintf(pFile.get(), "%s\n", centerValue.c_str());
         }
      }
   }

   return true;
}
