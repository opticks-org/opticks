/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QImage>
#include <QtGui/QPainter>

#include "DrawUtil.h"
#include "Endian.h"
#include "glCommon.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "MipMappedTextures.h"
#include "MultiLineTextDialog.h"
#include "OrthographicView.h"
#include "ProductView.h"
#include "SpatialDataView.h"
#include "TextObjectImp.h"
#include "View.h"

#include <math.h>

#include <string>
#include <vector>
using namespace std;

TextObjectImp::TextObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                             LocationType pixelCoord) :
   RectangleObjectImp(id, type, pLayer, pixelCoord),
   mTextureId(0),
   mTextureWidth(1),
   mTextureHeight(1),
   mDataWidth(0),
   mDataHeight(0),
   mUpdateTexture(true),
   mUpdateBoundingBox(true)
{
   addProperty("Font");
   addProperty("TextAlignment");
   addProperty("TextColor");
   addProperty("TextString");

   setLineState(false);
   setFillStyle(EMPTY_FILL);
}

TextObjectImp::~TextObjectImp()
{
   while (!mTextureIdStack.empty())
   {
      // this does not attempt to delete any textures
      // except the top level texture since the GLContext
      // for other textures is likely to be invalid.
      // This shouldn't occur if callers are pairing
      // a temp context change with a draw...this just
      // prevents a possible crash due to an invalid texture id
      mTextureId = mTextureIdStack.top();
      mTextureIdStack.pop();
   }
   if (mTextureId != 0)
   {
      glDeleteTextures(1, &mTextureId);
   }
}

void TextObjectImp::draw(double zoomFactor) const
{
   if (mUpdateTexture == true)
   {
      (const_cast<TextObjectImp*> (this))->updateTexture();
   }

   RectangleObjectImp::draw(zoomFactor);
   drawTexture();
}

