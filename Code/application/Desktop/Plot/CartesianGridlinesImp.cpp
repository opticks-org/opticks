/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <qwt_scale_engine.h>
#include <qwt_scale_div.h>

#include "CartesianGridlinesImp.h"
#include "CartesianPlotImp.h"
#include "PlotViewImp.h"
#include "PointSet.h"
#include "Point.h"
#include "xmlreader.h"

#include <vector>
using namespace std;
XERCES_CPP_NAMESPACE_USE

CartesianGridlinesImp::CartesianGridlinesImp(OrientationType orientation, PlotViewImp* pPlot, bool bPrimary) :
   GridlinesImp(pPlot, bPrimary),
   mOrientation(orientation),
   mLines(pPlot, bPrimary)
{
   connect(this, SIGNAL(colorChanged(const QColor&)), this, SLOT(updateColor(const QColor&)));
   connect(this, SIGNAL(lineWidthChanged(int)), this, SLOT(updateLineWidth(int)));
   connect(this, SIGNAL(lineStyleChanged(LineStyle)), this, SLOT(updateLineStyle(LineStyle)));
}

CartesianGridlinesImp::~CartesianGridlinesImp()
{
   mLines.clear(true);
}

CartesianGridlinesImp& CartesianGridlinesImp::operator= (const CartesianGridlinesImp& object)
{
   if (this != &object)
   {
      GridlinesImp::operator= (object);

      mOrientation = object.mOrientation;
      mLines = object.mLines;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

PlotObjectType CartesianGridlinesImp::getType() const
{
   return CARTESIAN_GRIDLINES;
}

void CartesianGridlinesImp::draw()
{
   if (isVisible() == true)
   {
      mLines.draw();
   }
}

OrientationType CartesianGridlinesImp::getOrientation() const
{
   return mOrientation;
}

void CartesianGridlinesImp::updateLocations()
{
   if (isVisible() == false)
   {
      return;
   }

   CartesianPlotImp* pPlot = dynamic_cast<CartesianPlotImp*>(getPlot());
   if (pPlot == NULL)
   {
      return;
   }

   // Get the plot extents
   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 1.0;
   double dMaxY = 1.0;

   pPlot->translateScreenToData(0, 0, dMinX, dMinY);
   pPlot->translateScreenToData(pPlot->width(), pPlot->height(), dMaxX, dMaxY);

   // Get the scale type and the min and max line extents based on the orientation
   double dMin = 0.0;
   double dMax = 1.0;
   ScaleType scaleType = SCALE_LINEAR;

   if (mOrientation == HORIZONTAL)
   {
      dMin = dMinY;
      dMax = dMaxY;
      scaleType = pPlot->getYScaleType();
   }
   else if (mOrientation == VERTICAL)
   {
      dMin = dMinX;
      dMax = dMaxX;
      scaleType = pPlot->getXScaleType();
   }

   // Calculate a new scale
   QwtScaleDiv scaleDivision;
   int maxMajorTicks = getMaxNumMajorLines();
   int maxMinorTicks = getMaxNumMinorLines();
   double dStepSize = 0.0;

   if (scaleType == SCALE_LOG)
   {
      QwtLog10ScaleEngine logScale;
      logScale.autoScale(maxMajorTicks, dMin, dMax, dStepSize);
      scaleDivision = logScale.divideScale(dMin, dMax, maxMajorTicks, maxMinorTicks);
   }
   else
   {
      QwtLinearScaleEngine linearScale;
      linearScale.autoScale(maxMajorTicks, dMin, dMax, dStepSize);
      scaleDivision = linearScale.divideScale(dMin, dMax, maxMajorTicks, maxMinorTicks);
   }

   // Clear the existing lines
   mLines.clear(true);

   // Add the new lines at the tick locations
   QwtValueList tickList = scaleDivision.ticks(QwtScaleDiv::MajorTick);
   if (areMinorGridlinesEnabled() == true)
   {
      tickList += scaleDivision.ticks(QwtScaleDiv::MediumTick) + scaleDivision.ticks(QwtScaleDiv::MinorTick);
   }

   for (int i = 0; i < tickList.size(); ++i)
   {
      double dValue = tickList[i];

      // Create the line
      PointSetImp* pLine = dynamic_cast<PointSetImp*>(mLines.addObject(POINT_SET));
      if (pLine != NULL)
      {
         // Points
         if (mOrientation == HORIZONTAL)
         {
            pLine->addPoint(dMinX, dValue);
            pLine->addPoint(dMaxX, dValue);
         }
         else if (mOrientation == VERTICAL)
         {
            pLine->addPoint(dValue, dMinY);
            pLine->addPoint(dValue, dMaxY);
         }

         // Line Color
         pLine->setLineColor(getColor());
         pLine->setPointColor(getColor());

         // Line width
         int lineWidth = getLineWidth();
         pLine->setLineWidth(lineWidth);

         // Line style
         LineStyle lineStyle = getLineStyle();
         pLine->setLineStyle(lineStyle);
      }
   }

   emit extentsChanged();
   notify(SIGNAL_NAME(Subject, Modified));
}

void CartesianGridlinesImp::updateColor(const QColor& lineColor)
{
   const vector<PlotObject*>& lines = mLines.getObjects();
   for (vector<PlotObject*>::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
   {
      PointSetImp* pLine = dynamic_cast<PointSetImp*>(*iter);
      if (pLine != NULL)
      {
         pLine->setLineColor(lineColor);
         pLine->setPointColor(lineColor);
      }
   }
}

void CartesianGridlinesImp::updateLineWidth(int lineWidth)
{
   const vector<PlotObject*>& lines = mLines.getObjects();
   for (vector<PlotObject*>::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
   {
      PointSet* pLine = dynamic_cast<PointSet*>(*iter);
      if (pLine != NULL)
      {
         pLine->setLineWidth(lineWidth);
      }
   }
}

void CartesianGridlinesImp::updateLineStyle(LineStyle lineStyle)
{
   const vector<PlotObject*>& lines = mLines.getObjects();
   for (vector<PlotObject*>::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
   {
      PointSet* pLine = dynamic_cast<PointSet*>(*iter);
      if (pLine != NULL)
      {
         pLine->setLineStyle(lineStyle);
      }
   }
}

bool CartesianGridlinesImp::toXml(XMLWriter* pXml) const
{
   if (!GridlinesImp::toXml(pXml))
   {
      return false;
   }
   pXml->addAttr("orientation", mOrientation);
   pXml->pushAddPoint(pXml->addElement("Lines"));
   if (!mLines.toXml(pXml))
   {
      return false;
   }
   pXml->popAddPoint();
   return true;
}

bool CartesianGridlinesImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !GridlinesImp::fromXml(pDocument, version))
   {
      return false;
   }
   mOrientation = StringUtilities::fromXmlString<OrientationType>(
      A(static_cast<DOMElement*>(pDocument)->getAttribute(X("orientation"))));
   for (DOMNode* pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("Lines")))
      {
         if (!mLines.fromXml(pChld, version))
         {
            return false;
         }
      }
   }
   return true;
}
