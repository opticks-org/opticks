/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QVBoxLayout>

#include "ZoomPanWidget.h"

#include "AppVerify.h"
#include "DesktopServicesImp.h"
#include "Layer.h"
#include "LayerList.h"
#include "RasterElement.h"
#include "SpatialDataViewImp.h"

using namespace std;

ZoomPanWidget::ZoomPanWidget(SpatialDataViewImp* pView, QWidget* parent) :
   QWidget(parent),
   mpView(pView), mpLayer(NULL)
{
   // Initialize the view
   if (mpView != NULL)
   {
      mpView->setParent(this);
      mpView->setFocusProxy(this);
      mpView->installEventFilter(this);

      const LayerList* pLayerList = mpView->getLayerList();
      if (pLayerList != NULL)
      {
         mpLayer = pLayerList->getLayer(RASTER, pLayerList->getPrimaryRasterElement());
      }
   }

   // Widget layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(mpView);

   // Initialization
   setFocusPolicy(Qt::StrongFocus);
   setMinimumSize(200, 200);
   setMaximumSize(1000, 1000);
}

ZoomPanWidget::~ZoomPanWidget()
{
}

vector<LocationType> ZoomPanWidget::getSelection() const
{
   vector<LocationType> selectionAreaData;
   translateWorldToData(mpView->getSelectionBox(), selectionAreaData);

   return selectionAreaData;
}

void ZoomPanWidget::zoomExtents()
{
   if (mpView != NULL)
   {
      mpView->zoomExtents();
      mpView->repaint();
   }
}

QSize ZoomPanWidget::sizeHint() const
{
   QSize widgetSize;
   if (mpView != NULL)
   {
      widgetSize = mpView->sizeHint();
   }

   return widgetSize;
}

void ZoomPanWidget::setSelection(const vector<LocationType>& selectionData)
{
   vector<LocationType> selectionWorld;
   translateDataToWorld(selectionData, selectionWorld);
   
   const vector<LocationType>& currentSelection = mpView->getSelectionBox();
   if (currentSelection != selectionWorld)
   {
      mpView->setSelectionBox(selectionWorld);
      emit selectionChanged(selectionData);
      mpView->repaint();
   }
}

bool ZoomPanWidget::eventFilter(QObject* o, QEvent* e)
{
   QMouseEvent* m(NULL);
   QKeyEvent* k(NULL);
   switch (e->type())
   {
      case QEvent::Enter:
         enterEvent(e);
         break;

      case QEvent::MouseButtonPress:
         m = static_cast<QMouseEvent*>(e);
         mousePressEvent(m);
         break;

      case QEvent::MouseMove:
         m = static_cast<QMouseEvent*>(e);
         mouseMoveEvent(m);
         break;

      case QEvent::MouseButtonRelease:
         m = static_cast<QMouseEvent*>(e);
         mouseReleaseEvent(m);
         break;

      case QEvent::MouseButtonDblClick:
         m = static_cast<QMouseEvent*>(e);
         mouseDoubleClickEvent(m);
         break;

      case QEvent::Leave:
         leaveEvent(e);
         break;

      case QEvent::KeyPress:
         k = static_cast<QKeyEvent*>(e);
         keyPressEvent(k);
         break;

      case QEvent::KeyRelease:
         k = static_cast<QKeyEvent*>(e);
         keyReleaseEvent(k);
         break;

      default:
         return QWidget::eventFilter(o, e);
   }

   return true;
}

void ZoomPanWidget::enterEvent(QEvent* e)
{
   QWidget::enterEvent(e);
   updateMouseCursor();
}

void ZoomPanWidget::mousePressEvent(QMouseEvent* e)
{
   if ((e != NULL) && (mpView != NULL))
   {
      mMouseStart = e->pos();
      mMouseStart.setY(height() - e->pos().y());

      if (e->button() == Qt::LeftButton && e->modifiers() == Qt::ShiftModifier)
      {
         bool bHit = false;
         bHit = selectionHit(mMouseStart);
         if (bHit == false)
         {
            const vector<LocationType>& selection = mpView->getSelectionBox();

            LocationType currentCenter;
            if (selection.size() == 4)
            {
               currentCenter.mX = (selection[0].mX + selection[1].mX + selection[2].mX + selection[3].mX) / 4.0;
               currentCenter.mY = (selection[0].mY + selection[1].mY + selection[2].mY + selection[3].mY) / 4.0;
            }

            LocationType selectionCenter;
            mpView->translateScreenToWorld(mMouseStart.x(), mMouseStart.y(), selectionCenter.mX, selectionCenter.mY);

            LocationType delta = selectionCenter - currentCenter;
            vector<LocationType> newSelection;

            for (unsigned int i = 0; i < selection.size(); i++)
            {
               LocationType selectionPoint = selection.at(i);
               selectionPoint += delta;

               newSelection.push_back(selectionPoint);
            }

            mpView->setSelectionBox(newSelection);

            DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
            if (pDesktop != NULL)
            {
               PanModeType panMode = pDesktop->getPanMode();
               if (panMode == PAN_INSTANT)
               {
                  vector<LocationType> newSelectionData;
                  translateWorldToData(newSelection, newSelectionData);
                  emit selectionChanged(newSelectionData);
               }
            }
         }
      }

      mpView->repaint();
   }

   QWidget::mousePressEvent(e);
   updateMouseCursor();
}

