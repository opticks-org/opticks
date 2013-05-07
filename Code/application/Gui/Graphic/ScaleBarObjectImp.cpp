/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include "AppAssert.h"
#include "AppVerify.h"
#include "DrawUtil.h"
#include "GeoAlgorithms.h"
#include "GeoConversions.h"
#include "GraphicGroup.h"
#include "GraphicLayer.h"
#include "GraphicProperty.h"
#include "LineObjectImp.h"
#include "RasterElement.h"
#include "RectangleObjectImp.h"
#include "ScaleBarObjectImp.h"
#include "StringUtilities.h"
#include "TextObject.h"
#include "TextObjectImp.h"
#include "XercesIncludes.h"

#include <string>
#include <sstream>
#include <list>
using namespace std;

XERCES_CPP_NAMESPACE_USE

const double BOX_HEIGHT = 0.25;
const double LINE_HEIGHT = 0.375;
const double TEXT_HEIGHT = 0.375;
const double CENTER_LINE_HEIGHT = 0.05;

ScaleBarObjectImp::ScaleBarObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                     LocationType pixelCoord) :
   FilledObjectImp(id, type, pLayer, pixelCoord),
   mXgsd(1.0),
   mNeedsLayout(true),
   mpGroup(GROUP_OBJECT, pLayer, pixelCoord)
{
   addProperty("UnitSystem");

   mpView.addSignal(SIGNAL_NAME(PerspectiveView, PitchChanged), Slot(this, &ScaleBarObjectImp::viewModified));
   mpGeoreference.addSignal(SIGNAL_NAME(RasterElement, GeoreferenceModified),
      Slot(this, &ScaleBarObjectImp::georeferenceModified));

   double lineWidth = 1.0;
   double barWidth = 1e-2;

   ColorType blackColor(0, 0, 0);
   ColorType whiteColor(255, 255, 255);

   int i = 0;
   LocationType llObjCorner;
   LocationType urObjCorner;
   LocationType llCorner;
   LocationType urCorner;

   for (i = 0; i < 6; ++i)
   {
      RectangleObjectImp* pRect = dynamic_cast<RectangleObjectImp*>(mpGroup->addObject(RECTANGLE_OBJECT, pixelCoord));
      REQUIRE(pRect != NULL);

      if ((i + 1) % 2)
      {
         pRect->setFillColor(whiteColor);
      }
      else
      {
         pRect->setFillColor(blackColor);
      }

      pRect->setFillStyle(SOLID_FILL);
      pRect->setLineColor(blackColor);

      connect(pRect, SIGNAL(propertyModified(GraphicProperty*)), this,
         SIGNAL(propertyModified(GraphicProperty*)));
   }

   //define center line
   LineObjectImp* pCenterLine = dynamic_cast<LineObjectImp*>(mpGroup->addObject(LINE_OBJECT, pixelCoord));
   REQUIRE(pCenterLine != NULL);

   pCenterLine->setLineColor(blackColor);
   pCenterLine->setLineWidth(lineWidth);

   connect(pCenterLine, SIGNAL(propertyModified(GraphicProperty*)), this,
      SIGNAL(propertyModified(GraphicProperty*)));

   //define left line
   LineObjectImp* pLeftLine = dynamic_cast<LineObjectImp*>(mpGroup->addObject(LINE_OBJECT, pixelCoord));
   REQUIRE(pLeftLine != NULL);

   pLeftLine->setLineColor(blackColor);
   pLeftLine->setLineWidth(lineWidth);

   connect(pLeftLine, SIGNAL(propertyModified(GraphicProperty*)), this,
      SIGNAL(propertyModified(GraphicProperty*)));

   //define right line
   LineObjectImp* pRightLine = dynamic_cast<LineObjectImp*>(mpGroup->addObject(LINE_OBJECT, urCorner));
   REQUIRE(pRightLine != NULL);

   pRightLine->setLineColor(blackColor);
   pRightLine->setLineWidth(lineWidth);

   connect(pRightLine, SIGNAL(propertyModified(GraphicProperty*)), this,
      SIGNAL(propertyModified(GraphicProperty*)));

   //left text initialize
   string leftValue = "0";
   TextObjectImp* pLeftText = dynamic_cast<TextObjectImp*>(mpGroup->addObject(TEXT_OBJECT, pixelCoord));
   REQUIRE(pLeftText != NULL);

   pLeftText->setTextColor(blackColor);
   pLeftText->setText(leftValue);
   pLeftText->setTextEditable(false);

   connect(pLeftText, SIGNAL(propertyModified(GraphicProperty*)), this,
      SIGNAL(propertyModified(GraphicProperty*)));

   //define center text
   string centerValue = "1";
   TextObjectImp* pCenterText = dynamic_cast<TextObjectImp*>(mpGroup->addObject(TEXT_OBJECT, pixelCoord));
   REQUIRE(pCenterText != NULL);

   pCenterText->setTextColor(blackColor);
   pCenterText->setText(centerValue);
   pCenterText->setTextEditable(false);

   connect(pCenterText, SIGNAL(propertyModified(GraphicProperty*)), this,
      SIGNAL(propertyModified(GraphicProperty*)));

   //define right text
   string rightValue = "2";
   TextStringProperty rightText(rightValue);
   TextObjectImp* pRightText = dynamic_cast<TextObjectImp*>(mpGroup->addObject(TEXT_OBJECT, pixelCoord));
   REQUIRE(pRightText != NULL);

   pRightText->setTextColor(blackColor);
   pRightText->setText(rightValue);
   pRightText->setTextEditable(false);

   connect(pRightText, SIGNAL(propertyModified(GraphicProperty*)), this,
      SIGNAL(propertyModified(GraphicProperty*)));

   llCorner.mX = pixelCoord.mX;
   llCorner.mY = pixelCoord.mY;
   urCorner.mX = llCorner.mX + 5.0 * barWidth;
   urCorner.mY = llCorner.mY + barWidth;
   mpGroup->updateBoundingBox();
   setBoundingBox(llCorner, urCorner);
   updateHandles();
}

