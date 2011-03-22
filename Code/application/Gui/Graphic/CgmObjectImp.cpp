/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>
#include <sys/stat.h>

#include "AppConfig.h"
#include "AppVerify.h"
#include "ArcObjectImp.h"
#include "CgmObject.h"
#include "CgmObjectImp.h"
#include "DrawUtil.h"
#include "Endian.h"
#include "FileResource.h"
#include "GraphicLayerImp.h"
#include "GraphicLayerUndo.h"
#include "LatLonInsertObjectImp.h"
#include "NorthArrowObjectImp.h"
#include "PolygonObjectImp.h"
#include "PolylineObjectImp.h"
#include "ScaleBarObjectImp.h"
#include "TextObjectImp.h"
#include "Undo.h"
#include "XercesIncludes.h"

#include <algorithm>
#include <sstream>

XERCES_CPP_NAMESPACE_USE
using namespace std;

CgmObjectImp::CgmObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
   GraphicGroupImp(id, type, pLayer, pixelCoord),
   msVersion(1),
   msColorSelectionMode(1),
   msEdgeWidthMode(0),
   msLineWidthMode(0),
   mpCgm(NULL)
{
   mElementList.push_back(1);
   mElementList.push_back(-1);
   mElementList.push_back(1);
}

CgmObjectImp::~CgmObjectImp()
{
   if (mpCgm != NULL)
   {
      delete [] mpCgm;
      mpCgm = NULL;
   }
}

void CgmObjectImp::setMetafileName(const string& metafileName)
{
   mMetafileName = "" + metafileName;
}

const string& CgmObjectImp::getMetafileName() const
{
   return mMetafileName;
}

void CgmObjectImp::setPictureName(const string& metafileName)
{
   mPictureName = "" + metafileName;
}

const string& CgmObjectImp::getPictureName() const
{
   return mPictureName;
}

void CgmObjectImp::setMetafileDescription(const string& metafileDescription)
{
   mDescription = "" + metafileDescription;
}

const string& CgmObjectImp::getMetafileDescription() const
{
   return mDescription;
}

void CgmObjectImp::setFontList(const vector<string>& fontList)
{
   mFontList = fontList;
}

void CgmObjectImp::getFontList(vector<string>& fontList) const
{
   fontList = mFontList;
}

short CgmObjectImp::getVersion() const
{
   return msVersion;
}

short CgmObjectImp::getColorSelectionMode() const
{
   return msColorSelectionMode;
}

short CgmObjectImp::getEdgeWidthMode() const
{
   return msEdgeWidthMode;
}

short CgmObjectImp::getLineWidthMode() const
{
   return msLineWidthMode;
}

void CgmObjectImp::destroy()
{
   delete this;
}

bool CgmObjectImp::replicateObject(const GraphicObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicGroupImp::replicateObject(pObject);
   if (bSuccess == false)
   {
      return false;
   }

   const CgmObjectImp* pCgm = dynamic_cast<const CgmObjectImp*>(pObject);
   VERIFY(pCgm != NULL);

   mMetafileName = "" + pCgm->mMetafileName;
   mPictureName = "" + pCgm->mPictureName;
   msVersion = pCgm->msVersion;
   mDescription = "" + pCgm->mDescription;
   mElementList = pCgm->mElementList;
   msColorSelectionMode = pCgm->msColorSelectionMode;
   msEdgeWidthMode = pCgm->msEdgeWidthMode;
   msLineWidthMode = pCgm->msLineWidthMode;

   unsigned int uiSize = pCgm->mFontList.size();
   for (unsigned int i = 0; i < uiSize; ++i)
   {
      string fontName = pCgm->mFontList.at(i);
      if (fontName.empty() == false)
      {
         mFontList.push_back(fontName);
      }
   }

   return true;
}

CgmObject* CgmObjectImp::convertToCgm()
{
   return dynamic_cast<CgmObject*>(this);
}

short* CgmObjectImp::toCgm(int& lBytes)
{
   lBytes = 0;
   makeCgm(lBytes, true);
   if (lBytes <= 0)
   {
      return NULL;
   }

   short* pData = NULL;
   pData = makeCgm(lBytes, false);

   return pData;
}

