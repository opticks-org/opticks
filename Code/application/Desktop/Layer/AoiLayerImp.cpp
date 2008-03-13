/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include "glCommon.h"
#include "AoiLayerImp.h"
#include "AnnotationToolBar.h"
#include "AoiElement.h"
#include "AoiElementImp.h"
#include "AoiToolBar.h"
#include "AoiLayerAdapter.h"
#include "AoiLayerUndo.h"
#include "BitMaskImp.h"
#include "BitMaskObjectImp.h"
#include "AppVerify.h"
#include "DesktopServicesImp.h"
#include "DrawUtil.h"
#include "GraphicGroup.h"
#include "GraphicObject.h"
#include "GraphicObjectFactory.h"
#include "Icons.h"
#include "LayerList.h"
#include "PixelObjectImp.h"
#include "PolygonObject.h"
#include "PropertiesAoiLayer.h"
#include "SessionManager.h"
#include "SpatialDataView.h"
#include "SpatialDataViewImp.h"
#include "StringUtilities.h"
#include "SymbolRegionDrawer.h"
#include "TypesFile.h"
#include "Undo.h"
#include "UtilityServicesImp.h"
#include "View.h"
#include "xmlreader.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
XERCES_CPP_NAMESPACE_USE

unsigned int AoiLayerImp::msNumLayers = 0;

AoiLayerImp::AoiLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   GraphicLayerImp(id, layerName, pElement)
{
   addAcceptableGraphicType(LINE_OBJECT);
   addAcceptableGraphicType(HLINE_OBJECT);
   addAcceptableGraphicType(VLINE_OBJECT);
   addAcceptableGraphicType(POLYLINE_OBJECT);
   addAcceptableGraphicType(POLYGON_OBJECT);
   addAcceptableGraphicType(MULTIPOINT_OBJECT);
   addAcceptableGraphicType(RECTANGLE_OBJECT);
   addAcceptableGraphicType(ELLIPSE_OBJECT);
   addAcceptableGraphicType(GROUP_OBJECT);
   addAcceptableGraphicType(BITMASK_OBJECT);
   addAcceptableGraphicType(ROW_OBJECT);
   addAcceptableGraphicType(COLUMN_OBJECT);

   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      setIcon(pIcons->mDrawPixel);
   }

   mHandleSize = 4.0;
   mHandleDrawn = false;
   mLabelMoving = false;

   bool autoColorOn = AoiLayer::getSettingAutoColor();
   if (autoColorOn == true)
   {
      UtilityServicesImp *Utils = UtilityServicesImp::instance();
      ColorType color = Utils->getAutoColor(msNumLayers);
      mColor = COLORTYPE_TO_QCOLOR(color);
   }
   else
   {
      ColorType color = AoiLayer::getSettingMarkerColor();      
      mColor = COLORTYPE_TO_QCOLOR(color);
   }

   mSymbol = AoiLayer::getSettingMarkerSymbol();

   connect(this, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(symbolChanged(const SymbolType&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(objectAdded(GraphicObject *)), this, SLOT(objectWasAdded(GraphicObject *)));

   AoiElement *pAoi = dynamic_cast<AoiElement*>(pElement);
   if (pAoi != NULL)
   {
      GraphicGroup *pGroup = pAoi->getGroup();
      if (pGroup != NULL)
      {
         ColorType color(mColor.red(), mColor.green(), mColor.blue());
         pGroup->setLineColor(color);
         pGroup->setFillColor(color);
         pGroup->setLineWidth(1);
         pGroup->setLineStyle(SOLID_LINE);
         pGroup->setLineState(true);
         pGroup->setPixelSymbol(mSymbol);
         pGroup->setFillState(true);
         pGroup->setFillStyle(SOLID_FILL);
      }
   }

   msNumLayers++;

   addPropertiesPage(PropertiesAoiLayer::getName());
}

AoiLayerImp::~AoiLayerImp()
{
}

const string& AoiLayerImp::getObjectType() const
{
   static string type("AoiLayerImp");
   return type;
}

bool AoiLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AoiLayer"))
   {
      return true;
   }

   return LayerImp::isKindOf(className);
}