void ZoomPanWidget::mouseMoveEvent(QMouseEvent* e)
{
   if ((e != NULL) && (mpView != NULL))
   {
      QPoint ptMouse = e->pos();
      ptMouse.setY(height() - e->pos().y());

      if (e->buttons() == Qt::LeftButton && e->modifiers() == Qt::NoModifier)
      {
         mpView->setSelectionBox(mMouseStart, ptMouse);
      }
      else if (e->buttons() == Qt::LeftButton && e->modifiers() == Qt::ShiftModifier)
      {
         LocationType point;
         LocationType currentPoint;
         mpView->translateScreenToWorld(ptMouse.x(), ptMouse.y(), point.mX, point.mY);
         mpView->translateScreenToWorld(mMouseCurrent.x(), mMouseCurrent.y(), currentPoint.mX, currentPoint.mY);

         LocationType delta = point - currentPoint;
         vector<LocationType> newSelection;

         const vector<LocationType>& selection = mpView->getSelectionBox();
         for (unsigned int i = 0; i < selection.size(); i++)
         {
            LocationType selectionPoint = selection.at(i);
            selectionPoint += delta;

            newSelection.push_back(selectionPoint);
         }

         mpView->setSelectionBox(newSelection);

         DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
         if (pDesktop != NULL)
         {
            PanModeType panMode = pDesktop->getPanMode();
            if (panMode == PAN_INSTANT)
            {
               vector<LocationType> newSelectionData;
               translateWorldToData(newSelection, newSelectionData);
               emit selectionChanged(newSelectionData);
            }
         }
      }

      mMouseCurrent = ptMouse;
      mpView->repaint();

      LocationType dataCoord;
      mpLayer->translateScreenToData(mMouseCurrent.x(), mMouseCurrent.y(), dataCoord.mX, dataCoord.mY);
      emit locationChanged(dataCoord);
   }

   QWidget::mouseMoveEvent(e);
}

void ZoomPanWidget::mouseReleaseEvent(QMouseEvent* e)
{
   if (mpView != NULL)
   {
      if (e != NULL)
      {
         if (e->button() == Qt::LeftButton && e->modifiers() == Qt::NoModifier)
         {
            const vector<LocationType>& selectionWorld = mpView->getSelectionBox();

            vector<LocationType>::const_iterator iter;
            for (iter = selectionWorld.begin(); iter != selectionWorld.end(); ++iter)
            {
               LocationType pixelCoord;
               mpView->translateScreenToWorld(mMouseStart.x(), mMouseStart.y(), pixelCoord.mX, pixelCoord.mY);

               if (*iter != pixelCoord)
               {
                  vector<LocationType> newSelectionData;
                  translateWorldToData(selectionWorld, newSelectionData);
                  emit selectionChanged(newSelectionData);
                  break;
               }
            }
         }
         else if (e->button() == Qt::LeftButton && e->modifiers() == Qt::ShiftModifier)
         {
            DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
            if (pDesktop != NULL)
            {
               PanModeType panMode = pDesktop->getPanMode();
               if (panMode == PAN_DELAY)
               {
                  vector<LocationType> newSelectionData;
                  translateWorldToData(mpView->getSelectionBox(), newSelectionData);
                  emit selectionChanged(newSelectionData);
               }
            }
         }
      }

      mpView->repaint();
   }

   QWidget::mouseReleaseEvent(e);
   updateMouseCursor();
}

void ZoomPanWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
   if (e != NULL)
   {
      if (e->button() == Qt::LeftButton)
      {
         zoomExtents();
      }
   }

   QWidget::mouseDoubleClickEvent(e);
}

void ZoomPanWidget::leaveEvent(QEvent* e)
{
   emit locationChanged(LocationType(-1.0, -1.0));
   QWidget::leaveEvent(e);
}

