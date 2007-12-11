/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>
#include <limits>

#include <QtGui/QApplication>

#include "AppConfig.h"
#include "glCommon.h"
#include "LatLonLayerImp.h"
#include "GraphicObjectFactory.h"
#include "DrawUtil.h"
#include "GcpList.h"
#include "GeoPoint.h"
#include "LatLonLayerAdapter.h"
#include "LatLonLayerUndo.h"
#include "LayerList.h"
#include "PerspectiveView.h"
#include "ProductView.h"
#include "PropertiesLatLonLayer.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "SpatialDataView.h"
#include "Undo.h"
#include "ViewImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

/**
  * The static methods are documented below near their definitions.
  */
static LocationType adjustSegment (LocationType outside, LocationType inside, const vector<LocationType> &border);
static double computeSpacing (double range);
static double getStep (double diff);

LatLonLayerImp::LatLonLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   LayerImp(id, layerName, pElement),
   mComputedTickSpacing(0.0, 0.0),
   mComputedTickSpacingDirty(true),
   mBorderDirty(true),
   mCubeSize(LocationType(0.0, 0.0)),
   mMinCoord(LocationType(0.0, 0.0)),
   mMaxCoord(LocationType(0.0, 0.0)),
   mFont(QApplication::font()),
   mZone(0),
   mHemisphere('N'),
   mTickSpacing(0.0, 0.0),
   mComputeTickSpacing(true)
{
   mpElement.addSignal(SIGNAL_NAME(RasterElement, GeoreferenceModified), Slot(this, &LatLonLayerImp::georeferenceModified));

   ColorType color = LatLonLayer::getSettingGridlineColor();
   mColor = COLORTYPE_TO_QCOLOR(color);
   mStyle = LatLonLayer::getSettingGridlineStyle();
   mWidth = LatLonLayer::getSettingGridlineWidth();
   mFont = LatLonLayerImp::getDefaultFont();
   mGeocoordType = LatLonLayer::getSettingGeocoordType();
   mFormat = LatLonLayer::getSettingFormat();

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
{
}

const string& LatLonLayerImp::getObjectType() const
{
   static string type("LatLonLayerImp");
   return type;
}

bool LatLonLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "LatLonLayer"))
   {
      return true;
   }

   return LayerImp::isKindOf(className);
}

void LatLonLayerImp::georeferenceModified(Subject &subject,
   const std::string &signal, const boost::any &data)
{
   setBorderDirty(true);
   setTickSpacingDirty(true);
}

