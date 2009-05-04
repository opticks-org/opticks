/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "OpticksMethods.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "ImageHandler.h"
#include "IntrospectionMethods.h"
#include "MessageLogResource.h"
#include "PlugInRegistration.h"
#include "UtilityServices.h"
#include "XmlRpcServer.h"
#include "xmlreader.h"
#include <QtCore/QtDebug>

XERCES_CPP_NAMESPACE_USE

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Remove qDebug() calls " \
   "after debugging is complete (tclarke)")

REGISTER_PLUGIN_BASIC(OpticksXmlRpc, XmlRpcServer);

XmlRpcServer::XmlRpcServer() : MuHttpServer(getSettingXmlRpcServerPort(), NULL)
{
   PlugInShell::setName("XML-RPC Server");
   setDescriptorId("{AAE7071F-4861-4926-858E-A499C010F0AE}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setDescription("This is an XML-RPC server which provides a subset of the plug-in API.");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT_MSG);
   destroyAfterExecute(false);
   executeOnStartup(true);
   setWizardSupported(false);
   registerPath("images", new ImageHandler(this));
}

XmlRpcServer::~XmlRpcServer()
{
   for (QMap<QString, XmlRpcMethodCallImp*>::Iterator mit = mMethodCalls.begin(); mit != mMethodCalls.end(); ++mit)
   {
      delete mit.value();
   }
}

bool XmlRpcServer::getInputSpecification(PlugInArgList *&pInArgList)
{
   pInArgList = NULL;
   return true;
}

bool XmlRpcServer::getOutputSpecification(PlugInArgList *&pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool XmlRpcServer::execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList)
{
   registerMethodCall("opticks.annotation.create", new OpticksXmlRpcMethods::Annotation::Create);
   registerMethodCall("opticks.annotation.delete", new OpticksXmlRpcMethods::Annotation::Delete);
   registerMethodCall("opticks.aoi.create", new OpticksXmlRpcMethods::Aoi::Create);
   registerMethodCall("opticks.aoi.delete", new OpticksXmlRpcMethods::Aoi::Delete);
   registerMethodCall("opticks.aoi.getBoundingBox", new OpticksXmlRpcMethods::Aoi::GetBoundingBox);
   registerMethodCall("opticks.aoi.setMode", new OpticksXmlRpcMethods::Aoi::SetMode);
   registerMethodCall("opticks.close", new OpticksXmlRpcMethods::Close);
   registerMethodCall("opticks.createView", new OpticksXmlRpcMethods::CreateView);
   registerMethodCall("opticks.getViewInfo", new OpticksXmlRpcMethods::GetViewInfo);
   registerMethodCall("opticks.getViews", new OpticksXmlRpcMethods::GetViews);
   registerMethodCall("opticks.linkViews", new OpticksXmlRpcMethods::LinkViews);
   registerMethodCall("opticks.open", new OpticksXmlRpcMethods::Open);
   registerMethodCall("opticks.panBy", new OpticksXmlRpcMethods::PanBy);
   registerMethodCall("opticks.panTo", new OpticksXmlRpcMethods::PanTo);
   registerMethodCall("opticks.registerCallback", new OpticksXmlRpcMethods::RegisterCallback(*this));
   registerMethodCall("opticks.rotateBy", new OpticksXmlRpcMethods::RotateBy);
   registerMethodCall("opticks.rotateTo", new OpticksXmlRpcMethods::RotateTo);
   registerMethodCall("opticks.unlinkViews", new OpticksXmlRpcMethods::UnlinkViews);
   registerMethodCall("opticks.version", new OpticksXmlRpcMethods::Version);
   registerMethodCall("opticks.zoom", new OpticksXmlRpcMethods::Zoom);
   registerMethodCall("system.listMethods", new SystemListMethodsCallImp(mMethodCalls));
   registerMethodCall("system.methodHelp", new SystemMethodHelpCallImp(mMethodCalls));
   registerMethodCall("system.methodSignature", new SystemMethodSignatureCallImp(mMethodCalls));
   return start();
}

void XmlRpcServer::registerMethodCall(const QString &name, XmlRpcMethodCallImp *pMethodCall)
{
   if (pMethodCall == NULL)
   {
      mMethodCalls.remove(name);
   }
   else
   {
      mMethodCalls[name] = pMethodCall;
   }
}

MuHttpServer::Response XmlRpcServer::getRequest(const QString &uri, const QString &contentType,
                                                const QString &body, const FormValueMap &form)
{
   MuHttpServer::Response rsp;
   // Display help
   rsp.mBody = "<html><head><title>" APP_NAME" XML-RPC Interface</title></head><body>\n"
               "<h1>" APP_NAME " XML-RPC Interface</h1>\n"
               "<p>You have reached the XML-RPC Interface for " APP_NAME ". This is designed to complement "
               "the existing C++ (doxygen) documentation and explains the relationship between the C++ methods "
               "and the XML-RPC methods. Please see the C++ API documentation for more information.<br>"
               "The available methods are:</p>\n"
               "<table width=\"75%\" cellpadding=\"4\" border=\"1\" frame=\"border\" rules=\"all\">\n"
               "<tr><th>Method Signature</th><th>Method Help</th></tr>\n";
   for (QMap<QString, XmlRpcMethodCallImp*>::Iterator mit = mMethodCalls.begin(); mit != mMethodCalls.end(); ++mit)
   {
      XmlRpcMethodCallImp* pCall = mit.value();
      if (pCall == NULL)
      {
         continue;
      }
      rsp.mBody += "<tr><td>";
      XmlRpcArrayParam* pSigArray = pCall->getSignature();
      if (pSigArray != NULL)
      {
         bool firstSig = true;
         QString sig = mit.key() + "(";
         for (QVector<const XmlRpcParam*>::size_type i = 0; i < pSigArray->size(); i++)
         {
            const XmlRpcArrayParam* pSig = dynamic_cast<const XmlRpcArrayParam*>((*pSigArray)[i]);
            if (pSig == NULL)
            {
               continue;
            }
            for (QVector<const XmlRpcParam*>::size_type j = 0; j < pSig->size(); j++)
            {
               const XmlRpcParam* pParam = (*pSig)[j];
               QString param = "unknown";
               if (pParam != NULL)
               {
                  param = pParam->value().toString();
               }
               if (j == 0)
               {
                  sig = "<em>" + param + "</em>&nbsp;" + sig;
               }
               else if (j == 1)
               {
                  sig += "<em>" + param + "</em>";
               }
               else
               {
                  sig += ",<em>" + param + "</em>";
               }
            }
            if (firstSig)
            {
               firstSig = false;
            }
            else
            {
               rsp.mBody += "<br/>";
            }
            rsp.mBody += sig + ")";
            sig = mit.key() + "(";
         }
      }
      QString help = pCall->getHelp();
      rsp.mBody += "</td><td>" + help + "</td></tr>\n";
   }
   rsp.mBody += "</table><h1>Error Codes</h1>"
      "<table width=\"75%\" cellpadding=\"4\" border=\"1\" frame=\"border\" rules=\"all\">\n"
      "<tr><th>Error Code</th><th>Error Message</th></tr>\n";
   XmlRpcMethodFault::populateFaults();
   QMapIterator<unsigned int, QString> faultIt(XmlRpcMethodFault::sFaults);
   while (faultIt.hasNext())
   {
      faultIt.next();
      rsp.mBody += QString("<tr><td>%1</td><td>%2</td></tr>\n").arg(faultIt.key()).arg(faultIt.value());
   }
   rsp.mBody += "</table></body></html>";
   rsp.mHeaders["content-type"] = "text/html";
   rsp.mCode = HTTPRESPONSECODE_200_OK;
   return rsp;
}

MuHttpServer::Response XmlRpcServer::postRequest(const QString &uri, const QString &contentType,
                                                 const QString &body, const FormValueMap &form)
{
   MuHttpServer::Response rsp;
   if (contentType != "text/xml")
   {
      rsp.mCode = HTTPRESPONSECODE_500_INTERNALSERVERERROR;
      rsp.mHeaders["content-type"] = "text/html";
      rsp.mBody = "<html><body><h1>Invalid request</h1>The XML-RPC request is invalid.<br>"
         "Only content-type: text/xml is allowed.</body></html>";
      return rsp;
   }
   XmlReader xml(Service<UtilityServices>()->getMessageLog()->getLog(), false);
   try
   {
      string str = body.toStdString();
      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *pDoc = xml.parseString(str);
      DOMElement* pRoot = NULL;
      if (pDoc != NULL)
      {
         pRoot = pDoc->getDocumentElement();
      }
      if (pRoot == NULL)
      {
         throw "";
      }
      XmlRpcMethodCall rpcMsg(pRoot);
      processMethodCall(rpcMsg, rsp);
   }
   catch (...)
   {
      rsp.mHeaders["content-type"] = "text/html";
      rsp.mBody = "<html><body><h1>Internal server error</h1>"
                  "An exception was thrown while parsing the message</body></html>";
      rsp.mCode = HTTPRESPONSECODE_500_INTERNALSERVERERROR;
   }
   return rsp;
}

void XmlRpcServer::processMethodCall(const XmlRpcMethodCall &call, MuHttpServer::Response &rsp)
{
   rsp.mHeaders["content-type"] = "text/xml";
   rsp.mCode = HTTPRESPONSECODE_200_OK;
   QMap<QString, XmlRpcMethodCallImp*>::iterator mcit = mMethodCalls.find(call.getMethodName());
   if (mcit != mMethodCalls.end())
   {
      XmlRpcMethodCallImp* pCall = mcit.value();
      if (pCall != NULL)
      {
         try
         {
            XmlRpcParam* pRval = (*pCall)(call.getParams());
            rsp.mBody = XmlRpcMethodResponse(pRval).toXml();
         }
         catch (const XmlRpcMethodFault &fault)
         {
            rsp.mBody = fault.toXml();
         }
      }
   }
   else
   {
      XmlRpcMethodFault fault(100, QString("Unknown method: " + call.getMethodName()));
      rsp.mBody = fault.toXml();
   }
}

void XmlRpcServer::debug(HttpRequest *pHttpRequest)
{
#if defined(DEBUG)
   if (pHttpRequest != NULL)
   {
      qDebug() << QString::fromStdString(pHttpRequest->sBody);
   }
#endif
}

void XmlRpcServer::warning(const QString &msg)
{
   MessageResource m("XML-RPC Warning", "app", "62513161-989C-4F1B-B07F-683EF6E1B6A3");
   m->addProperty("message", msg.toStdString());
}
