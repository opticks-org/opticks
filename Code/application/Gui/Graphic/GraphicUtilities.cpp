/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FontImp.h"
#include "GraphicObject.h"
#include "GraphicObjectImp.h"
#include "GraphicProperty.h"
#include "GraphicUtilities.h"

using namespace std;

bool GraphicUtilities::setProperty(const list<GraphicObject*>& objects, GraphicProperty* pProperty)
{
   if ((objects.empty() == true) || (pProperty == NULL))
   {
      return false;
   }

   bool bSuccess = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->setProperty(pProperty) == true)
         {
            bSuccess = true;
         }
      }
   }

   return bSuccess;
}

const GraphicProperty* GraphicUtilities::getProperty(const list<GraphicObject*>& objects, const string& propertyName)
{
   if (propertyName.empty() == true)
   {
      return NULL;
   }

   const GraphicProperty* pProperty = NULL;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         const GraphicProperty* pCurrentProperty = pObject->getProperty(propertyName);
         if (pProperty != NULL)
         {
            if ((pCurrentProperty == NULL) || (pCurrentProperty->compare(pProperty) == true))
            {
               continue;
            }
            else
            {
               return NULL;
            }
         }
         else
         {
            pProperty = pCurrentProperty;
         }
      }
   }

   return pProperty;
}

bool GraphicUtilities::setBoundingBox(const list<GraphicObject*>& objects, LocationType lowerLeft,
                                      LocationType upperRight)
{
   BoundingBoxProperty boundingBoxProperty(lowerLeft, upperRight);
   return setProperty(objects, &boundingBoxProperty);
}

LocationType GraphicUtilities::getLlCorner(const list<GraphicObject*>& objects)
{
   const BoundingBoxProperty* pProperty =
      dynamic_cast<const BoundingBoxProperty*>(getProperty(objects, "BoundingBox"));
   if (pProperty != NULL)
   {
      return pProperty->getLlCorner();
   }

   return LocationType();
}

LocationType GraphicUtilities::getUrCorner(const list<GraphicObject*>& objects)
{
   const BoundingBoxProperty* pProperty =
      dynamic_cast<const BoundingBoxProperty*>(getProperty(objects, "BoundingBox"));
   if (pProperty != NULL)
   {
      return pProperty->getUrCorner();
   }

   return LocationType();
}

bool GraphicUtilities::setRotation(const list<GraphicObject*>& objects, double angle)
{
   RotationProperty rotationProperty(angle);
   return setProperty(objects, &rotationProperty);
}

double GraphicUtilities::getRotation(const list<GraphicObject*>& objects)
{
   const RotationProperty* pProperty = dynamic_cast<const RotationProperty*>(getProperty(objects, "Rotation"));
   if (pProperty != NULL)
   {
      return pProperty->getRotation();
   }

   return -1.0;
}

bool GraphicUtilities::setAngles(const list<GraphicObject*>& objects, double dStart, double dStop)
{
   WedgeProperty wedgeProperty(dStart, dStop);
   return setProperty(objects, &wedgeProperty);
}

double GraphicUtilities::getStartAngle(const list<GraphicObject*>& objects)
{
   const WedgeProperty* pProperty = dynamic_cast<const WedgeProperty*>(getProperty(objects, "Wedge"));
   if (pProperty != NULL)
   {
      return pProperty->getStartAngle();
   }

   return -1.0;
}

double GraphicUtilities::getStopAngle(const list<GraphicObject*>& objects)
{
   const WedgeProperty* pProperty = dynamic_cast<const WedgeProperty*>(getProperty(objects, "Wedge"));
   if (pProperty != NULL)
   {
      return pProperty->getStopAngle();
   }

   return -1.0;
}

bool GraphicUtilities::setArcRegion(const list<GraphicObject*>& objects, ArcRegion eRegion)
{
   ArcRegionProperty arcRegionProperty(eRegion);
   return setProperty(objects, &arcRegionProperty);
}

