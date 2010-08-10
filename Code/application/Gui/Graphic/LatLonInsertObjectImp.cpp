/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include "XercesIncludes.h"

#include "LatLonInsertObjectImp.h"
#include "AnnotationLayer.h"
#include "AnnotationLayerImp.h"
#include "ArrowObject.h"
#include "ArrowObjectImp.h"
#include "AppAssert.h"
#include "DrawUtil.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "GraphicProperty.h"
#include "LatLonLayer.h"
#include "LayerList.h"
#include "ProductView.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "TextObject.h"
#include "TextObjectImp.h"

#include <list>
using namespace std;

XERCES_CPP_NAMESPACE_USE

// set the constants
const static std::string UNAVAILABLE = "Lat/Long Coordinates Unavailable";
const static int BASE_HANDLE = 0;
const static int TIP_HANDLE = 1;
const static float PCT = 0.3f;   // how much of the arrow we want in relation to 1/2 the text width

LatLonInsertObjectImp::LatLonInsertObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                             LocationType pixelCoord) :
   GraphicObjectImp(id, type, pLayer, pixelCoord),
   mpGroup(GROUP_OBJECT, pLayer, pixelCoord)
{
   addProperty("FillColor");
   addProperty("FillStyle");
   addProperty("Font");
   addProperty("HatchStyle");
   addProperty("LatLon");
   addProperty("LineColor");
   addProperty("LineStyle");
   addProperty("LineWidth");
   addProperty("Scale");
   addProperty("TextColor");

   mpGeoreference.addSignal(SIGNAL_NAME(RasterElement, GeoreferenceModified), 
      Slot(this, &LatLonInsertObjectImp::georeferenceModified));

   // Create the arrow object
   ArrowObjectImp* pArrow = dynamic_cast<ArrowObjectImp*> (mpGroup->addObject(ARROW_OBJECT, pixelCoord));
   INVARIANT_DEBUG(pArrow != NULL);

   // Create the text object
   TextObjectImp* pLatLonText = dynamic_cast<TextObjectImp*> (mpGroup->addObject(TEXT_OBJECT, pixelCoord));
   INVARIANT_DEBUG (pLatLonText != NULL);

   // Set the default properties
   setFillColor(GraphicLayer::getSettingFillColor());
   setFillState(false);
   setFillStyle(EMPTY_FILL);
   setFont(AnnotationLayerImp::getDefaultFont());
   setHatchStyle(GraphicLayer::getSettingHatchStyle());
   setLineColor(GraphicLayer::getSettingLineColor());
   setLineState(GraphicLayer::getSettingBorder());
   setLineStyle(GraphicLayer::getSettingLineStyle());
   setLineWidth(GraphicLayer::getSettingLineWidth());
   setTextColor(GraphicLayer::getSettingTextColor());

   // Set the default text
   if (pLatLonText->setText(UNAVAILABLE))
   {
      pLatLonText->updateTexture();
   }

   // Make a vertical arrow by default
   double delta = 0.5 * fabs(pLatLonText->getUrCorner().mX - pLatLonText->getLlCorner().mX);

   LocationType llCorner(pixelCoord.mX, pixelCoord.mY - (PCT * delta));
   LocationType urCorner = pixelCoord;
   mHandles.push_back(llCorner); // add the tail of the arrow to special handles
   mHandles.push_back(urCorner); // add the head of the arrow to special handles

   // Set the bounding box
   pArrow->setBoundingBox(llCorner, urCorner);

   double textHeight = fabs(pLatLonText->getUrCorner().mY - pLatLonText->getLlCorner().mY);
   pLatLonText->move(LocationType(-delta, (-PCT * delta) - textHeight));

   setBoundingBox( mpGroup->getLlCorner(), mpGroup->getUrCorner() );
   updateHandles();
   updateLatLon();
}

LatLonInsertObjectImp::~LatLonInsertObjectImp()
{
   const list<GraphicObject*>& objects = mpGroup->getObjects();

   list<GraphicObject*>::const_iterator iter;
   for (iter = objects.begin(); iter != objects.end(); iter = objects.begin())
   {
      GraphicObject* pObject = (*iter);
      if (pObject != NULL)
      {
         mpGroup->removeObject(pObject, true);
         pObject = NULL;
      }
   }
}

void LatLonInsertObjectImp::setLayer(GraphicLayer* pLayer)
{
   GraphicObjectImp::setLayer(pLayer);
   mpGroup->setLayer(pLayer);
}

void LatLonInsertObjectImp::draw(double zoomFactor) const
{
   const_cast<LatLonInsertObjectImp*>(this)->updateGeoreferenceAttachment();
   mpGroup->draw(zoomFactor);
}