AoiLayerImp& AoiLayerImp::operator= (const AoiLayerImp& aoiLayer)
{
   if (this != &aoiLayer)
   {
      GraphicLayerImp::operator =(aoiLayer);

      mColor = aoiLayer.mColor;
      mSymbol = aoiLayer.mSymbol;
   }

   return *this;
}

LayerType AoiLayerImp::getLayerType() const
{
   return AOI_LAYER;
}

vector<ColorType> AoiLayerImp::getColors() const
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

QColor AoiLayerImp::getColor() const
{
   return mColor;
}

SymbolType AoiLayerImp::getSymbol() const
{
   return mSymbol;
}

ModeType AoiLayerImp::getMode() const
{
   return mCurrentMode;
}

bool AoiLayerImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   if (!GraphicLayerImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("aoiSymbol", mSymbol);
   pXml->addAttr("aoiColor", QCOLOR_TO_COLORTYPE(mColor));

   if(Service<SessionManager>()->isSessionSaving())
   {
      pXml->addAttr("aoiLabelOffset", mLabelOffset);
      pXml->addAttr("aoiLabelLocation", mLabelLocation);
   }

   return true;
}

bool AoiLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (pDocument->getNodeType() != DOMNode::ELEMENT_NODE)
   {
      return false;
   }

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);
   if ((pElement == NULL) || (GraphicLayerImp::fromXml(pDocument, version) == false))
   {
      return false;
   }

   // Color
   ColorType color = StringUtilities::fromXmlString<ColorType>(
      A(pElement->getAttribute(X("aoiColor"))));
   mColor = COLORTYPE_TO_QCOLOR(color);

   // Symbol
   string symbol = A(pElement->getAttribute(X("aoiSymbol")));
   mSymbol = StringUtilities::fromXmlString<SymbolType>(symbol);

   if(Service<SessionManager>()->isSessionLoading())
   {
      mLabelOffset = StringUtilities::fromXmlString<LocationType>(
         A(pElement->getAttribute(X("aoiLabelOffset"))));
      mLabelLocation = StringUtilities::fromXmlString<LocationType>(
         A(pElement->getAttribute(X("aoiLabelLocation"))));
   }

   return true;
}

void AoiLayerImp::setColor(const QColor& clrMarker)
{
   if (clrMarker.isValid() == false)
   {
      return;
   }

   if (mColor != clrMarker)
   {
      ColorType color(clrMarker.red(), clrMarker.green(), clrMarker.blue(), clrMarker.alpha());

      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetAoiColor(dynamic_cast<AoiLayer*>(this), QCOLOR_TO_COLORTYPE(mColor), color));
      }

      mColor = clrMarker;

      GraphicGroup* pGroup = getGroup();
      if (pGroup != NULL)
      {
         UndoLock lock(pView);
         pGroup->setFillColor(color);
         pGroup->setLineColor(color);
      }

      emit colorChanged(clrMarker);
      notify(SIGNAL_NAME(AoiLayer, ColorChanged), boost::any(color));
   }
}

void AoiLayerImp::setSymbol(const SymbolType& eSymbol)
{
   if (mSymbol != eSymbol)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetAoiSymbol(dynamic_cast<AoiLayer*>(this), mSymbol, eSymbol));
      }

      mSymbol = eSymbol;

      GraphicGroup* pGroup = getGroup();
      if (pGroup != NULL)
      {
         UndoLock lock(pView);
         pGroup->setPixelSymbol(mSymbol);
      }

      emit symbolChanged(mSymbol);
      notify(SIGNAL_NAME(AoiLayer, SymbolChanged), boost::any(mSymbol));
   }
}

