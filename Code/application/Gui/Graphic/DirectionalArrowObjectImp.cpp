/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "XercesIncludes.h"

#include "DirectionalArrowObjectImp.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "glCommon.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "StringUtilities.h"
#include "TypesFile.h"

#include <string>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

using namespace std;

XERCES_CPP_NAMESPACE_USE

DirectionalArrowObjectImp::DirectionalArrowObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                                     LocationType pixelCoord) :
   GraphicObjectImp(id, type, pLayer, pixelCoord),
   mpGroup(GROUP_OBJECT, pLayer, pixelCoord),
   mbOriented(false)
{
   addProperty("FillColor");
   addProperty("LineColor");
   addProperty("LineWidth");
   addProperty("TextColor");

   mpGeoreference.addSignal(SIGNAL_NAME(RasterElement, GeoreferenceModified),
      Slot(this, &DirectionalArrowObjectImp::georeferenceModified));
}

DirectionalArrowObjectImp::~DirectionalArrowObjectImp()
{
   const list<GraphicObject*>& objects = mpGroup->getObjects();

   list<GraphicObject*>::const_iterator iter;
   for (iter = objects.begin(); iter != objects.end(); iter = objects.begin())
   {
      GraphicObject* pObject = NULL;
      pObject = (*iter);
      if (pObject != NULL)
      {
         mpGroup->removeObject(pObject, true);
         pObject = NULL;
      }
   }
}

void DirectionalArrowObjectImp::setLayer(GraphicLayer* pLayer)
{
   GraphicObjectImp::setLayer(pLayer);
   mpGroup->setLayer(pLayer);
}

void DirectionalArrowObjectImp::draw(double zoomFactor) const
{
   const_cast<DirectionalArrowObjectImp*>(this)->updateGeoreferenceAttachment();
   bool bRotatedMatrix = false;
   if (isOriented() == false)
   {
      const_cast<DirectionalArrowObjectImp*>(this)->orient();
      if (isOriented() == true)
      {
         // ordinarily, this is done by the GraphicGroup, but since we've changed the
         // rotation, we need to update it ourselves
         rotateViewMatrix();
         bRotatedMatrix = true;
      }
   }

   mpGroup->draw(zoomFactor);

   if (bRotatedMatrix == true)
   {
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
   }
}

bool DirectionalArrowObjectImp::setProperty(const GraphicProperty* pProperty)
{
   if (pProperty == NULL)
   {
      return false;
   }

   string propertyName = pProperty->getName();
   if (propertyName == "Rotation")
   {
      if (isOriented() == true)
      {
         return false;
      }
   }

   bool bSuccess = false;
   bSuccess = mpGroup->setProperty(pProperty);
   if (bSuccess == true)
   {
      mpGroup->updateLayout();
      bSuccess = GraphicObjectImp::setProperty(pProperty);
   }

   return bSuccess;
}

void DirectionalArrowObjectImp::moveHandle(int handle, LocationType pixel, bool bMaintainAspect)
{
   double dWidth = 0.0;
   double dHeight = 0.0;

   switch (handle)
   {
      case 0:
         dWidth = mHandles[4].mX - pixel.mX;
         dHeight = mHandles[4].mY - pixel.mY;
         break;

      case 1:
         dHeight = mHandles[5].mY - pixel.mY;
         break;

      case 2:
         dWidth = pixel.mX - mHandles[6].mX;
         dHeight = mHandles[6].mY - pixel.mY;
         break;

      case 3:
         dWidth = pixel.mX - mHandles[7].mX;
         break;

      case 4:
         dWidth = pixel.mX - mHandles[0].mX;
         dHeight = pixel.mY - mHandles[0].mY;
         break;

      case 5:
         dHeight = pixel.mY - mHandles[1].mY;
         break;

      case 6:
         dWidth = mHandles[2].mX - pixel.mX;
         dHeight = pixel.mY - mHandles[2].mY;
         break;

      case 7:
         dWidth = mHandles[3].mX - pixel.mX;
         break;

      default:
         break;
   }

   if ((dWidth < 0.0) || (dHeight < 0.0))
   {
      return;
   }

   mpGroup->moveHandle(handle, pixel, true);
   GraphicObjectImp::moveHandle(handle, pixel, true);
}

bool DirectionalArrowObjectImp::hit(LocationType pixelCoord) const
{
   bool bHit = false;
   bHit = mpGroup->hit(pixelCoord);

   return bHit;
}

bool DirectionalArrowObjectImp::isOriented() const
{
   return mbOriented;
}

bool DirectionalArrowObjectImp::replicateObject(const GraphicObject* pObject)
{
   const DirectionalArrowObjectImp* pDirArrowObject = dynamic_cast<const DirectionalArrowObjectImp*>(pObject);
   if (pDirArrowObject == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicObjectImp::replicateObject(dynamic_cast<const GraphicObject*>(pDirArrowObject));
   if (bSuccess == false)
   {
      return false;
   }

   const GraphicGroup& group = pDirArrowObject->getGroup();
   bSuccess = mpGroup->replicateObject(&group);
   if (bSuccess == true)
   {
      updateHandles();
   }

   return bSuccess;
}

bool DirectionalArrowObjectImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicObjectImp::toXml(pXml);
   if (bSuccess == true)
   {
      pXml->pushAddPoint(pXml->addElement("objects"));
      pXml->pushAddPoint(pXml->addElement("Graphic"));

      bSuccess = mpGroup->toXml(pXml);

      pXml->popAddPoint();
      pXml->popAddPoint();
   }

   return bSuccess;
}

