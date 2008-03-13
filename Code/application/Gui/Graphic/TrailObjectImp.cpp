/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "glCommon.h"
#include "TrailObjectImp.h"

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

void TrailObjectImp::addToStencil(LocationType lowerLeft, LocationType lowerRight, LocationType upperLeft,
                                  LocationType upperRight)
{
   if (mpBuffer == NULL)
   {
      return;
   }

   // Calculate the number of rows and columns used
   int numCols = 0;
   int numRows = 0;
   int offset = 0;

   if (upperRight.mX >= lowerLeft.mX)
   {
      numCols = static_cast<int>(upperRight.mX) - static_cast<int>(lowerLeft.mX) + 1;
      offset  = static_cast<int>(lowerLeft.mX);
   }
   else
   {
      numCols = static_cast<int>(lowerLeft.mX) - static_cast<int>(upperRight.mX) + 1;
      offset  = static_cast<int>(upperRight.mX);
   }

   if (upperRight.mY >= lowerLeft.mY)
   {
      numRows = static_cast<int>(upperRight.mY) - static_cast<int>(lowerLeft.mY) + 1;
      offset += ((static_cast<int>(lowerLeft.mY)) * mColumns);
   }
   else
   {
      numRows = static_cast<int>(lowerLeft.mY)- static_cast<int>(upperRight.mY) + 1;
      offset += ((static_cast<int>(upperRight.mY)) * mRows);
   }

   // Write 0 to all locations within the rectangle
   char* pTmp = mpBuffer + offset;
   for (int row = 0; row < numRows; ++row)
   {
      memset(pTmp, 0, numCols);
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
   glStencilFunc(GL_NOTEQUAL, 0x1,0x1);
   glStencilOp (GL_KEEP,GL_KEEP,GL_KEEP);

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

   // Set the current GL context to the overview context
   GlContextSave contextSave;

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
