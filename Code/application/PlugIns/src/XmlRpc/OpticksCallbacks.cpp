/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AnnotationLayer.h"
#include "DesktopServices.h"
#include "GraphicObject.h"
#include "Layer.h"
#include "LayerList.h"
#include "OpticksCallbacks.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "UtilityServices.h"
#include "WorkspaceWindow.h"
#include "xmlreader.h"
#include "XmlRpc.h"
#include "XmlRpcArrayParam.h"
#include "XmlRpcStructParam.h"

#include <list>
#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <string>

using namespace std;
XERCES_CPP_NAMESPACE_USE

XmlRpcAnnotationCallback::XmlRpcAnnotationCallback(LayerType layerType,
                                                   const QString &url,
                                                   const QString &method,
                                                   const XmlRpcStructParam &params,
                                                   OpticksXmlRpcMethods::RegisterCallback &callbackRegistrar,
                                                   QObject *pParent) :
      XmlRpcCallback(url, method, params, pParent), mpLayer(NULL), mpObject(NULL),
      mRegistrar(callbackRegistrar)
{
   const XmlRpcParam *pNameParam = params["name"];
   if(pNameParam != NULL)
   {
      mObjectName = pNameParam->value().toString();
   }
   getGraphicLayerAndObject(layerType, mObjectName, mpLayer, mpObject, NULL);
   if((mpLayer == NULL) || (mpObject == NULL))
   {
      throw XmlRpcMethodFault(303);
   }
   mpLayer->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &XmlRpcAnnotationCallback::processModified));
}

XmlRpcAnnotationCallback::~XmlRpcAnnotationCallback()
{
}

void XmlRpcAnnotationCallback::processModified(Subject &subject, const string &signal, const boost::any &val)
{
   GraphicLayer *pLayer = dynamic_cast<GraphicLayer*>(&subject);
   if(pLayer == NULL)
   {
      return;
   }
   std::list<GraphicObject*> objects;
   pLayer->getObjects(objects);
   bool found = false;
   for(std::list<GraphicObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
   {
      if(*it == mpObject)
      {
         found = true;
         break;
      }
   }
   if(!found)
   {
      XmlRpcParams params;
      params.push_back(new XmlRpcParam("string", QString("deleted")));
      params.push_back(new XmlRpcParam("string", mObjectName));
      call(params);
      mpLayer->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &XmlRpcAnnotationCallback::processModified));
   }
}

bool XmlRpcAnnotationCallback::getGraphicLayerAndObject(LayerType layerType, const QString &name, GraphicLayer *&pLayer,
                                                        GraphicObject *&pObject, SpatialDataView *pView,
                                                        bool createNewObject, GraphicObjectType newObjectType)
{
   // LayerName[ObjectType:ObjectIndex]
   // or LayerName[ObjectName]
   // LayerName: space separated words
   // ObjectType: per StringUtilities XML role
   // ObjectIndex: zero indexed
   // ObjectName: space separated words
   // important match groups in this regexp
   // 1 - the layer name
   // 6 - the (optional) object type
   // 7 - the (optional) object index
   // 8 - the (optional) object name
   QRegExp regexp("(\\w+(\\s+\\w*)?)(\\[((((\\w+):)?(\\d+))|(\\w+(\\s+\\w*)?))\\])?");
   if(!regexp.exactMatch(name))
   {
      return false;
   }
   string layerName = regexp.cap(1).toStdString();
   string objType = regexp.cap(6).toStdString();
   unsigned int objIdx = regexp.cap(7).toUInt();
   string objName = regexp.cap(8).toStdString();

   // get the layer
   if(pView == NULL)
   {
      WorkspaceWindow *pWindow = Service<DesktopServices>()->getCurrentWorkspaceWindow();
      if(pWindow != NULL)
      {
         pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      }
   }
   if(pView == NULL)
   {
      return false;
   }
   LayerList *pLayerList = pView->getLayerList();
   if(pLayerList == NULL)
   {
      return false;
   }
   vector<Layer*> graphicLayers;
   pLayerList->getLayers(layerType, graphicLayers);
   for(vector<Layer*>::iterator lit = graphicLayers.begin(); lit != graphicLayers.end(); ++lit)
   {
      if(*lit != NULL)
      {
         string testLayerName = (*lit)->getName();
         if(testLayerName == layerName)
         {
            pLayer = static_cast<GraphicLayer*>(*lit);
            break;
         }
      }
   }
   if(pLayer == NULL)
   {
      if(createNewObject)
      {
         pLayer = static_cast<GraphicLayer*>(pView->createLayer(layerType, NULL, layerName));
      }
      if(pLayer == NULL)
      {
         return false;
      }
   }

   if(objName.empty() && objType.empty())
   {
      // caller only wants the layer, not an object
      return true;
   }
   // get the object
   if(!objName.empty() && objType.empty())
   {
      // specified an object name
      pObject = pLayer->getObjectByName(objName);
   }
   else if(!objType.empty())
   {
      // specified an object type and index
      list<GraphicObject*> objects;
      GraphicObjectType type = StringUtilities::fromXmlString<GraphicObjectType>(objType);
      pLayer->getObjects(type, objects);
      if(objects.size() > objIdx)
      {
         list<GraphicObject*>::iterator it = objects.begin();
         for(unsigned int i = 0; i < objIdx; i++, ++it) ; // empty body
         pObject = *it;
      }
   }
   else if(objType.empty())
   {
      // specified an object index
      list<GraphicObject*> objects;
      pLayer->getObjects(objects);
      if(objects.size() > objIdx)
      {
         list<GraphicObject*>::iterator it = objects.begin();
         for(unsigned int i = 0; i < objIdx; i++, ++it) ; // empty body
         pObject = *it;
      }
   }
   if(pObject == NULL && createNewObject)
   {
      VERIFY((pObject = pLayer->addObject(newObjectType)) != NULL);
      if(!objName.empty())
      {  
         pObject->setName(objName.c_str());
      }
   }
   return pObject != NULL;
}
