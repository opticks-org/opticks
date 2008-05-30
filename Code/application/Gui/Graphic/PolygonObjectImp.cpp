/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "DesktopServicesImp.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "PolygonObjectImp.h"
#include "DrawUtil.h"
#include "boost/shared_array.hpp"

#include <list>
#include <cmath>

#include <limits>
using namespace std;

static std::vector<boost::shared_array<GLdouble> > combinedVertices;

PolygonObjectImp::PolygonObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                   LocationType pixelCoord) :
   PolylineObjectImp(id, type, pLayer, pixelCoord)
{
   addProperty("LineOn");
   addProperty("FillStyle");
   addProperty("HatchStyle");
   addProperty("FillColor");
}

void PolygonObjectImp::drawVector(double zoomFactor) const
{
   combinedVertices.clear(); // clear from previous redraw
   // doesn't work right if done at bottom of method

   unsigned int uiSegments = 0;
   uiSegments = getNumSegments();
   if (uiSegments == 0)
   {
      return;
   }

   bool bFilled = false;
   bFilled = getFillState();
   if (bFilled == true)
   {
      FillStyle eFillStyle = getFillStyle();
      SymbolType eHatch = getHatchStyle();

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

      ColorType fillColor = getFillColor();
      glColor3ub(fillColor.mRed, fillColor.mGreen, fillColor.mBlue);

      GLUtesselator* pTess = NULL;
      pTess = gluNewTess();
      if (pTess == NULL)
      {
         return;
      }

      gluTessCallback(pTess, GLU_TESS_BEGIN, (void (__stdcall *)(void)) glBegin);
      gluTessCallback(pTess, GLU_TESS_VERTEX, (void (__stdcall *)(void)) glVertex3dv);
      gluTessCallback(pTess, GLU_TESS_COMBINE, (void (__stdcall *)(void)) PolygonObjectImp::combineVertexData);
      gluTessCallback(pTess, GLU_TESS_END, glEnd);
      gluTessNormal(pTess, 0.0, 0.0, 1.0);
      gluTessBeginPolygon(pTess, NULL);

      const vector<LocationType> &vertices = getVertices();
      unsigned int max = vertices.size();
      unsigned int min = 0;
      int maxPaths = mPaths.size()-1;
      // gluTess* doesn't like identical vertices
      LocationType lastVertex(numeric_limits<double>::max(), numeric_limits<double>::max());
      for (int i = maxPaths; i >=0 ; --i)
      {
         min = mPaths[i];
         gluTessBeginContour(pTess);
         for (unsigned int j = min; j < max-1; ++j) // do not use final point
         {
            const LocationType &currentVertex = vertices[j];
            if (currentVertex == lastVertex)
            {
               continue;
            }
            boost::shared_array<GLdouble> vertex(new GLdouble[3]);
            vertex[0] = currentVertex.mX;
            vertex[1] = currentVertex.mY;
            vertex[2] = 0.0;
            combinedVertices.push_back(vertex);
           
            gluTessVertex(pTess, vertex.get(), vertex.get());
            lastVertex = currentVertex;
         }

         gluTessEndContour(pTess);
         max = min;
      }
      gluTessEndPolygon(pTess);
      gluDeleteTess(pTess);

      if (pPattern != NULL)
      {
         glDisable(GL_POLYGON_STIPPLE);
      }
   }
   bool bLined = false;
   bLined = getLineState();
   if (bLined == true)
   {
      PolylineObjectImp::drawVector(zoomFactor);
   }
}

void PolygonObjectImp::drawPixels(double zoomFactor) const
{
   ColorType fillColor = getFillColor();
   glColor3ub(fillColor.mRed, fillColor.mGreen, fillColor.mBlue);
  
   const std::vector<LocationType> &vertices = getVertices();

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   int startCol = min(llCorner.mX, urCorner.mX);
   int startRow = min(llCorner.mY, urCorner.mY);
   int endCol = max(llCorner.mX, urCorner.mX);
   int endRow = max(llCorner.mY, urCorner.mY);

   DrawUtil::PixelDrawer drawer(getPixelSymbol());
   DrawUtil::drawPixelPolygon(vertices, mPaths, startCol, startRow, endCol, endRow, drawer);
   PolylineObjectImp::drawPixels(zoomFactor);
}

void PolygonObjectImp::moveHandle(int handle, LocationType pixel, bool bMaintainAspect)
{
   const vector<LocationType> &vertices = getVertices();
   int movedVertex = handle - 8;

   if (movedVertex >= 0)
   {
      bool endPiece = false;
      int otherVertex = 0;
   
      if (movedVertex == vertices.size()-1) // last handle of last path
      {
         endPiece = true;
         otherVertex = mPaths.back();
      }
      else if (movedVertex == mPaths.back()) // first handle of last path
      {
         endPiece = true;
         otherVertex = vertices.size()-1;
      }
      else
      {
         vector<unsigned int>::iterator itr = find(mPaths.begin(), mPaths.end(), movedVertex+1);
         if (itr != mPaths.end()) // last in a path
         {
            endPiece = true;
            otherVertex = *--itr;
         }
         else
         {
            itr = find(mPaths.begin(), mPaths.end(), movedVertex);
            if (itr != mPaths.end()) // first in a path
            {
               endPiece = true;
               otherVertex = (*++itr)-1;
            }
         }

      }

      if (endPiece && abs(movedVertex-otherVertex) > 1 && vertices[movedVertex] == vertices[otherVertex])
      {
         PolylineObjectImp::moveHandle(otherVertex+8, pixel, false);
      }
   }
   PolylineObjectImp::moveHandle(handle, pixel, bMaintainAspect);

}