void LatLonInsertObjectImp::updateHandles()
{
   GraphicObjectImp::updateHandles();

   ArrowObjectImp* pArrow = getArrowObject();
   if (pArrow != NULL)
   {
      LocationType llCorner = pArrow->getLlCorner();
      LocationType urCorner = pArrow->getUrCorner();

      mHandles.push_back(llCorner);
      mHandles.push_back(urCorner);
   }
}

bool LatLonInsertObjectImp::setProperty(const GraphicProperty* pProp)
{
   if (pProp == NULL)
   {
      return false;
   }

   ArrowObjectImp* pArrow = getArrowObject();
   TextObjectImp* pLatLonText = getTextObject();

   const string& name = pProp->getName();
   if (name == "Rotation")
   {
      return false;
   }
   else if (name == "BoundingBox")
   {
      mpGroup->setProperty(pProp);
      mpGroup->updateLayout();
   }
   else if ((name == "LineWidth") || (name == "LineColor") || (name == "LineStyle") || (name == "Scale"))
   {
      if (pArrow != NULL)
      {
         pArrow->setProperty(pProp);
      }
   }
   else if (name == "Font")
   {
      if ((pLatLonText != NULL) && (pArrow != NULL))
      {
         pLatLonText->setProperty(pProp);
         pLatLonText->updateTexture();

         pArrow->moveHandle(BOTTOM_LEFT, 
            LocationType((pLatLonText->getLlCorner().mX + pLatLonText->getUrCorner().mX) / 2.0,
            pLatLonText->getUrCorner().mY));

         setBoundingBox(mpGroup->getLlCorner(), mpGroup->getUrCorner());
         updateHandles();
      }
   }
   else if ((name == "FillColor") || (name == "FillStyle") || (name == "HatchStyle") || (name == "TextColor"))
   {
      if (pLatLonText != NULL)
      {
         pLatLonText->setProperty(pProp);
      }
   }

   bool bSuccess = false;
   bSuccess = GraphicObjectImp::setProperty(pProp);
   if ((bSuccess == true) && (name == "LatLon"))
   {
      updateLatLonText();
   }

   return bSuccess;
}

GraphicProperty* LatLonInsertObjectImp::getProperty(const std::string &name) const
{
   if (name == "TextString")
   {
      TextObjectImp* pText = getTextObject();
      if (pText != NULL)
      {
         return pText->getProperty(name);
      }
   }

   return GraphicObjectImp::getProperty(name);
}

void LatLonInsertObjectImp::move(LocationType delta)
{
   // just move the text and the arrow base

   ArrowObjectImp* pArrow = getArrowObject();
   LocationType arrowBase = pArrow->getLlCorner();
   pArrow->moveHandle(BOTTOM_LEFT, arrowBase+delta);

   TextObjectImp* pText = getTextObject();
   pText->move(delta);

   setBoundingBox(mpGroup->getLlCorner(), mpGroup->getUrCorner());
   updateHandles();
}

void LatLonInsertObjectImp::moveHandle(int handle, LocationType pixel, bool bMaintainAspect)
{
   ArrowObjectImp* pArrow = getArrowObject();
   TextObjectImp* pLatLonText = getTextObject();

   if (handle == BASE_HANDLE)
   {
      move(pixel - mHandles[BASE_HANDLE]);
   }
   else if (handle == TIP_HANDLE)
   {
      if (pArrow != NULL)
      {
         // Move the handle
         mHandles[handle] = pixel;

         // Move the arrow UR corner (tip)
         pArrow->moveHandle(TOP_RIGHT, pixel);

         updateLatLon();
         setBoundingBox(mpGroup->getLlCorner(), mpGroup->getUrCorner());
         updateHandles();
      }
   }
   else
   {
      LocationType oldPoint = mHandles[handle];
      LocationType delta = pixel - oldPoint;
      if (fabs(delta.mY) < 0.5)
      {
         delta.mY = 0.0;
      }
      if (fabs(delta.mX) < 0.5)
      {
         delta.mX = 0.0;
      }
      move(delta);
   }
}

bool LatLonInsertObjectImp::hit(LocationType pixelCoord) const
{
   return mpGroup->hit(pixelCoord);
}

bool LatLonInsertObjectImp::replicateObject(const GraphicObject* pObject)
{
   const LatLonInsertObjectImp* pLLIObject = dynamic_cast<const LatLonInsertObjectImp*>(pObject);
   if (pLLIObject == NULL)
   {
      return false;
   }

   const GraphicGroup& group = pLLIObject->getGroup();

   bool bSuccess = mpGroup->replicateObject(&group);
   if (bSuccess == true)
   {
      bSuccess = GraphicObjectImp::replicateObject(pObject);
   }

   return bSuccess;
}

