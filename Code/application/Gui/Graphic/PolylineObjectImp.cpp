/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <float.h>

#include "XercesIncludes.h"

#include "AppConfig.h"
#include "PolylineObjectImp.h"
#include "GraphicLayer.h"
#include "DrawUtil.h"
#include "glCommon.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "GraphicLayerUndo.h"
#include "MessageLogResource.h"
#include "View.h"

#include <vector>
#include <string>

using namespace std;
XERCES_CPP_NAMESPACE_USE

PolylineObjectImp::PolylineObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                     LocationType pixelCoord) :
   MultipointObjectImp(id, type, pLayer, pixelCoord),
   mPaths(1, 0),
   mUseHitTolerance(true),
   mResetSymbolName(true)
{
   addProperty("LineWidth");
   addProperty("LineColor");
   addProperty("LineStyle");
   addProperty("LineScaled");
}

PolylineObjectImp::~PolylineObjectImp()
{
}

void PolylineObjectImp::drawVector(double zoomFactor) const
{
   if (mResetSymbolName)
   {
      // The symbol name cannot be cleared in the constructor 
      // without calling virtual functions, so clear it on first
      // draw
      const_cast<PolylineObjectImp*>(this)->setSymbolName("");
      mResetSymbolName = false;
   }

   if (mPaths.empty())
   {
      return;
   }

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

   const vector<LocationType>& vertices = getVertices();
   unsigned int max = vertices.size();
   unsigned int min = 0;
   
   for (int i = static_cast<int>(mPaths.size() - 1); i >= 0; --i)
   {
      min = mPaths[i];
      glBegin(GL_LINE_STRIP);

      for (unsigned int j = min; j < max; ++j)
      {
         glVertex2d(vertices.at(j).mX, vertices.at(j).mY);
      }
      glEnd();
      max = min;
   }

   if (eStyle != SOLID_LINE)
   {
      glDisable(GL_LINE_STIPPLE);
   }

   glDisable(GL_BLEND);
   glDisable(GL_LINE_SMOOTH);
   glLineWidth(1);

   MultipointObjectImp::drawVector(zoomFactor);
}

void PolylineObjectImp::drawPixels(double zoomFactor) const
{
   if (mPaths.empty())
   {
      return;
   }

   ColorType color = getLineColor();

   glColor3ub(color.mRed, color.mGreen, color.mBlue);

   const vector<LocationType>& vertices = getVertices();
   unsigned int ignorePos = 1;
   for (unsigned int i = 1; i < vertices.size(); ++i)
   {
      if (ignorePos < mPaths.size() && mPaths[ignorePos] == i)
      {
         ++ignorePos;
         continue;
      }
      DrawUtil::PixelDrawer drawer(getPixelSymbol());
      DrawUtil::drawPixelLine(vertices[i-1], vertices[i], drawer);
   }
}

bool PolylineObjectImp::hit(LocationType pixelCoord) const
{
   bool bHit = false;

   LocationType llCorner(getLlCorner());
   LocationType urCorner(getUrCorner());

   double dTolerance = 0.5 + DBL_EPSILON;
   if (mUseHitTolerance == true)
   {
      GraphicLayer* pLayer = NULL;
      pLayer = getLayer();
      if (pLayer != NULL)
      {
         View* pView = pLayer->getView();
         if (pView != NULL)
         {
            dTolerance = 2.0 / pView->getPixelSize(llCorner, urCorner);
         }
         dTolerance /= min(pLayer->getXScaleFactor(), pLayer->getYScaleFactor());
      }
   }

   const vector<LocationType>& vectices = getVertices();
   int numLines = vectices.size();
   unsigned int ignorePos = 1;
   for (unsigned int i = 1; i < vectices.size() && bHit == false; ++i)
   {
      if (ignorePos < mPaths.size() && mPaths[ignorePos] == i)
      {
         ++ignorePos;
         continue;
      }
      bHit = DrawUtil::lineHit(vectices.at(i-1), vectices.at(i), pixelCoord, dTolerance);
   }
   return bHit;
}

unsigned int PolylineObjectImp::getNumSegments() const
{
   const vector<LocationType>& vectices = getVertices();
   return vectices.size() == 0 ? 0 : vectices.size() - 1;
}

const BitMask* PolylineObjectImp::getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow)
{
   double dAngle = getRotation();
   if (dAngle != 0.0)
   {
      mUseHitTolerance = false;
      const BitMask* pMask = GraphicObjectImp::getPixels(iStartColumn, iStartRow, iEndColumn, iEndRow);
      mUseHitTolerance = true;

      return pMask;
   }

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

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

   int iObjectStartRow = llCorner.mY;
   int iObjectEndRow = urCorner.mY;
   int iObjectStartColumn = llCorner.mX;
   int iObjectEndColumn = urCorner.mX;

   mPixelMask.clear();
   mPixelMask.setPixel(iObjectStartColumn, iObjectStartRow, true);
   mPixelMask.setPixel(iObjectEndColumn, iObjectEndRow, true);
   mPixelMask.setPixel(iObjectStartColumn, iObjectStartRow, false);
   mPixelMask.setPixel(iObjectEndColumn, iObjectEndRow, false);

   DrawUtil::BitMaskPixelDrawer drawer(&mPixelMask);
   const vector<LocationType>& vertices = getVertices();
   unsigned int ignorePos = 1;
   for (unsigned int i = 1; i < vertices.size(); ++i)
   {
      if (ignorePos < mPaths.size() && mPaths[ignorePos] == i)
      {
         ++ignorePos;
         continue;
      }
      DrawUtil::drawPixelLine(vertices[i-1], vertices[i], drawer);
   }

   if ((iStartRow != iObjectStartRow) || (iStartColumn != iObjectStartColumn) ||
      (iEndRow != iObjectEndRow) || (iEndColumn != iObjectEndColumn))
   {
      mPixelMask.clipBoundingBox(iStartColumn, iStartRow, iEndColumn, iEndRow);
   }

   return &mPixelMask;
}

