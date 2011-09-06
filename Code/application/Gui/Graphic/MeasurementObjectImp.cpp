/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QFontMetrics>

#include "AppConfig.h"
#include "DrawUtil.h"
#include "GeoAlgorithms.h"
#include "GeoConversions.h"
#include "Georeference.h"
#include "glCommon.h"
#include "GraphicLayer.h"
#include "Layer.h"
#include "LayerList.h"
#include "MeasurementLayer.h"
#include "MeasurementLayerImp.h"
#include "MeasurementObjectImp.h"
#include "SpatialDataView.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "Units.h"

#include <string>
using namespace std;
XERCES_CPP_NAMESPACE_USE

MeasurementObjectImp::MeasurementObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                           LocationType pixelCoord) :
   LineObjectImp(id, type, pLayer, pixelCoord),
   mUsingInaccurateGeocoords(false)
{
   // Properties used by MeasurementObject
   addProperty("LineColor");
   addProperty("LineWidth");
   addProperty("Font");
   addProperty("FontSize");
   addProperty("TextColor");

   mpGeoreference.addSignal(SIGNAL_NAME(RasterElement, GeoreferenceModified), 
      Slot(this, &MeasurementObjectImp::georeferenceModified));

   // Default Graphic Object Values
   string face = MeasurementLayer::getSettingTextFont();
   int iSize = MeasurementLayer::getSettingTextFontSize();
   bool bBold = MeasurementLayer::getSettingTextBold();
   bool bItalic = MeasurementLayer::getSettingTextItalics();
   bool bUnderline = MeasurementLayer::getSettingTextUnderline();
   QFont font(QString::fromStdString(face), iSize);
   font.setBold(bBold);
   font.setItalic(bItalic);
   font.setUnderline(bUnderline);

   setLineColor(MeasurementLayer::getSettingLineColor());
   setLineWidth(MeasurementLayer::getSettingLineWidth());
   setTextColor(MeasurementLayer::getSettingTextColor());
   setFont(font);

   // Set default member variable values
   mArrowHeadOffset = 8;
   mArrowRelStartLoc = 0.5;
   mArrowRelEndLoc = 0.95;
   mArrowOffset = 8;
   mBarEndLength = 12;
   mTextOffset = 2;
   mBearingText = "";
   mDistanceText = "";
   mStartLocText = "";
   mEndLocText = "";

   mBearingPrecision = MeasurementLayer::getSettingBearingPrecision();
   mDistancePrecision = MeasurementLayer::getSettingDistancePrecision();
   mEndPointsPrecision = MeasurementLayer::getSettingEndPointsPrecision();

   setHitToleranceFactor(16.0);
   setUseHitTolerance(true);
}

MeasurementObjectImp::~MeasurementObjectImp()
{
}

