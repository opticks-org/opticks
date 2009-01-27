/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtGui/QPainter>

#include "ArrowImp.h"
#include "PlotViewImp.h"
#include "Point.h"

#include <vector>
using namespace std;
XERCES_CPP_NAMESPACE_USE

ArrowImp::ArrowImp(PlotViewImp* pPlot, bool bPrimary) :
   PlotObjectImp(pPlot, bPrimary),
   mStyle(ARROW_SMALL),
   mLine(pPlot, bPrimary),
   mArrowHead(pPlot, bPrimary)
{
   mLine.addPoint();
   mLine.addPoint();

   if (pPlot != NULL)
   {
      connect(pPlot, SIGNAL(displayAreaChanged()), this, SLOT(updateArrowHead()));
   }
}

ArrowImp::ArrowImp(ArrowStyle arrowStyle, PlotViewImp* pPlot, bool bPrimary) :
   PlotObjectImp(pPlot, bPrimary),
   mStyle(arrowStyle),
   mLine(pPlot, bPrimary),
   mArrowHead(pPlot, bPrimary)
{
   mLine.addPoint();
   mLine.addPoint();

   if (pPlot != NULL)
   {
      connect(pPlot, SIGNAL(displayAreaChanged()), this, SLOT(updateArrowHead()));
   }
}

ArrowImp::~ArrowImp()
{}

