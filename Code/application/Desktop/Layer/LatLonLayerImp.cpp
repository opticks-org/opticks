/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>

#include "AppConfig.h"
#include "DrawUtil.h"
#include "GcpList.h"
#include "GeoPoint.h"
#include "glCommon.h"
#include "LatLonLayerAdapter.h"
#include "LatLonLayerImp.h"
#include "LatLonLayerUndo.h"
#include "PerspectiveView.h"
#include "PropertiesLatLonLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "Undo.h"
#include "ViewImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <limits>
#include <math.h>
#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

/**
  * The static methods are documented below near their definitions.
  */
static LocationType adjustSegment(LocationType outside, LocationType inside, const vector<LocationType>& border);
static double computeSpacing(double range);
static double getStep(double diff);

LatLonLayerImp::LatLonLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   LayerImp(id, layerName, pElement),
   mGeocoordType(LatLonLayer::getSettingGeocoordType()),
   mFormat(LatLonLayer::getSettingFormat()),
   mStyle(LatLonLayer::getSettingGridlineStyle()),
   mColor(COLORTYPE_TO_QCOLOR(LatLonLayer::getSettingGridlineColor())),
   mWidth(LatLonLayer::getSettingGridlineWidth()),
   mCubeSize(LocationType(0.0, 0.0)),
   mMaxCoord(LocationType(0.0, 0.0)),
   mMinCoord(LocationType(0.0, 0.0)),
   mZone(0),
   mHemisphere('N'),
   mComputeTickSpacing(true),
   mTickSpacing(0.0, 0.0),
   mComputedTickSpacing(0.0, 0.0),
   mComputedTickSpacingDirty(true),
   mBorderDirty(true),
   mFont(LatLonLayerImp::getDefaultFont())
{
   mpElement.addSignal(SIGNAL_NAME(RasterElement, GeoreferenceModified),
      Slot(this, &LatLonLayerImp::georeferenceModified));

   addPropertiesPage(PropertiesLatLonLayer::getName());

   connect(this, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(fontChanged(const QFont&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(styleChanged(const LatLonStyle&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(widthChanged(unsigned int)), this, SIGNAL(modified()));
   connect(this, SIGNAL(tickSpacingChanged(const LocationType&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(autoTickSpacingChanged(bool)), this, SIGNAL(modified()));
   connect(this, SIGNAL(coordTypeChanged(const GeocoordType&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(formatChanged(const DmsFormatType&)), this, SIGNAL(modified()));
}

LatLonLayerImp::~LatLonLayerImp()
{}

const string& LatLonLayerImp::getObjectType() const
{
   static string sType("LatLonLayerImp");
   return sType;
}

bool LatLonLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "LatLonLayer"))
   {
      return true;
   }

   return LayerImp::isKindOf(className);
}

void LatLonLayerImp::georeferenceModified(Subject& subject, const string& signal, const boost::any& data)
{
   setBorderDirty(true);
   setTickSpacingDirty(true);
}

LatLonLayerImp& LatLonLayerImp::operator=(const LatLonLayerImp& latLonLayer)
{
   if (this != &latLonLayer)
   {
      LayerImp::operator =(latLonLayer);

      mColor = latLonLayer.mColor;
      mStyle = latLonLayer.mStyle;
      mWidth = latLonLayer.mWidth;
      mTickSpacing = latLonLayer.mTickSpacing;
      mComputeTickSpacing = latLonLayer.mComputeTickSpacing;
      mFont = latLonLayer.mFont;
      mGeocoordType = latLonLayer.mGeocoordType;
      mFormat = latLonLayer.mFormat;
   }

   return *this;
}

LayerType LatLonLayerImp::getLayerType() const
{
   return LAT_LONG;
}

vector<ColorType> LatLonLayerImp::getColors() const
{
   vector<ColorType> colors;

   QColor currentColor = getColor();
   if (currentColor.isValid() == true)
   {
      ColorType color(currentColor.red(), currentColor.green(), currentColor.blue());
      colors.push_back(color);
   }

   return colors;
}

void LatLonLayerImp::draw()
{
   const int stepCount = 100;
   LocationType tickSpacing;
   LocationType start;
   LocationType stop;
   LocationType stepSize;
   int xCount;
   int yCount;
   int i;
   int j;
   vector<LocationType> stepValues;
   stepValues.reserve(stepCount+2);
   LocationType geoVertex;
   LocationType pixelVertex;
   bool haveX = false;
   bool haveY = false;

   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;

   bool bProductView = false;
   LocationType textOffset;
   vector<LocationType> viewportBox;
   int iHeight(0);
   ViewImp* pView = dynamic_cast<ViewImp*>(getView());
   if (pView == NULL)
   {
      return;
   }

   QWidget* pParent = pView->parentWidget();
   if (pParent != NULL)
   {
      // Check to see if the view's parent widget is a view, which
      // indicates that the layer is being drawn in a product
      if (pParent->inherits("ViewImp") == true)
      {
         bProductView = true;

         textOffset = LocationType(viewPort[0], viewPort[1]);
         viewportBox.push_back(LocationType(viewPort[0], viewPort[1]));
         viewportBox.push_back(LocationType(viewPort[0] + viewPort[2], viewPort[1]));
         viewportBox.push_back(LocationType(viewPort[0] + viewPort[2], viewPort[1] + viewPort[3]));
         viewportBox.push_back(LocationType(viewPort[0], viewPort[1] + viewPort[3]));

         if (mStyle != LATLONSTYLE_CROSS)
         {
            glDisable(GL_SCISSOR_TEST);
         }

         int iWidth = ::max(1, pParent->width());
         iHeight = ::max(1, pParent->height());

         glViewport(0, 0, iWidth, iHeight);
         glMatrixMode(GL_PROJECTION);
         glLoadIdentity();
         gluOrtho2D(0, iWidth, 0, iHeight);
         glMatrixMode(GL_MODELVIEW);
         glLoadIdentity();
      }
   }
   pView->getExtents(dMinX, dMinY, dMaxX, dMaxY);

   vector<LocationType> boundingBox;
   boundingBox.push_back(LocationType(dMinX, dMinY));
   boundingBox.push_back(LocationType(dMaxX, dMinY));
   boundingBox.push_back(LocationType(dMaxX, dMaxY));
   boundingBox.push_back(LocationType(dMinX, dMaxY));

   setBoundingBox(boundingBox);

   RasterElement* pRaster = dynamic_cast<RasterElement*>(getDataElement());
   if (pRaster == NULL)
   {
      return;
   }

   struct LineAttributeRestorer
   {
      LineAttributeRestorer()
      {
         glPushAttrib(GL_LINE_BIT);
      }

      ~LineAttributeRestorer()
      {
         glPopAttrib();
      }
   } lineAttributeRestorer;

   glColor4ub(mColor.red(), mColor.green(), mColor.blue(), mColor.alpha());
   glLineWidth(mWidth);

   tickSpacing = getTickSpacing();
   if (tickSpacing.mX != 0.0)
   {
      haveX = true;
   }
   else
   {
      tickSpacing.mX = computeSpacing (mMaxCoord.mX - mMinCoord.mX);
   }

   if (tickSpacing.mY != 0.0)
   {
      haveY = true;
   }
   else
   {
      tickSpacing.mY = computeSpacing(mMaxCoord.mY - mMinCoord.mY);
   }

   start.mX = floor(mMinCoord.mX / tickSpacing.mX) * tickSpacing.mX;
   start.mY = floor(mMinCoord.mY / tickSpacing.mY) * tickSpacing.mY;

   stop.mX = ceil(mMaxCoord.mX / tickSpacing.mX) * tickSpacing.mX;
   stop.mY = ceil(mMaxCoord.mY / tickSpacing.mY) * tickSpacing.mY;

   stepSize.mX = (mMaxCoord.mX - mMinCoord.mX) / static_cast<double>(stepCount - 1);
   stepSize.mY = (mMaxCoord.mY - mMinCoord.mY) / static_cast<double>(stepCount - 1);

   xCount = 1.5 + (stop.mX - start.mX) / tickSpacing.mX;
   yCount = 1.5 + (stop.mY - start.mY) / tickSpacing.mY;

   vector<LocationType> vertices;
   vertices.reserve(stepCount);

   if (mStyle == LATLONSTYLE_DASHED)
   {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(2, 0x0f0f);
   }

   if (mStyle == LATLONSTYLE_SOLID || mStyle == LATLONSTYLE_DASHED)
   {
      bool onOff = false;
      bool isInData = false;
      LocationType oldVertex;

      // extend values by 1 step on each side
      stepValues.push_back(LocationType(mMinCoord.mX - stepSize.mX, mMinCoord.mY - stepSize.mY));
      for (j = 0; j < stepCount; ++j)
      {
         stepValues.push_back(LocationType(mMinCoord.mX + static_cast<double>(j) * stepSize.mX, 
            mMinCoord.mY + static_cast<double>(j) * stepSize.mY));
      }
      stepValues.push_back(LocationType(mMinCoord.mX + static_cast<double>(stepCount) * stepSize.mX, 
         mMinCoord.mY + static_cast<double>(stepCount) * stepSize.mY));

      int numSteps = static_cast<int>(stepValues.size());

      vector<LocationType> borderBox(mBoundingBox);
      BorderType nearestBorder;
      LocationType startLabel;
      LocationType endLabel;
      LocationType clippedLabel;
      LocationType labelLoc;
      bool adjustForRotation(true);

      if (haveX)
      {
         for (i = 0; i < xCount; ++i)
         {
            onOff = false;
            isInData = false;
            geoVertex.mX = start.mX + static_cast<double>(i) * tickSpacing.mX;
            vertices.clear();
            vertices.reserve(numSteps);

            string xLabel = "";
            if (mGeocoordType == GEOCOORD_LATLON)
            {
               LatLonPoint latLonPoint(geoVertex);
               xLabel = latLonPoint.getLatitudeText(mFormat);
            }
            else if (mGeocoordType == GEOCOORD_UTM)
            {
               UtmPoint utmPoint(geoVertex.mX, 0.0, mZone, mHemisphere);
               xLabel = utmPoint.getEastingText() + ", " + utmPoint.getZoneText();
            }
            else if (mGeocoordType == GEOCOORD_MGRS)
            {
               xLabel = convertGcpValueToText(geoVertex.mX, true, mGeocoordType);
            }

            // find left most location in data
            int left(-1);
            isInData = false;
            while (!isInData && left < numSteps - 1)
            {
               ++left;
               geoVertex.mY = stepValues[left].mY;
               LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
               pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
               pixelVertex.mX += 0.5;
               pixelVertex.mY += 0.5;
               isInData = DrawUtil::isWithin(pixelVertex, &(*mBoundingBox.begin()), 4);
            }

            if (isInData)
            {
               // save left most vertex in data
               vertices.push_back(pixelVertex);

               // find right most location in data
               int right(numSteps);
               isInData = false;
               while (!isInData)
               {
                  --right;
                  geoVertex.mY = stepValues[right].mY;
                  LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  isInData = DrawUtil::isWithin(pixelVertex, &(*mBoundingBox.begin()), 4);
               }

               if (left > 0) // left most not at edge so need to get that pixel location
               {
                  LocationType leftEdge;
                  leftEdge.mX = geoVertex.mX;
                  leftEdge.mY = stepValues[left-1].mY;
                  LocationType geoDraw = convertPointToLatLon(mGeocoordType, leftEdge);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  pixelVertex = adjustSegment(pixelVertex, vertices.front(), mBoundingBox);
                  vertices.insert(vertices.begin(), pixelVertex);
               }

               // add rest of locations in data
               for (int loc = left + 1; loc <= right; ++loc)
               {
                  geoVertex.mY = stepValues[loc].mY;
                  LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  vertices.push_back(pixelVertex);
               }

               if (right < numSteps - 1)  // right most not at edge so need to get that pixel location
               {
                  LocationType rightEdge;
                  rightEdge.mX = geoVertex.mX;
                  rightEdge.mY = stepValues[right + 1].mY;
                  LocationType geoDraw = convertPointToLatLon(mGeocoordType, rightEdge);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  pixelVertex = adjustSegment(pixelVertex, vertices.back(), mBoundingBox);
                  vertices.push_back(pixelVertex);
               }

               // now draw the grid line
               if (vertices.empty() == false)
               {
                  startLabel = vertices.front();
                  endLabel = vertices.back();
               }
               if (bProductView)
               {
                  clipToView(vertices, viewportBox, modelMatrix, projectionMatrix, viewPort);
               }

               if (vertices.empty() == false)
               {
                  adjustForRotation = true;
                  borderBox = mBoundingBox;
                  labelLoc = startLabel;
                  if (bProductView)
                  {
                     clippedLabel = vertices.front();
                     DrawUtil::unProjectToZero(clippedLabel.mX, clippedLabel.mY, modelMatrix,
                        projectionMatrix, viewPort, &clippedLabel.mX, &clippedLabel.mY);

                     // if the original vertex has changed, then the label will be at edge
                     // of the viewport (grid line clipped to the view) and not at edge
                     // of data coordinates. Need to determine the nearest view border using
                     // screen coords and not use the view's rotation in figuring the
                     // label offset. Considered not changed if < 0.5 pixel distance between points.
                     if (!isCloseTo(startLabel, clippedLabel, 0.25))
                     {
                        labelLoc = vertices.front();
                        borderBox = viewportBox;
                        startLabel = clippedLabel;

                        // nearest border will be determined using screen coordinates and
                        // viewport not data coordinates and mBoundingBox, so turn off 
                        // rotation and origin corrections in drawLabel.
                        adjustForRotation = false;
                     }
                  }
                  nearestBorder = getNearestBorder(labelLoc, borderBox);
                  drawLabel(startLabel, textOffset, xLabel, nearestBorder, modelMatrix,
                     projectionMatrix, viewPort, adjustForRotation);
                  glLineWidth(1);
                  glLineWidth(mWidth);
                  glBegin (GL_LINE_STRIP);
                  vector<LocationType>::iterator it;
                  for (it = vertices.begin(); it != vertices.end(); ++it)
                  {
                     glVertex2f(it->mX, it->mY);
                  }
                  glEnd();

                  borderBox = mBoundingBox;
                  labelLoc = endLabel;
                  adjustForRotation = true;
                  if (bProductView)
                  {
                     clippedLabel = vertices.back();
                     DrawUtil::unProjectToZero(clippedLabel.mX, clippedLabel.mY, modelMatrix,
                        projectionMatrix, viewPort, &clippedLabel.mX, &clippedLabel.mY);

                     if (!isCloseTo(endLabel, clippedLabel, 0.25))
                     {
                        labelLoc = vertices.back();
                        borderBox = viewportBox;
                        endLabel = clippedLabel;
                        adjustForRotation = false;
                     }
                  }
                  nearestBorder = getNearestBorder(labelLoc, borderBox);
                  drawLabel(endLabel, textOffset, xLabel, nearestBorder, modelMatrix,
                     projectionMatrix, viewPort, adjustForRotation);
               }
            }
         }
      }

      if (haveY)
      {
         for (i = 0; i < yCount; ++i)
         {
            onOff = false;
            isInData = false;
            geoVertex.mY = start.mY + static_cast<double>(i) * tickSpacing.mY;
            vertices.clear();
            vertices.reserve(numSteps);

            string yLabel = "";
            if (mGeocoordType == GEOCOORD_LATLON)
            {
               LatLonPoint latLonPoint(geoVertex);
               yLabel = latLonPoint.getLongitudeText(mFormat);
            }
            else if (mGeocoordType == GEOCOORD_UTM)
            {
               UtmPoint utmPoint(0.0, geoVertex.mY, mZone, mHemisphere);
               yLabel = utmPoint.getNorthingText() + " " + utmPoint.getHemisphere();
            }
            else if (mGeocoordType == GEOCOORD_MGRS)
            {
               yLabel = convertGcpValueToText(geoVertex.mY, false, mGeocoordType);
            }

            // find bottom most location in data
            int bottom(-1);
            isInData = false;
            while (!isInData && bottom < numSteps - 1)
            {
               ++bottom;
               geoVertex.mX = stepValues[bottom].mX;
               LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
               pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
               pixelVertex.mX += 0.5;
               pixelVertex.mY += 0.5;
               isInData = DrawUtil::isWithin(pixelVertex, &(*mBoundingBox.begin()), 4);
            }

            if (isInData)
            {
               // save bottom most vertex in data
               vertices.push_back(pixelVertex);

               // find top most location in data
               int top(numSteps);
               isInData = false;
               while (!isInData)
               {
                  --top;
                  geoVertex.mX = stepValues[top].mX;
                  LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  isInData = DrawUtil::isWithin(pixelVertex, &(*mBoundingBox.begin()), 4);
               }

               if (bottom > 0) // bottom most not at edge so need to get that pixel location
               {
                  LocationType bottomEdge;
                  bottomEdge.mX = stepValues[bottom-1].mX;
                  bottomEdge.mY = geoVertex.mY;
                  LocationType geoDraw = convertPointToLatLon(mGeocoordType, bottomEdge);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  pixelVertex = adjustSegment(pixelVertex, vertices.front(), mBoundingBox);
                  vertices.insert(vertices.begin(), pixelVertex);
               }

               // add rest of locations in data
               for (int loc = bottom + 1; loc <= top; ++loc)
               {
                  geoVertex.mX = stepValues[loc].mX;
                  LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  vertices.push_back(pixelVertex);
               }

               if (top < numSteps - 1)  // top most not at edge so need to get that pixel location
               {
                  LocationType topEdge;
                  topEdge.mX = stepValues[top + 1].mX;
                  topEdge.mY = geoVertex.mY;
                  LocationType geoDraw = convertPointToLatLon(mGeocoordType, topEdge);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  pixelVertex = adjustSegment(pixelVertex, vertices.back(), mBoundingBox);
                  vertices.push_back(pixelVertex);
               }

               // now draw the grid line
               if (vertices.empty() == false)
               {
                  startLabel = vertices.front();
                  endLabel = vertices.back();
               }
               if (bProductView)
               {
                  clipToView(vertices, viewportBox, modelMatrix, projectionMatrix, viewPort);
               }

               if (vertices.empty() == false)  // check again in case none visible in product view
               {
                  adjustForRotation = true;
                  borderBox = mBoundingBox;
                  labelLoc = startLabel;
                  if (bProductView)
                  {
                     clippedLabel = vertices.front();
                     DrawUtil::unProjectToZero(clippedLabel.mX, clippedLabel.mY, modelMatrix,
                        projectionMatrix, viewPort, &clippedLabel.mX, &clippedLabel.mY);

                     if (!isCloseTo(startLabel, clippedLabel, 0.25))
                     {
                        labelLoc = vertices.front();
                        borderBox = viewportBox;
                        startLabel = clippedLabel;
                        adjustForRotation = false;
                     }
                  }
                  nearestBorder = getNearestBorder(labelLoc, borderBox);
                  drawLabel(startLabel, textOffset, yLabel, nearestBorder, modelMatrix,
                     projectionMatrix, viewPort, adjustForRotation);
                  glLineWidth(1);
                  glLineWidth(mWidth);
                  glBegin (GL_LINE_STRIP);
                  vector<LocationType>::iterator it;
                  for (it = vertices.begin(); it != vertices.end(); ++it)
                  {
                     glVertex2f(it->mX, it->mY);
                  }
                  glEnd();

                  borderBox = mBoundingBox;
                  labelLoc = endLabel;
                  adjustForRotation = true;
                  if (bProductView)
                  {
                     clippedLabel = vertices.back();
                     DrawUtil::unProjectToZero(clippedLabel.mX, clippedLabel.mY, modelMatrix,
                        projectionMatrix, viewPort, &clippedLabel.mX, &clippedLabel.mY);

                     if (!isCloseTo(endLabel, clippedLabel, 0.25))
                     {
                        labelLoc = vertices.back();
                        borderBox = viewportBox;
                        endLabel = clippedLabel;
                        adjustForRotation = false;
                     }
                  }
                  nearestBorder = getNearestBorder(labelLoc, borderBox);
                  drawLabel(endLabel, textOffset, yLabel, nearestBorder, modelMatrix,
                     projectionMatrix, viewPort, adjustForRotation);
               }
            }
         }
      }
   }
   else if (mStyle == LATLONSTYLE_CROSS)
   {
      glBegin (GL_LINES);
      GLdouble winZ;

      if (haveX)
      {
         for (i = 0; i < xCount; ++i)
         {
            geoVertex.mX = start.mX + static_cast<double>(i) * tickSpacing.mX;
            for (j = 0; j < yCount; ++j)
            {
               geoVertex.mY = start.mY + static_cast<double>(j) * tickSpacing.mY;
               LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
               pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
               if (DrawUtil::isWithin(pixelVertex, &(*mBoundingBox.begin()), 4))
               {
                  vector<LocationType> verticalSegment;
                  geoVertex.mY -= tickSpacing.mY / 20.0;
                  geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  verticalSegment.push_back(pixelVertex);

                  geoVertex.mY += tickSpacing.mY / 10.0;
                  geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  verticalSegment.push_back(pixelVertex);
                  vector<LocationType>::iterator it;
                  for (it = verticalSegment.begin(); it != verticalSegment.end(); ++it)
                  {
                     if (bProductView)
                     {
                        gluProject(it->mX, it->mY, 0.0, modelMatrix, projectionMatrix, viewPort,
                           &it->mX, &it->mY, &winZ);
                     }
                     glVertex2f(it->mX, it->mY);
                  }
               }
            }
         }
      }

      if (haveY)
      {
         for (i = 0; i < yCount; ++i)
         {
            geoVertex.mY = start.mY + static_cast<double>(i) * tickSpacing.mY;
            for (j = 0; j < xCount; ++j)
            {
               geoVertex.mX = start.mX + static_cast<double>(j) * tickSpacing.mX;
               LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
               pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
               if (DrawUtil::isWithin(pixelVertex, &(*mBoundingBox.begin()), 4))
               {
                  vector<LocationType> horizontalSegment;
                  geoVertex.mX -= tickSpacing.mX / 20.0;
                  geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  horizontalSegment.push_back(pixelVertex);

                  geoVertex.mX += tickSpacing.mX / 10.0;
                  geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel(geoDraw);
                  pixelVertex.mX += 0.5;
                  pixelVertex.mY += 0.5;
                  horizontalSegment.push_back(pixelVertex);
                  vector<LocationType>::iterator it;
                  for (it = horizontalSegment.begin(); it != horizontalSegment.end(); ++it)
                  {
                     if (bProductView)
                     {
                        gluProject(it->mX, it->mY, 0.0, modelMatrix, projectionMatrix, viewPort,
                           &it->mX, &it->mY, &winZ);
                     }
                     glVertex2f(it->mX, it->mY);
                  }
               }
            }
         }
      }
      glEnd ();
   }

   if (bProductView == true && mStyle != LATLONSTYLE_CROSS)
   {
      glEnable(GL_SCISSOR_TEST);
   }

   // Restore the previous matrices
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(projectionMatrix);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd(modelMatrix);
   glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);
}

void LatLonLayerImp::setLatLonFormat(const DmsFormatType& newFormat)
{
   if (newFormat == mFormat)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetLatLonFormat(dynamic_cast<LatLonLayer*>(this), mFormat, newFormat));
      }

      mFormat = newFormat;
      setBorderDirty(true);
      setTickSpacingDirty(true);
      emit formatChanged(mFormat);
      notify(SIGNAL_NAME(LatLonLayer, FormatChanged), boost::any(mFormat));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setLatLonFormat(newFormat);
         }
      }

      mbLinking = false;
   }
}

