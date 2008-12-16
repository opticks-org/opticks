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

#include "AppConfig.h"
#include "AppVerify.h"
#include "DrawUtil.h"
#include "glCommon.h"
#include "Icons.h"
#include "PerspectiveView.h"
#include "PropertiesTiePointLayer.h"
#include "TiePointLayer.h"
#include "TiePointLayerImp.h"
#include "TiePointLayerUndo.h"
#include "TiePointList.h"
#include "TiePointListUndo.h"
#include "UtilityServices.h"
#include "ViewImp.h"
#include "XercesIncludes.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <vector>

using namespace std;
XERCES_CPP_NAMESPACE_USE

unsigned int TiePointLayerImp::msNumLayers = 0;
LocationType TiePointLayerImp::sAnchor;

TiePointLayerImp::TiePointLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   LayerImp(id, layerName, pElement),
   mIsMission(false)
{
   bool autoColorOn = TiePointLayer::getSettingAutoColor();
   if (autoColorOn == true)
   {
      Service<UtilityServices> pUtils;
      ColorType color = pUtils->getAutoColor(msNumLayers);
      mColor = COLORTYPE_TO_QCOLOR(color);
   }
   else
   {
      ColorType color = TiePointLayer::getSettingMarkerColor();
      mColor = COLORTYPE_TO_QCOLOR(color);
   }

   mSymbolSize = TiePointLayer::getSettingMarkerSize();
   mLabelsEnabled = TiePointLayer::getSettingLabelEnabled();

   // Initialization
   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      setIcon(pIcons->mTiePointMarker);
   }

   addPropertiesPage(PropertiesTiePointLayer::getName());

   // Connections
   connect(this, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(labelEnabledChanged(bool)), this, SIGNAL(modified()));
   connect(this, SIGNAL(sizeChanged(int)), this, SIGNAL(modified()));
}

TiePointLayerImp::~TiePointLayerImp()
{
}

const string& TiePointLayerImp::getObjectType() const
{
   static string type("TiePointLayerImp");
   return type;
}

bool TiePointLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "TiePointLayer"))
   {
      return true;
   }

   return LayerImp::isKindOf(className);
}

bool TiePointLayerImp::toXml(XMLWriter* pXml) const
{
   if (!LayerImp::toXml(pXml))
   {
      return false;
   }
   pXml->addAttr("symbolSize", mSymbolSize);
   pXml->addAttr("symbolColor", QCOLOR_TO_COLORTYPE(mColor));
   pXml->addAttr("labelIsEnabled", mLabelsEnabled);
   pXml->addAttr("isMission", mIsMission);

   return true;
}

bool TiePointLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (!LayerImp::fromXml(pDocument, version))
   {
      return false;
   }

   vector<TiePoint> oldPoints;

   TiePointList* pTiePointList = dynamic_cast<TiePointList*>(getDataElement());
   if (pTiePointList != NULL)
   {
      oldPoints = pTiePointList->getTiePoints();
   }

   DOMElement* pElmnt = static_cast<DOMElement*>(pDocument);

   setSymbolSize(StringUtilities::fromXmlString<int>(A(pElmnt->getAttribute(X("symbolSize")))));
   setColor(COLORTYPE_TO_QCOLOR(StringUtilities::fromXmlString<ColorType>(A(pElmnt->getAttribute(X("symbolColor"))))));
   enableLabels(StringUtilities::fromXmlString<bool>(A(pElmnt->getAttribute(X("labelIsEnabled")))));
   setIsMission(StringUtilities::fromXmlString<bool>(A(pElmnt->getAttribute(X("isMission")))));

   View* pView = getView();
   if ((pView != NULL) && (pTiePointList != NULL))
   {
      vector<TiePoint> newPoints = pTiePointList->getTiePoints();
      pView->addUndoAction(new SetTiePoints(pTiePointList, oldPoints, newPoints));
   }

   return true;
}

LayerType TiePointLayerImp::getLayerType() const
{
   return TIEPOINT_LAYER;
}

vector<ColorType> TiePointLayerImp::getColors() const
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

