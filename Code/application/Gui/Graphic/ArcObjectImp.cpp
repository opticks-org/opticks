/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include "ArcObjectImp.h"
#include "DrawUtil.h"
#include "glCommon.h"
#include "GraphicLayer.h"
#include "GraphicProperty.h"
#include "StringUtilities.h"
#include "View.h"

#include <map>
#include <string>
using namespace std;

XERCES_CPP_NAMESPACE_USE

ArcObjectImp::ArcObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
   FilledObjectImp(id, type, pLayer, pixelCoord),
   mpEllipse(ELLIPSE_OBJECT, pLayer, pixelCoord),
   mbNeedsLayout(true)
{
   addProperty("Wedge");
   addProperty("ArcRegion");
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);

   // Set a non-zero bounding box so that the ellipse can be properly resized
   // when the object is not added via the mouse (e.g. paste or undo/redo)
   if (pLayer != NULL)
   {
      pixelCoord = pLayer->correctCoordinate(pixelCoord);
   }

   if (mpEllipse.get() != NULL)
   {
      LocationType urCorner(pixelCoord.mX + HANDLE_DELTA_MINIMUM, pixelCoord.mY + HANDLE_DELTA_MINIMUM);
      mpEllipse->setBoundingBox(pixelCoord, urCorner);
      updateBoundingBox();
   }
}

void ArcObjectImp::draw(double zoomFactor) const
{
   const int step = 2;

   if (mbNeedsLayout == true)
   {
      const_cast<ArcObjectImp*>(this)->updateLayout();
   }

   double lineWidth = getLineWidth();
   ColorType lineColor = getLineColor();
   FillStyle eFillStyle = getFillStyle();
   SymbolType eHatch = getHatchStyle();
   bool bLined = getLineState();
   ColorType fillColor = getFillColor();
   LineStyle eLineStyle = getLineStyle();
   ArcRegion eArcRegion = getArcRegion();

   int lineStart;
   int lineStop;
   int gonStart;
   int gonStop;
   int i;

   int iStart = static_cast<int>(getStartAngle());
   if (iStart < 0)
   {
      iStart--;
   }

   int iStop = static_cast<int>(getStopAngle());
   if (iStop < 0)
   {
      iStop--;
   }

   lineStart = iStart;
   lineStop = iStop;
   gonStart = iStart;
   gonStop = iStop;

   if (lineStart == lineStop) // full ellipse
   {
      // start/stop for the border
      lineStart = 0;
      lineStop = 360;

      // start/stop for the fill
      gonStart = lineStart;
      gonStop = lineStop - step; // keeps the first and last point from being the same
   }

   LocationType center = getCenter();

   if (((eFillStyle == SOLID_FILL) || (eFillStyle == HATCH)) && (eArcRegion != ARC_OPEN))
   {
      unsigned char* pPattern = NULL;
      if (eFillStyle == HATCH)
      {
         pPattern = DrawUtil::getHatchPattern(eHatch);
         if (pPattern != NULL)
         {
            glEnable(GL_POLYGON_STIPPLE);
            glPolygonStipple(pPattern);
         }
      }

      glColor3ub (fillColor.mRed, fillColor.mGreen, fillColor.mBlue);
      glBegin (GL_POLYGON);

      if (eArcRegion == ARC_CENTER)
      {
         glVertex2f (center.mX, center.mY);
      }

      for (i = gonStart; i <= gonStop; i += step)
      {
         LocationType point = getLocation(static_cast<double>(i));
         glVertex2f(point.mX, point.mY);

         if ((gonStop - i) < step)
         {
            i--;
         }
      }

      glEnd();

      if (pPattern != NULL)
      {
         glDisable(GL_POLYGON_STIPPLE);
      }
   }

   if (bLined)
   {
      glColor3ub (lineColor.mRed, lineColor.mGreen, lineColor.mBlue);
      glLineWidth (lineWidth);

#if defined(WIN_API)
      glEnable(GL_LINE_SMOOTH);
#else
      if (lineWidth == 1.0)
      {
         glEnable(GL_LINE_SMOOTH);
      }
      else
      {
         glDisable(GL_LINE_SMOOTH);
      }
#endif
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      if (eLineStyle != SOLID_LINE)
      {
         glEnable(GL_LINE_STIPPLE);
      }

      if (eLineStyle == DASHED)
      {
         glLineStipple(3, 0x3f3f);
      }
      else if (eLineStyle == DOT)
      {
         glLineStipple(2, 0x1111);
      }
      else if (eLineStyle == DASH_DOT)
      {
         glLineStipple(2, 0x11ff);
      }
      else if (eLineStyle == DASH_DOT_DOT)
      {
         glLineStipple(2, 0x24ff);
      }

      glBegin (GL_LINE_STRIP);
      for (i = lineStart; i <= lineStop; i += step)
      {
         LocationType point = getLocation(static_cast<double>(i));
         glVertex2f(point.mX, point.mY);

         if ((lineStop - i) < step)
         {
            i--;
         }
      }

      if ((eArcRegion == ARC_CENTER) || (eArcRegion == ARC_CHORD))
      {
         if (eArcRegion == ARC_CENTER)
         {
            glVertex2f(center.mX, center.mY);
         }

         LocationType point = getLocation(static_cast<double>(lineStart));
         glVertex2f(point.mX, point.mY);
      }

      glEnd();

      if (eLineStyle != SOLID_LINE)
      {
         glDisable(GL_LINE_STIPPLE);
      }

      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
   }
}