void LatLonLayerImp::reset()
{
   ColorType color = LatLonLayer::getSettingGridlineColor();
   QColor clrDefault = COLORTYPE_TO_QCOLOR(color);
   setColor(clrDefault);
   setStyle(LatLonLayer::getSettingGridlineStyle());
   setWidth(LatLonLayer::getSettingGridlineWidth());

   //NOTE: There is no configuration setting for auto spacing, this is intentional
   setTickSpacing(LocationType(0.0, 0.0));
   setAutoTickSpacing(true);
   setFont(LatLonLayerImp::getDefaultFont());
   setGeocoordType(GEOCOORD_LATLON);
   setLatLonFormat(LatLonLayer::getSettingFormat());
}

void LatLonLayerImp::drawLabel(const LocationType& location, const LocationType& textOffset,
                               const string& text, const BorderType& borderType, const double modelMatrix[16],
                               const double projectionMatrix[16], const int viewPort[4], bool adjustForRotation)
{
   if (text.empty() == true)
   {
      return;
   }

   ViewImp* pView = dynamic_cast<ViewImp*> (getView());
   if (pView == NULL)
   {
      return;
   }

   QString strLabel = QString::fromStdString(text);

   LocationType screenCoord = location;
   GLdouble winZ;
   gluProject(location.mX, location.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
      &screenCoord.mX, &screenCoord.mY, &winZ);

   screenCoord -= textOffset;

   QFontMetrics fontMetrics(mFont.getQFont());
   double dWidth = fontMetrics.width(strLabel);
   double dHeight = fontMetrics.ascent();

   LocationType offset;
   double dRotation = 0.0;
   double dPitch = 0.0;


   PerspectiveView* pPerspectiveView = dynamic_cast<PerspectiveView*> (pView);

   if (pPerspectiveView != NULL)
   {
      // If borderType was determined using data coordinates, then need to adjust for the
      // view's rotation and pitch (origin).
      // If it was determined using screen coordinates, then don't need to make adjustments.
      if (adjustForRotation)
      {
         dRotation = pPerspectiveView->getRotation();
         dPitch = pPerspectiveView->getPitch();
      }
   }

   switch (borderType)
   {
   case LEFT_BORDER:
      dRotation += 180.0;
      break;

   case BOTTOM_BORDER:
      dRotation += 90.0;
      break;

   case TOP_BORDER:
      dRotation += 270.0;
      break;

   default:
      break;
   }

   // Update the rotation angle for the pitch since Qt always uses the same
   // origin location for the screen coordinates
   if (dPitch < 0.0)
   {
      dRotation *= -1.0;
   }

   double dPixelMargin = 5.0;
   offset.mX = (dWidth / 2.0 * cos(dRotation * PI / 180.0)) - (dWidth / 2.0) +
      (dPixelMargin * cos(dRotation * PI / 180.0));
   offset.mY = (-dHeight / 2.0 * sin(dRotation * PI / 180.0)) - (dHeight / 2.0) -
      (dPixelMargin * sin(dRotation * PI / 180.0));

   LocationType textLocation = screenCoord + offset;

   int screenX = static_cast<int>(textLocation.mX);
   int screenY = pView->height() - static_cast<int>(textLocation.mY);

   pView->renderText(screenX, screenY, strLabel, mFont.getQFont());
}

