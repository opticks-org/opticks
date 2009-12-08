/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DRAWUTIL_H
#define DRAWUTIL_H

#include <math.h>
#include <memory>

#include <QtCore/QString>
#include <QtGui/QFont>

#include "ColorType.h"
#include "AppVerify.h"
#include "glCommon.h"
#include "LocationType.h"
#include "MipMappedTextures.h"
#include "TypesFile.h"

class BitMask;

namespace DrawUtil
{
int determineOctant(double dx, double dy);

template<class T>
T minimum(T t1, T t2)
{
   return t1 < t2 ? t1 : t2;
}

template<class T>
T maximum(T t1, T t2)
{
   return t2 < t1 ? t1 : t2;
}

template<class T>
T sign(T t1)
{
   if (t1 > (T) 0.0)
   {
      return (T) 1.0;
   }
   else if (t1 == (T) 0.0)
   {
      return (T) 0.0;
   }

   return (T) -1.0;
}

/**
 * This class is used to store a text texture along with 
 * information that allows the texture to be redrawn
 *
 * It uses reference counting on a private implementation class
 * to clean up the GL texture after itself.
 */
class TextTexture
{
public:

   // Constructor
   TextTexture();
   
   // Accessor Methods
   unsigned int getTextureWidth() const;
   unsigned int getTextureHeight() const;
   unsigned int getTextWidth() const;
   unsigned int getTextHeight() const;
   unsigned int getTextureId() const;

   /**
    * Mark the object as invalid.  If this object is the only one
    * referencing the texture, it will be deleted.
    */
   void invalidate();

   /**
    * Test whether the object is valid.
    *
    * @return true if the object is a valid TextTexture
    */
   bool isValid() const;
   

   /**
   *   This is an overloaded constructor for TextTexture.  It will
   *   create a valid TextTexture, unlike the default constructor.
   *
   *   @param GLuint textureId
   *      The texture id of the texture to store
   *   @param QFont font
   *      The font that was used when generating the texture
   *   @param unsigned int textureWidth
   *      The width of the texture
   *   @param unsigned int textureHeight
   *      The height of the texture
   *   @param unsigned int textWidth
   *      The width of the bounding box of the text that was used to generate the
   *      texture
   *   @param unsigned int textHeight
   *      The height of the bounding box of the text that was used to generate the
   *      texture
   */
   TextTexture(GLuint textureId, QFont font, unsigned int textureWidth,
      unsigned int textureHeight, unsigned int textWidth, unsigned int textHeight);
   
   TextTexture(const TextTexture &original);

   const TextTexture &operator=(const TextTexture &rhs);

   ~TextTexture();   

private:

   class TextTextureImp
   {
   public:
      unsigned int mTextureWidth;      // The width of the texture
      unsigned int mTextureHeight;     // The height of the texture
      unsigned int mTextWidth;         // The width of the text (actually bounding box of text)
      unsigned int mTextHeight;        // The height of the text (acually bounding box of text)
      QFont mFont;                     // The font to use when rendering the text
      GLuint mTextureId;               // The texture id of the rendered texture
      int mReferenceCount;               // Should delete texture on destruction.  Always pass to copies
   
      TextTextureImp(GLuint textureId, QFont font, unsigned int textureWidth,
         unsigned int textureHeight, unsigned int textWidth, unsigned int textHeight);
      ~TextTextureImp();
   private:
      TextTextureImp();
      TextTextureImp(const TextTextureImp& rhs);
      const TextTextureImp &operator=(const TextTextureImp &rhs);
   };