bool ArcObjectImp::setProperty(const GraphicProperty* pProperty)
{
   if (pProperty == NULL)
   {
      return false;
   }

   const BoundingBoxProperty* pBBoxProp = dynamic_cast<const BoundingBoxProperty*>(pProperty);
   if (pBBoxProp != NULL && pBBoxProp->hasPixelCoords())
   {
      LocationType ll = pBBoxProp->getLlCorner();
      LocationType ur = pBBoxProp->getUrCorner();

      bool newBBox = false;
      if (fabs(ll.mX - ur.mX) < HANDLE_DELTA_THRESHOLD)
      {
         if (ll.mX < ur.mX)
         {
            ur.mX = ll.mX + HANDLE_DELTA_MINIMUM;
         }
         else
         {
            ll.mX = ur.mX + HANDLE_DELTA_MINIMUM;
         }
         newBBox = true;
      }
      if (fabs(ll.mY - ur.mY) < HANDLE_DELTA_THRESHOLD)
      {
         if (ll.mY < ur.mY)
         {
            ur.mY = ll.mY + HANDLE_DELTA_MINIMUM;
         }
         else
         {
            ll.mY = ur.mY + HANDLE_DELTA_MINIMUM;
         }
         newBBox = true;
      }
      
      if (newBBox)
      {
         return setBoundingBox(ll, ur);
      }

      mbNeedsLayout = true;
   }

   bool bSuccess = FilledObjectImp::setProperty(pProperty);
   if (bSuccess == true)
   {
      if (pProperty->getName() == "Wedge")
      {
         updateBoundingBox();
      }
   }

   return bSuccess;
}

GraphicProperty* ArcObjectImp::getProperty(const std::string& name) const
{
   GraphicProperty* pProperty = NULL;
   pProperty = FilledObjectImp::getProperty(name);
   if (pProperty == NULL)
   {
      // Provide backward compatibility with the obsolete FillRegion property
      if (name == "FillRegion")
      {
         pProperty = getProperty("ArcRegion");
      }
   }

   return pProperty;
}

void ArcObjectImp::moveHandle(int handle, LocationType point, bool bMaintainAspect)
{
   if ((handle == 8) || (handle == 9))
   {
      // Calculate the new angle
      LocationType center = getCenter();
      double dXRadius = getXRadius();
      double dYRadius = getYRadius();
      double dRatio = dXRadius / dYRadius;

      double dAngle = 0;
      dAngle = atan2((point.mY - center.mY) * dRatio, point.mX - center.mX) * 180.0 / PI;
      dAngle = normalizeAngle(dAngle);

      // Update the angle in the wedge property
      if (handle == 8)
      {
         setStartAngle(dAngle);
      }
      else if (handle == 9)
      {
         setStopAngle(dAngle);
      }
   }
   else
   {
      FilledObjectImp::moveHandle(handle, point, bMaintainAspect);
   }
}

void ArcObjectImp::updateHandles()
{
   // Clear the handles list and add the regular handles
   FilledObjectImp::updateHandles();

   // Add the special handles
   double dStartAngle = getStartAngle();
   double dStopAngle = getStopAngle();

   LocationType startPoint = getLocation(dStartAngle);
   LocationType stopPoint = getLocation(dStopAngle);

   mHandles.push_back(startPoint);
   mHandles.push_back(stopPoint);
}

