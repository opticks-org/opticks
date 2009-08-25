/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <cmath>

#include <QtCore/QRect>
#include <QtGui/QBitmap>
#include <QtGui/QImage>
#include <QtGui/QPainter>

#include "BitMask.h"
#include "DrawUtil.h"
#include "glCommon.h"
#include "ColorType.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "Endian.h"
#include "LocationType.h"
#include "MipMappedTextures.h"
#include "TypesFile.h"

#include <algorithm>
#include <limits>
using namespace std;

LocationType DrawUtil::sCirclePoints[VERTEX_COUNT];

void DrawUtil::initializeCircle()
{
   static bool isInitialized = false;
   if (isInitialized == false)
   {
      for (int i = 0; i < VERTEX_COUNT; ++i)
      {
         sCirclePoints[i].mX = cos(2.0 * PI * (double) i / VERTEX_COUNT);
         sCirclePoints[i].mY = sin(2.0 * PI * (double) i / VERTEX_COUNT);
      }
      isInitialized = true;
   }
}

unsigned char* DrawUtil::sHatchPattern = NULL;

unsigned char* DrawUtil::getHatchPattern(SymbolType eSymbol)
{
   if (sHatchPattern == NULL)
   {
      sHatchPattern = new unsigned char[128];
      if (sHatchPattern == NULL)
      {
         return NULL;
      }
   }

   memset(sHatchPattern, 0, 128 * sizeof(unsigned char));

   for (int i = 0; i < 128; ++i)
   {
      int iShift = 0;
      iShift = (i % 32) / 4;

      unsigned char ucSolid = 0xff;
      unsigned char ucHoriz = 0x00;
      if (iShift == 4)
      {
         ucHoriz = 0xff;
      }

      unsigned char ucVert = 0x08;
      unsigned char ucCross = ucHoriz | ucVert;
      unsigned char ucForward = 0x80 >> iShift;
      unsigned char ucBack = 0x01 << iShift;
      unsigned char ucX = ucForward | ucBack;
      unsigned char ucAsterisk = ucCross | ucX;

      switch (eSymbol)
      {
         case SOLID: default:
            sHatchPattern[i] = ucSolid;
            break;

         case X:
            sHatchPattern[i] = ucX;
            break;

         case CROSS_HAIR:
            sHatchPattern[i] = ucCross;
            break;

         case ASTERISK:
            sHatchPattern[i] = ucAsterisk;
            break;

         case HORIZONTAL_LINE:
            sHatchPattern[i] = ucHoriz;
            break;

         case VERTICAL_LINE:
            sHatchPattern[i] = ucVert;
            break;

         case FORWARD_SLASH:
            sHatchPattern[i] = ucForward;
            break;

         case BACK_SLASH:
            sHatchPattern[i] = ucBack;
            break;
      }
   }

   return sHatchPattern;
}

/**
  * Draws a circle centered on the specified point and of the specified
  * radius. The values are all in scene coordinates. The circle is drawn
  * as an unfilled polygon with 120 sides.
  *
  * @param x
  *     The x coordinate of the center of the circle
  *
  * @param y
  *     The y coordinate of the center of the circle
  *
  * @param radius
  *     The radius of the circle.
  */
void DrawUtil::drawEllipse(LocationType corner1, LocationType corner2, bool filled)
{
   initializeCircle();

   LocationType center;
   center.mX = (corner1.mX + corner2.mX) / 2.0;
   center.mY = (corner1.mY + corner2.mY) / 2.0;

   LocationType radii;
   radii.mX = fabs (corner1.mX - corner2.mX) / 2.0;
   radii.mY = fabs (corner1.mY - corner2.mY) / 2.0;

   if (filled)
   {
      glBegin(GL_POLYGON);
   }
   else
   {
      glBegin(GL_LINE_LOOP);
   }

   int step = 2;
   for (int i = 0; i < VERTEX_COUNT; i += step)
   {
      glVertex2f(center.mX + radii.mX * sCirclePoints[i].mX, center.mY+ radii.mY * sCirclePoints[i].mY);
   }

   glEnd();
}

double DrawUtil::linePointDistance(LocationType p1, LocationType p2, LocationType target, LocationType *pIntersection)
{
   LocationType intersection;
   double distance = -1.0;

   if (fabs(p2.mX - p1.mX) <= 1e-13 * maximum(fabs(p2.mX), fabs(p1.mX))) // effectively: if (p2.mX == p1.mX)
   { // vertical line
      distance = fabs(p1.mX - target.mX);
      intersection.mX = p2.mX;
      intersection.mY = target.mY;
   }
   else if (fabs(p2.mY - p1.mY) <= 1e-13 * maximum(fabs(p2.mY), fabs(p1.mY))) // effectively: if (p2.mY == p1.mY)
   { // horizontal line
      distance = fabs(p1.mY - target.mY);
      intersection.mX = target.mX;
      intersection.mY = p2.mY;
   }
   else
   {
      double m1 = (p2.mY - p1.mY) / (p2.mX - p1.mX);
      double b1 = p1.mY - m1 * p1.mX;
      double m2 = -1.0 / m1;
      double b2 = target.mY - m2 * target.mX;

      intersection.mX = (b2 - b1) / (m1 - m2);
      intersection.mY = m1 * intersection.mX + b1;

      double dx = intersection.mX - target.mX;
      double dy = intersection.mY - target.mY;
      distance = sqrt(dx * dx + dy * dy);
   }

   if (pIntersection)
   {
      *pIntersection = intersection;
   }

   return distance;
}

bool DrawUtil::lineHit(LocationType p1, LocationType p2, LocationType target, double qualifier)
{
   LocationType intersection;

   double distance = linePointDistance(p1, p2, target, &intersection);

   double xMin = minimum(p1.mX, p2.mX);
   double xMax = maximum(p1.mX, p2.mX);
   double yMin = minimum(p1.mY, p2.mY);
   double yMax = maximum(p1.mY, p2.mY);

   if (intersection.mX >= xMin && intersection.mY >= yMin)
   {
      if (intersection.mX <= xMax && intersection.mY <= yMax)
      {
         return (distance < qualifier);
      }
   }

   return false;
}