ArcRegion GraphicUtilities::getArcRegion(const list<GraphicObject*>& objects)
{
   const ArcRegionProperty* pProperty = dynamic_cast<const ArcRegionProperty*>(getProperty(objects, "ArcRegion"));
   if (pProperty != NULL)
   {
      return pProperty->getRegion();
   }

   return ArcRegion();
}

bool GraphicUtilities::setFillState(const list<GraphicObject*>& objects, bool fillState)
{
   bool bSuccess = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         if (pObject->setFillState(fillState) == true)
         {
            bSuccess = true;
         }
      }
   }

   return bSuccess;
}

bool GraphicUtilities::getFillState(const list<GraphicObject*>& objects)
{
   bool bFill = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         bool bCurrentFill = pObject->getFillState();
         if (bCurrentFill == bFill)
         {
            continue;
         }

         if (bFill == true)
         {
            return false;
         }

         bFill = bCurrentFill;
      }
   }

   return bFill;
}

bool GraphicUtilities::setFillColor(const list<GraphicObject*>& objects, ColorType fillColor)
{
   FillColorProperty fillColorProperty(fillColor);
   return setProperty(objects, &fillColorProperty);
}

ColorType GraphicUtilities::getFillColor(const list<GraphicObject*>& objects)
{
   const FillColorProperty* pProperty = dynamic_cast<const FillColorProperty*>(getProperty(objects, "FillColor"));
   if (pProperty != NULL)
   {
      return pProperty->getColor();
   }

   return ColorType();
}

bool GraphicUtilities::setFillStyle(const list<GraphicObject*>& objects, FillStyle eStyle)
{
   FillStyleProperty fillStyleProperty(eStyle);
   return setProperty(objects, &fillStyleProperty);
}

FillStyle GraphicUtilities::getFillStyle(const list<GraphicObject*>& objects)
{
   const FillStyleProperty* pProperty = dynamic_cast<const FillStyleProperty*>(getProperty(objects, "FillStyle"));
   if (pProperty != NULL)
   {
      return pProperty->getFillStyle();
   }

   return FillStyle();
}

bool GraphicUtilities::setHatchStyle(const list<GraphicObject*>& objects, SymbolType eHatch)
{
   HatchStyleProperty hatchStyleProperty(eHatch);
   return setProperty(objects, &hatchStyleProperty);
}

SymbolType GraphicUtilities::getHatchStyle(const list<GraphicObject*>& objects)
{
   const HatchStyleProperty* pProperty = dynamic_cast<const HatchStyleProperty*>(getProperty(objects, "HatchStyle"));
   if (pProperty != NULL)
   {
      return pProperty->getHatchStyle();
   }

   return SymbolType();
}

bool GraphicUtilities::setImageFile(const list<GraphicObject*>& objects, const string& filename)
{
   FileNameProperty filenameProperty(filename);
   return setProperty(objects, &filenameProperty);
}

string GraphicUtilities::getImageFile(const list<GraphicObject*>& objects)
{
   const FileNameProperty* pProperty = dynamic_cast<const FileNameProperty*>(getProperty(objects, "Filename"));
   if (pProperty != NULL)
   {
      return pProperty->getFileName();
   }

   return string();
}

bool GraphicUtilities::setAlpha(const list<GraphicObject*>& objects, double alpha)
{
   AlphaProperty alphaProperty(alpha);
   return setProperty(objects, &alphaProperty);
}

double GraphicUtilities::getAlpha(const list<GraphicObject*>& objects)
{
   const AlphaProperty* pProperty = dynamic_cast<const AlphaProperty*>(getProperty(objects, "Alpha"));
   if (pProperty != NULL)
   {
      return pProperty->getAlpha();
   }

   return -1.0;
}

bool GraphicUtilities::setLineState(const list<GraphicObject*>& objects, bool lineState)
{
   LineOnProperty lineOnProperty(lineState);
   return setProperty(objects, &lineOnProperty);
}