void AoiLayerImp::reset()
{
   ColorType color = AoiLayer::getSettingMarkerColor();
   QColor clrAoi = COLORTYPE_TO_QCOLOR(color);
   bool autoColorOn = AoiLayer::getSettingAutoColor();
   if (autoColorOn == true)
   {
      UtilityServicesImp* pUtilities = UtilityServicesImp::instance();
      if (pUtilities != NULL)
      {
         ColorType color = pUtilities->getAutoColor(msNumLayers);
         clrAoi.setRgb(color.mRed, color.mGreen, color.mBlue);
      }
   }

   setColor(clrAoi);
   setSymbol(AoiLayer::getSettingMarkerSymbol());
}

LocationType AoiLayerImp::correctCoordinate(const LocationType &coord) const
{
   LocationType loc;
   loc.mX = static_cast<int>(coord.mX) + 0.5;
   loc.mY = static_cast<int>(coord.mY) + 0.5;
   return loc;
}

GraphicObject *AoiLayerImp::addObject(const GraphicObjectType &objectType, LocationType point)
{
   Service<DesktopServices> pDesktop;
   AoiToolBar* pToolbar = static_cast<AoiToolBar*>(pDesktop->getWindow("AOI", TOOLBAR));
   if((pToolbar != NULL) && (pToolbar->getAddMode() == REPLACE_AOI))
   {
      // erase the current AOI
      clear();
   }
   GraphicObject *pObject = GraphicLayerImp::addObject(objectType, point);
   if (pObject != NULL)
   {
      ColorType color(mColor.red(), mColor.green(), mColor.blue());
      pObject->setFillColor(color);
      pObject->setLineColor(color);
      pObject->setPixelSymbol(mSymbol);
      pObject->setDrawMode(mCurrentMode);
   }
   return pObject;
}

void AoiLayerImp::setMode(ModeType mode)
{
   if (mode != mCurrentMode)
   {
      mCurrentMode = mode;
      emit modeChanged(mode);
   }
}

void AoiLayerImp::objectWasAdded(GraphicObject *pObject)
{
   if (pObject != NULL)
   {

      ColorType color(mColor.red(), mColor.green(), mColor.blue());
      pObject->setFillColor(color);
      pObject->setLineColor(color);
      pObject->setPixelSymbol(mSymbol);
      ModeType mode = mCurrentMode;
      AoiElementImp *pAoiElement = dynamic_cast<AoiElementImp*>(getGroup());
      if (pAoiElement != NULL)
      {
         mode = pAoiElement->correctedDrawMode(mode);
      }
      pObject->setDrawMode(mode);
      if (pObject->isVisible())
      {
         deselectAllObjects();
      }
   }
}

void AoiLayerImp::drawGroup()
{
   if (mustDrawAsBitmask())
   {
      PerspectiveView *pView = dynamic_cast<PerspectiveView*>(getView());
      double zoomPercent = 100;
      if (pView != NULL)
      {
         zoomPercent = pView->getZoomPercentage();
      }

      AoiElement *pAoi = static_cast<AoiElement*>(getDataElement());
      VERIFYNRV(pAoi != NULL);
      GraphicResource<BitMaskObjectImp> pMaskObj(BITMASK_OBJECT, dynamic_cast<AoiLayer*>(this));
      pMaskObj->setFillColor(ColorType(mColor.red(), mColor.green(), mColor.blue()));
      pMaskObj->setPixelSymbol(mSymbol);
      pMaskObj->setBitMask(pAoi->getSelectedPoints(), false);
      pMaskObj->draw(zoomPercent / 100);

      if (getShowLabels())
      {
         GraphicGroup *pGroup = getGroup();
         VERIFYNRV(pGroup != NULL);
         const std::list<GraphicObject*> &objects = pGroup->getObjects();
         for (std::list<GraphicObject*>::const_iterator iter = objects.begin();
            iter != objects.end(); ++iter)
         {
            GraphicObjectImp *pObj = dynamic_cast<GraphicObjectImp*>(*iter);
            if (pObj != NULL)
            {
               pObj->drawLabel();
            }
         }
      }
   }
   else
   {
      GraphicLayerImp::drawGroup();
   }
}