void TiePointLayerImp::draw()
{
   const int labelTolerance = 5000; // only draw labels if <labelTolerance to be drawn
   TiePointList* pList = static_cast<TiePointList*>(getDataElement());
   VERIFYNRV(pList != NULL);

   const vector<TiePoint>& points = pList->getTiePoints();

   glColor3ub(mColor.red(), mColor.green(), mColor.blue());

   // compute the symbol size in scene coordinates
   double sceneSymbolSize = static_cast<double>(mSymbolSize) / DrawUtil::getPixelSize(0.0, 0.0, 1.0, 1.0);

   int viewableBounds[4] = {INT_MIN, INT_MIN, INT_MAX, INT_MAX};
   DrawUtil::restrictToViewport(viewableBounds[0], viewableBounds[1], viewableBounds[2], viewableBounds[3]);

   int visibleCount = drawSymbols(points, sceneSymbolSize, viewableBounds);
   if (mLabelsEnabled && visibleCount < labelTolerance)
   {
      drawLabels(points, sceneSymbolSize, viewableBounds);
   }
}

namespace
{
bool isInBounds(const LocationType &point, const int viewableBounds[4])
{
   if (point.mX < viewableBounds[0] || point.mX > viewableBounds[2])
   {
      return false;
   }

   if (point.mY < viewableBounds[1] || point.mY > viewableBounds[3])
   {
      return false;
   }

   return true;
}
}

int TiePointLayerImp::drawSymbols(const vector<TiePoint> &points, double sceneSymbolSize,
                                  const int viewableBounds[4]) const
{
   int viewableCount = 0;
   vector<TiePoint>::const_iterator pPoint;
   if (mSymbolSize <= 1)
   {
      glBegin(GL_POINTS);
      for (pPoint = points.begin(); pPoint != points.end(); ++pPoint)
      {
         LocationType point = getPoint(*pPoint) + LocationType(0.5, 0.5);
         if (isInBounds(point, viewableBounds))
         {
            viewableCount++;
            glVertex2d(point.mX, point.mY);
         }
      }
      glEnd();
   }
   else
   {
      glLineWidth(1.0);
      for (pPoint = points.begin(); pPoint != points.end(); ++pPoint)
      {
         LocationType point = getPoint(*pPoint) + LocationType(0.5, 0.5);
         if (isInBounds(point, viewableBounds))
         {
            viewableCount++;
            drawSymbol(point, sceneSymbolSize);
         }
      }
   }

   return viewableCount;
}

void TiePointLayerImp::drawLabels(const vector<TiePoint>& points, double sceneSymbolSize,
                                  const int viewableBounds[4]) const
{
   double dRotation = 0.0;

   ViewImp* pView = dynamic_cast<ViewImp*>(getView());
   VERIFYNRV(pView != NULL);

   PerspectiveView* pPerspectiveView = dynamic_cast<PerspectiveView*>(pView);
   if (pPerspectiveView != NULL)
   {
      dRotation = pPerspectiveView->getRotation() * PI / 180.0;
   }

   double textScale = sceneSymbolSize / mSymbolSize;

   LocationType offset;
   sceneSymbolSize *= 1.2; // push the label off the symbol a little
   offset.mX = cos(dRotation) * sceneSymbolSize + 0.5;
   offset.mY = sin(dRotation) * sceneSymbolSize + 0.5;

   QFont font = QApplication::font();
   font.setBold(false);
   font.setPointSize(12);

   vector<TiePoint>::const_iterator pPoint;
   for (pPoint = points.begin(); pPoint != points.end(); ++pPoint)
   {
      LocationType point = getPoint(*pPoint);
      if (isInBounds(point, viewableBounds))
      {
         point += offset;

         QString strText;
         strText.sprintf("%d", pPoint-points.begin()+1);

         LocationType screenCoord;
         translateDataToScreen(point.mX, point.mY, screenCoord.mX, screenCoord.mY);

         int screenX = static_cast<int>(screenCoord.mX);
         int screenY = pView->height() - static_cast<int>(screenCoord.mY);
         pView->renderText(screenX, screenY, strText, font);
      }
   }
}