ScaleBarObjectImp::~ScaleBarObjectImp()
{
}

void ScaleBarObjectImp::setLayer(GraphicLayer* pLayer)
{
   FilledObjectImp::setLayer(pLayer);
   mpGroup->setLayer(pLayer);
}

void ScaleBarObjectImp::draw(double zoomFactor) const 
{
   const_cast<ScaleBarObjectImp*>(this)->updateAttachments();
   if (mNeedsLayout)
   {
      const_cast<ScaleBarObjectImp*>(this)->updateLayout();
   }

   mpGroup->draw(zoomFactor);
}

void ScaleBarObjectImp::updateLayout()
{
   if (mNeedsLayout == false)
   {
      return;
   }

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   bool bHorizontalFlip = false;
   bool bVerticalFlip = false;

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      pLayer->isFlipped(llCorner, urCorner, bHorizontalFlip, bVerticalFlip);
   }

   if (bHorizontalFlip == true)
   {
      double temp = llCorner.mX;
      llCorner.mX = urCorner.mX;
      urCorner.mX = temp;
   }

   if (bVerticalFlip == true)
   {
      double temp = llCorner.mY;
      llCorner.mY = urCorner.mY;
      urCorner.mY = temp;
   }

   double barHeight = urCorner.mY - llCorner.mY;

   double spacingScale = 1.0;
   string unitsText = "pix";
   double unitScale = 1.0; //mXgsd is calculated in meters
   if (mpGeoreference.get() != NULL)
   {
      if (getUnitSystem() == UNIT_KFT)
      {
         unitsText = "ft";
      }
      else if (getUnitSystem() == UNIT_KM)
      {
         unitsText = "m";
      }
      else if (getUnitSystem() == UNIT_MI)
      {
         unitsText = "mi";
      }

      // if the scalebar is an object in the layout layer, we need to scale the
      // scalebar length to scene pixels from page coordinates
      if (getUnitSystem() == UNIT_KFT)
      {
         unitScale = GeoConversions::convertMetersToFeet(1);
      }
      else if (getUnitSystem() == UNIT_MI)
      {
         unitScale = GeoConversions::convertMetersToMiles(1);
      }
   }
   else
   {
      mXgsd = 1.0;
   }

   double deltaX = urCorner.mX - llCorner.mX;
   double lengthBar = fabs(deltaX * mXgsd * unitScale);
   double totalLengthPixels = fabs(deltaX);
   double ticksize = 0.0; //ticksize in pixels
   double ticksizeDistance = pow(10.0, floor(log10(lengthBar / 5.0))); //ticksize in specific units (feet or meters)

   if (ticksizeDistance * 5.0 * 5.0 > lengthBar)
   {
      if (ticksizeDistance * 2.0 * 5.0 > lengthBar)
      {
         // times 1.0;
      }
      else
      {
         ticksizeDistance *= 2.0;
      }
   }
   else
   {
      ticksizeDistance *= 5.0;
   }

   if (mpGeoreference.get() == NULL || getUnitSystem() == UNIT_KM || getUnitSystem() == UNIT_KFT)
   {
      if (ticksizeDistance >= (1000 / 5.0)) // 5 ticks >= 1000 base units (either meters or feet)
      {
         spacingScale /= 1000.0; //meters to kilometers, feet to kilofeet
         unitsText = "k" + unitsText;
      }
   }
   else if (getUnitSystem() == UNIT_MI)
   {
      if (ticksizeDistance < (0.25 / 5.0)) //5 ticks is less than 1/4 mile, convert to feet.
      {
         spacingScale = GeoConversions::convertMilesToFeet(spacingScale);
         unitsText = "ft";
      }
   }

   // need to divide ticksize by gsd here
   if (lengthBar != 0.0)
   {
      ticksize = DrawUtil::sign(deltaX) * ticksizeDistance * totalLengthPixels / lengthBar; // ticksize in pixels
   }

   ostringstream center;
   center << (ticksizeDistance * 5.0) * spacingScale << unitsText;
   TextStringProperty centerText(center.str());

   ostringstream right;
   right << lengthBar * spacingScale << unitsText;
   string newRightVal = right.str();
   TextStringProperty rightText(right.str());
   TextStringProperty leftText("0" + unitsText);

   double width = 0.0;
   double height = 0.0;

   int i = 0;
   list<GraphicObject*> objects = mpGroup->getObjects();
   list<GraphicObject*>::iterator it = objects.begin();

   for (i = 0; it != objects.end(); ++it, ++i)
   {
      GraphicObjectImp* pObj = dynamic_cast<GraphicObjectImp*>(*it);
      TextObjectImp* pText = NULL;

      if (pObj == NULL)
      {
         continue;
      }

      LocationType llObjCorner = pObj->getLlCorner();
      LocationType urObjCorner = pObj->getUrCorner();

      switch (i)
      {
         case 0:
            // big, stretchy rectangle
            llObjCorner.mX = llCorner.mX;
            urObjCorner.mX = urCorner.mX;
            llObjCorner.mY = llCorner.mY + barHeight * (1.0-BOX_HEIGHT);
            urObjCorner.mY = llObjCorner.mY + barHeight * BOX_HEIGHT;
            pObj->setBoundingBox(llObjCorner, urObjCorner);
            
            pObj->updateHandles();
            break;
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
            // 5 rectangles
            llObjCorner.mX = ((i-1) * ticksize) + llCorner.mX;
            urObjCorner.mX = (i * ticksize) + llCorner.mX;
            llObjCorner.mY = llCorner.mY + barHeight * (1.0-BOX_HEIGHT);
            urObjCorner.mY = llObjCorner.mY + barHeight * BOX_HEIGHT;
            pObj->setBoundingBox(llObjCorner, urObjCorner);
            pObj->updateHandles();
            break;
         case 6:
            //center line
            llObjCorner.mX = (5.0 * ticksize) + llCorner.mX;
            urObjCorner.mX = llObjCorner.mX;
            llObjCorner.mY = llCorner.mY + barHeight * (1.0-BOX_HEIGHT - CENTER_LINE_HEIGHT);
            urObjCorner.mY = llObjCorner.mY + barHeight * CENTER_LINE_HEIGHT;
            pObj->setBoundingBox(llObjCorner, urObjCorner);
            pObj->updateHandles();
            break;
         case 7:
            // left line
            llObjCorner.mX = llCorner.mX;
            urObjCorner.mX = llCorner.mX;
            llObjCorner.mY = llCorner.mY + barHeight * TEXT_HEIGHT;
            urObjCorner.mY = llObjCorner.mY + barHeight * LINE_HEIGHT;
            pObj->setBoundingBox(llObjCorner, urObjCorner);
            pObj->updateHandles();
            break;
         case 8:
            //right line
            llObjCorner.mX = urCorner.mX;
            urObjCorner.mX = llObjCorner.mX;
            llObjCorner.mY = llCorner.mY + barHeight * TEXT_HEIGHT;
            urObjCorner.mY = llObjCorner.mY + barHeight * LINE_HEIGHT;
            pObj->setBoundingBox(llObjCorner, urObjCorner);
            pObj->updateHandles();
            break;
         case 9:
         {
            //left text
            pText = dynamic_cast<TextObjectImp*>(pObj);
            VERIFYNRV(pText != NULL);
            if (pObj->getText() != leftText.getString())
            {
               pText->setProperty(&leftText);
               pText->updateTexture();
            }
            else
            {
               pText->updateBoundingBox();
            }

            LocationType textLlCorner = pText->getLlCorner();
            LocationType textUrCorner = pText->getUrCorner();
            width = textUrCorner.mX - textLlCorner.mX;
            height = textUrCorner.mY - textLlCorner.mY;
            llObjCorner.mX = llCorner.mX;
            llObjCorner.mY = llCorner.mY;
            if (height != 0.0)
            {
               urObjCorner.mX = llObjCorner.mX + width * barHeight * TEXT_HEIGHT / height;
            }
            else
            {
               urObjCorner.mX = llObjCorner.mX + width;
            }
            urObjCorner.mY = llObjCorner.mY + barHeight * TEXT_HEIGHT;
            pObj->setBoundingBox(llObjCorner, urObjCorner);
            break;
         }
         case 10:
         {
            //center text
            pText = dynamic_cast<TextObjectImp*>(pObj);
            VERIFYNRV(pText != NULL);
            if (pObj->getText() != centerText.getString())
            {
               pText->setProperty(&centerText);
               pText->updateTexture();
            }
            else
            {
               pText->updateBoundingBox();
            }

            LocationType textLlCorner = pText->getLlCorner();
            LocationType textUrCorner = pText->getUrCorner();
            width = textUrCorner.mX - textLlCorner.mX;
            height = textUrCorner.mY - textLlCorner.mY;

            if (height != 0.0)
            {
               llObjCorner.mX = llCorner.mX + (5.0 * ticksize) - (width / 2.0) * barHeight * TEXT_HEIGHT / height;
               urObjCorner.mX = llObjCorner.mX + width * barHeight * TEXT_HEIGHT / height;
            }
            else
            {
               llObjCorner.mX = llCorner.mX + (5.0 * ticksize);
               urObjCorner.mX = llObjCorner.mX + width;
            }
            llObjCorner.mY = llCorner.mY + (1.0-BOX_HEIGHT-TEXT_HEIGHT-CENTER_LINE_HEIGHT)*barHeight;
            urObjCorner.mY = llObjCorner.mY + barHeight*TEXT_HEIGHT;
            pObj->setBoundingBox(llObjCorner, urObjCorner);
            break;
         }
         case 11:
         {
            //right text
            pText = dynamic_cast<TextObjectImp*>(pObj);
            VERIFYNRV(pText != NULL);
            if (pObj->getText() != rightText.getString())
            {
               pText->setProperty(&rightText);
               pText->updateTexture();
            }
            else
            {
               pText->updateBoundingBox();
            }

            LocationType textLlCorner = pText->getLlCorner();
            LocationType textUrCorner = pText->getUrCorner();
            width = textUrCorner.mX - textLlCorner.mX;
            height = textUrCorner.mY - textLlCorner.mY;
            if (height != 0.0)
            {
               llObjCorner.mX = urCorner.mX - width * barHeight * TEXT_HEIGHT / height;
               urObjCorner.mX = llObjCorner.mX + width * barHeight * TEXT_HEIGHT / height;
            }
            else
            {
               llObjCorner.mX = urCorner.mX;
               urObjCorner.mX = llObjCorner.mX + width;
            }
            llObjCorner.mY = llCorner.mY;
            urObjCorner.mY = llObjCorner.mY + barHeight*TEXT_HEIGHT;
            pObj->setBoundingBox(llObjCorner, urObjCorner);
            break;
         }

         default:
            break;
      }
   }

   mpGroup->updateBoundingBox();
   mNeedsLayout = false;
}