int CgmObjectImp::fromCgm(short* pData)
{
   if (pData == NULL)
   {
      return -1;
   }

   int lIndex = 0;
   short sElement = 0;
   short sElementClass = 0;
   short sElementID = 0;

   // Begin metafile
   sElement = readElement(pData, lIndex);
   if (sElement == BEGIN_METAFILE)
   {
      lIndex++;
   }

   mMetafileName = readStringElement(pData, lIndex);

   // Metafile version
   sElement = readElement(pData, lIndex);
   if (sElement != METAFILE_VERSION)
   {
      return -1;
   }

   msVersion = readElement(pData, lIndex);
   if (msVersion != 1)
   {
      return -1;
   }

   // Metafile element list
   sElement = readElement(pData, lIndex);
   if (sElement != METAFILE_ELEMENT_LIST)
   {
      return -1;
   }

   mElementList.clear();
   for (int i = 0; i < 3; ++i)
   {
      short sListItem = readElement(pData, lIndex);
      mElementList.push_back(sListItem);
   }

   // Metafile description
   sElement = readElement(pData, lIndex);
   if (sElement == METAFILE_DESCRIPTION)
   {
      lIndex++;
   }

   mDescription = readStringElement(pData, lIndex);

   // Font list
   sElement = readElement(pData, lIndex);
   sElementID = getElementID(sElement);

   if (sElementID == 13)
   {
      short sFontBytes = 0;
      if (sElement == FONT_LIST)
      {
         sFontBytes = readElement(pData, lIndex);
      }
      else
      {
         sFontBytes = getElementParameterLength(sElement);
      }

      mFontList.clear();

      char* pCharData = reinterpret_cast<char*>(&(pData[lIndex]));
      short sBytes = sFontBytes;
      while (sBytes > 0)
      {
         unsigned char ucLength = pCharData[0];
         pCharData++;

         sBytes -= sizeof(unsigned char);
         if (ucLength == 0)
         {
            break;
         }

         char* pFontName = new char[ucLength + 1];
         if (pFontName != NULL)
         {
            strncpy(pFontName, pCharData, ucLength);
            pFontName[ucLength] = '\0';

            mFontList.push_back(std::string(pFontName));
            delete [] pFontName;
         }

         sBytes -= ucLength;
         pCharData += ucLength;
      }

      if ((sFontBytes % 2) == 1)
      {
         sFontBytes++;
      }

      lIndex += (sFontBytes / 2);

      sElement = readElement(pData, lIndex);
      sElementID = getElementID(sElement);
   }

   // Begin picture
   if (sElementID != 3)
   {
      return -1;
   }

   if (sElement == BEGIN_PICTURE)
   {
      lIndex++;
   }

   mPictureName = readStringElement(pData, lIndex);

   // Color selection mode
   sElement = readElement(pData, lIndex);
   if (sElement != COLOR_SELECTION_MODE)
   {
      return -1;
   }

   msColorSelectionMode = readElement(pData, lIndex);
   if (msColorSelectionMode != 1)
   {
      return -1;
   }

   // Edge width mode
   sElement = readElement(pData, lIndex);
   if (sElement == EDGE_WIDTH_SPECIFICATION_MODE)
   {
      msEdgeWidthMode = readElement(pData, lIndex);
      if (msEdgeWidthMode != 0)
      {
         return -1;
      }

      sElement = readElement(pData, lIndex);
   }

   // Line width mode
   if (sElement == LINE_WIDTH_SPECIFICATION_MODE)
   {
      msLineWidthMode = readElement(pData, lIndex);
      if (msLineWidthMode != 0)
      {
         return -1;
      }

      sElement = readElement(pData, lIndex);
      if (sElement == EDGE_WIDTH_SPECIFICATION_MODE)
      {
         msEdgeWidthMode = readElement(pData, lIndex);
         if (msEdgeWidthMode != 0)
         {
            return -1;
         }

         sElement = readElement(pData, lIndex);
      }
   }

   // VDC extent
   if (sElement != VDC_EXTENT)
   {
      return -1;
   }

   short sLlObjectCornerX = readElement(pData, lIndex);
   short sLlObjectCornerY = readElement(pData, lIndex);
   short sUrObjectCornerX = readElement(pData, lIndex);
   short sUrObjectCornerY = readElement(pData, lIndex);

   LocationType llCorner(sLlObjectCornerX, sLlObjectCornerY);
   LocationType urCorner(sUrObjectCornerX, sUrObjectCornerY);

   int iVdcXOrientation = 1;
   int iVdcYOrientation = -1;

   if (llCorner.mX > urCorner.mX)
   {
      iVdcXOrientation = -1;
   }

   if (llCorner.mY > urCorner.mY)
   {
      iVdcYOrientation = 1;
   }

   setBoundingBox(llCorner, urCorner);

   // Begin picture body
   sElement = readElement(pData, lIndex);
   if (sElement != BEGIN_PICTURE_BODY)
   {
      return -1;
   }

   // Objects
   GraphicObject* pCurrentObject = NULL;
   ColorType textColor;
   QFont textFont;
   int iTextXOrientation = 1;
   int iTextYOrientation = 1;
   ColorType fillColor;
   FillStyle eFillStyle = EMPTY_FILL;
   SymbolType eHatchStyle = HORIZONTAL_LINE;
   bool bEdge = false;
   double dEdgeWidth = 0.0;
   LineStyle eEdgeStyle = SOLID_LINE;
   ColorType edgeColor;
   double dLineWidth = 0.0;
   LineStyle eLineStyle = SOLID_LINE;
   ColorType lineColor;

   View* pView = NULL;

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      pView = pLayer->getView();
   }

   sElement = readElement(pData, lIndex);
   while (sElement != END_PICTURE)
   {
      UndoLock lock(pView);

      sElementClass = getElementClass(sElement);
      sElementID = getElementID(sElement);

      if (sElementClass == 4)
      {
         bool bFilledObject = true;
         bool bOrigEdge = bEdge;
         double dOrigEdgeWidth = dEdgeWidth;
         LineStyle eOrigEdgeStyle = eEdgeStyle;
         ColorType origEdgeColor = edgeColor;
         FillStyle eOrigFillStyle = eFillStyle;

         switch (sElementID)
         {
            case 1: case 7:      // Polyline and polygon
            {
               short sLength = 0;
               if ((sElement == POLYLINE) || (sElement == POLYGON))
               {
                  sLength = readElement(pData, lIndex) / 4;
               }
               else
               {
                  sLength = getElementParameterLength(sElement) / 4;
               }

               vector<LocationType> segments;
               short sXCoord = readElement(pData, lIndex);
               short sYCoord = readElement(pData, lIndex);

               LocationType origPoint(sXCoord, sYCoord);
               LocationType startPoint = origPoint;

               segments.push_back(startPoint);

               GraphicObjectType eObjType = POLYLINE_OBJECT;
               if (sElementID == 7)
               {
                  eObjType = POLYGON_OBJECT;
               }

               pCurrentObject = addObject(eObjType, startPoint);
               if (pCurrentObject == NULL)
               {
                  return -1;
               }

               PolylineObjectImp* pPolyline = dynamic_cast<PolylineObjectImp*>(pCurrentObject);
               VERIFYRV(pPolyline != NULL, -1);
               for (short s = 1; s < sLength; ++s)
               {
                  sXCoord = readElement(pData, lIndex);
                  sYCoord = readElement(pData, lIndex);

                  LocationType endPoint(sXCoord, sYCoord);
                  if (startPoint != endPoint)
                  {
                     segments.push_back(endPoint);
                  }

                  startPoint = endPoint;
               }

               if (sElementID == 1)
               {
                  bFilledObject = false;
               }
               else if (sElementID == 7)
               {
                  if (origPoint != startPoint)
                  {
                     segments.push_back(origPoint);
                  }
               }

               pPolyline->addVertices(segments);
               pPolyline->setLineScaled(true);

               break;
            }

            case 4:      // Text
            {
               if (sElement == TEXT)
               {
                  lIndex++;
               }

               short sXCoord = 0;
               short sYCoord = 0;
               sXCoord = readElement(pData, lIndex);
               sYCoord = readElement(pData, lIndex);

               LocationType point(sXCoord, sYCoord);

               pCurrentObject = addObject(TEXT_OBJECT, point);
               TextObjectImp* pCurrentText = dynamic_cast<TextObjectImp*>(pCurrentObject);
               VERIFYRV(pCurrentText != NULL, -1);

               lIndex++;

               pCurrentText->setText("ABC");
               pCurrentText->setTextColor(textColor);
               pCurrentText->setFont(textFont);
               pCurrentText->updateTexture();

               // Ensure proper font scale
               LocationType textLlCorner = pCurrentObject->getLlCorner();
               LocationType textUrCorner = pCurrentObject->getUrCorner();

               double dHeight = textUrCorner.mY - textLlCorner.mY;
               double dWidth = textUrCorner.mX - textLlCorner.mX;
               double scale = dHeight/textFont.pointSize();

               pCurrentObject->setText(readStringElement(pData, lIndex));
               pCurrentText->updateTexture();

               // Ensure proper font scale
               textLlCorner = pCurrentObject->getLlCorner();
               textUrCorner = pCurrentObject->getUrCorner();

               dHeight = textUrCorner.mY - textLlCorner.mY;
               dWidth = textUrCorner.mX - textLlCorner.mX;
               dHeight /= scale;
               dWidth /= scale;

               if (iTextXOrientation == -1)
               {
                  textUrCorner.mX = textLlCorner.mX - dWidth;
               }
               else
               {
                  textUrCorner.mX = textLlCorner.mX + dWidth;
               }

               if (iTextYOrientation == -1)
               {
                  textUrCorner.mY = textLlCorner.mY - dHeight;
               }
               else
               {
                  textUrCorner.mY = textLlCorner.mY + dHeight;
               }

               // Adjust the bounding box for the VDC extent
               if ((iVdcXOrientation * iTextXOrientation) == -1)
               {
                  double dTemp = textLlCorner.mX;
                  textLlCorner.mX = textUrCorner.mX;
                  textUrCorner.mX = dTemp;
               }

               if ((iVdcYOrientation * iTextYOrientation) == -1)
               {
                  double dTemp = textLlCorner.mY;
                  textLlCorner.mY = textUrCorner.mY;
                  textUrCorner.mY = dTemp;
               }

               pCurrentObject->setBoundingBox(textLlCorner, textUrCorner);

               // CGM does not support text fill
               bFilledObject = false;
               break;
            }

            case 8:      // Polygon set
            {
               short sLength = 0;
               if (sElement == POLYGON_SET)
               {
                  sLength = readElement(pData, lIndex) / 6;
               }
               else
               {
                  sLength = getElementParameterLength(sElement) / 6;
               }

               pCurrentObject = NULL;

               LocationType origPoint;
               LocationType startPoint;
               short sXCoord = -1;
               short sYCoord = -1;
               short sEdgeFlag = -1;

               for (short s = 0; s < sLength; ++s)
               {
                  sXCoord = readElement(pData, lIndex);
                  sYCoord = readElement(pData, lIndex);
                  sEdgeFlag = readElement(pData, lIndex);

                  LocationType endPoint(sXCoord, sYCoord);

                  if (pCurrentObject == NULL)
                  {
                     pCurrentObject = addObject(POLYGON_OBJECT, endPoint);
                     if (pCurrentObject == NULL)
                     {
                        return -1;
                     }

                     origPoint = endPoint;
                  }
                  else
                  {
                     PolygonObjectImp* pPolygon = dynamic_cast<PolygonObjectImp*>(pCurrentObject);
                     VERIFYRV(pPolygon != NULL, -1);
                     if (startPoint != endPoint)
                     {
                        pPolygon->addVertex(endPoint);
                     }
                  }

                  startPoint = endPoint;

                  if ((sEdgeFlag == 2) || (sEdgeFlag == 3))
                  {
                     PolygonObjectImp* pPolygon = dynamic_cast<PolygonObjectImp*>(pCurrentObject);
                     VERIFYRV(pPolygon != NULL, -1);
                     if (origPoint != startPoint)
                     {
                        pPolygon->addVertex(origPoint);
                     }

                     bool bLineOn = false;
                     if (sEdgeFlag == 3)
                     {
                        bLineOn = true;
                     }

                     pPolygon->setLineState(bLineOn);

                     if (bLineOn == true)
                     {
                        pPolygon->setLineWidth(dEdgeWidth);
                        pPolygon->setLineStyle(eEdgeStyle);
                        pPolygon->setLineColor(edgeColor);
                     }

                     pPolygon->setFillColor(fillColor);
                     pPolygon->setFillStyle(eFillStyle);
                     pPolygon->setHatchStyle(eHatchStyle);

                     pPolygon->updateHandles();
                     updateBoundingBox();

                     pCurrentObject = NULL;
                  }
               }

               break;
            }

            case 11:   // Rectangle
            {
               pCurrentObject = addObject(RECTANGLE_OBJECT);
               if (pCurrentObject == NULL)
               {
                  return -1;
               }

               short sLlCornerX = readElement(pData, lIndex);
               short sLlCornerY = readElement(pData, lIndex);
               short sUrCornerX = readElement(pData, lIndex);
               short sUrCornerY = readElement(pData, lIndex);

               LocationType rectLlCorner(sLlCornerX, sLlCornerY);
               LocationType rectUrCorner(sUrCornerX, sUrCornerY);

               pCurrentObject->setBoundingBox(rectLlCorner, rectUrCorner);
               break;
            }

            case 12:   // Circle
            {
               pCurrentObject = addObject(ELLIPSE_OBJECT);
               if (pCurrentObject == NULL)
               {
                  return -1;
               }

               short sCenterX = readElement(pData, lIndex);
               short sCenterY = readElement(pData, lIndex);
               short sRadius = readElement(pData, lIndex);

               LocationType circleLlCorner;
               circleLlCorner.mX = static_cast<double>(sCenterX) - static_cast<double>(sRadius);
               circleLlCorner.mY = static_cast<double>(sCenterY) - static_cast<double>(sRadius);

               LocationType circleUrCorner;
               circleUrCorner.mX = static_cast<double>(sCenterX) + static_cast<double>(sRadius);
               circleUrCorner.mY = static_cast<double>(sCenterY) + static_cast<double>(sRadius);

               pCurrentObject->setBoundingBox(circleLlCorner, circleUrCorner);
               break;
            }

            case 15: case 16:   // Circle arc and circle arc close
            {
               pCurrentObject = addObject(ARC_OBJECT);
               if (pCurrentObject == NULL)
               {
                  return -1;
               }

               // Center
               short sCenterX = readElement(pData, lIndex);
               short sCenterY = readElement(pData, lIndex);

               // Start and stop points
               short sStartX = readElement(pData, lIndex);
               short sStartY = readElement(pData, lIndex);
               short sStopX = readElement(pData, lIndex);
               short sStopY = readElement(pData, lIndex);

               // Radius
               short sRadius = readElement(pData, lIndex);

               // Set the ellipse location
               LocationType ellipseLlCorner;
               ellipseLlCorner.mX = static_cast<double>(sCenterX - sRadius);
               ellipseLlCorner.mY = static_cast<double>(sCenterY - sRadius);

               LocationType ellipseUrCorner;
               ellipseUrCorner.mX = static_cast<double>(sCenterX + sRadius);
               ellipseUrCorner.mY = static_cast<double>(sCenterY + sRadius);

               ArcObjectImp* pCurrentArc = dynamic_cast<ArcObjectImp*>(pCurrentObject);
               VERIFYRV(pCurrentArc != NULL, -1);
               pCurrentArc->setEllipseCorners(ellipseLlCorner, ellipseUrCorner);

               // Set the start and stop angles
               double dStart = 0.0;
               double dStop = 0.0;
               dStart = atan2(static_cast<double>(sStartY), static_cast<double>(sStartX)) * 180.0 / PI;
               dStop = atan2(static_cast<double>(sStopY), static_cast<double>(sStopX)) * 180.0 / PI;

               pCurrentArc->setAngles(dStart, dStop);

               // Close type
               ArcRegion eClose = ARC_OPEN;
               if (sElementID == 16)
               {
                  short sCloseType = readElement(pData, lIndex);

                  if (sCloseType == 0)
                  {
                     eClose = ARC_CENTER;
                  }
                  else if (sCloseType == 1)
                  {
                     eClose = ARC_CHORD;
                  }
               }

               pCurrentObject->setArcRegion(eClose);

               // Ensure the properties are correctly set
               if (sElementID == 15)
               {
                  bEdge = true;
                  dEdgeWidth = dLineWidth;
                  eEdgeStyle = eLineStyle;
                  edgeColor = lineColor;
                  eFillStyle = EMPTY_FILL;
               }

               break;
            }

            case 17:   // Ellipse
            {
               pCurrentObject = addObject(ELLIPSE_OBJECT);
               if (pCurrentObject == NULL)
               {
                  return -1;
               }

               // Read the center and endpoints
               short sCenterX = readElement(pData, lIndex);
               short sCenterY = readElement(pData, lIndex);
               short sEndX1 = readElement(pData, lIndex);
               short sEndY1 = readElement(pData, lIndex);
               short sEndX2 = readElement(pData, lIndex);
               short sEndY2 = readElement(pData, lIndex);

               // Calculate the ellipse width and height
               double x1 = static_cast<double>(sEndX1 - sCenterX);
               double y1 = static_cast<double>(sEndY1 - sCenterY);
               double x2 = static_cast<double>(sEndX2 - sCenterX);
               double y2 = static_cast<double>(sEndY2 - sCenterY);

               double dC = 0.0;
               double dB = 0.0;
               double dA = 0.0;

               if (x1 == 0.0)
               {
                  dC = 1 / (y1 * y1);
                  dB = -2 * y2 / (x2 * y1 * y1);
                  dA = (1 - (dC * y2 * y2) - (dB * x2 * y2)) / (x2 * x2);
               }
               else if (x2 == 0.0)
               {
                  dC = 1 / (y2 * y2);
                  dB = -2 * y1 / (x1 * y2 * y2);
                  dA = (1 - (dC * y1 * y1) - (dB * x1 * y1)) / (x1 * x1);
               }
               else
               {
                  double m1 = y1 / x1;
                  double m2 = y2 / x2;

                  double dEta = (x1 * y1) - (x1 * x1 * m2);
                  double dGamma = (y2 * y2) - (x2 * y2 * m1);
                  double dBeta = (x2 * y2) - (x2 * x2 * m1);
                  double dAlpha = (y1 * y1) - (x1 * y1 * m2);

                  dC = (dEta - dBeta) / ((dEta * dGamma) - (dBeta * dAlpha));
                  dB = (2 * (1 - (dAlpha * dC))) / dEta;
                  dA = (1 - (dB * x1 * y1) - (dC * y1 * y1)) / (x1 * x1);
               }

               double dRotation = 0.0;
               if (dB != 0.0)
               {
                  dRotation = 0.5 * ((2 * atan(1.0)) - atan((dA - dC) / dB));   // Radians
               }

               double dWidth = sqrt(1 / ((dA * pow(cos(dRotation), 2)) + (dB * cos(dRotation) * sin(dRotation)) +
                  (dC * pow(sin(dRotation), 2))));
               double dHeight = sqrt(1 / ((dA * pow(sin(dRotation), 2)) - (dB * cos(dRotation) * sin(dRotation)) +
                  (dC * pow(cos(dRotation), 2))));

               // Set the ellipse location
               LocationType ellipseLlCorner;
               ellipseLlCorner.mX = static_cast<double>(sCenterX) - dWidth;
               ellipseLlCorner.mY = static_cast<double>(sCenterY) - dHeight;

               LocationType ellipseUrCorner;
               ellipseUrCorner.mX = static_cast<double>(sCenterX) + dWidth;
               ellipseUrCorner.mY = static_cast<double>(sCenterY) + dHeight;

               pCurrentObject->setBoundingBox(ellipseLlCorner, ellipseUrCorner);

               // Set the object rotation
               dRotation *= 180.0 / PI;      // Convert to degrees
               pCurrentObject->setRotation(dRotation);

               break;
            }

            case 18: case 19:   // Arc and arc close
            {
               pCurrentObject = addObject(ARC_OBJECT);
               if (pCurrentObject == NULL)
               {
                  return -1;
               }

               // Read the center and endpoints
               short sCenterX = readElement(pData, lIndex);
               short sCenterY = readElement(pData, lIndex);
               short sEndX1 = readElement(pData, lIndex);
               short sEndY1 = readElement(pData, lIndex);
               short sEndX2 = readElement(pData, lIndex);
               short sEndY2 = readElement(pData, lIndex);

               // Calculate the ellipse width and height
               double x1 = static_cast<double>(sEndX1 - sCenterX);
               double y1 = static_cast<double>(sEndY1 - sCenterY);
               double x2 = static_cast<double>(sEndX2 - sCenterX);
               double y2 = static_cast<double>(sEndY2 - sCenterY);

               double dC = 0.0;
               double dB = 0.0;
               double dA = 0.0;

               if (x1 == 0.0)
               {
                  dC = 1 / (y1 * y1);
                  dB = -2 * y2 / (x2 * y1 * y1);
                  dA = (1 - (dC * y2 * y2) - (dB * x2 * y2)) / (x2 * x2);
               }
               else if (x2 == 0.0)
               {
                  dC = 1 / (y2 * y2);
                  dB = -2 * y1 / (x1 * y2 * y2);
                  dA = (1 - (dC * y1 * y1) - (dB * x1 * y1)) / (x1 * x1);
               }
               else
               {
                  double m1 = y1 / x1;
                  double m2 = y2 / x2;

                  double dEta = (x1 * y1) - (x1 * x1 * m2);
                  double dGamma = (y2 * y2) - (x2 * y2 * m1);
                  double dBeta = (x2 * y2) - (x2 * x2 * m1);
                  double dAlpha = (y1 * y1) - (x1 * y1 * m2);

                  dC = (dEta - dBeta) / ((dEta * dGamma) - (dBeta * dAlpha));
                  dB = (2 * (1 - (dAlpha * dC))) / dEta;
                  dA = (1 - (dB * x1 * y1) - (dC * y1 * y1)) / (x1 * x1);
               }

               double dRotation = 0.0;
               if (dB != 0.0)
               {
                  dRotation = 0.5 * ((2 * atan(1.0)) - atan((dA - dC) / dB));   // Radians
               }

               double dWidth = sqrt(1 / ((dA * pow(cos(dRotation), 2)) + (dB * cos(dRotation) * sin(dRotation)) +
                  (dC * pow(sin(dRotation), 2))));
               double dHeight = sqrt(1 / ((dA * pow(sin(dRotation), 2)) - (dB * cos(dRotation) * sin(dRotation)) +
                  (dC * pow(cos(dRotation), 2))));

               // Set the ellipse location
               LocationType ellipseLlCorner;
               ellipseLlCorner.mX = static_cast<double>(sCenterX) - dWidth;
               ellipseLlCorner.mY = static_cast<double>(sCenterY) - dHeight;

               LocationType ellipseUrCorner;
               ellipseUrCorner.mX = static_cast<double>(sCenterX) + dWidth;
               ellipseUrCorner.mY = static_cast<double>(sCenterY) + dHeight;

               ArcObjectImp* pCurrentArc = dynamic_cast<ArcObjectImp*>(pCurrentObject);
               VERIFYRV(pCurrentArc != NULL, -1);
               pCurrentArc->setEllipseCorners(ellipseLlCorner, ellipseUrCorner);

               // Set the object rotation
               dRotation *= 180.0 / PI;      // Convert to degrees
               while (dRotation < 0.0)
               {
                  dRotation += 360.0;
               }

               pCurrentObject->setRotation(dRotation);

               // Start and stop points
               short sStartX = readElement(pData, lIndex);
               short sStartY = readElement(pData, lIndex);
               short sStopX = readElement(pData, lIndex);
               short sStopY = readElement(pData, lIndex);

               double dStartAngle = 0.0;
               double dStopAngle = 0.0;
               double dStartDisplacement = sqrt(pow(static_cast<double>(sStartX), 2) +
                  pow(static_cast<double>(sStartY), 2));
               double dStopDisplacement = sqrt(pow(static_cast<double>(sStopX), 2) +
                  pow(static_cast<double>(sStopY), 2));

               dStartAngle = atan2(static_cast<double>(sStartY), static_cast<double>(sStartX)) * 180.0 / PI;
               dStopAngle = atan2(static_cast<double>(sStopY), static_cast<double>(sStopX)) * 180.0 / PI;

               dStartAngle -= dRotation;
               dStopAngle -= dRotation;

               LocationType startPoint;
               startPoint.mX = static_cast<double>(sCenterX) + (dStartDisplacement * cos(dStartAngle * PI / 180.0));
               startPoint.mY = static_cast<double>(sCenterY) + (dStartDisplacement * sin(dStartAngle * PI / 180.0));

               LocationType stopPoint;
               stopPoint.mX = static_cast<double>(sCenterX) + (dStopDisplacement * cos(dStopAngle * PI / 180.0));
               stopPoint.mY = static_cast<double>(sCenterY) + (dStopDisplacement * sin(dStopAngle * PI / 180.0));

               double dStart = pCurrentArc->getAngle(startPoint);
               double dStop = pCurrentArc->getAngle(stopPoint);

               double dPrimaryRotation = 0.0;
               dPrimaryRotation = atan2(y1, x1) * 180.0 / PI;
               while (dPrimaryRotation < 0.0)
               {
                  dPrimaryRotation += 360.0;
               }

               double dSecondaryRotation = 0.0;
               dSecondaryRotation = atan2(y2, x2) * 180.0 / PI;
               while (dSecondaryRotation < 0.0)
               {
                  dSecondaryRotation += 360.0;
               }

               double dRotationDifference = 0.0;
               dRotationDifference = dSecondaryRotation - dPrimaryRotation;
               while (dRotationDifference < 0.0)
               {
                  dRotationDifference += 360.0;
               }

               if (dRotationDifference > 180.0)
               {
                  double dTemp = dStart;
                  dStart = dStop;
                  dStop = dTemp;
               }

               pCurrentArc->setAngles(dStart, dStop);

               // Adjust the arc location to account for rotation around the ellipse center
               LocationType arcLlCorner = pCurrentObject->getLlCorner();
               LocationType arcUrCorner = pCurrentObject->getUrCorner();

               LocationType arcCenter;
               arcCenter.mX = (arcLlCorner.mX + arcUrCorner.mX) / 2;
               arcCenter.mY = (arcLlCorner.mY + arcUrCorner.mY) / 2;

               double dRadius = sqrt(pow(arcCenter.mX - static_cast<double>(sCenterX), 2) +
                  pow(arcCenter.mY - static_cast<double>(sCenterY), 2));
               double dTheta = dRotation + atan2(arcCenter.mY - static_cast<double>(sCenterY),
                  arcCenter.mX - static_cast<double>(sCenterX)) * 180.0 / PI;

               LocationType newArcCenter;
               newArcCenter.mX = static_cast<double>(sCenterX) + (dRadius * cos(dTheta * PI / 180.0));
               newArcCenter.mY = static_cast<double>(sCenterY) + (dRadius * sin(dTheta * PI / 180.0));

               LocationType translation;
               translation.mX = newArcCenter.mX - arcCenter.mX;
               translation.mY = newArcCenter.mY - arcCenter.mY;

               pCurrentArc->move(translation);

               // Close type
               ArcRegion eClose = ARC_OPEN;
               if (sElementID == 19)
               {
                  short sCloseType = readElement(pData, lIndex);

                  if (sCloseType == 0)
                  {
                     eClose = ARC_CENTER;
                  }
                  else if (sCloseType == 1)
                  {
                     eClose = ARC_CHORD;
                  }
               }

               pCurrentObject->setArcRegion(eClose);

               // Ensure the properties are correctly set
               if (sElementID == 18)
               {
                  bEdge = true;
                  dEdgeWidth = dLineWidth;
                  eEdgeStyle = eLineStyle;
                  edgeColor = lineColor;
                  eFillStyle = EMPTY_FILL;
               }

               break;
            }

            default:
               break;
         }

         if (pCurrentObject != NULL)
         {
            if (bFilledObject == true)
            {
               pCurrentObject->setFillColor(fillColor);
               pCurrentObject->setFillStyle(eFillStyle);
               pCurrentObject->setHatchStyle(eHatchStyle);
               pCurrentObject->setLineState(bEdge);

               if (bEdge == true)
               {
                  pCurrentObject->setLineWidth(dEdgeWidth);
                  pCurrentObject->setLineColor(edgeColor);

                  if (eEdgeStyle != -1)
                  {
                     pCurrentObject->setLineStyle(eEdgeStyle);
                  }
                  else if (eLineStyle != -1)
                  {
                     pCurrentObject->setLineStyle(eLineStyle);
                  }
               }
            }
            else
            {
               pCurrentObject->setLineWidth(dLineWidth);
               pCurrentObject->setLineStyle(eLineStyle);
               pCurrentObject->setLineColor(lineColor);
            }

            dynamic_cast<GraphicObjectImp*>(pCurrentObject)->updateHandles();
            updateBoundingBox();
         }

         // Reset the edge values to their original values
         bEdge = bOrigEdge;
         dEdgeWidth = dOrigEdgeWidth;
         eEdgeStyle = eOrigEdgeStyle;
         edgeColor = origEdgeColor;
         eFillStyle = eOrigFillStyle;
      }
      else if (sElementClass == 5)
      {
         switch (sElement)
         {
            case TEXT_COLOR:
            {
               unsigned char ucRed = 0;
               unsigned char ucGreen = 0;
               unsigned char ucBlue = 0;

               unsigned char* pTextData = reinterpret_cast<unsigned char*>(&(pData[lIndex]));
               if (pTextData != NULL)
               {
                  ucRed = pTextData[0];
                  ucGreen = pTextData[1];
                  ucBlue = pTextData[2];
               }

               lIndex += 2;

               textColor.mRed = static_cast<int>(ucRed);
               textColor.mGreen = static_cast<int>(ucGreen);
               textColor.mBlue = static_cast<int>(ucBlue);
               break;
            }

            case CHARACTER_HEIGHT:
            {
               short sFontSize = readElement(pData, lIndex);
               textFont.setPointSize(sFontSize);
               break;
            }

            case TEXT_FONT_INDEX:
            {
               char *pCgmFonts[] = 
               {
                  "TIMES_ROMAN",
                  "TIMES_ITALIC",
                  "TIMES_BOLD",
                  "TIMES_BOLD_ITALIC",
                  "HELVETICA",
                  "HELVETICA_OBLIQUE",
                  "HELVETICA_BOLD",
                  "HELVETICA_BOLD_OBLIQUE",
                  "COURIER",
                  "COURIER_BOLD",
                  "COURIER_ITALIC",
                  "COURIER_BOLD_ITALIC"/*,
                  "HERSHEY/CARTOGRAPHIC_ROMAN",
                  "HERSHEY/CARTOGRAPHIC_GREEK",
                  "HERSHEY/SIMPLEX_ROMAN",
                  "HERSHEY/SIMPLEX_GREEK",
                  "HERSHEY/SIMPLEX_SCRIPT",
                  "HERSHEY/COMPLEX_ROMAN",
                  "HERSHEY/COMPLEX_GREEK",
                  "HERSHEY/COMPLEX_SCRIPT",
                  "HERSHEY/COMPLEX_ITALIC",
                  "HERSHEY/COMPLEX_CYRILLIC",
                  "HERSHEY/DUPLEX_ROMAN",
                  "HERSHEY/TRIPLEX_ROMAN",
                  "HERSHEY/TRIPLEX_ITALIC",
                  "HERSHEY/GOTHIC_GERMAN",
                  "HERSHEY/GOTHIC_ENGLISH",
                  "HERSHEY/GOTHIC_ITALIAN"*/
               };
#if defined(WIN_API)
               char *pReplacementFonts[] = 
               {
                  "Times New Roman",
                  "Times New Roman",
                  "Times New Roman",
                  "Times New Roman",
                  "Arial",
                  "Arial",
                  "Arial",
                  "Arial",
                  "Courier New",
                  "Courier New",
                  "Courier New",
                  "Courier New",
               };
#else
               char *pReplacementFonts[] = 
               {
                  "Times Roman",
                  "Times Roman",
                  "Times Roman",
                  "Times Roman",
                  "Helvetica",
                  "Helvetica",
                  "Helvetica",
                  "Helvetica",
                  "Courier",
                  "Courier",
                  "Courier",
                  "Courier",
               };
#endif
               bool isFontBold[] = { false, false, true, true, false, false, true, true, false, true, false, true };
               bool isFontItalic[] = { false, true, false, true, false, true, false, true, false, false, true, true };
               const int fontCount = sizeof(pCgmFonts)/sizeof(pCgmFonts[0]);
               short sFontIndex = readElement(pData, lIndex);
               bool isBold = false;
               bool isItalic = false;

               string fontName = "";

               int iFonts = mFontList.size();
               if (sFontIndex <= iFonts)
               {
                  fontName = mFontList.at(sFontIndex - 1);

                  for (int i = 0; i < fontCount; ++i)
                  {
                     if (fontName == pCgmFonts[i])
                     {
                        fontName = pReplacementFonts[i];
                        isBold = isFontBold[i];
                        isItalic = isFontItalic[i];
                        break;
                     }
                  }

                  QString strFont;
                  if (fontName.empty() == false)
                  {
                     strFont = QString::fromStdString(fontName);
                  }
               }

               if (fontName.empty() == false)
               {
                  textFont.setFamily(QString::fromStdString(fontName));
                  textFont.setBold(isBold);
                  textFont.setItalic(isItalic);
               }

               break;
            }

            case CHARACTER_ORIENTATION:
            {
               lIndex++;
               short sYValue = readElement(pData, lIndex);
               short sXValue = readElement(pData, lIndex);
               lIndex++;

               iTextXOrientation = sXValue;
               iTextYOrientation = sYValue;
               break;
            }

            case FILL_COLOR:
            {
               unsigned char ucRed = 0;
               unsigned char ucGreen = 0;
               unsigned char ucBlue = 0;

               unsigned char* pFillData = reinterpret_cast<unsigned char*>(&(pData[lIndex]));
               if (pFillData != NULL)
               {
                  ucRed = pFillData[0];
                  ucGreen = pFillData[1];
                  ucBlue = pFillData[2];
               }

               lIndex += 2;

               fillColor.mRed = static_cast<int>(ucRed);
               fillColor.mGreen = static_cast<int>(ucGreen);
               fillColor.mBlue = static_cast<int>(ucBlue);
               break;
            }

            case INTERIOR_STYLE:
            {
               short sFill = readElement(pData, lIndex);
               if (sFill == 1)
               {
                  eFillStyle = SOLID_FILL;
               }
               else if (sFill == 3)
               {
                  eFillStyle = HATCH;
               }
               else if (sFill == 4)
               {
                  eFillStyle = EMPTY_FILL;
               }
               else
               {
                  eFillStyle = EMPTY_FILL;
               }

               break;
            }

            case HATCH_INDEX:
            {
               short sHatch = readElement(pData, lIndex);
               switch (sHatch)
               {
                  case 1:
                     eHatchStyle = HORIZONTAL_LINE;
                     break;

                  case 2:
                     eHatchStyle = VERTICAL_LINE;
                     break;

                  case 3:
                     eHatchStyle = FORWARD_SLASH;
                     break;

                  case 4:
                     eHatchStyle = BACK_SLASH;
                     break;

                  case 5:
                     eHatchStyle = CROSS_HAIR;
                     break;

                  case 6:
                     eHatchStyle = X;
                     break;

                  default:
                     eHatchStyle = HORIZONTAL_LINE;
                     break;
               }

               break;
            }

            case EDGE_VISIBILITY:
            {
               short sEdge = readElement(pData, lIndex);
               if (sEdge == 1)
               {
                  bEdge = true;
               }
               else
               {
                  bEdge = false;
               }

               break;
            }

            case EDGE_WIDTH:
            {
               short sWidth = readElement(pData, lIndex);
               dEdgeWidth = sWidth;
               break;
            }

            case EDGE_TYPE:
            {
               short sEdge = readElement(pData, lIndex);
               switch (sEdge)
               {
                  case 1:
                     eEdgeStyle = SOLID_LINE;
                     break;

                  case 2:
                     eEdgeStyle = DASHED;
                     break;

                  case 3:
                     eEdgeStyle = DOT;
                     break;

                  case 4:
                     eEdgeStyle = DASH_DOT;
                     break;

                  case 5:
                     eEdgeStyle = DASH_DOT_DOT;
                     break;

                  default:
                     eEdgeStyle = SOLID_LINE;
                     break;
               }

               break;
            }

            case EDGE_COLOR:
            {
               unsigned char ucRed = 0;
               unsigned char ucGreen = 0;
               unsigned char ucBlue = 0;

               unsigned char* pEdgeData = reinterpret_cast<unsigned char*>(&(pData[lIndex]));
               if (pEdgeData != NULL)
               {
                  ucRed = pEdgeData[0];
                  ucGreen = pEdgeData[1];
                  ucBlue = pEdgeData[2];
               }

               lIndex += 2;

               edgeColor.mRed = static_cast<int>(ucRed);
               edgeColor.mGreen = static_cast<int>(ucGreen);
               edgeColor.mBlue = static_cast<int>(ucBlue);
               break;
            }

            case LINE_WIDTH:
            {
               short sWidth = readElement(pData, lIndex);
               dLineWidth = sWidth;
               break;
            }

            case LINE_TYPE:
            {
               short sLine = readElement(pData, lIndex);
               switch (sLine)
               {
                  case 1:
                     eLineStyle = SOLID_LINE;
                     break;

                  case 2:
                     eLineStyle = DASHED;
                     break;

                  case 3:
                     eLineStyle = DOT;
                     break;

                  case 4:
                     eLineStyle = DASH_DOT;
                     break;

                  case 5:
                     eLineStyle = DASH_DOT_DOT;
                     break;

                  default:
                     eLineStyle = SOLID_LINE;
                     break;
               }

               break;
            }

            case LINE_COLOR:
            {
               unsigned char ucRed = 0;
               unsigned char ucGreen = 0;
               unsigned char ucBlue = 0;

               unsigned char* pLineData = reinterpret_cast<unsigned char*>(&(pData[lIndex]));
               if (pLineData != NULL)
               {
                  ucRed = pLineData[0];
                  ucGreen = pLineData[1];
                  ucBlue = pLineData[2];
               }

               lIndex += 2;

               lineColor.mRed = static_cast<int>(ucRed);
               lineColor.mGreen = static_cast<int>(ucGreen);
               lineColor.mBlue = static_cast<int>(ucBlue);
               break;
            }

            default:
               break;
         }
      }
      else
      {
         return -1;
      }

      sElement = readElement(pData, lIndex);
   }

   // Add the undo actions
   if (pView != NULL)
   {
      for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
      {
         GraphicObject* pObject = *iter;
         if (pObject != NULL)
         {
            pView->addUndoAction(new AddGraphicObject(dynamic_cast<GraphicGroup*>(this), pObject));
         }
      }
   }

   // Update the bounding box and rotation based on the VDC extent
   llCorner = getLlCorner();
   urCorner = getUrCorner();

   if (iVdcXOrientation == -1)
   {
      llCorner.mX *= -1;
      urCorner.mX *= -1;

      for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
      {
         GraphicObject* pObject = *iter;
         if (pObject != NULL)
         {
            double dRotation = pObject->getRotation();
            dRotation *= -1.0;

            pObject->setRotation(dRotation);
         }
      }
   }

   if (iVdcYOrientation == -1)
   {
      llCorner.mY *= -1;
      urCorner.mY *= -1;

      for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
      {
         GraphicObject* pObject = *iter;
         if (pObject != NULL)
         {
            double dRotation = pObject->getRotation();
            dRotation *= -1.0;

            pObject->setRotation(dRotation);
         }
      }
   }

   setBoundingBox(llCorner, urCorner);

   // End metafile
   sElement = readElement(pData, lIndex);
   if (sElement != END_METAFILE)
   {
      return -1;
   }

   return lIndex * 2;
}

