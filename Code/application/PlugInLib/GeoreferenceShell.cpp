/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "GeoreferenceShell.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"

using namespace std;

GeoreferenceShell::GeoreferenceShell()
{
   setType(PlugInManagerServices::GeoreferenceType());
   allowMultipleInstances(true);
}

GeoreferenceShell::~GeoreferenceShell()
{
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
      success = success && pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription());
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

QWidget* GeoreferenceShell::getGui(RasterElement* pRaster)
{
   return NULL;
}

bool GeoreferenceShell::validateGuiInput() const
{
   return true;
}