bool ScaleBarObjectImp::setProperty(const GraphicProperty* pProp)
{
   GraphicObjectImp* pCurrentObject = NULL;
   list<GraphicObject*> objects;
   objects = mpGroup->getObjects();
   int i = 0;
   list<GraphicObject*>::iterator it;
   it = objects.begin();
   string name = pProp->getName();

   if ((name == "Rotation") || (name == "BoundingBox") || (name =="UnitSystem"))
   {
      bool bSuccess = false;
      bSuccess = FilledObjectImp::setProperty(pProp);
      if (bSuccess == true)
      {
         mNeedsLayout = true;
      }

      return bSuccess;
   }

   for (i = 0; it!=objects.end(); ++it, ++i)
   {
      pCurrentObject = dynamic_cast<GraphicObjectImp*>(*it);
      if (pCurrentObject != NULL)
      {
         switch (i)
         {
            case 0:        //movable rectangle
            case 2:        //white rectangle 1
            case 4:        //white rectangle 2
               if (name != "LineOn")
               {
                  pCurrentObject->setProperty(pProp);
               }
               break;
            case 1:        //black rectangle 1
            case 3:        //black rectangle 2
            case 5:        //black rectangle 3
               if ((name != "LineOn") &&
                  (name != "FillColor") &&
                  (name != "FillStyle"))
               {
                  pCurrentObject->setProperty(pProp);
               }
               break;
            case 6:        //short center line
            case 7:        //left line
            case 8:        //right line
               pCurrentObject->setProperty(pProp);
               break;
            case 9:        //left text
            case 10:       //center text
            case 11:       //right text
               if ((name != "LineOn") &&
                  (name != "FillStyle"))
               {
                  pCurrentObject->setProperty(pProp);
               }
               break;
            default:
               break;
         }//end switch
      }//end if
   }//end for

   if ((name == "TextString") || (name == "Font"))
   {
      mNeedsLayout = true;
   }

   return true;
}