bool LatLonInsertObjectImp::toXml(XMLWriter* pXml) const
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

bool LatLonInsertObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicObjectImp::fromXml(pDocument, version);
   if (bSuccess == true)
   {
      mpGroup->removeAllObjects(true);

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

      if (bSuccess == true)
      {
         setBoundingBox(mpGroup->getLlCorner(), mpGroup->getUrCorner());
         updateHandles();
      }
   }

   return bSuccess;
}

const string& LatLonInsertObjectImp::getObjectType() const
{
   static string type("LatLonInsertObjectImp");
   return type;
}

bool LatLonInsertObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "LatLonInsertObject"))
   {
      return true;
   }

   return GraphicObjectImp::isKindOf(className);
}

LocationType LatLonInsertObjectImp::getLatLonLoc() const
{
   LocationType location;

   ArrowObjectImp* pArrow = getArrowObject();
   if (pArrow != NULL)
   {
      location = pArrow->getUrCorner();
   }

   return location;
}

void LatLonInsertObjectImp::updateLatLon()
{
   if (mpGeoreference.get() == NULL)
   {
      return;
   }

   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   if (pLayer == NULL)
   {
      return;
   }

   // Get the scene coordinate at the arrow tip location
   LocationType sceneCoord;
   if (dynamic_cast<SpatialDataView*>(pLayer->getView()) != NULL)
   {
      sceneCoord = getLatLonLoc();
   }
   else
   {
      ProductView* pProductView = dynamic_cast<ProductView*>(pLayer->getView());
      if (pProductView != NULL)
      {
         if (pLayer == dynamic_cast<GraphicLayerImp*>(pProductView->getLayoutLayer()))
         {
            LocationType paperCoord = getLatLonLoc();
            LocationType worldCoord;
            pLayer->translateDataToWorld(paperCoord.mX, paperCoord.mY, worldCoord.mX, worldCoord.mY);
            LocationType screenCoord;
            pProductView->translateWorldToScreen(worldCoord.mX, worldCoord.mY, screenCoord.mX, screenCoord.mY);

            GraphicObject* pViewObject = NULL;
            pViewObject = pProductView->getActiveEditObject();
            if (pViewObject != NULL)
            {
               QPoint screenPoint(static_cast<int>(screenCoord.mX), static_cast<int>(screenCoord.mY));
               if (pLayer->hit(screenPoint) == pViewObject)
               {
                  SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pViewObject->getObjectView());
                  if (pSpatialDataView != NULL)
                  {
                     pSpatialDataView->translateScreenToWorld(screenCoord.mX, screenCoord.mY,
                        sceneCoord.mX, sceneCoord.mY);
                  }
               }
            }
         }
      }
   }

   // Update the lat/long value
   LocationType dataCoord;
   pLayer->translateWorldToData(sceneCoord.mX, sceneCoord.mY, dataCoord.mX, dataCoord.mY);

   LatLonPoint latLonPoint = mpGeoreference->convertPixelToGeocoord(dataCoord);
   if (setLatLon(latLonPoint) == true)
   {
      updateLatLonText();
   }
}

