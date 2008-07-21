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
#include "EllipseObjectImp.h"
#include "GraphicLayer.h"
#include "DrawUtil.h"
#include "View.h"

#include <string>
using namespace std;

EllipseObjectImp::EllipseObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                   LocationType pixelCoord) :
   FilledObjectImp(id, type, pLayer, pixelCoord)
{
}

void EllipseObjectImp::draw (double zoomFactor) const
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   double lineWidth = getLineWidth();
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
      DrawUtil::drawEllipse(llCorner, urCorner, true);

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
      if(lineWidth == 1.0)
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

      DrawUtil::drawEllipse(llCorner, urCorner, false);

      if (eLineStyle != SOLID_LINE)
      {
         glDisable(GL_LINE_STIPPLE);
      }

      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
   }
}

bool EllipseObjectImp::hit(LocationType pixelCoord) const
{ 
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   double xVertices[DrawUtil::VERTEX_COUNT+1], yVertices[DrawUtil::VERTEX_COUNT+1];
   DrawUtil::initializeCircle();

   LocationType center, radii;
   center.mX = (llCorner.mX + urCorner.mX) / 2.0;
   center.mY = (llCorner.mY + urCorner.mY) / 2.0;

   radii.mX = fabs (llCorner.mX - urCorner.mX) / 2.0;
   radii.mY = fabs (llCorner.mY - urCorner.mY) / 2.0;

   int i = 0;
   for (i = 0; i < DrawUtil::VERTEX_COUNT; i++)
   {
      xVertices[i] = center.mX + radii.mX * DrawUtil::sCirclePoints[i].mX;
      yVertices[i] = center.mY + radii.mY * DrawUtil::sCirclePoints[i].mY;
   }

   xVertices[DrawUtil::VERTEX_COUNT] = xVertices[0];
   yVertices[DrawUtil::VERTEX_COUNT] = yVertices[0];

   bool bHit = false;

   // Filled area
   FillStyle eFillStyle = getFillStyle();
   if (eFillStyle != EMPTY_FILL)
   {
      bHit = DrawUtil::isWithin(pixelCoord.mX, pixelCoord.mY, xVertices, yVertices, DrawUtil::VERTEX_COUNT+1);
   }

   // Line
   if (bHit == false)
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
               size = pView->getPixelSize(llCorner, urCorner);
            }
            size *= min(pLayer->getXScaleFactor(), pLayer->getYScaleFactor());
         }

         LocationType basePoint;
         for (i = 0; i < DrawUtil::VERTEX_COUNT; i++)
         {
            LocationType currentPoint(xVertices[i], yVertices[i]);

            if (i != 0)
            {
               bHit = DrawUtil::lineHit(currentPoint, basePoint, pixelCoord, 2.0/size);
               if (bHit == true)
               {
                  break;
               }
            }

            basePoint = currentPoint;
         }
      }
   }

   return bHit;
}

const string& EllipseObjectImp::getObjectType() const
{
   static string type("EllipseObjectImp");
   return type;
}

bool EllipseObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "EllipseObject"))
   {
      return true;
   }

   return FilledObjectImp::isKindOf(className);
}
