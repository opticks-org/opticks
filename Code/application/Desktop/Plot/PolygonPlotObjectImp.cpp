/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QPainter>
#include <QtGui/QPolygon>

#include "AppConfig.h"
#include "AppVerify.h"
#include "DrawUtil.h"
#include "glCommon.h"
#include "PlotViewImp.h"
#include "Point.h"
#include "PolygonPlotObjectImp.h"

#include <vector>
#include <boost/shared_array.hpp>

using namespace std;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

namespace
{
   static std::vector<boost::shared_array<GLdouble> > sCombinedVertices;

   extern "C" void combineVertexData(GLdouble coords[3], GLdouble* pVertexData[4], GLfloat weight[4], void** pOutData)
   {
      if (pOutData == NULL)
      {
         return;
      }

      boost::shared_array<GLdouble> vertex(new GLdouble[3]);
      sCombinedVertices.push_back(vertex);

      if (vertex.get() == NULL)
      {
         return;
      }

      vertex[0] = coords[0];
      vertex[1] = coords[1];
      vertex[2] = coords[2];
      *pOutData = vertex.get();
   }
}

PolygonPlotObjectImp::PolygonPlotObjectImp(PlotViewImp* pPlot, bool bPrimary) :
   PointSetImp(pPlot, bPrimary),
   mFillColor(Qt::black),
   mFillStyle(EMPTY_FILL),
   mHatchStyle(SOLID)
{
   VERIFYNR(connect(this, SIGNAL(lineDisplayChanged(bool)), this, SIGNAL(legendPixmapChanged())));
}

PolygonPlotObjectImp::~PolygonPlotObjectImp()
{
}