int DrawUtil::isWithin(double x, double y, const double *bx, const double *by, int count)
{
   double slope;
   int above = 0;

   for (int i = 0; i < count - 1; ++i)
   {
      if (isBetween (x, bx[i], bx[i + 1]))
      {
         slope = (by[i + 1] - by[i]) / (bx[i + 1] - bx[i]);
         if ((slope * (x - bx[i])) + by[i] <= y)
         {
            above++;
         }
      }
   }

   return (above & 1);
}

/**
 * Determines whether a point is within a polygon.
 *
 * @param point
 *        Point to test for
 * @param pRegion
 *        Pointer to array of LocationTypes, forming a 
 *        polygon path.
 * @param count
 *        Number of LocationTypes in pRegion
 * @param pIgnoreSegmentsEnd
 *        Pointer to an array of indices into pRegion which
 *        are to be ignored for testing.  This allows
 *        pRegion to describe doughnuts or other non-contiguous paths.
 *        This array must be ordered from low to high.
 * @param ignoreCount
 *        Number of indices in pIgnoreSegmentsEnd
 * @return True if the point is within the described polygon, false otherwise.
 */
bool DrawUtil::isWithin(LocationType point, const LocationType *pRegion, int count,
                        const unsigned int *pIgnoreSegmentsEnd, int ignoreCount)
{
   double slope;
   int ignorePos = 0;
   int above = 0;

   if (pIgnoreSegmentsEnd == NULL)
   {
      ignoreCount = 0;
   }
   else if (pIgnoreSegmentsEnd[0] == 0) // first vertex cannot be a segment end
   {
      ++ignorePos;
   }

   for (int i = 0; i < count - 1; ++i)
   {
      if (ignorePos < ignoreCount && i + 1 == pIgnoreSegmentsEnd[ignorePos])
      {
         ++ignorePos;
         continue;
      }
      if (isBetween(point.mX, pRegion[i].mX, pRegion[i + 1].mX))
      {
         slope = (pRegion[i + 1].mY - pRegion[i].mY) / (pRegion[i + 1].mX - pRegion[i].mX);
         if ((slope * (point.mX - pRegion[i].mX)) + pRegion[i].mY <= point.mY)
         {
            above++;
         }
      }
   }

   return (above & 1);
}

// pixel = V*PD*P*MV*vertex: see OpenGL Programming Guide p.68
static void project(double xCoord, double yCoord, double zCoord, const double modelMatrix[16],
                    const double projectionMatrix[16], const int viewport[4],
                    double *xPixel, double *yPixel, double *zPixel)
{
   double vector1[4];
   double vector2[4];
   int i;
   int j;

   // object coordinates
   vector1[0] = xCoord;
   vector1[1] = yCoord;
   vector1[2] = zCoord;
   vector1[3] = 1.0;

   // to eye coordinates
   for (i = 0; i < 4; ++i)
   {
      vector2[i] = 0.0;
      for (j = 0; j < 4; ++j)
      {
         vector2[i] += vector1[j] * modelMatrix[4 * j + i];
      }
   }

   // to clip coordinates
   for (i = 0; i < 4; ++i)
   {
      vector1[i] = 0.0;
      for (j = 0; j < 4; ++j)
      {
         vector1[i] += vector2[j] * projectionMatrix[4 * j + i];
      }
   }

   // to normalized device coordinates
   for (i = 0; i < 3; ++i)
   {
      vector1[i] /= vector1[3];
   }

   // to window coordinates
   *xPixel = (1.0 + vector1[0]) / 2.0 * static_cast<double>(viewport[2]) + static_cast<double>(viewport[0]);
   *yPixel = (1.0 + vector1[1]) / 2.0 * static_cast<double>(viewport[3]) + static_cast<double>(viewport[1]);
   *zPixel = vector1[2];
}

// vertex = MVI*PI*PDI*VI*pixel: see OpenGL Programming Guide p.68
bool DrawUtil::unProject(double xPixel, double yPixel, double zPixel, const double modelMatrix[16],
                         const double projectionMatrix[16], const int viewport[4],
                         double *xCoord, double *yCoord, double *zCoord)
{
   double vector1[4];
   double vector2[4];
   int i;
   int j;
   double projectionInverse[16];
   double modelInverse[16];

   memset(projectionInverse, 0, 16*sizeof(double));
   memset(modelInverse, 0, 16*sizeof(double));

   // invert projection matrix: see OpenGL Programming Guide p.480
   projectionInverse[0] = 1.0 / projectionMatrix[0];
   projectionInverse[5] = 1.0 / projectionMatrix[5];
   projectionInverse[11] = 1.0 / projectionMatrix[14];
   projectionInverse[12] = projectionMatrix[8] / projectionMatrix[0];
   projectionInverse[13] = projectionMatrix[9] / projectionMatrix[5];
   projectionInverse[14] = -1.0;
   projectionInverse[15] = projectionMatrix[10] / projectionMatrix[14];

   // invert model-view matrix: see OpenGL Programming Guide pp. 478-479
   modelInverse[0] = modelMatrix[0];
   modelInverse[1] = modelMatrix[4];
   modelInverse[2] = modelMatrix[8];
   modelInverse[4] = modelMatrix[1];
   modelInverse[5] = modelMatrix[5];
   modelInverse[6] = modelMatrix[9];
   modelInverse[8] = modelMatrix[2];
   modelInverse[9] = modelMatrix[6];
   modelInverse[10] = modelMatrix[10];
   for (i = 0; i < 3; ++i)
   {
      modelInverse[12 + i] = 0.0;
      for (j = 0; j < 3; ++j)
      {
         modelInverse[12 + i] += modelMatrix[12 + j] * modelMatrix[4 * i + j];
      }
      modelInverse[12 + i] = -modelInverse[12 + i];
   }
   modelInverse[15] = 1.0;

   // to normalized device coords
   vector1[0] = ((xPixel - static_cast<double>(viewport[0])) / static_cast<double>(viewport[2])) * 2.0 - 1.0;
   vector1[1] = ((yPixel - static_cast<double>(viewport[1])) / static_cast<double>(viewport[3])) * 2.0 - 1.0;
   vector1[2] = zPixel;
   vector1[3] = -projectionMatrix[14] / (-zPixel - projectionMatrix[10]);

   // to clip coordinates
   for (i = 0; i < 3; ++i)
   {
      vector1[i] *= vector1[3];
   }

   // to eye coordinates
   for (i = 0; i < 4; ++i)
   {
      vector2[i] = 0.0;
      for (j = 0; j < 4; ++j)
      {
         vector2[i] += vector1[j] * projectionInverse[4 * j + i];
      }
   }

   // to object coordinates
   for (i = 0; i < 4; ++i)
   {
      vector1[i] = 0.0;
      for (j = 0; j < 4; ++j)
      {
         vector1[i] += vector2[j] * modelInverse[4 * j + i];
      }
   }

   *xCoord = vector1[0];
   *yCoord = vector1[1];
   *zCoord = vector1[2];

   return true;
}

