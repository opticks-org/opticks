/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <float.h>

#include <QtGui/QPainter>

#include "glCommon.h"
#include "PointSet.h"
#include "PointSetImp.h"
#include "AppVerify.h"
#include "DrawUtil.h"
#include "PlotView.h"
#include "PlotViewImp.h"
#include "PointAdapter.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

PointSetImp::PointSetImp(PlotViewImp* pPlot, bool bPrimary) : 
   PlotObjectImp(pPlot, bPrimary),
   mSymbols(false),
   mLine(true),
   mLineColor(Qt::black),
   mLineWidth(1),
   mLineStyle(SOLID_LINE),
   mInteractive(true),
   mDirty(false)
{
   connect(this, SIGNAL(pointAdded(Point*)), this, SIGNAL(extentsChanged()));
   connect(this, SIGNAL(pointRemoved(Point*)), this, SIGNAL(extentsChanged()));
   connect(this, SIGNAL(pointsSet(const std::vector<Point*>&)), this, SIGNAL(extentsChanged()));
   connect(this, SIGNAL(pointLocationChanged(Point*, const LocationType&)), this, SIGNAL(extentsChanged()));
}

PointSetImp::~PointSetImp()
{
   clear(true);
}

PointSetImp& PointSetImp::operator= (const PointSetImp& object)
{
   if (this != &object)
   {
      PlotObjectImp::operator= (object);

      clear(true);

      vector<Point*>::const_iterator iter = object.mPoints.begin();
      while (iter != object.mPoints.end())
      {
         Point* pPoint = NULL;
         pPoint = *iter;
         if (pPoint != NULL)
         {
            PointAdapter* pNewPoint = NULL;
            pNewPoint = static_cast<PointAdapter*> (addPoint());
            if (pNewPoint != NULL)
            {
               *pNewPoint = *(static_cast<PointAdapter*> (pPoint));
            }
         }

         ++iter;
      }

      mSymbols = object.mSymbols;
      mLine = object.mLine;
      mLineColor = object.mLineColor;
      mLineWidth = object.mLineWidth;
      mLineStyle = object.mLineStyle;

      if (getInteractive())
      {
         emit legendPixmapChanged();
         notify(SIGNAL_NAME(Subject, Modified));
      }
      else
      {
         mDirty = true;
      }
   }

   return *this;
}

PlotObjectType PointSetImp::getType() const
{
   return POINT_SET;
}

void PointSetImp::draw()
{
   if (isVisible() == false)
   {
      return;
   }

   if (mPoints.size() == 0)
   {
      return;
   }

   // Line
   if (mLine == true)
   {
      PlotViewImp* pPlot = getPlot();
      VERIFYNRV(pPlot != NULL);

      if (pPlot->isShadingEnabled() == false)
      {
         glColor3ub(mLineColor.red(), mLineColor.green(), mLineColor.blue());
         glShadeModel(GL_FLAT);
      }
      else
      {
         glShadeModel(GL_SMOOTH);
      }
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

      double dWorldX = 0.0;
      double dWorldY = 0.0;

      vector<Point*>::iterator iter = mPoints.begin();
      while (iter != mPoints.end())
      {
         Point* pPoint = NULL;
         pPoint = *iter;
         if (pPoint != NULL)
         {
            const LocationType& point = pPoint->getLocation();
            pPlot->translateDataToWorld(point.mX, point.mY, dWorldX, dWorldY);
            if (pPlot->isShadingEnabled() == true)
            {
               ColorType color = pPoint->getColor();
               glColor3ub(color.mRed, color.mGreen, color.mBlue);
            }
            glVertex2d(dWorldX, dWorldY);
         }

         ++iter;
      }

      glEnd();

      if (mLineStyle != SOLID_LINE)
      {
         glDisable(GL_LINE_STIPPLE);
      }

      glLineWidth(1);

      // Turn the shade model back to flat to not impact other types of plot objects
      glShadeModel(GL_FLAT);
   }
   
   // Points
   LocationType pixelSize(1.0, 1.0);

   PlotViewImp* pPlot = dynamic_cast<PlotViewImp*> (getPlot());
   if (pPlot != NULL)
   {
      pixelSize = pPlot->getPixelSize();
   }

   vector<Point*>::iterator iter = mPoints.begin();
   while (iter != mPoints.end())
   {
      PointAdapter* pPoint = NULL;
      pPoint = static_cast<PointAdapter*> (*iter);
      if (pPoint != NULL)
      {
         if ((mSymbols == true) || (mLine == false) || (pPoint->isSelected() == true))
         {
            pPoint->draw(pixelSize);
         }
      }

      ++iter;
   }
}