void ZoomPanWidget::keyPressEvent(QKeyEvent* e)
{
   if ((e != NULL) && (mpView != NULL))
   {
      double dDeltaX = 0.0;
      double dDeltaY = 0.0;

      switch (e->key())
      {
         case Qt::Key_Up:
            dDeltaY = 2.0;
            break;

         case Qt::Key_Down:
            dDeltaY = -2.0;
            break;

         case Qt::Key_Left:
            dDeltaX = -2.0;
            break;

         case Qt::Key_Right:
            dDeltaX = 2.0;
            break;

         default:
            break;
      }

      if ((dDeltaX != 0.0) || (dDeltaY != 0.0))
      {
         const vector<LocationType>& selection = mpView->getSelectionBox();

         LocationType currentCenter;
         if (selection.size() == 4)
         {
            currentCenter.mX = (selection[0].mX + selection[1].mX + selection[2].mX + selection[3].mX) / 4.0;
            currentCenter.mY = (selection[0].mY + selection[1].mY + selection[2].mY + selection[3].mY) / 4.0;
         }

         LocationType screenCenter;
         mpView->translateWorldToScreen(currentCenter.mX, currentCenter.mY, screenCenter.mX, screenCenter.mY);

         screenCenter.mX += dDeltaX;
         screenCenter.mY += dDeltaY;

         LocationType selectionCenter;
         mpView->translateScreenToWorld(screenCenter.mX, screenCenter.mY, selectionCenter.mX, selectionCenter.mY);

         LocationType delta = selectionCenter - currentCenter;
         vector<LocationType> newSelection;

         for (unsigned int i = 0; i < selection.size(); i++)
         {
            LocationType selectionPoint = selection.at(i);
            selectionPoint += delta;

            newSelection.push_back(selectionPoint);
         }

         mpView->setSelectionBox(newSelection);

         vector<LocationType> newSelectionData;
         translateWorldToData(newSelection, newSelectionData);
         emit selectionChanged(newSelectionData);

         mpView->repaint();
      }
   }

   QWidget::keyPressEvent(e);
   updateMouseCursor();
}

void ZoomPanWidget::keyReleaseEvent(QKeyEvent* pEvent)
{
   QWidget::keyReleaseEvent(pEvent);
   updateMouseCursor();
}

bool ZoomPanWidget::selectionHit(const QPoint& screenCoord) const
{
   if (mpView == NULL)
   {
      return false;
   }

   const vector<LocationType>& selection = mpView->getSelectionBox();
   if (selection.size() == 0)
   {
      return false;
   }

   LocationType coord;
   mpView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), coord.mX, coord.mY);

   int iAbove = 0;
   for (unsigned int i = 0; i < selection.size() - 1; i++)
   {
      bool bBetween = false;
      if (selection[i + 1].mX < selection[i].mX)
      {
         if ((selection[i + 1].mX < coord.mX) && (coord.mX <= selection[i].mX))
         {
            bBetween = true;
         }
      }
      else
      {
         if ((selection[i].mX < coord.mX) && (coord.mX <= selection[i + 1].mX))
         {
            bBetween = true;
         }
      }

      if (bBetween == true)
      {
         double dSlope = (selection[i + 1].mY - selection[i].mY) / (selection[i + 1].mX - selection[i].mX);
         if (((dSlope * (coord.mX - selection[i].mX)) + selection[i].mY) >= coord.mY)
         {
            iAbove++;
         }
      }
   }

   return (iAbove & 1);
}

void ZoomPanWidget::updateMouseCursor()
{
   if (mpView == NULL)
   {
      return;
   }

   if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
   {
      if (QApplication::mouseButtons() == Qt::LeftButton)
      {
         mpView->setCursor(QCursor(Qt::ClosedHandCursor));
      }
      else
      {
         mpView->setCursor(QCursor(Qt::OpenHandCursor));
      }
   }
   else
   {
      mpView->setCursor(QCursor(QPixmap(":/icons/ZoomRectCursor"), 0, 0));
   }
}

void ZoomPanWidget::translateDataToWorld(const std::vector<LocationType>& dataValue, 
   std::vector<LocationType>& worldValue) const
{
   VERIFYNRV(mpView != NULL && mpLayer != NULL);

   worldValue.resize(dataValue.size());

   vector<LocationType>::iterator iterWorld = worldValue.begin();
   for (vector<LocationType>::const_iterator iterData = dataValue.begin();
      iterData != dataValue.end(); ++iterData, ++iterWorld)
   {
      mpLayer->translateDataToWorld(iterData->mX, iterData->mY, iterWorld->mX, iterWorld->mY);
   }

}

void ZoomPanWidget::translateWorldToData(const std::vector<LocationType>& worldValue,
   std::vector<LocationType>& dataValue) const
{
   VERIFYNRV(mpView != NULL && mpLayer != NULL);

   dataValue.resize(worldValue.size());

   vector<LocationType>::iterator iterData = dataValue.begin();
   for (vector<LocationType>::const_iterator iterWorld = worldValue.begin();
      iterWorld != worldValue.end(); ++iterWorld, ++iterData)
   {
      mpLayer->translateWorldToData(iterWorld->mX, iterWorld->mY, iterData->mX, iterData->mY);
   }
}