void TiePointLayerImp::drawSymbol(const LocationType& point, double symbolSize) const
{
   double minX = point.mX - symbolSize;
   double minY = point.mY - symbolSize;
   double maxX = point.mX + symbolSize;
   double maxY = point.mY + symbolSize;

   // Draw box
   glBegin(GL_LINE_LOOP);
   glVertex2d(minX, minY);
   glVertex2d(minX, maxY);
   glVertex2d(maxX, maxY);
   glVertex2d(maxX, minY);
   glEnd();

   // Draw X
   glBegin(GL_LINES);
   glVertex2d(minX, minY);
   glVertex2d(maxX, maxY);
   glVertex2d(minX, maxY);
   glVertex2d(maxX, minY);
   glEnd();
}

bool TiePointLayerImp::getExtents(double& x1, double& y1, double& x4, double& y4)
{
   TiePointList* pList = static_cast<TiePointList*>(getDataElement());
   VERIFY(pList != NULL);

   const vector<TiePoint>& points = pList->getTiePoints();
   if (!points.empty())
   {
      int xMinData = numeric_limits<int>::max();
      int yMinData = numeric_limits<int>::max();
      int xMaxData = numeric_limits<int>::min();
      int yMaxData = numeric_limits<int>::min();
      
      for (vector<TiePoint>::const_iterator pPoint = points.begin();
         pPoint != points.end(); ++pPoint)
      {
         xMinData = min(xMinData, pPoint->mReferencePoint.mX);
         yMinData = min(yMinData, pPoint->mReferencePoint.mY);
         xMaxData = max(xMaxData, pPoint->mReferencePoint.mX);
         yMaxData = max(yMaxData, pPoint->mReferencePoint.mY);
      }
      translateDataToWorld(xMinData, yMinData, x1, y1);
      translateDataToWorld(xMaxData, yMaxData, x4, y4);
   }

   return true;
}

TiePointLayerImp& TiePointLayerImp::operator= (const TiePointLayerImp& tiePointLayer)
{
   if (this != &tiePointLayer)
   {
      LayerImp::operator =(tiePointLayer);

      mColor = tiePointLayer.mColor;
      mSymbolSize = tiePointLayer.mSymbolSize;
      mLabelsEnabled = tiePointLayer.mLabelsEnabled;
      mIsMission = tiePointLayer.mIsMission;
   }

   return *this;
}

void TiePointLayerImp::setColor(const QColor& colorType)
{
   if (colorType == mColor)
   {
      return;
   }

   if (mbLinking == false)
   {
      ColorType color(colorType.red(), colorType.green(), colorType.blue(), colorType.alpha());

      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetTiePointColor(dynamic_cast<TiePointLayer*>(this),
            QCOLOR_TO_COLORTYPE(mColor), color));
      }

      mColor = colorType;
      emit colorChanged(mColor);
      notify(SIGNAL_NAME(TiePointLayer, ColorChanged), boost::any(color));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         TiePointLayer* pLayer = dynamic_cast<TiePointLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setColor(color);
         }
      }

      mbLinking = false;
   }
}

QColor TiePointLayerImp::getColor() const
{
   return mColor;
}

void TiePointLayerImp::setSymbolSize(int iSize)
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
         pView->addUndoAction(new SetTiePointSymbolSize(dynamic_cast<TiePointLayer*>(this), mSymbolSize, iSize));
      }

      mSymbolSize = iSize;
      emit sizeChanged(mSymbolSize);
      notify(SIGNAL_NAME(TiePointLayer, SizeChanged), boost::any(mSymbolSize));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         TiePointLayer* pLayer = dynamic_cast<TiePointLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setSymbolSize(iSize);
         }
      }

      mbLinking = false;
   }
}

int TiePointLayerImp::getSymbolSize() const
{
   return mSymbolSize;
}

void TiePointLayerImp::enableLabels(bool enabled)
{
   if (enabled == mLabelsEnabled)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetTiePointLabels(dynamic_cast<TiePointLayer*>(this), mLabelsEnabled, enabled));
      }

      mLabelsEnabled = enabled;
      emit labelEnabledChanged(mLabelsEnabled);
      notify(SIGNAL_NAME(TiePointLayer, LabelsEnabled), boost::any(enabled));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         TiePointLayer* pLayer = dynamic_cast<TiePointLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->enableLabels(enabled);
         }
      }

      mbLinking = false;
   }
}

