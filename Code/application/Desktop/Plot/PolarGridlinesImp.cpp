/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <qwt_scale_engine.h>
#include <qwt_scale_div.h>

#include "PolarGridlinesImp.h"
#include "DrawUtil.h"
#include "glCommon.h"
#include "PlotView.h"
#include "PlotViewImp.h"

#include <limits>
using namespace std;
XERCES_CPP_NAMESPACE_USE

PolarGridlinesImp::PolarGridlinesImp(PlotViewImp* pPlot, bool bPrimary) :
   GridlinesImp(pPlot, bPrimary),
   mAngle(45.0)
{
}

PolarGridlinesImp::~PolarGridlinesImp()
{
}

PolarGridlinesImp& PolarGridlinesImp::operator= (const PolarGridlinesImp& object)
{
   if (this != &object)
   {
      GridlinesImp::operator= (object);

      mAngle = object.mAngle;
      mDrawLocations = object.mDrawLocations;

      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

PlotObjectType PolarGridlinesImp::getType() const
{
   return POLAR_GRIDLINES;
}

void PolarGridlinesImp::draw()
{
   if (isVisible() == false)
   {
      return;
   }

   PlotViewImp* pPlot = dynamic_cast<PlotViewImp*>(getPlot());
   if (pPlot == NULL)
   {
      return;
   }

   // Set the draw properties
   QColor clrGridlines = getColor();
   glColor3ub(clrGridlines.red(), clrGridlines.green(), clrGridlines.blue());

   int iWidth = getLineWidth();
   glLineWidth(iWidth);

   LineStyle lineStyle = getLineStyle();
   if (lineStyle != SOLID_LINE)
   {
      glEnable(GL_LINE_STIPPLE);

      if (lineStyle == DASHED)
      {
         glLineStipple(3, 0x3f3f);
      }
      else if (lineStyle == DOT)
      {
         glLineStipple(2, 0x1111);
      }
      else if (lineStyle == DASH_DOT)
      {
         glLineStipple(2, 0x11ff);
      }
      else if (lineStyle == DASH_DOT_DOT)
      {
         glLineStipple(2, 0x24ff);
      }
   }

   // Draw the rings
   for (unsigned int i = 0; i < mDrawLocations.size(); ++i)
   {
      double xCoord = 0.0;
      double yCoord = 0.0;
      pPlot->translateDataToWorld(mDrawLocations[i], 0.0, xCoord, yCoord);

      GLUquadricObj* pQuadric = gluNewQuadric();
      gluQuadricDrawStyle(pQuadric, GLU_LINE);
      gluDisk(pQuadric, xCoord, xCoord, 360, 1);
      gluDeleteQuadric(pQuadric);
   }

   // Draw the radial lines
   LocationType worldStart;
   pPlot->translateDataToWorld(0.0, 0.0, worldStart.mX, worldStart.mY);

   LocationType worldLlCorner;
   LocationType worldUrCorner;
   pPlot->translateScreenToWorld(0.0, 0.0, worldLlCorner.mX, worldLlCorner.mY);
   pPlot->translateScreenToWorld(pPlot->width(), pPlot->height(), worldUrCorner.mX, worldUrCorner.mY);

   glBegin(GL_LINES);
   for (double i = 0.0; i < 360.0; i += mAngle)
   {
      double dSlope = tan(i * PI / 180.0);
      double dB = worldStart.mY - (dSlope * worldStart.mX);

      LocationType leftIntersection(worldLlCorner.mX, (dSlope * worldLlCorner.mX) + dB);
      LocationType topIntersection((worldUrCorner.mY - dB) / dSlope, worldUrCorner.mY);
      LocationType rightIntersection(worldUrCorner.mX, (dSlope * worldUrCorner.mX) + dB);
      LocationType bottomIntersection((worldLlCorner.mY - dB) / dSlope, worldLlCorner.mY);

      LocationType worldEnd;
      if (i == 0.0)
      {
         if (worldStart.mX < worldUrCorner.mX)
         {
            worldEnd.mX = worldUrCorner.mX;
         }
      }
      else if (i == 90.0)
      {
         if (worldStart.mY < worldUrCorner.mY)
         {
            worldEnd.mY = worldUrCorner.mY;
         }
      }
      else if (i == 180.0)
      {
         if (worldStart.mX > worldLlCorner.mX)
         {
            worldEnd.mX = worldLlCorner.mX;
         }
      }
      else if (i == 270.0)
      {
         if (worldStart.mY > worldLlCorner.mY)
         {
            worldEnd.mY = worldLlCorner.mY;
         }
      }
      else
      {
         if (i < 90.0)
         {
            if ((worldStart.mX < worldUrCorner.mX) && (worldStart.mY < worldUrCorner.mY))
            {
               if (topIntersection.mY < rightIntersection.mY)
               {
                  worldEnd = topIntersection;
               }
               else
               {
                  worldEnd = rightIntersection;
               }
            }
         }
         else if (i < 180.0)
         {
            if ((worldStart.mX > worldLlCorner.mX) && (worldStart.mY < worldUrCorner.mY))
            {
               if (topIntersection.mY < leftIntersection.mY)
               {
                  worldEnd = topIntersection;
               }
               else
               {
                  worldEnd = leftIntersection;
               }
            }
         }
         else if (i < 270.0)
         {
            if ((worldStart.mX > worldLlCorner.mX) && (worldStart.mY > worldLlCorner.mY))
            {
               if (bottomIntersection.mY > leftIntersection.mY)
               {
                  worldEnd = bottomIntersection;
               }
               else
               {
                  worldEnd = leftIntersection;
               }
            }
         }
         else if (i < 360.0)
         {
            if ((worldStart.mX < worldUrCorner.mX) && (worldStart.mY > worldLlCorner.mY))
            {
               if (bottomIntersection.mY > rightIntersection.mY)
               {
                  worldEnd = bottomIntersection;
               }
               else
               {
                  worldEnd = rightIntersection;
               }
            }
         }
      }

      glVertex2d(worldStart.mX, worldStart.mY);
      glVertex2d(worldEnd.mX, worldEnd.mY);
   }

   glEnd();
   glLineWidth(1.0);

   if (lineStyle != SOLID_LINE)
   {
      glDisable(GL_LINE_STIPPLE);
   }

   // Draw the text
   glColor3ub(0, 0, 0);
   for (unsigned int i = 0; i < mDrawLocations.size(); ++i)
   {
      double xCoord = 0.0;
      double yCoord = 0.0;
      pPlot->translateDataToScreen(mDrawLocations[i], 0.0, xCoord, yCoord);

      int screenX = static_cast<int>(xCoord) + 3;
      int screenY = pPlot->height() - (static_cast<int>(yCoord) + 3);
      pPlot->renderText(screenX, screenY, QString::number(mDrawLocations[i]));
   }
}

double PolarGridlinesImp::getRadialInterval() const
{
   return mAngle;
}

void PolarGridlinesImp::setRadialInterval(double angle)
{
   if (angle != mAngle)
   {
      while (angle > 360.0)
      {
         angle -= 360.0;
      }

      if (angle < 0.0)
      {
         angle = 360.0;
      }

      mAngle = angle;
      updateLocations();
   }
}

void PolarGridlinesImp::updateLocations()
{
   if (isVisible() == false)
   {
      return;
   }

   // Get the minimum and maximum radius values of the displayed area
   double minRadius = numeric_limits<double>::max();
   double maxRadius = numeric_limits<double>::min();

   PlotViewImp* pPlot = dynamic_cast<PlotViewImp*>(getPlot());
   if (pPlot != NULL)
   {
      double radius = 0.0;
      double angle = 0.0;
      double plotWidth = pPlot->width();
      double plotHeight = pPlot->height();

      pPlot->translateScreenToData(0, 0, radius, angle);
      minRadius = min(minRadius, radius);
      maxRadius = max(maxRadius, radius);

      pPlot->translateScreenToData(plotWidth, 0, radius, angle);
      minRadius = min(minRadius, radius);
      maxRadius = max(maxRadius, radius);

      pPlot->translateScreenToData(plotWidth, plotHeight, radius, angle);
      minRadius = min(minRadius, radius);
      maxRadius = max(maxRadius, radius);

      pPlot->translateScreenToData(0, plotHeight, radius, angle);
      minRadius = min(minRadius, radius);
      maxRadius = max(maxRadius, radius);

      // Update the minimum value based on whether the plot center is visible
      LocationType center(0.0, 0.0);
      pPlot->translateDataToScreen(0.0, 0.0, center.mX, center.mY);

      QRect viewRect = pPlot->rect();
      if (viewRect.contains(QPoint(center.mX, center.mY)) == true)
      {
         minRadius = 0.0;
      }
      else
      {
         // Ensure that the minimum radius is the minimum viewable radius
         LocationType llCorner(0.0, 0.0);
         LocationType lrCorner(plotWidth, 0.0);
         LocationType urCorner(plotWidth, plotHeight);
         LocationType ulCorner(0.0, plotHeight);
         LocationType intersection;

         DrawUtil::linePointDistance(llCorner, lrCorner, center, &intersection);
         pPlot->translateScreenToData(intersection.mX, intersection.mY, radius, angle);
         minRadius = min(minRadius, radius);

         DrawUtil::linePointDistance(lrCorner, urCorner, center, &intersection);
         pPlot->translateScreenToData(intersection.mX, intersection.mY, radius, angle);
         minRadius = min(minRadius, radius);

         DrawUtil::linePointDistance(ulCorner, urCorner, center, &intersection);
         pPlot->translateScreenToData(intersection.mX, intersection.mY, radius, angle);
         minRadius = min(minRadius, radius);

         DrawUtil::linePointDistance(llCorner, ulCorner, center, &intersection);
         pPlot->translateScreenToData(intersection.mX, intersection.mY, radius, angle);
         minRadius = min(minRadius, radius);
      }
   }

   // Calculate a new scale
   int maxMajorTicks = getMaxNumMajorLines();
   int maxMinorTicks = getMaxNumMinorLines();
   double dStepSize = 0.0;

   QwtLinearScaleEngine scale;
   scale.autoScale(maxMajorTicks, minRadius, maxRadius, dStepSize);
   QwtScaleDiv scaleDivision = scale.divideScale(minRadius, maxRadius, maxMajorTicks, maxMinorTicks);

   // Clear the existing locations
   mDrawLocations.clear();

   // Set the ring locations as the tick locations
   QwtValueList tickList = scaleDivision.ticks(QwtScaleDiv::MajorTick);
   if (areMinorGridlinesEnabled() == true)
   {
      tickList += scaleDivision.ticks(QwtScaleDiv::MediumTick) + scaleDivision.ticks(QwtScaleDiv::MinorTick);
   }

   for (int i = 0; i < tickList.size(); ++i)
   {
      double value = tickList[i];
      mDrawLocations.push_back(value);
   }

   emit modified();
   notify(SIGNAL_NAME(Subject, Modified));
}

bool PolarGridlinesImp::toXml(XMLWriter* pXml) const
{
   if (!GridlinesImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("radialInterval", getRadialInterval());
   for(vector<double>::const_iterator it = mDrawLocations.begin(); it != mDrawLocations.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("Location"));
      pXml->addText(*it);
      pXml->popAddPoint();
   }

   return true;
}

bool PolarGridlinesImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !GridlinesImp::fromXml(pDocument, version))
   {
      return false;
   }
   setRadialInterval(StringUtilities::fromXmlString<double>(
      A(static_cast<DOMElement*>(pDocument)->getAttribute(X("radialInterval")))));
   for(DOMNode *pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if(XMLString::equals(pChld->getNodeName(), X("Location")))
      {
         mDrawLocations.push_back(StringUtilities::fromXmlString<double>(A(pChld->getTextContent())));
      }
   }

   return true;
}