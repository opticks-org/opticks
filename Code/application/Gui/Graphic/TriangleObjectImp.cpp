/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "glCommon.h"
#include "AppConfig.h"
#include "TriangleObjectImp.h"
#include "GraphicLayer.h"
#include "DrawUtil.h"
#include "View.h"

#include <string>
using namespace std;

TriangleObjectImp::TriangleObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                     LocationType pixelCoord) :
   FilledObjectImp(id, type, pLayer, pixelCoord)
{
   addProperty("Apex");
   mHandles.push_back(pixelCoord);
}

bool TriangleObjectImp::setProperty(const GraphicProperty* pProperty)
{
   bool bSuccess = FilledObjectImp::setProperty(pProperty);
   if ((bSuccess == true) && (pProperty != NULL))
   {
      if (pProperty->getName() == "Apex")
      {
         updateHandles();
      }
   }

   return bSuccess;
}

void TriangleObjectImp::draw(double zoomFactor) const
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   double lineWidth = getLineWidth();
   double apex = getApex();
   ColorType lineColor = getLineColor();
   FillStyle eFillStyle = getFillStyle();
   SymbolType eHatch = getHatchStyle();
   bool bLined = getLineState();
   ColorType fillColor = getFillColor();
   LineStyle eLineStyle = getLineStyle();

   if ((eFillStyle == SOLID_FILL) || (eFillStyle == HATCH))
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

      glColor3ub(fillColor.mRed, fillColor.mGreen, fillColor.mBlue);
      glBegin(GL_POLYGON);
      glVertex2f(llCorner.mX, llCorner.mY);
      glVertex2f(urCorner.mX, llCorner.mY);
      glVertex2f(llCorner.mX + apex * (urCorner.mX - llCorner.mX), urCorner.mY);
      glEnd();

      if (pPattern != NULL)
      {
         glDisable(GL_POLYGON_STIPPLE);
      }
   }

   if (bLined)
   {
      glColor3ub(lineColor.mRed, lineColor.mGreen, lineColor.mBlue);
      glLineWidth(lineWidth);

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

      glBegin(GL_LINE_LOOP);
      glVertex2f(llCorner.mX, llCorner.mY);
      glVertex2f(urCorner.mX, llCorner.mY);
      glVertex2f(llCorner.mX + apex * (urCorner.mX - llCorner.mX), urCorner.mY);
      glEnd();

      if (eLineStyle != SOLID_LINE)
      {
         glDisable(GL_LINE_STIPPLE);
      }

      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
   }
}

void TriangleObjectImp::moveHandle(int handle, LocationType point, bool bMaintainAspect)
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   if (handle != 8)
   {
      FilledObjectImp::moveHandle(handle, point, bMaintainAspect);

      mHandles[8].mX = getApex() * (urCorner.mX - llCorner.mX) + llCorner.mX;
      mHandles[8].mY = urCorner.mY;
   }
   else
   {
      if (point.mX < llCorner.mX)
      {
         point.mX = llCorner.mX;
      }

      if (point.mX > urCorner.mX)
      {
         point.mX = urCorner.mX;
      }

      mHandles[8].mX = point.mX;
      setApex((point.mX - llCorner.mX) / (urCorner.mX - llCorner.mX));
   }
}

void TriangleObjectImp::updateHandles()
{
   FilledObjectImp::updateHandles();

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   double dApex = getApex();

   LocationType apexPoint;
   apexPoint.mX = dApex * (urCorner.mX - llCorner.mX) + llCorner.mX;
   apexPoint.mY = urCorner.mY;

   mHandles.push_back(apexPoint);
}

bool TriangleObjectImp::hit(LocationType pixelCoord) const
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   double apex = getApex();

   LocationType corners[3];
   corners[0] = llCorner;
   corners[1] = LocationType(urCorner.mX, llCorner.mY);
   corners[2] = LocationType(llCorner.mX + apex * (urCorner.mX - llCorner.mX), urCorner.mY);

   bool bHit = false;

   // Filled area
   FillStyle eFillStyle = getFillStyle();
   if (eFillStyle != EMPTY_FILL)
   {
      double xCoords[4];
      xCoords[0] = corners[0].mX;
      xCoords[1] = corners[1].mX;
      xCoords[2] = corners[2].mX;
      xCoords[3] = corners[0].mX;

      double yCoords[4];
      yCoords[0] = corners[0].mY;
      yCoords[1] = corners[1].mY;
      yCoords[2] = corners[2].mY;
      yCoords[3] = corners[0].mY;

      bHit = DrawUtil::isWithin(pixelCoord.mX, pixelCoord.mY, xCoords, yCoords, 4);
   }

   // Line
   if (bHit == false)
   {
      bool bLine = getLineState();
      if (bLine == true)
      {
         double size = 1.0;

         GraphicLayer* pLayer = getLayer();
         if (pLayer != NULL)
         {
            View* pView = pLayer->getView();
            if (pView != NULL)
            {
               size = pView->getPixelSize(llCorner, urCorner);
            }
            size *= min(pLayer->getXScaleFactor(), pLayer->getYScaleFactor());
         }

         for (int i = 0; i < 3; i++)
         {
            bHit = DrawUtil::lineHit(corners[i], corners[(i + 1) % 3], pixelCoord, 2.0/size);
            if (bHit == true)
            {
               break;
            }
         }
      }
   }

   return bHit;
}

const string& TriangleObjectImp::getObjectType() const
{
   static string type("TriangleObjectImp");
   return type;
}

bool TriangleObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "TriangleObject"))
   {
      return true;
   }

   return FilledObjectImp::isKindOf(className);
}