void AoiLayerImp::draw()
{
   GraphicLayerImp::draw();

   SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*> (getView());
   if (pView == NULL)
   {
      return;
   }

   DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
   if (pDesktop == NULL)
   {
      return;
   }

   AoiToolBar* pToolbar = static_cast<AoiToolBar*>(pDesktop->getWindow("AOI", TOOLBAR));
   if (pToolbar == NULL)
   {
      return;
   }

   // Draw the point labels
   QFont aoiFont = QApplication::font();
   aoiFont.setBold(true);
   aoiFont.setPointSize(12);

   // Draw the layer name label
   mLabelLocation = getGroup()->getLlCorner() - mLabelOffset;

   if (!getGroup()->getObjects().empty() && pToolbar->getAoiShowLabels())
   {
      QColor labelColor = getLabelColor(NULL);
      pView->qglColor(labelColor);

      LocationType screenCoord;
      translateDataToScreen(mLabelLocation.mX, mLabelLocation.mY, screenCoord.mX, screenCoord.mY);
      screenCoord = screenCoord + mHandleSize + 2.0;

      int screenX = static_cast<int>(screenCoord.mX);
      int screenY = pView->height() - static_cast<int>(screenCoord.mY);
      QString strText = QString::fromStdString(getName());
      pView->renderText(screenX, screenY, strText, aoiFont);

      // Draw text handle if appropriate
      Layer* pCurrentLayer = pView->getTopMostLayer(AOI_LAYER);
      if (pCurrentLayer == (AoiLayerAdapter*) this)
      {
         mHandleDrawn = true;
         glColor3f(255.0,255.0,255.0);
         glLineWidth(1);

         double zoomFactor = pView->getZoomPercentage();
         zoomFactor /= 100.0;
         zoomFactor = 1.0 / zoomFactor;

         float x1,x2,y1,y2;
         x1 = mLabelLocation.mX - mHandleSize * zoomFactor / getXScaleFactor();
         x2 = mLabelLocation.mX + mHandleSize * zoomFactor / getXScaleFactor();
         y1 = mLabelLocation.mY - mHandleSize * zoomFactor / getYScaleFactor();
         y2 = mLabelLocation.mY + mHandleSize * zoomFactor / getYScaleFactor();
         glBegin(GL_POLYGON);
         glVertex2f(x1,y1);
         glVertex2f(x2,y1);
         glVertex2f(x2,y2);
         glVertex2f(x1,y2);
         glEnd();
         glColor3f(0.0,0.0,0.0);
         glBegin(GL_LINE_LOOP);
         glVertex2f(x1,y1);
         glVertex2f(x2,y1);
         glVertex2f(x2,y2);
         glVertex2f(x1,y2);
         glEnd();
      }
   }
}

QCursor AoiLayerImp::getMouseCursor() const
{
   QCursor mouseCursor;

   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      mouseCursor = QCursor(pIcons->mAoiCursor, pIcons->mAoiMask, 1, 18);
   }

   return mouseCursor;
}

