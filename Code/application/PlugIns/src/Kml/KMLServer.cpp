/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DesktopServices.h"
#include "GcpList.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "ImageHandler.h"
#include "Kml.h"
#include "KMLServer.h"
#include "Layer.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "MultipointObject.h"
#include "PlugInRegistration.h"
#include "PolygonObject.h"
#include "PolylineObject.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SpatialDataWindow.h"
#include "SpatialDataView.h"
#include "Window.h"
#include "xmlwriter.h"

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtGui/QMatrix>
#include <QtGui/QWidget>

namespace
{
   string sKmlNamespace = "http://earth.google.com/kml/2.1";
};

REGISTER_PLUGIN_BASIC(OpticksKml, KMLServer);

// If there's no server port setting (i.e. the kml .cfg file is missing) don't verify, just don't start the server
KMLServer::KMLServer() : MuHttpServer(hasSettingKmlServerPort() ? getSettingKmlServerPort() : 0, NULL)
{
   AlgorithmShell::setName("KML HTTP Server");
   setShortDescription("Serve KML files.");
   setDescription("This plug-in servers KML content over an embedded web server.");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT_MSG);
   setDescriptorId("{4A33CE88-6F7C-4652-9EFA-73B4BA0BDDEC}");
   executeOnStartup(true);
   destroyAfterExecute(false);
   setWizardSupported(false);

   ImageHandler* pImageHandler = new ImageHandler(0, this);
   registerPath("images", pImageHandler);
}

KMLServer::~KMLServer()
{
}

bool KMLServer::getInputSpecification(PlugInArgList *&pInArgList)
{
   pInArgList = NULL;
   return true;
}

bool KMLServer::getOutputSpecification(PlugInArgList *&pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool KMLServer::execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList)
{
   return start();
}

MuHttpServer::Response KMLServer::getRequest(const QString &uri, const QString &contentType, const QString &body,
                                             const FormValueMap &form)
{
   Response r;
   QStringList path = uri.split("/", QString::SkipEmptyParts);
   Kml k;
   if (path.size() == 1 && path[0] == "index.kml" && k.addSession())
   {
      r.mCode = HTTPRESPONSECODE_200_OK;
      r.mHeaders["content-type"] = "application/vnd.google-earth.kml+xml";
      r.mBody = k.toString();
   }
   else
   {
      r.mCode = HTTPRESPONSECODE_404_NOTFOUND;
      r.mHeaders["content-type"] = "text/html";
      r.mBody = "<html><body><h1>Not found</h1>The requested document can not be located.</body></html>";
   }
   return r;
}