bool LatLonLayerImp::getExtents(double& x1, double& y1, double& x4, double& y4)
{
   translateDataToWorld(0, 0, x1, y1);
   translateDataToWorld(mCubeSize.mX, mCubeSize.mY, x4, y4);

   return true;
}

QColor LatLonLayerImp::getColor() const
{
   return mColor;
}

LatLonStyle LatLonLayerImp::getStyle() const
{
   return mStyle;
}

unsigned int LatLonLayerImp::getWidth() const
{
   return mWidth;
}

LocationType LatLonLayerImp::getTickSpacing() const
{
   if (mComputeTickSpacing)
   {
      if (mComputedTickSpacingDirty)
      {
         const_cast<LatLonLayerImp*>(this)->computeTickSpacing();
      }

      return mComputedTickSpacing;
   }
   else if (mBorderDirty == true)
   {
      const_cast<LatLonLayerImp*>(this)->computeBorder();
   }

   return mTickSpacing;
}

bool LatLonLayerImp::getAutoTickSpacing() const
{
   return mComputeTickSpacing;
}

GeocoordType LatLonLayerImp::getGeocoordType() const
{
   return mGeocoordType;
}

DmsFormatType LatLonLayerImp::getLatLonFormat() const
{
   return mFormat;
}

string LatLonLayerImp::convertGcpValueToText(double value, bool isX, const GeocoordType& type)
{
   string gcpText = "";
   switch (type)
   {
      case GEOCOORD_LATLON:
      {
         DmsPoint::DmsType eDmsType = DmsPoint::DMS_LATITUDE;
         if (isX == false)
         {
            eDmsType = DmsPoint::DMS_LONGITUDE;
         }

         DmsPoint dmsPoint(eDmsType, value);
         gcpText = dmsPoint.getValueText(mFormat);
         break;
      }

      case GEOCOORD_UTM:
      case GEOCOORD_MGRS:
      {
         char buffer[256];
         sprintf(buffer, "%.1f", value);

         gcpText = buffer;
         break;
      }

      case GEOCOORD_GENERAL:
      default:
         break;
   }

   return gcpText;
}

