/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "GeoreferenceDescriptor.h"
#include "GeoreferenceShell.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"

GeoreferenceShell::GeoreferenceShell()
{
   setType(PlugInManagerServices::GeoreferenceType());
   allowMultipleInstances(true);
}

GeoreferenceShell::~GeoreferenceShell()
{}

bool GeoreferenceShell::setInteractive()
{
   ExecutableShell::setInteractive();
   return false;
}

bool GeoreferenceShell::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool GeoreferenceShell::getInputSpecification(PlugInArgList*& pArgList)
{
   bool success = true;
   if (isBatch())
   {
      pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
      VERIFY(pArgList != NULL);
      success = success && pArgList->addArg<RasterElement>(Executable::DataElementArg(), NULL,
         "Raster element on which georeferencing will be performed.");
      success = success && pArgList->addArg<Progress>(Executable::ProgressArg(), NULL,
         Executable::ProgressArgDescription());
   }
   else
   {
      pArgList = NULL;
      success = false;
   }
   return success;
}

LocationType GeoreferenceShell::pixelToGeoQuick(LocationType pixel, bool* pAccurate) const
{
   return pixelToGeo(pixel, pAccurate);
}

LocationType GeoreferenceShell::geoToPixelQuick(LocationType geo, bool* pAccurate) const
{
   return geoToPixel(geo, pAccurate);
}

QWidget* GeoreferenceShell::getWidget(RasterDataDescriptor* pDescriptor)
{
   return NULL;
}

bool GeoreferenceShell::validate(const RasterDataDescriptor* pDescriptor, std::string& errorMessage) const
{
   // Check for a valid data descriptor
   if (pDescriptor == NULL)
   {
      errorMessage = "The raster data set information is invalid.";
      return false;
   }

   // Check for a valid georeference descriptor
   const GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
   if (pGeorefDescriptor == NULL)
   {
      errorMessage = "The georeference algorithm information is invalid.";
      return false;
   }

   // If creating a results layer, check for an invalid layer name
   if (pGeorefDescriptor->getCreateLayer() == true)
   {
      const std::string& layerName = pGeorefDescriptor->getLayerName();
      if (layerName.empty() == true)
      {
         errorMessage = "The georeference results layer name is invalid.";
         return false;
      }
   }

   return true;
}
