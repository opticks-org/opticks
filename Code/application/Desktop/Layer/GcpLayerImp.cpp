/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>

#include "glCommon.h"
#include "GcpLayerImp.h"
#include "DrawUtil.h"
#include "GcpLayerAdapter.h"
#include "GcpLayerUndo.h"
#include "GcpList.h"
#include "GcpListUndo.h"
#include "Icons.h"
#include "PerspectiveView.h"
#include "PropertiesGcpLayer.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "Undo.h"
#include "ViewImp.h"
#include "xmlreader.h"

#include <list>
using namespace std;
XERCES_CPP_NAMESPACE_USE

GcpLayerImp::GcpLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   LayerImp(id, layerName, pElement)
{
   ColorType color = GcpLayer::getSettingMarkerColor();
   mColor = COLORTYPE_TO_QCOLOR(color);
   mSymbol = GcpLayer::getSettingMarkerSymbol();
   mSymbolSize = GcpLayer::getSettingMarkerSize();

   // Initialization
   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      setIcon(pIcons->mGCPMarker);
   }

   addPropertiesPage(PropertiesGcpLayer::getName());

   // Connections
   connect(this, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(symbolChanged(const GcpSymbol&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(sizeChanged(int)), this, SIGNAL(modified()));
}

GcpLayerImp::~GcpLayerImp()
{
}

const string& GcpLayerImp::getObjectType() const
{
   static string type("GcpLayerImp");
   return type;
}

bool GcpLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "GcpLayer"))
   {
      return true;
   }

   return LayerImp::isKindOf(className);
}

GcpLayerImp& GcpLayerImp::operator= (const GcpLayerImp& gcpLayer)
{
   if (this != &gcpLayer)
   {
      LayerImp::operator =(gcpLayer);

      mColor = gcpLayer.mColor;
      mSymbol = gcpLayer.mSymbol;
      mSymbolSize = gcpLayer.mSymbolSize;
   }

   return *this;
}

LayerType GcpLayerImp::getLayerType() const
{
   return GCP_LAYER;
}

vector<ColorType> GcpLayerImp::getColors() const
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

void GcpLayerImp::draw()
{
   GcpList* pGcpList = dynamic_cast<GcpList*>(getDataElement());
   if (pGcpList == NULL)
   {
      return;
   }

   const list<GcpPoint>& points = pGcpList->getSelectedPoints();

   glColor3ub(mColor.red(), mColor.green(), mColor.blue());

   unsigned int gcpNumber = 1;
   list<GcpPoint>::const_iterator iter;
   for (iter = points.begin(); iter != points.end(); iter++, gcpNumber++)
   {
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLineWidth(2.0);

      ViewImp *pView = dynamic_cast<ViewImp*>(getView());
      PerspectiveView *pPerView = dynamic_cast<PerspectiveView*>(pView);
      VERIFYNRV(pView != NULL);
      double rotation = 0.0;
      double xScale = getXScaleFactor();
      double yScale = getYScaleFactor();
      if (pPerView != NULL)
      {
         rotation = pPerView->getRotation();
         double aspect = pPerView->getPixelAspect();
         if (aspect > 1)
         {
            xScale *= aspect;
         }
         else
         {
            yScale /= aspect;
         }
      }

      // Translation
      GcpPoint point = *iter;
      glTranslatef(point.mPixel.mX + 0.5, point.mPixel.mY + 0.5, 0);

      // Calculate the circle radius
      LocationType center;
      pView->translateWorldToScreen(0.0, 0.0, center.mX, center.mY);

      LocationType circlePoint;
      pView->translateScreenToWorld(center.mX + mSymbolSize, center.mY, circlePoint.mX, circlePoint.mY);

      double dRadius = sqrt(pow(circlePoint.mX, 2) + pow(circlePoint.mY, 2));

      if (mSymbol != GCP_NODRAW)
      {
         // Draw the circle
         GLUquadricObj* pQuadric = gluNewQuadric();
         gluQuadricDrawStyle(pQuadric, GLU_SILHOUETTE);
         gluDisk(pQuadric, dRadius, dRadius, 256, 1);
         gluDeleteQuadric(pQuadric);

         glRotated(rotation, 0.0, 0.0, 1.0);

         glBegin(GL_LINES);
         if (mSymbol == GCP_X)
         {
            glVertex2f(-dRadius * 0.707107, dRadius * 0.707107);
            glVertex2f(dRadius * 0.707107, -dRadius * 0.707107);
            glVertex2f(dRadius * 0.707107, dRadius * 0.707107);
            glVertex2f(-dRadius * 0.707107, -dRadius * 0.707107);
         }
         else if (mSymbol == GCP_PLUS)
         {
            glVertex2f(-dRadius, 0.0);
            glVertex2f(dRadius, 0.0);
            glVertex2f(0.0, -dRadius);
            glVertex2f(0.0, dRadius);
         }

         glEnd();

         // Draw the GCP name
         if (pView != NULL)
         {
            QFont gcpFont = QApplication::font();
            gcpFont.setBold(true);
            gcpFont.setPointSize(12);

            QString strText;
            strText.sprintf("GCP %d", gcpNumber);

            QFontMetrics fontMetrics(gcpFont);
            QRect textRect = fontMetrics.tightBoundingRect(strText);

            LocationType textCoord;
            pView->translateWorldToScreen(point.mPixel.mX + 0.5, point.mPixel.mY + 0.5, textCoord.mX, textCoord.mY);

            int screenX = static_cast<int>(textCoord.mX + mSymbolSize);
            int screenY = pView->height() - static_cast<int>(textCoord.mY - mSymbolSize - textRect.height());
            pView->renderText(screenX, screenY, strText, gcpFont);
         }

         glLineWidth(1.0);
      }

      glPopMatrix();
   }
}