bool ArcObjectImp::hit(LocationType pixelCoord) const
{
   int iStart = static_cast<int>(getStartAngle());
   if (iStart < 0)
   {
      iStart--;
   }

   int iStop = static_cast<int>(getStopAngle());
   if (iStop < 0)
   {
      iStop--;
   }

   if (iStart == iStop)
   {
      iStart = 0;
      iStop = 360;
   }

   ArcRegion eArcRegion = getArcRegion();

   DrawUtil::initializeCircle();
   LocationType center = getCenter();

   bool bHit = false;

   // Filled area
   FillStyle eFillStyle = getFillStyle();
   if (eFillStyle != EMPTY_FILL)
   {
      int iSize = abs(iStop - iStart) + 1;
      if (eArcRegion == ARC_CENTER)
      {
         iSize += 2;
      }
      else if (eArcRegion == ARC_CHORD)
      {
         iSize += 1;
      }

      double* pXVertices = new double[iSize];
      double* pYVertices = new double[iSize];

      int i = 0;
      int iIndex = 0;
      for (i = iStart, iIndex = 0; i <= iStop; i++, iIndex++)
      {
         LocationType arcPoint = getLocation(static_cast<double>(i));
         pXVertices[iIndex] = arcPoint.mX;
         pYVertices[iIndex] = arcPoint.mY;
      }

      if ((eArcRegion == ARC_CENTER) || (eArcRegion == ARC_CHORD))
      {
         if (eArcRegion == ARC_CENTER)
         {
            pXVertices[iSize - 2] = center.mX;
            pYVertices[iSize - 2] = center.mY;
         }

         pXVertices[iSize - 1] = pXVertices[0];
         pYVertices[iSize - 1] = pYVertices[0];
      }

      bHit = DrawUtil::isWithin(pixelCoord.mX, pixelCoord.mY, pXVertices, pYVertices, iSize);

      delete [] pXVertices;
      delete [] pYVertices;
   }

   // Line
   if (bHit == false)
   {
      bool bLine = false;
      bLine = getLineState();
      if (bLine == true)
      {
         // Check the arc line
         double size = 1.0;

         GraphicLayer* pLayer = NULL;
         pLayer = getLayer();
         if (pLayer != NULL)
         {
            View* pView = pLayer->getView();
            if (pView != NULL)
            {
               size = pView->getPixelSize(center, center);
            }

            size *= min(pLayer->getXScaleFactor(), pLayer->getYScaleFactor());
         }

         LocationType basePoint;
         for (int i = iStart; i <= iStop; i++)
         {
            LocationType currentPoint = getLocation(static_cast<double>(i));
            if (i != iStart)
            {
               bHit = DrawUtil::lineHit(currentPoint, basePoint, pixelCoord, 2.0/size);
               if (bHit == true)
               {
                  break;
               }
            }

            basePoint.mX = currentPoint.mX;
            basePoint.mY = currentPoint.mY;
         }

         // Check the arc region lines
         if (bHit == false)
         {
            LocationType startPoint = getLocation(static_cast<double>(iStart));
            LocationType stopPoint = getLocation(static_cast<double>(iStop));

            if (eArcRegion == ARC_CENTER)
            {
               bHit = DrawUtil::lineHit(startPoint, center, pixelCoord, 2.0/size);
               if (bHit == false)
               {
                  bHit = DrawUtil::lineHit(stopPoint, center, pixelCoord, 2.0/size);
               }
            }
            else if (eArcRegion == ARC_CHORD)
            {
               bHit = DrawUtil::lineHit(startPoint, stopPoint, pixelCoord, 2.0/size);
            }
         }
      }
   }

   return bHit;
}

LocationType ArcObjectImp::getLocation(double dAngle) const
{
   dAngle = normalizeAngle(dAngle);
   if ((dAngle < 0.0) || (dAngle >= 360.0))
   {
      return LocationType();
   }

   DrawUtil::initializeCircle();

   LocationType center = getCenter();
   double dXRadius = getXRadius();
   double dYRadius = getYRadius();

   LocationType location;
   location.mX = center.mX + dXRadius * DrawUtil::sCirclePoints[static_cast<int>(dAngle)].mX;
   location.mY = center.mY + dYRadius * DrawUtil::sCirclePoints[static_cast<int>(dAngle)].mY;

   return location;
}