LatLonLayerImp& LatLonLayerImp::operator= (const LatLonLayerImp& latLonLayer)
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
   LocationType start, stop, stepSize;
   int xCount, yCount;
   int i, j;
   LocationType stepValues[stepCount+1];
   LocationType geoVertex, pixelVertex;
   LocationType geoDraw;
   bool haveX = false, haveY = false;

   double modelMatrix[16], projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;

   bool bScreenCoords = false;

   ViewImp* pView = dynamic_cast<ViewImp*>(getView());
   if (pView != NULL)
   {
      QWidget* pParent = pView->parentWidget();
      if (pParent != NULL)
      {
         // Check to see if the view's parent widget is a view, which
         // indicates that the layer is being drawn in a product
         if (pParent->inherits("ViewImp") == true)
         {
            bScreenCoords = true;

            dMinX = viewPort[0];
            dMinY = viewPort[1];
            dMaxX = viewPort[0] + viewPort[2];
            dMaxY = viewPort[1] + viewPort[3];

            glDisable(GL_SCISSOR_TEST);

            int iWidth = ::max(1, pParent->width());
            int iHeight = ::max(1, pParent->height());

            glViewport(0, 0, iWidth, iHeight);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0, iWidth, 0, iHeight);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
         }
         else
         {
            pView->getExtents(dMinX, dMinY, dMaxX, dMaxY);
         }
      }
   }

   vector<LocationType> boundingBox;
   boundingBox.push_back(LocationType(dMinX, dMinY));
   boundingBox.push_back(LocationType(dMaxX, dMinY));
   boundingBox.push_back(LocationType(dMaxX, dMaxY));
   boundingBox.push_back(LocationType(dMinX, dMaxY));

   setBoundingBox(boundingBox);

   RasterElement *pRaster = dynamic_cast<RasterElement*>(getDataElement());
   if (pRaster == NULL)
   {
      return;
   }

   glColor4ub(mColor.red(), mColor.green(), mColor.blue(), mColor.alpha());
   glLineWidth(mWidth);

   tickSpacing = getTickSpacing ();
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
      tickSpacing.mY = computeSpacing (mMaxCoord.mY - mMinCoord.mY);
   }

   start.mX = ceil (mMinCoord.mX / tickSpacing.mX) * tickSpacing.mX;
   start.mY = ceil (mMinCoord.mY / tickSpacing.mY) * tickSpacing.mY;

   stop.mX = floor (mMaxCoord.mX / tickSpacing.mX) * tickSpacing.mX;
   stop.mY = floor (mMaxCoord.mY / tickSpacing.mY) * tickSpacing.mY;

   stepSize.mX = (mMaxCoord.mX - mMinCoord.mX) / (double)(stepCount-1);
   stepSize.mY = (mMaxCoord.mY - mMinCoord.mY) / (double)(stepCount-1);

   xCount = 1.5 + (stop.mX - start.mX) / tickSpacing.mX;
   yCount = 1.5 + (stop.mY - start.mY) / tickSpacing.mY;

   if (mStyle == LATLONSTYLE_SOLID)
   {
      bool onOff = false;
      bool isInData = false;
      LocationType oldVertex;

      for (j=0; j<stepCount; j++)
      {
         stepValues[j].mX = mMinCoord.mX + (double)j * stepSize.mX;
         stepValues[j].mY = mMinCoord.mY + (double)j * stepSize.mY;
      }

      if (haveX)
      {
         for (i=0; i<xCount; i++)
         {
            onOff = false;
            isInData = false;
            geoVertex.mX = start.mX + (double)i * tickSpacing.mX;

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

            for (j=0; j<stepCount; j++)
            {
               geoVertex.mY = stepValues[j].mY;

               LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
               pixelVertex = pRaster->convertGeocoordToPixel (geoDraw);
               pixelVertex.mX += 0.5;
               pixelVertex.mY += 0.5;
               if (bScreenCoords == true)
               {
                  double dScreenX = 0;
                  double dScreenY = 0;
                  GLdouble winZ;
                  gluProject(pixelVertex.mX, pixelVertex.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
                     &dScreenX, &dScreenY, &winZ);

                  pixelVertex.mX = dScreenX;
                  pixelVertex.mY = dScreenY;
               }
               isInData = (pixelVertex.mX>=0.0 && pixelVertex.mY>=0.0) && 
                  (pixelVertex.mX<=mCubeSize.mX && pixelVertex.mY<=mCubeSize.mY);
               isInData = DrawUtil::isWithin(pixelVertex, &(*mBoundingBox.begin()), 4);
               if (onOff == true)
               {
                  if (!isInData)
                  {
                     // adjust pixelVertex to be the intersection of the segment with the edge of the data
                     pixelVertex = adjustSegment (pixelVertex, oldVertex, mBoundingBox);
                     glVertex2f(pixelVertex.mX, pixelVertex.mY);
                     glEnd ();

                     drawLabel(pixelVertex, bScreenCoords, xLabel, getNearestBorder(pixelVertex));
                     onOff = false;
                  }
                  else
                  {
                     glVertex2f (pixelVertex.mX, pixelVertex.mY);
                  }
               }
               else // onOff == false
               {
                  if (isInData)
                  {
                     onOff = true;
                     glLineWidth(1);
                     glLineWidth(mWidth);
                     if (j != 0)
                     {
                        // adjust oldVertex to be intersection of the segment with the edge of the data
                        oldVertex = adjustSegment (oldVertex, pixelVertex, mBoundingBox);
                        drawLabel(oldVertex, bScreenCoords, xLabel, getNearestBorder(pixelVertex));
                        glBegin (GL_LINE_STRIP);
                        glVertex2f (oldVertex.mX, oldVertex.mY);
                          glVertex2f (pixelVertex.mX, pixelVertex.mY);
                     }
                     else // j == 0
                     {
                        drawLabel(pixelVertex, bScreenCoords, xLabel, getNearestBorder(pixelVertex));
                        glBegin (GL_LINE_STRIP);
                        glVertex2f (pixelVertex.mX, pixelVertex.mY);
                     }
                  }
               }
               oldVertex.mX = pixelVertex.mX;
               oldVertex.mY = pixelVertex.mY;
            } // for (j=0...
            if (onOff == true) 
            {
               glEnd ();
               drawLabel(oldVertex, bScreenCoords, xLabel, getNearestBorder(oldVertex));
            }
         }
      }

      if (haveY)
      {
         for (i=0; i<yCount; i++)
         {
            onOff = false;
            isInData = false;
            geoVertex.mY = start.mY + (double)i * tickSpacing.mY;

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

            for (j=0; j<stepCount; j++)
            {
               geoVertex.mX = stepValues[j].mX;
               LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
               pixelVertex = pRaster->convertGeocoordToPixel (geoDraw);
               pixelVertex.mX += 0.5;
               pixelVertex.mY += 0.5;
               if (bScreenCoords == true)
               {
                  double dScreenX = 0;
                  double dScreenY = 0;
                  GLdouble winZ;
                  gluProject(pixelVertex.mX, pixelVertex.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
                     &dScreenX, &dScreenY, &winZ);

                  pixelVertex.mX = dScreenX;
                  pixelVertex.mY = dScreenY;
               }
               isInData = (pixelVertex.mX>=0.0 && pixelVertex.mY>=0.0) && 
                  (pixelVertex.mX<=mCubeSize.mX && pixelVertex.mY<=mCubeSize.mY);
               isInData = DrawUtil::isWithin(pixelVertex, &(*mBoundingBox.begin()), 4);
               if (onOff == true)
               {
                  if (!isInData)
                  {
                     // adjust pixelVertex to be the intersection of the segment with the edge of the data
                     pixelVertex = adjustSegment (pixelVertex, oldVertex, mBoundingBox);
                     glVertex2f(pixelVertex.mX, pixelVertex.mY);
                     glEnd ();

                     drawLabel(pixelVertex, bScreenCoords, yLabel, getNearestBorder(pixelVertex));

                     onOff = false;
                  }
                  else
                  {
                     glVertex2f (pixelVertex.mX, pixelVertex.mY);
                  }
               }
               else
               {
                  if (isInData)
                  {
                     onOff = true;
                     string str = convertGcpValueToText (geoVertex.mY, false, mGeocoordType);
                     glLineWidth(1);
                     glLineWidth(mWidth);
                     if (j != 0)
                     {
                        // adjust oldVertex to be intersection of the segment with the edge of the data
                        oldVertex = adjustSegment (oldVertex, pixelVertex, mBoundingBox);
                        drawLabel(oldVertex, bScreenCoords, yLabel, getNearestBorder(pixelVertex));
                        glBegin (GL_LINE_STRIP);
                          glVertex2f (oldVertex.mX, oldVertex.mY);
                        glVertex2f (pixelVertex.mX, pixelVertex.mY);
                     }
                     else
                     {
                        drawLabel(pixelVertex, bScreenCoords, yLabel, getNearestBorder(pixelVertex));
                        glBegin (GL_LINE_STRIP);
                        glVertex2f (pixelVertex.mX, pixelVertex.mY);
                     }
                  }
               }
               oldVertex.mX = pixelVertex.mX;
               oldVertex.mY = pixelVertex.mY;
            }
            if (onOff == true) 
            {
               glEnd ();
               drawLabel(oldVertex, bScreenCoords, yLabel, getNearestBorder(oldVertex));
            }
         }
      }
   }
   else if (mStyle == LATLONSTYLE_CROSS)
   {
      glBegin (GL_LINES);
      if (haveX)
      {
         for (i=0; i<xCount; i++)
         {
            geoVertex.mX = start.mX + (double)i * tickSpacing.mX;
            for (j=0; j<yCount; j++)
            {
               geoVertex.mY = start.mY + (double)j * tickSpacing.mY;
               LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
               pixelVertex = pRaster->convertGeocoordToPixel (geoDraw);
               if ((pixelVertex.mX>=0.0 && pixelVertex.mY>=0.0) && 
                  (pixelVertex.mX<=mCubeSize.mX && pixelVertex.mY<=mCubeSize.mY))
               {
                  geoVertex.mY = start.mY + (double)j * tickSpacing.mY - tickSpacing.mY/20.0;
                  LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel (geoDraw);
                  glVertex2f (pixelVertex.mX + 0.5, pixelVertex.mY + 0.5);

                  geoVertex.mY += tickSpacing.mY/10.0;
                  geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel (geoDraw);
                  glVertex2f (pixelVertex.mX + 0.5, pixelVertex.mY + 0.5);
               }
            }
         }
      }

      if (haveY)
      {
         for (i=0; i<yCount; i++)
         {
            geoVertex.mY = start.mY + (double)i * tickSpacing.mY;
            for (j=0; j<xCount; j++)
            {
               geoVertex.mX = start.mX + (double)j * tickSpacing.mX;
               LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
               pixelVertex = pRaster->convertGeocoordToPixel (geoDraw);
               if ((pixelVertex.mX>=0.0 && pixelVertex.mY>=0.0) && 
                  (pixelVertex.mX<=mCubeSize.mX && pixelVertex.mY<=mCubeSize.mY))
               {
                  geoVertex.mX = start.mX + (double)j * tickSpacing.mX - tickSpacing.mX/20.0;
                  LocationType geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel (geoDraw);
                  glVertex2f (pixelVertex.mX + 0.5, pixelVertex.mY + 0.5);

                  geoVertex.mX += tickSpacing.mX/10.0;
                  geoDraw = convertPointToLatLon(mGeocoordType, geoVertex);
                  pixelVertex = pRaster->convertGeocoordToPixel (geoDraw);
                  glVertex2f (pixelVertex.mX + 0.5, pixelVertex.mY + 0.5);
               }
            }
         }
      }

      glEnd ();
   }

   glLineWidth(1.0);

   if (bScreenCoords == true)
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