string LatLonLayerImp::convertGcpPointToText(const GcpPoint& point, const GeocoordType& type)
{
   string label = convertGcpPointToText(point.mCoordinate, type);
   return label;
}

string LatLonLayerImp::convertGcpPointToText(const LocationType& latLonCoords, const GeocoordType& type)
{
   string gcpText = "";

   LatLonPoint latLon(latLonCoords);
   if (type == GEOCOORD_LATLON)
   {
      gcpText = latLon.getText(mFormat);
   }
   else if (type == GEOCOORD_UTM)
   {
      UtmPoint utm(latLon);
      gcpText = utm.getText();
   }
   else if (type == GEOCOORD_MGRS)
   {
      MgrsPoint mgrs(latLon);
      gcpText = mgrs.getText();
   }

   return gcpText;
}

void LatLonLayerImp::setBorderDirty(bool bDirty)
{
   mBorderDirty = bDirty;
}

void LatLonLayerImp::setTickSpacingDirty(bool bDirty)
{
   mComputedTickSpacingDirty = bDirty;
}

QFont LatLonLayerImp::getDefaultFont()
{
   QString strFont = QString::fromStdString(LatLonLayer::getSettingFont());
   if (strFont.isEmpty() == true)
   {
      strFont = QApplication::font().family();
   }

   int iSize = LatLonLayer::getSettingFontSize();

   QFont font(strFont, iSize);
   return font;
}