Point* PointSetImp::addPoint()
{
   PlotViewImp* pPlot = getPlot();
   bool bPrimary = isPrimary();

   Point* pPoint = NULL;
   pPoint = new PointAdapter(pPlot, bPrimary);
   if (pPoint != NULL)
   {
      insertPoint(pPoint);
   }

   return pPoint;
}

Point* PointSetImp::addPoint(double dX, double dY)
{
   Point* pPoint = NULL;
   pPoint = addPoint();
   if (pPoint != NULL)
   {
      pPoint->setLocation(dX, dY);
   }

   return pPoint;
}

bool PointSetImp::insertPoint(Point* pPoint)
{
   if (pPoint == NULL)
   {
      return false;
   }

   if (hasPoint(pPoint) == true)
   {
      return false;
   }

   pPoint->attach(SIGNAL_NAME(Point, LocationChanged), Slot(this, &PointSetImp::propagateLocationChanged));
   mPoints.push_back(pPoint);
   if (getInteractive())
   {
      emit pointAdded(pPoint);
      notify(SIGNAL_NAME(PointSet, PointAdded), boost::any(pPoint));
   }
   else
   {
      mDirty = true;
   }
   return true;
}

void PointSetImp::setPoints(const vector<Point*>& points)
{
   clear(true);

   vector<Point*>::const_iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter)
   {
      Point* pPoint = NULL;
      pPoint = *iter;
      if (pPoint != NULL)
      {
         pPoint->attach(SIGNAL_NAME(Point, LocationChanged), Slot(this, &PointSetImp::propagateLocationChanged));
         mPoints.push_back(pPoint);
      }
   }

   if (getInteractive())
   {
      emit pointsSet(mPoints);
      notify(SIGNAL_NAME(PointSet, PointsSet), boost::any(mPoints));
   }
   else
   {
      mDirty = true;
   }
}

vector<Point*> PointSetImp::getPoints() const
{
   return mPoints;
}

unsigned int PointSetImp::getNumPoints() const
{
   return mPoints.size();
}

bool PointSetImp::hasPoint(Point* pPoint) const
{
   if (pPoint == NULL)
   {
      return false;
   }

   vector<Point*>::const_iterator iter = mPoints.begin();
   while (iter != mPoints.end())
   {
      Point* pCurrentPoint = NULL;
      pCurrentPoint = *iter;
      if (pCurrentPoint == pPoint)
      {
         return true;
      }

      ++iter;
   }

   return false;
}

bool PointSetImp::removePoint(Point* pPoint, bool bDelete)
{
   if (pPoint == NULL)
   {
      return false;
   }

   vector<Point*>::iterator iter = mPoints.begin();
   while (iter != mPoints.end())
   {
      Point* pCurrentPoint = NULL;
      pCurrentPoint = *iter;
      if (pCurrentPoint == pPoint)
      {
         PointImp* pPointImp = dynamic_cast<PointImp*> (pPoint);
         if (pPointImp != NULL)
         {
            pPointImp->detach(SIGNAL_NAME(Point, LocationChanged), Slot(this, &PointSetImp::propagateLocationChanged));
            mPoints.erase(iter);
            if (getInteractive())
            {
               emit pointRemoved(pPoint);
            }

            if (bDelete == true)
            {
               delete pPointImp;
            }

            if (getInteractive())
            {
               notify(SIGNAL_NAME(Subject, Modified));
            }
            else
            {
               mDirty = true;
            }
            return true;
         }
      }

      ++iter;
   }

   return false;
}