bool CgmObjectImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicGroupImp::toXml(pXml);
   if (bSuccess == false)
   {
      return false;
   }

   pXml->pushAddPoint(pXml->addElement("Cgm"));
   pXml->addAttr("metafile", mMetafileName);
   pXml->addAttr("picture", mPictureName);
   pXml->addAttr("description", mDescription);

   stringstream buf;
   buf << msVersion;
   pXml->addAttr("version", buf.str());

   buf.str("");
   buf << msColorSelectionMode;
   pXml->addAttr("colorSelectionMode", buf.str());

   buf.str("");
   buf << msEdgeWidthMode;
   pXml->addAttr("edgeWidthMode", buf.str());

   buf.str("");
   buf << msLineWidthMode;
   pXml->addAttr("lineWidthMode", buf.str());

   for (vector<short>::const_iterator elementIt = mElementList.begin(); elementIt != mElementList.end(); ++elementIt)
   {
      pXml->pushAddPoint(pXml->addElement("element"));
      buf.str("");
      buf << *elementIt;
      pXml->addText(buf.str());
      pXml->popAddPoint();
   }

   for (vector<string>::const_iterator fontIt = mFontList.begin(); fontIt != mFontList.end(); ++fontIt)
   {
      pXml->pushAddPoint(pXml->addElement("font"));
      pXml->addText(*fontIt);
      pXml->popAddPoint();
   }

   pXml->popAddPoint();
   pXml->popAddPoint();

   return true;
}