bool PolygonObjectImp::hit(LocationType pixelCoord) const
{
   bool bHit = false;

   // Filled area
   FillStyle eFillStyle = getFillStyle();
   if (eFillStyle != EMPTY_FILL)
   {
      unsigned int uiSegments = 0;
      uiSegments = getNumSegments();
      if (uiSegments == 0)
      {
         return false;
      }

      const vector<LocationType> &vertices = getVertices();
      bHit = DrawUtil::isWithin(pixelCoord, &vertices[0], uiSegments+1, &mPaths[0], mPaths.size());
   }

   // Line
   if (bHit == false)
   {
      bool bLine = false;
      bLine = getLineState();
      if (bLine == true)
      {
         bHit = PolylineObjectImp::hit(pixelCoord);
      }
   }

   return bHit;
}

void PolygonObjectImp::combineVertexData(GLdouble coords[3], GLdouble* pVertexData[4],
                                      GLfloat weight[4], void** pOutData)
{
   if (pOutData == NULL)
   {
      return;
   }

   boost::shared_array<GLdouble> newVertex(new GLdouble[3]);
   combinedVertices.push_back(newVertex);

   newVertex[0] = coords[0];
   newVertex[1] = coords[1];
   newVertex[2] = coords[2];
   *pOutData = newVertex.get();
}

const BitMask* PolygonObjectImp::getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow)
{
   if (mBitMaskDirty)
   {
      double dAngle = getRotation();
      if (dAngle != 0.0)
      {
         return GraphicObjectImp::getPixels(iStartColumn, iStartRow, iEndColumn, iEndRow);
      }

      // updates mPixelMask
      PolylineObjectImp::getPixels(iStartColumn, iStartRow, iEndColumn, iEndRow);

      mPixelMask.setPixel(iStartColumn, iStartRow, true);
      mPixelMask.setPixel(iEndColumn, iEndRow, true);
      mPixelMask.setPixel(iStartColumn, iStartRow, false);
      mPixelMask.setPixel(iEndColumn, iEndRow, false);

      const vector<LocationType>& vertices = getVertices();
      if (vertices.empty() == true)
      {
         return &mPixelMask;
      }

      DrawUtil::BitMaskPixelDrawer drawer(&mPixelMask);
      DrawUtil::drawPixelPolygon(vertices, mPaths, iStartColumn, iStartRow, iEndColumn, iEndRow, drawer);
      mBitMaskDirty = false;
   }

   return &mPixelMask;
}

bool PolygonObjectImp::newPath()
{
   const vector<LocationType> &vertices = getVertices();
   if (mPaths.empty() == false && vertices.empty() == false)
   {
      unsigned int firstVertex = mPaths.back();
      if (vertices[firstVertex] != vertices.back())
      {
         addVertex(vertices[firstVertex]);
      }    
   }
   return PolylineObjectImp::newPath();
}

bool PolygonObjectImp::processMousePress(LocationType screenCoord, Qt::MouseButton button,
                                         Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   screenCoord = getLayer()->correctCoordinate(screenCoord);
   if (getVertices().empty())
   {
      addVertex(screenCoord);
      addVertex(screenCoord);
   }

   return PolylineObjectImp::processMousePress(screenCoord, button, buttons, modifiers);
}

bool PolygonObjectImp::processMouseMove(LocationType screenCoord, Qt::MouseButton button,
                                        Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if (mHandles.size() > 10)
   {
      screenCoord = getLayer()->correctCoordinate(screenCoord);
      moveHandle(mHandles.size()-2, screenCoord);
      return true;
   }

   return false;
}

bool PolygonObjectImp::processMouseDoubleClick(LocationType screenCoord, Qt::MouseButton button,
                                               Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if (button == Qt::LeftButton)
   {
      // Remove the second to last vertex that was added from the first click of the double-click
      removeVertex(getVertices().size() - 2);

      // Complete the insertion
      GraphicLayerImp *pLayerImp = dynamic_cast<GraphicLayerImp*>(getLayer());
      if (pLayerImp != NULL)
      {
         pLayerImp->completeInsertion();
      }

      return true;
   }

   return false;
}

const string& PolygonObjectImp::getObjectType() const
{
   static string type("PolygonObjectImp");
   return type;
}

bool PolygonObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PolygonObject"))
   {
      return true;
   }

   return PolylineObjectImp::isKindOf(className);
}

bool PolygonObjectImp::addVertex(LocationType endPoint)
{
   LocationType loopStart = endPoint;
   const std::vector<LocationType> &vertices = getVertices();
   if (!mPaths.empty() && mPaths.back() < vertices.size())
   {
      loopStart = vertices[mPaths.back()];
   }

   PolylineObjectImp::addVertex(loopStart);
   if (mHandles.size() > 11)
   {
      moveHandle(mHandles.size()-2, endPoint);
   }

   return true;
}