bool GraphicUtilities::getLineState(const list<GraphicObject*>& objects)
{
   const LineOnProperty* pProperty = dynamic_cast<const LineOnProperty*>(getProperty(objects, "LineOn"));
   if (pProperty != NULL)
   {
      return pProperty->getState();
   }

   return false;
}

bool GraphicUtilities::setLineStyle(const list<GraphicObject*>& objects, LineStyle eStyle)
{
   LineStyleProperty lineStyleProperty(eStyle);
   return setProperty(objects, &lineStyleProperty);
}

LineStyle GraphicUtilities::getLineStyle(const list<GraphicObject*>& objects)
{
   const LineStyleProperty* pProperty = dynamic_cast<const LineStyleProperty*>(getProperty(objects, "LineStyle"));
   if (pProperty != NULL)
   {
      return pProperty->getStyle();
   }

   return LineStyle();
}

bool GraphicUtilities::setLineWidth(const list<GraphicObject*>& objects, double width)
{
   LineWidthProperty lineWidthProperty(width);
   return setProperty(objects, &lineWidthProperty);
}

double GraphicUtilities::getLineWidth(const list<GraphicObject*>& objects)
{
   const LineWidthProperty* pProperty = dynamic_cast<const LineWidthProperty*>(getProperty(objects, "LineWidth"));
   if (pProperty != NULL)
   {
      return pProperty->getWidth();
   }

   return -1.0;
}

bool GraphicUtilities::setLineColor(const list<GraphicObject*>& objects, ColorType lineColor)
{
   LineColorProperty lineColorProperty(lineColor);
   return setProperty(objects, &lineColorProperty);
}

ColorType GraphicUtilities::getLineColor(const list<GraphicObject*>& objects)
{
   const LineColorProperty* pProperty = dynamic_cast<const LineColorProperty*>(getProperty(objects, "LineColor"));
   if (pProperty != NULL)
   {
      return pProperty->getColor();
   }

   return ColorType();
}

bool GraphicUtilities::setLineScaled(const list<GraphicObject*>& objects, bool scaled)
{
   LineScaledProperty lineScaledProperty(scaled);
   return setProperty(objects, &lineScaledProperty);
}

bool GraphicUtilities::getLineScaled(const list<GraphicObject*>& objects)
{
   const LineScaledProperty* pProperty = dynamic_cast<const LineScaledProperty*>(getProperty(objects, "LineScaled"));
   if (pProperty != NULL)
   {
      return pProperty->getScaled();
   }

   return false;
}

bool GraphicUtilities::setScale(const list<GraphicObject*>& objects, double scale)
{
   ScaleProperty scaleProperty(scale);
   return setProperty(objects, &scaleProperty);
}

double GraphicUtilities::getScale(const list<GraphicObject*>& objects)
{
   const ScaleProperty* pProperty = dynamic_cast<const ScaleProperty*>(getProperty(objects, "Scale"));
   if (pProperty != NULL)
   {
      return pProperty->getScale();
   }

   return -1.0;
}

bool GraphicUtilities::setGraphicSymbol(const list<GraphicObject*>& objects, const string& symbolName)
{
   GraphicSymbolProperty symbolProperty(symbolName);
   return setProperty(objects, &symbolProperty);
}

string GraphicUtilities::getGraphicSymbol(const list<GraphicObject*>& objects)
{
   const GraphicSymbolProperty* pProperty =
      dynamic_cast<const GraphicSymbolProperty*>(getProperty(objects, "GraphicSymbol"));
   if (pProperty != NULL)
   {
      return pProperty->getSymbolName();
   }

   return string();
}

bool GraphicUtilities::setGraphicSymbolSize(const list<GraphicObject*>& objects, unsigned int size)
{
   GraphicSymbolSizeProperty symbolSizeProperty(size);
   return setProperty(objects, &symbolSizeProperty);
}