bool CgmObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (!GraphicGroupImp::fromXml(pDocument, version))
   {
      return false;
   }

   for (DOMNode* pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("Cgm")))
      {
         DOMElement* elmnt(static_cast<DOMElement*>(pChld));
         mMetafileName = A(elmnt->getAttribute(X("metafile")));
         mPictureName = A(elmnt->getAttribute(X("picture")));
         mDescription = A(elmnt->getAttribute(X("description")));
         msVersion = atoi(A(elmnt->getAttribute(X("version"))));
         msColorSelectionMode = atoi(A(elmnt->getAttribute(X("colorSelectionMode"))));
         msEdgeWidthMode = atoi(A(elmnt->getAttribute(X("edgeWidthMode"))));
         msLineWidthMode = atoi(A(elmnt->getAttribute(X("lineWidthMode"))));

         mElementList.clear();
         mFontList.clear();
         for (DOMNode* pGchld = pChld->getFirstChild(); pGchld != NULL; pGchld = pGchld->getNextSibling())
         {
            if (XMLString::equals(pGchld->getNodeName(), X("element")))
            {
               mElementList.push_back(atoi(A(pGchld->getTextContent())));
            }
            else if (XMLString::equals(pGchld->getNodeName(), X("font")))
            {
               mFontList.push_back(string(A(pGchld->getTextContent())));
            }
         }
      }
   }

   return true;
}

bool CgmObjectImp::serializeCgm(const string& fileName)
{
   FileResource pFile(fileName.c_str(), "wb");
   if (pFile.get() == NULL)
   {
      return false;
   }

   int lBytes = 0;

   short* pData = NULL;
   pData = toCgm(lBytes);
   if ((pData == NULL) || (lBytes <= 0))
   {
      return false;
   }

   unsigned int uiSize = 0;
   uiSize = fwrite(pData, lBytes, 1, pFile);
   if (uiSize != 1)
   {
      return false;
   }

   return true;
}

bool CgmObjectImp::deserializeCgm(const string& fileName)
{
   FileResource pFile(fileName.c_str(), "rb");
   if (pFile.get() == NULL)
   {
      return false;
   }

   int iHandle = 0;
   iHandle = FILENO(pFile);

   struct stat fileStats;

   int iSuccess = 0;
   iSuccess = fstat(iHandle, &fileStats);
   if (iSuccess != 0)
   {
      return false;
   }

   int lBytes = 0;
   lBytes = fileStats.st_size;
   if (lBytes <= 0)
   {
      return false;
   }

   if ((lBytes % 2) == 1)
   {
      lBytes++;
   }

   short* pData = NULL;
   pData = new short[lBytes / 2];
   if (pData == NULL)
   {
      return false;
   }

   unsigned int uiSize = 0;
   uiSize = fread(pData, lBytes, 1, pFile);
   if (uiSize != 1)
   {
      delete [] pData;
      return false;
   }

   int lReadBytes = 0;
   lReadBytes = fromCgm(pData);

   delete [] pData;

   if (lReadBytes != lBytes)
   {
      return false;
   }

   return true;
}

const string& CgmObjectImp::getObjectType() const
{
   static string type("CgmObjectImp");
   return type;
}

bool CgmObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "CgmObject"))
   {
      return true;
   }

   return GraphicGroupImp::isKindOf(className);
}

