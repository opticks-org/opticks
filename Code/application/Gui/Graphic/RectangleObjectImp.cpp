/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include "glCommon.h"
#include "AppConfig.h"
#include "RectangleObjectImp.h"
#include "GraphicLayer.h"
#include "DrawUtil.h"
#include "View.h"

#include <string>
using namespace std;

/////////////////////
// RectangleObject //
/////////////////////

RectangleObjectImp::RectangleObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                       LocationType pixelCoord) :
   PixelObjectImp(id, type, pLayer, pixelCoord)
{
   addProperty("FillStyle");
   addProperty("HatchStyle");
   addProperty("FillColor");
   addProperty("LineOn");
   addProperty("LineWidth");
   addProperty("LineColor");
   addProperty("LineScaled");
   addProperty("LineStyle");
   addProperty("PixelSymbol");
}

void RectangleObjectImp::drawVector(double zoomFactor) const
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   double lineWidth = getLineWidth();
   if (getLineScaled())
   {
      lineWidth *= zoomFactor;
   }
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
      glVertex2d(llCorner.mX, llCorner.mY);
      glVertex2d(urCorner.mX, llCorner.mY);
      glVertex2d(urCorner.mX, urCorner.mY);
      glVertex2d(llCorner.mX, urCorner.mY);
      glEnd();

      if (pPattern != NULL)
      {
         glDisable(GL_POLYGON_STIPPLE);
      }
   }

   if (bLined == true)
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
      glVertex2d(llCorner.mX, llCorner.mY);
      glVertex2d(urCorner.mX, llCorner.mY);
      glVertex2d(urCorner.mX, urCorner.mY);
      glVertex2d(llCorner.mX, urCorner.mY);
      glEnd();

      if (eLineStyle != SOLID_LINE)
      {
         glDisable(GL_LINE_STIPPLE);
      }

      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
   }
}

void RectangleObjectImp::drawPixels(double zoomFactor) const
{
   LocationType ll = getLlCorner();
   LocationType ur = getUrCorner();

   int minX = floor(min(ll.mX, ur.mX));
   int maxX = floor(max(ll.mX, ur.mX));
   int minY = floor(min(ll.mY, ur.mY));
   int maxY = floor(max(ll.mY, ur.mY));

   vector<LocationType> vertices(4);
   vertices[0] = LocationType(minX, minY);
   vertices[1] = LocationType(minX, maxY+1);
   vertices[2] = LocationType(maxX+1, maxY+1);
   vertices[3] = LocationType(maxX+1, minY);

   vector<unsigned int> paths(1, 0);

   ColorType color = getFillColor();
   glColor3ub(color.mRed, color.mGreen, color.mBlue);
   DrawUtil::PixelDrawer drawer(getPixelSymbol());
   DrawUtil::drawPixelPolygon(vertices, paths, minX, minY, maxX, maxY, drawer);
}

const BitMask *RectangleObjectImp::getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow)
{
   if (mBitMaskDirty)
   {
      mPixelMask.clear();

      LocationType ll = getLlCorner();
      LocationType ur = getUrCorner();

      int minX = min(ll.mX, ur.mX);
      int maxX = max(ll.mX, ur.mX);
      int minY = min(ll.mY, ur.mY);
      int maxY = max(ll.mY, ur.mY);

      vector<LocationType> vertices(4);
      vertices[0] = LocationType(minX, minY);
      vertices[1] = LocationType(minX, maxY+1);
      vertices[2] = LocationType(maxX+1, maxY+1);
      vertices[3] = LocationType(maxX+1, minY);

      vector<unsigned int> paths(1, 0);

      mPixelMask.setPixel(iStartColumn, iStartRow, true);
      mPixelMask.setPixel(iEndColumn, iEndRow, true);
      mPixelMask.setPixel(iStartColumn, iStartRow, false);
      mPixelMask.setPixel(iEndColumn, iEndRow, false);

      DrawUtil::BitMaskPixelDrawer drawer(&mPixelMask);
      DrawUtil::drawPixelPolygon(vertices, paths, iStartColumn, iStartRow, iEndColumn, iEndRow, drawer);

      mBitMaskDirty = false;
   }
   return &mPixelMask;
}

