/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <float.h>

#include "glCommon.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "LineObjectImp.h"
#include "GraphicLayer.h"
#include "DrawUtil.h"
#include "View.h"

#include <string>
using namespace std;

LineObjectImp::LineObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                             LocationType pixelCoord) :
   PixelObjectImp(id, type, pLayer, pixelCoord),
   mToleranceFactor(2.0),
   mUseHitTolerance(true)
{
   addProperty("LineWidth");
   addProperty("LineColor");
   addProperty("LineScaled");
   addProperty("LineStyle");
   addProperty("PixelSymbol");

   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
}

void LineObjectImp::drawVector(double zoomFactor) const
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   double lineWidth = getLineWidth();
   if (getLineScaled())
   {
      lineWidth *= zoomFactor;
   }
   ColorType color = getLineColor();
   LineStyle eStyle = getLineStyle();

   glColor3ub(color.mRed, color.mGreen, color.mBlue);
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

   if (eStyle != SOLID_LINE)
   {
      glEnable(GL_LINE_STIPPLE);
   }

   if (eStyle == DASHED)
   {
      glLineStipple(3, 0x3f3f);
   }
   else if (eStyle == DOT)
   {
      glLineStipple(2, 0x1111);
   }
   else if (eStyle == DASH_DOT)
   {
      glLineStipple(2, 0x11ff);
   }
   else if (eStyle == DASH_DOT_DOT)
   {
      glLineStipple(2, 0x24ff);
   }

   glBegin(GL_LINES);
   glVertex2d(llCorner.mX, llCorner.mY);
   glVertex2d(urCorner.mX, urCorner.mY);
   glEnd();

   if (eStyle != SOLID_LINE)
   {
      glDisable(GL_LINE_STIPPLE);
   }

   glDisable(GL_BLEND);
   glDisable(GL_LINE_SMOOTH);
   glLineWidth(1);
}

void LineObjectImp::drawPixels(double zoomFactor) const
{
   ColorType color = getLineColor();
   glColor3ub(color.mRed, color.mGreen, color.mBlue);

   DrawUtil::PixelDrawer drawer(getPixelSymbol());
   DrawUtil::drawPixelLine(getLlCorner(), getUrCorner(), drawer);
}

bool LineObjectImp::hit(LocationType pixelCoord) const
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   double dTolerance = 0.5 + DBL_EPSILON;
   if (getUseHitTolerance())
   {
      GraphicLayer* pLayer = NULL;
      pLayer = getLayer();
      if (pLayer != NULL)
      {
         View* pView = pLayer->getView();
         if (pView != NULL)
         {
            dTolerance = mToleranceFactor / pView->getPixelSize(llCorner, urCorner);
         }
         dTolerance /= min(pLayer->getXScaleFactor(), pLayer->getYScaleFactor());
      }
   }

   return DrawUtil::lineHit(llCorner, urCorner, pixelCoord, dTolerance);
}

bool LineObjectImp::getExtents(vector<LocationType>& dataCoords) const
{
   LocationType lowerLeft = getLlCorner();
   LocationType upperRight = getUrCorner();

   dataCoords.clear();
   dataCoords.push_back(lowerLeft);
   dataCoords.push_back(upperRight);

   return true;
}

const BitMask* LineObjectImp::getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow)
{
   if (getRotation() != 0)
   {
      return GraphicObjectImp::getPixels(iStartColumn, iStartRow, iEndColumn, iEndRow);
   }
   mPixelMask.clear();
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   int iObjectStartRow = min(llCorner.mY, urCorner.mY);
   int iObjectEndRow = max(llCorner.mY, urCorner.mY);
   int iObjectStartColumn = min(llCorner.mX, urCorner.mX);
   int iObjectEndColumn = max(llCorner.mX, urCorner.mX);

   mPixelMask.clear();
   mPixelMask.setPixel(iObjectStartColumn, iObjectStartRow, true);
   mPixelMask.setPixel(iObjectEndColumn, iObjectEndRow, true);
   mPixelMask.setPixel(iObjectStartColumn, iObjectStartRow, false);
   mPixelMask.setPixel(iObjectEndColumn, iObjectEndRow, false);

   DrawUtil::BitMaskPixelDrawer drawer(&mPixelMask);
   DrawUtil::drawPixelLine(llCorner, urCorner, drawer);

   if ((iStartRow != iObjectStartRow) || (iStartColumn != iObjectStartColumn) ||
      (iEndRow != iObjectEndRow) || (iEndColumn != iObjectEndColumn))
   {
      mPixelMask.clipBoundingBox(iStartColumn, iStartRow, iEndColumn, iEndRow);
   }

   return &mPixelMask;
}