short* CgmObjectImp::makeCgm(int& lBytes, bool bCountOnly)
{
   if (bCountOnly == false)
   {
      if (mpCgm != NULL)
      {
         delete [] mpCgm;
         mpCgm = NULL;
      }

      if ((lBytes % 2) == 1)
      {
         lBytes++;
      }

      mpCgm = new short[lBytes / 2];
      if (mpCgm == NULL)
      {
         return NULL;
      }
   }

   int lIndex = 0;

   // Begin metafile
   writeElement(BEGIN_METAFILE, mpCgm, lIndex, bCountOnly);

   unsigned char ucLength = static_cast<unsigned char>(mMetafileName.length());

   writeElement(static_cast<short>(ucLength + 1), mpCgm, lIndex, bCountOnly);
   writeStringElement(mMetafileName.data(), mpCgm, lIndex, bCountOnly);

   // Metafile version
   writeElement(METAFILE_VERSION, mpCgm, lIndex, bCountOnly);
   writeElement(msVersion, mpCgm, lIndex, bCountOnly);

   // Metafile element list
   writeElement(METAFILE_ELEMENT_LIST, mpCgm, lIndex, bCountOnly);

   int iListCount = mElementList.size();
   for (int i = 0; i < iListCount; ++i)
   {
      short sListItem = mElementList.at(i);
      writeElement(sListItem, mpCgm, lIndex, bCountOnly);
   }

   // Metafile description
   writeElement(METAFILE_DESCRIPTION, mpCgm, lIndex, bCountOnly);

   ucLength = static_cast<unsigned char>(mDescription.length());

   writeElement(static_cast<short>(ucLength + 1), mpCgm, lIndex, bCountOnly);
   writeStringElement(mDescription.data(), mpCgm, lIndex, bCountOnly);

   // Font list
   bool bText = false;

   list<GraphicObject*>::iterator iter;
   for (iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      GraphicObjectImp* pGroupObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pGroupObject != NULL)
      {
         bText = pGroupObject->hasProperty("TextString");
      }
   }

   if (bText == true)
   {
      writeElement(FONT_LIST, mpCgm, lIndex, bCountOnly);

      short sFontCount = mFontList.size();

      bool bDefaultFont = false;
      if (sFontCount == 0)
      {
         mFontList.push_back("HELVETICA");
         bDefaultFont = true;
         sFontCount++;
      }

      short s = 0;
      short sListLength = 0;
      for (s = 0; s < sFontCount; ++s)
      {
         string fontName = mFontList.at(s);
         if (fontName.empty() == false)
         {
            int iSize = fontName.size();
            sListLength += static_cast<short>(iSize) + 1;
         }
      }

      writeElement(sListLength, mpCgm, lIndex, bCountOnly);

      if ((bCountOnly == false) && (mpCgm != NULL))
      {
         char* pCharData = reinterpret_cast<char*>(&(mpCgm[lIndex]));
         for (s = 0; s < sFontCount; ++s)
         {
            string fontName = mFontList.at(s);
            if (fontName.empty() == false)
            {
               ucLength = static_cast<unsigned char>(fontName.length());

               pCharData[0] = ucLength;
               pCharData++;

               strcpy(pCharData, fontName.data());
               pCharData += ucLength;

               if ((s == (sFontCount - 1)) && ((sListLength % 2) == 1))
               {
                  pCharData[ucLength] = 0;
               }
            }
         }
      }

      if ((sListLength % 2) == 1)
      {
         sListLength++;
      }

      lIndex += sListLength / 2;

      if (bDefaultFont == true)
      {
         mFontList.clear();
      }
   }

   // Begin picture
   writeElement(BEGIN_PICTURE, mpCgm, lIndex, bCountOnly);

   ucLength = static_cast<unsigned char>(mPictureName.length());

   writeElement(static_cast<short>(ucLength + 1), mpCgm, lIndex, bCountOnly);
   writeStringElement(mPictureName.data(), mpCgm, lIndex, bCountOnly);

   // Color selection mode
   writeElement(COLOR_SELECTION_MODE, mpCgm, lIndex, bCountOnly);
   writeElement(msColorSelectionMode, mpCgm, lIndex, bCountOnly);

   // Edge width mode
   writeElement(EDGE_WIDTH_SPECIFICATION_MODE, mpCgm, lIndex, bCountOnly);
   writeElement(msEdgeWidthMode, mpCgm, lIndex, bCountOnly);

   // Line width mode
   writeElement(LINE_WIDTH_SPECIFICATION_MODE, mpCgm, lIndex, bCountOnly);
   writeElement(msLineWidthMode, mpCgm, lIndex, bCountOnly);

   // VDC extent
   writeElement(VDC_EXTENT, mpCgm, lIndex, bCountOnly);

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   if (DrawUtil::sign(llCorner.mX) == -1.0)
   {
      llCorner.mX -= 1.0;
   }

   if (DrawUtil::sign(llCorner.mY) == -1.0)
   {
      llCorner.mY -= 1.0;
   }

   if (DrawUtil::sign(urCorner.mX) == 1.0)
   {
      urCorner.mX += 1.0;
   }

   if (DrawUtil::sign(urCorner.mY) == 1.0)
   {
      urCorner.mY += 1.0;
   }

   // Swap the y-values since CGM expects object origin in upper left
   short sLlCornerX = static_cast<short>(llCorner.mX);
   short sLlCornerY = static_cast<short>(urCorner.mY);
   short sUrCornerX = static_cast<short>(urCorner.mX);
   short sUrCornerY = static_cast<short>(llCorner.mY);

   writeElement(sLlCornerX, mpCgm, lIndex, bCountOnly);
   writeElement(sLlCornerY, mpCgm, lIndex, bCountOnly);
   writeElement(sUrCornerX, mpCgm, lIndex, bCountOnly);
   writeElement(sUrCornerY, mpCgm, lIndex, bCountOnly);

   // Begin picture body
   writeElement(BEGIN_PICTURE_BODY, mpCgm, lIndex, bCountOnly);

   // Objects
   for (iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      GraphicObject* pObject = NULL;
      pObject = *iter;
      if (pObject != NULL)
      {
         bool bSuccess = false;
         bSuccess = serializeCgmProperties(pObject, mpCgm, lIndex, bCountOnly);
         if (bSuccess == false)
         {
            return NULL;
         }

         bSuccess = serializeCgmObject(pObject, mpCgm, lIndex, bCountOnly);
         if (bSuccess == false)
         {
            return NULL;
         }
      }
   }

   // End picture
   writeElement(END_PICTURE, mpCgm, lIndex, bCountOnly);

   // End metafile
   writeElement(END_METAFILE, mpCgm, lIndex, bCountOnly);

   lBytes = lIndex * 2;
   return mpCgm;
}

short CgmObjectImp::readElement(short* pData, int& lByteIndex)
{
   if (pData == NULL)
   {
      return -1;
   }

   short sElement = -1;
   sElement = pData[lByteIndex];

   if (Endian::getSystemEndian() == LITTLE_ENDIAN_ORDER)
   {
      // Swap the bytes
      unsigned char* buffer = reinterpret_cast<unsigned char*>(&sElement);
      unsigned char byte1 = buffer[0];
      buffer[0] = buffer[1];
      buffer[1] = byte1;
   }

   lByteIndex++;
   return sElement;
}

string CgmObjectImp::readStringElement(short* pData, int& lByteIndex)
{
   if (pData == NULL)
   {
      return NULL;
   }

   char* pStringData = reinterpret_cast<char*>(&(pData[lByteIndex]));

   unsigned char ucLength = pStringData[0];
   pStringData++;

   string text(pStringData, ucLength);

   if ((ucLength % 2) == 0)
   {
      ucLength++;
   }

   lByteIndex += 1 + (ucLength / 2);
   return text;
}

bool CgmObjectImp::writeElement(short sElement, short* pData, int& lByteIndex, bool bCountOnly)
{
   if ((bCountOnly == false) && (pData != NULL))
   {
      if (Endian::getSystemEndian() == LITTLE_ENDIAN_ORDER)
      {
         // Swap the bytes
         unsigned char* buffer = reinterpret_cast<unsigned char*>(&sElement);
         unsigned char byte1 = buffer[0];
         buffer[0] = buffer[1];
         buffer[1] = byte1;
      }

      pData[lByteIndex] = sElement;
   }

   lByteIndex++;
   return true;
}

bool CgmObjectImp::writeStringElement(const char* pText, short* pData, int& lByteIndex, bool bCountOnly)
{
   if (pText == NULL)
   {
      return false;
   }

   unsigned char ucLength = strlen(pText);

   if ((bCountOnly == false) && (pData != NULL))
   {
      char* pStringData = reinterpret_cast<char*>(&(pData[lByteIndex]));

      pStringData[0] = ucLength;
      pStringData++;

      strcpy(pStringData, pText);

      if ((ucLength % 2) == 0)
      {
         pStringData[ucLength] = 0;
      }
   }

   if ((ucLength % 2) == 0)
   {
      ucLength++;
   }

   lByteIndex += 1 + (ucLength / 2);
   return true;
}

short CgmObjectImp::getElementClass(short sElement)
{
   unsigned short usElement = sElement;
   usElement >>= 12;

   return usElement;
}

short CgmObjectImp::getElementID(short sElement)
{
   unsigned short usElement = sElement;
   usElement <<= 4;
   usElement >>= 9;

   return usElement;
}

short CgmObjectImp::getElementParameterLength(short sElement)
{
   unsigned short usElement = sElement;
   usElement <<= 11;
   usElement >>= 11;

   return usElement;
}