bool DrawUtil::unProjectToZero(double xPixel, double yPixel, const double modelMatrix[16],
                               const double projectionMatrix[16], const int viewport[4],
                               double *xCoord, double *yCoord)
{
   LocationType temp1;
   LocationType temp2;
   double winZ1;
   double winZ2;

   bool success = gluUnProject(xPixel, yPixel, 0.0, modelMatrix, projectionMatrix, viewport, &temp1.mX,
      &temp1.mY, &winZ1);
   if (success == true)
   {
      success = gluUnProject(xPixel, yPixel, 1e5, modelMatrix, projectionMatrix, viewport,
         &temp2.mX, &temp2.mY, &winZ2);
   }

   if (success == true)
   {
      if (winZ2 != winZ1)
      {
         double m1 = (temp2.mX - temp1.mX) / (winZ2 - winZ1);
         double m2 = (temp2.mY - temp1.mY) / (winZ2 - winZ1);
         *xCoord = (m1 * (0 - winZ1)) + temp1.mX;
         *yCoord = (m2 * (0 - winZ1)) + temp1.mY;
      }
      else
      {
         success = false;
      }
   }

   return success;
}

void DrawUtil::restrictToViewport(int &ulStartColumn, int &ulStartRow,
                                  int &ulEndColumn, int &ulEndRow)
{
   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
   int viewExtents[4];
   viewExtents[0] = viewPort[0];
   viewExtents[1] = viewPort[1];
   viewExtents[2] = viewPort[0] + viewPort[2] - 1;
   viewExtents[3] = viewPort[1] + viewPort[3] - 1;
   if (glIsEnabled(GL_SCISSOR_TEST))
   {
      int scissorBox[4];
      glGetIntegerv(GL_SCISSOR_BOX, scissorBox);
      int scissorExtents[4];
      scissorExtents[0] = scissorBox[0];
      scissorExtents[1] = scissorBox[1];
      scissorExtents[2] = scissorBox[0]+scissorBox[2]-1;
      scissorExtents[3] = scissorBox[1]+scissorBox[3]-1;
      viewExtents[0] = max(viewExtents[0], scissorExtents[0]);
      viewExtents[1] = max(viewExtents[1], scissorExtents[1]);
      viewExtents[2] = min(viewExtents[2], scissorExtents[2]);
      viewExtents[3] = min(viewExtents[3], scissorExtents[3]);
   }

   LocationType loc;

   int xCoords[4];
   xCoords[0] = viewExtents[0];
   xCoords[1] = viewExtents[0];
   xCoords[2] = viewExtents[2];
   xCoords[3] = viewExtents[2];
   int yCoords[4];
   yCoords[0] = viewExtents[1];
   yCoords[1] = viewExtents[3];
   yCoords[2] = viewExtents[1];
   yCoords[3] = viewExtents[3];

   bool maxXset = false;
   bool maxYset = false;
   bool minXset = false;
   bool minYset = false;
   LocationType maxes(-1e20, -1e20);
   LocationType mins(1e20, 1e20);

   int i;
   bool success = true;
   for (i = 0; i < 4; ++i)
   {
      success = unProjectToZero(xCoords[i], yCoords[i], modelMatrix, projectionMatrix, viewPort, &loc.mX, &loc.mY);
      if (success)
      {
         if (loc.mX < mins.mX)
         {
            mins.mX = loc.mX - 1.0;
            minXset = true;
         }
         if (loc.mY < mins.mY)
         {
            mins.mY = loc.mY - 1.0;
            minYset = true;
         }
         if (loc.mX > maxes.mX)
         {
            maxes.mX = loc.mX + 1.0;
            maxXset = true;
         }
         if (loc.mY > maxes.mY)
         {
            maxes.mY = loc.mY + 1.0;
            maxYset = true;
         }
      }
   }

   if (minXset && ulStartColumn < mins.mX)
   {
      ulStartColumn = min(mins.mX, static_cast<double>(ulEndColumn));
   }
   if (maxXset && ulEndColumn > maxes.mX)
   {
      ulEndColumn = max(maxes.mX, static_cast<double>(ulStartColumn));
   }
   if (minYset && ulStartRow < mins.mY)
   {
      ulStartRow = min(mins.mY, static_cast<double>(ulEndRow));
   }
   if (maxYset && ulEndRow > maxes.mY)
   {
      ulEndRow = max(maxes.mY, static_cast<double>(ulStartRow));
   }
}