ArrowImp& ArrowImp::operator=(const ArrowImp& object)
{
   if (this != &object)
   {
      PlotObjectImp::operator= (object);

      mStyle = object.mStyle;
      mLine = object.mLine;
      mArrowHead = object.mArrowHead;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

PlotObjectType ArrowImp::getType() const
{
   return ARROW;
}

void ArrowImp::draw()
{
   if (isVisible() == true)
   {
      mLine.draw();
      mArrowHead.draw();
   }
}

ArrowStyle ArrowImp::getArrowStyle() const
{
   return mStyle;
}

LocationType ArrowImp::getBaseLocation() const
{
   LocationType baseLocation;

   Point* pPoint = getBasePoint();
   if (pPoint != NULL)
   {
      baseLocation = pPoint->getLocation();
   }

   return baseLocation;
}

LocationType ArrowImp::getTipLocation() const
{
   LocationType tipLocation;

   Point* pPoint = getTipPoint();
   if (pPoint != NULL)
   {
      tipLocation = pPoint->getLocation();
   }

   return tipLocation;
}

QColor ArrowImp::getColor() const
{
   return QColor(mLine.getLineColor().mRed, mLine.getLineColor().mGreen, mLine.getLineColor().mBlue);
}

bool ArrowImp::hit(LocationType point) const
{
   bool bHit = mLine.hit(point);
   if (bHit == false)
   {
      bHit = mArrowHead.hit(point);
   }

   return bHit;
}

bool ArrowImp::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
   dMinX = -1.0;
   dMinY = -1.0;
   dMaxX = 1.0;
   dMaxY = 1.0;

   double dLineMinX = 0.0;
   double dLineMinY = 0.0;
   double dLineMaxX = 0.0;
   double dLineMaxY = 0.0;

   bool bSuccess = mLine.getExtents(dLineMinX, dLineMinY, dLineMaxX, dLineMaxY);
   if (bSuccess == false)
   {
      return false;
   }

   double dArrowHeadMinX = 0.0;
   double dArrowHeadMinY = 0.0;
   double dArrowHeadMaxX = 0.0;
   double dArrowHeadMaxY = 0.0;

   bSuccess = mArrowHead.getExtents(dArrowHeadMinX, dArrowHeadMinY, dArrowHeadMaxX, dArrowHeadMaxY);
   if (bSuccess == false)
   {
      return false;
   }

   dMinX = min(dLineMinX, dArrowHeadMinX);
   dMinY = min(dLineMinY, dArrowHeadMinY);
   dMaxX = max(dLineMaxX, dArrowHeadMaxX);
   dMaxY = max(dLineMaxY, dArrowHeadMaxY);

   return true;
}

const QPixmap& ArrowImp::getLegendPixmap(bool bSelected) const
{
   static QPixmap pix(25, 15);
   static QPixmap selectedPix(25, 15);
   static ColorType pixColor;
   static ColorType selectedPixColor;

   if ((bSelected == true) && (selectedPix.isNull() == false))
   {
      if (selectedPixColor != mLine.getLineColor())
      {
         selectedPixColor = mLine.getLineColor();
         selectedPix.fill(Qt::transparent);

         QRect rcPixmap = selectedPix.rect();

         QPainter p(&selectedPix);
         ColorType color = mLine.getLineColor();
         p.setPen(QPen(QColor(color.mRed, color.mGreen, color.mBlue), 3));
         p.drawLine(rcPixmap.left() + 2, rcPixmap.center().y(), rcPixmap.right() - 2, rcPixmap.center().y());
         p.drawLine(rcPixmap.center().x(), rcPixmap.center().y() - (rcPixmap.height() / 2) + 2,
            rcPixmap.right() - 2, rcPixmap.center().y());
         p.drawLine(rcPixmap.center().x(), rcPixmap.center().y() + (rcPixmap.height() / 2) - 2,
            rcPixmap.right() - 2, rcPixmap.center().y());
         p.end();
      }

      return selectedPix;
   }
   else if ((bSelected == false) && (pix.isNull() == false))
   {
      if (pixColor != mLine.getLineColor())
      {
         pixColor = mLine.getLineColor();
         pix.fill(Qt::transparent);

         QRect rcPixmap = pix.rect();

         QPainter p(&pix);
         ColorType color = mLine.getLineColor();
         p.setPen(QPen(QColor(color.mRed, color.mGreen, color.mBlue), 1));
         p.drawLine(rcPixmap.left() + 2, rcPixmap.center().y(), rcPixmap.right() - 2, rcPixmap.center().y());
         p.drawLine(rcPixmap.center().x(), rcPixmap.center().y() - (rcPixmap.height() / 2) + 2,
            rcPixmap.right() - 2, rcPixmap.center().y());
         p.drawLine(rcPixmap.center().x(), rcPixmap.center().y() + (rcPixmap.height() / 2) - 2,
            rcPixmap.right() - 2, rcPixmap.center().y());
         p.end();
      }

      return pix;
   }

   return PlotObjectImp::getLegendPixmap(bSelected);
}

void ArrowImp::setArrowStyle(const ArrowStyle& eStyle)
{
   if (eStyle == mStyle)
   {
      return;
   }

   mStyle = eStyle;

   updateArrowHead();
   notify(SIGNAL_NAME(Subject, Modified));
}

void ArrowImp::setLocation(const LocationType& baseLocation, const LocationType& tipLocation)
{
   setBaseLocation(baseLocation);
   setTipLocation(tipLocation);
}

void ArrowImp::setBaseLocation(const LocationType& baseLocation)
{
   Point* pPoint = getBasePoint();
   if (pPoint != NULL)
   {
      pPoint->setLocation(baseLocation);
      updateArrowHead();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void ArrowImp::setTipLocation(const LocationType& tipLocation)
{
   Point* pPoint = getTipPoint();
   if (pPoint != NULL)
   {
      pPoint->setLocation(tipLocation);
      updateArrowHead();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void ArrowImp::setColor(const QColor& newColor)
{
   if (newColor.isValid() == true)
   {
      ColorType arrowColor(newColor.red(), newColor.green(), newColor.blue());
      mLine.setLineColor(arrowColor);
      mArrowHead.setLineColor(arrowColor);
      mArrowHead.setFillColor(arrowColor);
      emit legendPixmapChanged();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

Point* ArrowImp::getBasePoint() const
{
   vector<Point*> points = mLine.getPoints();
   if (points.empty() == false)
   {
      return points.front();
   }

   return NULL;
}

Point* ArrowImp::getTipPoint() const
{
   vector<Point*> points = mLine.getPoints();
   if (points.empty() == false)
   {
      return points.back();
   }

   return NULL;
}

void ArrowImp::updateArrowHead()
{
   // Reset the arrow head
   mArrowHead.clear(true);
   mArrowHead.setFillStyle(EMPTY_FILL);

   // Convert the line points to screen coordinates
   PlotViewImp* pPlot = getPlot();
   if (pPlot == NULL)
   {
      return;
   }

   LocationType baseLocation = getBaseLocation();
   LocationType tipLocation = getTipLocation();

   double dBaseX = 0;
   double dBaseY = 0;
   double dTipX = 0;
   double dTipY = 0;

   pPlot->translateDataToScreen(baseLocation.mX, baseLocation.mY, dBaseX, dBaseY);
   pPlot->translateDataToScreen(tipLocation.mX, tipLocation.mY, dTipX , dTipY);

   // Calculate the end points using screen pixels
   double arrowHeadSize = 10;
   if ((mStyle == ARROW_LARGE) || (mStyle == ARROW_TRIANGLE_LARGE) || (mStyle == ARROW_TRIANGLE_LARGE_FILL))
   {
      arrowHeadSize *= 2;
   }

   double h = arrowHeadSize * sqrt(static_cast<double>(mArrowHead.getLineWidth()));
   double theta = atan2(dTipY - dBaseY, dTipX - dBaseX);
   double hcTheta = h * cos(theta);
   double hsTheta = h * sin(theta);

   double dScreenEnd1X = dTipX - hcTheta - hsTheta;
   double dScreenEnd1Y = dTipY + hcTheta - hsTheta;
   double dScreenEnd2X = dTipX - hcTheta + hsTheta;
   double dScreenEnd2Y = dTipY - hcTheta - hsTheta;
   double dScreenEnd3X = dTipX - (2 * hcTheta);
   double dScreenEnd3Y = dTipY - (2 * hsTheta);

   double dEnd1X = 0.0;
   double dEnd1Y = 0.0;
   double dEnd2X = 0.0;
   double dEnd2Y = 0.0;
   double dEnd3X = 0.0;
   double dEnd3Y = 0.0;

   pPlot->translateScreenToData(dScreenEnd1X, dScreenEnd1Y, dEnd1X, dEnd1Y);
   pPlot->translateScreenToData(dScreenEnd2X, dScreenEnd2Y, dEnd2X, dEnd2Y);
   pPlot->translateScreenToData(dScreenEnd3X, dScreenEnd3Y, dEnd3X, dEnd3Y);

   // Add the points to the arrow head object
   if (mStyle != ARROW_NONE)
   {
      mArrowHead.addPoint(dEnd1X, dEnd1Y);
      mArrowHead.addPoint(tipLocation.mX, tipLocation.mY);
      mArrowHead.addPoint(dEnd2X, dEnd2Y);
   }

   if ((mStyle == ARROW_DIAMOND) || (mStyle == ARROW_DIAMOND_FILL))
   {
      mArrowHead.addPoint(dEnd3X, dEnd3Y);
   }

   if ((mStyle == ARROW_TRIANGLE_SMALL) || (mStyle == ARROW_TRIANGLE_LARGE) ||
      (mStyle == ARROW_TRIANGLE_SMALL_FILL) || (mStyle == ARROW_TRIANGLE_LARGE_FILL) ||
      (mStyle == ARROW_DIAMOND) || (mStyle == ARROW_DIAMOND_FILL))
   {
      mArrowHead.addPoint(dEnd1X, dEnd1Y);
   }

   // Set the fill
   if ((mStyle == ARROW_TRIANGLE_SMALL_FILL) || (mStyle == ARROW_TRIANGLE_LARGE_FILL) ||
      (mStyle == ARROW_DIAMOND_FILL))
   {
      mArrowHead.setFillStyle(SOLID_FILL);
   }
}

bool ArrowImp::toXml(XMLWriter* pXml) const
{
   if (!PlotObjectImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("arrowStyle", mStyle);
   pXml->pushAddPoint(pXml->addElement("Line"));
   if (!mLine.toXml(pXml))
   {
      return false;
   }
   pXml->popAddPoint();

   pXml->pushAddPoint(pXml->addElement("ArrowHead"));
   if (!mArrowHead.toXml(pXml))
   {
      return false;
   }
   pXml->popAddPoint();

   pXml->popAddPoint();
   return true;
}

bool ArrowImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !PlotObjectImp::fromXml(pDocument, version))
   {
      return false;
   }
   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   mStyle = StringUtilities::fromXmlString<ArrowStyle>(A(pElem->getAttribute(X("arrowStyle"))));
   for (DOMNode* pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("Line")))
      {
         if (!mLine.fromXml(pChld, version))
         {
            return false;
         }
      }
      else if (XMLString::equals(pChld->getNodeName(), X("ArrowHead")))
      {
         if (!mArrowHead.fromXml(pChld, version))
         {
            return false;
         }
      }
   }

   return true;
}