LatLonLayerImp::BorderType LatLonLayerImp::getNearestBorder(const LocationType& location) const
{
   const BorderType sides[] = {BOTTOM_BORDER, RIGHT_BORDER, TOP_BORDER, LEFT_BORDER};
   double minDistance = 1e38;
   BorderType best;
   vector<BorderType> borders;
   borders.push_back(LEFT_BORDER);
   borders.push_back(RIGHT_BORDER);
   borders.push_back(BOTTOM_BORDER);
   borders.push_back(TOP_BORDER);
   if (mBoundingBox.size() == 4)
   {
      for (vector<BorderType>::iterator iter = borders.begin(); iter != borders.end(); ++iter)
      {
         double distance = DrawUtil::linePointDistance(mBoundingBox[*iter], mBoundingBox[(*iter+1)%4], location, NULL);
         if (distance < minDistance)
         {
            minDistance = distance;
            best = *iter;
         }
      }
   }

   best = sides[best];

   return best;
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

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         Layer* pLayer = NULL;
         pLayer = *iter;
         if (pLayer != NULL)
         {
            if (pLayer->isKindOf("LatLonLayer") == true)
            {
               ((LatLonLayer*) pLayer)->setLatLonFormat(newFormat);
            }
         }

         ++iter;
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

void LatLonLayerImp::drawLabel(const LocationType& location, bool bScreenCoords, const string& text,
                               const BorderType& borderType)
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

   double modelMatrix[16], projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   LocationType screenCoord = location;
   if (bScreenCoords == false)
   {
      GLdouble winZ;
      gluProject(location.mX, location.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
         &screenCoord.mX, &screenCoord.mY, &winZ);
   }

   QFontMetrics fontMetrics(mFont.getQFont());
   double dWidth = fontMetrics.width(strLabel);
   double dHeight = fontMetrics.ascent();

   LocationType offset;
   double dRotation = 0.0;
   double dPitch = 0.0;

   // Check to see if the view's parent widget is a perspective view,
   // which indicates that the layer is being drawn in a product
   PerspectiveView* pPerspectiveView = dynamic_cast<PerspectiveView*> (pView->parentWidget());
   if (pPerspectiveView == NULL)
   {
      pPerspectiveView = dynamic_cast<PerspectiveView*> (pView);
   }

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

   // Update the rotation angle for the pitch only if using world coordinates,
   // since Qt always uses the same origin location for the screen coordinates
   if ((dPitch < 0.0) && (bScreenCoords == false))
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
   int screenY = static_cast<int>(textLocation.mY);

   // Since the text is drawn outside of the view object area in a product view,
   // map the product view screen coordinates to spatial data view coordinates
   // if drawing the layer in a product
   if (dynamic_cast<ViewImp*>(pView->parentWidget()) != NULL)
   {
      QPoint screenCoord = pView->mapFromParent(QPoint(screenX, screenY));
      screenX = screenCoord.x();
      screenY = screenCoord.y();
   }

   pView->renderText(screenX, pView->height() - screenY, strLabel, mFont.getQFont());
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
         ((LatLonLayerImp*) this)->computeTickSpacing();

      return mComputedTickSpacing;
   }
   else if (mBorderDirty == true)
   {
      ((LatLonLayerImp*) this)->computeBorder();
   }

   return mTickSpacing;
}