void MeasurementObjectImp::draw(double zoomFactor) const
{
   const_cast<MeasurementObjectImp*>(this)->updateGeoreferenceAttachment();

   // Verify that factor values are valid
   if ((mArrowRelStartLoc < 0.0) || (mArrowRelStartLoc > mArrowRelEndLoc) || (mArrowRelStartLoc > 1.0) ||
      (mArrowRelEndLoc < 0.0) || (mArrowRelEndLoc > 1.0) || (mBarEndLength < 0))
   {
      return;
   }
    
   // Draw the main line
   LineObjectImp::draw(zoomFactor);
   
   // Get the current view 
   SpatialDataView* pView = NULL;

   MeasurementLayerImp* pLayer = dynamic_cast<MeasurementLayerImp*>(getLayer());
   if (pLayer != NULL)
   {
      pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
   }

   if (pView == NULL)
   {
      return;
   }

   // Misc Variables
   LocationType junkLocation;    // junk variable used to call methods that require unneeded parameters
   double startPerc = 0.0;       // The relative start location along a line to start drawing
   double lineLength = 0;        // The length of the main line
   double pixelSize = 0;         // The number of screen pixels in a scene pixel
   double lineWidth = 0;         // The width of the main line
   double sqrtLineWidth = 0;     // The square root of the line width
   LocationType llCorner;        // lower left corner of annotation bounding box
   LocationType urCorner;        // upper right corner of annotation bounding box
   ColorType textColor;          // The color to draw the text
   ColorType lineColor;          // The color to draw the line
   ColorType fillColor;          // The color to draw the stippled line

   // Misc Calculations
   pixelSize = DrawUtil::getPixelSize(junkLocation.mX, junkLocation.mY, junkLocation.mX, junkLocation.mY);
   llCorner = getLlCorner();
   urCorner = getUrCorner();
   lineWidth = getLineWidth();
   sqrtLineWidth = sqrt(lineWidth);
   textColor = getTextColor();
   lineLength = sqrt(pow(abs(urCorner.mX - llCorner.mX), 2) + pow(abs(urCorner.mY - llCorner.mY), 2));
   lineColor = getLineColor();
   fillColor = getFillColor();

   // Get text font info (used for all text, set to italic if using inaccurate extrapolation)
   QFont font = getFont();
   font.setItalic(mUsingInaccurateGeocoords);

   // Calculate arrow info (line only)
   LocationType arrowStartPoint; // The start point of the arrow line
   LocationType arrowEndPoint;   // The end point of the arrow line
   double arrowOffset = 0;       // Normal offset from main line to arrow
   double arrowLength = 0;       // The length of the arrow line (in pixels)
   arrowOffset = (pixelSize == 0.0) ? 0.0 : mArrowOffset * sqrtLineWidth / pixelSize;
   DrawUtil::getParallelLine(llCorner, urCorner, arrowOffset, mArrowRelStartLoc, mArrowRelEndLoc,
      arrowStartPoint, arrowEndPoint);
   arrowLength = sqrt(pow(abs(arrowEndPoint.mX - arrowStartPoint.mX), 2) + 
      pow(abs(arrowEndPoint.mY - arrowStartPoint.mY), 2));

   // Calculate arrow head info (Only half arrow head is drawn)
   LocationType arrowHeadBasePoint; // Location of arrow head base point
   double arrowHeadOffset = 0;      // Perpendicular offset from arrow line to arrow head base
   arrowHeadOffset = (pixelSize == 0.0) ? 0.0 : (mArrowHeadOffset * sqrtLineWidth) / pixelSize;
   
   // Adjust arrow head size if arrow length becomes smaller then arrow head length.
   while (arrowHeadOffset > arrowLength)
   {  // Since arrow head is at 45 degree angle, arrowHeadOffset is same as arrow head length

      // Adjust size of arrow head
      if (arrowHeadOffset >= 1)
      {
         arrowHeadOffset -= 1;
      }
      else if (arrowHeadOffset > .2)
      {
         arrowHeadOffset = arrowHeadOffset - .1;
      }
      else
      {
         break;
      }
      arrowHeadOffset = (arrowHeadOffset < 0) ? 0.0 : arrowHeadOffset;
   }
   
   // Get arrow head base point coordinates from calculated arrow head info
   startPerc = (arrowLength == 0.0) ? 0.0 : 1 - (arrowHeadOffset/arrowLength);
   startPerc = (startPerc < 0.0) ? 0.0 : startPerc;
   startPerc = (startPerc > 1.0) ? 1.0 : startPerc;
   DrawUtil::getParallelLine(arrowStartPoint, arrowEndPoint, arrowHeadOffset, startPerc, 1.0f,
                             arrowHeadBasePoint, junkLocation);

   // End bar coordinates
   LocationType barStartPoint1;
   LocationType barStartPoint2;     // The points that make up the start bar-end (ll corner)
   LocationType barEndPoint1;
   LocationType barEndPoint2;       // The points that make up the end bar-end (ur corner)
   double barLength = 0;            // Bar-ends length
   barLength = (pixelSize == 0.0) ? 0.0 : (mBarEndLength * sqrtLineWidth) / pixelSize;
   DrawUtil::getPerpendicularLine(llCorner, urCorner, barLength, barStartPoint1, barStartPoint2);
   DrawUtil::getPerpendicularLine(urCorner, llCorner, barLength, barEndPoint1, barEndPoint2);

   // Calculate text info
   double textOffset = 0;     // Perpendicular offset from text to text anchor
   int maxTextureSize = 0;    // The max allowable texture size
   textOffset = (pixelSize == 0.0) ? 0.0 : (mTextOffset * sqrtLineWidth) / pixelSize;
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
   QFontMetrics ftMetrics(font);
  
   double viewRotation = pView->getRotation();
   viewRotation = GeoConversions::convertDegToRad(viewRotation);

   if (mDrawnDistanceUnit != pLayer->getDistanceUnits() ||
      mDrawnGeocoord != pLayer->getGeocoordType() ||
      mDrawnDmsFormat != pLayer->getGeoFormat())
   {
      refreshGeoInformation(); // only modifies cache stuff
   }

   if (pLayer->getDisplayEndPoints())
   {
      // Calculate start and end text info
      bool startLocDrawTop = false; // Whether to draw the start lat/lon text on the top of the specified point
      bool endLocDrawTop = true;    // Whether to draw the end lat/lon text on the top of the specified point
      if (urCorner.mY < llCorner.mY)
      {
         startLocDrawTop = true;
         endLocDrawTop = false;
      }

      // Calculate start and end location text info
      LocationType startLocPoint;   // The location to display the "start location" text
      LocationType endLocPoint;     // The location to display the "end location" text
      startLocPoint = llCorner;
      endLocPoint = urCorner;
      startLocPoint.mY += textOffset;
      endLocPoint.mY += textOffset;

      if (!mStartLocText.isEmpty())
      {
         DrawUtil::drawRotatedText(mStartLocTextTexture, mStartLocText, font,
            textColor, startLocPoint, viewRotation, startLocDrawTop);
      }
      if (!mEndLocText.isEmpty())
      {
         DrawUtil::drawRotatedText(mEndLocTextTexture, mEndLocText, font,
            textColor, endLocPoint, viewRotation, endLocDrawTop);
      }
   }
   
   if (pLayer->getDisplayBearing())
   {
      // Calculate bearing text info
      LocationType bearingTextStartPoint; // The location to display the bearing text
      LocationType bearingTextEndPoint;   // The pseudo end location of the baring text (only used to calculate angle)
      double bearingTextTheta = 0;        // Angle (in radians) of bearing text
      bool bearingDrawTop = false;        // The vertical origin to draw text from (True = top, False = bottom)
      QRect bearingTextBoundingBox;       // Bounding box surrounding the bearing text
      bearingTextBoundingBox = ftMetrics.boundingRect(0, 0, maxTextureSize, maxTextureSize, 
         Qt::AlignLeft | Qt::TextWordWrap, mBearingText);
      if (arrowEndPoint.mX < arrowStartPoint.mX)   // To the left of the origin (text underneath line)
      {
         startPerc = ((pixelSize == 0.0) || (arrowLength == 0.0)) ? 0.0 :
            1.0 - (bearingTextBoundingBox.width()/pixelSize)/arrowLength;
         startPerc = (startPerc < 0.0) ? 0.0 : startPerc;
         startPerc = (startPerc > 1.0) ? 1.0 : startPerc;
         DrawUtil::getParallelLine(arrowEndPoint, arrowStartPoint, (-1) * textOffset, startPerc, 1.0f,
            bearingTextStartPoint, bearingTextEndPoint);
         bearingDrawTop = true;
      } 
      else   // To the right of the origin (text on top of the line)
      {
         DrawUtil::getParallelLine(arrowStartPoint, arrowEndPoint, textOffset, 0.0f, 1.0f,
            bearingTextStartPoint, bearingTextEndPoint);
         bearingDrawTop = false;
      }
      bearingTextTheta = ((bearingTextEndPoint.mX - bearingTextStartPoint.mX) == 0.0) ? 0.0 :
         bearingTextTheta = atan((bearingTextEndPoint.mY - bearingTextStartPoint.mY)/
         (bearingTextEndPoint.mX - bearingTextStartPoint.mX));

      if (!mBearingText.isEmpty())
      {
         DrawUtil::drawRotatedText(mBearingTextTexture, mBearingText, font,
            textColor, bearingTextStartPoint, bearingTextTheta, bearingDrawTop);
      }  
   }
   
   if (pLayer->getDisplayDistance())
   {
      // Calculate distance text info
      LocationType distanceTextStartPoint;   // The location to display the distance text
      LocationType distanceTextEndPoint;     // The pseudo end location of the distance text (only used to calculate
                                             // angle)
      double distanceTextTheta = 0;          // Angle (in radians) of distnace text
      bool distanceDrawTop = false;          // The vertical origin to draw text from (True = top, False = bottom)
      double distanceTextWidth = 0;          // The width of the ditance text bounding box (in screen pixels)
      QRect distanceTextBoundingBox;         // Bounding box that surrounds the distance text
      distanceTextBoundingBox = ftMetrics.boundingRect(0, 0, maxTextureSize, maxTextureSize, 
         Qt::AlignLeft | Qt::TextWordWrap, mDistanceText);
      distanceTextWidth = distanceTextBoundingBox.width();
      if ((pixelSize == 0.0) || (lineLength == 0.0))
      {
         startPerc = 0.0;
      } 
      else
      {
         if (urCorner.mX < llCorner.mX)
         {
            startPerc = .5 + (distanceTextWidth/pixelSize/2)/lineLength;
            distanceDrawTop = false;
         }
         else
         {
            startPerc = .5 - (distanceTextWidth/pixelSize/2)/lineLength;
            distanceDrawTop = true;
         }
      }
      startPerc = (startPerc < 0.0) ? 0.0 : startPerc;
      startPerc = (startPerc > 1.0) ? 1.0 : startPerc;
      DrawUtil::getParallelLine(llCorner, urCorner, (-1) * textOffset, startPerc, 1.0f, distanceTextStartPoint,
         distanceTextEndPoint);
      if (lineLength < (distanceTextBoundingBox.width()/pixelSize))
      {
         if (urCorner.mX < llCorner.mX)
         {
            DrawUtil::getParallelLine(llCorner, urCorner, (-1) * textOffset, 0.0f, 1.0f, distanceTextEndPoint,
               distanceTextStartPoint);
         } 
         else
         {
            DrawUtil::getParallelLine(llCorner, urCorner, (-1) * textOffset, 0.0f, 1.0f, distanceTextStartPoint,
               distanceTextEndPoint);
         }

      }
      distanceTextTheta = ((distanceTextEndPoint.mX - distanceTextStartPoint.mX) == 0.0) ? 0.0 :
         distanceTextTheta = atan((distanceTextEndPoint.mY - distanceTextStartPoint.mY)/
         (distanceTextEndPoint.mX - distanceTextStartPoint.mX));

      if (!mDistanceText.isEmpty())
      {
         DrawUtil::drawRotatedText(mDistanceTextTexture, mDistanceText, font,
            textColor, distanceTextStartPoint, distanceTextTheta, distanceDrawTop);
      }
   }

   // GL options
   glLineWidth(lineWidth);
   #if defined(WIN_API)
      glEnable(GL_LINE_SMOOTH);
   #else
      if (lineWidth == 1.0)
      {
         glEnable(GL_LINE_SMOOTH);
      }
      else
      {
         glDisable(GL_LINE_SMOOTH);
      }
   #endif
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glColor3ub(lineColor.mRed, lineColor.mGreen, lineColor.mBlue);
   
   // Draw the end bar
   glBegin(GL_LINE_STRIP);
   glVertex2d(barEndPoint1.mX, barEndPoint1.mY);
   glVertex2d(barEndPoint2.mX, barEndPoint2.mY);
   glEnd();

   // Draw the start bar
   glBegin(GL_LINE_STRIP);
   glVertex2d(barStartPoint1.mX, barStartPoint1.mY);
   glVertex2d(barStartPoint2.mX, barStartPoint2.mY);
   glEnd();

   // Draw the arrow if bearing is displayed
   if (pLayer->getDisplayBearing())
   {
      glBegin(GL_LINE_STRIP);
      glVertex2d(arrowStartPoint.mX, arrowStartPoint.mY);
      glVertex2d(arrowEndPoint.mX, arrowEndPoint.mY);
      glVertex2d(arrowHeadBasePoint.mX, arrowHeadBasePoint.mY);
      glEnd();
   }

   // Set GL options to draw stippled line (Main line)
   glEnable(GL_LINE_STIPPLE);
   glLineStipple(3, 0xf0f0);
   glColor3ub(fillColor.mRed, fillColor.mGreen, fillColor.mBlue);

   // Draw the stippled line (on top of regular line)
   glBegin(GL_LINE_STRIP);
   glVertex2d(llCorner.mX, llCorner.mY);
   glVertex2d(urCorner.mX, urCorner.mY);
   glEnd();

   // Reset GL options
   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_LINE_SMOOTH);
   glDisable(GL_LINE_STIPPLE);
   glLineWidth(1);
}