bool DirectionalArrowObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicObjectImp::fromXml(pDocument, version);
   if (bSuccess == true)
   {
      const list<GraphicObject*>& groupObjects = mpGroup->getObjects();
      if (groupObjects.empty() == false)
      {
         // Only call removeAllObjects() if objects exist because it calls completeInsertion() on the graphic layer,
         // which removes the undo lock when the user adds this object into the layer.  This then adds numerous
         // actions to the undo stack which should not be added.
         mpGroup->removeAllObjects(true);
      }

      mbOriented = false;

      DOMNode* pObjectNode = pDocument->getFirstChild();
      while (pObjectNode != NULL)
      {
         if (XMLString::equals(pObjectNode->getNodeName(), X("objects")))
         {
            DOMNode* pGroupNode = pObjectNode->getFirstChild();
            while (pGroupNode != NULL)
            {
               if (XMLString::equals(pGroupNode->getNodeName(), X("Graphic")))
               {
                  DOMElement* pElement(static_cast<DOMElement*> (pGroupNode));
                  string type(A(pElement->getAttribute(X("type"))));

                  GraphicObjectType objectType = StringUtilities::fromXmlString<GraphicObjectType>(type);
                  if (objectType == GROUP_OBJECT)
                  {
                     bSuccess = mpGroup->fromXml(pGroupNode, version);
                     break;
                  }
               }

               pGroupNode = pGroupNode->getNextSibling();
            }
         }

         pObjectNode = pObjectNode->getNextSibling();
      }
   }

   return bSuccess;
}

const string& DirectionalArrowObjectImp::getObjectType() const
{
   static string type("DirectionalArrowObjectImp");
   return type;
}

bool DirectionalArrowObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DirectionalArrowObject"))
   {
      return true;
   }

   return GraphicObjectImp::isKindOf(className);
}

const GraphicGroup& DirectionalArrowObjectImp::getGroup() const
{
   return *(dynamic_cast<const GraphicGroup*>(mpGroup.get()));
}

bool DirectionalArrowObjectImp::import(const string& filename)
{
   bool bSuccess = false;

   // Try to load as XML
   Service<MessageLogMgr> pMsgLog;
   MessageLog* pLog = pMsgLog->getLog();

   XmlReader xml(pLog);

   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDocument = NULL;
   pDocument = xml.parse(filename);
   if (pDocument != NULL)
   {
      XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pRootElement = NULL;
      pRootElement = pDocument->getDocumentElement();
      if (pRootElement != NULL)
      {
         unsigned int version = atoi(A(pRootElement->getAttribute(X("version"))));
         try
         {
            bSuccess = fromXml(pRootElement, version);
         }
         catch (XmlReader::DomParseException&)
         {
            bSuccess = false;
         }
      }
   }

   return bSuccess;
}

void DirectionalArrowObjectImp::orient()
{
   mbOriented = true;
}

void DirectionalArrowObjectImp::updateOrientation()
{
   if (isOriented() == true)
   {
      mbOriented = false;
      orient();
   }
}

bool DirectionalArrowObjectImp::processMousePress(LocationType screenCoord, 
                                 Qt::MouseButton button,
                                 Qt::MouseButtons buttons,
                                 Qt::KeyboardModifiers modifiers)
{
   if (!isOriented())
   {
      QMessageBox::warning(qApp->activeWindow(), APP_NAME, 
         "The arrow has not been oriented.  "
         "The data set may not be georeferenced.\nThe arrow orientation will "
         "automatically "
         "update when georeferencing is performed.");
   }

   GraphicLayerImp* pLayerImp = dynamic_cast<GraphicLayerImp*>(getLayer());
   if (pLayerImp != NULL)
   {
      pLayerImp->completeInsertion();
   }

   return true; // bounding box is set up by constructor
}

bool DirectionalArrowObjectImp::processMouseMove(LocationType screenCoord, 
                                 Qt::MouseButton button,
                                 Qt::MouseButtons buttons,
                                 Qt::KeyboardModifiers modifiers)
{
   // should never get here
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   VERIFY(pLayer != NULL);
   pLayer->completeInsertion(false);

   VERIFY(false);
}

bool DirectionalArrowObjectImp::processMouseRelease(LocationType screenCoord, 
                                 Qt::MouseButton button,
                                 Qt::MouseButtons buttons,
                                 Qt::KeyboardModifiers modifiers)
{
   // should never get here
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   VERIFY(pLayer != NULL);
   pLayer->completeInsertion(false);

   VERIFY(false);
}

void DirectionalArrowObjectImp::updateGeo()
{
   // Do nothing, we're always listening
}

void DirectionalArrowObjectImp::georeferenceModified(Subject &subject, const std::string &signal, const boost::any &v)
{
   updateOrientation();
}

void DirectionalArrowObjectImp::updateGeoreferenceAttachment()
{
   if (mpGeoreference.get() == NULL)
   {
      const RasterElement* pGeoreference = getGeoreferenceElement();
      if (pGeoreference != NULL)
      {
         mpGeoreference.reset(const_cast<RasterElement*>(pGeoreference));
         updateOrientation();
      }
   }
}