GraphicProperty* ScaleBarObjectImp::getProperty(const string& name) const
{
   GraphicProperty* pProperty = NULL;
   if ((name == "Rotation") || (name == "BoundingBox") || (name == "UnitSystem"))
   {
      pProperty = FilledObjectImp::getProperty(name);
      if (pProperty != NULL)
      {
         return pProperty;
      }
   }

   list<GraphicObject*> groupObjects = mpGroup->getObjects();
   list<GraphicObject*>::const_iterator iter = groupObjects.begin();

   for (int i = 0; iter != groupObjects.end(); ++iter, ++i)
   {
      GraphicObjectImp* pObject = NULL;
      pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         switch (i)
         {
            case 0:        //movable rectangle
            case 2:        //white rectangle 1
            case 4:        //white rectangle 2
               if (name != "LineOn")
               {
                  pProperty = pObject->getProperty(name);
               }
               break;

            case 1:        //black rectangle 1
            case 3:        //black rectangle 2
            case 5:        //black rectangle 3
               if ((name != "LineOn") &&
                  (name != "FillColor") &&
                  (name != "FillStyle"))
               {
                  pProperty = pObject->getProperty(name);
               }
               break;

            case 6:        //short center line
            case 7:        //left line
            case 8:        //right line
               pProperty = pObject->getProperty(name);
               break;

            case 9:        //left text
            case 10:       //center text
            case 11:       //right text
               if ((name != "LineOn") && (name != "FillStyle") && (name != "TextString"))
               {
                  pProperty = pObject->getProperty(name);
               }
               break;

            default:
               break;
         }

         if (pProperty != NULL)
         {
            return pProperty;
         }
      }
   }

   return NULL;
}