inline double magdiff2(LocationType temp1, LocationType temp2)
{
   double tempX = temp1.mX-temp2.mX; 
   double tempY = temp1.mY-temp2.mY;
   return tempX * tempX + tempY * tempY;
}

double DrawUtil::getPixelSize(GLfloat ulStartColumn, GLfloat ulStartRow,
                              GLfloat ulEndColumn, GLfloat ulEndRow)
{
   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   double pixelSize = 0.0;

   LocationType temp1;
   LocationType temp2;
   GLdouble winZ;

   project(ulStartColumn, ulStartRow, 0.0, modelMatrix, projectionMatrix, viewPort, &temp1.mX, &temp1.mY, &winZ);
   gluProject(ulStartColumn, ulStartRow, 0.0, modelMatrix, projectionMatrix, viewPort, &temp1.mX, &temp1.mY, &winZ);
   gluProject(ulStartColumn+1, ulStartRow, 0.0, modelMatrix, projectionMatrix, viewPort, &temp2.mX, &temp2.mY, &winZ);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   gluProject(ulStartColumn, ulStartRow+1, 0.0, modelMatrix, projectionMatrix, viewPort, &temp2.mX, &temp2.mY, &winZ);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   gluProject(ulStartColumn, ulEndRow, 0.0, modelMatrix, projectionMatrix, viewPort, &temp1.mX, &temp1.mY, &winZ);
   gluProject(ulStartColumn+1, ulEndRow, 0.0, modelMatrix, projectionMatrix, viewPort, &temp2.mX, &temp2.mY, &winZ);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));
   gluProject(ulStartColumn, ulEndRow+1, 0.0, modelMatrix, projectionMatrix, viewPort, &temp2.mX, &temp2.mY, &winZ);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   gluProject(ulEndColumn, ulStartRow, 0.0, modelMatrix, projectionMatrix, viewPort, &temp1.mX, &temp1.mY, &winZ);
   gluProject(ulEndColumn+1, ulStartRow, 0.0, modelMatrix, projectionMatrix, viewPort, &temp2.mX, &temp2.mY, &winZ);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));
   gluProject(ulEndColumn, ulStartRow+1, 0.0, modelMatrix, projectionMatrix, viewPort, &temp2.mX, &temp2.mY, &winZ);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   gluProject(ulEndColumn, ulEndRow, 0.0, modelMatrix, projectionMatrix, viewPort, &temp1.mX, &temp1.mY, &winZ);
   gluProject(ulEndColumn+1, ulEndRow, 0.0, modelMatrix, projectionMatrix, viewPort, &temp2.mX, &temp2.mY, &winZ);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));
   gluProject(ulEndColumn, ulEndRow+1, 0.0, modelMatrix, projectionMatrix, viewPort, &temp2.mX, &temp2.mY, &winZ);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   return sqrt(pixelSize);
}

bool DrawUtil::isSmall(GLfloat ulStartColumn, GLfloat ulStartRow,
                       GLfloat ulEndColumn, GLfloat ulEndRow, double dist)
{
   return getPixelSize(ulStartColumn, ulStartRow, ulEndColumn, ulEndRow) < dist;
}

LocationType DrawUtil::getRotatedCoordinate(const LocationType& coord, const LocationType& rotationCenter,
                                            double dRotation)
{
   if (dRotation == 0.0)
   {
      return coord;
   }

   double dTheta = (PI / 180.0) * dRotation;

   double dX = coord.mX - rotationCenter.mX;
   double dY = coord.mY - rotationCenter.mY;

   LocationType newCoord;
   newCoord.mX = (cos(dTheta) * dX) - (sin(dTheta) * dY);
   newCoord.mY = (cos(dTheta) * dY) + (sin(dTheta) * dX);

   newCoord += rotationCenter;
   return newCoord;
}

void DrawUtil::updateBoundingBox(LocationType &llCorner, LocationType &urCorner,
                                 LocationType vertex)
{
   llCorner.mX = MIN(llCorner.mX, vertex.mX);
   llCorner.mY = MIN(llCorner.mY, vertex.mY);
   urCorner.mX = MAX(urCorner.mX, vertex.mX);
   urCorner.mY = MAX(urCorner.mY, vertex.mY);
}