double ArcObjectImp::getAngle(LocationType point) const
{
   DrawUtil::initializeCircle();

   LocationType center = getCenter();
   double dXRadius = getXRadius();
   double dYRadius = getYRadius();

   float xVertices[DrawUtil::VERTEX_COUNT];
   float yVertices[DrawUtil::VERTEX_COUNT];
   map<int, double> indexDistances;

   for (int i = 0; i < DrawUtil::VERTEX_COUNT; i++)
   {
      LocationType ellipsePoint;
      ellipsePoint.mX = center.mX + dXRadius * DrawUtil::sCirclePoints[i].mX;
      ellipsePoint.mY = center.mY + dYRadius * DrawUtil::sCirclePoints[i].mY;

      xVertices[i] = ellipsePoint.mX;
      yVertices[i] = ellipsePoint.mY;

      double dDistance = 1e38;
      if (ellipsePoint.mY == center.mY)
      {
         dDistance = fabs(point.mY - center.mY);
      }
      else if (ellipsePoint.mX == center.mX)
      {
         dDistance = fabs(point.mX - center.mX);
      }
      else
      {
         double m1 = (ellipsePoint.mY - center.mY) / (ellipsePoint.mX - center.mX);
         double b1 = center.mY - m1 * center.mX;
         double m2 = -1.0 / m1;
         double b2 = point.mY - m2 * point.mX;

         LocationType intersection;
         intersection.mX = (b2 - b1) / (m1 - m2);
         intersection.mY = m1 * intersection.mX + b1;

         double dX = intersection.mX - point.mX;
         double dY = intersection.mY - point.mY;
         dDistance = sqrt((dX * dX) + (dY * dY));
      }

      if (indexDistances.size() < 4)
      {
         indexDistances[i] = dDistance;
      }
      else
      {
         bool bInsert = false;
         int iOldIndex = -1;

         map<int, double>::iterator iter;
         for (iter = indexDistances.begin(); iter != indexDistances.end(); ++iter)
         {
            double dCurrentDistance = iter->second;
            if (dDistance < dCurrentDistance)
            {
               bInsert = true;
               iOldIndex = iter->first;
            }
         }

         if (bInsert == true)
         {
            indexDistances.erase(iOldIndex);
            indexDistances[i] = dDistance;
         }
      }
   }

   double dOldDistance = 1e38;
   int iIndex = -1;

   for (map<int, double>::iterator iter = indexDistances.begin(); iter != indexDistances.end(); ++iter)
   {
      int iCurrentIndex = iter->first;
      LocationType point1(xVertices[iCurrentIndex], yVertices[iCurrentIndex]);

      double dDistance = sqrt(pow(point1.mX - point.mX, 2) + pow(point1.mY - point.mY, 2));
      if (dDistance < dOldDistance)
      {
         dOldDistance = dDistance;
         iIndex = iCurrentIndex;
      }
   }

   return iIndex;
}

LocationType ArcObjectImp::getCenter() const
{
   LocationType llCorner = mpEllipse->getLlCorner();
   LocationType urCorner = mpEllipse->getUrCorner();

   LocationType center;
   center.mX = (llCorner.mX + urCorner.mX) / 2;
   center.mY = (llCorner.mY + urCorner.mY) / 2;

   return center;
}

double ArcObjectImp::getXRadius() const
{
   LocationType llCorner = mpEllipse->getLlCorner();
   LocationType urCorner = mpEllipse->getUrCorner();

   double dXRadius = -1.0;
   dXRadius = fabs(urCorner.mX - llCorner.mX) / 2;

   return dXRadius;
}

double ArcObjectImp::getYRadius() const
{
   LocationType llCorner = mpEllipse->getLlCorner();
   LocationType urCorner = mpEllipse->getUrCorner();

   double dYRadius = -1.0;
   dYRadius = fabs(urCorner.mY - llCorner.mY) / 2;

   return dYRadius;
}

bool ArcObjectImp::setEllipseCorners(LocationType llCorner, LocationType urCorner)
{
   bool bSuccess = false;
   if (mpEllipse.get() != NULL)
   {
      bSuccess = mpEllipse->setBoundingBox(llCorner, urCorner);
   }

   return bSuccess;
}

LocationType ArcObjectImp::getEllipseLlCorner() const
{
   LocationType llCorner;
   if (mpEllipse.get() != NULL)
   {
      llCorner = mpEllipse->getLlCorner();
   }

   return llCorner;
}

LocationType ArcObjectImp::getEllipseUrCorner() const
{
   LocationType urCorner;
   if (mpEllipse.get() != NULL)
   {
      urCorner = mpEllipse->getUrCorner();
   }

   return urCorner;
}