bool RectangleObjectImp::hit(LocationType pixelCoord) const
{
   LocationType corners[4];
   corners[0] = getLlCorner();
   corners[2] = getUrCorner();

   bool bHit = false;

   // Filled area
   FillStyle eFillStyle = getFillStyle();
   if (eFillStyle != EMPTY_FILL)
   {
      double xMin = DrawUtil::minimum(corners[0].mX, corners[2].mX);
      double xMax = DrawUtil::maximum(corners[0].mX, corners[2].mX);
      double yMin = DrawUtil::minimum(corners[0].mY, corners[2].mY);
      double yMax = DrawUtil::maximum(corners[0].mY, corners[2].mY);

      if ((pixelCoord.mX > xMin) && (pixelCoord.mY > yMin) &&
         (pixelCoord.mX < xMax) && (pixelCoord.mY < yMax))
      {
         bHit = true;
      }
   }

   // Line
   if (bHit == false && eFillStyle == EMPTY_FILL)
   {
      bool bLine = false;
      bLine = getLineState();
      if (bLine == true)
      {
         double size = 1.0;

         GraphicLayer* pLayer = NULL;
         pLayer = getLayer();
         if (pLayer != NULL)
         {
            View* pView = pLayer->getView();
            if (pView != NULL)
            {
               size = pView->getPixelSize(corners[0], corners[2]);
            }
            size *= min(pLayer->getXScaleFactor(), pLayer->getYScaleFactor());
         }

         corners[1].mX = corners[2].mX;
         corners[1].mY = corners[0].mY;
         corners[3].mX = corners[0].mX;
         corners[3].mY = corners[2].mY;

         for (int i = 0; i < 4; i++)
         {
            bHit = DrawUtil::lineHit(corners[i], corners[(i + 1) % 4], pixelCoord, 2.0/size);
            if (bHit == true)
            {
               break;
            }
         }
      }
   }

   return bHit;
}

const string& RectangleObjectImp::getObjectType() const
{
   static string type("RectangleObjectImp");
   return type;
}

bool RectangleObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RectangleObject"))
   {
      return true;
   }

   return PixelObjectImp::isKindOf(className);
}

////////////////////////////
// RoundedRectangleObject //
////////////////////////////

RoundedRectangleObjectImp::RoundedRectangleObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                                     LocationType pixelCoord) :
   RectangleObjectImp(id, type, pLayer, pixelCoord)
{
}