void DrawUtil::drawRotatedText(DrawUtil::TextTexture &tex, QString text, QFont font, 
                                                ColorType textColor, LocationType textLocation, 
                                                double textDirection, bool drawFromTop)
{
   glPushAttrib(GL_COLOR_BUFFER_BIT);

   if (!tex.isValid())
   {
      // generate the texture

      // Get bounding box for text using specified font
      QRect textBoundingBox;           // Bounding box that surrounds the text
      QFontMetrics ftMetrics(font);    // Used to get width and height of text
      int maxTextureSize = 0;          // The max allowable texture size
      double textWidth = 0;            // The width of the text bounding box
      double textHeight = 0;           // The height of the text bounding box
      glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
      textBoundingBox = ftMetrics.boundingRect(0, 0, maxTextureSize, maxTextureSize,
         Qt::AlignLeft | Qt::TextWordWrap, text);
      textWidth = textBoundingBox.width();
      textHeight = textBoundingBox.height();

      // Get corner locations of area to draw on screen
      double pixelSize = 0;            // The number of screen pixels represented by a coordinate pixel
      double locationWidth = 0;        // The width of the area on the screen that will be drawn
      double locationHeight = 0;       // The height of the area on the screen that will be drawn
      LocationType lowerStartLocation; // The lower left corner of the text display on the screen
      LocationType lowerEndLocation;   // The lower right corner of the text display on the screen
      LocationType upperStartLocation; // The upper left corner of the text display on the screen
      LocationType upperEndLocation;   // The upper right corner of the text display on the screen
      pixelSize = getPixelSize(textLocation.mX, textLocation.mY, textLocation.mX, textLocation.mY);
      locationWidth = (textWidth/pixelSize);
      locationHeight = (textHeight/pixelSize);
      if (drawFromTop)  // Upper left origin
      {
         upperStartLocation = textLocation;
         upperEndLocation.mX = upperStartLocation.mX + cos(textDirection) * locationWidth;
         upperEndLocation.mY = upperStartLocation.mY + sin(textDirection) * locationWidth;
         getParallelLine(upperStartLocation, upperEndLocation, locationHeight * (-1), 0.0f, 1.0f,
            lowerStartLocation, lowerEndLocation);
      }
      else   // Lower left origin
      {
         lowerStartLocation = textLocation;
         lowerEndLocation.mX = lowerStartLocation.mX + cos(textDirection) * locationWidth;
         lowerEndLocation.mY = lowerStartLocation.mY + sin(textDirection) * locationWidth;
         getParallelLine(lowerStartLocation, lowerEndLocation, locationHeight, 0.0f, 1.0f, upperStartLocation,
            upperEndLocation);
      }

      // Create text image
      QBitmap textBitmap(textWidth, textHeight);
      textBitmap.clear();

      QPainter painter(&textBitmap);
      painter.setFont(font);
      painter.setPen(QPen(Qt::color1));
      painter.drawText(textBoundingBox, Qt::AlignLeft | Qt::TextWordWrap, text);
      painter.end();
      QImage image = textBitmap.toImage();

      // Create GL texture
      int textureWidth = 0;      // The width of the GL texture
      int textureHeight = 0;     // The height of the GL texture
      int bufSize = 0;           // The size of the memory buffer used to hold the GL texture
      GLuint textureId = 0;      // Id of generated texture
      unsigned int* target = NULL;
      textureWidth = pow(2.0, ceil(log10(textWidth) / log10(2.0))) + 0.5;
      textureHeight = pow(2.0, ceil(log10(textHeight) / log10(2.0))) + 0.5;
      bufSize = textureWidth * textureHeight;
      vector<unsigned int> texData(bufSize);
      fill(texData.begin(), texData.end(), 0);
      target = &texData[0];
      for (int j = 0; j < textHeight; ++j)
      {
         target = &texData[textureWidth * j];
         for (int i = 0; i < textWidth; ++i)
         {
            if (image.pixel(i, j) != 0xffffffff)
            {
               *target = 0xffffffff;
            }
            else
            {
               if (Endian::getSystemEndian() == BIG_ENDIAN_ORDER)
               {
                  *target = 0xffffff00;
               }
               else
               {
                  *target = 0x00ffffff;
               }
            }
            target++;
         }
      }

      // Set GL options for text
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_TEXTURE_2D);
      glGenTextures(1, &textureId);
      glBindTexture(GL_TEXTURE_2D, textureId);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texData[0]);
      tex = DrawUtil::TextTexture(textureId, font, textureWidth, textureHeight, textWidth, textHeight);
   }

   // Get corner locations of area to draw on screen
   LocationType lowerStartLocation; // The lower left corner of the text display on the screen
   LocationType lowerEndLocation;   // The lower right corner of the text display on the screen
   LocationType upperStartLocation; // The upper left corner of the text display on the screen
   LocationType upperEndLocation;   // The upper right corner of the text display on the screen
   double locationWidth = 0;        // The width of the text image on the screen
   double locationHeight = 0;       // The height of the text image on the screen
   locationWidth = (tex.getTextWidth()/getPixelSize(textLocation.mX, textLocation.mY, textLocation.mX,
      textLocation.mY));
   locationHeight = (tex.getTextHeight()/getPixelSize(textLocation.mX, textLocation.mY, textLocation.mX,
      textLocation.mY));
   if (drawFromTop)  // Upper left origin
   {
      upperStartLocation = textLocation;
      upperEndLocation.mX = upperStartLocation.mX + cos(textDirection) * locationWidth;
      upperEndLocation.mY = upperStartLocation.mY + sin(textDirection) * locationWidth;
      getParallelLine(upperStartLocation, upperEndLocation, locationHeight * (-1), 0.0f, 1.0f, lowerStartLocation,
         lowerEndLocation);
   }
   else   // Lower left origin
   {
      lowerStartLocation = textLocation;
      lowerEndLocation.mX = lowerStartLocation.mX + cos(textDirection) * locationWidth;
      lowerEndLocation.mY = lowerStartLocation.mY + sin(textDirection) * locationWidth;
      getParallelLine(lowerStartLocation, lowerEndLocation, locationHeight, 0.0f, 1.0f, upperStartLocation,
         upperEndLocation);
   }

   // Set GL options for text
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, tex.getTextureId());

   // Get direction of modelView
   bool bHorizontalFlip = false;
   bool bVerticalFlip = false;
   {
      double projection[16];
      double modelView[16];
      int viewport[4];
      glGetIntegerv(GL_VIEWPORT, viewport);
      glGetDoublev(GL_PROJECTION_MATRIX, projection);
      glGetDoublev(GL_MODELVIEW_MATRIX, modelView);

      LocationType screenLlCorner;
      LocationType screenLrCorner;
      LocationType screenUlCorner;

      GLdouble winZ;
      gluProject(lowerStartLocation.mX, lowerStartLocation.mY, 0.0, modelView, projection, viewport,
         &screenLlCorner.mX, &screenLlCorner.mY, &winZ);
      gluProject(lowerEndLocation.mX, lowerEndLocation.mY, 0.0, modelView, projection, viewport,
         &screenLrCorner.mX, &screenLrCorner.mY, &winZ);
      gluProject(upperStartLocation.mX, upperStartLocation.mY, 0.0, modelView, projection, viewport,
         &screenUlCorner.mX, &screenUlCorner.mY, &winZ);

      bHorizontalFlip = screenLlCorner.mX > screenLrCorner.mX;
      bVerticalFlip = screenLlCorner.mY < screenUlCorner.mY;   // Account for difference between screen widget origin
                                                               // (upper left) and OpenGL drawing origin (lower left)
   }

   // Draw text
   double maxS = tex.getTextWidth() / static_cast<double>(tex.getTextureWidth());
   double maxT = tex.getTextHeight() / static_cast<double>(tex.getTextureHeight());

   glBegin(GL_QUADS);
   glColor4ub(textColor.mRed, textColor.mGreen, textColor.mBlue, 255);

   if ((bHorizontalFlip == false) && (bVerticalFlip == false))
   {
      glTexCoord2f(0.0, 0.0);
      glVertex3f(lowerStartLocation.mX, lowerStartLocation.mY, 0.0);

      glTexCoord2f(maxS, 0.0);
      glVertex3f(lowerEndLocation.mX, lowerEndLocation.mY, 0.0);

      glTexCoord2f(maxS, maxT);
      glVertex3f(upperEndLocation.mX, upperEndLocation.mY, 0.0);

      glTexCoord2f(0.0, maxT);
      glVertex3f(upperStartLocation.mX, upperStartLocation.mY, 0.0);
   }
   else if ((bHorizontalFlip == true) && (bVerticalFlip == false))
   {
      glTexCoord2f(0.0, 0.0);
      glVertex3f(lowerEndLocation.mX, lowerEndLocation.mY, 0.0);

      glTexCoord2f(maxS, 0.0);
      glVertex3f(lowerStartLocation.mX, lowerStartLocation.mY, 0.0);

      glTexCoord2f(maxS, maxT);
      glVertex3f(upperStartLocation.mX, upperStartLocation.mY, 0.0);

      glTexCoord2f(0.0, maxT);
      glVertex3f(upperEndLocation.mX, upperEndLocation.mY, 0.0);
   }
   else if ((bHorizontalFlip == true) && (bVerticalFlip == true))
   {
      glTexCoord2f(0.0, 0.0);
      glVertex3f(upperEndLocation.mX, upperEndLocation.mY, 0.0);

      glTexCoord2f(maxS, 0.0);
      glVertex3f(upperStartLocation.mX, upperStartLocation.mY, 0.0);

      glTexCoord2f(maxS, maxT);
      glVertex3f(lowerStartLocation.mX, lowerStartLocation.mY, 0.0);

      glTexCoord2f(0.0, maxT);
      glVertex3f(lowerEndLocation.mX, lowerEndLocation.mY, 0.0);
   }
   else if ((bHorizontalFlip == false) && (bVerticalFlip == true))
   {
      glTexCoord2f(0.0, 0.0);
      glVertex3f(upperStartLocation.mX, upperStartLocation.mY, 0.0);

      glTexCoord2f(maxS, 0.0);
      glVertex3f(upperEndLocation.mX, upperEndLocation.mY, 0.0);

      glTexCoord2f(maxS, maxT);
      glVertex3f(lowerEndLocation.mX, lowerEndLocation.mY, 0.0);

      glTexCoord2f(0.0, maxT);
      glVertex3f(lowerStartLocation.mX, lowerStartLocation.mY, 0.0);
   }
   glEnd();
   
   // Reset GL options
   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
   glPopAttrib();
}