bool LatLonLayerImp::toXml(XMLWriter* pXml) const
{
   if (!LayerImp::toXml(pXml))
   {
      return false;
   }

   stringstream buf;
   pXml->addAttr("geocoordType", mGeocoordType);
   pXml->addAttr("dmsFormatType", mFormat);
   pXml->addAttr("latlonStyle", mStyle);
   pXml->addAttr("color", mColor.name().toStdString());
   pXml->addAttr("width", mWidth);

   buf << mCubeSize.mX << " " << mCubeSize.mY;
   pXml->addAttr("cubeSize", buf.str());

   buf.str("");
   buf << mMaxCoord.mX << " " << mMaxCoord.mY;
   pXml->addAttr("maxCoord", buf.str());

   buf.str("");
   buf << mMinCoord.mX << " " << mMinCoord.mY;
   pXml->addAttr("minCoord", buf.str());

   pXml->addAttr("zone", mZone);
   pXml->addAttr("hemisphere", mHemisphere);
   pXml->addAttr("mapGrid", mMapGrid);
   pXml->addAttr("computeTickSpacing", mComputeTickSpacing);

   buf.str("");
   buf << mTickSpacing.mX << " " << mTickSpacing.mY;
   pXml->addAttr("tickSpacing", buf.str() );

   buf.str("");
   buf << mComputedTickSpacing.mX << " " << mComputedTickSpacing.mY;
   pXml->addAttr("computedTickSpacing", buf.str() );

   pXml->addFontElement("font", mFont);

   return true;
}

bool LatLonLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   stringstream str;
   DO_IF(!LayerImp::fromXml(pDocument, version), return false);
   DOMElement* pElement = dynamic_cast<DOMElement*>(pDocument);
   VERIFY(pElement != NULL);
   mGeocoordType = StringUtilities::fromXmlString<GeocoordType>(A(pElement->getAttribute(X("geocoordType"))));
   mFormat = StringUtilities::fromXmlString<DmsFormatType>(A(pElement->getAttribute(X("dmsFormatType"))));
   mStyle = StringUtilities::fromXmlString<LatLonStyle>(A(pElement->getAttribute(X("latlonStyle"))));
   mColor = QColor(A(pElement->getAttribute(X("color"))));
   mWidth = StringUtilities::fromXmlString<unsigned int>(A(pElement->getAttribute(X("width"))));
   str << A(pElement->getAttribute(X("cubeSize")));
   str >> mCubeSize.mX >> mCubeSize.mY;
   str.clear();
   str << A(pElement->getAttribute(X("maxCoord")));
   str >> mMaxCoord.mX >> mMaxCoord.mY;
   str.clear();
   str << A(pElement->getAttribute(X("minCoord")));
   str >> mMinCoord.mX >> mMinCoord.mY;
   mZone = StringUtilities::fromXmlString<int>(A(pElement->getAttribute(X("zone"))));
   mHemisphere = StringUtilities::fromXmlString<char>(A(pElement->getAttribute(X("hemisphere"))));
   mMapGrid = A(pElement->getAttribute(X("mapGrid")));
   mComputeTickSpacing = StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("computeTickSpacing"))));
   str.clear();
   str << A(pElement->getAttribute(X("tickSpacing")));
   str >> mTickSpacing.mX >> mTickSpacing.mY;
   str.clear();
   str << A(pElement->getAttribute(X("computedTickSpacing")));
   str >> mComputedTickSpacing.mX >> mComputedTickSpacing.mY;
   readFontElement("font", pElement, mFont);
   return true;
}

void LatLonLayerImp::setColor(const QColor& newColor)
{
   if (newColor.isValid() == false)
   {
      return;
   }

   if (newColor == mColor)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetLatLonColor(dynamic_cast<LatLonLayer*>(this), QCOLOR_TO_COLORTYPE(mColor),
            QCOLOR_TO_COLORTYPE(newColor)));
      }

      mColor = newColor;
      emit colorChanged(mColor);
      notify(SIGNAL_NAME(LatLonLayer, ColorChanged), boost::any(QCOLOR_TO_COLORTYPE(mColor)));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();

      vector<Layer*>::iterator iter = linkedLayers.begin();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         LatLonLayerImp* pLayer = dynamic_cast<LatLonLayerImp*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setColor(newColor);
         }
      }

      mbLinking = false;
   }
}

void LatLonLayerImp::setFont(const QFont& font)
{
   if (font == mFont.getQFont())
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetLatLonFont(dynamic_cast<LatLonLayer*>(this), mFont.toQFont(), font));
      }

      mFont = font;
      emit fontChanged(mFont.getQFont());
      notify(SIGNAL_NAME(LatLonLayer, FontChanged), boost::any(font));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         LatLonLayerImp* pLayer = dynamic_cast<LatLonLayerImp*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setFont(font);
         }
      }

      mbLinking = false;
   }
}

void LatLonLayerImp::setStyle(const LatLonStyle& newStyle)
{
   if (newStyle == mStyle)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetLatLonStyle(dynamic_cast<LatLonLayer*>(this), mStyle, newStyle));
      }

      mStyle = newStyle;
      emit styleChanged(mStyle);
      notify(SIGNAL_NAME(LatLonLayer, StyleChanged), boost::any(mStyle));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setStyle(newStyle);
         }
      }

      mbLinking = false;
   }
}

void LatLonLayerImp::setWidth(unsigned int width)
{
   if (width == mWidth)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetLatLonWidth(dynamic_cast<LatLonLayer*>(this), mWidth, width));
      }

      mWidth = width;
      emit widthChanged(mWidth);
      notify(SIGNAL_NAME(LatLonLayer, WidthChanged), boost::any(mWidth));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setWidth(mWidth);
         }
      }

      mbLinking = false;
   }
}

void LatLonLayerImp::setTickSpacing(const LocationType& spacing)
{
   if (spacing == mTickSpacing)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      UndoGroup group(pView, "Set Tick Spacing");
      setAutoTickSpacing(false);

      if (pView != NULL)
      {
         pView->addUndoAction(new SetLatLonTickSpacing(dynamic_cast<LatLonLayer*>(this), mTickSpacing, spacing));
      }

      mTickSpacing = spacing;
      emit tickSpacingChanged(mTickSpacing);
      notify(SIGNAL_NAME(LatLonLayer, TickSpacingChanged), boost::any(spacing));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setTickSpacing(spacing);
         }
      }

      mbLinking = false;
   }
}