unsigned int GraphicUtilities::getGraphicSymbolSize(const list<GraphicObject*>& objects)
{
   const GraphicSymbolSizeProperty* pProperty =
      dynamic_cast<const GraphicSymbolSizeProperty*>(getProperty(objects, "GraphicSymbolSize"));
   if (pProperty != NULL)
   {
      return pProperty->getSymbolSize();
   }

   return 0;
}

bool GraphicUtilities::setText(const list<GraphicObject*>& objects, const string& text)
{
   TextStringProperty textProperty(text);
   return setProperty(objects, &textProperty);
}

string GraphicUtilities::getText(const list<GraphicObject*>& objects)
{
   const TextStringProperty* pProperty = dynamic_cast<const TextStringProperty*>(getProperty(objects, "TextString"));
   if (pProperty != NULL)
   {
      return pProperty->getString();
   }

   return string();
}

bool GraphicUtilities::setTextAlignment(const list<GraphicObject*>& objects, int iAlignment)
{
   TextAlignmentProperty textAlignProperty(iAlignment);
   return setProperty(objects, &textAlignProperty);
}

int GraphicUtilities::getTextAlignment(const list<GraphicObject*>& objects)
{
   const TextAlignmentProperty* pProperty =
      dynamic_cast<const TextAlignmentProperty*>(getProperty(objects, "TextAlignment"));
   if (pProperty != NULL)
   {
      return pProperty->getAlignment();
   }

   return -1;
}

bool GraphicUtilities::setFont(const list<GraphicObject*>& objects, const QFont& font)
{
   FontProperty fontProperty(font);
   return setProperty(objects, &fontProperty);
}

QFont GraphicUtilities::getFont(const list<GraphicObject*>& objects)
{
   const FontProperty* pProperty = dynamic_cast<const FontProperty*>(getProperty(objects, "Font"));
   if (pProperty != NULL)
   {
      return pProperty->getFont().toQFont();
   }

   return QFont();
}

bool GraphicUtilities::setFontName(const list<GraphicObject*>& objects, const string& fontName)
{
   bool bSuccess = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->hasProperty("Font") == true)
         {
            QFont objectFont = pObject->getFont();
            objectFont.setFamily(QString::fromStdString(fontName));
            if (pObject->setFont(objectFont) == true)
            {
               bSuccess = true;
            }
         }
      }
   }

   return bSuccess;
}

string GraphicUtilities::getFontName(const list<GraphicObject*>& objects)
{
   string fontName;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->hasProperty("Font") == false)
         {
            continue;
         }

         QFont currentFont = pObject->getFont();
         string currentFontName = currentFont.family().toStdString();
         if (currentFontName == fontName)
         {
            continue;
         }

         if (fontName.empty() == false)
         {
            return string();
         }

         fontName = currentFontName;
      }
   }

   return fontName;
}

bool GraphicUtilities::setFontSize(const list<GraphicObject*>& objects, int fontSize)
{
   bool bSuccess = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->hasProperty("Font") == true)
         {
            QFont objectFont = pObject->getFont();
            objectFont.setPointSize(fontSize);
            if (pObject->setFont(objectFont) == true)
            {
               bSuccess = true;
            }
         }
      }
   }

   return bSuccess;
}

int GraphicUtilities::getFontSize(const list<GraphicObject*>& objects)
{
   int fontSize = -1;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->hasProperty("Font") == false)
         {
            continue;
         }

         QFont currentFont = pObject->getFont();
         int currentFontSize = currentFont.pointSize();
         if (currentFontSize == fontSize)
         {
            continue;
         }

         if (fontSize != -1)
         {
            return -1;
         }

         fontSize = currentFontSize;
      }
   }

   return fontSize;
}

bool GraphicUtilities::setFontBold(const list<GraphicObject*>& objects, bool bBold)
{
   bool bSuccess = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->hasProperty("Font") == true)
         {
            QFont objectFont = pObject->getFont();
            objectFont.setBold(bBold);
            if (pObject->setFont(objectFont) == true)
            {
               bSuccess = true;
            }
         }
      }
   }

   return bSuccess;
}