bool GcpLayerImp::getExtents(double& x1, double& y1, double& x4, double& y4)
{
   return false;
}

QColor GcpLayerImp::getColor() const
{
   return mColor;
}

GcpSymbol GcpLayerImp::getSymbol() const
{
   return mSymbol;
}

int GcpLayerImp::getSymbolSize() const
{
   return mSymbolSize;
}

bool GcpLayerImp::acceptsMouseEvents() const
{
   return true;
}

QCursor GcpLayerImp::getMouseCursor() const
{
   QCursor mouseCursor;

   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      mouseCursor = QCursor(pIcons->mGcpCursor, pIcons->mGcpMask, 0, 17);
   }

   return mouseCursor;
}

bool GcpLayerImp::processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
                                    Qt::KeyboardModifiers modifiers)
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView == NULL)
   {
      return false;
   }

   LocationType pixelCoord;
   pView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), pixelCoord.mX, pixelCoord.mY);

   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;
   pView->getExtents(dMinX, dMinY, dMaxX, dMaxY);

   if ((pixelCoord.mX >= dMinX) && (pixelCoord.mX <= dMaxX) && (pixelCoord.mY >= dMinY) && (pixelCoord.mY <= dMaxY))
   {
      GcpList* pGcpList = static_cast<GcpList*>(getDataElement());
      if (pGcpList != NULL)
      {
         GcpPoint gcpPoint;
         gcpPoint.mPixel = pixelCoord - 0.5;

         RasterElement* pRaster = dynamic_cast<RasterElement*>(pGcpList->getParent());
         if ((pRaster != NULL) && (pRaster->isGeoreferenced() == true))
         {
            gcpPoint.mCoordinate = pRaster->convertPixelToGeocoord(pixelCoord);
         }

         pView->addUndoAction(new AddGcpPoint(pGcpList, gcpPoint));
         pGcpList->addPoint(gcpPoint);
         return true;
      }
   }

   return false;
}

bool GcpLayerImp::toXml(XMLWriter* pXml) const
{
   if(!LayerImp::toXml(pXml))
   {
      return false;
   }

   // Color
   string colorName = mColor.name().toStdString();
   pXml->addAttr("symbolColor", colorName);

   // Symbol
   string symbolStr = StringUtilities::toXmlString(mSymbol);
   pXml->addAttr("symbol", symbolStr);

   // Symbol size
   stringstream buf;
   buf << mSymbolSize;
   pXml->addAttr("symbolSize", buf.str());

   return true;
}