bool MeasurementObjectImp::setProperty(const GraphicProperty* pProp)
{
   if (pProp->getName() == "BoundingBox")
   {
      const BoundingBoxProperty* pBBoxProp = dynamic_cast<const BoundingBoxProperty*>(pProp);
      if (mpGeoreference.get() == NULL || !mpGeoreference->isGeoreferenced())
      {
         bool success = LineObjectImp::setProperty(pProp);
         if (success)
         {
            refreshGeoInformation();
         }
         return success;
      }

      LocationType ll = pBBoxProp->getLlCorner();
      LocationType ur = pBBoxProp->getUrCorner();
      LocationType llGeo = pBBoxProp->getLlLatLong();
      LocationType urGeo = pBBoxProp->getUrLatLong();

      if (!pBBoxProp->hasGeoCoords())
      {
         llGeo = mpGeoreference->convertPixelToGeocoord(ll);
         urGeo = mpGeoreference->convertPixelToGeocoord(ur);
      }
      else
      {
         ll = mpGeoreference->convertGeocoordToPixel(llGeo);
         ur = mpGeoreference->convertGeocoordToPixel(urGeo);
      }
      
      BoundingBoxProperty newProp(ll, ur, llGeo, urGeo);
      bool success = LineObjectImp::setProperty(&newProp);
      if (success)
      {
         refreshGeoInformation();
      }
      return success;
   }
   else if (pProp->getName() == "Font")
   {
      mBearingTextTexture.invalidate();
      mDistanceTextTexture.invalidate();
      mStartLocTextTexture.invalidate();
      mEndLocTextTexture.invalidate();
   }

   return LineObjectImp::setProperty(pProp);
}
void MeasurementObjectImp::setDistancePrecision(int precision)
{
   if (mDistancePrecision != precision)
   {
      mDistancePrecision = precision;
      refreshGeoInformation();
   }
}