PolygonPlotObjectImp& PolygonPlotObjectImp::operator= (const PolygonPlotObjectImp& object)
{
   if (this != &object)
   {
      PointSetImp::operator= (object);

      mFillColor = object.mFillColor;
      mFillStyle = object.mFillStyle;
      mHatchStyle = object.mHatchStyle;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

PlotObjectType PolygonPlotObjectImp::getType() const
{
   return POLYGON_OBJECT_TYPE;
}

void PolygonPlotObjectImp::draw()
{
   PlotViewImp* pPlot = getPlot();
   VERIFYNRV(pPlot != NULL);

   if (isVisible() == false)
   {
      return;
   }

   vector<Point*> points = getPoints();
   if (points.size() < 2)
   {
      return;
   }

   if (mFillStyle != EMPTY_FILL)
   {
      unsigned char* pPattern = NULL;
      if (mFillStyle == HATCH)
      {
         pPattern = DrawUtil::getHatchPattern(mHatchStyle);
         if (pPattern != NULL)
         {
            glEnable(GL_POLYGON_STIPPLE);
            glPolygonStipple(pPattern);
         }
      }

      glColor3ub(mFillColor.red(), mFillColor.green(), mFillColor.blue());

      GLUtesselator* pTess = NULL;
      pTess = gluNewTess();
      if (pTess == NULL)
      {
         return;
      }

      gluTessCallback(pTess, GLU_TESS_BEGIN, (void (__stdcall *)(void)) glBegin);
      gluTessCallback(pTess, GLU_TESS_VERTEX, (void (__stdcall *)(void)) glVertex3dv);
      gluTessCallback(pTess, GLU_TESS_COMBINE, (void (__stdcall *)(void)) combineVertexData);
      gluTessCallback(pTess, GLU_TESS_END, glEnd);
      gluTessNormal(pTess, 0.0, 0.0, 1.0);
      gluTessBeginPolygon(pTess, NULL);
      gluTessBeginContour(pTess);
      sCombinedVertices.clear();

      for (unsigned int i = 0; i < points.size(); i++)
      {
         Point* pPoint = NULL;
         pPoint = points.at(i);
         if (pPoint != NULL)
         {
            boost::shared_array<GLdouble> vertex(new GLdouble[3]);
            pPlot->translateDataToWorld(pPoint->getXLocation(), pPoint->getYLocation(),
               vertex[0], vertex[1]);
            vertex[2] = 0.0;

            gluTessVertex(pTess, vertex.get(), vertex.get());
            sCombinedVertices.push_back(vertex);
         }
      }

      gluTessEndContour(pTess);
      gluTessEndPolygon(pTess);
      gluDeleteTess(pTess);
      sCombinedVertices.clear();

      if (pPattern != NULL)
      {
         glDisable(GL_POLYGON_STIPPLE);
      }
   }

   PointSetImp::draw();
}

QColor PolygonPlotObjectImp::getFillColor() const
{
   return mFillColor;
}

FillStyle PolygonPlotObjectImp::getFillStyle() const
{
   return mFillStyle;
}

SymbolType PolygonPlotObjectImp::getHatchStyle() const
{
   return mHatchStyle;
}

bool PolygonPlotObjectImp::isFilled() const
{
   return (mFillStyle != EMPTY_FILL);
}

bool PolygonPlotObjectImp::hit(LocationType point) const
{
   bool bHit = false;
   if (mFillStyle == EMPTY_FILL)
   {
      bHit = PointSetImp::hit(point);
   }
   else
   {
      vector<Point*> points = getPoints();

      unsigned int uiPoints = points.size();
      if (uiPoints < 1)
      {
         return false;
      }

      double* pXVertices = new double[uiPoints + 1];
      double* pYVertices = new double[uiPoints + 1];
      PlotViewImp* pPlot = getPlot();

      for (unsigned int i = 0; i < points.size(); i++)
      {
         Point* pPoint = NULL;
         pPoint = points.at(i);
         if (pPoint != NULL)
         {
            double worldX = pPoint->getXLocation();
            double worldY = pPoint->getYLocation();

            if (pPlot != NULL)
            {
               pPlot->translateDataToWorld(worldX, worldY, worldX, worldY);
            }

            pXVertices[i] = worldX;
            pYVertices[i] = worldY;
         }
      }

      Point* pPoint = NULL;
      pPoint = points.front();
      if (pPoint != NULL)
      {
         double worldX = pPoint->getXLocation();
         double worldY = pPoint->getYLocation();

         if (pPlot != NULL)
         {
            pPlot->translateDataToWorld(worldX, worldY, worldX, worldY);
         }

         pXVertices[uiPoints] = worldX;
         pYVertices[uiPoints] = worldY;
      }

      bHit = DrawUtil::isWithin(point.mX, point.mY, pXVertices, pYVertices, uiPoints + 1);

      delete [] pXVertices;
      delete [] pYVertices;
   }

   return bHit;
}

const QPixmap& PolygonPlotObjectImp::getLegendPixmap(bool bSelected) const
{
   static QPixmap* pix(NULL);
   static QPixmap* selectedPix(NULL);
   if (!pix) pix=new QPixmap(25, 15);
   if (!selectedPix) selectedPix=new QPixmap(25, 15);
   static bool pixLine;
   static QColor pixColor;
   static QColor pixFillColor;
   static FillStyle pixFillStyle;
   static SymbolType pixHatchStyle;
   static bool selectedPixLine;
   static QColor selectedPixColor;
   static QColor selectedPixFillColor;
   static FillStyle selectedPixFillStyle;
   static SymbolType selectedPixHatchStyle;

   QPolygon points(6);
   points.setPoint(0, 6, 13);
   points.setPoint(1, 18, 13);
   points.setPoint(2, 12, 10);
   points.setPoint(3, 17, 5);
   points.setPoint(4, 10, 2);
   points.setPoint(5, 6, 8);

   if ((bSelected == true) && (selectedPix->isNull() == false))
   {
      if ((selectedPixLine != isLineDisplayed()) || (selectedPixColor != getLineColor()) ||
         (selectedPixFillColor != mFillColor) || (selectedPixFillStyle != mFillStyle) ||
         (selectedPixHatchStyle != mHatchStyle))
      {
         selectedPixLine = isLineDisplayed();
         selectedPixColor = getLineColor();
         selectedPixFillColor = mFillColor;
         selectedPixFillStyle = mFillStyle;
         selectedPixHatchStyle = mHatchStyle;
         selectedPix->fill(Qt::transparent);

         QPainter p(selectedPix);

         if (isLineDisplayed() == true)
         {
            p.setPen(QPen(getLineColor(), 1));
         }
         else
         {
            p.setPen(Qt::NoPen);
         }

         switch (getFillStyle())
         {
            default:
            case SOLID_FILL:
               p.setBrush(QBrush(getFillColor(), Qt::SolidPattern));
               break;

            case HATCH:
            {
               switch (getHatchStyle())
               {
                  default:
                  case SOLID:
                     p.setBrush(QBrush(getFillColor(), Qt::SolidPattern));
                     break;

                  case X:
                  case BOXED_X:
                     p.setBrush(QBrush(getFillColor(), Qt::DiagCrossPattern));
                     break;

                  case CROSS_HAIR:
                  case BOXED_CROSS_HAIR:
                     p.setBrush(QBrush(getFillColor(), Qt::CrossPattern));
                     break;

                  case ASTERISK:
                  case BOXED_ASTERISK:
                     p.setBrush(QBrush(getFillColor(), Qt::Dense5Pattern));
                     break;

                  case HORIZONTAL_LINE:
                  case BOXED_HORIZONTAL_LINE:
                     p.setBrush(QBrush(getFillColor(), Qt::HorPattern));
                     break;

                  case VERTICAL_LINE:
                  case BOXED_VERTICAL_LINE:
                     p.setBrush(QBrush(getFillColor(), Qt::VerPattern));
                     break;

                  case FORWARD_SLASH:
                  case BOXED_FORWARD_SLASH:
                     p.setBrush(QBrush(getFillColor(), Qt::BDiagPattern));
                     break;

                  case BACK_SLASH:
                  case BOXED_BACK_SLASH:
                     p.setBrush(QBrush(getFillColor(), Qt::FDiagPattern));
                     break;

                  case BOX:
                     p.setBrush(Qt::NoBrush);
                     break;
               }

               break;
            }

            case EMPTY_FILL:
               p.setBrush(Qt::NoBrush);
               break;
         }

         p.drawPolygon(points);

         if (bSelected == true)
         {
            QBrush pointBrush(Qt::black);
            p.fillRect(5, 12, 3, 3, pointBrush);
            p.fillRect(17, 12, 3, 3, pointBrush);
            p.fillRect(11, 9, 3, 3, pointBrush);
            p.fillRect(16, 4, 3, 3, pointBrush);
            p.fillRect(9, 1, 3, 3, pointBrush);
            p.fillRect(5, 7, 3, 3, pointBrush);
         }

         p.end();
      }

      return *selectedPix;
   }
   else if ((bSelected == false) && (pix->isNull() == false))
   {
      if ((pixLine != isLineDisplayed()) || (pixColor != getLineColor()) || (pixFillColor != mFillColor) ||
         (pixFillStyle != mFillStyle) || (pixHatchStyle != mHatchStyle))
      {
         pixLine = isLineDisplayed();
         pixColor = getLineColor();
         pixFillColor = mFillColor;
         pixFillStyle = mFillStyle;
         pixHatchStyle = mHatchStyle;
         pix->fill(Qt::transparent);

         QPainter p(pix);

         if (isLineDisplayed() == true)
         {
            p.setPen(QPen(getLineColor(), 1));
         }
         else
         {
            p.setPen(Qt::NoPen);
         }

         switch (getFillStyle())
         {
            default:
            case SOLID_FILL:
               p.setBrush(QBrush(getFillColor(), Qt::SolidPattern));
               break;

            case HATCH:
            {
               switch (getHatchStyle())
               {
                  default:
                  case SOLID:
                     p.setBrush(QBrush(getFillColor(), Qt::SolidPattern));
                     break;

                  case X:
                  case BOXED_X:
                     p.setBrush(QBrush(getFillColor(), Qt::DiagCrossPattern));
                     break;

                  case CROSS_HAIR:
                  case BOXED_CROSS_HAIR:
                     p.setBrush(QBrush(getFillColor(), Qt::CrossPattern));
                     break;

                  case ASTERISK:
                  case BOXED_ASTERISK:
                     p.setBrush(QBrush(getFillColor(), Qt::Dense5Pattern));
                     break;

                  case HORIZONTAL_LINE:
                  case BOXED_HORIZONTAL_LINE:
                     p.setBrush(QBrush(getFillColor(), Qt::HorPattern));
                     break;

                  case VERTICAL_LINE:
                  case BOXED_VERTICAL_LINE:
                     p.setBrush(QBrush(getFillColor(), Qt::VerPattern));
                     break;

                  case FORWARD_SLASH:
                  case BOXED_FORWARD_SLASH:
                     p.setBrush(QBrush(getFillColor(), Qt::BDiagPattern));
                     break;

                  case BACK_SLASH:
                  case BOXED_BACK_SLASH:
                     p.setBrush(QBrush(getFillColor(), Qt::FDiagPattern));
                     break;

                  case BOX:
                     p.setBrush(Qt::NoBrush);
                     break;
               }

               break;
            }

            case EMPTY_FILL:
               p.setBrush(Qt::NoBrush);
               break;
         }

         p.drawPolygon(points);
         p.end();
      }

      return *pix;
   }

   return PointSetImp::getLegendPixmap(bSelected);
}

void PolygonPlotObjectImp::setFillColor(const QColor& fillColor)
{
   if (fillColor.isValid() == false)
   {
      return;
   }

   if (fillColor != mFillColor)
   {
      mFillColor = fillColor;
      emit legendPixmapChanged();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void PolygonPlotObjectImp::setFillStyle(const FillStyle& fillStyle)
{
   if (fillStyle != mFillStyle)
   {
      mFillStyle = fillStyle;
      emit legendPixmapChanged();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void PolygonPlotObjectImp::setHatchStyle(const SymbolType& hatchStyle)
{
   if (hatchStyle != mHatchStyle)
   {
      mHatchStyle = hatchStyle;
      emit legendPixmapChanged();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

bool PolygonPlotObjectImp::toXml(XMLWriter* pXml) const
{
   if (!PointSetImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("fillStyle", mFillStyle);
   pXml->addAttr("hatchStyle", mHatchStyle);
   pXml->addAttr("fillColor", QCOLOR_TO_COLORTYPE(mFillColor));

   return true;
}

bool PolygonPlotObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !PlotObjectImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   mFillStyle = StringUtilities::fromXmlString<FillStyle>(A(pElem->getAttribute(X("fillStyle"))));
   mHatchStyle = StringUtilities::fromXmlString<SymbolType>(A(pElem->getAttribute(X("hatchStyle"))));
   ColorType color = StringUtilities::fromXmlString<ColorType>(A(pElem->getAttribute(X("fillColor"))));
   mFillColor = COLORTYPE_TO_QCOLOR(color);

   return true;
}
