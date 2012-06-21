/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DesktopServices.h"
#include "GeoMosaicDlg.h"
#include "GeoMosaic.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"

REGISTER_PLUGIN_BASIC(OpticksGeoMosaic, GeoMosaic);

GeoMosaic::GeoMosaic()
{
   setName("GeoMosaic");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("GeoMosaic");
   setDescription("GeoMosaic.");
   setMenuLocation("[Geo]\\Mosaic");
   setDescriptorId("{2E3A7F84-D5CA-47D5-AB70-0DF9804AF0C7}");
   allowMultipleInstances(true);
   destroyAfterExecute(true);
   setAbortSupported(true);
   setWizardSupported(false);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GeoMosaic::~GeoMosaic()
{}

bool GeoMosaic::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   return true;
}

bool GeoMosaic::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool GeoMosaic::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());

   Service<DesktopServices> pDesktop;
   GeoMosaicDlg dlg(pProgress, pDesktop->getMainWidget());

   return dlg.exec();
}