void TextObjectImp::drawTexture() const
{
   if (mUpdateTexture == true)
   {
      (const_cast<TextObjectImp*> (this))->updateTexture();
   }

   if (mTextureId == 0)
   {
      return;
   }

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   glEnable(GL_TEXTURE_2D);

   // switch to using this tile's texture
   glBindTexture(GL_TEXTURE_2D, mTextureId);

   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   double maxS = mDataWidth / static_cast<double>(mTextureWidth);
   double maxT = mDataHeight / static_cast<double>(mTextureHeight);

   ColorType color = getTextColor();
   glBegin(GL_QUADS);
   glColor4ub(color.mRed, color.mGreen, color.mBlue, 255);

   // Determine if the bounding box is flipped based on its screen locations
   bool bHorizontalFlip = false;
   bool bVerticalFlip = false;

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      pLayer->isFlipped(llCorner, urCorner, bHorizontalFlip, bVerticalFlip);

      // Account for the difference between screen origin (upper left) and OpenGL origin (lower left)
      bVerticalFlip = !bVerticalFlip;
   }

   // Draw the text left to right and right side up
   if ((bHorizontalFlip == false) && (bVerticalFlip == false))
   {
      glTexCoord2f(0.0, 0.0);
      glVertex3f(llCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(maxS, 0.0);
      glVertex3f(urCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(maxS, maxT);
      glVertex3f(urCorner.mX, urCorner.mY, 0.0);

      glTexCoord2f(0.0, maxT);
      glVertex3f(llCorner.mX, urCorner.mY, 0.0);
   }
   else if ((bHorizontalFlip == true) && (bVerticalFlip == false))
   {
      glTexCoord2f(maxS, 0.0);
      glVertex3f(llCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(0.0, 0.0);
      glVertex3f(urCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(0.0, maxT);
      glVertex3f(urCorner.mX, urCorner.mY, 0.0);

      glTexCoord2f(maxS, maxT);
      glVertex3f(llCorner.mX, urCorner.mY, 0.0);
   }
   else if ((bHorizontalFlip == true) && (bVerticalFlip == true))
   {
      glTexCoord2f(maxS, maxT);
      glVertex3f(llCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(0.0, maxT);
      glVertex3f(urCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(0.0, 0.0);
      glVertex3f(urCorner.mX, urCorner.mY, 0.0);

      glTexCoord2f(maxS, 0.0);
      glVertex3f(llCorner.mX, urCorner.mY, 0.0);
   }
   else if ((bHorizontalFlip == false) && (bVerticalFlip == true))
   {
      glTexCoord2f(0.0, maxT);
      glVertex3f(llCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(maxS, maxT);
      glVertex3f(urCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(maxS, 0.0);
      glVertex3f(urCorner.mX, urCorner.mY, 0.0);

      glTexCoord2f(0.0, 0.0);
      glVertex3f(llCorner.mX, urCorner.mY, 0.0);
   }

   glEnd();

   glDisable(GL_BLEND);
   glDisable(GL_ALPHA_TEST);
   glDisable(GL_TEXTURE_2D);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   if (!mTextureIdStack.empty())
   {
      TextObjectImp* pNonConst = const_cast<TextObjectImp*>(this);
      if (mTextureId != 0)
      {
         glDeleteTextures(1, &mTextureId);
      }
      pNonConst->mTextureId = mTextureIdStack.top();
      pNonConst->mTextureIdStack.pop();
   }
}

void TextObjectImp::updateTexture()
{
   if (mTextureId != 0)
   {
      glDeleteTextures(1, &mTextureId);
      mTextureId = 0;
   }

   // Get the text to display
   string text = getText();
   if (text.empty())
   {
      return;
   }

   glGenTextures(1, &mTextureId);
   if (mTextureId == 0)
   {
      return;
   }

   mUpdateTexture = false;
   QString strMessage = QString::fromStdString(text);

   // Get a scaled font for the text image
   QFont font = getFont();
   double minSize = font.pointSizeF();

   QFont scaledFont = getScaledFont(minSize);

   int iMaxSize = 0;
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &iMaxSize);

   int iAlignment = getTextAlignment();

   QFontMetrics ftMetrics(scaledFont);
   QRect boundingBox = ftMetrics.boundingRect(0, 0, iMaxSize, iMaxSize,
      iAlignment | Qt::TextWordWrap, strMessage);

   int iWidth = boundingBox.width();
   int iHeight = boundingBox.height();

   QBitmap textBitmap(iWidth, iHeight);
   textBitmap.clear();

   QPainter painter(&textBitmap);
   painter.setFont(scaledFont);
   painter.setPen(QPen(Qt::color1));
   painter.drawText(0, 0, iWidth, iHeight, iAlignment | Qt::TextWordWrap, strMessage);
   painter.end();

   QImage image = textBitmap.toImage();

   mDataWidth = iWidth;
   mDataHeight = iHeight;

   mTextureWidth = pow(2.0, ceil(log10(static_cast<double>(iWidth)) / log10(2.0))) + 0.5;
   mTextureHeight = pow(2.0, ceil(log10(static_cast<double>(iHeight)) / log10(2.0))) + 0.5;

   int bufSize = mTextureWidth * mTextureHeight;
   vector<unsigned int> texData(bufSize, 0);

   unsigned int* target = &texData[0];
   for (int j = 0; j < iHeight; j++)
   {
      target = &texData[mTextureWidth * j];
      for (int i = 0; i < iWidth; i++)
      {
         QRgb rgb = image.pixel(i, j);
         if (image.pixel(i, j) != 0xffffffff)
         {
            *target = 0xffffffff;
         }
         else
         {
            if (Endian::getSystemEndian() == LITTLE_ENDIAN)
            {
               *target = 0xffffff;
            }
            else
            {
               *target = 0xffffff00;
            }
         }
         target++;
      }
   }

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, mTextureId);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   gluBuild2DMipmaps(GL_TEXTURE_2D, 4, mTextureWidth, mTextureHeight, GL_RGBA, GL_UNSIGNED_BYTE, &texData[0]);
   glDisable(GL_TEXTURE_2D);

   if (mUpdateBoundingBox == true)
   {
      updateBoundingBox();
   }
}

bool TextObjectImp::setProperty(const GraphicProperty* pProperty)
{
   if ((pProperty->getName() == "TextString") || (pProperty->getName() == "Font") ||
      (pProperty->getName() == "TextAlignment"))
   {
      mUpdateTexture = true;
      mUpdateBoundingBox = true;
   }

   return RectangleObjectImp::setProperty(pProperty);
}

bool TextObjectImp::hit(LocationType pixelCoord) const 
{ 
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   double xMin = DrawUtil::minimum(llCorner.mX, urCorner.mX);
   double xMax = DrawUtil::maximum(llCorner.mX, urCorner.mX);
   double yMin = DrawUtil::minimum(llCorner.mY, urCorner.mY);
   double yMax = DrawUtil::maximum(llCorner.mY, urCorner.mY);

   bool bHit = false;

   if ((pixelCoord.mX > xMin) && (pixelCoord.mY > yMin) &&
      (pixelCoord.mX < xMax) && (pixelCoord.mY < yMax))
   {
      bHit = true;
   }

   return bHit;
}

bool TextObjectImp::replicateObject(const GraphicObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   bool bSuccess = RectangleObjectImp::replicateObject(pObject);
   mUpdateTexture = true;
   mUpdateBoundingBox = false;

   return bSuccess;
}

bool TextObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   bool bSuccess = RectangleObjectImp::fromXml(pDocument, version);
   mUpdateTexture = true;
   mUpdateBoundingBox = false;

   return bSuccess;
}

const string& TextObjectImp::getObjectType() const
{
   static string type("TextObjectImp");
   return type;
}

bool TextObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "TextObject"))
   {
      return true;
   }

   return RectangleObjectImp::isKindOf(className);
}

void TextObjectImp::updateBoundingBox()
{
   // Get the width and height of the bounding box in screen pixels based on the scaled text image size
   int iWidth = 0;
   int iHeight = 0;

   string text = getText();
   if (text.empty() == false)
   {
      QString strMessage = QString::fromStdString(text);
      QFont scaledFont = getScaledFont();

      int iMaxSize = 0;
      glGetIntegerv(GL_MAX_TEXTURE_SIZE, &iMaxSize);

      int iAlignment = getTextAlignment();

      QFontMetrics ftMetrics(scaledFont);
      QRect boundingBox = ftMetrics.boundingRect(0, 0, iMaxSize, iMaxSize,
         iAlignment | Qt::TextWordWrap, strMessage);

      iWidth = boundingBox.width();
      iHeight = boundingBox.height();
   }

   // Get the current bounding box
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   // Use the lower left corner as the anchor and compute the data coordinate of the upper right corner
   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      // Compute the upper right coordinate
      PerspectiveView* pView = dynamic_cast<PerspectiveView*>(pLayer->getView());
      if (pView != NULL)
      {
         double zoomFactor = 100.0 / pView->getZoomPercentage();
         double xScale = zoomFactor / pLayer->getXScaleFactor();
         double yScale = zoomFactor / pLayer->getYScaleFactor();

         urCorner.mX = llCorner.mX + (iWidth * xScale);
         urCorner.mY = llCorner.mY + (iHeight * yScale);
      }

      if (dynamic_cast<OrthographicView*>(pLayer->getView()) != NULL)
      {
         double dScreenX = 0.0;
         double dScreenY = 0.0;
         pLayer->translateDataToScreen(llCorner.mX, llCorner.mY, dScreenX, dScreenY);
         pLayer->translateScreenToData(dScreenX + iWidth, dScreenY + iHeight, urCorner.mX, urCorner.mY);
      }
   }
   else
   {
      urCorner.mX = llCorner.mX + iWidth;
      urCorner.mY = llCorner.mY + iHeight;
   }

   // Update the bounding box and selection handles
   setBoundingBox(llCorner, urCorner);
   updateHandles();

   mUpdateBoundingBox = false;
}