void DrawUtil::getParallelLine(LocationType origLineStart, LocationType origLineEnd, float offset,
                               float startRatio, float endRatio, LocationType &newLineStart, LocationType &newLineEnd)
{
   // Verify that arguments are valid
   if (((startRatio < 0.0) || (startRatio > 1.0)) || ((endRatio < 0.0) || (endRatio > 1.0)))
   {
      return;
   }
   if (startRatio >= endRatio)
   {
      return;
   }
   
   // Variables
   double offsetScreenX = 0;  // Screen coord X offset from main line to new line
   double offsetScreenY = 0;  // Screen coord Y offset from main line to new line
   double width = 0;          // Width of box bounding original line
   double height = 0;         // Height of box bounding original line
   double theta = 0;          // angle of line

   // Calculate parameters
   width = origLineEnd.mX - origLineStart.mX;
   height = origLineEnd.mY - origLineStart.mY;
   theta = atan2(height, width);
   offsetScreenX = offset * cos(theta);
   offsetScreenY = offset * sin(theta);

   // Calculate new line coordinates
   newLineStart.mX = origLineStart.mX + (width * startRatio) + offsetScreenY;
   newLineStart.mY = origLineStart.mY + (height * startRatio) - offsetScreenX;
   newLineEnd.mX = origLineEnd.mX - (width * (1 - endRatio)) + offsetScreenY;
   newLineEnd.mY = origLineEnd.mY - (height * (1 - endRatio)) - offsetScreenX;
}

void DrawUtil::getPerpendicularLine(LocationType origLineStart, LocationType origLineEnd,
                                    float length, LocationType& newLineStart, LocationType& newLineEnd)
{
   // Verify that arguments are valid
   if (length < 0)
   {
      return;
   }

   // Variables
   double perpLineLength = 0;    // The length of the perpendicular line
   double perpLineScreenX = 0;   // Screen coord x component of perp. line length
   double perpLineScreenY = 0;   // Screen coord y component of perp. line length
   double width = 0;             // Width of box bounding original line
   double height = 0;            // Height of box bounding original line
   double theta = 0;             // angle of original line

   // Calculate parameters
   width = origLineEnd.mX - origLineStart.mX;
   height = origLineEnd.mY - origLineStart.mY;
   theta = atan2(height, width);
   perpLineScreenX = length * cos(theta);
   perpLineScreenY = length * sin(theta);

   // Calculate perpendicular line coordinates
   newLineStart.mX = origLineEnd.mX - width - perpLineScreenY;
   newLineStart.mY = origLineEnd.mY - height + perpLineScreenX;
   newLineEnd.mX = origLineEnd.mX - width + perpLineScreenY;
   newLineEnd.mY = origLineEnd.mY - height - perpLineScreenX;
}