bool CgmObjectImp::serializeCgmProperties(const GraphicObject* pObject, short* pData, int& lByteIndex, bool bCountOnly)
{
   if (pObject == NULL)
   {
      return false;
   }

   vector<GraphicProperty*> properties = dynamic_cast<const GraphicObjectImp*>(pObject)->getProperties();
   
   short fontSize = 21; // standard default CGM font size
   vector<GraphicProperty*>::iterator iter;
   for (iter = properties.begin(); iter != properties.end(); ++iter)
   {
      GraphicProperty* pProperty = *iter;
      if (pProperty != NULL)
      {
         string propertyName = pProperty->getName();
         if (propertyName == "BoundingBox")
         {
            BoundingBoxProperty* pBoxProperty = static_cast<BoundingBoxProperty*>(pProperty);
            LocationType llCorner = pBoxProperty->getLlCorner();
            LocationType urCorner = pBoxProperty->getUrCorner();
            fontSize = static_cast<short>(std::abs(llCorner.mY - urCorner.mY));
         }
         else if (propertyName == "LineColor")
         {
            GraphicObjectType eType = pObject->getGraphicObjectType();
            bool bFilled = pObject->getFillState();

            if ((eType == LINE_OBJECT) || (eType == ARROW_OBJECT) || (eType == POLYLINE_OBJECT) ||
               ((eType == ARC_OBJECT) && (bFilled == false)) ||
               ((eType == ROUNDEDRECTANGLE_OBJECT) && (bFilled == false)))
            {
               writeElement(LINE_COLOR, pData, lByteIndex, bCountOnly);
            }
            else
            {
               writeElement(EDGE_COLOR, pData, lByteIndex, bCountOnly);
            }

            LineColorProperty* pLineProperty = static_cast<LineColorProperty*>(pProperty);
            ColorType lineColor = pLineProperty->getColor();

            unsigned char ucRed = static_cast<unsigned char>(lineColor.mRed);
            unsigned char ucGreen = static_cast<unsigned char>(lineColor.mGreen);
            unsigned char ucBlue = static_cast<unsigned char>(lineColor.mBlue);

            if (pData != NULL)
            {
               unsigned char* pLineData = reinterpret_cast<unsigned char*>(&(pData[lByteIndex]));
               if (pLineData != NULL)
               {
                  pLineData[0] = ucRed;
                  pLineData[1] = ucGreen;
                  pLineData[2] = ucBlue;
                  pLineData[3] = 0;
               }
            }

            lByteIndex += 2;
         }
         else if (propertyName == "LineWidth")
         {
            GraphicObjectType eType = pObject->getGraphicObjectType();
            bool bFilled = pObject->getFillState();

            if ((eType == LINE_OBJECT) || (eType == ARROW_OBJECT) || (eType == POLYLINE_OBJECT) ||
               ((eType == ARC_OBJECT) && (bFilled == false)) ||
               ((eType == ROUNDEDRECTANGLE_OBJECT) && (bFilled == false)))
            {
               writeElement(LINE_WIDTH, pData, lByteIndex, bCountOnly);
            }
            else
            {
               writeElement(EDGE_WIDTH, pData, lByteIndex, bCountOnly);
            }

            LineWidthProperty* pLineProperty = static_cast<LineWidthProperty*>(pProperty);
            double dWidth = pLineProperty->getWidth();
            short sWidth = static_cast<short>(dWidth);

            writeElement(sWidth, pData, lByteIndex, bCountOnly);
         }
         else if (propertyName == "FillColor")
         {
            writeElement(FILL_COLOR, pData, lByteIndex, bCountOnly);

            FillColorProperty* pFillProperty = static_cast<FillColorProperty*>(pProperty);
            ColorType fillColor = pFillProperty->getColor();

            unsigned char ucRed = static_cast<unsigned char>(fillColor.mRed);
            unsigned char ucGreen = static_cast<unsigned char>(fillColor.mGreen);
            unsigned char ucBlue = static_cast<unsigned char>(fillColor.mBlue);

            if (pData != NULL)
            {
               unsigned char* pFillData = reinterpret_cast<unsigned char*>(&(pData[lByteIndex]));
               if (pFillData != NULL)
               {
                  pFillData[0] = ucRed;
                  pFillData[1] = ucGreen;
                  pFillData[2] = ucBlue;
                  pFillData[3] = 0;
               }
            }

            lByteIndex += 2;
         }
         else if (propertyName == "TextColor")
         {
            writeElement(TEXT_COLOR, pData, lByteIndex, bCountOnly);

            TextColorProperty* pTextProperty = static_cast<TextColorProperty*>(pProperty);
            ColorType textColor = pTextProperty->getColor();

            unsigned char ucRed = static_cast<unsigned char>(textColor.mRed);
            unsigned char ucGreen = static_cast<unsigned char>(textColor.mGreen);
            unsigned char ucBlue = static_cast<unsigned char>(textColor.mBlue);

            if (pData != NULL)
            {
               unsigned char* pTextData = reinterpret_cast<unsigned char*>(&(pData[lByteIndex]));
               if (pTextData != NULL)
               {
                  pTextData[0] = ucRed;
                  pTextData[1] = ucGreen;
                  pTextData[2] = ucBlue;
                  pTextData[3] = 0;
               }
            }

            lByteIndex += 2;
         }
         else if (propertyName == "LineOn")
         {
            GraphicObjectType eType = pObject->getGraphicObjectType();
            bool bFilled = pObject->getFillState();

            bool bLineObject = false;
            if ((eType == LINE_OBJECT) || (eType == ARROW_OBJECT) || (eType == POLYLINE_OBJECT) ||
               ((eType == ARC_OBJECT) && (bFilled == false)) ||
               ((eType == ROUNDEDRECTANGLE_OBJECT) && (bFilled == false)))
            {
               bLineObject = true;
            }

            if (bLineObject == false)
            {
               LineOnProperty* pLineProperty = static_cast<LineOnProperty*>(pProperty);
               bool bState = pLineProperty->getState();

               short sState = 0;
               if (bState == true)
               {
                  sState = 1;
               }

               writeElement(EDGE_VISIBILITY, pData, lByteIndex, bCountOnly);
               writeElement(sState, pData, lByteIndex, bCountOnly);
            }
         }
         else if (propertyName == "Font")
         {
            FontProperty* pFontProperty = static_cast<FontProperty*>(pProperty);
            QFont textFont = pFontProperty->getFont().toQFont();

            // Character height
            short sHeight = fontSize;

            writeElement(CHARACTER_HEIGHT, pData, lByteIndex, bCountOnly);
            writeElement(sHeight, pData, lByteIndex, bCountOnly);

            // Text font index
            string fontName = "";

            QString strFont = textFont.family();
            if (strFont.isEmpty() == false)
            {
               fontName = strFont.toStdString();
            }

            writeElement(TEXT_FONT_INDEX, pData, lByteIndex, bCountOnly);

            int i = 0;
            int iFonts = mFontList.size();
            for (i = 0; i < iFonts; ++i)
            {
               string currentFont = mFontList.at(i);
               if (currentFont == fontName)
               {
                  short sIndex = static_cast<short>(i + 1);
                  writeElement(sIndex, pData, lByteIndex, bCountOnly);
                  break;
               }
            }

            if (i == iFonts)
            {
               writeElement(1, pData, lByteIndex, bCountOnly);
            }

            // Character orientation
            writeElement(CHARACTER_ORIENTATION, pData, lByteIndex, bCountOnly);

            LocationType llCorner = pObject->getLlCorner();
            LocationType urCorner = pObject->getUrCorner();

            short sXValue = 1;
            short sYValue = 1;

            if (llCorner.mX > urCorner.mX)
            {
               sXValue = -1;
            }

            if (llCorner.mY > urCorner.mY)
            {
               sYValue = -1;
            }

            writeElement(0, pData, lByteIndex, bCountOnly);
            writeElement(sYValue, pData, lByteIndex, bCountOnly);
            writeElement(sXValue, pData, lByteIndex, bCountOnly);
            writeElement(0, pData, lByteIndex, bCountOnly);
         }
         else if (propertyName == "LineStyle")
         {
            GraphicObjectType eType = pObject->getGraphicObjectType();
            bool bFilled = pObject->getFillState();

            if ((eType == LINE_OBJECT) || (eType == ARROW_OBJECT) || (eType == POLYLINE_OBJECT) ||
               ((eType == ARC_OBJECT) && (bFilled == false)) ||
               ((eType == ROUNDEDRECTANGLE_OBJECT) && (bFilled == false)))
            {
               writeElement(LINE_TYPE, pData, lByteIndex, bCountOnly);
            }
            else
            {
               writeElement(EDGE_TYPE, pData, lByteIndex, bCountOnly);
            }

            LineStyleProperty* pLineProperty = static_cast<LineStyleProperty*>(pProperty);
            LineStyle eStyle = pLineProperty->getStyle();

            short sStyle = 1;
            if (eStyle == DASHED)
            {
               sStyle = 2;
            }
            else if (eStyle == DOT)
            {
               sStyle = 3;
            }
            else if (eStyle == DASH_DOT)
            {
               sStyle = 4;
            }
            else if (eStyle == DASH_DOT_DOT)
            {
               sStyle = 5;
            }

            writeElement(sStyle, pData, lByteIndex, bCountOnly);
         }
         else if (propertyName == "FillStyle")
         {
            FillStyleProperty* pFillProperty = static_cast<FillStyleProperty*>(pProperty);
            FillStyle eFill = pFillProperty->getFillStyle();

            // Fill style
            short sFill = 1;

            if (eFill == HATCH)
            {
               sFill = 3;
            }
            else if (eFill == EMPTY_FILL)
            {
               sFill = 4;
            }

            writeElement(INTERIOR_STYLE, pData, lByteIndex, bCountOnly);
            writeElement(sFill, pData, lByteIndex, bCountOnly);
         }
         else if (propertyName == "HatchStyle")
         {
            HatchStyleProperty* pHatchProperty = static_cast<HatchStyleProperty*>(pProperty);
            SymbolType eHatch = pHatchProperty->getHatchStyle();

            // Hatch style
            FillStyle eFill = pObject->getFillStyle();
            if (eFill == HATCH)
            {
               short sHatch = 1;

               if (eHatch == VERTICAL_LINE)
               {
                  sHatch = 2;
               }
               else if (eHatch == FORWARD_SLASH)
               {
                  sHatch = 3;
               }
               else if (eHatch == BACK_SLASH)
               {
                  sHatch = 4;
               }
               else if (eHatch == CROSS_HAIR)
               {
                  sHatch = 5;
               }
               else if (eHatch == X)
               {
                  sHatch = 6;
               }

               writeElement(HATCH_INDEX, pData, lByteIndex, bCountOnly);
               writeElement(sHatch, pData, lByteIndex, bCountOnly);
            }
         }
      }
   }

   return true;
}

