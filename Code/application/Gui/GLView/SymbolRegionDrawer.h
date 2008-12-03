/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SYMBOLREGIONDRAWER_H
#define SYMBOLREGIONDRAWER_H

#include <QtGui/QColor>

#include "glCommon.h"
#include "TypesFile.h"

#include <algorithm>

namespace SymbolRegionDrawer
{

template<class Oper>
class DrawInfo
{
public:
   DrawInfo(int startColumn, int endColumn, int startRow, int endRow, int visStartColumn, int visEndColumn,
      int visStartRow, int visEndRow, double pixelSize, const Oper& oper) :
   mStartColumn(startColumn), mEndColumn(endColumn),
   mStartRow(startRow), mEndRow(endRow),
   mVisStartColumn(visStartColumn), mVisEndColumn(visEndColumn),
   mVisStartRow(visStartRow), mVisEndRow(visEndRow),
   mPixelSize(pixelSize), mOper(oper) {}

   int mStartColumn;
   int mEndColumn;
   int mStartRow;
   int mEndRow;
   int mVisStartColumn;
   int mVisEndColumn;
   int mVisStartRow;
   int mVisEndRow;
   double mPixelSize;
   const Oper& mOper;
};

template<class Oper> 
void drawMarkers(int startColumn, int startRow, int endColumn, int endRow, int visStartColumn, int visStartRow,
   int visEndColumn, int visEndRow, SymbolType eSymbol, QColor clrMarker, const Oper &oper)
{
   if (startColumn > endColumn || startRow > endRow)
   {
      return;
   }

   unsigned int alpha = 0xff;
   double pixelSize = DrawUtil::getPixelSize(visStartColumn, visStartRow, visEndColumn, visEndRow);

   DrawInfo<Oper> info(startColumn, endColumn, startRow, endRow,
      visStartColumn, visEndColumn, visStartRow, visEndRow, pixelSize, oper);

   if (pixelSize < 2.0 && eSymbol != BOX && eSymbol != SOLID)
   {
      glEnable(GL_BLEND);
      glAlphaFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      alpha = 0xff;//0x7f;
   }

   if (pixelSize < 0.5 && eSymbol != BOX)
   {
      if (eSymbol != BOX && eSymbol != SOLID)
      {
         alpha = 0xff;//0x3f;
      }
      glColor4ub(clrMarker.red(), clrMarker.green(), clrMarker.blue(), alpha);
      drawHorizontalLine(info); 
   }
   else
   {
      if (pixelSize < 2.0 && eSymbol != BOX)
      {
         eSymbol = SOLID;
      }

      glColor4ub(clrMarker.red(), clrMarker.green(), clrMarker.blue(), alpha);

      switch (eSymbol)
      {
         case X:
            drawNegativeSlope(info); 
            drawPositiveSlope(info); 
            break;

         case CROSS_HAIR:
            drawVerticalLine(info); 
            drawHorizontalLine(info); 
            break;

         case VERTICAL_LINE:
            drawVerticalLine(info); 
            break;

         case HORIZONTAL_LINE:
            drawHorizontalLine(info); 
            break;

         case ASTERISK:
            drawNegativeSlope(info); 
            drawPositiveSlope(info); 
            drawHorizontalLine(info); 
            drawVerticalLine(info); 
            break;

         case FORWARD_SLASH:
            drawPositiveSlope(info); 
            break;

         case BACK_SLASH:
            drawNegativeSlope(info); 
            break;

         case SOLID:
            drawSolid(info); 
            break;

         case BOXED_X:
            drawNegativeSlope(info); 
            drawPositiveSlope(info); 
            drawVerticalBorder(info); 
            drawHorizontalBorder(info); 
            break;

         case BOXED_CROSS_HAIR:
            drawVerticalLine(info); 
            drawHorizontalLine(info); 
            drawVerticalBorder(info); 
            drawHorizontalBorder(info); 
            break;

         case BOXED_VERTICAL_LINE:
            drawVerticalLine(info); 
            drawVerticalBorder(info); 
            drawHorizontalBorder(info); 
            break;

         case BOXED_HORIZONTAL_LINE:
            drawHorizontalLine(info); 
            drawVerticalBorder(info); 
            drawHorizontalBorder(info); 
            break;

         case BOXED_ASTERISK:
            drawNegativeSlope(info); 
            drawPositiveSlope(info); 
            drawHorizontalLine(info); 
            drawVerticalLine(info); 
            drawVerticalBorder(info); 
            drawHorizontalBorder(info); 
            break;

         case BOXED_FORWARD_SLASH:
            drawPositiveSlope(info); 
            drawVerticalBorder(info); 
            drawHorizontalBorder(info); 
            break;

         case BOXED_BACK_SLASH:
            drawNegativeSlope(info); 
            drawVerticalBorder(info); 
            drawHorizontalBorder(info); 
            break;

         case BOX:
            drawVerticalBorder(info); 
            drawHorizontalBorder(info); 
            break;
      }
   }
   glDisable(GL_BLEND);
}

template<class Oper>
void drawSolid(const DrawInfo<Oper> &info)
{
   int xx = 0;
   int yy = 0;
   bool prevDraw = false;
   int start, stop;

   float pixelSize = std::max(1.0, 1.0/info.mPixelSize);

   glBegin(GL_QUADS);
   for (yy = info.mVisStartRow; yy <= info.mVisEndRow; yy++)
   {
      prevDraw = false;
      for (xx = info.mVisStartColumn; xx <= info.mVisEndColumn; xx++)
      {
         bool bDraw = info.mOper(yy, xx);
         if (bDraw == true && prevDraw == false)
         {
            start = xx;
            prevDraw = true;
         }

         if (bDraw == false && prevDraw == true)
         {
            float stopPlus1 = xx-1+pixelSize;
            float yyPlus1 = yy + pixelSize;
            glVertex2f(stopPlus1, yyPlus1);
            glVertex2f(stopPlus1, yy);
            glVertex2f(start, yy);
            glVertex2f(start, yyPlus1);
            prevDraw = false;
         }
      }

      if (prevDraw == true)
      {
         stop = xx - 1;
         glVertex2f(stop + pixelSize, yy + pixelSize);
         glVertex2f(stop + pixelSize, yy + 0.0f);
         glVertex2f(start + 0.0f, yy + 0.0f);
         glVertex2f(start + 0.0f, yy + pixelSize);
         prevDraw = false;
      }
   }
   glEnd();
}

template<class Oper>
void drawHorizontalBorder(const DrawInfo<Oper> &info)
{
   int xx = 0;
   int yy = 0;

   glBegin(GL_LINES);

   bool prevDraw = false;
   int start, stop;

   for (yy = info.mVisStartRow; yy <= info.mVisEndRow; yy++)
   {
      for (xx = info.mVisStartColumn; xx <= info.mVisEndColumn; xx++)
      {
         bool bDrawThis = false;
         bDrawThis = info.mOper(yy, xx);
         bool bDrawPrev = false;
         if (yy != info.mStartRow)
         {
            bDrawPrev = info.mOper(yy - 1, xx);
         }

         bDrawThis = (bDrawThis != bDrawPrev);

         if (bDrawThis == true && prevDraw == false)
         {
            start = xx;
            prevDraw = true;
         }
         if (bDrawThis == false && prevDraw == true)
         {
            stop = xx - 1;
            glVertex2f(start + 0.0f, yy);
            glVertex2f(stop + 1.0f, yy);
            prevDraw = false;
         }
      }

      if (prevDraw == true)
      {
         stop = xx - 1;
         glVertex2f(start + 0.0f, yy);
         glVertex2f(stop + 1.0f, yy);
         prevDraw = false;
      }

      if (yy == info.mVisEndRow)
      {
         for (xx = info.mVisStartColumn; xx <= info.mVisEndColumn; xx++)
         {
            bool bDraw = info.mOper(yy, xx);

            if (bDraw == true && prevDraw == false)
            {
               start = xx;
               prevDraw = true;
            }
            if (bDraw == false && prevDraw == true)
            {
               stop = xx - 1;
               glVertex2f(start + 0.0f, yy + 1.0);
               glVertex2f(stop + 1.0f, yy + 1.0);
               prevDraw = false;
            }
         }

         if (prevDraw == true)
         {
            stop = xx - 1;
            glVertex2f(start + 0.0f, yy + 1.0);
            glVertex2f(stop + 1.0f, yy + 1.0);
            prevDraw = false;
         }
      }
   }

   glEnd();
}

template<class Oper>
void drawVerticalBorder(const DrawInfo<Oper> &info)
{
   int xx = 0;
   int yy = 0;

   glBegin(GL_LINES);

   bool prevDraw = false;
   int start, stop;

   for (xx = info.mVisStartColumn; xx <= info.mVisEndColumn; xx++)
   {
      for (yy = info.mVisStartRow; yy <= info.mVisEndRow; yy++)
      {
         bool bDrawThis = false;
         bDrawThis = info.mOper(yy, xx);
         bool bDrawPrev = false;
         if (xx != info.mStartColumn)
         {
            bDrawPrev = info.mOper(yy, xx - 1);
         }

         bDrawThis = (bDrawThis != bDrawPrev);

         if (bDrawThis == true && prevDraw == false)
         {
            start = yy;
            prevDraw = true;
         }
         if (bDrawThis == false && prevDraw == true)
         {
            stop = yy - 1;
            glVertex2f(xx, start + 0.0f);
            glVertex2f(xx, stop + 1.0f);
            prevDraw = false;
         }
      }

      if (prevDraw == true)
      {
         stop = yy - 1;
         glVertex2f(xx, start + 0.0f);
         glVertex2f(xx, stop + 1.0f);
         prevDraw = false;
      }

      if (xx == info.mVisEndColumn)
      {
         for (yy = info.mVisStartRow; yy <= info.mVisEndRow; yy++)
         {
            bool bDraw = info.mOper(yy, xx);

            if (bDraw == true && prevDraw == false)
            {
               start = yy;
               prevDraw = true;
            }
            if (bDraw == false && prevDraw == true)
            {
               stop = yy - 1;
               glVertex2f(xx + 1.0f, start + 0.0);
               glVertex2f(xx + 1.0f, stop + 1.0);
               prevDraw = false;
            }
         }

         if (prevDraw == true)
         {
            stop = yy - 1;
            glVertex2f(xx + 1.0f, start + 0.0);
            glVertex2f(xx + 1.0f, stop + 1.0);
            prevDraw = false;
         }
      }
   }

   glEnd();
}

template<class Oper>
void drawHorizontalLine(const DrawInfo<Oper> &info)
{
   int xx = 0;
   int yy = 0;

   float pixelSizeF = 1.0 / DrawUtil::getPixelSize(info.mVisStartColumn, info.mVisStartRow, 
      info.mVisEndColumn, info.mVisEndRow);

   float pixelSize2 = pixelSizeF / 2.0f;
   int pixelSizeY = std::max(1, (int)(floor(pixelSizeF/2.0)));
   int pixelSizeX = std::max(1, (int)floor(pixelSizeF));

   if (pixelSizeF < 1.0)
   {
      pixelSizeF = 1.0;
   }

   glBegin(GL_LINES);

   bool prevDraw = false;
   int start, stop;

   for (yy = info.mVisStartRow; yy <= info.mVisEndRow; yy += pixelSizeY)
   {
      for (xx = info.mVisStartColumn; xx <= info.mVisEndColumn; xx += pixelSizeX)
      {
         bool bDraw = false;
         int yyy, xxx;
         int i, j;
         for (j = 0, yyy = yy; j < pixelSizeY && !bDraw && yyy <= info.mVisEndRow; ++j, ++yyy)
         {
            for (i = 0, xxx = xx; i < pixelSizeX && !bDraw && xxx <= info.mVisEndColumn; ++i, ++xxx)
            {
               bDraw |= info.mOper(yyy, xxx);
            }
         }
         if (bDraw == true && prevDraw == false)
         {
            start = xx;
            prevDraw = true;
         }
         if (bDraw == false && prevDraw == true)
         {
            stop = xx - 1;
            glVertex2f(start + 0.0f, yy + pixelSize2+0.5);
            glVertex2f(stop + pixelSizeF, yy + pixelSize2+0.5);
            prevDraw = false;
         }
      }

      if (prevDraw == true)
      {
         stop = xx - 1;
         glVertex2f(start + 0.0f, yy + pixelSize2+0.5);
         glVertex2f(stop + pixelSizeF, yy + pixelSize2+0.5);
         prevDraw = false;
      }
   }

   glEnd();
}

template<class Oper>
void drawVerticalLine(const DrawInfo<Oper> &info)
{
   int xx = 0;
   int yy = 0;

   glBegin(GL_LINES);

   bool prevDraw = false;
   int start, stop;

   for (xx = info.mVisStartColumn; xx <= info.mVisEndColumn; xx++)
   {
      for (yy = info.mVisStartRow; yy <= info.mVisEndRow; yy++)
      {
         bool bDraw = false;
         bDraw = info.mOper(yy, xx);

         if (bDraw == true && prevDraw == false)
         {
            start = yy;
            prevDraw = true;
         }
         if (bDraw == false && prevDraw == true)
         {
            stop = yy - 1;
            glVertex2f(xx + 0.5f, start + 0.0f);
            glVertex2f(xx + 0.5f, stop + 1.0f);
            prevDraw = false;
         }
      }

      if (prevDraw == true)
      {
         stop = yy-1;
         glVertex2f(xx + 0.5f, start + 0.0f);
         glVertex2f(xx + 0.5f, stop + 1.0f);
         prevDraw = false;
      }
   }

   glEnd();
}

template<class Oper>
void drawPositiveSlope(const DrawInfo<Oper> &info)
{
   int xx = 0;
   int yy = 0;
   int rowStart, maxLength, pixelIndex;
   int start, stop;
   int xSize = info.mVisEndColumn - info.mVisStartColumn + 1;
   int ySize = info.mVisEndRow - info.mVisStartRow + 1;

   glBegin(GL_LINES);

   for (rowStart = 0; rowStart < ySize; rowStart++)
   {
      maxLength = std::min(xSize - 1, rowStart);
      bool prevDraw = false;
      for (pixelIndex = 0; pixelIndex <= maxLength; pixelIndex++)
      {
         xx = pixelIndex + info.mVisStartColumn;
         yy = ySize - rowStart + pixelIndex - 1 + info.mVisStartRow;

         bool bDraw = false;
         bDraw = info.mOper(yy, xx);

         if (bDraw == true && prevDraw == false)
         {
            start = pixelIndex;
            prevDraw = true;
         }
         if (bDraw == false && prevDraw == true)
         {
            stop = pixelIndex - 1;
            glVertex2f(start + info.mVisStartColumn + 0, ySize - rowStart + start - 1 + info.mVisStartRow + 0);
            glVertex2f(stop + info.mVisStartColumn + 1, ySize - rowStart + stop - 1 + info.mVisStartRow + 1);
            prevDraw = false;
         }
      }

      if (prevDraw == true)
      {
         stop = pixelIndex - 1;
         glVertex2f(start + info.mVisStartColumn + 0, ySize - rowStart + start - 1 + info.mVisStartRow + 0);
         glVertex2f(stop + info.mVisStartColumn + 1, ySize - rowStart + stop - 1 + info.mVisStartRow + 1);
         prevDraw = false;
      }
   }

   for (rowStart = 1; rowStart < xSize; rowStart++)
   {
      maxLength = std::min(ySize - 1, xSize - rowStart - 1);
      bool prevDraw = false;
      for (pixelIndex = 0; pixelIndex <= maxLength; pixelIndex++)
      {
         xx = pixelIndex + rowStart + info.mVisStartColumn;
         yy = pixelIndex + info.mVisStartRow;

         bool bDraw = false;
         bDraw = info.mOper(yy, xx);

         if (bDraw == true && prevDraw == false)
         {
            start = pixelIndex;
            prevDraw = true;
         }
         if (bDraw == false && prevDraw == true)
         {
            stop = pixelIndex - 1;
            glVertex2f(start + rowStart + info.mVisStartColumn + 0, start + info.mVisStartRow + 0);
            glVertex2f(stop + rowStart + info.mVisStartColumn + 1, stop + info.mVisStartRow + 1);
            prevDraw = false;
         }
      }

      if (prevDraw == true)
      {
         stop = pixelIndex - 1;
         glVertex2f(start + rowStart + info.mVisStartColumn + 0, start + info.mVisStartRow + 0);
         glVertex2f(stop + rowStart + info.mVisStartColumn + 1, stop + info.mVisStartRow + 1);
         prevDraw = false;
      }
   }

   glEnd();
}

template<class Oper>
void drawNegativeSlope(const DrawInfo<Oper> &info)
{
   int xx = 0;
   int yy = 0;
   int rowStart, maxLength, pixelIndex;
   int start, stop;
   int xSize = info.mVisEndColumn - info.mVisStartColumn + 1;
   int ySize = info.mVisEndRow - info.mVisStartRow + 1;

   glBegin(GL_LINES);

   for (rowStart = 0; rowStart < ySize; rowStart++)
   {
      maxLength = std::min(xSize - 1, rowStart);
      bool prevDraw = false;
      for (pixelIndex = 0; pixelIndex <= maxLength; pixelIndex++)
      {
         xx = pixelIndex + info.mVisStartColumn;
         yy = rowStart - pixelIndex + info.mVisStartRow;

         bool bDraw = false;
         bDraw = info.mOper(yy, xx);

         if (bDraw == true && prevDraw == false)
         {
            start = pixelIndex;
            prevDraw = true;
         }
         if (bDraw == false && prevDraw == true)
         {
            stop = pixelIndex - 1;
            glVertex2f(start + info.mVisStartColumn + 0, rowStart - start + info.mVisStartRow + 1);
            glVertex2f(stop + info.mVisStartColumn + 1, rowStart - stop + info.mVisStartRow + 0);
            prevDraw = false;
         }
      }

      if (prevDraw == true)
      {
         stop = pixelIndex - 1;
         glVertex2f(start + info.mVisStartColumn + 0, rowStart - start + info.mVisStartRow + 1);
         glVertex2f(stop + info.mVisStartColumn + 1, rowStart - stop + info.mVisStartRow + 0);
         prevDraw = false;
      }
   }

   for (rowStart = 1; rowStart < xSize; rowStart++)
   {
      maxLength = std::min(ySize - 1, xSize - rowStart - 1);
      bool prevDraw = false;
      for (pixelIndex = 0; pixelIndex <= maxLength; pixelIndex++)
      {
         xx = pixelIndex + rowStart + info.mVisStartColumn;
         yy = ySize - pixelIndex - 1 + info.mVisStartRow;

         bool bDraw = false;
         bDraw = info.mOper(yy, xx);

         if (bDraw == true && prevDraw == false)
         {
            start = pixelIndex;
            prevDraw = true;
         }
         if (bDraw == false && prevDraw == true)
         {
            stop = pixelIndex - 1;
            glVertex2f(start + rowStart + info.mVisStartColumn + 0, ySize - start - 1 + info.mVisStartRow + 1);
            glVertex2f(stop + rowStart + info.mVisStartColumn + 1, ySize - stop - 1 + info.mVisStartRow + 0);
            prevDraw = false;
         }
      }

      if (prevDraw == true)
      {
         stop = pixelIndex - 1;
         glVertex2f(start + rowStart + info.mVisStartColumn + 0, ySize - start - 1 + info.mVisStartRow + 1);
         glVertex2f(stop + rowStart + info.mVisStartColumn + 1, ySize - stop - 1 + info.mVisStartRow + 0);
         prevDraw = false;
      }
   }

   glEnd();
}

template<class Oper>
void drawPoints(const DrawInfo<Oper> &info)
{
   int xx = 0;
   int yy = 0;
   float pixelSizeF = 1.0 / info.mPixelSize;

   int pixelSize = floor(pixelSizeF);
   if (pixelSize <= 0)
   {
      pixelSize = 1;
   }

   float pixelSize2 = pixelSize/2.0f;
   glBegin(GL_POINTS);
   for (yy = info.mVisStartRow; yy <= info.mVisEndRow; yy += pixelSize)
   {
      for (xx = info.mVisStartColumn; xx <= info.mVisEndColumn; xx += pixelSize)
      {
         bool drawn = false;
         for (int i = 0; i < pixelSize && !drawn; ++i)
         {
            for (int j = 0; j < pixelSize && !drawn; ++j)
            {
               if (info.mOper(yy+i, xx+j))
               {
                  glVertex2f(xx + pixelSize2, yy + pixelSize2);
                  drawn = true;
               }
            }
         }
      }
   }
   glEnd();
}

}

#endif
