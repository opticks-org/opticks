/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "FileResource.h"
#include "PlugInRegistration.h"
#include "StringUtilities.h"
#include "WavelengthMetadataExporter.h"
#include "Wavelengths.h"
#include "xmlwriter.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWavelength, WavelengthMetadataExporter);

WavelengthMetadataExporter::WavelengthMetadataExporter()
{
   setName("Wavelength Metadata Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Saves wavelength values as stored in metadata to a file");
   setDescriptorId("{C11A6DDB-E9BF-4133-BBA4-B215F24A8097}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

WavelengthMetadataExporter::~WavelengthMetadataExporter()
{}

bool WavelengthMetadataExporter::saveWavelengths(const Wavelengths* pWavelengths) const
{
   if (pWavelengths == NULL)
   {
      return false;
   }

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

   // Version
   XMLWriter xml("Wavelengths");
   xml.addAttr("version", WavelengthMetadataExporter::WavelengthFileVersion());

   // Units
   xml.addAttr("units", StringUtilities::toXmlString(pWavelengths->getUnits()));

   // Wavelengths
   const vector<double>& startWavelengths = pWavelengths->getStartValues();
   const vector<double>& centerWavelengths = pWavelengths->getCenterValues();
   const vector<double>& endWavelengths = pWavelengths->getEndValues();

   unsigned int numWavelengths = pWavelengths->getNumWavelengths();
   for (unsigned int i = 0; i < numWavelengths; ++i)
   {
      xml.pushAddPoint(xml.addElement("value"));

      if (startWavelengths.empty() == false)
      {
         xml.addAttr("start", startWavelengths[i]);
      }

      if (centerWavelengths.empty() == false)
      {
         xml.addAttr("center", centerWavelengths[i]);
      }

      if (endWavelengths.empty() == false)
      {
         xml.addAttr("end", endWavelengths[i]);
      }

      xml.popAddPoint();
   }

   // Write the data to the file
   xml.writeToFile(pFile.get());

   return true;
}