bool TiePointLayerImp::areLabelsEnabled() const
{
   return mLabelsEnabled;
}

bool TiePointLayerImp::isMission() const
{
   return mIsMission;
}

void TiePointLayerImp::setIsMission(bool isMission)
{
   mIsMission = isMission;
}

bool TiePointLayerImp::acceptsMouseEvents() const
{
   return true;
}

QCursor TiePointLayerImp::getMouseCursor() const
{
   QCursor mouseCursor;

   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      mouseCursor = QCursor(pIcons->mTiePointCursor, pIcons->mTiePointMask, 1, 15);
   }

   return mouseCursor;
}

bool TiePointLayerImp::processMousePress(const QPoint& screenCoord, Qt::MouseButton button,
                                         Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if (mIsMission == false)
   {
      if (button == Qt::LeftButton)
      {
         View* pView = getView();
         VERIFY(pView != NULL);

         pView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), sAnchor.mX, sAnchor.mY);
         translateWorldToData(sAnchor.mX, sAnchor.mY, sAnchor.mX, sAnchor.mY);
         return true;
      }
   }
   return false;
}

bool TiePointLayerImp::processMouseMove(const QPoint& screenCoord, Qt::MouseButton button,
                                        Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if (mIsMission == false)
   {
      if (buttons == Qt::LeftButton)
      {
         LocationType point;

         ViewImp* pView = dynamic_cast<ViewImp*>(getView());
         VERIFY(pView != NULL);

         pView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), point.mX, point.mY);
         translateWorldToData(point.mX, point.mY, point.mX, point.mY);
         pView->setSelectionBox(sAnchor, point);
         return true;
      }
   }

   return false;
}

bool TiePointLayerImp::processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button,
                                           Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if (mIsMission == false)
   {
      if (button == Qt::LeftButton)
      {
         LocationType point;

         ViewImp* pView = dynamic_cast<ViewImp*>(getView());
         VERIFY(pView != NULL);

         pView->setSelectionBox(QRect());
         pView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), point.mX, point.mY);
         translateWorldToData(point.mX, point.mY, point.mX, point.mY);

         TiePointList* pElement = dynamic_cast<TiePointList*>(getDataElement());

         int bounds[4];
         bounds[0] = sAnchor.mX;
         bounds[1] = sAnchor.mY;
         bounds[2] = point.mX;
         bounds[3] = point.mY;

         if (bounds[0] > bounds[2])
         {
            int temp = bounds[0];
            bounds[0] = bounds[2];
            bounds[2] = temp;
         }
         if (bounds[1] > bounds[3])
         {
            int temp = bounds[1];
            bounds[1] = bounds[3];
            bounds[3] = temp;
         }

         vector<TiePoint> remainingPoints;
         const vector<TiePoint>& oldPoints = pElement->getTiePoints();
         vector<TiePoint>::const_iterator pPoint;
         for (pPoint = oldPoints.begin(); pPoint != oldPoints.end(); ++pPoint)
         {
            LocationType point = getPoint(*pPoint);
            if (!isInBounds(point, bounds))
            {
               remainingPoints.push_back(*pPoint);
            }
         }

         if (remainingPoints != oldPoints)
         {
            pView->addUndoAction(new SetTiePoints(pElement, oldPoints, remainingPoints));
            pElement->adoptTiePoints(remainingPoints);
         }

         return true;
      }
   }

   return false;
}

void TiePointLayerImp::reset()
{
   ColorType color = TiePointLayer::getSettingMarkerColor();
   QColor clrTiePoint = COLORTYPE_TO_QCOLOR(color);

   bool autoColorOn = TiePointLayer::getSettingAutoColor();
   if (autoColorOn == true)
   {
      Service<UtilityServices> pUtilities;
      ColorType autoColor = pUtilities->getAutoColor(msNumLayers);
      clrTiePoint = COLORTYPE_TO_QCOLOR(autoColor);
   }

   setColor(clrTiePoint);
   setSymbolSize(TiePointLayer::getSettingMarkerSize());
   enableLabels(TiePointLayer::getSettingLabelEnabled());
}