   TextTextureImp* mImp;
};


const int VERTEX_COUNT = 360;
extern LocationType sCirclePoints[VERTEX_COUNT];

void initializeCircle();

extern unsigned char* sHatchPattern;

unsigned char* getHatchPattern(SymbolType eSymbol);

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
void drawEllipse(LocationType corner1, LocationType corner2, bool filled);

double linePointDistance(LocationType p1, LocationType p2, LocationType target, LocationType* pIntersection = NULL);

bool lineHit(LocationType p1, LocationType p2, LocationType target, double qualifier = 1.0);

inline bool isBetween(double x, double x1, double x2)
{
   if (x2 < x1)
   {
      if ((x2 < x) && (x <= x1))
      {
         return true;
      }
   }
   else
   {
      if ((x1 < x) && (x <= x2))
      {
         return true;
      }
   }

   return false;
}

bool isWithin(LocationType point, const LocationType region[], int count,
              const unsigned int ignoreSegmentsEnd[] = NULL, int ignoreCount = 0);

int isWithin(double x, double y, const double *pbx, const double *pby, int bcount);

bool unProject(double xPixel, double yPixel, double zPixel, const double modelMatrix[16],
               const double projectionMatrix[16], const int viewport[4],
               double *xCoord, double *yCoord, double *zCoord);

bool unProjectToZero(double xPixel, double yPixel, const double modelMatrix[16],
                     const double projectionMatrix[16], const int viewport[4],
                     double *xCoord, double *yCoord);

void restrictToViewport(int &ulStartColumn, int &ulStartRow,
                        int &ulEndColumn, int &ulEndRow);

double getPixelSize(GLfloat ulStartColumn, GLfloat ulStartRow,
                    GLfloat ulEndColumn, GLfloat ulEndRow);

bool isSmall(GLfloat ulStartColumn, GLfloat ulStartRow,
             GLfloat ulEndColumn, GLfloat ulEndRow, double dist);

LocationType getRotatedCoordinate(const LocationType& coord, const LocationType& rotationCenter,
                                  double dRotation);

template <class itemType>
inline const itemType& MAX(const itemType& left, const itemType& right)
{
   return left > right ? left : right;
}

template <class itemType>
inline const itemType& MIN(const itemType& left, const itemType& right)
{
   return left < right ? left : right;
}

/**
 * Updates the bounding box to include the vertex.
 *
 * @param llCorner
 *        Lower left (xmin, ymin) corner of bounding box.  
 *        May be modified to reflect updated box.
 *
 * @param urCorner
 *        Upper right (xmax, ymax) corner of bounding box.
 *        May be modified to reflect updated box.
 *
 * @param vertex
 *        New vertex to modify bounding box to include.
 */
void updateBoundingBox(LocationType &llCorner, LocationType &urCorner,
                       LocationType vertex);
                       

/**
*   This method renders and draws rotated text to the screen.
*
*   This method renders rotated GL text and draws it to the screen.  It also
*   returns a TextTexture object that contains all the information needed to 
*   redraw the text without rerendering it.
*
*   @param tex
*      The TextTexture object that contains the text texture to redraw.
*      If this is an invalid texture, this method will generate one and
*      fill in the reference.
*   @param text
*      The text to render and draw
*   @param font
*      The font to use when rendering and drawing the text
*   @param textColor
*      The color to draw the text
*   @param textLocation
*      The location to start drawing the text from
*   @param textDirection
*      The direction to draw the text from the start point
*   @param drawFromTop
*      True if start point is upper left corner of text, false if start point
*      is lower left corner of text
*   @return 
*      A TextTexture object that contains information about the text texture.
*      This object can be used to redraw the text without re-rendering it.
*/
void drawRotatedText(DrawUtil::TextTexture &tex, QString text, QFont font, 
                                                ColorType textColor, LocationType textLocation, 
                                                double textDirection, bool drawFromTop);

/**
*   This method calculates the coordinates for a line that is parallel to
*   the input coordinates.
*
*   This method calculates the coordinates for a line that is parallel to
*   the input coordinates.  Offset from the original line and length of the
*   new line can be specified.
*
*   @param origLineStart
*      The start point of the line used to get a parallel line from
*   @param origLineEnd
*      The end point of the line used to get a parallel line from
*   @param offset
*      The number of screen pixels to offset the parallel line from the original
*      line.
*   @param startRatio
*      The percentage along the original line to start drawing the parallel
*      line.  This is used along with the endRation to determine line length.
*      if startRatio is 0 and endRatio is 1, the parallel line will be the same
*      length as the original line.  The value for this argument must be between
*      0 and 1.
*   @param endRatio
*      The percentage along the original line to stop drawing the parallel line.
*      This value must be between 0 and 1.  It also must be greater than
*      startRatio.
*   @param newLineStart
*      The returned start point of the new parallel line
*   @param newLineEnd
*      The returned end point of the new parallel line
*/
void getParallelLine(LocationType origLineStart, LocationType origLineEnd, float offset,
                     float startRatio, float endRatio, LocationType &newLineStart, LocationType &newLineEnd);
                     

/**
*   This method calculates the coordinates for a line that is perpendicular
*   to the input coordinates.
*
*   This method calculates the coordinates for a line that is perpendicular
*   to the input coordinates.  The length of the perpendicular line can be
*   specified.  The perpendicular line is calculated at the end point of the
*   original line.
*
*   @param origLineStart
*      The start point of the line used to get a perpendicular line from
*   @param origLineEnd
*      The end point of the line used to get a perpendicular line from
*   @param length
*      The length of the perpendicular line in screen pixels
*   @param &newLineStart
*      The returned start point of the new perpendicular line
*   @param &newLineEnd
*      The returned end point of the new perpendicular line
*/
void getPerpendicularLine(LocationType origLineStart, LocationType origLineEnd,
                          float length, LocationType &newLineStart, LocationType &newLineEnd);


class BitMaskPixelDrawer
{
public:
   BitMaskPixelDrawer(BitMask *pMask);

   ~BitMaskPixelDrawer();

   void operator()(int x, int y);

private:
   BitMask* mpMask;
};



class PixelDrawer
{
public:
   class PixelDrawerImp
   {
   public:
      virtual ~PixelDrawerImp() {};
      virtual void drawPoint(int x, int y) = 0;
   };