bool ArcObjectImp::replicateObject(const GraphicObject* pObject)
{
   if ((pObject == NULL) || (mpEllipse.get() == NULL))
   {
      return false;
   }

   bool bSuccess = false;

   // Update the ellipse properties before the arc properties so that the ellipse bounding box
   // will be correct when calculating the arc bounding box after setting the start and stop angles
   const GraphicObjectImp* pEllipse = dynamic_cast<const ArcObjectImp*>(pObject)->mpEllipse.get();
   if (pEllipse != NULL)
   {
      bSuccess = mpEllipse->replicateObject(dynamic_cast<const GraphicObject*>(pEllipse));
   }

   // Update the arc properties
   if (bSuccess == true)
   {
      bSuccess = FilledObjectImp::replicateObject(pObject);
      if (bSuccess == true)
      {
         updateHandles();
         mbNeedsLayout = true;
      }
   }

   return bSuccess;
}

bool ArcObjectImp::toXml(XMLWriter* pXml) const
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

      bSuccess = mpEllipse->toXml(pXml);

      pXml->popAddPoint();
      pXml->popAddPoint();
   }

   return bSuccess;
}

bool ArcObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   bool bSuccess = false;

   // Load the ellipse properties first to ensure that it has a valid bounding box when loading the arc properties
   DOMNode* pObjectNode = NULL;
   for (pObjectNode = pDocument->getFirstChild(); pObjectNode != NULL; pObjectNode = pObjectNode->getNextSibling())
   {
      if (XMLString::equals(pObjectNode->getNodeName(), X("objects")))
      {
         DOMNode* pEllipseNode = NULL;
         for (pEllipseNode = pObjectNode->getFirstChild();
              pEllipseNode != NULL;
              pEllipseNode = pEllipseNode->getNextSibling())
         {
            if (XMLString::equals(pEllipseNode->getNodeName(), X("Graphic")))
            {
               DOMElement* pElement(static_cast<DOMElement*> (pEllipseNode));
               string type(A(pElement->getAttribute(X("type"))));

               GraphicObjectType objectType = StringUtilities::fromXmlString<GraphicObjectType>(type);
               if (objectType == ELLIPSE_OBJECT)
               {
                  bSuccess = mpEllipse->fromXml(pEllipseNode, version);
                  break;
               }
            }
         }
      }
   }

   if (bSuccess == true)
   {
      bSuccess = FilledObjectImp::fromXml(pDocument, version);
   }

   return bSuccess;
}

const string& ArcObjectImp::getObjectType() const
{
   static string type("ArcObjectImp");
   return type;
}

bool ArcObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ArcObject"))
   {
      return true;
   }

   return FilledObjectImp::isKindOf(className);
}

void ArcObjectImp::updateBoundingBox()
{
   int iStart = static_cast<int>(getStartAngle());
   if (iStart < 0)
   {
      iStart--;
   }

   int iStop = static_cast<int>(getStopAngle());
   if (iStop < 0)
   {
      iStop--;
   }

   if (iStart == iStop)
   {
      iStart = 0;
      iStop = 360;
   }

   LocationType llCorner(1e38, 1e38);
   LocationType urCorner(-1e38, -1e38);

   for (int i = iStart; i <= iStop; i++)
   {
      LocationType currentPoint = getLocation(static_cast<double>(i));
      if (currentPoint.mX < llCorner.mX)
      {
         llCorner.mX = currentPoint.mX;
      }

      if (currentPoint.mY < llCorner.mY)
      {
         llCorner.mY = currentPoint.mY;
      }

      if (currentPoint.mX > urCorner.mX)
      {
         urCorner.mX = currentPoint.mX;
      }

      if (currentPoint.mY > urCorner.mY)
      {
         urCorner.mY = currentPoint.mY;
      }
   }

   LocationType center = getCenter();
   if (center.mX < llCorner.mX)
   {
      llCorner.mX = center.mX;
   }

   if (center.mY < llCorner.mY)
   {
      llCorner.mY = center.mY;
   }

   if (center.mX > urCorner.mX)
   {
      urCorner.mX = center.mX;
   }

   if (center.mY > urCorner.mY)
   {
      urCorner.mY = center.mY;
   }

   LocationType oldLlCorner = getLlCorner();
   LocationType oldUrCorner = getUrCorner();

   if (oldLlCorner.mX > oldUrCorner.mX)
   {
      double dTemp = llCorner.mX;
      llCorner.mX = urCorner.mX;
      urCorner.mX = dTemp;
   }

   if (oldLlCorner.mY > oldUrCorner.mY)
   {
      double dTemp = llCorner.mY;
      llCorner.mY = urCorner.mY;
      urCorner.mY = dTemp;
   }

   setBoundingBox(llCorner, urCorner);
   updateHandles();

   mbNeedsLayout = false;
   mLlCorner = llCorner;
   mUrCorner = urCorner;
}

