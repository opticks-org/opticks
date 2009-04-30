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

#include "AppVerify.h"
#include "glCommon.h"
#include "Point.h"
#include "PointImp.h"
#include "PointSet.h"
#include "PlotView.h"
#include "PlotViewImp.h"

#include <sstream>

using namespace std;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

PointImp::PointImp(PlotViewImp* pPlot, bool bPrimary) :
   PlotObjectImp(pPlot, bPrimary),
   mLocation(LocationType()),
   mSymbol(Point::SOLID),
   mSymbolSize(5),
   mColor(Qt::black),
   mpPointSet(NULL)
{
   VERIFYNR(connect(this, SIGNAL(locationChanged(const LocationType&)), this, SIGNAL(extentsChanged())));
   VERIFYNR(connect(this, SIGNAL(symbolSizeChanged(int)), this, SIGNAL(legendPixmapChanged())));
   VERIFYNR(connect(this, SIGNAL(symbolChanged(const Point::PointSymbolType&)), this, SIGNAL(legendPixmapChanged())));
   VERIFYNR(connect(this, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(legendPixmapChanged())));
}

PointImp::PointImp(PlotViewImp* pPlot, bool bPrimary, LocationType point) :
   PlotObjectImp(pPlot, bPrimary),
   mLocation(point),
   mSymbol(Point::SOLID),
   mSymbolSize(5),
   mColor(Qt::black),
   mpPointSet(NULL)
{
   VERIFYNR(connect(this, SIGNAL(locationChanged(const LocationType&)), this, SIGNAL(extentsChanged())));
   VERIFYNR(connect(this, SIGNAL(symbolSizeChanged(int)), this, SIGNAL(legendPixmapChanged())));
   VERIFYNR(connect(this, SIGNAL(symbolChanged(const Point::PointSymbolType&)), this, SIGNAL(legendPixmapChanged())));
   VERIFYNR(connect(this, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(legendPixmapChanged())));
}

PointImp::PointImp(PlotViewImp* pPlot, bool bPrimary, double dX, double dY) :
   PlotObjectImp(pPlot, bPrimary),
   mLocation(dX, dY),
   mSymbol(Point::SOLID),
   mSymbolSize(5),
   mColor(Qt::black),
   mpPointSet(NULL)
{
   VERIFYNR(connect(this, SIGNAL(locationChanged(const LocationType&)), this, SIGNAL(extentsChanged())));
   VERIFYNR(connect(this, SIGNAL(symbolSizeChanged(int)), this, SIGNAL(legendPixmapChanged())));
   VERIFYNR(connect(this, SIGNAL(symbolChanged(const Point::PointSymbolType&)), this, SIGNAL(legendPixmapChanged())));
   VERIFYNR(connect(this, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(legendPixmapChanged())));
}

PointImp::~PointImp()
{
}