   PixelDrawer(SymbolType symbol);
   ~PixelDrawer();

   void operator()(int x, int y);
private:
   std::auto_ptr<PixelDrawerImp> mpDrawer;
};

template<typename T>
void drawPixelLine(LocationType p0, LocationType p1, T &drawer)
{
   int x0 = floor(p0.mX);
   int y0 = floor(p0.mY);
   int x1 = floor(p1.mX);
   int y1 = floor(p1.mY);

   int dx = x1 - x0;
   int dy = y1 - y0;

   if (dx == 0)
   {
      int miny = std::min(y0, y1);
      int maxy = std::max(y0, y1);
      for (int y = miny; y <= maxy; ++y)
      {
         drawer(x0, y);
      }
      return;
   }
   else if (dy == 0)
   {
      int minx = std::min(x0, x1);
      int maxx = std::max(x0, x1);
      for (int x = minx; x <= maxx; ++x)
      {
         drawer(x, y0);
      }
      return;
   }

   int oct = determineOctant(dx, dy);

   // sanitize
   dx = abs(dx);
   dy = abs(dy);
   if (dy > dx)
   {
      int tmp = dx;
      dx = dy;
      dy = tmp;
   }

   int d = 2 * dy - dx;

   int incrE = 2 * dy;
   int incrNE = 2 * (dy - dx);

   int x = 0;
   int y = 0;
   drawer(x0, y0);
   // E and NE are using the canonical octant 0
   // for other octants, transformations are done in the methods below
   while (x < dx)
   {
      if (d <= 0)
      {
         d += incrE;
         x += 1.0;
      }
      else
      {
         d += incrNE;
         x += 1.0;
         y += 1.0;
      }

      switch (oct)
      {
         // draw as appropriate for the given octant
         case 0:
            drawer(x0 + x, y0 + y);
            break;
         case 1:
            drawer(x0 + y, y0 + x);
            break;
         case 2:
            drawer(x0 - y, y0 + x);
            break;
         case 3:
            drawer(x0 - x, y0 + y);
            break;
         case 4:
            drawer(x0 - x, y0 - y);
            break;
         case 5:
            drawer(x0 - y, y0 - x);
            break;
         case 6:
            drawer(x0 + y, y0 - x);
            break;
         case 7:
            drawer(x0 + x, y0 - y);
            break;
         default:
            break;
      }
   }
}

template<typename T>
void drawPixelPolygon(const std::vector<LocationType> &vertices, const std::vector<unsigned int> &paths, 
                      int iStartColumn, int iStartRow, int iEndColumn, int iEndRow, T &drawer)
{
   unsigned int numVertices = vertices.size();
   std::vector<double> xVertices(numVertices);
   std::vector<double> yVertices(numVertices);

   unsigned int k = 0;
   for (k = 0; k < numVertices; ++k)
   {
      xVertices[k] = vertices[k].mX - 0.5;
      yVertices[k] = vertices[k].mY - 0.5;
   }

   std::vector<double> slopes(numVertices - 1);
   for (k = 0; k < numVertices - 1; ++k)
   {
      if (fabs(yVertices[k + 1] - yVertices[k]) > 1e-6)
      {
         slopes[k] = (xVertices[k + 1] - xVertices[k]) / (yVertices[k + 1] - yVertices[k]);
      }
      else
      {
         slopes[k] = 0.0;
      }
   }

   double* pX = &(xVertices.front());
   double* pY = &(yVertices.front());
   double* pSlopes = &(slopes.front());

   if ((pX == NULL) || (pY == NULL) || (pSlopes == NULL))
   {
      return;
   }

   int iRowIndex = 0;
   std::vector<unsigned char> insides(iEndColumn - iStartColumn + 1);
   unsigned char *pInsides = &insides.front();
   for (int i = iStartRow, iRowIndex = 0; i <= iEndRow; ++i, ++iRowIndex)
   {
      bool isInside = false;
      double dRow = i;
      fill(insides.begin(), insides.end(), '\0');

      int iColumnIndex = 0;
      int j = 0;

      unsigned int pathCount = 1;
      for (k = 0; k < numVertices - 1; ++k)
      {
         if ( (pathCount < paths.size()) && (k+1 == paths[pathCount]) )
         {
            ++pathCount;
            continue;
         }
         if (DrawUtil::isBetween(dRow, pY[k], pY[k + 1]))
         {
            double x = (pSlopes[k] * (dRow - pY[k])) + pX[k];
            for (j = iStartColumn, iColumnIndex = 0; j <= iEndColumn; ++j, ++iColumnIndex)
            {
               if (x <= j)
               {
                  pInsides[iColumnIndex] = !pInsides[iColumnIndex];
               }
            }
         }
      }
      for (j = iStartColumn, iColumnIndex = 0; j <= iEndColumn; ++j, ++iColumnIndex)
      {
         if (pInsides[iColumnIndex] != '\0')
         {
            drawer(j, i);
         }
      }
   }

}
}

#endif