bool TextObjectImp::processMousePress(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
                                      Qt::KeyboardModifiers modifiers)
{
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   VERIFY(pLayer != NULL);

   bool bValidText = false;
   if (edit() == true)
   {
      string text = getText();
      bValidText = !(text.empty());
   }

   mUpdateTexture = bValidText;
   mUpdateBoundingBox = bValidText;
   pLayer->completeInsertion(bValidText);

   return bValidText;
}

bool TextObjectImp::processMouseMove(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
                                     Qt::KeyboardModifiers modifiers)
{
   // should never get here
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   VERIFY(pLayer != NULL);
   pLayer->completeInsertion(false);

   VERIFY(false);
}

bool TextObjectImp::processMouseRelease(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
                                        Qt::KeyboardModifiers modifiers)
{
   // should never get here
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   VERIFY(pLayer != NULL);
   pLayer->completeInsertion(false);

   VERIFY(false);
}

void TextObjectImp::temporaryGlContextChange()
{
   mUpdateTexture = true;
   mUpdateBoundingBox = true;
   mTextureIdStack.push(mTextureId);
   mTextureId = 0;
}

bool TextObjectImp::edit()
{
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   VERIFY(pLayer != NULL);

   QWidget* pWidget = NULL;

   View* pView = pLayer->getView();
   if (pView != NULL)
   {
      pWidget = pView->getWidget();
   }

   MultiLineTextDialog dialog(pWidget);

   string text = getText();
   if (text.empty() == false)
   {
      dialog.setText(QString::fromStdString(text));
   }

   dialog.exec();

   text = dialog.getText().toStdString();
   if (text.empty() == false)
   {
      setText(text);
   }

   return true;
}

QFont TextObjectImp::getScaledFont(double minSize, double maxSize)
{
   QFont scaledFont = getFont();

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      // Scale the point size
      double pointSize = scaledFont.pointSizeF();

      PerspectiveView* pPerspectiveView = dynamic_cast<PerspectiveView*>(pLayer->getView());
      if (pPerspectiveView != NULL)
      {
         // Zoom percentage
         double zoomPercent = pPerspectiveView->getZoomPercentage();
         pointSize *= zoomPercent / 100.0;

         // Product DPI
         ProductView* pProductView = dynamic_cast<ProductView*>(pPerspectiveView);
         if (pProductView != NULL)
         {
            int dpi = pProductView->getDpi();
            pointSize *= dpi / 72.0;
         }
      }

      // Restrict to the minimum size
      if (minSize > 0.0)
      {
         pointSize = max(pointSize, minSize);
      }

      // Restrict to the maximum size
      if (maxSize > 0.0)
      {
         pointSize = min(pointSize, maxSize);
      }

      // Set the scaled point size in the font
      scaledFont.setPointSizeF(pointSize);
   }

   return scaledFont;
}

void TextObjectImp::moveHandle(int handle, LocationType point, bool bMaintainAspect)
{
   // Do nothing
}