void LatLonInsertObjectImp::updateLatLonText()
{
   TextObjectImp* pLatLonText = getTextObject();
   if (pLatLonText == NULL)
   {
      return;
   }

   string latLonText = UNAVAILABLE;
   if (mpGeoreference.get() != NULL)
   {
      // Get the text format from the current lat/long layer
      GeocoordType eGeocoord = LatLonLayer::getSettingGeocoordType();
      DmsFormatType eDmsFormat = LatLonLayer::getSettingFormat();

      GraphicLayer* pLayer = getLayer();
      if (pLayer != NULL)
      {
         View* pView = pLayer->getView();
         if (pView != NULL)
         {
            SpatialDataView* pSpatialView = dynamic_cast<SpatialDataView*>(pView);
            if (pSpatialView == NULL)
            {
               ProductView* pProductView = dynamic_cast<ProductView*>(pView);
               pSpatialView = dynamic_cast<SpatialDataView*>(pProductView->getActiveEditView());
            }

            if (pSpatialView != NULL)
            {
               LatLonLayer* pLatLonLayer = dynamic_cast<LatLonLayer*>(pSpatialView->getTopMostLayer(LAT_LONG));
               if (pLatLonLayer != NULL)
               {
                  eGeocoord = pLatLonLayer->getGeocoordType();
                  eDmsFormat = pLatLonLayer->getLatLonFormat();
               }
            }
         }
      }

      // Get the lat/long text in the current text format
      LatLonPoint latLonPoint = getLatLon();
      if (eGeocoord == GEOCOORD_LATLON)
      {
         latLonText = latLonPoint.getText(eDmsFormat);
      }
      else if (eGeocoord == GEOCOORD_UTM)
      {
         UtmPoint utmPoint(latLonPoint);
         latLonText = utmPoint.getText();
      }
      else if (eGeocoord == GEOCOORD_MGRS)
      {
         MgrsPoint mgrsPoint(latLonPoint);
         latLonText = mgrsPoint.getText();
      }
   }

   // Only update the text object if the text has changed
   if (latLonText != pLatLonText->getText())
   {
      // Set the text
      if (pLatLonText->setText(latLonText))
      {
         pLatLonText->updateTexture();
      }

      // Update the arrow handle
      ArrowObjectImp* pArrow = getArrowObject();
      if (pArrow != NULL)
      {
         pArrow->moveHandle(BOTTOM_LEFT,
            LocationType((pLatLonText->getLlCorner().mX + pLatLonText->getUrCorner().mX) / 2.0,
            pLatLonText->getUrCorner().mY));
      }

      // Update the bounding box
      setBoundingBox(mpGroup->getLlCorner(), mpGroup->getUrCorner());

      // Update the handles
      updateHandles();
   }
}

TextObjectImp *LatLonInsertObjectImp::getTextObject() const
{
   const list<GraphicObject*>& objects = mpGroup->getObjects();
   if (objects.empty() == true)
   {
      return NULL;
   }

   return dynamic_cast<TextObjectImp*> (objects.back());
}

ArrowObjectImp *LatLonInsertObjectImp::getArrowObject() const
{
   const list<GraphicObject*>& objects = mpGroup->getObjects();
   if (objects.empty() == true)
   {
      return NULL;
   }

   return dynamic_cast<ArrowObjectImp*> (objects.front());
}

const GraphicGroup& LatLonInsertObjectImp::getGroup() const
{
   return *dynamic_cast<const GraphicGroup*>(mpGroup.get());
}

bool LatLonInsertObjectImp::processMousePress(LocationType screenCoord, Qt::MouseButton button,
                                              Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   // all insertion is done in the constructor
   GraphicLayerImp* pLayerImp = dynamic_cast<GraphicLayerImp*>(getLayer());
   if (pLayerImp != NULL)
   {
      pLayerImp->completeInsertion();
   }
   
   return false;
}

bool LatLonInsertObjectImp::processMouseMove(LocationType screenCoord, Qt::MouseButton button,
                                             Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   // should never get here
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   VERIFY(pLayer != NULL);
   pLayer->completeInsertion(false);

   VERIFY(false);
}

bool LatLonInsertObjectImp::processMouseRelease(LocationType screenCoord, Qt::MouseButton button,
                                                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   // should never get here
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   VERIFY(pLayer != NULL);
   pLayer->completeInsertion(false);

   VERIFY(false);
}

bool LatLonInsertObjectImp::hasCornerHandles() const
{
   return false;
}

void LatLonInsertObjectImp::updateGeo()
{
   // Do nothing, we're always listening
}

void LatLonInsertObjectImp::georeferenceModified(Subject &subject, const std::string &signal, const boost::any &v)
{
   // keep the text in place, move the arrow to the appropriate point

   ArrowObjectImp* pArrow = getArrowObject();
   VERIFYNRV(pArrow != NULL);

   BoundingBoxProperty* pProp = dynamic_cast<BoundingBoxProperty*>(pArrow->getProperty("BoundingBox"));
   VERIFYNRV(pProp != NULL);

   updateGeoreferenceAttachment();
   if (mpGeoreference.get() != NULL && pProp->hasGeoCoords())
   {
      LocationType llLatLong = mpGeoreference->convertPixelToGeocoord(pProp->getLlCorner());
      BoundingBoxProperty newProp(llLatLong, pProp->getUrLatLong(), true);
      pArrow->setProperty(&newProp);
   }

   updateHandles();
   setBoundingBox(mpGroup->getLlCorner(), mpGroup->getUrCorner());
}

void LatLonInsertObjectImp::updateGeoreferenceAttachment()
{
   if (mpGeoreference.get() == NULL)
   {
      const RasterElement* pGeoreference = getGeoreferenceElement();
      if (pGeoreference != NULL)
      {
         mpGeoreference.reset(const_cast<RasterElement*>(pGeoreference));
         updateLatLon();
      }
   }
}