bool GraphicUtilities::getFontBold(const list<GraphicObject*>& objects)
{
   bool bBold = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->hasProperty("Font") == false)
         {
            continue;
         }

         QFont currentFont = pObject->getFont();
         bool bCurrentBold = currentFont.bold();
         if (bCurrentBold == bBold)
         {
            continue;
         }

         if (bBold == true)
         {
            return false;
         }

         bBold = bCurrentBold;
      }
   }

   return bBold;
}

bool GraphicUtilities::setFontItalics(const list<GraphicObject*>& objects, bool bItalics)
{
   bool bSuccess = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->hasProperty("Font") == true)
         {
            QFont objectFont = pObject->getFont();
            objectFont.setItalic(bItalics);
            if (pObject->setFont(objectFont) == true)
            {
               bSuccess = true;
            }
         }
      }
   }

   return bSuccess;
}

bool GraphicUtilities::getFontItalics(const list<GraphicObject*>& objects)
{
   bool bItalics = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->hasProperty("Font") == false)
         {
            continue;
         }

         QFont currentFont = pObject->getFont();
         bool bCurrentItalics = currentFont.italic();
         if (bCurrentItalics == bItalics)
         {
            continue;
         }

         if (bItalics == true)
         {
            return false;
         }

         bItalics = bCurrentItalics;
      }
   }

   return bItalics;
}

bool GraphicUtilities::setFontUnderline(const list<GraphicObject*>& objects, bool bUnderline)
{
   bool bSuccess = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->hasProperty("Font") == true)
         {
            QFont objectFont = pObject->getFont();
            objectFont.setUnderline(bUnderline);
            if (pObject->setFont(objectFont) == true)
            {
               bSuccess = true;
            }
         }
      }
   }

   return bSuccess;
}

bool GraphicUtilities::getFontUnderline(const list<GraphicObject*>& objects)
{
   bool bUnderline = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->hasProperty("Font") == false)
         {
            continue;
         }

         QFont currentFont = pObject->getFont();
         bool bCurrentUnderline = currentFont.underline();
         if (bCurrentUnderline == bUnderline)
         {
            continue;
         }

         if (bUnderline == true)
         {
            return false;
         }

         bUnderline = bCurrentUnderline;
      }
   }

   return bUnderline;
}

bool GraphicUtilities::setTextColor(const list<GraphicObject*>& objects, ColorType textColor)
{
   TextColorProperty textColorProperty(textColor);
   return setProperty(objects, &textColorProperty);
}

ColorType GraphicUtilities::getTextColor(const list<GraphicObject*>& objects)
{
   const TextColorProperty* pProperty = dynamic_cast<const TextColorProperty*>(getProperty(objects, "TextColor"));
   if (pProperty != NULL)
   {
      return pProperty->getColor();
   }

   return ColorType();
}

bool GraphicUtilities::setApex(const list<GraphicObject*>& objects, double dApex)
{
   ApexProperty apexProperty(dApex);
   return setProperty(objects, &apexProperty);
}

double GraphicUtilities::getApex(const list<GraphicObject*>& objects)
{
   const ApexProperty* pProperty = dynamic_cast<const ApexProperty*>(getProperty(objects, "Apex"));
   if (pProperty != NULL)
   {
      return pProperty->getApex();
   }

   return -1.0;
}

bool GraphicUtilities::setObjectView(const list<GraphicObject*>& objects, View* pView)
{
   bool bSuccess = false;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         if (pObject->setObjectView(pView) == true)
         {
            bSuccess = true;
         }
      }
   }

   return bSuccess;
}

View* GraphicUtilities::getObjectView(const list<GraphicObject*>& objects)
{
   View* pView = NULL;
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         View* pCurrentView = pObject->getObjectView();
         if (pCurrentView == pView)
         {
            continue;
         }

         if (pView != NULL)
         {
            return NULL;
         }

         pView = pCurrentView;
      }
   }

   return pView;
}