bool LatLonLayerImp::getAutoTickSpacing() const
{
   return mComputeTickSpacing;
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
   if(!LayerImp::toXml(pXml))
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

   return true;;
}

bool LatLonLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   stringstream str;
   DO_IF(!LayerImp::fromXml(pDocument, version), return false);
   DOMElement *pElement = dynamic_cast<DOMElement*>(pDocument);
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
      notify(SIGNAL_NAME(LatLonLayer, ColorChanged), boost::any(
         ColorType(mColor.red(),mColor.green(),mColor.blue())));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         Layer* pLayer = NULL;
         pLayer = *iter;
         if (pLayer != NULL)
         {
            if (pLayer->isKindOf("LatLonLayer") == true)
            {
               ((LatLonLayerAdapter*) pLayer)->LatLonLayerImp::setColor(newColor);
            }
         }

         ++iter;
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

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         LatLonLayerImp* pLayer = dynamic_cast<LatLonLayerImp*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setFont(font);
         }

         ++iter;
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

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         Layer* pLayer = NULL;
         pLayer = *iter;
         if (pLayer != NULL)
         {
            if (pLayer->isKindOf("LatLonLayer") == true)
            {
               ((LatLonLayer*) pLayer)->setStyle(newStyle);
            }
         }

         ++iter;
      }

      mbLinking = false;
   }
}