bool PolylineObjectImp::replicateObject(const GraphicObject *pObject)
{
   const PolylineObjectImp* pPoly = dynamic_cast<const PolylineObjectImp*>(pObject);
   if (pPoly != NULL)
   {
      bool bSuccess = MultipointObjectImp::replicateObject(pObject);
      mPaths = pPoly->mPaths;
      return bSuccess;
   }

   return false;
}

bool PolylineObjectImp::newPath()
{
   const unsigned int path = getVertices().size();
   if (addPath(path) == false)
   {
      return false;
   }

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new NewPath(this, path));
      }
   }

   return true;
}

bool PolylineObjectImp::addPath(unsigned int path)
{
   if (mPaths.empty() == false)
   {
      if (path <= mPaths.back())
      {
         // Do not allow the same value twice in the vector; force the values to be sorted
         return false;
      }
   }

   if (path > getVertices().size())
   {
      return false;
   }

   mPaths.push_back(path);
   return true;
}

bool PolylineObjectImp::removePath(unsigned int path)
{
   if (path != mPaths.back())
   {
      return false;
   }

   mPaths.pop_back();
   return true;
}

bool PolylineObjectImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   bool bSuccess = MultipointObjectImp::toXml(pXml);
   if (bSuccess == true)
   {
      if (mPaths.size() > 0)
      {
         DOMElement* pAllVertElement = pXml->addElement("paths");
         if (pAllVertElement != NULL)
         {
            pXml->pushAddPoint(pAllVertElement);

            stringstream buf;
            for (unsigned int i = 0; i < mPaths.size(); ++i)
            {
               buf.str("");

               DOMElement* pPixel = pXml->addElement("pixel", pAllVertElement);
               if (pPixel != NULL)
               {
                  buf << mPaths[i];
                  pXml->addText(buf.str().c_str(), pPixel);
                  pAllVertElement->appendChild(pPixel);
               }
            }

            pXml->popAddPoint();
         }
      }
   }

   return bSuccess;
}

bool PolylineObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   mPaths.clear();

   bool bSuccess = MultipointObjectImp::fromXml(pDocument, version);
   if (bSuccess == true)
   {
      DOMNode* pObjectNode = pDocument->getFirstChild();
      while (pObjectNode != NULL)
      {
         if (XMLString::equals(pObjectNode->getNodeName(), X("paths")))
         {
            for (DOMNode *path = pObjectNode->getFirstChild();
               path != NULL;
               path = path->getNextSibling())
            {
               string name(A(path->getNodeName()));
               if (XMLString::equals(path->getNodeName(), X("pixel")))
               {
                  DOMNode* pValue = path->getFirstChild();
                  unsigned int pixel = atoi(A(pValue->getNodeValue()));
                  if (mPaths.empty() || mPaths.back() < pixel)
                  {
                     mPaths.push_back(pixel);
                  }
                  else
                  {
                     MessageResource msg("Error", "app", "D4DF1061-ACD1-405c-A887-84CAED14B387");
                     msg->addProperty("Message", "The Annotation paths are not stored in increasing order.");
                     bSuccess = false;
                  }
               }
            }
         }

         pObjectNode = pObjectNode->getNextSibling();
      }
   }

   if (mPaths.empty())
   {
      mPaths.push_back(0);
   }

   return bSuccess;
}

const string& PolylineObjectImp::getObjectType() const
{
   static string type("PolylineObjectImp");
   return type;
}

bool PolylineObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PolylineObject"))
   {
      return true;
   }

   return MultipointObjectImp::isKindOf(className);
}

bool PolylineObjectImp::processMousePress(LocationType screenCoord, Qt::MouseButton button,
                                          Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if (button == Qt::LeftButton)
   {
      screenCoord = getLayer()->correctCoordinate(screenCoord);
      if (getVertices().empty())
      {
         addVertex(screenCoord);
         addVertex(screenCoord);
      }
      else
      {
         addVertex(screenCoord);
      }

      return true;
   }

   return false;
}

bool PolylineObjectImp::processMouseMove(LocationType screenCoord, Qt::MouseButton button,
                                         Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   screenCoord = getLayer()->correctCoordinate(screenCoord);
   moveHandle(getNumSegments()+8, screenCoord);
   return true;
}

bool PolylineObjectImp::processMouseDoubleClick(LocationType screenCoord, Qt::MouseButton button,
                                                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if (button == Qt::LeftButton)
   {
      // Remove the last vertex that was added from the first click of the double-click
      removeVertex(getVertices().size() - 1);

      // Complete the insertion
      GraphicLayerImp* pLayerImp = dynamic_cast<GraphicLayerImp*>(getLayer());
      if (pLayerImp != NULL)
      {
         pLayerImp->completeInsertion();
      }

      return true;
   }

   return false;
}