void PointSetImp::clear(bool bDelete)
{
   for (vector<Point*>::iterator iter = mPoints.begin(); iter != mPoints.end();
      ++iter)
   {
      PointImp* pPoint = dynamic_cast<PointImp*>(*iter);
      if (NN(pPoint))
      {
         pPoint->detach(SIGNAL_NAME(Point, LocationChanged), Slot(this, &PointSetImp::propagateLocationChanged));
         if (bDelete == true)
         {
            delete pPoint;
         }
      }
   }

   mPoints.clear();
   if (getInteractive())
   {
      emit pointsSet(mPoints);
      notify(SIGNAL_NAME(PointSet, PointsSet), boost::any(mPoints));
   }
   else
   {
      mDirty = true;
   }
}

bool PointSetImp::areSymbolsDisplayed() const
{
   return mSymbols;
}

bool PointSetImp::isLineDisplayed() const
{
   return mLine;
}

QColor PointSetImp::getLineColor() const
{
   return mLineColor;
}

int PointSetImp::getLineWidth() const
{
   return mLineWidth;
}

LineStyle PointSetImp::getLineStyle() const
{
   return mLineStyle;
}

Point* PointSetImp::hitPoint(LocationType point) const
{
   vector<Point*>::const_reverse_iterator iter = mPoints.rbegin();
   while (iter != mPoints.rend())
   {
      Point* pPoint = NULL;
      pPoint = *iter;
      if (pPoint != NULL)
      {
         bool bHit = false;
         bHit = pPoint->hit(point);
         if (bHit == true)
         {
            return pPoint;
         }
      }

      ++iter;
   }

   return NULL;
}

bool PointSetImp::hit(LocationType point) const
{
   if (mLine == true)
   {
      if (mPoints.size() > 0)
      {
         PlotViewImp* pPlot = getPlot();
         if (pPlot != NULL)
         {
            double dPointX = 0.0;
            double dPointY = 0.0;
            pPlot->translateWorldToScreen(point.mX, point.mY, dPointX, dPointY);

            point.mX = dPointX;
            point.mY = dPointY;

            LocationType oldLocation;
            LocationType currentLocation;

            Point* pPoint = NULL;
            pPoint = mPoints.front();
            if (pPoint != NULL)
            {
               const LocationType& location = pPoint->getLocation();

               double dLocationX = 0.0;
               double dLocationY = 0.0;
               pPlot->translateDataToScreen(location.mX, location.mY, dLocationX, dLocationY);

               oldLocation.mX = dLocationX;
               oldLocation.mY = dLocationY;
            }

            for (unsigned int i = 1; i < mPoints.size(); i++)
            {
               pPoint = mPoints.at(i);
               if (pPoint != NULL)
               {
                  const LocationType& location = pPoint->getLocation();

                  double dLocationX = 0.0;
                  double dLocationY = 0.0;
                  pPlot->translateDataToScreen(location.mX, location.mY, dLocationX, dLocationY);

                  currentLocation.mX = dLocationX;
                  currentLocation.mY = dLocationY;

                  bool bHit = false;
                  bHit = DrawUtil::lineHit(oldLocation, currentLocation, point, 3.0);
                  if (bHit == true)
                  {
                     return true;
                  }

                  oldLocation = currentLocation;
               }
            }
         }
      }
   }

   Point* pPoint = hitPoint(point);
   return (pPoint != NULL);
}

bool PointSetImp::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
   if (mPoints.size() == 0)
   {
      return false;
   }

   dMinX = DBL_MAX;
   dMinY = DBL_MAX;
   dMaxX = -DBL_MAX;
   dMaxY = -DBL_MAX;

   vector<Point*>::iterator iter = mPoints.begin();
   while (iter != mPoints.end())
   {
      Point* pPoint = NULL;
      pPoint = *iter;
      if (pPoint != NULL)
      {
         double dCurrentMinX = 0.0;
         double dCurrentMinY = 0.0;
         double dCurrentMaxX = 0.0;
         double dCurrentMaxY = 0.0;

         bool bSuccess = false;
         bSuccess = pPoint->getExtents(dCurrentMinX, dCurrentMinY, dCurrentMaxX, dCurrentMaxY);
         if (bSuccess == false)
         {
            dMinX = -1.0;
            dMinY = -1.0;
            dMaxX = 1.0;
            dMaxY = 1.0;

            return false;
         }

         if (dCurrentMinX < dMinX)
         {
            dMinX = dCurrentMinX;
         }

         if (dCurrentMinY < dMinY)
         {
            dMinY = dCurrentMinY;
         }

         if (dCurrentMaxX > dMaxX)
         {
            dMaxX = dCurrentMaxX;
         }

         if (dCurrentMaxY > dMaxY)
         {
            dMaxY = dCurrentMaxY;
         }
      }

      ++iter;
   }

   return true;
}