PointImp& PointImp::operator= (const PointImp& object)
{
   if (this != &object)
   {
      PlotObjectImp::operator= (object);

      mLocation = object.mLocation;
      mSymbol = object.mSymbol;
      mSymbolSize = object.mSymbolSize;
      mColor = object.mColor;
      mpPointSet = object.mpPointSet;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

PlotObjectType PointImp::getType() const
{
   return POINT_OBJECT;
}

void PointImp::draw()
{
   LocationType pixelSize(1.0, 1.0);

   PlotViewImp* pPlot = dynamic_cast<PlotViewImp*> (getPlot());
   if (pPlot != NULL)
   {
      pixelSize = pPlot->getPixelSize();
   }

   draw(pixelSize);
}

void PointImp::draw(LocationType pixelSize)
{
   if (isVisible() == false)
   {
      return;
   }

   PlotViewImp* pPlot = dynamic_cast<PlotViewImp*>(getPlot());
   VERIFYNRV(pPlot != NULL);

   if (pixelSize.mX == 0.0)
   {
      pixelSize.mX = 1.0;
   }

   if (pixelSize.mY == 0.0)
   {
      pixelSize.mY = 1.0;
   }

   double dSymbolSizeX = mSymbolSize / pixelSize.mX;
   double dSymbolSizeY = mSymbolSize / pixelSize.mY;
   double dWorldX = 0.0;
   double dWorldY = 0.0;

   pPlot->translateDataToWorld(mLocation.mX, mLocation.mY, dWorldX, dWorldY);

   // Increase point size when selected
   if (isSelected() == true)
   {
      switch (pPlot->getSelectionDisplayMode())
      {
      case SYMBOL_SELECTION: default:
         dSymbolSizeX = (mSymbolSize + 3) / pixelSize.mX;
         dSymbolSizeY = (mSymbolSize + 3) / pixelSize.mY;
         break;

      case INVERT_SELECTION:
         // Invert symbol color and draw slightly enlarged symbol behind it when selected
         glPushMatrix();
         glLineWidth(2);

         // Invert the color
         glColor3ub(255 - mColor.red(), 255 - mColor.green(), 255 - mColor.blue());

         // Draw the inverted symbol
         glTranslatef(dWorldX, dWorldY, 0);
         glScalef( (mSymbolSize + 1) / pixelSize.mX, (mSymbolSize + 1) / pixelSize.mY, 0);
         glCallList(pPlot->getDisplayListIndex() + mSymbol);
         glLineWidth(1);
         glPopMatrix();
         break;
      case BOX_SELECTION:
         // Draw a gray box around the symbol if it is selected
         glPushMatrix();
         glLineWidth(2);
         glColor3ub(212, 208, 200);

         glTranslatef(dWorldX, dWorldY, 0);
         glScalef((mSymbolSize+2) / pixelSize.mX, (mSymbolSize+2) / pixelSize.mY, 0);
         glCallList(pPlot->getDisplayListIndex() + BOX);
         glLineWidth(1);
         glPopMatrix();
         break;
      }
   }

   // Set the color
   glColor3ub(mColor.red(), mColor.green(), mColor.blue());

   glPushMatrix();
   glTranslatef(dWorldX, dWorldY, 0);
   glScalef(dSymbolSizeX, dSymbolSizeY, 0);
   glCallList(pPlot->getDisplayListIndex() + mSymbol);
   glPopMatrix();
}

double PointImp::getXLocation() const
{
   return mLocation.mX;
}

double PointImp::getYLocation() const
{
   return mLocation.mY;
}

const LocationType& PointImp::getLocation() const
{
   return mLocation;
}

Point::PointSymbolType PointImp::getSymbol() const
{
   return mSymbol;
}

int PointImp::getSymbolSize() const
{
   return mSymbolSize;
}

QColor PointImp::getColor() const
{
   return mColor;
}

bool PointImp::hit(LocationType point) const
{
   PlotViewImp* pPlot = getPlot();
   if (pPlot == NULL)
   {
      return false;
   }

   double dScreenX = 0.0;
   double dScreenY = 0.0;
   pPlot->translateWorldToScreen(point.mX, point.mY, dScreenX, dScreenY);

   double dCurrentScreenX = 0.0;
   double dCurrentScreenY = 0.0;
   double dCurrentWorldX = 0.0;
   double dCurrentWorldY = 0.0;

   pPlot->translateDataToScreen(mLocation.mX, mLocation.mY, dCurrentScreenX, dCurrentScreenY);

   double dXDist = fabs(dCurrentScreenX - dScreenX);
   double dYDist = fabs(dCurrentScreenY - dScreenY);

   double dDist = mSymbolSize;
   if (isSelected() == true)
   {
      dDist *= 2.0;
   }

   if ((dXDist < dDist) && (dYDist < dDist))
   {
      return true;
   }

   return false;
}

bool PointImp::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
   PlotViewImp* pPlot = getPlot();
   VERIFY(pPlot != NULL);

   double dWorldX = 0.0;
   double dWorldY = 0.0;

   pPlot->translateDataToWorld(mLocation.mX, mLocation.mY, dWorldX, dWorldY);

   dMinX = dMaxX = dWorldX;
   dMinY = dMaxY = dWorldY;
   return true;
}

const QPixmap& PointImp::getLegendPixmap(bool bSelected) const
{
   static QPixmap pix(25, 15);
   static QPixmap selectedPix(25, 15);
   static QColor pixColor;
   static Point::PointSymbolType pixSymbol;
   static int pixWidth;
   static int pixHeight;
   static QColor selectedPixColor;
   static Point::PointSymbolType selectedPixSymbol;
   static int selectedPixWidth;
   static int selectedPixHeight;

   int iSymbolWidth = mSymbolSize;
   int iSymbolHeight = mSymbolSize;
   if (bSelected == true)
   {
      iSymbolWidth = mSymbolSize * 2;
      iSymbolHeight = mSymbolSize * 2;
   }

   int iWidth = iSymbolWidth;
   int iHeight = iSymbolHeight;

   if (iWidth < 25)
   {
      iWidth = 25;
   }

   if (iHeight < 15)
   {
      iHeight = 15;
   }

   if ((bSelected == true) && (selectedPix.isNull() == false))
   {
      if ((selectedPixColor != mColor) || (selectedPixSymbol != mSymbol) ||
         (selectedPixWidth != iWidth) || (selectedPixHeight != iHeight))
      {
         selectedPixColor = mColor;
         selectedPixSymbol = mSymbol;
         selectedPixWidth = iWidth;
         selectedPixHeight = iHeight;
         selectedPix.fill(Qt::transparent);

         QRect rcPix = selectedPix.rect();

         int iSymbolMinX = rcPix.center().x() - iSymbolWidth / 2.0;
         int iSymbolMinY = rcPix.center().y() - iSymbolHeight / 2.0;
         QRect rcSymbol(iSymbolMinX, iSymbolMinY, iSymbolWidth, iSymbolHeight);

         QPainter p(&selectedPix);
         p.setPen(QPen(mColor, 1));

         // Used with drawPolygon
         QPointF points[8];

         switch (mSymbol)
         {
            case Point::SOLID: default:
               p.fillRect(rcSymbol, mColor);
               break;

            case Point::X:
               p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
               p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
               break;

            case Point::CROSS_HAIR:
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
                  rcSymbol.center().y());
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               break;

            case Point::ASTERISK:
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               p.drawLine(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.right(),
                  rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3),
                  rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
               break;

            case Point::VERTICAL_LINE:
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               break;

            case Point::HORIZONTAL_LINE:
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
                  rcSymbol.center().y());
               break;

            case Point::FORWARD_SLASH:
               p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
               break;

            case Point::BACK_SLASH:
               p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
               break;

            case Point::BOX:
               p.drawRect(rcSymbol);
               break;

            case Point::BOXED_X:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
               p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
               break;

            case Point::BOXED_CROSS_HAIR:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
                  rcSymbol.center().y());
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               break;

            case Point::BOXED_ASTERISK:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               p.drawLine(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.right(),
                  rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3),
                  rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
               break;

            case Point::BOXED_VERTICAL_LINE:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               break;

            case Point::BOXED_HORIZONTAL_LINE:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
                  rcSymbol.center().y());
               break;

            case Point::BOXED_FORWARD_SLASH:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
               break;

            case Point::BOXED_BACK_SLASH:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
               break;
            case Point::DIAMOND:
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.right(), rcSymbol.center().y());
               p.drawLine(rcSymbol.right(), rcSymbol.center().y(), rcSymbol.center().x(), rcSymbol.bottom());
               p.drawLine(rcSymbol.center().x(), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.center().y());
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.center().x(), rcSymbol.top());
               break;

            case Point::DIAMOND_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.center().x(), rcSymbol.top());
               points[1] = QPointF(rcSymbol.right(), rcSymbol.center().y());
               points[2] = QPointF(rcSymbol.center().x(), rcSymbol.bottom());
               points[3] = QPointF(rcSymbol.left(), rcSymbol.center().y());
               p.drawPolygon(points, 4);
               break;

            case Point::DIAMOND_CROSS_HAIR:
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.right(), rcSymbol.center().y());
               p.drawLine(rcSymbol.right(), rcSymbol.center().y(), rcSymbol.center().x(), rcSymbol.bottom());
               p.drawLine(rcSymbol.center().x(), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.center().y());
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.center().x(), rcSymbol.top());

               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(), rcSymbol.center().y());
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(), rcSymbol.bottom());
               break;

            case Point::TRIANGLE:
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.right(), rcSymbol.bottom());
               p.drawLine(rcSymbol.bottomRight(), rcSymbol.bottomLeft());
               p.drawLine(rcSymbol.left(), rcSymbol.bottom(), rcSymbol.center().x(), rcSymbol.top());
               break;

            case Point::TRIANGLE_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.center().x(), rcSymbol.top());
               points[1] = QPointF(rcSymbol.right(), rcSymbol.bottom());
               points[2] = QPointF(rcSymbol.left(), rcSymbol.bottom());
               p.drawPolygon(points, 3);
               break;

            case Point::RIGHT_TRIANGLE:
               p.drawLine(rcSymbol.left(), rcSymbol.top(), rcSymbol.right(), rcSymbol.center().y());
               p.drawLine(rcSymbol.right(), rcSymbol.center().y(), rcSymbol.left(), rcSymbol.bottom());
               p.drawLine(rcSymbol.left(), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.top());
               break;

            case Point::RIGHT_TRIANGLE_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.left(), rcSymbol.top());
               points[1] = QPointF(rcSymbol.right(), rcSymbol.center().y());
               points[2] = QPointF(rcSymbol.left(), rcSymbol.bottom());
               p.drawPolygon(points, 3);
               break;

            case Point::LEFT_TRIANGLE:
               p.drawLine(rcSymbol.right(), rcSymbol.top(), rcSymbol.right(), rcSymbol.bottom());
               p.drawLine(rcSymbol.right(), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.center().y());
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(), rcSymbol.top());
               break;

            case Point::LEFT_TRIANGLE_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.right(), rcSymbol.top());
               points[1] = QPointF(rcSymbol.right(), rcSymbol.bottom());
               points[2] = QPointF(rcSymbol.left(), rcSymbol.center().y());
               p.drawPolygon(points, 3);
               break;

            case Point::DOWN_TRIANGLE:
               p.drawLine(rcSymbol.topLeft(), rcSymbol.topRight());
               p.drawLine(rcSymbol.right(), rcSymbol.top(), rcSymbol.center().x(), rcSymbol.bottom());
               p.drawLine(rcSymbol.center().x(), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.top());
               break;

            case Point::DOWN_TRIANGLE_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.left(), rcSymbol.top());
               points[1] = QPointF(rcSymbol.right(), rcSymbol.top());
               points[2] = QPointF(rcSymbol.center().x(), rcSymbol.bottom());
               p.drawPolygon(points, 3);
               break;

            case Point::CIRCLE:
               p.drawEllipse(rcSymbol);
               break;

            case Point::CIRCLE_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               p.drawEllipse(rcSymbol);
               break;

            case Point::OCTAGON:
               p.drawLine(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.top(), rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.top());
               p.drawLine(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.top(), rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
               p.drawLine(rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.right(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.right(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3), rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.bottom());
               p.drawLine(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.bottom(), rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.bottom());
               p.drawLine(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3), rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.top());
               break;

            case Point::OCTAGON_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.top());
               points[1] = QPointF(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.top());
               points[2] = QPointF(rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
               points[3] = QPointF(rcSymbol.right(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               points[4] = QPointF(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.bottom());
               points[5] = QPointF(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.bottom());
               points[6] = QPointF(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               points[7] = QPointF(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4));
               p.drawPolygon(points, 8);
               break;

            case Point::OCTAGON_CROSS_HAIR:
               p.drawLine(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.top(), rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.top());
               p.drawLine(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.top(), rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
               p.drawLine(rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.right(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.right(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3), rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.bottom());
               p.drawLine(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.bottom(), rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.bottom());
               p.drawLine(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3), rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.top());

               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(), rcSymbol.center().y());
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(), rcSymbol.bottom());
               break;
         }

         p.end();
      }

      return selectedPix;
   }
   else if ((bSelected == false) && (pix.isNull() == false))
   {
      if ((pixColor != mColor) || (pixSymbol != mSymbol) ||
         (pixWidth != iWidth) || (pixHeight != iHeight))
      {
         pixColor = mColor;
         pixSymbol = mSymbol;
         pixWidth = iWidth;
         pixHeight = iHeight;
         pix.fill(Qt::transparent);

         QRect rcPix = pix.rect();

         int iSymbolMinX = rcPix.center().x() - iSymbolWidth / 2.0;
         int iSymbolMinY = rcPix.center().y() - iSymbolHeight / 2.0;
         QRect rcSymbol(iSymbolMinX, iSymbolMinY, iSymbolWidth, iSymbolHeight);

         QPainter p(&pix);
         p.setPen(QPen(mColor, 1));

         // Used with drawPolygon
         QPointF points[8];

         switch (mSymbol)
         {
            case Point::SOLID: default:
               p.fillRect(rcSymbol, mColor);
               break;

            case Point::X:
               p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
               p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
               break;

            case Point::CROSS_HAIR:
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
                  rcSymbol.center().y());
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               break;

            case Point::ASTERISK:
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               p.drawLine(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.right(),
                  rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3),
                  rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
               break;

            case Point::VERTICAL_LINE:
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               break;

            case Point::HORIZONTAL_LINE:
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
                  rcSymbol.center().y());
               break;

            case Point::FORWARD_SLASH:
               p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
               break;

            case Point::BACK_SLASH:
               p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
               break;

            case Point::BOX:
               p.drawRect(rcSymbol);
               break;

            case Point::BOXED_X:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
               p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
               break;

            case Point::BOXED_CROSS_HAIR:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
                  rcSymbol.center().y());
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               break;

            case Point::BOXED_ASTERISK:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               p.drawLine(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.right(),
                  rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3),
                  rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
               break;

            case Point::BOXED_VERTICAL_LINE:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
                  rcSymbol.bottom());
               break;

            case Point::BOXED_HORIZONTAL_LINE:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
                  rcSymbol.center().y());
               break;

            case Point::BOXED_FORWARD_SLASH:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
               break;

            case Point::BOXED_BACK_SLASH:
               p.drawRect(rcSymbol);
               p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
               break;
            
            case Point::DIAMOND:
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.right(), rcSymbol.center().y());
               p.drawLine(rcSymbol.right(), rcSymbol.center().y(), rcSymbol.center().x(), rcSymbol.bottom());
               p.drawLine(rcSymbol.center().x(), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.center().y());
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.center().x(), rcSymbol.top());
               break;

            case Point::DIAMOND_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.center().x(), rcSymbol.top());
               points[1] = QPointF(rcSymbol.right(), rcSymbol.center().y());
               points[2] = QPointF(rcSymbol.center().x(), rcSymbol.bottom());
               points[3] = QPointF(rcSymbol.left(), rcSymbol.center().y());
               p.drawPolygon(points, 4);
               break;

            case Point::DIAMOND_CROSS_HAIR:
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.right(), rcSymbol.center().y());
               p.drawLine(rcSymbol.right(), rcSymbol.center().y(), rcSymbol.center().x(), rcSymbol.bottom());
               p.drawLine(rcSymbol.center().x(), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.center().y());
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.center().x(), rcSymbol.top());

               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(), rcSymbol.center().y());
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(), rcSymbol.bottom());
               break;

            case Point::TRIANGLE:
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.right(), rcSymbol.bottom());
               p.drawLine(rcSymbol.bottomRight(), rcSymbol.bottomLeft());
               p.drawLine(rcSymbol.left(), rcSymbol.bottom(), rcSymbol.center().x(), rcSymbol.top());
               break;

            case Point::TRIANGLE_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.center().x(), rcSymbol.top());
               points[1] = QPointF(rcSymbol.right(), rcSymbol.bottom());
               points[2] = QPointF(rcSymbol.left(), rcSymbol.bottom());
               p.drawPolygon(points, 3);
               break;

            case Point::RIGHT_TRIANGLE:
               p.drawLine(rcSymbol.left(), rcSymbol.top(), rcSymbol.right(), rcSymbol.center().y());
               p.drawLine(rcSymbol.right(), rcSymbol.center().y(), rcSymbol.left(), rcSymbol.bottom());
               p.drawLine(rcSymbol.left(), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.top());
               break;

            case Point::RIGHT_TRIANGLE_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.left(), rcSymbol.top());
               points[1] = QPointF(rcSymbol.right(), rcSymbol.center().y());
               points[2] = QPointF(rcSymbol.left(), rcSymbol.bottom());
               p.drawPolygon(points, 3);
               break;

            case Point::LEFT_TRIANGLE:
               p.drawLine(rcSymbol.right(), rcSymbol.top(), rcSymbol.right(), rcSymbol.bottom());
               p.drawLine(rcSymbol.right(), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.center().y());
               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(), rcSymbol.top());
               break;

            case Point::LEFT_TRIANGLE_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.right(), rcSymbol.top());
               points[1] = QPointF(rcSymbol.right(), rcSymbol.bottom());
               points[2] = QPointF(rcSymbol.left(), rcSymbol.center().y());
               p.drawPolygon(points, 3);
               break;

            case Point::DOWN_TRIANGLE:
               p.drawLine(rcSymbol.topLeft(), rcSymbol.topRight());
               p.drawLine(rcSymbol.right(), rcSymbol.top(), rcSymbol.center().x(), rcSymbol.bottom());
               p.drawLine(rcSymbol.center().x(), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.top());
               break;

            case Point::DOWN_TRIANGLE_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.left(), rcSymbol.top());
               points[1] = QPointF(rcSymbol.right(), rcSymbol.top());
               points[2] = QPointF(rcSymbol.center().x(), rcSymbol.bottom());
               p.drawPolygon(points, 3);
               break;

            case Point::CIRCLE:
               p.drawEllipse(rcSymbol);
               break;

            case Point::CIRCLE_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               p.drawEllipse(rcSymbol);
               break;

            case Point::OCTAGON:
               p.drawLine(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.top(), rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.top());
               p.drawLine(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.top(), rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
               p.drawLine(rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.right(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.right(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3), rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.bottom());
               p.drawLine(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.bottom(), rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.bottom());
               p.drawLine(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3), rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.top());
               break;

            case Point::OCTAGON_FILLED:
               p.setBrush(QBrush(Qt::SolidPattern));
               points[0] = QPointF(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.top());
               points[1] = QPointF(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.top());
               points[2] = QPointF(rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
               points[3] = QPointF(rcSymbol.right(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               points[4] = QPointF(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.bottom());
               points[5] = QPointF(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.bottom());
               points[6] = QPointF(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               points[7] = QPointF(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4));
               p.drawPolygon(points, 8);
               break;

            case Point::OCTAGON_CROSS_HAIR:
               p.drawLine(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.top(), rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.top());
               p.drawLine(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.top(), rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
               p.drawLine(rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.right(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.right(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3), rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.bottom());
               p.drawLine(rcSymbol.left() + ((rcSymbol.width() / 4) * 3), rcSymbol.bottom(), rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.bottom());
               p.drawLine(rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.bottom(), rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3), rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4));
               p.drawLine(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.left() + (rcSymbol.width() / 4), rcSymbol.top());

               p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(), rcSymbol.center().y());
               p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(), rcSymbol.bottom());
               break;         
         }

         p.end();
      }

      return pix;
   }

   return PlotObjectImp::getLegendPixmap(bSelected);
}