void LatLonLayerImp::setWidth(unsigned int uiWidth)
{
   if (uiWidth == mWidth)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetLatLonWidth(dynamic_cast<LatLonLayer*>(this), mWidth, uiWidth));
      }

      mWidth = uiWidth;
      emit widthChanged(mWidth);
      notify(SIGNAL_NAME(LatLonLayer, WidthChanged), boost::any(mWidth));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         Layer* pLayer = NULL;
         pLayer = *iter;
         if (pLayer != NULL)
         {
            if (pLayer->isKindOf("LatLonLayer") == true)
            {
               ((LatLonLayer*) pLayer)->setWidth(uiWidth);
            }
         }

         ++iter;
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

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         Layer* pLayer = NULL;
         pLayer = *iter;
         if (pLayer != NULL)
         {
            if (pLayer->isKindOf("LatLonLayer") == true)
            {
               ((LatLonLayer*) pLayer)->setTickSpacing(spacing);
            }
         }

         ++iter;
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

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         Layer* pLayer = NULL;
         pLayer = *iter;
         if (pLayer != NULL)
         {
            if (pLayer->isKindOf("LatLonLayer") == true)
            {
               ((LatLonLayer*) pLayer)->setAutoTickSpacing(compute);
            }
         }

         ++iter;
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

   const RasterDataDescriptor* pDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   double rows = pDescriptor->getRowCount();
   double cols = pDescriptor->getColumnCount();

   mZone = 100;
   mCubeSize.mX = cols;
   mCubeSize.mY = rows;
   mMaxCoord = LocationType (-numeric_limits<double>::max(), -numeric_limits<double>::max());
   mMinCoord = LocationType (numeric_limits<double>::max(), numeric_limits<double>::max());

   vector<LocationType> borderPixels;
   borderPixels.push_back(LocationType(0,      0));
   borderPixels.push_back(LocationType(0,      rows/2));
   borderPixels.push_back(LocationType(0,      rows));
   borderPixels.push_back(LocationType(cols/2, rows));
   borderPixels.push_back(LocationType(cols,   rows));
   borderPixels.push_back(LocationType(cols,   rows/20));
   borderPixels.push_back(LocationType(cols,   0));
   borderPixels.push_back(LocationType(cols/2,0));

   for (vector<LocationType>::const_iterator iter = borderPixels.begin();
      iter != borderPixels.end(); ++iter)
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