void LatLonLayerImp::setAutoTickSpacing(bool compute)
{
   if (compute == mComputeTickSpacing)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetLatLonAutoTickSpacing(dynamic_cast<LatLonLayer*>(this), mComputeTickSpacing,
            compute));
      }

      mComputeTickSpacing = compute;
      emit autoTickSpacingChanged(mComputeTickSpacing);
      notify(SIGNAL_NAME(LatLonLayer, AutoTickSpacingChanged), boost::any(compute));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setAutoTickSpacing(compute);
         }
      }

      mbLinking = false;
   }
}

void LatLonLayerImp::computeBorder()
{
   RasterElement* pRaster = dynamic_cast<RasterElement*>(getDataElement());
   if (pRaster == NULL)
   {
      return;
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   double rows = pDescriptor->getRowCount();
   double cols = pDescriptor->getColumnCount();

   mZone = 100;
   mCubeSize.mX = cols;
   mCubeSize.mY = rows;
   mMaxCoord = LocationType(-numeric_limits<double>::max(), -numeric_limits<double>::max());
   mMinCoord = LocationType(numeric_limits<double>::max(), numeric_limits<double>::max());

   vector<LocationType> borderPixels;
   borderPixels.push_back(LocationType(0, 0));
   borderPixels.push_back(LocationType(0, rows / 2));
   borderPixels.push_back(LocationType(0, rows));
   borderPixels.push_back(LocationType(cols / 2, rows));
   borderPixels.push_back(LocationType(cols, rows));
   borderPixels.push_back(LocationType(cols, rows / 2));
   borderPixels.push_back(LocationType(cols, 0));
   borderPixels.push_back(LocationType(cols / 2, 0));

   for (vector<LocationType>::const_iterator iter = borderPixels.begin(); iter != borderPixels.end(); ++iter)
   {
      LocationType latlon = pRaster->convertPixelToGeocoord(*iter, true);

      LocationType geoCoord;
      LatLonPoint latLonPoint(latlon);
      if (mGeocoordType == GEOCOORD_LATLON)
      {
         geoCoord = latLonPoint.getCoordinates();
      }
      else if (mGeocoordType == GEOCOORD_UTM)
      {
         UtmPoint utmPoint(latLonPoint);
         geoCoord = utmPoint.getCoordinates();
         mZone = utmPoint.getZone();
         mHemisphere = utmPoint.getHemisphere();
      }
      else if (mGeocoordType == GEOCOORD_MGRS)
      {
         MgrsPoint mgrsPoint(latLonPoint);
         geoCoord = mgrsPoint.getCoordinates();
         mZone = mgrsPoint.getZone();
         mMapGrid = mgrsPoint.getScrCodeText();
      }

      mMinCoord.mX = min(geoCoord.mX, mMinCoord.mX);
      mMinCoord.mY = min(geoCoord.mY, mMinCoord.mY);
      mMaxCoord.mX = max(geoCoord.mX, mMaxCoord.mX);
      mMaxCoord.mY = max(geoCoord.mY, mMaxCoord.mY);
   }

   mBorderDirty = false;
}

void LatLonLayerImp::computeTickSpacing()
{
   if (mBorderDirty)
   {
      computeBorder();
   }

   if (mBorderDirty)
   {
      return;
   }

   LocationType diff;
   diff.mX = mMaxCoord.mX - mMinCoord.mX;
   diff.mY = mMaxCoord.mY - mMinCoord.mY;

   if (mGeocoordType== GEOCOORD_LATLON)
   {
      mComputedTickSpacing.mX = computeSpacing(diff.mX);
      mComputedTickSpacing.mY = computeSpacing(diff.mY);
   }
   else
   {
      mComputedTickSpacing.mX = getStep(diff.mX);
      mComputedTickSpacing.mY = getStep(diff.mY);
   }

   mComputedTickSpacingDirty = false;
}

/**
  * Determines where a line segment intersects the edge of the cube and
  * returns the intersection point. It does this by dividing up the outside
  * area into 9 regions (region 5 is inside the cube) as shown here:
  * Regions:
  * 123
  * 456
  * 789
  * It determines which region the outside point resides in. This allows it
  * to simply determine which edge of the cube the segment intersected.It then
  * performs a simple computation to determine the intersection point.
  *
  * @param outside
  *     The line segment end point that is outside the cube
  *
  * @param inside
  *     The line segment end point that is inside the cube
  *
  * @param size
  *     The size of the cube in pixels.
  *
  * @return The intersection point of the line segment with the edge of the cube
  */
static LocationType adjustSegment(LocationType outside, LocationType inside, const vector<LocationType>& border)
{
   enum { BELOW, BETWEEN, ABOVE } xRegion = BETWEEN, yRegion = BETWEEN;
   enum { XABOVE, XBELOW, YABOVE, YBELOW } intersectLine = XBELOW;
   LocationType intersection (0.0, 0.0);
   LocationType delta (0.0, 0.0);
   double f;

   if (outside.mX < border[0].mX)
   {
      xRegion = BELOW;
   }
   else if (outside.mX > border[1].mX)
   {
      xRegion = ABOVE;
   }

   if (outside.mY < border[1].mY)
   {
      yRegion = BELOW;
   }
   else if (outside.mY > border[2].mY)
   {
      yRegion = ABOVE;
   }

   delta.mX = outside.mX - inside.mX;
   delta.mY = outside.mY - inside.mY;

   if (xRegion == BETWEEN) // in region 2 or 8
   {
      if (yRegion == ABOVE) // in region 2
      {
         intersectLine = YABOVE;
      }
      else // yRegion == BELOW: can't ==BETWEEN or both points would be inside: region 8
      {
         intersectLine = YBELOW;
      }
   }
   else // xRegion != BETWEEN: region 1, 3, 4, 6, 7 or 9
   {
      if (yRegion == BETWEEN) // region 4 or 6
      {
         if (xRegion == ABOVE) // region 6
         {
            intersectLine = XABOVE;
         }
         else // xRegion == BELOW: region 4
         {
            intersectLine = XBELOW;
         }
      }
      else // region 1, 3, 7 or 9: one of the 4 corners
      {
         if (xRegion == BELOW) // region 1 or 7
         {
            if (yRegion == BELOW) // region 7
            {
               if (outside.mY + delta.mY * (border[0].mX - outside.mX) / delta.mX >= border[1].mY)
               {
                  intersectLine = XBELOW;
               }
               else
               {
                  intersectLine = YBELOW;
               }
            }
            else // yRegion == ABOVE: region 1
            {
               if (outside.mY + delta.mY * (border[0].mX - outside.mX) / delta.mX <= border[2].mY)
               {
                  intersectLine = XBELOW;
               }
               else
               {
                  intersectLine = YABOVE;
               }
            }
         }
         else // xRegion == ABOVE // region 3 or 9
         {
            if (yRegion == BELOW) // region 9
            {
               if (inside.mY + delta.mY * (border[1].mX - inside.mX) / delta.mX >= border[1].mY)
               {
                  intersectLine = XABOVE;
               }
               else
               {
                  intersectLine = YBELOW;
               }
            }
            else // yRegion == ABOVE: region 3
            {
               if (inside.mY + delta.mY * (border[1].mX - inside.mX) / delta.mX <= border[2].mY)
               {
                  intersectLine = XABOVE;
               }
               else
               {
                  intersectLine = YABOVE;
               }
            }
         }
      }
   }

   switch (intersectLine)
   {
   case XABOVE:
      f = (border[1].mX - inside.mX) / delta.mX;
      intersection.mY = f * delta.mY + inside.mY;
      intersection.mX = border[1].mX;
      break;
   case XBELOW:
      f = (border[0].mX-inside.mX) / delta.mX;
      intersection.mY = f * delta.mY + inside.mY;
      intersection.mX = border[0].mX;
      break;
   case YABOVE:
      f = (border[2].mY - inside.mY) / delta.mY;
      intersection.mX = f * delta.mX + inside.mX;
      intersection.mY = border[2].mY;
      break;
   case YBELOW:
      f = (border[1].mY-inside.mY) / delta.mY;
      intersection.mX = f * delta.mX + inside.mX;
      intersection.mY = border[1].mY;
      break;
   default:
      break;
   }

   return intersection;
}

/**
  * Computes the tick spacing that will give a pleasing number of grid
  * lines over the range of values specified. It assumes DMS type data.
  *
  * @param range
  *     The max value - the min value
  *
  * @return The tick spacing.
  */
static double computeSpacing(double range)
{
   double baseValues[] = {60.0, 30.0, 15.0, 10.0, 5.0, 2.0, 1.0};
   int baseValueCount = sizeof(baseValues) / sizeof(baseValues[0]);
   double spacing = 0.0;
   int i;

   // Treat baseValues as degrees. Break if one of them 
   // would result in 5+ tick marks.
   for (i = 0; i < baseValueCount; ++i)
   {
      if (5.0 * baseValues[i] < range)
      {
         spacing = baseValues[i];
         break;
      }
   }

   if (i == baseValueCount)
   {
      // Treat baseValues as minute. Break if one of them 
      // would result in 5+ tick marks.
      for (i = 0; i < baseValueCount; ++i)
      {
         if (5.0 * (baseValues[i] / 60.0) < range)
         {
            spacing = baseValues[i] / 60.0;
            break;
         }
      }

      if (i == baseValueCount)
      {
         for (i = 0; i < baseValueCount; ++i)
         {
            // Treat baseValues as seconds. Break if one of them 
            // would result in 5+ tick marks.
            if (5.0 * (baseValues[i] / 3600.0) < range)
            {
               spacing = baseValues[i] / 3600.0;
               break;
            }
         }

         // General case. Tick mark will be in decimal seconds.
         if (i == baseValueCount)
         {
            spacing = getStep (range*3600.0)/3600.0;
         }
      }
   }

   return spacing;
}

/**
  * Computes the tick spacing that will give a pleasing number of grid
  * lines over the range of values specified. It assumes non DMS data.
  *
  * @param range
  *     The max value - the min value
  *
  * @return The tick spacing.
  */
static double getStep(double diff)
{
   double logdiff = floor(log10 (diff));
   double powdiff = pow(10.0, logdiff);
   double scaleddiff = diff / powdiff;

   if (scaleddiff < 0.95)
   {
      return powdiff * 0.1;
   }
   if (scaleddiff < 1.9)
   {
      return 0.2 * powdiff;
   }
   if (scaleddiff < 4.75)
   {
      return 0.5 * powdiff;
   }

   return powdiff;
}

void LatLonLayerImp::setBoundingBox(const vector<LocationType>& boundingBox)
{
   if (boundingBox != mBoundingBox)
   {
      mBoundingBox = boundingBox;
      mBorderDirty = true;
   }
}

QFont LatLonLayerImp::getFont() const
{
   return mFont.toQFont();
}

LocationType LatLonLayerImp::convertPointToLatLon(const GeocoordType& type, const LocationType& point)
{
   LocationType coord;
   char buffer[256];
   switch (type)
   {
      case GEOCOORD_LATLON:
         coord = point;
         break;

      case GEOCOORD_UTM:
      {
         UtmPoint utmPoint(point.mX, point.mY, mZone, mHemisphere);
         LatLonPoint latLonPoint = utmPoint.getLatLonCoordinates();
         coord = latLonPoint.getCoordinates();
         break;
      }

      case GEOCOORD_MGRS:
      {
         snprintf(buffer, 255, "%d%s%05d%05d", mZone, mMapGrid.c_str(), static_cast<int>(point.mX),
            static_cast<int>(point.mY));

         MgrsPoint mgrsPoint(buffer);
         LatLonPoint latLonPoint = mgrsPoint.getLatLonCoordinates();
         coord = latLonPoint.getCoordinates();
         break;
      }

      default:
         break;
   }

   return coord;
}

void LatLonLayerImp::setGeocoordType(const GeocoordType& eGeocoord)
{
   if (eGeocoord == mGeocoordType)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetLatLonGeocoordType(dynamic_cast<LatLonLayer*>(this), mGeocoordType, eGeocoord));
      }

      mGeocoordType = eGeocoord;
      setBorderDirty(true);
      setTickSpacingDirty(true);
      emit coordTypeChanged(mGeocoordType);
      notify(SIGNAL_NAME(LatLonLayer, CoordTypeChanged), boost::any(mGeocoordType));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setGeocoordType(eGeocoord);
         }
      }

      mbLinking = false;
   }
}