void PointImp::setLocation(const LocationType& location)
{
   if (location != mLocation)
   {
      mLocation = location;
      emit locationChanged(mLocation);
      notify(SIGNAL_NAME(Point, LocationChanged), boost::any(mLocation));
   }
}

void PointImp::setLocation(double dX, double dY)
{
   setLocation(LocationType(dX, dY));
}

void PointImp::setSymbol(const Point::PointSymbolType& eSymbol)
{
   if (eSymbol != mSymbol)
   {
      mSymbol = eSymbol;
      emit symbolChanged(mSymbol);
      notify(SIGNAL_NAME(Point, SymbolChanged), boost::any(mSymbol));
   }
}

void PointImp::setSymbolSize(int iSize)
{
   if (iSize != mSymbolSize)
   {
      mSymbolSize = iSize;
      emit symbolSizeChanged(mSymbolSize);
      notify(SIGNAL_NAME(Point, SymbolSizeChanged), boost::any(mSymbolSize));
   }
}

void PointImp::setColor(const QColor& clrSymbol)
{
   if (clrSymbol.isValid() == false)
   {
      return;
   }

   if (clrSymbol != mColor)
   {
      mColor = clrSymbol;
      emit colorChanged(mColor);
      notify(SIGNAL_NAME(Point, ColorChanged), boost::any(
         ColorType(mColor.red(), mColor.green(), mColor.blue())));
   }
}

bool PointImp::toXml(XMLWriter* pXml) const
{
   if (!PlotObjectImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("symbolType", mSymbol);
   pXml->addAttr("symbolSize", mSymbolSize);
   pXml->addAttr("color", QCOLOR_TO_COLORTYPE(mColor));
   pXml->addAttr("location", mLocation);
   return true;
}

bool PointImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !PlotObjectImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   mSymbol = StringUtilities::fromXmlString<Point::PointSymbolType>(A(pElem->getAttribute(X("symbolType"))));
   mSymbolSize = StringUtilities::fromXmlString<int>(A(pElem->getAttribute(X("symbolSize"))));
   ColorType color = StringUtilities::fromXmlString<ColorType>(A(pElem->getAttribute(X("color"))));
   mColor = COLORTYPE_TO_QCOLOR(color);
   mLocation = StringUtilities::fromXmlString<LocationType>(A(pElem->getAttribute(X("location"))));

   return true;
}

const PointSet* PointImp::getPointSet() const
{
   return mpPointSet;
}

PointSet* PointImp::getPointSet()
{
   return mpPointSet;
}

void PointImp::setPointSet(PointSet* pPointSet)
{
   mpPointSet = pPointSet;
}
