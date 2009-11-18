/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataDescriptor.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "DrawUtil.h"
#include "Endian.h"
#include "glCommon.h"
#include "GraphicElement.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "LayerList.h"
#include "MipMappedTextures.h"
#include "MultiLineTextDialog.h"
#include "OrthographicView.h"
#include "ProductView.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "TextObjectImp.h"
#include "TypeConverter.h"
#include "View.h"

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtOpenGL/QGLContext>

#include <math.h>
#include <string>
#include <vector>
using namespace std;

TextObjectImp::TextObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                             LocationType pixelCoord) :
   RectangleObjectImp(id, type, pLayer, pixelCoord),
   mpDrawContext(NULL),
   mTextureWidth(1),
   mTextureHeight(1),
   mDataWidth(0),
   mDataHeight(0),
   mUpdateTexture(true),
   mUpdateBoundingBox(true),
   mTextureResource(0),
   mTempTextureResource(0)
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
   for (std::map<DynamicObject*, AttachmentPtr<DynamicObject>* >::iterator it = mMetadataObjects.begin();
      it != mMetadataObjects.end(); ++it)
   {
      delete it->second;
   }
}

void TextObjectImp::draw(double zoomFactor) const
{
   RectangleObjectImp::draw(zoomFactor);
   if (QGLContext::currentContext() != mpDrawContext || mUpdateTexture == true)
   {
      (const_cast<TextObjectImp*> (this))->updateTexture();
   }

   if (mpDrawContext == NULL)
   {
      return;
   }

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   glEnable(GL_TEXTURE_2D);

   // switch to using this tile's texture
   GLuint textureId = mTextureResource;
   if (mpDrawContext == mTempTextureResource.getContext())
   {
      textureId = mTempTextureResource;
   }

   glBindTexture(GL_TEXTURE_2D, textureId);

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

   // mTempTextureResource is allocated by the updateTexture method when the gl context has changed
   // Free the texture here; more importantly set mpDrawContext to mTextureResource so that the next
   // draw operation completes successfully without the need to recreate the texture.
   if (mTempTextureResource.get() != NULL)
   {
      TextObjectImp* pNonConst = const_cast<TextObjectImp*>(this);
      pNonConst->mTempTextureResource = GlTextureResource(0);
      pNonConst->mpDrawContext = mTextureResource.getContext();
   }
}