bool CgmObjectImp::serializeCgmObject(const GraphicObject* pObject, short* pData, int& lByteIndex, bool bCountOnly)
{
   if (pObject == NULL)
   {
      return false;
   }

   LocationType llCorner = pObject->getLlCorner();
   LocationType urCorner = pObject->getUrCorner();

   LocationType center;
   center.mX = (urCorner.mX + llCorner.mX) / 2;
   center.mY = (urCorner.mY + llCorner.mY) / 2;

   double dRotation = pObject->getRotation();

   GraphicObjectType eType = pObject->getGraphicObjectType();
   switch (eType)
   {
      case ARC_OBJECT:
      {
         ArcRegion eRegion = pObject->getArcRegion();

         // Element
         if (eRegion == ARC_OPEN)
         {
            writeElement(ELLIPTICAL_ARC, pData, lByteIndex, bCountOnly);
         }
         else
         {
            writeElement(ELLIPTICAL_ARC_CLOSE, pData, lByteIndex, bCountOnly);
         }

         // Center
         const ArcObjectImp* pArcObject = dynamic_cast<const ArcObjectImp*>(pObject);
         VERIFY(pObject != NULL);
         LocationType ellipseCenter = pArcObject->getCenter();

         double dCenterDisplacement = sqrt(pow(ellipseCenter.mX - center.mX, 2) + pow(ellipseCenter.mY - center.mY, 2));
         double dTheta = dRotation + atan2(ellipseCenter.mY - center.mY, ellipseCenter.mX - center.mX) * 180.0 / PI;

         LocationType newEllipseCenter;
         newEllipseCenter.mX = center.mX + (dCenterDisplacement * cos(dTheta * PI / 180.0));
         newEllipseCenter.mY = center.mY + (dCenterDisplacement * sin(dTheta * PI / 180.0));

         writeElement(static_cast<short>(newEllipseCenter.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newEllipseCenter.mY), pData, lByteIndex, bCountOnly);

         // Endpoints
         LocationType endPoint1 = pArcObject->getLocation(0.0);
         LocationType endPoint2 = pArcObject->getLocation(90.0);

         double dFirstDisplacement = sqrt(pow(endPoint1.mX - center.mX, 2) + pow(endPoint1.mY - center.mY, 2));
         double dSecondDisplacement = sqrt(pow(endPoint2.mX - center.mX, 2) + pow(endPoint2.mY - center.mY, 2));
         double dFirstTheta = dRotation + atan2(endPoint1.mY - center.mY, endPoint1.mX - center.mX) * 180.0 / PI;
         double dSecondTheta = dRotation + atan2(endPoint2.mY - center.mY, endPoint2.mX - center.mX) * 180.0 / PI;

         LocationType newEndPoint1;
         newEndPoint1.mX = center.mX + (dFirstDisplacement * cos(dFirstTheta * PI / 180.0));
         newEndPoint1.mY = center.mY + (dFirstDisplacement * sin(dFirstTheta * PI / 180.0));

         LocationType newEndPoint2;
         newEndPoint2.mX = center.mX + (dSecondDisplacement * cos(dSecondTheta * PI / 180.0));
         newEndPoint2.mY = center.mY + (dSecondDisplacement * sin(dSecondTheta * PI / 180.0));

         writeElement(static_cast<short>(newEndPoint1.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newEndPoint1.mY), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newEndPoint2.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newEndPoint2.mY), pData, lByteIndex, bCountOnly);

         // Start and stop points
         double dStart = pObject->getStartAngle();
         double dStop = pObject->getStopAngle();

         LocationType startPoint = pArcObject->getLocation(dStart);
         LocationType stopPoint = pArcObject->getLocation(dStop);

         double dStartDisplacement = sqrt(pow(startPoint.mX - center.mX, 2) + pow(startPoint.mY - center.mY, 2));
         double dStopDisplacement = sqrt(pow(stopPoint.mX - center.mX, 2) + pow(stopPoint.mY - center.mY, 2));
         double dStartTheta = dRotation + atan2(startPoint.mY - center.mY, startPoint.mX - center.mX) * 180.0 / PI;
         double dStopTheta = dRotation + atan2(stopPoint.mY - center.mY, stopPoint.mX - center.mX) * 180.0 / PI;

         LocationType newStartPoint;
         newStartPoint.mX = center.mX + (dStartDisplacement * cos(dStartTheta * PI / 180.0));
         newStartPoint.mY = center.mY + (dStartDisplacement * sin(dStartTheta * PI / 180.0));

         LocationType newStopPoint;
         newStopPoint.mX = center.mX + (dStopDisplacement * cos(dStopTheta * PI / 180.0));
         newStopPoint.mY = center.mY + (dStopDisplacement * sin(dStopTheta * PI / 180.0));

         writeElement(static_cast<short>(newStartPoint.mX - newEllipseCenter.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newStartPoint.mY - newEllipseCenter.mY), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newStopPoint.mX - newEllipseCenter.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newStopPoint.mY - newEllipseCenter.mY), pData, lByteIndex, bCountOnly);

         if (eRegion != ARC_OPEN)
         {
            short sCloseType = 0;
            if (eRegion == ARC_CHORD)
            {
               sCloseType = 1;
            }

            writeElement(sCloseType, pData, lByteIndex, bCountOnly);
         }

         break;
      }

      case ARROW_OBJECT:
      {
         // Main line
         double dLlDisplacement = sqrt(pow(center.mX - llCorner.mX, 2) + pow(center.mY - llCorner.mY, 2));
         double dLlTheta = dRotation + atan2(center.mY - llCorner.mY, center.mX - llCorner.mX) * 180.0 / PI;
         double dUrDisplacement = sqrt(pow(urCorner.mX - center.mX, 2) + pow(urCorner.mY - center.mY, 2));
         double dUrTheta = dRotation + atan2(urCorner.mY - center.mY, urCorner.mX - center.mX) * 180.0 / PI;

         LocationType newLlCorner;
         newLlCorner.mX = center.mX - (dLlDisplacement * cos(dLlTheta * PI / 180.0));
         newLlCorner.mY = center.mY - (dLlDisplacement * sin(dLlTheta * PI / 180.0));

         LocationType newUrCorner;
         newUrCorner.mX = center.mX + (dUrDisplacement * cos(dUrTheta * PI / 180.0));
         newUrCorner.mY = center.mY + (dUrDisplacement * sin(dUrTheta * PI / 180.0));

         writeElement(POLYLINE, pData, lByteIndex, bCountOnly);
         writeElement(8, pData, lByteIndex, bCountOnly);      // Two nodes * four
         writeElement(static_cast<short>(newLlCorner.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newLlCorner.mY), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newUrCorner.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newUrCorner.mY), pData, lByteIndex, bCountOnly);

         // Arrow tips
         double arrowHeadSize = 6;
         double lineWidth = pObject->getLineWidth();
         double scaleFactor = pObject->getScale();

         double h = arrowHeadSize * sqrt(lineWidth) * scaleFactor;
         double theta = atan2(urCorner.mY - llCorner.mY, urCorner.mX - llCorner.mX);
         double hcTheta = h * cos(theta);
         double hsTheta = h * sin(theta);

         LocationType endPoint1;
         endPoint1.mX = urCorner.mX - hcTheta - hsTheta;
         endPoint1.mY = urCorner.mY - hsTheta + hcTheta;

         LocationType endPoint2;
         endPoint2.mX = urCorner.mX - hcTheta + hsTheta;
         endPoint2.mY = urCorner.mY - hsTheta - hcTheta;

         double dEnd1Displacement = sqrt(pow(endPoint1.mX - center.mX, 2) + pow(endPoint1.mY - center.mY, 2));
         double dEnd1Theta = dRotation + atan2(endPoint1.mY - center.mY, endPoint1.mX - center.mX) * 180.0 / PI;
         double dEnd2Displacement = sqrt(pow(endPoint2.mX - center.mX, 2) + pow(endPoint2.mY - center.mY, 2));
         double dEnd2Theta = dRotation + atan2(endPoint2.mY - center.mY, endPoint2.mX - center.mX) * 180.0 / PI;

         LocationType newEndPoint1;
         newEndPoint1.mX = center.mX + (dEnd1Displacement * cos(dEnd1Theta * PI / 180.0));
         newEndPoint1.mY = center.mY + (dEnd1Displacement * sin(dEnd1Theta * PI / 180.0));

         LocationType newEndPoint2;
         newEndPoint2.mX = center.mX + (dEnd2Displacement * cos(dEnd2Theta * PI / 180.0));
         newEndPoint2.mY = center.mY + (dEnd2Displacement * sin(dEnd2Theta * PI / 180.0));

         writeElement(POLYLINE, pData, lByteIndex, bCountOnly);
         writeElement(12, pData, lByteIndex, bCountOnly);   // Three nodes * four
         writeElement(static_cast<short>(newEndPoint1.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newEndPoint1.mY), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newUrCorner.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newUrCorner.mY), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newEndPoint2.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newEndPoint2.mY), pData, lByteIndex, bCountOnly);

         break;
      }

      case CGM_OBJECT:
      case GROUP_OBJECT:
      {
         list<GraphicObject*> groupObjects = static_cast<const GraphicGroup*>(pObject)->getObjects();
         for (list<GraphicObject*>::iterator iter = groupObjects.begin(); iter != groupObjects.end(); ++iter)
         {
            GraphicObject* pGroupObject = *iter;
            if (pGroupObject != NULL)
            {
               bool bSuccess = serializeCgmProperties(pGroupObject, pData, lByteIndex, bCountOnly);
               if (bSuccess == false)
               {
                  return false;
               }

               bSuccess = serializeCgmObject(pGroupObject, pData, lByteIndex, bCountOnly);
               if (bSuccess == false)
               {
                  return false;
               }
            }
         }

         break;
      }

      case ELLIPSE_OBJECT:
      {
         // Element
         writeElement(ELLIPSE, pData, lByteIndex, bCountOnly);

         // Center
         writeElement(static_cast<short>(center.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(center.mY), pData, lByteIndex, bCountOnly);

         // Endpoints
         double dWidth = urCorner.mX - center.mX;
         double dHeight = urCorner.mY - center.mY;

         double dEndX1 = center.mX + (dWidth * cos(dRotation * PI / 180.0));
         double dEndY1 = center.mY + (dWidth * sin(dRotation * PI / 180.0));
         double dEndX2 = center.mX + (dHeight * cos((dRotation + 90.0) * PI / 180.0));
         double dEndY2 = center.mY + (dHeight * sin((dRotation + 90.0) * PI / 180.0));

         writeElement(static_cast<short>(dEndX1), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(dEndY1), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(dEndX2), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(dEndY2), pData, lByteIndex, bCountOnly);

         break;
      }

      case LINE_OBJECT:
      {
         // Element
         writeElement(POLYLINE, pData, lByteIndex, bCountOnly);

         // Parameters
         writeElement(8, pData, lByteIndex, bCountOnly);   // Two nodes * four

         // Points
         double dLlDisplacement = sqrt(pow(center.mX - llCorner.mX, 2) + pow(center.mY - llCorner.mY, 2));
         double dLlTheta = dRotation + atan2(center.mY - llCorner.mY, center.mX - llCorner.mX) * 180.0 / PI;
         double dUrDisplacement = sqrt(pow(urCorner.mX - center.mX, 2) + pow(urCorner.mY - center.mY, 2));
         double dUrTheta = dRotation + atan2(urCorner.mY - center.mY, urCorner.mX - center.mX) * 180.0 / PI;

         LocationType newLlCorner;
         newLlCorner.mX = center.mX - (dLlDisplacement * cos(dLlTheta * PI / 180.0));
         newLlCorner.mY = center.mY - (dLlDisplacement * sin(dLlTheta * PI / 180.0));

         LocationType newUrCorner;
         newUrCorner.mX = center.mX + (dUrDisplacement * cos(dUrTheta * PI / 180.0));
         newUrCorner.mY = center.mY + (dUrDisplacement * sin(dUrTheta * PI / 180.0));

         writeElement(static_cast<short>(newLlCorner.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newLlCorner.mY), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newUrCorner.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newUrCorner.mY), pData, lByteIndex, bCountOnly);

         break;
      }

      case POLYGON_OBJECT:
      {
         // Element
         writeElement(POLYGON, pData, lByteIndex, bCountOnly);

         // Parameters
         unsigned int uiSegments = 0;
         const PolygonObjectImp* pPolygonObject = dynamic_cast<const PolygonObjectImp*>(pObject);
         VERIFY(pPolygonObject != NULL);
         uiSegments = (pPolygonObject->getNumSegments() + 1) * 4;
         writeElement(static_cast<short>(uiSegments), pData, lByteIndex, bCountOnly);

         // Start point
         std::vector<LocationType> vertices = pPolygonObject->getVertices();

         LocationType startPoint;
         if (vertices.empty() == false)
         {
            startPoint = vertices.front();
         }

         double dStartDisplacement = sqrt(pow(center.mX - startPoint.mX, 2) + pow(center.mY - startPoint.mY, 2));
         double dStartTheta = dRotation + atan2(center.mY - startPoint.mY, center.mX - startPoint.mX) * 180.0 / PI;

         LocationType newStartPoint;
         newStartPoint.mX = center.mX - (dStartDisplacement * cos(dStartTheta * PI / 180.0));
         newStartPoint.mY = center.mY - (dStartDisplacement * sin(dStartTheta * PI / 180.0));

         writeElement(static_cast<short>(newStartPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newStartPoint.mY), pData, lByteIndex, bCountOnly);

         // Vertices
         vector<LocationType>::iterator iter = vertices.begin();
         iter++;
         for (; iter != vertices.end(); ++iter)
         {
            LocationType endPoint = *iter;

            double dEndDisplacement = sqrt(pow(endPoint.mX - center.mX, 2) + pow(endPoint.mY - center.mY, 2));
            double dEndTheta = dRotation + atan2(endPoint.mY - center.mY, endPoint.mX - center.mX) * 180.0 / PI;

            LocationType newEndPoint;
            newEndPoint.mX = center.mX + (dEndDisplacement * cos(dEndTheta * PI / 180.0));
            newEndPoint.mY = center.mY + (dEndDisplacement * sin(dEndTheta * PI / 180.0));

            writeElement(static_cast<short>(newEndPoint.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(newEndPoint.mY), pData, lByteIndex, bCountOnly);
         }

         break;
      }

      case POLYLINE_OBJECT:
      {
         // Element
         writeElement(POLYLINE, pData, lByteIndex, bCountOnly);

         // Parameters
         const PolylineObjectImp* pPolylineObject = dynamic_cast<const PolylineObjectImp*>(pObject);
         VERIFY(pPolylineObject != NULL);
         unsigned int uiSegments = (pPolylineObject->getNumSegments() + 1) * 4;
         writeElement(static_cast<short>(uiSegments), pData, lByteIndex, bCountOnly);

         // Start point
         vector<LocationType> vertices = pPolylineObject->getVertices();

         LocationType startPoint;
         if (vertices.empty() == false)
         {
            startPoint = vertices.front();
         }

         double dStartDisplacement = sqrt(pow(center.mX - startPoint.mX, 2) + pow(center.mY - startPoint.mY, 2));
         double dStartTheta = dRotation + atan2(center.mY - startPoint.mY, center.mX - startPoint.mX) * 180.0 / PI;

         LocationType newStartPoint;
         newStartPoint.mX = center.mX - (dStartDisplacement * cos(dStartTheta * PI / 180.0));
         newStartPoint.mY = center.mY - (dStartDisplacement * sin(dStartTheta * PI / 180.0));

         writeElement(static_cast<short>(newStartPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newStartPoint.mY), pData, lByteIndex, bCountOnly);

         // Vertices
         vector<LocationType>::iterator iter = vertices.begin();
         iter++;
         for (; iter != vertices.end(); ++iter)
         {
            LocationType endPoint = *iter;

            double dEndDisplacement = sqrt(pow(endPoint.mX - center.mX, 2) + pow(endPoint.mY - center.mY, 2));
            double dEndTheta = dRotation + atan2(endPoint.mY - center.mY, endPoint.mX - center.mX) * 180.0 / PI;

            LocationType newEndPoint;
            newEndPoint.mX = center.mX + (dEndDisplacement * cos(dEndTheta * PI / 180.0));
            newEndPoint.mY = center.mY + (dEndDisplacement * sin(dEndTheta * PI / 180.0));

            writeElement(static_cast<short>(newEndPoint.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(newEndPoint.mY), pData, lByteIndex, bCountOnly);
         }

         break;
      }

      case RECTANGLE_OBJECT:
      {
         if (dRotation == 0.0)
         {
            // Element
            writeElement(RECTANGLE, pData, lByteIndex, bCountOnly);

            // Location
            writeElement(static_cast<short>(llCorner.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(llCorner.mY), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(urCorner.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(urCorner.mY), pData, lByteIndex, bCountOnly);
         }
         else
         {
            // Element
            writeElement(POLYGON, pData, lByteIndex, bCountOnly);

            // Parameters
            writeElement(20, pData, lByteIndex, bCountOnly);   // 5 defining points * 4

            // Lower left
            double dLlDisplacement = sqrt(pow(center.mX - llCorner.mX, 2) + pow(center.mY - llCorner.mY, 2));
            double dLlTheta = dRotation + atan2(center.mY - llCorner.mY, center.mX - llCorner.mX) * 180.0 / PI;

            LocationType newLlPoint;
            newLlPoint.mX = center.mX - (dLlDisplacement * cos(dLlTheta * PI / 180.0));
            newLlPoint.mY = center.mY - (dLlDisplacement * sin(dLlTheta * PI / 180.0));

            writeElement(static_cast<short>(newLlPoint.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(newLlPoint.mY), pData, lByteIndex, bCountOnly);

            // Lower right
            double dLrDisplacement = sqrt(pow(urCorner.mX - center.mX, 2) + pow(llCorner.mY - center.mY, 2));
            double dLrTheta = dRotation + atan2(llCorner.mY - center.mY, urCorner.mX - center.mX) * 180.0 / PI;

            LocationType newLrPoint;
            newLrPoint.mX = center.mX + (dLrDisplacement * cos(dLrTheta * PI / 180.0));
            newLrPoint.mY = center.mY + (dLrDisplacement * sin(dLrTheta * PI / 180.0));

            writeElement(static_cast<short>(newLrPoint.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(newLrPoint.mY), pData, lByteIndex, bCountOnly);

            // Upper right
            double dUrDisplacement = sqrt(pow(urCorner.mX - center.mX, 2) + pow(urCorner.mY - center.mY, 2));
            double dUrTheta = dRotation + atan2(urCorner.mY - center.mY, urCorner.mX - center.mX) * 180.0 / PI;

            LocationType newUrPoint;
            newUrPoint.mX = center.mX + (dUrDisplacement * cos(dUrTheta * PI / 180.0));
            newUrPoint.mY = center.mY + (dUrDisplacement * sin(dUrTheta * PI / 180.0));

            writeElement(static_cast<short>(newUrPoint.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(newUrPoint.mY), pData, lByteIndex, bCountOnly);

            // Upper left
            double dUlDisplacement = sqrt(pow(llCorner.mX - center.mX, 2) + pow(urCorner.mY - center.mY, 2));
            double dUlTheta = dRotation + atan2(urCorner.mY - center.mY, llCorner.mX - center.mX) * 180.0 / PI;

            LocationType newUlPoint;
            newUlPoint.mX = center.mX + (dUlDisplacement * cos(dUlTheta * PI / 180.0));
            newUlPoint.mY = center.mY + (dUlDisplacement * sin(dUlTheta * PI / 180.0));

            writeElement(static_cast<short>(newUlPoint.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(newUlPoint.mY), pData, lByteIndex, bCountOnly);

            // Lower left again to close the polygon
            writeElement(static_cast<short>(newLlPoint.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(newLlPoint.mY), pData, lByteIndex, bCountOnly);
         }

         break;
      }

      case ROUNDEDRECTANGLE_OBJECT:
      {
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

         double dRadius = 0.0;
         dRadius = DrawUtil::minimum(fabs(urCorner.mX - llCorner.mX),
            fabs(urCorner.mY - llCorner.mY)) * 0.15;

         DrawUtil::initializeCircle();

         // Element
         bool bFilled = false;
         bFilled = pObject->getFillState();
         if (bFilled == false)
         {
            writeElement(POLYLINE, pData, lByteIndex, bCountOnly);
         }
         else
         {
            writeElement(POLYGON, pData, lByteIndex, bCountOnly);
         }

         // Parameters
         writeElement(1460, pData, lByteIndex, bCountOnly);      // 365 nodes (including duplicate end node) * four

         // Points
         double dX = llCorner.mX + dRadius;
         double dY = llCorner.mY;
         double dDisplacement = sqrt(pow(dX - center.mX, 2) + pow(dY - center.mY, 2));
         double dTheta = dRotation + atan2(dY - center.mY, dX - center.mX) * 180.0 / PI;

         LocationType newPoint;
         newPoint.mX = center.mX + (dDisplacement * cos(dTheta * PI / 180.0));
         newPoint.mY = center.mY + (dDisplacement * sin(dTheta * PI / 180.0));

         writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);

         dX = urCorner.mX - dRadius;
         dY = llCorner.mY;
         dDisplacement = sqrt(pow(dX - center.mX, 2) + pow(dY - center.mY, 2));
         dTheta = dRotation + atan2(dY - center.mY, dX - center.mX) * 180.0 / PI;

         newPoint.mX = center.mX + (dDisplacement * cos(dTheta * PI / 180.0));
         newPoint.mY = center.mY + (dDisplacement * sin(dTheta * PI / 180.0));

         writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);

         int i = 0;
         for (i = 3 * DrawUtil::VERTEX_COUNT / 4; i < DrawUtil::VERTEX_COUNT; i += 2)
         {
            dX = urCorner.mX - dRadius + DrawUtil::sCirclePoints[i].mX * dRadius;
            dY = llCorner.mY + dRadius + DrawUtil::sCirclePoints[i].mY * dRadius;
            dDisplacement = sqrt(pow(dX - center.mX, 2) + pow(dY - center.mY, 2));
            dTheta = dRotation + atan2(dY - center.mY, dX - center.mX) * 180.0 / PI;

            newPoint.mX = center.mX + (dDisplacement * cos(dTheta * PI / 180.0));
            newPoint.mY = center.mY + (dDisplacement * sin(dTheta * PI / 180.0));

            writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);
         }

         dX = urCorner.mX;
         dY = urCorner.mY - dRadius;
         dDisplacement = sqrt(pow(dX - center.mX, 2) + pow(dY - center.mY, 2));
         dTheta = dRotation + atan2(dY - center.mY, dX - center.mX) * 180.0 / PI;

         newPoint.mX = center.mX + (dDisplacement * cos(dTheta * PI / 180.0));
         newPoint.mY = center.mY + (dDisplacement * sin(dTheta * PI / 180.0));

         writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);

         for (i = 0; i < DrawUtil::VERTEX_COUNT / 4; i += 2)
         {
            dX = urCorner.mX - dRadius + DrawUtil::sCirclePoints[i].mX * dRadius;
            dY = urCorner.mY - dRadius + DrawUtil::sCirclePoints[i].mY * dRadius;
            dDisplacement = sqrt(pow(dX - center.mX, 2) + pow(dY - center.mY, 2));
            dTheta = dRotation + atan2(dY - center.mY, dX - center.mX) * 180.0 / PI;

            newPoint.mX = center.mX + (dDisplacement * cos(dTheta * PI / 180.0));
            newPoint.mY = center.mY + (dDisplacement * sin(dTheta * PI / 180.0));

            writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);
         }

         dX = llCorner.mX + dRadius;
         dY = urCorner.mY;
         dDisplacement = sqrt(pow(dX - center.mX, 2) + pow(dY - center.mY, 2));
         dTheta = dRotation + atan2(dY - center.mY, dX - center.mX) * 180.0 / PI;

         newPoint.mX = center.mX + (dDisplacement * cos(dTheta * PI / 180.0));
         newPoint.mY = center.mY + (dDisplacement * sin(dTheta * PI / 180.0));

         writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);

         for (i = DrawUtil::VERTEX_COUNT / 4; i < DrawUtil::VERTEX_COUNT / 2; i += 2)
         {
            dX = llCorner.mX + dRadius + DrawUtil::sCirclePoints[i].mX * dRadius;
            dY = urCorner.mY - dRadius + DrawUtil::sCirclePoints[i].mY * dRadius;
            dDisplacement = sqrt(pow(dX - center.mX, 2) + pow(dY - center.mY, 2));
            dTheta = dRotation + atan2(dY - center.mY, dX - center.mX) * 180.0 / PI;

            newPoint.mX = center.mX + (dDisplacement * cos(dTheta * PI / 180.0));
            newPoint.mY = center.mY + (dDisplacement * sin(dTheta * PI / 180.0));

            writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);
         }

         dX = llCorner.mX;
         dY = llCorner.mY + dRadius;
         dDisplacement = sqrt(pow(dX - center.mX, 2) + pow(dY - center.mY, 2));
         dTheta = dRotation + atan2(dY - center.mY, dX - center.mX) * 180.0 / PI;

         newPoint.mX = center.mX + (dDisplacement * cos(dTheta * PI / 180.0));
         newPoint.mY = center.mY + (dDisplacement * sin(dTheta * PI / 180.0));

         writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);

         for (i = DrawUtil::VERTEX_COUNT / 2; i < 3 * DrawUtil::VERTEX_COUNT / 4; i += 2)
         {
            dX = llCorner.mX + dRadius + DrawUtil::sCirclePoints[i].mX * dRadius;
            dY = llCorner.mY + dRadius + DrawUtil::sCirclePoints[i].mY * dRadius;
            dDisplacement = sqrt(pow(dX - center.mX, 2) + pow(dY - center.mY, 2));
            dTheta = dRotation + atan2(dY - center.mY, dX - center.mX) * 180.0 / PI;

            newPoint.mX = center.mX + (dDisplacement * cos(dTheta * PI / 180.0));
            newPoint.mY = center.mY + (dDisplacement * sin(dTheta * PI / 180.0));

            writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
            writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);
         }

         break;
      }

      case SCALEBAR_OBJECT:
      {
         const ScaleBarObjectImp* pScaleBarObject = dynamic_cast<const ScaleBarObjectImp*>(pObject);
         VERIFY(pScaleBarObject != NULL);
         const GraphicGroup* pGroup = &pScaleBarObject->getGroup();
         if (pGroup != NULL)
         {
            bool bSuccess = serializeCgmObject(pGroup, pData, lByteIndex, bCountOnly);
            if (bSuccess == false)
            {
               return false;
            }
         }

         break;
      }

      case LATLONINSERT_OBJECT:
      {
         const LatLonInsertObjectImp* pLatLonObject = dynamic_cast<const LatLonInsertObjectImp*>(pObject);
         const GraphicGroup* pGroup = &pLatLonObject->getGroup();
         if (pGroup != NULL)
         {
            bool bSuccess = false;
            bSuccess = serializeCgmObject(pGroup, pData, lByteIndex, bCountOnly);
            if (bSuccess == false)
            {
               return false;
            }
         }
         
         break;
      }

      case NORTHARROW_OBJECT:
         // fall through
      case EASTARROW_OBJECT:
         {
            const DirectionalArrowObjectImp* pDirArrowObj = dynamic_cast<const DirectionalArrowObjectImp*>(pObject);
            VERIFY(pDirArrowObj != NULL);
            const GraphicGroup* pGroup = &pDirArrowObj->getGroup();
            if (pGroup != NULL)
            {
               bool bSuccess = serializeCgmObject(pGroup, pData, lByteIndex, bCountOnly);
               if (bSuccess == false)
               {
                  return false;
               }
            }
            
            break;
         }

      case TEXT_OBJECT:
      {
         // Element
         writeElement(TEXT, pData, lByteIndex, bCountOnly);

         // Parameters
         string text = pObject->getText();

         unsigned char ucLength = static_cast<unsigned char>(text.length());

         writeElement(static_cast<short>(ucLength + 7), pData, lByteIndex, bCountOnly);

         // Location
         double dDisplacement = sqrt(pow(center.mX - llCorner.mX, 2) + pow(center.mY - llCorner.mY, 2));
         double dTheta = dRotation + atan2(center.mY - llCorner.mY, center.mX - llCorner.mX) * 180.0 / PI;

         LocationType newPoint;
         newPoint.mX = center.mX - (dDisplacement * cos(dTheta * PI / 180.0));
         newPoint.mY = center.mY - (dDisplacement * sin(dTheta * PI / 180.0));

         writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);
         writeElement(1, pData, lByteIndex, bCountOnly);

         // Length
         writeStringElement(text.data(), pData, lByteIndex, bCountOnly);
         break;
      }

      case TRIANGLE_OBJECT:
      {
         // Element
         writeElement(POLYGON, pData, lByteIndex, bCountOnly);

         // Parameters
         writeElement(16, pData, lByteIndex, bCountOnly);   // Four nodes (including duplicate end node) * four

         // Points
         double dApex = pObject->getApex();
         double dWidth = urCorner.mX - llCorner.mX;

         // Lower left
         double dDisplacement = sqrt(pow(center.mX - llCorner.mX, 2) + pow(center.mY - llCorner.mY, 2));
         double dTheta = dRotation + atan2(center.mY - llCorner.mY, center.mX - llCorner.mX) * 180.0 / PI;

         LocationType newLlPoint;
         newLlPoint.mX = center.mX - (dDisplacement * cos(dTheta * PI / 180.0));
         newLlPoint.mY = center.mY - (dDisplacement * sin(dTheta * PI / 180.0));

         writeElement(static_cast<short>(newLlPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newLlPoint.mY), pData, lByteIndex, bCountOnly);

         // Lower right
         dDisplacement = sqrt(pow(urCorner.mX - center.mX, 2) + pow(llCorner.mY - center.mY, 2));
         dTheta = dRotation + atan2(llCorner.mY - center.mY, urCorner.mX - center.mX) * 180.0 / PI;

         LocationType newPoint;
         newPoint.mX = center.mX + (dDisplacement * cos(dTheta * PI / 180.0));
         newPoint.mY = center.mY + (dDisplacement * sin(dTheta * PI / 180.0));

         writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);

         // Apex
         dDisplacement = sqrt(pow((llCorner.mX + (dApex * dWidth)) - center.mX, 2) + pow(urCorner.mY - center.mY, 2));
         dTheta = dRotation + atan2(urCorner.mY - center.mY, (llCorner.mX + (dApex * dWidth)) - center.mX) * 180.0 / PI;

         newPoint.mX = center.mX + (dDisplacement * cos(dTheta * PI / 180.0));
         newPoint.mY = center.mY + (dDisplacement * sin(dTheta * PI / 180.0));

         writeElement(static_cast<short>(newPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newPoint.mY), pData, lByteIndex, bCountOnly);

         // Lower left again to close the polygon
         writeElement(static_cast<short>(newLlPoint.mX), pData, lByteIndex, bCountOnly);
         writeElement(static_cast<short>(newLlPoint.mY), pData, lByteIndex, bCountOnly);

         break;
      }

      default:
         return false;
   }

   return true;
}
