/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DrawUtil.h"
#include "glCommon.h"
#include "TrailObjectImp.h"

#include <math.h>
#include <limits>

using namespace std;

TrailObjectImp::TrailObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                               LocationType pixelCoord) :
   RectangleObjectImp(id, type, pLayer, pixelCoord),
   mpBuffer(NULL),
   mRows(0),
   mColumns(0)
{
}

TrailObjectImp::~TrailObjectImp()
{
   setStencilBufferSize(0, 0);
}

void TrailObjectImp::setStencilBufferSize(int rows, int columns)
{
   if ((rows == mRows) && (columns == mColumns))
   {
      return;
   }

   if (mpBuffer != NULL)
   {
      delete [] mpBuffer;
      mpBuffer = NULL;
   }

   mRows = rows;
   mColumns = columns;

   int bufferSize = mRows * mColumns;
   if (bufferSize > 0)
   {
      mpBuffer = new (nothrow) char[bufferSize];
      if (mpBuffer != NULL)
      {
         memset(mpBuffer, 1, bufferSize);
      }
      else
      {
         mRows = 0;
         mColumns = 0;
      }
   }
}

void TrailObjectImp::addToStencil(LocationType lowerLeft, LocationType lowerRight,
                                  LocationType upperLeft, LocationType upperRight)
{
   if (mpBuffer == NULL)
   {
      return;
   }

   // find min rect to contain selection box
   vector<double> xVals;
   vector<double> yVals;
   xVals.push_back(lowerLeft.mX);
   xVals.push_back(lowerRight.mX);
   xVals.push_back(upperRight.mX); 
   xVals.push_back(upperLeft.mX);
   yVals.push_back(lowerLeft.mY);
   yVals.push_back(lowerRight.mY);
   yVals.push_back(upperRight.mY);
   yVals.push_back(upperLeft.mY);
   double minX = *(min_element(xVals.begin(), xVals.end()));
   double minY = *(min_element(yVals.begin(), yVals.end()));
   double maxX = *(max_element(xVals.begin(), xVals.end()));
   double maxY = *(max_element(yVals.begin(), yVals.end()));
   
   const int numVertices = 5;
   LocationType vertices[numVertices];
   double smallIncrement = numeric_limits<double>::epsilon();
   vertices[0] = lowerLeft;
   vertices[0].mX -= smallIncrement;  // expand polygon by fraction of pixel around border so pixels on edge
   vertices[0].mY -= smallIncrement;  // or on vertices of selection box are "within" polygon
   vertices[1] = lowerRight;
   vertices[1].mX += smallIncrement;
   vertices[1].mY -= smallIncrement;
   vertices[2] = upperRight;
   vertices[2].mX += smallIncrement;
   vertices[2].mY += smallIncrement;
   vertices[3] = upperLeft;
   vertices[3].mX -= smallIncrement;
   vertices[3].mY += smallIncrement;
   vertices[4] = vertices[0];  // close polygon

   // clamp min/max to overview window
   int minXi = static_cast<int>(max(0.0, minX));
   int minYi = static_cast<int>(max(0.0, minY));
   int maxXi = static_cast<int>(min(static_cast<double>(mColumns-1), maxX));
   int maxYi = static_cast<int>(min(static_cast<double>(mRows-1), maxY));

   // Calculate the number of rows and columns used
   int numCols = maxXi - minXi + 1;
   int numRows = maxYi - minYi + 1;
   int offset = minYi * mColumns + minXi;

   // Write 0 to all locations within the selection box area
   char* pTmp = mpBuffer + offset;
   int hitOffset;
   int hitColumns;
   for (int row = minYi; row < minYi+numRows; ++row)
   {
      hitOffset = 0;
      while (!DrawUtil::isWithin(LocationType(hitOffset+minXi, row), vertices, numVertices)
         && hitOffset < numCols)
      {
         ++hitOffset;
      }
      if (hitOffset < numCols)
      {
         hitColumns = 0;
         while (DrawUtil::isWithin(LocationType(hitOffset+hitColumns+minXi, row), vertices, numVertices)
            && (hitOffset + hitColumns) < numCols )
         {
            ++hitColumns;
         }
         if ((hitOffset + hitColumns) <= numCols)
         {
            memset(pTmp+hitOffset, 0, hitColumns);
         }
      }
      pTmp += mColumns;
   }
}

void TrailObjectImp::clearStencil()
{
   if ((mpBuffer != NULL) && (mRows > 0) && (mColumns > 0))
   {
      memset(mpBuffer, 1, mRows * mColumns);
   }
}

void TrailObjectImp::drawVector(double zoomFactor) const
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   ColorType fillColor = getFillColor();

   const_cast<TrailObjectImp*>(this)->updateStencil();

   glEnable(GL_STENCIL_TEST);
   glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
   glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glColor4ub(fillColor.mRed, fillColor.mGreen, fillColor.mBlue, fillColor.mAlpha);
   glBegin(GL_POLYGON);
   glVertex2d(llCorner.mX, llCorner.mY);
   glVertex2d(urCorner.mX, llCorner.mY);
   glVertex2d(urCorner.mX, urCorner.mY);
   glVertex2d(llCorner.mX, urCorner.mY);
   glEnd();

   glDisable(GL_STENCIL_TEST);
   glDisable(GL_BLEND);
}

const string& TrailObjectImp::getObjectType() const
{
   static string type("TrailObjectImp");
   return type;
}

bool TrailObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "TrailObject"))
   {
      return true;
   }

   return RectangleObjectImp::isKindOf(className);
}

void TrailObjectImp::updateStencil()
{
   if (mpBuffer == NULL)
   {
      return;
   }

   // Setup screen matrices
   int viewPort[4];
   int unpackAlignment = 0;
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackAlignment);

   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(viewPort[0], viewPort[0] + viewPort[2], viewPort[1], viewPort[1] + viewPort[3]);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   // Set the raster position to the screen origin
   glRasterPos2i(0, 0);

   // Update the stencil buffer
   glClearStencil(1);
   glClear(GL_STENCIL_BUFFER_BIT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glDrawPixels(static_cast<GLsizei>(mColumns), static_cast<GLsizei>(mRows), GL_STENCIL_INDEX,
      GL_UNSIGNED_BYTE, mpBuffer);

   // Restore the original matrices
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
}
