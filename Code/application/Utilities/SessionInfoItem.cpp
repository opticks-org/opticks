/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SessionInfoItem.h"

#include "AnnotationLayer.h"
#include "ApplicationWindow.h"
#include "ClassificationLayer.h"
#include "DesktopServices.h"
#include "GraphicLayer.h"
#include "Layer.h"
#include "LayerList.h"
#include "MouseMode.h"
#include "ProductView.h"
#include "Resource.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManagerImp.h"
#include "SpatialDataView.h"
#include "View.h"
#include "WorkspaceWindow.h"
#include "xmlreader.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

SessionInfoItem::SessionInfoItem(const string& id, const string& name) :
   SettableSessionItemAdapter(id)
{
   SettableSessionItemAdapter::setName(name);
}

SessionInfoItem::~SessionInfoItem()
{
}


const string& SessionInfoItem::getObjectType() const
{
   static string type("SessionInfoItem");
   return type;
}

bool SessionInfoItem::serialize(SessionItemSerializer &serializer) const
{
   XMLWriter xml(getObjectType().c_str());

   if (!toXml(&xml))
   {
      return false;
   }

   return serializer.serialize(xml);
}

bool SessionInfoItem::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader xml(NULL, false);
   DOMNode* pRoot = deserializer.deserialize(xml, getObjectType().c_str());
   return fromXml(pRoot, XmlBase::VERSION);
}

bool SessionInfoItem::toXml(XMLWriter* pXml) const
{
   Service<DesktopServices> pDesk;
   ApplicationWindow* pApp = dynamic_cast<ApplicationWindow*>(pDesk->getMainWidget());
   if (pApp != NULL)
   {
      WorkspaceWindow* pWork = pApp->getCurrentWorkspaceWindow();
      if (pWork != NULL)
      {
         pXml->addAttr("currentWorkspaceWindowId", pWork->getId());
         View* pView = pWork->getActiveView();
         if (pView == NULL)
         {
            // nothing more to save
            return true;
         }
         pXml->addAttr("activeViewId", pView->getId());

         Layer* pLayer(NULL);
         SpatialDataView* pSDV = dynamic_cast<SpatialDataView*>(pView);
         if (pSDV != NULL)
         {
            pLayer = pSDV->getActiveLayer();
         }
         else
         {
            ProductView* pPV = dynamic_cast<ProductView*>(pView);
            if (pPV != NULL)
            {
               pLayer = static_cast<Layer*>(pPV->getActiveLayer());
            }
         }

         if (pLayer != NULL)
         {
            pXml->addAttr("activeLayerName", pLayer->getName());
            pXml->addAttr("activeLayerType", pLayer->getLayerType());
            MouseMode* pMode = pView->getCurrentMouseMode();
            if (pMode != NULL)
            {
               string mouseModeStr;
               pMode->getName(mouseModeStr);
               pXml->addAttr("activeMouseMode", mouseModeStr); 
            }
         }
      }
   }

   return true;
}

bool SessionInfoItem::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   DOMElement *pElem = static_cast<DOMElement*>(pDocument);

   if (pElem->hasAttribute(X("currentWorkspaceWindowId")))
   {
      string winId(A(pElem->getAttribute(X("currentWorkspaceWindowId"))));

      WorkspaceWindow* pWork(NULL);
      pWork = dynamic_cast<WorkspaceWindow*>(SessionManagerImp::instance()->getSessionItem(winId));

      if (pWork == NULL)
      {
         return false;
      }

      Service<DesktopServices> pDesk;
      ApplicationWindow* pApp = dynamic_cast<ApplicationWindow*>(pDesk->getMainWidget());
      VERIFYRV(pApp != NULL, false);
      pApp->setCurrentWorkspaceWindow(pWork);

      View* pView(NULL);
      if (pElem->hasAttribute(X("activeViewId")))
      {
         string viewId(A(pElem->getAttribute(X("activeViewId"))));
         pView = dynamic_cast<View*>(SessionManagerImp::instance()->getSessionItem(viewId));
      }

      if (pView != NULL && pElem->hasAttribute(X("activeLayerName")))
      {
         string layerName(A(pElem->getAttribute(X("activeLayerName"))));
         string mouseModeName(A(pElem->getAttribute(X("activeMouseMode"))));
         LayerType eType = StringUtilities::fromXmlString<LayerType>(
            A(pElem->getAttribute(X("activeLayerType"))));
         Layer* pLayer(NULL);
         ViewType vType = pView->getViewType();
         switch(vType)
         {
            case SPATIAL_DATA_VIEW:
            {
               SpatialDataView* pSDV = dynamic_cast<SpatialDataView*>(pView);
               //force resetting active layer
               LayerList* pLayerList = pSDV->getLayerList();
               if (pLayerList != NULL)
               {
                  vector<Layer*> layers;
                  vector<Layer*>::iterator it;
                  pLayerList->getLayers(eType, layers);
                  for (it=layers.begin(); it!=layers.end(); ++it)
                  {
                     string name = (*it)->getName();
                     if (name == layerName)
                     {
                        pLayer = *it;
                        break;
                     }
                  }
               }
               if (pLayer != NULL)
               {
                  pSDV->setActiveLayer(NULL);  // force reset
                  pSDV->setMouseMode(mouseModeName);
                  pSDV->setActiveLayer(pLayer);
               }
               break;
            }

            case PRODUCT_VIEW:
            {
               GraphicLayer* pGLayer(NULL);
               ProductView* pPV = dynamic_cast<ProductView*>(pView);
               if (layerName == "Classification")
               {
                  pGLayer = dynamic_cast<GraphicLayer*>(pPV->getClassificationLayer());
               }
               else
               {
                  pGLayer = dynamic_cast<GraphicLayer*>(pPV->getLayoutLayer());
               }
               if (pGLayer != NULL)
               {            
                  //force resetting active layer
                  pPV->setActiveLayer(NULL);
                  pPV->setMouseMode(mouseModeName);
                  pPV->setActiveLayer(pGLayer);
               }
               break;
            }

            default:
               break;
         }
      }
   }

   return true;
}