bool AoiLayerImp::processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
{
   Service<DesktopServices> pDesktop;
   AoiToolBar* pToolbar = static_cast<AoiToolBar*>(pDesktop->getWindow("AOI", TOOLBAR));
   bool addNewLayer = (getCurrentGraphicObjectType() != MOVE_OBJECT) && (button == Qt::LeftButton) && 
                      (pToolbar != NULL) && (pToolbar->getAddMode() == NEW_AOI) &&
                      !getObjects().empty() && insertingObjectNull();
   if (getCurrentGraphicObjectType() == MOVE_OBJECT && button == Qt::LeftButton)
   {
      ViewImp* pView = dynamic_cast<ViewImp*> (getView());
      VERIFY(pView != NULL);

      GraphicGroup *pGroup = getGroup();
      VERIFY(pGroup != NULL);

      LocationType labelPos = pGroup->getLlCorner() - mLabelOffset;
      LocationType labelScreen;
      LocationType world;
      translateDataToWorld(labelPos.mX, labelPos.mY, world.mX, world.mY);
      pView->translateWorldToScreen(world.mX, world.mY, labelScreen.mX, labelScreen.mY);
      if (fabs(screenCoord.x() - labelScreen.mX) < mHandleSize && 
         fabs(screenCoord.y() - labelScreen.mY) < mHandleSize)
      {
         mLabelMoving = true;
         return true;
      }
   }
   else if(addNewLayer)
   {
      SpatialDataViewImp* pSpatialDataView = dynamic_cast<SpatialDataViewImp*> (getView());
      if (pSpatialDataView != NULL)
      {
         Layer* pNewLayer = pSpatialDataView->createLayer(AOI_LAYER);
         if (pNewLayer != NULL)
         {
            pSpatialDataView->setActiveLayer(pNewLayer);
            if (pNewLayer->getDataElement() == NULL)
            {
               pSpatialDataView->deleteLayer(pNewLayer);
            }
            else
            {
               AoiLayerImp *pNewLayerImp = dynamic_cast<AoiLayerImp*>(pNewLayer);
               if(pNewLayerImp != NULL)
               {
                  pNewLayerImp->processMousePress(screenCoord, button, buttons, modifiers);
                  return false;
               }
            }
         }
      }
   }

   return GraphicLayerImp::processMousePress(screenCoord, button, buttons, modifiers);
}

bool AoiLayerImp::processMouseMove(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
{
   if (buttons & Qt::LeftButton && mLabelMoving)
   {
      ViewImp* pView = dynamic_cast<ViewImp*> (getView());
      VERIFY(pView != NULL);

      GraphicGroup *pGroup = getGroup();
      VERIFY(pGroup != NULL);

      LocationType newLabelPos;
      pView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), newLabelPos.mX, newLabelPos.mY);
      translateWorldToData(newLabelPos.mX, newLabelPos.mY, newLabelPos.mX, newLabelPos.mY);
      mLabelOffset = pGroup->getLlCorner() - newLabelPos;
      return true;
   }

   return GraphicLayerImp::processMouseMove(screenCoord, button, buttons, modifiers);

}

bool AoiLayerImp::processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
{
   if (button == Qt::LeftButton && mLabelMoving)
   {
      processMouseMove(screenCoord, button, buttons, modifiers);
      mLabelMoving = false;
      return true;
   }

   return GraphicLayerImp::processMouseRelease(screenCoord, button, buttons, modifiers);
}

QColor AoiLayerImp::getLabelColor(const GraphicObjectImp *pObj)
{
   QColor labelColor(mColor);
   int h,s,v;
   labelColor.getHsv(&h,&s,&v);
   h = (h + 180) % 360;
   labelColor.setHsv(h,s,v);
   return labelColor;

}

bool AoiLayerImp::mustDrawAsBitmask() const
{
   AoiElementImp *pAoi = dynamic_cast<AoiElementImp*>(getDataElement());
   VERIFY(pAoi != NULL);
   if (pAoi->getAllPointsToggled())
   {
      return true;
   }
   const list<GraphicObject*> &objects = getGroup()->getObjects();
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); 
      iter != objects.end(); ++iter)
   {
      GraphicObject *pObj = *iter;
      if (pObj != NULL)
      {
         if (pObj->getDrawMode() != DRAW || pObj->getPixelSymbol() >= BOX)
         {
            return true;
         }
      }
   }
   return false;
}

bool AoiLayerImp::mayDrawAsPixels() const
{
   return true;
}

bool AoiLayerImp::willDrawAsPixels() const
{
   if (mayDrawAsPixels())
   {
      double maxScale = max(getXScaleFactor(), getYScaleFactor());
      PerspectiveView *pView = dynamic_cast<PerspectiveView*>(getView());
      VERIFY(pView != NULL);
      if (pView->getZoomPercentage() > 100 / maxScale)
      {
         return true;
      }
   }

   return false;

}