void ScaleBarObjectImp::moveHandle(int handle, LocationType pixel, bool bMaintainAspect)
{
   mpGroup->moveHandle(handle, pixel, bMaintainAspect);
   FilledObjectImp::moveHandle(handle, pixel, bMaintainAspect);

   mNeedsLayout = true;
}

bool ScaleBarObjectImp::hit(LocationType pixelCoord) const
{
   return mpGroup->hit(pixelCoord);
}

bool ScaleBarObjectImp::getExtents(vector<LocationType>& dataCoords) const
{
   return mpGroup->getExtents(dataCoords);
}

bool ScaleBarObjectImp::replicateObject(const GraphicObject* pObject)
{
   const ScaleBarObjectImp* pScaleBar = dynamic_cast<const ScaleBarObjectImp*>(pObject);
   if (pScaleBar == NULL)
   {
      return false;
   }

   bool bSuccess = false;
   bSuccess = GraphicObjectImp::replicateObject(pObject);
   if (bSuccess == false)
   {
      return false;
   }

   const GraphicGroup* pGroup = dynamic_cast<const GraphicGroup*>(pScaleBar->mpGroup.get());
   if (pGroup != NULL)
   {
      bSuccess = mpGroup->replicateObject(pGroup);
      if (bSuccess == true)
      {
         updateHandles();
      }
   }

   return bSuccess;
}