int MeasurementObjectImp::getDistancePrecision() const
{
   return mDistancePrecision;
}

void MeasurementObjectImp::setBearingPrecision(int precision)
{
   if (mBearingPrecision != precision)
   {
      mBearingPrecision = precision;
      refreshGeoInformation();
   }
}

int MeasurementObjectImp::getBearingPrecision() const
{
   return mBearingPrecision;
}

void MeasurementObjectImp::setEndPointsPrecision(int precision)
{
   if (mEndPointsPrecision != precision)
   {
      mEndPointsPrecision = precision;
      refreshGeoInformation();
   }
}

int MeasurementObjectImp::getEndPointsPrecision() const
{
   return mEndPointsPrecision;
}

void MeasurementObjectImp::refreshGeoInformation() const
{
   MeasurementLayerImp* pLayer = dynamic_cast<MeasurementLayerImp*>(getLayer());
   if (pLayer != NULL)
   {
      mDrawnDistanceUnit = pLayer->getDistanceUnits();
      mDrawnGeocoord = pLayer->getGeocoordType();
      mDrawnDmsFormat = pLayer->getGeoFormat();
   }

   generateGeoStrings();
}

string MeasurementObjectImp::generateGeoStrings() const
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   LocationType llCornerLatLon;
   LocationType urCornerLatLon;
   bool unitsValid = false;

   // Get lat lon coordinates and terrain raster
   const RasterElement* pTerrain = NULL;

   bool geoValid(false);
   if (mpGeoreference.get() != NULL)
   {
      GraphicLayer* pLayer = getLayer();
      if (pLayer != NULL)
      {
         SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
         if (pView != NULL)
         {
            LayerList* pLayerList = pView->getLayerList();
            VERIFYRV(pLayerList != NULL, "");
            VERIFYRV(pLayerList->getPrimaryRasterElement() == mpGeoreference.get(), "");

            pTerrain = mpGeoreference->getTerrain();

            Layer* pPrimaryRasterLayer = pLayerList->getLayer(RASTER, mpGeoreference.get());
            if (pPrimaryRasterLayer != NULL)
            {
               pPrimaryRasterLayer->translateWorldToData(llCorner.mX, llCorner.mY, llCorner.mX, llCorner.mY);
               pPrimaryRasterLayer->translateWorldToData(urCorner.mX, urCorner.mY, urCorner.mX, urCorner.mY);
            }
         }
      }

      if (mpGeoreference->isGeoreferenced())
      {
         bool llValid(false);
         bool urValid(false);
         llCornerLatLon = mpGeoreference->convertPixelToGeocoord(llCorner, false, &llValid);
         urCornerLatLon = mpGeoreference->convertPixelToGeocoord(urCorner, false, &urValid);
         geoValid = llValid && urValid;
      }
   }

   mUsingInaccurateGeocoords = !geoValid;
   unitsValid = geoValid;

   //String Variables
   string startLoc = "";
   string endLoc = "";
   string distance = "";
   string bearing = "";
   string distanceUnit = "";

   GeoAlgorithms algs;
   double distanceVal = 0;
   double azimuthVal = 0;

   // Create GeoPoint objects
   LatLonPoint startLlPoint = llCornerLatLon;
   LatLonPoint endLlPoint = urCornerLatLon;
   UtmPoint startUtmPoint = startLlPoint;
   UtmPoint endUtmPoint = endLlPoint;
   MgrsPoint startMgrsPoint = startLlPoint;
   MgrsPoint endMgrsPoint = endLlPoint;

   // find elevations
   double elevation1(0.0);
   double elevation2(0.0);
   if (pTerrain != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pTerrain->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         const vector<DimensionDescriptor>& activeRows = pDescriptor->getRows();
         const vector<DimensionDescriptor>& activeColumns = pDescriptor->getColumns();
         if ( llCorner.mY >= 0 && llCorner.mY < activeRows.size() &&
              llCorner.mX >= 0 && llCorner.mX < activeColumns.size() &&
              urCorner.mY >= 0 && urCorner.mY < activeRows.size() &&
              urCorner.mX >= 0 && urCorner.mX < activeColumns.size() )
         {
            DimensionDescriptor llRowDim(activeRows[llCorner.mY]);
            DimensionDescriptor llColumnDim(activeColumns[llCorner.mX]);
            DimensionDescriptor urRowDim(activeRows[urCorner.mY]);
            DimensionDescriptor urColumnDim(activeColumns[urCorner.mX]);
            elevation1 = pTerrain->getPixelValue(llColumnDim, llRowDim, DimensionDescriptor(), COMPLEX_MAGNITUDE);
            elevation2 = pTerrain->getPixelValue(urColumnDim, urRowDim, DimensionDescriptor(), COMPLEX_MAGNITUDE);
            const Units* pElevationUnits = pDescriptor->getUnits();
            if (pElevationUnits != NULL)
            {
               double scale = pElevationUnits->getScaleFromStandard();
               elevation1 *= scale;
               elevation2 *= scale;
            }
         }
      }
   }

   if (unitsValid == true)
   {
      // Calculate bearing and distance
      distanceVal = algs.getPythagoreanOrVincentyDistance(startLlPoint.getLatitude().getValue(),
         startLlPoint.getLongitude().getValue(), endLlPoint.getLatitude().getValue(),
         endLlPoint.getLongitude().getValue(), elevation1, elevation2);
      azimuthVal = algs.getVincentyAzimuth(startLlPoint.getLatitude().getValue(),
         startLlPoint.getLongitude().getValue(), endLlPoint.getLatitude().getValue(),
         endLlPoint.getLongitude().getValue());
      azimuthVal = GeoConversions::convertRadToDeg(azimuthVal);

      // Set distance text
      if (mDrawnDistanceUnit == KILOMETER)
      {
         distanceUnit = "km";
         distanceVal = distanceVal/1000;
      }
      else if (mDrawnDistanceUnit == MILE)
      {
         distanceUnit = "mi";
         distanceVal = GeoConversions::convertMetersToMiles(distanceVal);
      }
      else if (mDrawnDistanceUnit == NAUTICAL_MILE)
      {
         distanceUnit = "Nm";
         distanceVal = GeoConversions::convertMetersToNm(distanceVal);
      }
      else if (mDrawnDistanceUnit == METER)
      {
         distanceUnit = "m";
      }
      else if (mDrawnDistanceUnit == YARD)
      {
         distanceUnit = "yd";
         distanceVal = GeoConversions::convertMetersToFeet(distanceVal)/3;
      }
      else if (mDrawnDistanceUnit == FOOT)
      {
         distanceUnit = "ft";
         distanceVal = GeoConversions::convertMetersToFeet(distanceVal);
      }

      // set location text
      if (mDrawnGeocoord == GEOCOORD_LATLON)
      {
         startLoc = startLlPoint.getText(mDrawnDmsFormat, mEndPointsPrecision);
         endLoc = endLlPoint.getText(mDrawnDmsFormat, mEndPointsPrecision);
      }
      else if (mDrawnGeocoord == GEOCOORD_UTM)
      {
         startLoc = startUtmPoint.getText();
         endLoc = endUtmPoint.getText();
      }
      else if (mDrawnGeocoord == GEOCOORD_MGRS)
      {
         startLoc = startMgrsPoint.getText();
         endLoc = endMgrsPoint.getText();
      }
      else if (mDrawnGeocoord == GEOCOORD_GENERAL)
      {
         startLoc = "(" + QString::number(llCorner.mX, 'f', mEndPointsPrecision).toStdString() + ", " +
            QString::number(llCorner.mY, 'f', mEndPointsPrecision).toStdString() + ")";
         endLoc = "(" + QString::number(urCorner.mX, 'f', mEndPointsPrecision).toStdString() + ", " +
            QString::number(urCorner.mY, 'f', mEndPointsPrecision).toStdString() + ")";
      }
   }
   else
   {
      startLoc = "(" + QString::number(llCorner.mX, 'f', mEndPointsPrecision).toStdString() + ", " +
         QString::number(llCorner.mY, 'f', mEndPointsPrecision).toStdString() + ")";
      endLoc = "(" + QString::number(urCorner.mX, 'f', mEndPointsPrecision).toStdString() + ", " +
         QString::number(urCorner.mY, 'f', mEndPointsPrecision).toStdString() + ")";
      azimuthVal = algs.getPythagoreanAzimuth(llCorner.mX, llCorner.mY, urCorner.mX, urCorner.mY);
      azimuthVal = GeoConversions::convertRadToDeg(azimuthVal);
      distanceVal = algs.getPythagoreanDistance(urCorner.mX, urCorner.mY, llCorner.mX, llCorner.mY);
      distanceUnit = "pix";
   }

   bearing = QString::number(azimuthVal, 'f', mBearingPrecision).toStdString();
   distance = QString::number(distanceVal, 'f', mDistancePrecision).toStdString();

   QString bearingText = QString::fromStdString(bearing) + " deg";
   QString distanceText = QString::fromStdString(distance) + " " + QString::fromStdString(distanceUnit);
   QString startLocText = QString::fromStdString(startLoc);
   QString endLocText = QString::fromStdString(endLoc);

   // Final strings
   if (bearingText != mBearingText)
   {
      mBearingText = bearingText;
      mBearingTextTexture.invalidate();
   }

   if (distanceText != mDistanceText)
   {
      mDistanceText = distanceText;
      mDistanceTextTexture.invalidate();
   }
   if (startLocText != mStartLocText)
   {
      mStartLocText = startLocText;
      mStartLocTextTexture.invalidate();
   }
   if (endLocText != mEndLocText)
   {
      mEndLocText = endLocText;
      mEndLocTextTexture.invalidate();
   }

   string rtnVal = "DISTANCE: " + distanceText.toStdString() + " : LOCATION: " + startLoc + " to " +
      endLoc + " at " + bearingText.toStdString();
   return rtnVal;
}