void TextObjectImp::updateTexture()
{
   for (std::map<DynamicObject*, AttachmentPtr<DynamicObject>* >::iterator it = mMetadataObjects.begin();
      it != mMetadataObjects.end(); ++it)
   {
      delete it->second;
   }

   mMetadataObjects.clear();

   // Get the text to display
   string text = getSubstitutedText();
   if (text.empty())
   {
      return;
   }

   // Reallocate the existing texture or create a temporary texture if the context has changed.
   GLuint textureId;
   if (mTextureResource.get() == NULL || mTextureResource.getContext() == QGLContext::currentContext())
   {
      mTextureResource = GlTextureResource(1);
      textureId = mTextureResource;
      mpDrawContext = mTextureResource.getContext();
   }
   else
   {
      mTempTextureResource = GlTextureResource(1);
      textureId = mTempTextureResource;
      mpDrawContext = mTempTextureResource.getContext();
   }

   if (mpDrawContext == NULL || textureId == 0)
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
            if (Endian::getSystemEndian() == LITTLE_ENDIAN_ORDER)
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
   glBindTexture(GL_TEXTURE_2D, textureId);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

   string text = getSubstitutedText();
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

   if (bValidText == true)
   {
      // Update the texture here to ensure a correct bounding box when the undo action is added
      updateTexture();
   }

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

string TextObjectImp::getSubstitutedText()
{
   string txt = getText();

   DataElement* pParent = getElement();
   pParent = (pParent == NULL) ? NULL : pParent->getParent();
   DataDescriptor* pParentDesc = (pParent == NULL) ? NULL : pParent->getDataDescriptor();
   DynamicObject* pParentMetadata = (pParentDesc == NULL) ? NULL : pParentDesc->getMetadata();
   for (int i = 0; i < 50; ++i)
   {
      //each pass does replacement of $M(a) currently in the string.
      //do 50 passes to perform sub-expansion at most fifty times, ie. prevent infinite loop
      //for non-terminating recursive expansion
      string::size_type pos = txt.find("$");
      while (pos != string::npos)
      {
         if (pos + 1 >= txt.size())
         {
            break;
         }
         string type = txt.substr(pos+1, 1);
         if (type != "$") //ie. not $$, the escape sequence so continue
         {
            bool replaced = false;
            if (pos+4 < txt.size()) //ie. $M(a)
            {
               if (txt[pos+2] == '(')
               {
                  string::size_type closeParen = txt.find(')', pos+2);
                  if (closeParen == string::npos)
                  {
                     closeParen = txt.size();
                  }
                  string variableName = txt.substr(pos+3, closeParen-(pos+2)-1);
                  string replacementString;
                  if (type == "M")
                  {
                     DynamicObject* pMetadata = pParentMetadata;
                     if (variableName.substr(0, 2) == "//")
                     {
                        string::size_type endNamePos = variableName.find("//", 2);
                        if (endNamePos != string::npos)
                        {
                           string elementName = variableName.substr(2, endNamePos - 2);
                           variableName = variableName.substr(endNamePos + 2);
                           if (!variableName.empty())
                           {
                              if (elementName[0] == '[' && elementName[elementName.size() - 1] == ']')
                              {
                                 elementName = elementName.substr(1, elementName.size() - 2);
                                 std::list<GraphicObject*> objects;
                                 getLayer()->getObjects(VIEW_OBJECT, objects);
                                 for (std::list<GraphicObject*>::iterator object = objects.begin();
                                    object != objects.end(); ++object)
                                 {
                                    GraphicObject* pObj = *object;
                                    if (pObj->getName() == elementName)
                                    {
                                       SpatialDataView* pSdv = dynamic_cast<SpatialDataView*>(pObj->getObjectView());
                                       if (pSdv != NULL)
                                       {
                                          DataElement* pElmnt = pSdv->getLayerList()->getPrimaryRasterElement();
                                          DataDescriptor* pDesc =
                                             (pElmnt == NULL) ? NULL : pElmnt->getDataDescriptor();
                                          pMetadata = (pDesc == NULL) ? NULL : pDesc->getMetadata();
                                       }
                                       break;
                                    }
                                 }
                              }
                              else
                              {
                                 DataElement* pElmnt = Service<ModelServices>()->getElement(elementName,
                                    TypeConverter::toString<RasterElement>(), NULL);
                                 DataDescriptor* pDesc = (pElmnt == NULL) ? NULL : pElmnt->getDataDescriptor();
                                 pMetadata = (pDesc == NULL) ? NULL : pDesc->getMetadata();
                              }
                           }
                           else
                           {
                              pMetadata = NULL;
                           }
                        }
                     }
                     bool success = false;
                     if (pMetadata != NULL)
                     {
                        DataVariant var = pMetadata->getAttributeByPath(variableName);
                        if (var.isValid())
                        {
                           DataVariant::Status status;
                           replacementString = var.toDisplayString(&status);
                           success = (status == DataVariant::SUCCESS);
                           if (mMetadataObjects.find(pMetadata) == mMetadataObjects.end())
                           {
                              mMetadataObjects.insert(make_pair(pMetadata, new AttachmentPtr<DynamicObject>(
                                 pMetadata, SIGNAL_NAME(Subject, Modified),
                                 Slot(this, &TextObjectImp::invalidateTexture))));
                           }
                        }
                     }
                     if (!success)
                     {
                        replacementString = "Error!";
                     }
                     replaced = true;
                  }
                  if (replaced)
                  {
                     txt.replace(pos, closeParen-pos+1, replacementString);
                     pos = txt.find("$", pos+replacementString.size());
                  }
               }
            }
            if (!replaced)
            {
               pos = txt.find("$", pos+1);
            }
         }
         else
         {
            pos = txt.find("$", pos+2);
         }
      }
   }
   string::size_type pos = txt.find("$$");
   while (pos != string::npos)
   {
      txt.replace(pos, 2, "$");
      pos = txt.find("$$");
   }

   return txt;
}

void TextObjectImp::invalidateTexture(Subject& subject, const std::string& signal, const boost::any& v)
{
   mUpdateTexture = true;
   mUpdateBoundingBox = true;
   getLayer()->getView()->refresh();
}