bool GcpLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if(pDocument->getNodeType() != DOMNode::ELEMENT_NODE)
   {
      return false;
   }

   list<GcpPoint> oldPoints;

   GcpList* pGcpList = dynamic_cast<GcpList*>(getDataElement());
   if (pGcpList != NULL)
   {
      oldPoints = pGcpList->getSelectedPoints();
   }

   DOMElement *pElement = static_cast<DOMElement*>(pDocument);
   if((pElement == NULL) || !LayerImp::fromXml(pDocument, version))
   {
      return false;
   }

   // Color
   string color = A(pElement->getAttribute(X("symbolColor")));
   setColor(QColor(QString::fromStdString(color)));

   // Symbol
   setSymbol(StringUtilities::fromXmlString<GcpSymbol>(A(pElement->getAttribute(X("symbol")))));

   // Symbol size
   XmlReader::StringStreamAssigner<int> intConvertor;
   setSymbolSize(intConvertor(A(pElement->getAttribute(X("symbolSize")))));

   View* pView = getView();
   if ((pView != NULL) && (pGcpList != NULL))
   {
      list<GcpPoint> newPoints = pGcpList->getSelectedPoints();
      pView->addUndoAction(new SetGcpPoints(pGcpList, oldPoints, newPoints));
   }

   return true;
}

void GcpLayerImp::setColor(const QColor& clrGcp)
{
   if (clrGcp == mColor)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetGcpColor(dynamic_cast<GcpLayer*>(this), QCOLOR_TO_COLORTYPE(mColor),
            QCOLOR_TO_COLORTYPE(clrGcp)));
      }

      mColor = clrGcp;
      emit colorChanged(mColor);
      notify(SIGNAL_NAME(GcpLayer, ColorChanged), boost::any(ColorType(mColor.red(),mColor.green(),mColor.blue())));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         Layer* pLayer = NULL;
         pLayer = *iter;
         if (pLayer != NULL)
         {
            if (pLayer->isKindOf("GcpLayer") == true)
            {
               ((GcpLayerAdapter*) pLayer)->GcpLayerImp::setColor(clrGcp);
            }
         }

         ++iter;
      }

      mbLinking = false;
   }
}

void GcpLayerImp::setSymbol(const GcpSymbol& gcpSymbol)
{
   if (gcpSymbol == mSymbol)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetGcpSymbol(dynamic_cast<GcpLayer*>(this), mSymbol, gcpSymbol));
      }

      mSymbol = gcpSymbol;
      emit symbolChanged(mSymbol);
      notify(SIGNAL_NAME(GcpLayer, SymbolChanged), boost::any(mSymbol));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         Layer* pLayer = NULL;
         pLayer = *iter;
         if (pLayer != NULL)
         {
            if (pLayer->isKindOf("GcpLayer") == true)
            {
               ((GcpLayer*) pLayer)->setSymbol(gcpSymbol);
            }
         }

         ++iter;
      }

      mbLinking = false;
   }
}

void GcpLayerImp::setSymbolSize(int iSize)
{
   if (iSize == mSymbolSize)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetGcpSymbolSize(dynamic_cast<GcpLayer*>(this), mSymbolSize, iSize));
      }

      mSymbolSize = iSize;
      emit sizeChanged(mSymbolSize);
      notify(SIGNAL_NAME(GcpLayer, SizeChanged), boost::any(mSymbolSize));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         Layer* pLayer = NULL;
         pLayer = *iter;
         if (pLayer != NULL)
         {
            if (pLayer->isKindOf("GcpLayer") == true)
            {
               ((GcpLayer*) pLayer)->setSymbolSize(iSize);
            }
         }

         ++iter;
      }

      mbLinking = false;
   }
}

void GcpLayerImp::reset()
{
   ColorType color = GcpLayer::getSettingMarkerColor();
   setColor(COLORTYPE_TO_QCOLOR(color));
   setSymbol(GcpLayer::getSettingMarkerSymbol());
   setSymbolSize(GcpLayer::getSettingMarkerSize());
}
