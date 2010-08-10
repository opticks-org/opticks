/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QInputDialog>

#include "AppVersion.h"
#include "DesktopServices.h"
#include "PlugInRegistration.h"
#include "StringUtilities.h"
#include "WavelengthMetadataExporter.h"
#include "WavelengthMetadataImporter.h"
#include "Wavelengths.h"
#include "xmlreader.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

REGISTER_PLUGIN_BASIC(OpticksWavelength, WavelengthMetadataImporter);

WavelengthMetadataImporter::WavelengthMetadataImporter()
{
   setName("Wavelength Metadata Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Loads wavelength values in metadata format from a file");
   setDescriptorId("{F7683C81-97B4-4C91-B6F4-9C617254D534}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

WavelengthMetadataImporter::~WavelengthMetadataImporter()
{}

bool WavelengthMetadataImporter::loadWavelengths(Wavelengths* pWavelengths, string& errorMessage) const
{
   if (pWavelengths == NULL)
   {
      return false;
   }

   string filename = getFilename();
   if (filename.empty() == true)
   {
      errorMessage = "The wavelength filename is empty.";
      return false;
   }

   XmlReader xml(NULL, false);

   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDocument = xml.parse(filename);
   if (pDocument != NULL)
   {
      DOMElement* pRootElement = pDocument->getDocumentElement();
      if (pRootElement != NULL)
      {
         // Check for a wavelengths file
         if (XMLString::equals(pRootElement->getNodeName(), X("Wavelengths")) == false)
         {
            errorMessage = "The file is not a valid wavelength file.";
            return false;
         }

         // Version
         unsigned int version =
            StringUtilities::fromXmlString<unsigned int>(A(pRootElement->getAttribute(X("version"))));
         if (version != WavelengthMetadataExporter::WavelengthFileVersion())
         {
            errorMessage = "The wavelength file version is not supported.";
            return false;
         }

         // Units
         bool bError = false;

         WavelengthUnitsType units = StringUtilities::fromXmlString<WavelengthUnitsType>(
            string(A(pRootElement->getAttribute(X("units")))), &bError);
         if (bError == true)
         {
            if (isBatch() == false)
            {
               QStringList unitList;
               unitList.append(QString::fromStdString(StringUtilities::toDisplayString(MICRONS)));
               unitList.append(QString::fromStdString(StringUtilities::toDisplayString(NANOMETERS)));
               unitList.append(QString::fromStdString(StringUtilities::toDisplayString(INVERSE_CENTIMETERS)));

               Service<DesktopServices> pDesktop;

               QString strUnits = QInputDialog::getItem(pDesktop->getMainWidget(), "Select Wavelength Units",
                  "Units:", unitList, 0, false);
               if (strUnits.isEmpty() == false)
               {
                  units = StringUtilities::fromDisplayString<WavelengthUnitsType>(strUnits.toStdString());
               }
               else
               {
                  errorMessage = "Could not read the wavelength units in the file.";
                  return false;
               }
            }
            else
            {
               errorMessage = "The wavelength values are assumed to be in microns.";
               units = MICRONS;
            }
         }

         // Wavelengths
         vector<double> startWavelengths;
         vector<double> centerWavelengths;
         vector<double> endWavelengths;

         for (DOMNode* pChild = pRootElement->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
         {
            if (XMLString::equals(pChild->getNodeName(), X("value")))
            {
               DOMElement* pElement = static_cast<DOMElement*>(pChild);

               string start = A(pElement->getAttribute(X("start")));
               if (start.empty() == false)
               {
                  double wavelength = QString::fromStdString(start).toDouble();
                  startWavelengths.push_back(wavelength);
               }

               string center = A(pElement->getAttribute(X("center")));
               if (center.empty() == false)
               {
                  double wavelength = QString::fromStdString(center).toDouble();
                  centerWavelengths.push_back(wavelength);
               }

               string end = A(pElement->getAttribute(X("end")));
               if (end.empty() == false)
               {
                  double wavelength = QString::fromStdString(end).toDouble();
                  endWavelengths.push_back(wavelength);
               }
            }
         }

         // Set the values in the Wavelengths object
         pWavelengths->setUnits(units);
         pWavelengths->setStartValues(startWavelengths, units);
         pWavelengths->setCenterValues(centerWavelengths, units);
         pWavelengths->setEndValues(endWavelengths, units);

         return !(pWavelengths->isEmpty());
      }
   }

   errorMessage = "The wavelength file is not in the appropriate format.";
   return false;
}