const FontImp& LatLonLayerImp::getFontImp() const
{
   return mFont;
}

void LatLonLayerImp::clipToView(vector<LocationType>& vertices, const vector<LocationType>& clipBox,
                    const double modelMatrix[16], const double projectionMatrix[16], const int viewPort[4])
{
   int numPts = static_cast<int>(vertices.size());
   if (numPts < 2)
   {
      vertices.clear();
      return;
   }

   // project vertices to screen coordinates
   bool inClipBox;
   bool firstFound(false);
   bool lastFound(false);
   int index(0);
   int firstVisible(-1);
   int lastVisible(numPts);
   vector<LocationType>::iterator it;
   LocationType pixel;
   GLdouble winZ;
   vector<LocationType> screenPixels;
   screenPixels.reserve(numPts);
   LocationType center((clipBox[0].mX + clipBox[1].mX) / 2.0, (clipBox[0].mY + clipBox[3].mY) / 2.0);
   double closest(numeric_limits<double>::max());
   int closestPoint(-1);
   int secondClosestPoint(-1);
   for (it = vertices.begin(); it != vertices.end(); ++it)
   {
      pixel = *it;
      gluProject(pixel.mX, pixel.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
         &pixel.mX, &pixel.mY, &winZ);
      double distance = sqrt(pow(center.mX - pixel.mX, 2.0) + pow(center.mY - pixel.mY, 2.0));
      if (distance < closest)
      {
         secondClosestPoint = closestPoint;
         closestPoint = index;
         closest = distance;
      }
      inClipBox = DrawUtil::isWithin(pixel, &(*clipBox.begin()), 4);
      if (inClipBox)
      {
         screenPixels.push_back(pixel);
         if (firstFound == false)
         {
            firstVisible = index;
            firstFound = true;
         }
         else
         {
            lastVisible = index;
            lastFound = true;
         }
      }
      ++index;
   }

   if (firstFound)                        // have at least one visible grid point
   {
      if (firstVisible > 0)               // need to extend line to edge
      {
         pixel = vertices[firstVisible - 1];
         gluProject(pixel.mX, pixel.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
            &pixel.mX, &pixel.mY, &winZ);
         pixel = adjustSegment(pixel, screenPixels.front(), clipBox);
         screenPixels.insert(screenPixels.begin(), pixel);
      }

      if (lastFound)                      // have two visible grid points
      {
         if (lastVisible < numPts - 1)    // need to extend line to edge
         {
            pixel = vertices[lastVisible + 1];
            gluProject(pixel.mX, pixel.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
               &pixel.mX, &pixel.mY, &winZ);
            pixel = adjustSegment(pixel, screenPixels.back(), clipBox);
            screenPixels.push_back(pixel);
         }
      }
      else if (firstVisible + 1 < numPts) // only have one visible grid point and its not the last point,
      {                                   // so need to extend to edge
         pixel = vertices[firstVisible + 1];
         gluProject(pixel.mX, pixel.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
            &pixel.mX, &pixel.mY, &winZ);
         pixel = adjustSegment(pixel, screenPixels.back(), clipBox);
         screenPixels.push_back(pixel);
      }
   }
   // no visible grid points, check if line crosses clipBox
   else if (closestPoint >= 0 && secondClosestPoint >= 0)
   {
      LocationType closestPixel = vertices[closestPoint];
      gluProject(closestPixel.mX, closestPixel.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
         &closestPixel.mX, &closestPixel.mY, &winZ);
      LocationType secondClosestPixel = vertices[secondClosestPoint];
      gluProject(secondClosestPixel.mX, secondClosestPixel.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
         &secondClosestPixel.mX, &secondClosestPixel.mY, &winZ);

      // get points in ascending mX order
      LocationType first;
      LocationType second;
      if (closestPixel.mX < secondClosestPixel.mX)
      {
         first = closestPixel;
         second = secondClosestPixel;
      }
      else
      {
         first = secondClosestPixel;
         second = closestPixel;
      }

      screenPixels = findVisibleLineSegment(first, second, clipBox);
   }

   swap(vertices, screenPixels);
}

