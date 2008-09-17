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

#include "glCommon.h"
#include "AppVerify.h"
#include "Curve.h"
#include "CurveImp.h"
#include "PlotViewImp.h"
#include "xmlreader.h"

#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

CurveImp::CurveImp(PlotViewImp* pPlot, bool bPrimary) :
   PlotObjectImp(pPlot, bPrimary),
   mColor(Qt::black),
   mLineWidth(1),
   mLineStyle(SOLID_LINE)
{
   connect(this, SIGNAL(pointsChanged(const std::vector<LocationType>&)),
      this, SIGNAL(extentsChanged()));
}

CurveImp::~CurveImp()
{
}

CurveImp& CurveImp::operator= (const CurveImp& object)
{
   if (this != &object)
   {
      PlotObjectImp::operator= (object);

      mPoints = object.mPoints;
      mColor = object.mColor;
      mLineWidth = object.mLineWidth;
      mLineStyle = object.mLineStyle;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

const string& CurveImp::getObjectType() const
{
   static string type("CurveImp");
   return type;
}

bool CurveImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Curve"))
   {
      return true;
   }

   return PlotObjectImp::isKindOf(className);
}

PlotObjectType CurveImp::getType() const
{
   return CURVE;
}

void CurveImp::draw()
{
   if (isVisible() == false)
   {
      return;
   }

   unsigned int numPoints = mPoints.size();
   if (numPoints == 0)
   {
      return;
   }

   glColor3ub(mColor.red(), mColor.green(), mColor.blue());
   glLineWidth(mLineWidth);

   if (mLineStyle != SOLID_LINE)
   {
      glEnable(GL_LINE_STIPPLE);

      if (mLineStyle == DASHED)
      {
         glLineStipple(3, 0x3f3f);
      }
      else if (mLineStyle == DOT)
      {
         glLineStipple(2, 0x1111);
      }
      else if (mLineStyle == DASH_DOT)
      {
         glLineStipple(2, 0x11ff);
      }
      else if (mLineStyle == DASH_DOT_DOT)
      {
         glLineStipple(2, 0x24ff);
      }
   }

   glBegin(GL_LINE_STRIP);

   PlotViewImp* pPlot = getPlot();
   VERIFYNRV(pPlot != NULL);

   double dWorldX = 0.0;
   double dWorldY = 0.0;

   unsigned int i = 0;
   for (i = 0; i < numPoints; i++)
   {
      LocationType point = mPoints.at(i);
      pPlot->translateDataToWorld(point.mX, point.mY, dWorldX, dWorldY);
      glVertex2d(dWorldX, dWorldY);
   }

   glEnd();

   if (mLineStyle != SOLID_LINE)
   {
      glDisable(GL_LINE_STIPPLE);
   }

   glLineWidth(1);

   if (isSelected() == true)
   {
      PlotViewImp* pPlot = getPlot();
      if (pPlot == NULL)
      {
         return;
      }

      glColor3ub(0, 0, 0);

      for (i = 0; i < numPoints; i++)
      {
         LocationType point = mPoints.at(i);

         double dScreenX = 0;
         double dScreenY = 0;
         pPlot->translateDataToScreen(point.mX, point.mY, dScreenX, dScreenY);

         double dX1 = dScreenX - 4;
         double dY1 = dScreenY - 4;
         double dX2 = dScreenX + 4;
         double dY2 = dScreenY + 4;

         double dPixelX1 = 0.0;
         double dPixelY1 = 0.0;
         double dPixelX2 = 0.0;
         double dPixelY2 = 0.0;

         pPlot->translateScreenToWorld(dX1, dY1, dPixelX1, dPixelY1);
         pPlot->translateScreenToWorld(dX2, dY2, dPixelX2, dPixelY2);

         glBegin(GL_POLYGON);
         glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
         glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
         glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
         glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
         glEnd();
      }
   }
}

bool CurveImp::setPoints(const vector<LocationType>& points)
{
   if (points == mPoints)
   {
      return false;
   }

   mPoints = points;
   emit pointsChanged(mPoints);
   notify(SIGNAL_NAME(Curve, PointsChanged), boost::any(mPoints));
   return true;
}

const vector<LocationType>& CurveImp::getPoints() const
{
   return mPoints;
}

QColor CurveImp::getColor() const
{
   return mColor;
}

int CurveImp::getLineWidth() const
{
   return mLineWidth;
}

LineStyle CurveImp::getLineStyle() const
{
   return mLineStyle;
}

bool CurveImp::hit(LocationType point) const
{
   PlotViewImp* pPlot = getPlot();
   if (pPlot == NULL)
   {
      return false;
   }

   unsigned int numPoints = mPoints.size();
   for (unsigned int i = 0; i < numPoints; i++)
   {
      double dScreenX = 0;
      double dScreenY = 0;
      pPlot->translateWorldToScreen(point.mX, point.mY, dScreenX, dScreenY);

      LocationType currentPoint = mPoints.at(i);

      double dCurrentScreenX = 0;
      double dCurrentScreenY = 0;
      pPlot->translateDataToScreen(currentPoint.mX, currentPoint.mY, 
         dCurrentScreenX, dCurrentScreenY);

      double dXDist = abs(dCurrentScreenX - dScreenX);
      double dYDist = abs(dCurrentScreenY - dScreenY);

      if ((dXDist < 3.0) && (dYDist < 3.0))
      {
         return true;
      }
   }

   return false;
}

bool CurveImp::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
   unsigned int numPoints = mPoints.size();
   if (numPoints == 0)
   {
      return false;
   }

   PlotViewImp* pPlot = getPlot();
   VERIFY(pPlot != NULL);

   dMinX = 1e38;
   dMinY = 1e38;
   dMaxX = -1e38;
   dMaxY = -1e38;

   for (unsigned int i = 0; i < numPoints; i++)
   {
      LocationType point = mPoints.at(i);

      double dWorldX = 0.0;
      double dWorldY = 0.0;

      pPlot->translateDataToWorld(point.mX, point.mY, dWorldX, dWorldY);

      if (dWorldX < dMinX)
      {
         dMinX = dWorldX;
      }

      if (dWorldY < dMinY)
      {
         dMinY = dWorldY;
      }

      if (dWorldX > dMaxX)
      {
         dMaxX = dWorldX;
      }

      if (dWorldY > dMaxY)
      {
         dMaxY = dWorldY;
      }
   }

   return true;
}