void ArcObjectImp::updateLayout()
{
   if (mbNeedsLayout == false)
   {
      return;
   }

   // Compute the new ellipse width, height, and center
   LocationType oldLlEllipse = mpEllipse->getLlCorner();
   LocationType oldUrEllipse = mpEllipse->getUrCorner();
   LocationType oldEllipseCenter = getCenter();

   double dOldArcWidth = mUrCorner.mX - mLlCorner.mX;
   double dOldArcHeight = mUrCorner.mY - mLlCorner.mY;
   double dOldEllipseWidth = oldUrEllipse.mX - oldLlEllipse.mX;
   double dOldEllipseHeight = oldUrEllipse.mY - oldLlEllipse.mY;

   LocationType llArc = getLlCorner();
   LocationType urArc = getUrCorner();

   double dNewArcWidth = urArc.mX - llArc.mX;
   double dNewArcHeight = urArc.mY - llArc.mY;
   double dNewEllipseWidth = dOldEllipseWidth * (dNewArcWidth / dOldArcWidth);
   double dNewEllipseHeight = dOldEllipseHeight * (dNewArcHeight / dOldArcHeight);

   LocationType newEllipseCenter;
   newEllipseCenter.mX = ((oldEllipseCenter.mX - mLlCorner.mX) * (dNewEllipseWidth / dOldEllipseWidth)) + llArc.mX;
   newEllipseCenter.mY = ((oldEllipseCenter.mY - mLlCorner.mY) * (dNewEllipseHeight / dOldEllipseHeight)) + llArc.mY;

   // Update the ellipse bounding box
   LocationType newEllipseLl;
   newEllipseLl.mX = newEllipseCenter.mX - (dNewEllipseWidth / 2);
   newEllipseLl.mY = newEllipseCenter.mY - (dNewEllipseHeight / 2);

   LocationType newEllipseUr;
   newEllipseUr.mX = newEllipseCenter.mX + (dNewEllipseWidth / 2);
   newEllipseUr.mY = newEllipseCenter.mY + (dNewEllipseHeight / 2);

   mpEllipse->setBoundingBox(newEllipseLl, newEllipseUr);

   // Move the special handles
   double dStartAngle = getStartAngle();
   double dStopAngle = getStopAngle();

   LocationType startPoint = getLocation(dStartAngle);
   LocationType stopPoint = getLocation(dStopAngle);

   mHandles[8] = startPoint;
   mHandles[9] = stopPoint;

   // Update the member variables
   mbNeedsLayout = false;
   mLlCorner = llArc;
   mUrCorner = urArc;
}

double ArcObjectImp::normalizeAngle(double dAngle) const
{
   while (dAngle < 0.0)
   {
      dAngle += 360.0;
   }

   while (dAngle >= 360.0)
   {
      dAngle -= 360.0;
   }

   if ((dAngle < 0.0) || (dAngle >= 360.0))
   {
      return -1.0;
   }

   int iQuadrant = 1;
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   if (llCorner.mX > urCorner.mX)   // Quadrant 2 or 3
   {
      iQuadrant = 2;
   }

   if (llCorner.mY > urCorner.mY)   // Quadrant 3 or 4
   {
      iQuadrant = 3;

      if (llCorner.mX < urCorner.mX)   // Quadrant 4
      {
         iQuadrant = 4;
      }
   }

   if (iQuadrant == 2)
   {
      dAngle = 180.0 - dAngle;
   }
   else if (iQuadrant == 3)
   {
      dAngle += 180.0;
   }
   else if (iQuadrant == 4)
   {
      dAngle = 360.0 - dAngle;
   }

   while (dAngle < 0.0)
   {
      dAngle += 360.0;
   }

   while (dAngle >= 360.0)
   {
      dAngle -= 360.0;
   }

   if ((dAngle < 0.0) || (dAngle >= 360.0))
   {
      return -1.0;
   }

   return dAngle;
}
