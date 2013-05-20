/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>
#include <QtGui/QApplication>

#include "AppConfig.h"
#include "AppVerify.h"
#include "DrawUtil.h"
#include "GeoPoint.h"
#include "GeoreferenceDescriptor.h"
#include "glCommon.h"
#include "LatLonLayer.h"
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

//Define the limits of the georeference plugins for resolving lat/lon points.  These will cap coordinates to
//these values so that the geopoint utils don't return garbage strings for labels
#define LAT_MIN -90.0
#define LAT_MAX 90.0
#define LAT_UTMMIN -80.0
#define LAT_UTMMAX 84.0
#define LON_MIN -180.0
#define LON_MAX 360.0

//Define how many samples in X (rown) and Y (column) directions to take to determine lat/lon min/max
//in bounding box.
#define ROW_SAMPLING 10
#define COL_SAMPLING 10

/**
  * The static methods are documented below near their definitions.
  */
static double computeSpacing(double range);
static double getStep(double diff);

LatLonLayerImp::LatLonLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   LayerImp(id, layerName, pElement),
   mGeocoordType(GeoreferenceDescriptor::getSettingGeocoordType()),
   mbExtrapolate(LatLonLayer::getSettingAlwaysExtrapolate()),
   mFormat(GeoreferenceDescriptor::getSettingLatLonFormat()),
   mStyle(LatLonLayer::getSettingGridlineStyle()),
   mColor(COLORTYPE_TO_QCOLOR(LatLonLayer::getSettingGridlineColor())),
   mWidth(LatLonLayer::getSettingGridlineWidth()),
   mMaxCoord(LocationType(0.0, 0.0)),
   mMinCoord(LocationType(0.0, 0.0)),
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

   VERIFYNR(connect(this, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(fontChanged(const QFont&)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(styleChanged(const LatLonStyle&)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(widthChanged(unsigned int)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(tickSpacingChanged(const LocationType&)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(autoTickSpacingChanged(bool)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(extrapolationChanged(bool)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(coordTypeChanged(const GeocoordType&)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(formatChanged(const DmsFormatType&)), this, SIGNAL(modified())));
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

bool LatLonLayerImp::isKindOfLayer(const string& className)
{
   if ((className == "LatLonLayerImp") || (className == "LatLonLayer"))
   {
      return true;
   }

   return LayerImp::isKindOfLayer(className);
}

void LatLonLayerImp::getLayerTypes(vector<string>& classList)
{
   classList.push_back("LatLonLayer");
   LayerImp::getLayerTypes(classList);
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
   double stepValuesX[stepCount];
   double stepValuesY[stepCount];
   LocationType geoVertex;
   LocationType pixelVertex;
   LocationType lastGeoVertex;
   LocationType badVertex(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
   LocationType lastPixelVertex;
   LocationType firstGoodPixelVertex;
   bool haveX = false;
   bool haveY = false;

   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   bool bProductView = false;
   LocationType textOffset;
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

   RasterElement* pRaster = dynamic_cast<RasterElement*>(getDataElement());
   if (pRaster == NULL)
   {
      return;
   }

   //Get bounding box of view
   LocationType worldLowerLeft;
   LocationType worldUpperLeft;
   LocationType worldUpperRight;
   LocationType worldLowerRight;
   pView->getVisibleCorners(worldLowerLeft, worldUpperLeft, worldUpperRight, worldLowerRight);

   LocationType dataLowerLeft;
   LocationType dataUpperLeft;
   LocationType dataUpperRight;
   LocationType dataLowerRight;
   translateWorldToData(worldLowerLeft.mX, worldLowerLeft.mY, dataLowerLeft.mX, dataLowerLeft.mY);
   translateWorldToData(worldUpperLeft.mX, worldUpperLeft.mY, dataUpperLeft.mX, dataUpperLeft.mY);
   translateWorldToData(worldUpperRight.mX, worldUpperRight.mY, dataUpperRight.mX, dataUpperRight.mY);
   translateWorldToData(worldLowerRight.mX, worldLowerRight.mY, dataLowerRight.mX, dataLowerRight.mY);

   vector<LocationType> boundingBox;
   boundingBox.push_back(dataUpperLeft);
   boundingBox.push_back(dataUpperRight);
   boundingBox.push_back(dataLowerRight);
   boundingBox.push_back(dataLowerLeft);

   setBoundingBox(boundingBox);

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

   tickSpacing = getTickSpacing(true);
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

   //Divide each lat/lon grid line into small segments for drawing.  Drawing will be from vertex to vertex.
   //Calculate start lat and lon (x and y) based on commanded tick spacing (as well as lat/lon increment)
   start.mX = ceil(mMinCoord.mX / tickSpacing.mX) * tickSpacing.mX;
   start.mY = ceil(mMinCoord.mY / tickSpacing.mY) * tickSpacing.mY;

   stop.mX = floor(mMaxCoord.mX / tickSpacing.mX) * tickSpacing.mX;
   stop.mY = floor(mMaxCoord.mY / tickSpacing.mY) * tickSpacing.mY;

   stepSize.mX = (mMaxCoord.mX - mMinCoord.mX) / static_cast<double>(stepCount - 1);
   stepSize.mY = (mMaxCoord.mY - mMinCoord.mY) / static_cast<double>(stepCount - 1);

   xCount = 1.5 + (stop.mX - start.mX) / tickSpacing.mX;
   yCount = 1.5 + (stop.mY - start.mY) / tickSpacing.mY;

   //Vertices array will be populated wtih vertexes for drawing each lat/lon line
   vector<LocationType> vertices;
   vertices.reserve(stepCount);
   bool vertexValid;

   if (mStyle == LATLONSTYLE_DASHED)
   {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(2, 0x0f0f);
   }

   if (mStyle == LATLONSTYLE_SOLID || mStyle == LATLONSTYLE_DASHED)
   {
      LocationType oldVertex;

      for (j = 0; j < stepCount; ++j)
      {
         stepValuesX[j] = mMinCoord.mX + static_cast<double>(j) * stepSize.mX;  //lat points for drawing lon lines
         stepValuesY[j] = mMinCoord.mY + static_cast<double>(j) * stepSize.mY;  //lon points for drawing lat lines
      }

      BorderType startXBorderType = LEFT_BORDER;
      BorderType endXBorderType = RIGHT_BORDER;
      BorderType startYBorderType = TOP_BORDER;
      BorderType endYBorderType = BOTTOM_BORDER;

      LocationType minPixel = pRaster->convertGeocoordToPixel(mMinCoord);
      LocationType maxPixel = pRaster->convertGeocoordToPixel(mMaxCoord);
      if (minPixel.mX > maxPixel.mX)
      {
         startXBorderType = RIGHT_BORDER;
         endXBorderType = LEFT_BORDER;
      }
      if (minPixel.mY < maxPixel.mY)
      {
         startYBorderType = BOTTOM_BORDER;
         endYBorderType = TOP_BORDER;
      }
    
      LocationType startLabel;
      LocationType endLabel;
      LocationType clippedLabel;

      bool bDrewLatLine = false;
      bool bDrewLonLine = false;
      //Loop twice, once for lat once for lon
      for (int axis = 0; axis < 2; axis++)
      {
         //During lat pass, use X values for geo coordinates, switch to Y for lon
         bool lat = true;
         if (axis == 1)
         {
            lat = false;
         }
         bool& haveAxis = (lat ? haveX : haveY);
         int& count = (lat ? xCount : yCount);
         double& geoVertexAxis = (lat ? geoVertex.mX : geoVertex.mY);
         double& geoVertexOffAxis = (lat ? geoVertex.mY : geoVertex.mX);
         double& lastGeoVertexAxis = (lat ? lastGeoVertex.mX : lastGeoVertex.mY);
         double& lastGeoVertexOffAxis = (lat ? lastGeoVertex.mY : lastGeoVertex.mX);
         double& startAxis = (lat ? start.mX : start.mY);
         double& tickSpacingAxis = (lat ? tickSpacing.mX : tickSpacing.mY);
         double* stepValuesOffAxis = (lat ? stepValuesY : stepValuesX);
         BorderType& startBorderType = (lat ? startXBorderType : startYBorderType);
         BorderType& endBorderType = (lat ? endXBorderType : endYBorderType);
         bool& bDrewLine = (lat ? bDrewLatLine : bDrewLonLine);

         if (haveAxis)
         {
            //Loop for each lat or lon line that needs potentially drawn
            for (i = 0; i < count; ++i)
            {
               //Fix lat or lon used for the current line being drawn.  This will not change during this iteration
               //of the loop.
               geoVertexAxis = startAxis + static_cast<double>(i) * tickSpacingAxis;
               geoVertexOffAxis = 0.0;
               vertices.clear();

               if (lat)
               {
                  if (mGeocoordType == GEOCOORD_LATLON)
                  {
                     if (geoVertexAxis < LAT_MIN || geoVertexAxis > LAT_MAX)
                     {
                        continue;
                     }
                  }
                  else
                  {
                     //UTM/MGRS only defined between S80 and N84
                     if (geoVertexAxis < LAT_UTMMIN || geoVertexAxis > LAT_UTMMAX)
                     {
                        continue;
                     }
                  }
               }
               else
               {
                  if (geoVertexAxis < LON_MIN || geoVertexAxis > LON_MAX)
                  {
                     continue;
                  }
               }

               //Loop for each vertex point, drawing the line segment if it should be displayed
               bool bVertexGoneValid = false;
               for (int loc = 0; loc < stepCount; ++loc)
               {
                  double screenX1;
                  double screenY1;
                  double screenX2;
                  double screenY2;

                  //Latch the geo point along the line into geoVertex
                  //Vertex is NOT valid if the geo reference says it is not.  Will not be drawn if not valid
                  geoVertexOffAxis = stepValuesOffAxis[loc];

                  if (lat)
                  {
                     if (geoVertexOffAxis < LON_MIN || geoVertexOffAxis > LON_MAX)
                     {
                        continue;
                     }
                  }
                  else
                  {
                     if (mGeocoordType == GEOCOORD_LATLON)
                     {
                        if (geoVertexOffAxis < LAT_MIN || geoVertexOffAxis > LAT_MAX)
                        {
                           continue;
                        }
                     }
                     else
                     {
                        //UTM/MGRS only defined between S80 and N84
                        if (geoVertexOffAxis < LAT_UTMMIN || geoVertexOffAxis > LAT_UTMMAX)
                        {
                           continue;
                        }
                     }
                  }

                  pixelVertex = pRaster->convertGeocoordToPixel(geoVertex, false, &vertexValid);
                  vertexValid |= mbExtrapolate;

                  //Check if pixel loc is in bounding box, if not it's invalid regardless of geo reference validity.
                  if (vertexValid)
                  {
                     translateDataToScreen(pixelVertex.mX, pixelVertex.mY, screenX1, screenY1);
                     vertexValid = (screenX1 > 0.0 && screenY1 > 0.0 &&
                        screenX1 < pView->width() && screenY1 < pView->height());
                  }

                  if ((vertexValid != bVertexGoneValid) && loc != 0)
                  {
                     //The vertex is transitioning between invalid and valid, need to determine
                     //where along the segment it changed and insert vertex there...
                     lastGeoVertexAxis = geoVertexAxis;
                     lastGeoVertexOffAxis = stepValuesOffAxis[loc-1];
                     lastPixelVertex = pRaster->convertGeocoordToPixel(lastGeoVertex);
                     translateDataToScreen(pixelVertex.mX, pixelVertex.mY, screenX1, screenY1);
                     translateDataToScreen(lastPixelVertex.mX, lastPixelVertex.mY, screenX2, screenY2);
                     //Start at valid vertex and work back one pixel at a time until bad vertex found...
                     double geoPerPixel = (geoVertexOffAxis - lastGeoVertexOffAxis)/sqrt(pow((screenX1 - screenX2), 2) +
                        pow((screenY1 - screenY2), 2));
                     lastPixelVertex = pixelVertex;
                     bool valid = vertexValid;
                     while ((valid != bVertexGoneValid) && (geoVertexOffAxis > lastGeoVertexOffAxis))
                     {
                        firstGoodPixelVertex = lastPixelVertex;
                        geoVertexOffAxis -= geoPerPixel;
                        lastPixelVertex = pRaster->convertGeocoordToPixel(geoVertex, false, &valid);
                        valid |= mbExtrapolate;

                        if (valid)
                        {
                           //Georeference says it's valid, now check if it's in the view
                           translateDataToScreen(lastPixelVertex.mX, lastPixelVertex.mY, screenX1, screenY1);
                           valid = (screenX1 > 0.0 && screenY1 > 0.0 &&
                              screenX1 < pView->width() && screenY1 < pView->height());
                        }
                     }
                     //Found which pixel along the line segment was the first valid one, add it to the vertex array.
                     vertices.push_back(firstGoodPixelVertex);
                  }

                  if (vertexValid)
                  {
                     //Mark that we have seen a valid vertex and add it to the vertex array for drawing.
                     bVertexGoneValid = true;
                     vertices.push_back(pixelVertex);
                  }
                  else //vertex not valid
                  {
                     //Mark this vertex as bad (don't draw to this vertex)
                     bVertexGoneValid = false;
                     vertices.push_back(badVertex);
                  }
               }

               // now draw the grid line
               startLabel = badVertex;
               endLabel = badVertex;
 
               if (bProductView)
               {
                  // project vertices to screen coordinates
                  GLdouble winZ;
                  for (vector<LocationType>::iterator it = vertices.begin(); it != vertices.end(); ++it)
                  {
                     if (*it != badVertex)
                     {
                        gluProject((*it).mX, (*it).mY, 0.0, modelMatrix, projectionMatrix, viewPort,
                           &(*it).mX, &(*it).mY, &winZ);
                     }
                  }
               }

               if (!vertices.empty())
               {
                  glLineWidth(1);
                  glLineWidth(mWidth);
                  glBegin (GL_LINE_STRIP);
                  vector<LocationType>::iterator it;
                  for (it = vertices.begin(); it != vertices.end(); ++it)
                  {
                     if (*it != badVertex)
                     {
                        if (startLabel == badVertex)
                        {
                           //First good vertex, save label position
                           startLabel = *it;

                        }
                        glVertex2f(it->mX, it->mY);
                        bDrewLine = true;
                        endLabel = *it;  //End label will be drawn at last good vertex
                     }
                  }
                  glEnd();

                  if (startLabel != badVertex)
                  {
                     if (bProductView)
                     {
                        DrawUtil::unProjectToZero(startLabel.mX, startLabel.mY, modelMatrix,
                           projectionMatrix, viewPort, &startLabel.mX, &startLabel.mY);
                     }

                     LocationType geoCoord = pRaster->convertPixelToGeocoord(startLabel);

                     drawLabel(startLabel, textOffset, geoCoord, lat, startBorderType, modelMatrix,
                        projectionMatrix, viewPort, bProductView);
                  }
 
                  if (endLabel != badVertex)
                  {
                     if (bProductView)
                     {
                        DrawUtil::unProjectToZero(endLabel.mX, endLabel.mY, modelMatrix,
                           projectionMatrix, viewPort, &endLabel.mX, &endLabel.mY);
                     }

                     LocationType geoCoord = pRaster->convertPixelToGeocoord(endLabel);

                     drawLabel(endLabel, textOffset, geoCoord, lat, endBorderType, modelMatrix,
                        projectionMatrix, viewPort, bProductView);
                  }
               }
            }
         }
      }
      if (!(bDrewLatLine && bDrewLonLine))
      {
         mComputedTickSpacingDirty = true;
      }
   }
   else if (mStyle == LATLONSTYLE_CROSS)
   {
      glBegin (GL_LINES);
      GLdouble winZ;

      int numDrawn = 0;
      if (haveX)
      {
         for (i = 0; i < xCount; ++i)
         {
            geoVertex.mX = start.mX + static_cast<double>(i) * tickSpacing.mX;
            for (j = 0; j < yCount; ++j)
            {
               geoVertex.mY = start.mY + static_cast<double>(j) * tickSpacing.mY;
               pixelVertex = pRaster->convertGeocoordToPixel(geoVertex,false, &vertexValid);
               vertexValid |= mbExtrapolate;
               if (vertexValid && DrawUtil::isWithin(pixelVertex, &(*mBoundingBox.begin()), 4))
               {
                  vector<LocationType> verticalSegment;
                  geoVertex.mY -= tickSpacing.mY / 20.0;
                  pixelVertex = pRaster->convertGeocoordToPixel(geoVertex);
                  verticalSegment.push_back(pixelVertex);

                  geoVertex.mY += tickSpacing.mY / 10.0;
                  pixelVertex = pRaster->convertGeocoordToPixel(geoVertex);
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
                     ++numDrawn;
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
               pixelVertex = pRaster->convertGeocoordToPixel(geoVertex, false, &vertexValid);
               vertexValid |= mbExtrapolate;
               if (vertexValid && DrawUtil::isWithin(pixelVertex, &(*mBoundingBox.begin()), 4))
               {
                  vector<LocationType> horizontalSegment;
                  geoVertex.mX -= tickSpacing.mX / 20.0;
                  pixelVertex = pRaster->convertGeocoordToPixel(geoVertex);
                  horizontalSegment.push_back(pixelVertex);

                  geoVertex.mX += tickSpacing.mX / 10.0;
                  pixelVertex = pRaster->convertGeocoordToPixel(geoVertex);
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
                     ++numDrawn;
                  }
               }
            }
         }
      }

      double worldMinX = 0.0;
      double worldMinY = 0.0;
      double worldMaxX = 0.0;
      double worldMaxY = 0.0;
      pView->getExtents(worldMinX, worldMinY, worldMaxX, worldMaxY);

      double dataMinX = 0.0;
      double dataMinY = 0.0;
      double dataMaxX = 0.0;
      double dataMaxY = 0.0;
      translateWorldToData(worldMinX, worldMinY, dataMinX, dataMinY);
      translateWorldToData(worldMaxX, worldMaxY, dataMaxX, dataMaxY);

      if (!(DrawUtil::isWithin(LocationType(dataMinX, dataMinY), &(*mBoundingBox.begin()), 4) &&
            DrawUtil::isWithin(LocationType(dataMinX, dataMaxY), &(*mBoundingBox.begin()), 4) &&
            DrawUtil::isWithin(LocationType(dataMaxX, dataMinY), &(*mBoundingBox.begin()), 4) &&
            DrawUtil::isWithin(LocationType(dataMaxX, dataMaxY), &(*mBoundingBox.begin()), 4)) ||
            numDrawn < 16)
      {
         mComputedTickSpacingDirty = true;
      }

      glEnd();
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
   setExtrapolation(false);
   setFont(LatLonLayerImp::getDefaultFont());
   setGeocoordType(GeoreferenceDescriptor::getSettingGeocoordType());
   setLatLonFormat(GeoreferenceDescriptor::getSettingLatLonFormat());
}

void LatLonLayerImp::drawLabel(const LocationType& location, const LocationType& textOffset,
                               LocationType geoCoord, bool lat, const BorderType& borderType,
                               const double modelMatrix[16], const double projectionMatrix[16], const int viewPort[4],
                               bool bProduct)
{
   //Set up the label for this lat/lon line
   string text = "";
   if (mGeocoordType == GEOCOORD_LATLON)
   {
      LatLonPoint latLonPoint(geoCoord);
      text = (lat ? latLonPoint.getLatitudeText(mFormat) : latLonPoint.getLongitudeText(mFormat));
   }
   else if (mGeocoordType == GEOCOORD_UTM || mGeocoordType == GEOCOORD_MGRS)
   {
      if (geoCoord.mX < LAT_UTMMIN)
      {
         geoCoord.mX = LAT_UTMMIN;
      }
      if (geoCoord.mX > LAT_UTMMAX)
      {
         geoCoord.mX = LAT_UTMMAX;
      }
      if (geoCoord.mY < LON_MIN)
      {
         geoCoord.mY = LON_MIN;
      }
      if (geoCoord.mY > LON_MAX)
      {
         geoCoord.mY = LON_MAX;
      }
      if (mGeocoordType == GEOCOORD_UTM)
      {
         UtmPoint utmPoint(geoCoord);
         if (lat)
         {
            text = utmPoint.getNorthingText() + " " + utmPoint.getHemisphere();
         }
         else
         {
            text = utmPoint.getEastingText() + " " + utmPoint.getZoneText();
         }
      }
      else
      {
         MgrsPoint mgrsPoint(geoCoord);
         if (lat)
         {
            text = mgrsPoint.getZoneText() + mgrsPoint.getScrCodeText() + 
               QString::number(static_cast<int>(mgrsPoint.getNorthing())).toStdString();
         }
         else
         {
            text = mgrsPoint.getZoneText() + mgrsPoint.getScrCodeText() + 
               QString::number(static_cast<int>(mgrsPoint.getEasting())).toStdString();
         }
      }
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
      dRotation = pPerspectiveView->getRotation();
      dPitch = pPerspectiveView->getPitch();
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
   offset.mY = (-dHeight/2.0 * sin(dRotation * PI / 180.0)) - (dHeight / 2.0) -
      (dPixelMargin * sin(dRotation * PI / 180.0));

   int screenX = static_cast<int>(screenCoord.mX + offset.mX);

   if (screenX < 1)
   {
      if (!bProduct)
      {
         screenX = 1;
         offset.mY -= (dHeight / 2.0 * abs(cos(dRotation * PI / 180.0)));
      }
      mComputedTickSpacingDirty = true;
   }

   if (screenX + dWidth > pView->width() - 1)
   {
      if (!bProduct)
      {
         screenX = pView->width() - dWidth - 1;
         offset.mY -= (dHeight / 2.0 * abs(cos(dRotation * PI / 180.0)));
      }
      mComputedTickSpacingDirty = true;
   }
 
   int screenY = pView->height() - static_cast<int>(screenCoord.mY + offset.mY);

   if (screenY - dHeight < 1)
   {
      if (!bProduct)
      {
         screenY = dHeight + 1;
      }
      mComputedTickSpacingDirty = true;
   }

   if (screenY > pView->height() - 1)
   {
      if (!bProduct)
      {
         screenY = pView->height() - 1;
      }
      mComputedTickSpacingDirty = true;
   }

   pView->renderText(screenX, screenY, strLabel, mFont.getQFont());
}

bool LatLonLayerImp::getExtents(double& x1, double& y1, double& x4, double& y4)
{
   RasterElement* pRaster = dynamic_cast<RasterElement*>(getDataElement());
   if (pRaster == NULL)
   {
      // This occurs during application shutdown when the overview window is open and pan limits have been disabled.
      return false;
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   translateDataToWorld(0, 0, x1, y1);
   translateDataToWorld(pDescriptor->getColumnCount(), pDescriptor->getRowCount(), x4, y4);

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

LocationType LatLonLayerImp::getTickSpacing(bool bDrawing) const
{
   if (!bDrawing)
   {
      return mUserTickSpacing;
   }

   LocationType retVal = mTickSpacing;

   if (mComputeTickSpacing)
   {
      if (mComputedTickSpacingDirty)
      {
         const_cast<LatLonLayerImp*>(this)->computeTickSpacing(bDrawing);
      }

      retVal = mComputedTickSpacing;
   }
   else if (mBorderDirty == true)
   {
      const_cast<LatLonLayerImp*>(this)->computeBorder();
   }

   return retVal;
}

bool LatLonLayerImp::getExtrapolation() const
{
   return mbExtrapolate;
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
   pXml->addAttr("extrapolate", mbExtrapolate);

   buf.str("");
   buf << mMaxCoord.mX << " " << mMaxCoord.mY;
   pXml->addAttr("maxCoord", buf.str());

   buf.str("");
   buf << mMinCoord.mX << " " << mMinCoord.mY;
   pXml->addAttr("minCoord", buf.str());

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
   mbExtrapolate = StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("extrapolate"))));
   str << A(pElement->getAttribute(X("maxCoord")));
   str >> mMaxCoord.mX >> mMaxCoord.mY;
   str.clear();
   str << A(pElement->getAttribute(X("minCoord")));
   str >> mMinCoord.mX >> mMinCoord.mY;
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
   if (spacing == mUserTickSpacing)
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
      mUserTickSpacing = spacing;
      if (mGeocoordType == GEOCOORD_LATLON)
      {
         mTickSpacing = spacing;
      }
      else
      {
         RasterElement* pRaster = dynamic_cast<RasterElement*>(getDataElement());
         if (pRaster == NULL)
         {
            mTickSpacing = spacing;
         }
         else
         {
            UtmPoint utm1(pRaster->convertPixelToGeocoord(LocationType(0.0, 0.0)));
            UtmPoint utm2(utm1.getEasting() + spacing.mX, utm1.getNorthing() + spacing.mY,
               utm1.getZone(), utm1.getHemisphere());
            mTickSpacing = utm2.getLatLonCoordinates().getCoordinates() - utm1.getLatLonCoordinates().getCoordinates();
         }
      }
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

void LatLonLayerImp::setExtrapolation(bool bExtrapolate)
{
   if (bExtrapolate == mbExtrapolate)
   {
      return;
   }

   if (mbLinking == false)
   {
      mbExtrapolate = bExtrapolate;
      emit extrapolationChanged(mbExtrapolate);

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setExtrapolation(bExtrapolate);
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

   mMaxCoord = LocationType(-numeric_limits<double>::max(), -numeric_limits<double>::max());
   mMinCoord = LocationType(numeric_limits<double>::max(), numeric_limits<double>::max());

   vector<LocationType> borderPixels;
   //If we have a bounding box, use it.  Otherwise use the cube rows/cols
   if (mBoundingBox.size() != 4)
   {
      borderPixels.push_back(LocationType(0, 0));
      borderPixels.push_back(LocationType(0, rows / 2));
      borderPixels.push_back(LocationType(0, rows));
      borderPixels.push_back(LocationType(cols / 2, rows));
      borderPixels.push_back(LocationType(cols, rows));
      borderPixels.push_back(LocationType(cols, rows / 2));
      borderPixels.push_back(LocationType(cols, 0));
      borderPixels.push_back(LocationType(cols / 2, 0));
   }
   else
   {
      double startRow = mBoundingBox[0].mY;
      double endRow = mBoundingBox[3].mY;
      double startCol = mBoundingBox[0].mX;
      double endCol = mBoundingBox[1].mX;
      double rowInc = (endRow - startRow) / ROW_SAMPLING;
      double colInc = (endCol - startCol) / COL_SAMPLING;
      for (double row = startRow; row < endRow; row += rowInc)
      {
         for (double col = startCol; col < endCol; col += colInc)
         {
            borderPixels.push_back(LocationType(col, row));
         }
      }

      borderPixels.push_back(mBoundingBox[0]);
      borderPixels.push_back(LocationType(mBoundingBox[0].mX + (mBoundingBox[3].mX - mBoundingBox[0].mX)/2.0,
         mBoundingBox[0].mY + (mBoundingBox[3].mY - mBoundingBox[0].mY) / 2.0));
      borderPixels.push_back(mBoundingBox[3]);
      borderPixels.push_back(LocationType(mBoundingBox[3].mX + (mBoundingBox[2].mX - mBoundingBox[3].mX)/2.0,
         mBoundingBox[3].mY + (mBoundingBox[2].mY - mBoundingBox[3].mY) / 2.0));
      borderPixels.push_back(mBoundingBox[2]);
      borderPixels.push_back(LocationType(mBoundingBox[1].mX + (mBoundingBox[2].mX - mBoundingBox[1].mX)/2.0,
         mBoundingBox[1].mY + (mBoundingBox[2].mY - mBoundingBox[1].mY) / 2.0));
      borderPixels.push_back(mBoundingBox[1]);
      borderPixels.push_back(LocationType(mBoundingBox[0].mX + (mBoundingBox[1].mX - mBoundingBox[0].mX)/2.0,
         mBoundingBox[0].mY + (mBoundingBox[1].mY - mBoundingBox[0].mY) / 2.0));
   }

   for (vector<LocationType>::const_iterator iter = borderPixels.begin(); iter != borderPixels.end(); ++iter)
   {
      LocationType latlon = pRaster->convertPixelToGeocoord(*iter, false);

      LocationType geoCoord;
      LatLonPoint latLonPoint(latlon);

      geoCoord = latLonPoint.getCoordinates();

      mMinCoord.mX = min(geoCoord.mX, mMinCoord.mX);
      mMinCoord.mY = min(geoCoord.mY, mMinCoord.mY);
      mMaxCoord.mX = max(geoCoord.mX, mMaxCoord.mX);
      mMaxCoord.mY = max(geoCoord.mY, mMaxCoord.mY);
   }

   if (mMinCoord.mX < LAT_MIN)
   {
      mMinCoord.mX = LAT_MIN;
   }
   if (mMinCoord.mX > LAT_MAX)
   {
      mMinCoord.mX = LAT_MAX;
   }
   if (mMaxCoord.mX > LAT_MAX)
   {
      mMaxCoord.mX = LAT_MAX;
   }
   if (mMaxCoord.mX < LAT_MIN)
   {
      mMaxCoord.mX = LAT_MIN;
   }
   if (mMinCoord.mY < LON_MIN)
   {
      mMinCoord.mY = LON_MIN;
   }
   if (mMinCoord.mY > LON_MAX)
   {
      mMinCoord.mY = LON_MAX;
   }
   if (mMaxCoord.mY > LON_MAX)
   {
      mMaxCoord.mY = LON_MAX;
   }
   if (mMaxCoord.mY < LON_MIN)
   {
      mMaxCoord.mY = LON_MIN;
   }
   mBorderDirty = false;
}

void LatLonLayerImp::computeTickSpacing(bool bDrawing)
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

   mComputedTickSpacing.mX = computeSpacing(diff.mX);
   mComputedTickSpacing.mY = computeSpacing(diff.mY);

   if (bDrawing)
   {
      mComputedTickSpacingDirty = false;
   }
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