const QPixmap& PointSetImp::getLegendPixmap(bool bSelected) const
{
   static QPixmap pix(25, 15);
   static QPixmap selectedPix(25, 15);
   static QColor pixColor;
   static QColor selectedPixColor;

   if ((bSelected == true) && (selectedPix.isNull() == false))
   {
      if (selectedPixColor != mLineColor)
      {
         selectedPixColor = mLineColor;
         selectedPix.fill(Qt::white);

         QRect rcPixmap = selectedPix.rect();

         QPolygon points(4);
         points.setPoint(0, rcPixmap.center().x() - 4, rcPixmap.center().y());
         points.setPoint(1, rcPixmap.center().x(), rcPixmap.center().y() + 4);
         points.setPoint(2, rcPixmap.center().x() + 4, rcPixmap.center().y());
         points.setPoint(3, rcPixmap.center().x(), rcPixmap.center().y() - 4);

         QPainter p(&selectedPix);
         p.setPen(QPen(mLineColor, 1));
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
      if (pixColor != mLineColor)
      {
         pixColor = mLineColor;
         pix.fill(Qt::white);

         QRect rcPixmap = pix.rect();

         QPainter p(&pix);
         p.setPen(QPen(mLineColor, 1));
         p.drawLine(rcPixmap.left() + 2, rcPixmap.center().y(), rcPixmap.right() - 2, rcPixmap.center().y());
         p.end();
      }

      return pix;
   }

   return PlotObjectImp::getLegendPixmap(bSelected);
}

void PointSetImp::setVisible(bool bVisible)
{
   if (isVisible() != bVisible)
   {
      vector<Point*>::iterator iter = mPoints.begin();
      while (iter != mPoints.end())
      {
         Point* pPoint = NULL;
         pPoint = *iter;
         if (pPoint != NULL)
         {
            pPoint->setVisible(bVisible);
         }

         ++iter;
      }
   }

   PlotObjectImp::setVisible(bVisible);
}

void PointSetImp::setSelected(bool bSelect)
{
   PlotViewImp* pPlot = getPlot();
   VERIFYNRV(pPlot != NULL);

   vector<Point*>::iterator iter = mPoints.begin();
   while (iter != mPoints.end())
   {
      Point* pPoint = NULL;
      pPoint = *iter;
      if (pPoint != NULL)
      {
         pPoint->setSelected(bSelect);
      }
      ++iter;
   }

   PlotObjectImp::setSelected(bSelect);
}

void PointSetImp::displaySymbols(bool bDisplay)
{
   mSymbols = bDisplay;
}

void PointSetImp::setPointSymbol(const Point::PointSymbolType& eSymbol)
{
   vector<Point*>::iterator iter = mPoints.begin();
   while (iter != mPoints.end())
   {
      Point* pPoint = NULL;
      pPoint = *iter;
      if (pPoint != NULL)
      {
         pPoint->setSymbol(eSymbol);
      }

      ++iter;
   }
}

void PointSetImp::setPointSymbolSize(unsigned int symbolSize)
{
   vector<Point*>::iterator iter = mPoints.begin();
   while (iter != mPoints.end())
   {
      Point* pPoint = NULL;
      pPoint = *iter;
      if (pPoint != NULL)
      {
         pPoint->setSymbolSize(symbolSize);
      }

      ++iter;
   }
}

void PointSetImp::setPointColor(const QColor& clrSymbol)
{
   if (clrSymbol.isValid() == false)
   {
      return;
   }

   vector<Point*>::iterator iter = mPoints.begin();
   while (iter != mPoints.end())
   {
      Point* pPoint = *iter;
      if (pPoint != NULL)
      {
         pPoint->setColor(ColorType(clrSymbol.red(), clrSymbol.green(), clrSymbol.blue()));
      }

      ++iter;
   }
}

void PointSetImp::displayLine(bool bDisplay)
{
   if (mLine != bDisplay)
   {
      mLine = bDisplay;
      emit lineDisplayChanged(bDisplay);
   }
}

void PointSetImp::setLineColor(const QColor& clrLine)
{
   if (clrLine.isValid() == false)
   {
      return;
   }

   if (clrLine != mLineColor)
   {
      mLineColor = clrLine;
      if (getInteractive())
      {
         emit legendPixmapChanged();
         notify(SIGNAL_NAME(Subject, Modified));
      }
      else
      {
         mDirty = true;
      }
   }
}

void PointSetImp::setLineWidth(int iWidth)
{
   if (iWidth < 0)
   {
      iWidth = 0;
   }

   if (iWidth != mLineWidth)
   {
      mLineWidth = iWidth;
      if (getInteractive())
      {
         notify(SIGNAL_NAME(Subject, Modified));
      }
      else
      {
         mDirty = true;
      }
   }
}

void PointSetImp::setLineStyle(const LineStyle& eStyle)
{
   if (eStyle != mLineStyle)
   {
      mLineStyle = eStyle;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void PointSetImp::propagateLocationChanged(Subject& subject, const string& signal, const boost::any& value)
{
   Point* pPoint = dynamic_cast<Point*>(&subject);
   LocationType pointLocation = boost::any_cast<LocationType>(value);
   if (pPoint != NULL)
   {
      if (getInteractive())
      {
         emit pointLocationChanged(pPoint, pointLocation);
      }
      else
      {
         mDirty = true;
      }
   }
}

void PointSetImp::setInteractive(bool interactive)
{
   if (interactive == mInteractive)
   {
      return;
   }

   if (interactive)
   {
      if (mDirty)
      {
         emit pointsSet(mPoints);
         notify(SIGNAL_NAME(PointSet, PointsSet), boost::any(mPoints));
      }
   }

   mInteractive = interactive;
   mDirty = false;
}

bool PointSetImp::getInteractive()
{
   return mInteractive;
}

void PointSetImp::deleteSelectedPoints(bool filterVisible)
{
   vector<Point*> newPoints;
   for (vector<Point*>::iterator iter = mPoints.begin(); iter != mPoints.end(); ++iter)
   {
      Point* pPoint = *iter;
      if (NN(pPoint))
      {
         if (pPoint->isSelected() && (!filterVisible || pPoint->isVisible()))
         {
            PointImp* pPointImp = dynamic_cast<PointImp*>(pPoint);
            if (NN(pPointImp))
            {
               delete pPointImp;
            }
         }
         else
         {
            newPoints.push_back(pPoint);
         }
      }
   }
   mPoints = newPoints;
   emit pointsSet(mPoints);
   notify(SIGNAL_NAME(PointSet, PointsSet), boost::any(mPoints));
}

bool PointSetImp::toXml(XMLWriter* pXml) const
{
   if (!PlotObjectImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("symbols", mSymbols);
   pXml->addAttr("line", mLine);
   pXml->addAttr("lineColor", QCOLOR_TO_COLORTYPE(mLineColor));
   pXml->addAttr("lineWidth", mLineWidth);
   pXml->addAttr("lineStyle", mLineStyle);

   for (vector<Point*>::const_iterator it = mPoints.begin(); it != mPoints.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("Point"));
      const PointImp* pPoint = dynamic_cast<PointImp*>(*it);
      if (pPoint == NULL || !pPoint->toXml(pXml))
      {
         return false;
      }
      pXml->popAddPoint();
   }
   return true;
}

bool PointSetImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !PlotObjectImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   mSymbols = StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("symbols"))));
   mLine = StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("line"))));
   ColorType color = StringUtilities::fromXmlString<ColorType>(A(pElem->getAttribute(X("lineColor"))));
   mLineColor = COLORTYPE_TO_QCOLOR(color);
   mLineWidth = StringUtilities::fromXmlString<int>(A(pElem->getAttribute(X("lineWidth"))));
   mLineStyle = StringUtilities::fromXmlString<LineStyle>(A(pElem->getAttribute(X("lineStyle"))));

   for (DOMNode* pChld = pElem->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("Point")))
      {
         PointImp* pPoint = dynamic_cast<PointImp*>(addPoint());
         if (pPoint == NULL || !pPoint->fromXml(pChld, version))
         {
            return false;
         }
      }
   }

   return true;
}