bool LineObjectImp::setProperty(const GraphicProperty* pProp)
{
   const BoundingBoxProperty* pBbOld = dynamic_cast<const BoundingBoxProperty*>(pProp);
   if (pBbOld != NULL)
   {
      bool changed = false;
      LocationType ll = pBbOld->getLlCorner();
      LocationType ur = pBbOld->getUrCorner();
      GraphicObjectType type = getGraphicObjectType();
      if (type == HLINE_OBJECT && ur.mY != ll.mY)
      {
         ur.mY = ll.mY;
         changed = true;
      }
      else if (type == VLINE_OBJECT && ur.mX != ll.mX)
      {
         ur.mX = ll.mX;
         changed = true;
      }
      if (changed)
      {
         BoundingBoxProperty bbNew(ll, ur);
         return GraphicObjectImp::setProperty(&bbNew);
      }
   }

   return GraphicObjectImp::setProperty(pProp);
}

void LineObjectImp::moveHandle(int handle, LocationType pixel, bool bMaintainAspect)
{
   VERIFYNRV(static_cast<unsigned int>(handle) < mHandles.size());
   GraphicObjectType type = getGraphicObjectType();

   LocationType ll = getLlCorner();
   LocationType ur = getUrCorner();

   if (type == HLINE_OBJECT)
   {
      switch (handle)
      {
      case TOP_LEFT: // fall through
      case MIDDLE_LEFT: // fall through
      case BOTTOM_LEFT:
         // extend to the left
         ll.mX = pixel.mX;
         pixel.mY = ll.mY;
         break;
      case TOP_RIGHT: // fall through
      case MIDDLE_RIGHT: // fall through
      case BOTTOM_RIGHT:
         // extend to the right
         ur.mX = pixel.mX;
         pixel.mY = ll.mY;
         break;
      case TOP_CENTER: // fall through
      case BOTTOM_CENTER:
         // move the whole thing
         ll.mY = pixel.mY;
         ur.mY = pixel.mY;
         ll.mX = ll.mX - (mHandles[handle].mX - pixel.mX);
         ur.mX = ur.mX - (mHandles[handle].mX - pixel.mX);
         break;
      default:
         break;
      }
   }
   else if (type == VLINE_OBJECT)
   {
      switch (handle)
      {
      case TOP_LEFT: // fall through
      case TOP_CENTER: // fall through
      case TOP_RIGHT:
         // extend up
         ur.mY = pixel.mY;
         pixel.mX = ur.mX;
         break;
      case BOTTOM_LEFT: // fall through
      case BOTTOM_CENTER: // fall through
      case BOTTOM_RIGHT:
         // extend down
         ll.mY = pixel.mY;
         pixel.mX = ur.mX;
         break;
      case MIDDLE_LEFT: // fall through
      case MIDDLE_RIGHT:
         // move the whole thing
         ll.mX = pixel.mX;
         ur.mX = pixel.mX;
         ll.mY = ll.mY - (mHandles[handle].mY - pixel.mY);
         ur.mY = ur.mY - (mHandles[handle].mY - pixel.mY);
         break;
      default:
         break;
      }
   }
   else
   {
      return GraphicObjectImp::moveHandle(handle, pixel, bMaintainAspect);
   }
   mHandles[handle] = pixel;
   setBoundingBox(ll, ur);
}

const string& LineObjectImp::getObjectType() const
{
   static string sType("LineObjectImp");
   return sType;
}

bool LineObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "LineObject"))
   {
      return true;
   }

   return PixelObjectImp::isKindOf(className);
}

double LineObjectImp::getHitToleranceFactor() const
{
   return mToleranceFactor;
}

bool LineObjectImp::setHitToleranceFactor(double hitFactor)
{
   mToleranceFactor = hitFactor;
   return true;
}

bool LineObjectImp::getUseHitTolerance() const
{
   return mUseHitTolerance;
}

bool LineObjectImp::setUseHitTolerance(bool bUse)
{
   mUseHitTolerance = bUse;
   return true;
}