bool ScaleBarObjectImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   bool bSuccess = FilledObjectImp::toXml(pXml);
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

bool ScaleBarObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   bool bSuccess = FilledObjectImp::fromXml(pDocument, version);
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
   }

   return bSuccess;
}

const string& ScaleBarObjectImp::getObjectType() const
{
   static string type("ScaleBarObjectImp");
   return type;
}

bool ScaleBarObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ScaleBarObject"))
   {
      return true;
   }

   return FilledObjectImp::isKindOf(className);
}

const GraphicGroup &ScaleBarObjectImp::getGroup() const
{
   return *(dynamic_cast<const GraphicGroup*>(mpGroup.get()));
}

void ScaleBarObjectImp::updateGeo()
{
   // Do nothing, we're always listening
}

void ScaleBarObjectImp::viewModified(Subject& subject, const string& signal, const boost::any& value)
{
   mNeedsLayout = true;
}

void ScaleBarObjectImp::georeferenceModified(Subject &subject, const std::string &signal, const boost::any &v)
{
   mpGeoreference.reset(NULL);
}

void ScaleBarObjectImp::updateAttachments()
{
   if (mpGeoreference.get() == NULL)
   {
      const RasterElement* pGeoreference = getGeoreferenceElement();
      if (pGeoreference != NULL)
      {
         mpGeoreference.reset(const_cast<RasterElement*>(pGeoreference));
         mXgsd = GeoAlgorithms::getXaxisGSD(pGeoreference);
         mNeedsLayout = true;
      }
   }

   if (mpView.get() == NULL)
   {
      GraphicLayer* pLayer = getLayer();
      if (pLayer != NULL)
      {
         PerspectiveView* pPerspectiveView = dynamic_cast<PerspectiveView*>(pLayer->getView());
         if (pPerspectiveView != NULL)
         {
            mpView.reset(pPerspectiveView);
            mNeedsLayout = true;
         }
      }
   }
}