DrawUtil::TextTexture::TextTextureImp::TextTextureImp(GLuint textureId, QFont font, unsigned int textureWidth,
                                                      unsigned int textureHeight, unsigned int textWidth,
                                                      unsigned int textHeight) :
   mTextureWidth(textureWidth),
   mTextureHeight(textureHeight),
   mTextWidth(textWidth),
   mTextHeight(textHeight),
   mFont(font),
   mTextureId(textureId),
   mReferenceCount(1)
{
}

DrawUtil::TextTexture::TextTextureImp::TextTextureImp(const TextTextureImp& rhs)
{
   // should never get here
   VERIFYNRV(false);
}

const DrawUtil::TextTexture::TextTextureImp& DrawUtil::TextTexture::TextTextureImp::operator=
   (const TextTextureImp& rhs)
{
   // should never get here
   VERIFYRV(false, *this);
}

DrawUtil::TextTexture::TextTextureImp::~TextTextureImp()
{
   glDeleteTextures(1, &mTextureId);
}

unsigned int DrawUtil::TextTexture::getTextureWidth() const
{
   VERIFY(mImp != NULL);
   return mImp->mTextureWidth;
}

unsigned int DrawUtil::TextTexture::getTextureHeight() const
{ 
   VERIFY(mImp != NULL);
   return mImp->mTextureHeight; 
}

unsigned int DrawUtil::TextTexture::getTextWidth() const
{ 
   VERIFY(mImp != NULL);
   return mImp->mTextWidth;
}

unsigned int DrawUtil::TextTexture::getTextHeight() const
{ 
   VERIFY(mImp != NULL);
   return mImp->mTextHeight;
}

unsigned int DrawUtil::TextTexture::getTextureId() const
{ 
   VERIFY(mImp != NULL);
   return mImp->mTextureId;
}

DrawUtil::TextTexture::TextTexture() : mImp(NULL)
{
}

DrawUtil::TextTexture::TextTexture(GLuint textureId, QFont font, unsigned int textureWidth,
                      unsigned int textureHeight, unsigned int textWidth, 
                      unsigned int textHeight) :
   mImp(new TextTextureImp(textureId, font, textureWidth, textureHeight,
      textWidth, textHeight))
{
}

DrawUtil::TextTexture::TextTexture(const TextTexture &original) : mImp(original.mImp)
{
   if (mImp != NULL)
   {
      ++(mImp->mReferenceCount);
   }
}

const DrawUtil::TextTexture &DrawUtil::TextTexture::operator=(const TextTexture &rhs)
{
   if (this == &rhs)
   {
      return rhs;
   }

   invalidate();
   mImp = rhs.mImp;
   ++(mImp->mReferenceCount);

   return *this;
}
   
DrawUtil::TextTexture::~TextTexture()
{
   invalidate();
}

void DrawUtil::TextTexture::invalidate()
{
   if (mImp != NULL && --(mImp->mReferenceCount) == 0)
   {
      delete mImp;
   }
   mImp = NULL;
}

bool DrawUtil::TextTexture::isValid() const
{
   return (mImp != NULL);
}

DrawUtil::BitMaskPixelDrawer::BitMaskPixelDrawer(BitMask *pMask) 
: mpMask(pMask)
{
}

DrawUtil::BitMaskPixelDrawer::~BitMaskPixelDrawer()
{
}

void DrawUtil::BitMaskPixelDrawer::operator()(int x, int y)
{
   mpMask->setPixel(x, y, true);
}


namespace 
{

class SolidPixelDrawerImp : public DrawUtil::PixelDrawer::PixelDrawerImp
{
public:
   SolidPixelDrawerImp()
   {
      glBegin(GL_QUADS);
   }

   ~SolidPixelDrawerImp()
   {
      glEnd();
   }
   
   void drawPoint(int x, int y)
   {
      glVertex2i(x, y);
      glVertex2i(x, y+1);
      glVertex2i(x+1, y+1);
      glVertex2i(x+1, y);
   }
};

class CrosshairPixelDrawerImp : public DrawUtil::PixelDrawer::PixelDrawerImp
{
public:
   CrosshairPixelDrawerImp()
   {
      glBegin(GL_LINES);
   }

   ~CrosshairPixelDrawerImp()
   {
      glEnd();
   }
   
   void drawPoint(int x, int y)
   {
      glVertex2f(x+0.5, y);
      glVertex2f(x+0.5, y+1);
      glVertex2f(x, y+0.5);
      glVertex2f(x+1, y+0.5);
   }
};

class XPixelDrawerImp : public DrawUtil::PixelDrawer::PixelDrawerImp
{
public:
   XPixelDrawerImp()
   {
      glBegin(GL_LINES);
   }

   ~XPixelDrawerImp()
   {
      glEnd();
   }
   
   void drawPoint(int x, int y)
   {
      glVertex2i(x, y);
      glVertex2i(x+1, y+1);
      glVertex2i(x+1, y);
      glVertex2i(x, y+1);
   }
};

class AsteriskPixelDrawerImp : public DrawUtil::PixelDrawer::PixelDrawerImp
{
public:
   AsteriskPixelDrawerImp()
   {
      glBegin(GL_LINES);
   }

   ~AsteriskPixelDrawerImp()
   {
      glEnd();
   }
   
   void drawPoint(int x, int y)
   {
      glVertex2f(x+0.5, y);
      glVertex2f(x+0.5, y+1);
      glVertex2f(x, y+0.5);
      glVertex2f(x+1, y+0.5);
      glVertex2f(x, y);
      glVertex2f(x+1, y+1);
      glVertex2f(x+1, y);
      glVertex2f(x, y+1);
   }
};

class HorizontalLinePixelDrawerImp : public DrawUtil::PixelDrawer::PixelDrawerImp
{
public:
   HorizontalLinePixelDrawerImp()
   {
      glBegin(GL_LINES);
   }

   ~HorizontalLinePixelDrawerImp()
   {
      glEnd();
   }
   