const string& MeasurementObjectImp::getObjectType() const
{
   static string type("MeasurementObjectImp");
   return type;
}

bool MeasurementObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "MeasurementObject"))
   {
      return true;
   }

   return LineObjectImp::isKindOf(className);
}

void MeasurementObjectImp::updateGeoreferenceAttachment()
{
   if (mpGeoreference.get() != NULL)
   {
      return;
   }

   RasterElement* pGeoreference = NULL;

   // Must find Georeference through the view, since the GraphicElement is a root element.
   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView != NULL)
      {
         LayerList* pLayerList = pView->getLayerList();
         VERIFYNRV(pLayerList != NULL);
         pGeoreference = pLayerList->getPrimaryRasterElement();
      }
   }

   if (pGeoreference != NULL && pGeoreference->isGeoreferenced())
   {
      mpGeoreference.reset(pGeoreference);
      enableGeo();
      generateGeoStrings();
   }
}

void MeasurementObjectImp::georeferenceModified(Subject &subject, const std::string &signal, const boost::any &v)
{
   updateGeo();
   generateGeoStrings();
   Layer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView != NULL)
      {
         pView->clearUndo();
      }
   }
}


bool MeasurementObjectImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL || !LineObjectImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("bearingPrecision", mBearingPrecision);
   pXml->addAttr("distancePrecision", mDistancePrecision);
   pXml->addAttr("endPointPrecision", mEndPointsPrecision);

   return true;
}

bool MeasurementObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !LineObjectImp::fromXml(pDocument, version))
   {
      return false;
   }

   mBearingPrecision = StringUtilities::fromXmlString<int>(
      A(static_cast<DOMElement*>(pDocument)->getAttribute(X("bearingPrecision"))));
   mDistancePrecision = StringUtilities::fromXmlString<int>(
      A(static_cast<DOMElement*>(pDocument)->getAttribute(X("distancePrecision"))));
   mEndPointsPrecision = StringUtilities::fromXmlString<int>(
      A(static_cast<DOMElement*>(pDocument)->getAttribute(X("endPointPrecision"))));

   return true;
}


bool MeasurementObjectImp::replicateObject(const GraphicObject* pObject)
{
   const MeasurementObjectImp* pMeas = dynamic_cast<const MeasurementObjectImp*>(pObject);
   if (pMeas == NULL)
   {
      return false;
   }

   bool bSuccess = false;
   bSuccess = LineObjectImp::replicateObject(pObject);
   if (bSuccess)
   {
      mBearingPrecision = pMeas->getBearingPrecision();
      mDistancePrecision = pMeas->getDistancePrecision();
      mEndPointsPrecision = pMeas->getEndPointsPrecision();
   }

   return bSuccess;
}