void LatLonLayerImp::computeTickSpacing ()
{
   LocationType diff;

   if (mBorderDirty)
      computeBorder ();
   if (mBorderDirty)
      return;
   
//NOTE: new code for UTM/MGRS
   diff.mX = mMaxCoord.mX - mMinCoord.mX;
   diff.mY = mMaxCoord.mY - mMinCoord.mY;

   if (mGeocoordType== GEOCOORD_LATLON)
   {
      mComputedTickSpacing.mX = computeSpacing (diff.mX);
      mComputedTickSpacing.mY = computeSpacing (diff.mY);
   }
   else
   {
      mComputedTickSpacing.mX = getStep (diff.mX);
      mComputedTickSpacing.mY = getStep (diff.mY);
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
static LocationType adjustSegment (LocationType outside, LocationType inside, const vector<LocationType> &border)
{
   enum { BELOW, BETWEEN, ABOVE } xRegion = BETWEEN, yRegion = BETWEEN;
   enum { XABOVE, XBELOW, YABOVE, YBELOW } intersectLine = XBELOW;
   LocationType intersection (0.0, 0.0);
   LocationType delta (0.0, 0.0);
   double f;

   if (outside.mX < border[0].mX)
      xRegion = BELOW;
   else if (outside.mX > border[1].mX)
      xRegion = ABOVE;

   if (outside.mY < border[1].mY)
      yRegion = BELOW;
   else if (outside.mY > border[2].mY)
      yRegion = ABOVE;

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
               if (outside.mY + delta.mY*(border[0].mX-outside.mX)/delta.mX >= border[1].mY)
                  intersectLine = XBELOW;
               else
                  intersectLine = YBELOW;
            }
            else // yRegion == ABOVE: region 1
            {
               if (outside.mY + delta.mY*(border[0].mX-outside.mX)/delta.mX <= border[2].mY)
                  intersectLine = XBELOW;
               else
                  intersectLine = YABOVE;
            }
         }
         else // xRegion == ABOVE // region 3 or 9
         {
            if (yRegion == BELOW) // region 9
            {
               if (inside.mY + delta.mY*(border[1].mX-inside.mX)/delta.mX >= border[1].mY)
                  intersectLine = XABOVE;
               else
                  intersectLine = YBELOW;
            }
            else // yRegion == ABOVE: region 3
            {
               if (inside.mY + delta.mY*(border[1].mX-inside.mX)/delta.mX <= border[2].mY)
                  intersectLine = XABOVE;
               else
                  intersectLine = YABOVE;
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
      intersection.mY = f*delta.mY + inside.mY;
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
static double computeSpacing (double range)
{
   double baseValues[] = {60.0, 30.0, 15.0, 10.0, 5.0, 2.0, 1.0};
   int baseValueCount = sizeof (baseValues)/sizeof (baseValues[0]);
   double spacing = 0.0;
   int i;

   // Treat baseValues as degrees. Break if one of them 
   // would result in 5+ tick marks.
   for (i=0; i<baseValueCount; i++)
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
      for (i=0; i<baseValueCount; i++)
      {
         if (5.0 * (baseValues[i] / 60.0) < range)
         {
            spacing = baseValues[i] / 60.0;
            break;
         }
      }

      if (i == baseValueCount)
      {
         for (i=0; i<baseValueCount; i++)
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
static double getStep (double diff)
{
   double logdiff, scaleddiff;
   double powdiff;
  
   logdiff = floor (log10 (diff));
   powdiff = pow (10.0, logdiff);
   scaleddiff = diff / powdiff;
  
   if (scaleddiff < 0.95)
      return powdiff * 0.1;
   if (scaleddiff < 1.9)
      return 0.2 * powdiff;
   if (scaleddiff < 4.75)
      return 0.5 * powdiff;
    
   return powdiff;
}

void LatLonLayerImp::setBoundingBox(const vector<LocationType> &boundingBox)
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
   switch(type)
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
         sprintf(buffer, "%d%s%05d%05d", mZone, mMapGrid.c_str(), (int)point.mX, (int)point.mY);

         MgrsPoint mgrsPoint(buffer);
         LatLonPoint latLonPoint = mgrsPoint.getLatLonCoordinates();
         coord = latLonPoint.getCoordinates();
         break;
      }
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

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         Layer* pLayer = NULL;
         pLayer = *iter;
         if (pLayer != NULL)
         {
            if (pLayer->isKindOf("LatLonLayer") == true)
            {
               ((LatLonLayer*) pLayer)->setGeocoordType(eGeocoord);
            }
         }

         ++iter;
      }

      mbLinking = false;
   }
}

const FontImp& LatLonLayerImp::getFontImp() const
{
   return mFont;
}