const QPixmap& CurveImp::getLegendPixmap(bool bSelected) const
{
   static QPixmap pix(25, 15);
   static QPixmap selectedPix(25, 15);
   static QColor pixColor;
   static QColor selectedPixColor;

   if ((bSelected == true) && (selectedPix.isNull() == false))
   {
      if (selectedPixColor != mColor)
      {
         selectedPixColor = mColor;
         selectedPix.fill(Qt::white);

         QRect rcPixmap = selectedPix.rect();

         QPolygon points(4);
         points.setPoint(0, rcPixmap.center().x() - 4, rcPixmap.center().y());
         points.setPoint(1, rcPixmap.center().x(), rcPixmap.center().y() + 4);
         points.setPoint(2, rcPixmap.center().x() + 4, rcPixmap.center().y());
         points.setPoint(3, rcPixmap.center().x(), rcPixmap.center().y() - 4);

         QPainter p(&selectedPix);
         p.setPen(QPen(mColor, 1));
         p.drawLine(rcPixmap.left() + 2, rcPixmap.center().y(), rcPixmap.right() - 2, rcPixmap.center().y());
         p.setBrush(Qt::black);
         p.setPen(QPen(Qt::black, 1));
         p.drawPolygon(points);
         p.end();
      }

      return selectedPix;
   }
   else if ((bSelected == false) && (pix.isNull() == false))
   {
      if (pixColor != mColor)
      {
         pixColor = mColor;
         pix.fill(Qt::white);

         QRect rcPixmap = pix.rect();

         QPainter p(&pix);
         p.setPen(QPen(mColor, 1));
         p.drawLine(rcPixmap.left() + 2, rcPixmap.center().y(), rcPixmap.right() - 2, rcPixmap.center().y());
         p.end();
      }

      return pix;
   }

   return PlotObjectImp::getLegendPixmap(bSelected);
}

void CurveImp::setColor(const QColor& clrCurve)
{
   if (clrCurve.isValid() == false)
   {
      return;
   }

   if (clrCurve != mColor)
   {
      mColor = clrCurve;
      emit legendPixmapChanged();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void CurveImp::setLineWidth(int iWidth)
{
   if (iWidth < 0)
   {
      iWidth = 0;
   }

   if (iWidth != mLineWidth)
   {
      mLineWidth = iWidth;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void CurveImp::setLineStyle(LineStyle eStyle)
{
   if (eStyle != mLineStyle)
   {
      mLineStyle = eStyle;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

bool CurveImp::toXml(XMLWriter* pXml) const
{
   if(!PlotObjectImp::toXml(pXml))
   {
      return false;
   }
   pXml->addAttr("color", QCOLOR_TO_COLORTYPE(mColor));
   pXml->addAttr("lineWidth", mLineWidth);
   pXml->addAttr("lineStyle", mLineStyle);
   for(vector<LocationType>::const_iterator it = mPoints.begin(); it != mPoints.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("Point"));
      pXml->addText(StringUtilities::toXmlString(*it));
      pXml->popAddPoint();
   }
   return true;
}

bool CurveImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !PlotObjectImp::fromXml(pDocument, version))
   {
      return false;
   }
   DOMElement *pElem = static_cast<DOMElement*>(pDocument);
   ColorType color = StringUtilities::fromXmlString<ColorType>(
      A(pElem->getAttribute(X("color"))));
   mColor = COLORTYPE_TO_QCOLOR(color);
   mLineWidth = StringUtilities::fromXmlString<int>(
      A(pElem->getAttribute(X("lineWidth"))));
   mLineStyle = StringUtilities::fromXmlString<LineStyle>(
      A(pElem->getAttribute(X("lineStyle"))));
   for(DOMNode *pChld = pElem->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if(XMLString::equals(pChld->getNodeName(), X("Point")))
      {
         mPoints.push_back(StringUtilities::fromXmlString<LocationType>(
            A(pChld->getTextContent())));
      }
   }
   return true;
}
