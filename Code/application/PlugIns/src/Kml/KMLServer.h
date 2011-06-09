/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef KMLSERVER_H
#define KMLSERVER_H

#include "AlgorithmShell.h"
#include "ConfigurationSettings.h"
#include "MuHttpServer.h"

class Layer;
class QString;
class QStringList;
class RasterElement;
class XMLWriter;

class KMLServer : public MuHttpServer, public AlgorithmShell
{
   Q_OBJECT

public:
   SETTING(KmlServerPort, Kml, int, 0);

   KMLServer();
   ~KMLServer();

   bool setBatch()
   {
      AlgorithmShell::setBatch();
      return false;
   }
   bool getInputSpecification(PlugInArgList *&pInArgList);
   bool getOutputSpecification(PlugInArgList *&pOutArgList);
   bool execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList);

protected:
   MuHttpServer::Response getRequest(const QString &uri, const QString &contentType, const QString &body,
      const FormValueMap &form);
};

#endif