   void drawPoint(int x, int y)
   {
      glVertex2f(x, y+0.5);
      glVertex2f(x+1, y+0.5);
   }
};

class VerticalLinePixelDrawerImp : public DrawUtil::PixelDrawer::PixelDrawerImp
{
public:
   VerticalLinePixelDrawerImp()
   {
      glBegin(GL_LINES);
   }

   ~VerticalLinePixelDrawerImp()
   {
      glEnd();
   }
   
   void drawPoint(int x, int y)
   {
      glVertex2f(x+0.5, y);
      glVertex2f(x+0.5, y+1);
   }
};

class ForwardSlashPixelDrawerImp : public DrawUtil::PixelDrawer::PixelDrawerImp
{
public:
   ForwardSlashPixelDrawerImp()
   {
      glBegin(GL_LINES);
   }

   ~ForwardSlashPixelDrawerImp()
   {
      glEnd();
   }
   
   void drawPoint(int x, int y)
   {
      glVertex2i(x, y);
      glVertex2i(x+1, y+1);
   }
};

class BackSlashPixelDrawerImp : public DrawUtil::PixelDrawer::PixelDrawerImp
{
public:
   BackSlashPixelDrawerImp()
   {
      glBegin(GL_LINES);
   }

   ~BackSlashPixelDrawerImp()
   {
      glEnd();
   }
   
   void drawPoint(int x, int y)
   {
      glVertex2i(x+1, y);
      glVertex2i(x, y+1);
   }
};

class BoxPixelDrawerImp : public DrawUtil::PixelDrawer::PixelDrawerImp
{
public:
   BoxPixelDrawerImp(DrawUtil::PixelDrawer::PixelDrawerImp *pInnerDraw) : mpInnerDraw(pInnerDraw)
   {
      // trusting pInnerDrawer to have done glBegin(GL_LINES)
   }

   ~BoxPixelDrawerImp()
   {
      // trusting pInnerDrawer to do glEnd()
   }
   
   void drawPoint(int x, int y)
   {
      glVertex2i(x, y);
      glVertex2i(x, y+1);
      glVertex2i(x, y+1);
      glVertex2i(x+1, y+1);
      glVertex2i(x+1, y+1);
      glVertex2i(x+1, y);
      glVertex2i(x+1, y);
      glVertex2i(x, y);
      if (mpInnerDraw.get() != NULL)
      {
         mpInnerDraw->drawPoint(x, y);
      }
   }

private:
   std::auto_ptr<DrawUtil::PixelDrawer::PixelDrawerImp> mpInnerDraw;
};
}

DrawUtil::PixelDrawer::PixelDrawer(SymbolType symbol)
{
   switch (symbol)
   {
   case SOLID:
      mpDrawer.reset(new ::SolidPixelDrawerImp);
      break;
   case CROSS_HAIR:
      mpDrawer.reset(new ::CrosshairPixelDrawerImp);
      break;
   case X:
      mpDrawer.reset(new ::XPixelDrawerImp);
      break;
   case ASTERISK:
      mpDrawer.reset(new ::AsteriskPixelDrawerImp);
      break;
   case HORIZONTAL_LINE:
      mpDrawer.reset(new ::HorizontalLinePixelDrawerImp);
      break;
   case VERTICAL_LINE:
      mpDrawer.reset(new ::VerticalLinePixelDrawerImp);
      break;
   case FORWARD_SLASH:
      mpDrawer.reset(new ::ForwardSlashPixelDrawerImp);
      break;
   case BACK_SLASH:
      mpDrawer.reset(new ::BackSlashPixelDrawerImp);
      break;
   case BOX:
      mpDrawer.reset(new ::BoxPixelDrawerImp(NULL));
      break;
   case BOXED_X:
      mpDrawer.reset(new ::BoxPixelDrawerImp(new ::XPixelDrawerImp));
      break;
   case BOXED_ASTERISK:
      mpDrawer.reset(new ::BoxPixelDrawerImp(new ::AsteriskPixelDrawerImp));
      break;
   case BOXED_HORIZONTAL_LINE:
      mpDrawer.reset(new ::BoxPixelDrawerImp(new ::HorizontalLinePixelDrawerImp));
      break;
   case BOXED_VERTICAL_LINE:
      mpDrawer.reset(new ::BoxPixelDrawerImp(new ::VerticalLinePixelDrawerImp));
      break;
   case BOXED_FORWARD_SLASH:
      mpDrawer.reset(new ::BoxPixelDrawerImp(new ::ForwardSlashPixelDrawerImp));
      break;
   case BOXED_BACK_SLASH:
      mpDrawer.reset(new ::BoxPixelDrawerImp(new ::BackSlashPixelDrawerImp));
      break;
   default:
      break;
   };
}

DrawUtil::PixelDrawer::~PixelDrawer()
{
}

void DrawUtil::PixelDrawer::operator()(int x, int y)
{
   if (mpDrawer.get() != NULL)
   {
      mpDrawer->drawPoint(x, y);
   }
}

// start from +x octant 0, count up as you go around counter-clockwise
//
/*
      +y
   \ 2 | 1 /
    \  |  /
   3 \ | / 0
  ----------- +x
   4 / | \ 7
    /  |  \
   / 5 | 6 \
*/
int DrawUtil::determineOctant(double dx, double dy)
{
   if (dy > 0)
   {
      if (dx > 0)
      {
         if (dx > dy)
         {
            return 0;
         }
         else
         {
            return 1;
         }
      }
      else
      {
         if (-dx > dy)
         {
            return 3;
         }
         else
         {
            return 2;
         }
      }
   }
   else
   {
      if (dx > 0)
      {
         if (dx > -dy)
         {
            return 7;
         }
         else
         {
            return 6;
         }
      }
      else
      {
         if (dx < dy)
         {
            return 4;
         }
         else
         {
            return 5;
         }
      }
   }
}