LatLonLayerImp::BorderType LatLonLayerImp::getNearestBorder(const LocationType& location, 
   const vector<LocationType>& box) const
{
   BorderType nearest(LEFT_BORDER);
   if (box.size() != 4)
   {
      return nearest;
   }

   BorderType sides[] = {BOTTOM_BORDER, RIGHT_BORDER, TOP_BORDER, LEFT_BORDER};

   double minDistance = 1e38;
   LocationType intersection;
   for (int i = 0; i < 4; ++i)
   {
      double distance = DrawUtil::linePointDistance(box[i], box[(i + 1) % 4], location, &intersection);
      if (distance < minDistance)
      {
         minDistance = distance;
         nearest = sides[i];
      }
   }

   return nearest;
}

void LatLonLayerImp::adjustBorderBox(vector<LocationType>& box, LocationType point1, LocationType point2,
                                     bool latitudeLine)
{
   if (box.size() != 4)
   {
      return;
   }

   if (latitudeLine)
   {
      box[0].mX = point1.mX;
      box[1].mX = point2.mX;
      box[2].mX = point2.mX;
      box[3].mX = point1.mX;
   }
   else
   {
      box[0].mY = point2.mY;
      box[1].mY = point2.mY;
      box[2].mY = point1.mY;
      box[3].mY = point1.mY;
   }
}

bool LatLonLayerImp::isCloseTo(const LocationType& point1, const LocationType& point2, const double tolerance)
{
   double xDiff = point2.mX - point1.mX;
   double yDiff = point2.mY - point1.mY;
   double distance = sqrt(xDiff * xDiff + yDiff * yDiff);

   return distance <= tolerance;
}

vector<LocationType> LatLonLayerImp::findVisibleLineSegment(const LocationType pixel1, const LocationType pixel2,
                                                            const vector<LocationType>& box)
{
   vector<LocationType> line;
   if (box.size() != 4)
   {
      return line;
   }
   line.reserve(2);

   // check if close to vertical line - undefined slope
   if (abs(pixel2.mX - pixel1.mX) < 0.001)
   {
      // if x between clipBox x's then line visible
      if (pixel1.mX >= box[0].mX && pixel1.mX <= box[1].mX)
      {
         line.push_back(LocationType(pixel1.mX, box[0].mY));
         line.push_back(LocationType(pixel1.mX, box[3].mY));
      }
   }
   else
   {
      // determine line equation, i.e. y = mx + b where m = (y2-y1)/(x2-x1)
      double slope = (pixel2.mY - pixel1.mY) / (pixel2.mX - pixel1.mX);
      double yIntercept = pixel1.mY - slope * pixel1.mX;

      // check if close to horizontal line - slope ~ 0
      if (abs(slope) < 0.001)
      {
         // since y = yIntercept for slope = 0, if y intercept between box y's then line visible
         if (yIntercept >= box[0].mY && yIntercept <= box[3].mY)
         {
            line.push_back(LocationType(box[0].mX, yIntercept));
            line.push_back(LocationType(box[1].mX, yIntercept));
         }
      }
      else if (slope > 0.0) // positive slope, pixel1 point must be on left or bottom border to be visible
      {
         double x(0.0);
         double y = box[0].mX * slope + yIntercept;
         if (y >= box[0].mY && y <= box[3].mY)        // pixel1 point on left edge
         {
            line.push_back(LocationType(box[0].mX, y));
         }
         else                                         // find line's x value for box bottom
         {
            x = (box[0].mY - yIntercept) / slope;
            if (x >= box[0].mX && x <= box[1].mX)     // pixel1 point on bottom edge
            {
               line.push_back(LocationType(x, box[0].mY));
            }
         }

         if (line.empty() == false)                   // have pixel1 now find pixel2
         {
            y = box[1].mX * slope + yIntercept;
            if (y >= box[1].mY && y < box[2].mY)      // pixel2 point on right edge
            {
               line.push_back(LocationType(box[1].mX, y));
            }
            else                                      // find line's x value for box top
            {
               x = (box[2].mY - yIntercept) / slope;
               if (x >= box[3].mX && x <= box[2].mX)  // pixel2 point on top edge
               {
                  line.push_back(LocationType(x, box[2].mY));
               }
            }
         }
      }
      else if (slope < 0.0) // negative slope, pixel1 point must be on left or top border to be visible
      {
         double x(0.0);
         double y = box[0].mX * slope + yIntercept;
         if (y >= box[0].mY && y <= box[3].mY)        // pixel1 point on left edge
         {
            line.push_back(LocationType(box[0].mX, y));
         }
         else                                         // find line's x value for box top
         {
            x = (box[3].mY - yIntercept) / slope;
            if (x >= box[3].mX && x <= box[2].mX)     // pixel1 point on top edge
            {
               line.push_back(LocationType(x, box[3].mY));
            }
         }

         if (line.empty() == false)                   // have pixel1 now find pixel2
         {
            y = box[1].mX * slope + yIntercept;
            if (y >= box[1].mY && y < box[2].mY)      // pixel2 point on right edge
            {
               line.push_back(LocationType(box[1].mX, y));
            }
            else                                      // find line's x value for box bottom
            {
               x = (box[1].mY - yIntercept) / slope;
               if (x >= box[0].mX && x <= box[1].mX)  // pixel2 point on bottom edge
               {
                  line.push_back(LocationType(x, box[1].mY));
               }
            }
         }
      }
   }

   // check that line is not being extrapolated - x end points within original segment
   if (line.empty() == false)
   {
      if (DrawUtil::isBetween(line.front().mX, pixel1.mX, pixel2.mX) == false ||
         DrawUtil::isBetween(line.back().mX, pixel1.mX, pixel2.mX) == false)
      {
         line.clear();
      }
   }

   return line;
}