void RoundedRectangleObjectImp::draw(double zoomFactor) const
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   double lineWidth = getLineWidth();
   if (getLineScaled())
   {
      lineWidth *= zoomFactor;
   }
   ColorType lineColor = getLineColor();
   FillStyle eFillStyle = getFillStyle();
   SymbolType eHatch = getHatchStyle();
   bool bLined = getLineState();
   ColorType fillColor = getFillColor();
   LineStyle eLineStyle = getLineStyle();

   if (llCorner.mX > urCorner.mX)
   {
      double dTemp = llCorner.mX;
      llCorner.mX = urCorner.mX;
      urCorner.mX = dTemp;
   }

   if (llCorner.mY > urCorner.mY)
   {
      double dTemp = llCorner.mY;
      llCorner.mY = urCorner.mY;
      urCorner.mY = dTemp;
   }

   int i;
   LocationType pixelSize = getPixelSize();
   double radiusX = DrawUtil::minimum(fabs((urCorner.mX - llCorner.mX) * pixelSize.mX),
      fabs((urCorner.mY - llCorner.mY) * pixelSize.mY)) * 0.15 / pixelSize.mX;
   double radiusY = DrawUtil::minimum(fabs((urCorner.mX - llCorner.mX) * pixelSize.mX),
      fabs((urCorner.mY - llCorner.mY) * pixelSize.mY)) * 0.15 / pixelSize.mY;

   DrawUtil::initializeCircle();

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

      glVertex2d(llCorner.mX + radiusX, llCorner.mY);
      glVertex2d(urCorner.mX - radiusX, llCorner.mY);

      for (i = 3 * DrawUtil::VERTEX_COUNT / 4; i < DrawUtil::VERTEX_COUNT; i += 2)
      {
         glVertex2d(urCorner.mX - radiusX + DrawUtil::sCirclePoints[i].mX * radiusX, 
            llCorner.mY + radiusY + DrawUtil::sCirclePoints[i].mY * radiusY);
      }

      glVertex2d(urCorner.mX, urCorner.mY - radiusY);

      for (i = 0; i < DrawUtil::VERTEX_COUNT / 4; i += 2)
      {
         glVertex2d(urCorner.mX - radiusX + DrawUtil::sCirclePoints[i].mX * radiusX, 
            urCorner.mY - radiusY + DrawUtil::sCirclePoints[i].mY * radiusY);
      }

      glVertex2d(llCorner.mX + radiusX, urCorner.mY);

      for (i = DrawUtil::VERTEX_COUNT / 4; i < DrawUtil::VERTEX_COUNT / 2; i += 2)
      {
         glVertex2d(llCorner.mX + radiusX + DrawUtil::sCirclePoints[i].mX * radiusX, 
            urCorner.mY - radiusY + DrawUtil::sCirclePoints[i].mY * radiusY);
      }

      glVertex2d(llCorner.mX, llCorner.mY + radiusY);

      for (i = DrawUtil::VERTEX_COUNT / 2; i < 3 * DrawUtil::VERTEX_COUNT / 4; i += 2)
      {
         glVertex2d(llCorner.mX + radiusX + DrawUtil::sCirclePoints[i].mX * radiusX, 
            llCorner.mY + radiusY + DrawUtil::sCirclePoints[i].mY * radiusY);
      }

      glEnd();

      if (pPattern != NULL)
      {
         glDisable(GL_POLYGON_STIPPLE);
      }
   }

   if (bLined == true)
   {
      glColor3ub(lineColor.mRed, lineColor.mGreen, lineColor.mBlue);
      glLineWidth(lineWidth);

      if (eLineStyle != SOLID_LINE)
      {
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

      glVertex2d(llCorner.mX + radiusX, llCorner.mY);
      glVertex2d(urCorner.mX - radiusX, llCorner.mY);

      for (i = 3 * DrawUtil::VERTEX_COUNT / 4; i < DrawUtil::VERTEX_COUNT; i += 2)
      {
         glVertex2d(urCorner.mX - radiusX + DrawUtil::sCirclePoints[i].mX * radiusX, 
            llCorner.mY + radiusY + DrawUtil::sCirclePoints[i].mY * radiusY);
      }

      glVertex2d(urCorner.mX, urCorner.mY - radiusY);

      for (i = 0; i < DrawUtil::VERTEX_COUNT / 4; i += 2)
      {
         glVertex2d(urCorner.mX - radiusX + DrawUtil::sCirclePoints[i].mX * radiusX, 
            urCorner.mY - radiusY + DrawUtil::sCirclePoints[i].mY * radiusY);
      }

      glVertex2d(llCorner.mX + radiusX, urCorner.mY);

      for (i = DrawUtil::VERTEX_COUNT / 4; i < DrawUtil::VERTEX_COUNT / 2; i += 2)
      {
         glVertex2d(llCorner.mX + radiusX + DrawUtil::sCirclePoints[i].mX * radiusX, 
            urCorner.mY - radiusY + DrawUtil::sCirclePoints[i].mY * radiusY);
      }

      glVertex2d(llCorner.mX, llCorner.mY + radiusY);

      for (i = DrawUtil::VERTEX_COUNT / 2; i < 3 * DrawUtil::VERTEX_COUNT / 4; i += 2)
      {
         glVertex2d(llCorner.mX + radiusX + DrawUtil::sCirclePoints[i].mX * radiusX, 
            llCorner.mY + radiusY + DrawUtil::sCirclePoints[i].mY * radiusY);
      }

      glEnd();

      if (eLineStyle != SOLID_LINE)
      {
         glDisable(GL_LINE_STIPPLE);
         glDisable(GL_LINE_SMOOTH);
      }
   }
}

const string& RoundedRectangleObjectImp::getObjectType() const
{
   static string type("RoundedRectangleObjectImp");
   return type;
}

bool RoundedRectangleObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RoundedRectangleObject"))
   {
      return true;
   }

   return RectangleObjectImp::isKindOf(className);
}
