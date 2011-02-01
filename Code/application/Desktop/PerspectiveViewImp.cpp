/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QCursor>
#include <QtGui/QKeyEvent>

#include "AppConfig.h"
#include "ApplicationWindow.h"
#include "ConfigurationSettings.h"
#include "DataVariant.h"
#include "DesktopServices.h"
#include "GeocoordLinkFunctor.h"
#include "GlContextSave.h"
#include "MouseModeImp.h"
#include "PerspectiveView.h"
#include "PerspectiveViewImp.h"
#include "SessionItemSerializer.h"
#include "StatusBar.h"
#include "Undo.h"
#include "ViewUndo.h"
#include "xmlreader.h"


#include <math.h>
#include <limits>
#include <boost/bind.hpp>

#include <sstream>
using namespace std;
XERCES_CPP_NAMESPACE_USE

unsigned int PerspectiveViewImp::sKeyboardNumber = 0;

PerspectiveViewImp::PerspectiveViewImp(const string& id, const string& viewName, QGLContext* drawContext,
                                       QWidget* parent) :
   ViewImp(id, viewName, drawContext, parent),
   mDist(1.0),
   mFullDistance(1.0),
   mCenter(0.0, 0.0),
   mHeading(0.0),
   mPitch(90.0),
   mFov(45.0),
   mFrontPlane(1.0),
   mBackPlane(160000.0),
   mPixelAspect(1.0),
   mCurrentModifiers(Qt::NoModifier),
   mDisplayContextMenu(true),
   mAllowZoomOnResize(false)
{
   // Keyboard shortcut actions
   Service<DesktopServices> pDesktop;
   string shortcutContext = "View";

   QAction* pInsetFactorAction = new QAction("Zoom Inset Factor", this);
   pInsetFactorAction->setAutoRepeat(false);
   pInsetFactorAction->setShortcut(QKeySequence(Qt::Key_F));
   pInsetFactorAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pInsetFactorAction, shortcutContext + "/Zoom");
   addAction(pInsetFactorAction);

   QAction* pZoomInsetAction = new QAction("Zoom to Inset", this);
   pZoomInsetAction->setAutoRepeat(false);
   pZoomInsetAction->setShortcut(QKeySequence(Qt::Key_C));
   pZoomInsetAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pZoomInsetAction, shortcutContext + "/Zoom");
   addAction(pZoomInsetAction);

   QAction* pToggleCoordinatesAction = new QAction("Toggle Pixel Coordinates", this);
   pToggleCoordinatesAction->setAutoRepeat(false);
   pToggleCoordinatesAction->setShortcut(QKeySequence(Qt::Key_N));
   pToggleCoordinatesAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pToggleCoordinatesAction, shortcutContext);
   addAction(pToggleCoordinatesAction);

   // Initialization
   if (getDataOrigin() == UPPER_LEFT)
   {
      mPitch = -90.0;
   }

   // Connections
   VERIFYNR(connect(this, SIGNAL(originChanged(const DataOrigin&)), this, SLOT(flipVertical())));
   VERIFYNR(connect(pInsetFactorAction, SIGNAL(triggered()), this, SLOT(zoomInsetFactor())));
   VERIFYNR(connect(pZoomInsetAction, SIGNAL(triggered()), this, SLOT(zoomToEnabledInset())));
   VERIFYNR(connect(pToggleCoordinatesAction, SIGNAL(triggered()), this, SLOT(togglePixelCoordinates())));
}

PerspectiveViewImp::~PerspectiveViewImp()
{
}

const string& PerspectiveViewImp::getObjectType() const
{
   static string type("PerspectiveViewImp");
   return type;
}

bool PerspectiveViewImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PerspectiveView"))
   {
      return true;
   }

   return ViewImp::isKindOf(className);
}

bool PerspectiveViewImp::isKindOfView(const string& className)
{
   if ((className == "PerspectiveViewImp") || (className == "PerspectiveView"))
   {
      return true;
   }

   return ViewImp::isKindOfView(className);
}

void PerspectiveViewImp::getViewTypes(vector<string>& classList)
{
   classList.push_back("PerspectiveView");
   ViewImp::getViewTypes(classList);
}

PerspectiveViewImp& PerspectiveViewImp::operator= (const PerspectiveViewImp& perspectiveView)
{
   if (this != &perspectiveView)
   {
      ViewImp::operator= (perspectiveView);

      mDist = perspectiveView.mDist;
      mFullDistance = perspectiveView.mFullDistance;
      mCenter = perspectiveView.mCenter;
      mHeading = perspectiveView.mHeading;
      mPitch = perspectiveView.mPitch;
      mFov = perspectiveView.mFov;
      mFrontPlane = perspectiveView.mFrontPlane;
      mBackPlane = perspectiveView.mBackPlane;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

double PerspectiveViewImp::getZoomPercentage() const
{
   return (mFullDistance / mDist * 100.0);
}

double PerspectiveViewImp::getRotation() const
{
   return mHeading;
}

double PerspectiveViewImp::getPitch() const
{
   return mPitch;
}

void PerspectiveViewImp::zoomExtents()
{
   // Get the extents
   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;
   getExtents(dMinX, dMinY, dMaxX, dMaxY);

   // Calculate the minimum and maximum screen coordinates at the world extents to account for rotation
   double pixelX = 0.0;
   double pixelY = 0.0;
   double minPixelX = 0.0;
   double minPixelY = 0.0;
   double maxPixelX = 0.0;
   double maxPixelY = 0.0;

   translateWorldToScreen(dMinX, dMinY, pixelX, pixelY);
   minPixelX = maxPixelX = pixelX;
   minPixelY = maxPixelY = pixelY;

   translateWorldToScreen(dMaxX, dMinY, pixelX, pixelY);
   minPixelX = min(minPixelX, pixelX);
   maxPixelX = max(maxPixelX, pixelX);
   minPixelY = min(minPixelY, pixelY);
   maxPixelY = max(maxPixelY, pixelY);

   translateWorldToScreen(dMaxX, dMaxY, pixelX, pixelY);
   minPixelX = min(minPixelX, pixelX);
   maxPixelX = max(maxPixelX, pixelX);
   minPixelY = min(minPixelY, pixelY);
   maxPixelY = max(maxPixelY, pixelY);

   translateWorldToScreen(dMinX, dMaxY, pixelX, pixelY);
   minPixelX = min(minPixelX, pixelX);
   maxPixelX = max(maxPixelX, pixelX);
   minPixelY = min(minPixelY, pixelY);
   maxPixelY = max(maxPixelY, pixelY);

   // Convert the screen coordinates back to world coordinates
   translateScreenToWorld(minPixelX, minPixelY, dMinX, dMinY);
   translateScreenToWorld(maxPixelX, maxPixelY, dMaxX, dMaxY);

   // Update the distance
   zoomToBox(LocationType(dMinX, dMinY), LocationType(dMaxX, dMaxY));
}

void PerspectiveViewImp::zoomBy(double dPercent)
{
   double dZoom = getZoomPercentage();
   if (dPercent > 0)
   {
      zoomTo(dZoom * (1.0 + dPercent / 100.0));
   }
   else
   {
      zoomTo(dZoom / (1.0 - dPercent / 100.0));
   }
}

void PerspectiveViewImp::zoomTo(double dPercent)
{
   double dCurrentPercent = getZoomPercentage();
   if (dPercent == dCurrentPercent)
   {
      return;
   }

   if (mbLinking == false)
   {
      addUndoAction(new ZoomViewPercent(dynamic_cast<PerspectiveView*>(this), dCurrentPercent, dPercent));

      // apply zoom limits
      dPercent = limitZoomPercentage(dPercent);
      // Update the distance
      mDist = mFullDistance / (dPercent / 100.0);

      // Update the matrices
      updateMatrices();

      // Notify connected and attached objects
      emit zoomChanged(dPercent);
      notify(SIGNAL_NAME(PerspectiveView, ZoomChanged), boost::any(dPercent));

      // Update the linked views
      executeOnLinks<PerspectiveViewImp>(boost::bind(&PerspectiveViewImp::zoomTo, _1, dPercent),
         GeocoordLinkFunctor(this));
   }
}

void PerspectiveViewImp::zoomAboutPoint(const QPoint& screenCoord, double dPercent)
{
   LocationType worldCoord;
   translateScreenToWorld(screenCoord.x(), screenCoord.y(), worldCoord.mX, worldCoord.mY);

   zoomAboutPoint(worldCoord, dPercent);
}

void PerspectiveViewImp::zoomAboutPoint(const LocationType& worldCoord, double dPercent)
{
   UndoGroup group(dynamic_cast<View*>(this), "Zoom About Point");

   // Get the screen coordinate
   double dScreenX = 0.0;
   double dScreenY = 0.0;
   translateWorldToScreen(worldCoord.mX, worldCoord.mY, dScreenX, dScreenY);

   // Zoom about the center point
   zoomToPoint(worldCoord, dPercent);

   // Pan back to the original location on the screen
   LocationType point;
   translateScreenToWorld(dScreenX, dScreenY, point.mX, point.mY);

   pan(point, worldCoord);
}

void PerspectiveViewImp::zoomToPoint(const QPoint& screenCoord, double dPercent)
{
   LocationType worldCoord;
   translateScreenToWorld(screenCoord.x(), screenCoord.y(), worldCoord.mX, worldCoord.mY);

   zoomToPoint(worldCoord, dPercent);
}

void PerspectiveViewImp::zoomToPoint(const LocationType& worldCoord, double dPercent)
{
   UndoGroup group(dynamic_cast<View*>(this), "Zoom To Point");

   // Pan to the new point
   LocationType startCoord = mCenter;     // Create a copy since pan() modifies mCenter which affects linked views
   pan(startCoord, worldCoord);

   // Zoom
   zoomTo(dPercent);
}

void PerspectiveViewImp::zoomToCenter(double dPercent)
{
   UndoGroup group(dynamic_cast<View*>(this), "Zoom To Center");

   // Pan to the center of the image
   panToCenter();

   // Zoom
   zoomTo(dPercent);
}

void PerspectiveViewImp::zoomToBox(const LocationType& worldLowerLeft, const LocationType& worldUpperRight)
{
   if (mbLinking == false)
   {
      LocationType llCorner;
      LocationType ulCorner;
      LocationType urCorner;
      LocationType lrCorner;
      getVisibleCorners(llCorner, ulCorner, urCorner, lrCorner);

      // Add the undo action
      addUndoAction(new ZoomViewBox(dynamic_cast<View*>(this), llCorner, worldLowerLeft, urCorner, worldUpperRight));

      // Update the center point
      mCenter = (worldLowerLeft + worldUpperRight) * 0.5;

      // constrain pan to keep center within pan options
      mCenter = limitPanCenter(mCenter);

      // Update the distance
      double dScreenBeginX = 0.0;
      double dScreenBeginY = 0.0;
      double dScreenEndX = 0.0;
      double dScreenEndY = 0.0;
      translateWorldToScreen(worldLowerLeft.mX, worldLowerLeft.mY, dScreenBeginX, dScreenBeginY);
      translateWorldToScreen(worldUpperRight.mX, worldUpperRight.mY, dScreenEndX, dScreenEndY);

      if (dScreenEndX < dScreenBeginX)
      {
         double dTemp = dScreenEndX;
         dScreenEndX = dScreenBeginX;
         dScreenBeginX = dTemp;
      }

      if (dScreenEndY < dScreenBeginY)
      {
         double dTemp = dScreenEndY;
         dScreenEndY = dScreenBeginY;
         dScreenBeginY = dTemp;
      }

      double xZoom = (dScreenEndX - dScreenBeginX) / static_cast<double>(width());
      double yZoom = (dScreenEndY - dScreenBeginY) / static_cast<double>(height());
      mDist *= max(xZoom, yZoom);

      if (mDist < mFrontPlane)
      {
         mDist = mFrontPlane;
      }

      // apply zoom limits
      double newPercent = limitZoomPercentage(mFullDistance / mDist * 100.0);

      // Update the distance
      mDist = mFullDistance / (newPercent / 100.0);

      // Update the matrices
      updateMatrices();

      // Notify connected and attached objects
      double dPercent = getZoomPercentage();

      emit zoomChanged(dPercent);
      notify(SIGNAL_NAME(PerspectiveView, ZoomChanged), boost::any(dPercent));

      // Update the linked views
      executeOnLinks<PerspectiveViewImp>(
         boost::bind(&PerspectiveViewImp::zoomToBox, _1, worldLowerLeft, worldUpperRight),
         GeocoordLinkFunctor(this));
   }
}

void PerspectiveViewImp::zoomToInset()
{
   double zoomMultiplier = static_cast<double>(View::getSettingInsetZoom());
   InsetZoomMode zoomMode = View::getSettingInsetZoomMode();

   if (zoomMode == RELATIVE_MODE)
   {
      zoomMultiplier *= static_cast<unsigned int>(getZoomPercentage() / 100.0);
   }

   LocationType insetPoint = getInsetLocation();
   zoomAboutPoint(insetPoint, zoomMultiplier * 100.0);
}

void PerspectiveViewImp::toggleShowCoordinates()
{
   View::setSettingInsetShowCoordinates(!View::getSettingInsetShowCoordinates());
}

void PerspectiveViewImp::pan(const LocationType& worldBegin, const LocationType& worldEnd)
{
   if (mbLinking == false)
   {
      // Update the center point
      LocationType oldCenter(mCenter);
      mCenter += (worldEnd - worldBegin);

      // constrain pan to keep center within extents of data
      mCenter = limitPanCenter(mCenter);

      // Add the undo action
      addUndoAction(new PanView(dynamic_cast<View*>(this), oldCenter, mCenter));

      // Update the matrices
      updateMatrices();

      // Notify connected and attached objects
      notify(SIGNAL_NAME(Subject, Modified));

      // Update the linked views
      executeOnLinks<PerspectiveViewImp>(boost::bind(&PerspectiveViewImp::pan, _1, worldBegin, worldEnd),
         GeocoordLinkFunctor(this));
   }
}

void PerspectiveViewImp::rotateBy(double dDegrees)
{
   // Adjust the angle based on the pitch
   if (mPitch < 0)
   {
      dDegrees = -1.0 * dDegrees;
   }

   // Update the heading
   rotateTo(mHeading + dDegrees);
}

void PerspectiveViewImp::rotateTo(double dDegrees)
{
   while (dDegrees >= 360.0)
   {
      dDegrees -= 360.0;
   }

   while (dDegrees < 0.0)
   {
      dDegrees += 360.0;
   }

   if (dDegrees == mHeading)
   {
      return;
   }

   if (mbLinking == false)
   {
      // Add the undo action
      addUndoAction(new RotateView(dynamic_cast<PerspectiveView*>(this), mHeading, dDegrees));

      // Update the heading
      mHeading = dDegrees;

      // Update the matrices
      updateMatrices();

      // Notify connected and attached objects
      emit rotationChanged(mHeading);
      notify(SIGNAL_NAME(PerspectiveView, RotationChanged), boost::any(mHeading));

      // Update the linked views
      executeOnLinks<PerspectiveViewImp>(boost::bind(&PerspectiveViewImp::rotateTo, _1, dDegrees),
         GeocoordLinkFunctor(this));
   }
}

void PerspectiveViewImp::flipBy(double dDegrees)
{
   flipTo(mPitch + dDegrees);
}

void PerspectiveViewImp::flipTo(double dDegrees)
{
   while (dDegrees > 180.0)
   {
      dDegrees -= 360.0;
   }

   while (dDegrees < -180.0)
   {
      dDegrees += 360.0;
   }

   if (dDegrees == mPitch)
   {
      return;
   }

   if (mbLinking == false)
   {
      addUndoAction(new FlipView(dynamic_cast<PerspectiveView*>(this), mPitch, dDegrees));

      // Update the pitch
      mPitch = dDegrees;

      // Update the matrices
      updateMatrices();

      // Notify connected and attached objects
      emit pitchChanged(mPitch);
      notify(SIGNAL_NAME(PerspectiveView, PitchChanged), boost::any(mPitch));

      // Update the linked views
      executeOnLinks<PerspectiveViewImp>(boost::bind(&PerspectiveViewImp::flipTo, _1, dDegrees),
         GeocoordLinkFunctor(this));
   }
}

void PerspectiveViewImp::flipHorizontal()
{
   UndoGroup group(dynamic_cast<View*>(this), "Flip Horizontal");

   flipBy(180.0);
   rotateBy(180.0);
}

void PerspectiveViewImp::flipVertical()
{
   flipBy(180.0);
}

void PerspectiveViewImp::resetZoom()
{
   unsigned int zoomPercent = PerspectiveView::getSettingZoomPercentage();
 
   if (zoomPercent == 0)
   {
      zoomExtents();
   }
   else
   {
      UndoGroup group(dynamic_cast<View*>(this), "Reset Zoom");

      // Zoom to the default percentage
      zoomTo(zoomPercent);

      // Align the data origin to the corner of the view
      QPoint screenCenter(width() / 2, height() / 2);
      LocationType worldCenter(screenCenter.x() * 100 / zoomPercent, screenCenter.y() * 100 / zoomPercent);
      panTo(worldCenter);
   }
}

void PerspectiveViewImp::resetOrientation()
{
   if (mbLinking == false)
   {
      double newPitch = 90.0;

      DataOrigin dataOrigin = getDataOrigin();
      if (dataOrigin == UPPER_LEFT)
      {
         newPitch = -90.0;
      }

      PerspectiveView* pView = dynamic_cast<PerspectiveView*>(this);
      UndoGroup group(pView, "Reset Orientation");

      addUndoAction(new RotateView(pView, mHeading, 0.0));
      addUndoAction(new FlipView(pView, mPitch, newPitch));

      mHeading = 0.0;
      mPitch = newPitch;
      mPixelAspect = 1;

      updateMatrices();
      notify(SIGNAL_NAME(Subject, Modified));

      // Update the linked views
      executeOnLinks<PerspectiveViewImp>(boost::bind(&PerspectiveViewImp::resetOrientation, _1),
         GeocoordLinkFunctor(this));
   }
}

void PerspectiveViewImp::reset()
{
   UndoGroup group(dynamic_cast<View*>(this), "Reset");

   resetOrientation();
   resetZoom();
}

void PerspectiveViewImp::updateStatusBar(const QPoint& screenCoord)
{
   Service<DesktopServices> pDesktop;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(pDesktop->getMainWidget());
   if (pAppWindow == NULL)
   {
      return;
   }

   StatusBar* pBar = static_cast<StatusBar*>(pAppWindow->statusBar());
   if (pBar == NULL)
   {
      return;
   }

   if (screenCoord.isNull() == false)
   {
      double dX = 0.0;
      double dY = 0.0;
      translateScreenToWorld(screenCoord.x(), screenCoord.y(), dX, dY);

      double dMinX = 0.0;
      double dMinY = 0.0;
      double dMaxX = 0.0;
      double dMaxY = 0.0;
      getExtents(dMinX, dMinY, dMaxX, dMaxY);

      if ((dX < dMinX) || (dX > dMaxX) || (dY < dMinY) || (dY > dMaxY))
      {
         pBar->clearPixelCoords();
         pBar->clearRotationValue();
      }
      else
      {
         // Set the paper coordinates
         QPoint ptMouse(dX, dY);
         pBar->setPixelCoords(ptMouse);

         // Set the rotation value
         double rotation = getRotation();
         pBar->setRotationValue(rotation);
      }
   }
   else
   {
      pBar->clearPixelCoords();
      pBar->clearRotationValue();
   }
}

void PerspectiveViewImp::setAllowZoomOnResize(bool allow)
{
   mAllowZoomOnResize = allow;
}

void PerspectiveViewImp::resizeEvent(QResizeEvent* e)
{
   // Do not call the base class method since it now does nothing

   double dZoom = getZoomPercentage();

   mFullDistance = (fabs(static_cast<double>(height())) / 2.0) / (tan(PI / 180.0 * (mFov / 2.0)));
   if (mFullDistance < mFrontPlane)
   {
      mFullDistance = mFrontPlane;
   }

   if (!mAllowZoomOnResize)
   {
      UndoLock lock(dynamic_cast<PerspectiveView*>(this));
      mDist = 0.0; // force zoomTo to work
      zoomTo(dZoom);
   }
   else
   {
      updateMatrices();
   }
}

void PerspectiveViewImp::keyPressEvent(QKeyEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   Qt::KeyboardModifiers modifiers = e->modifiers();
   if (modifiers & Qt::AltModifier)
   {
      switch (e->key())
      {
         case Qt::Key_Up:
            ViewImp::pan(QPoint(0, 0), QPoint(0, height()));
            updateGL();
            break;

         case Qt::Key_Down:
            ViewImp::pan(QPoint(0, 0), QPoint(0, -height()));
            updateGL();
            break;

         case Qt::Key_Left:
            ViewImp::pan(QPoint(0, 0), QPoint(-width(), 0));
            updateGL();
            break;

         case Qt::Key_Right:
            ViewImp::pan(QPoint(0, 0), QPoint(width(), 0));
            updateGL();
            break;

         default:
            break;
      }
   }
   else if (modifiers & Qt::ShiftModifier)
   {
      switch (e->key())
      {
         case Qt::Key_Left:
            rotateBy(-5.0);
            updateGL();
            break;

         case Qt::Key_Right:
            rotateBy(5.0);
            updateGL();
            break;

         default:
            return;
      }
   }
   else if ((modifiers & Qt::ControlModifier) == 0)
   {
      switch (e->key())
      {
         case Qt::Key_1:
         case Qt::Key_2:
         case Qt::Key_3:
         case Qt::Key_4:
         case Qt::Key_5:
         case Qt::Key_6:
         case Qt::Key_7:
         case Qt::Key_8:
         case Qt::Key_9:
         case Qt::Key_0:
            sKeyboardNumber = (e->key() - Qt::Key_0) + sKeyboardNumber * 10;
            break;

         case Qt::Key_Up:
            ViewImp::pan(QPoint(0, 0), QPoint(0, 10));
            updateGL();
            break;

         case Qt::Key_Down:
            ViewImp::pan(QPoint(0, 0), QPoint(0, -10));
            updateGL();
            break;

         case Qt::Key_Left:
            ViewImp::pan(QPoint(0, 0), QPoint(-10, 0));
            updateGL();
            break;

         case Qt::Key_Right:
            ViewImp::pan(QPoint(0, 0), QPoint(10, 0));
            updateGL();
            break;

         default:
            return;
      }
   }
}

void PerspectiveViewImp::mousePressEvent(QMouseEvent* e)
{
   bool bSuccess = false;
   if (e != NULL)
   {
      mMouseStart = e->pos();
      mMouseStart.setY(height() - e->pos().y());

      string mouseMode = "";

      const MouseMode* pMouseMode = getCurrentMouseMode();
      if (pMouseMode != NULL)
      {
         pMouseMode->getName(mouseMode);
      }

      mCurrentModifiers = e->modifiers();
      if (e->button() == Qt::LeftButton)
      {
         if (mouseMode == "PanMode")
         {
            Service<DesktopServices> pDesktop;

            PanModeType panMode = pDesktop->getPanMode();
            if (panMode == PAN_INSTANT)
            {
               startUndoGroup("Pan");
            }

            setCursor(Qt::ClosedHandCursor);
            bSuccess = true;
         }
         else if (mouseMode == "RotateMode")
         {
            startUndoGroup("Rotate");
            bSuccess = true;
         }
         else if (mouseMode == "ZoomInMode")
         {
            double dZoom = getZoomPercentage();
            dZoom *= 1.25;

            zoomAboutPoint(mMouseStart, dZoom);
            bSuccess = true;
         }
         else if (mouseMode == "ZoomOutMode")
         {
            double dZoom = getZoomPercentage();
            dZoom /= 1.25;

            zoomAboutPoint(mMouseStart, dZoom);
            bSuccess = true;
         }
      }
      else if (e->button() == Qt::MidButton)
      {
         setCursor(Qt::SizeAllCursor);
         // Do not set the success flag to fall through to the base class for directional pan
      }
      else if (e->button() == Qt::RightButton && mCurrentModifiers == Qt::ControlModifier)
      {
         bSuccess = enableInset(true);
         if (bSuccess == true)
         {
            setCursor(Qt::BlankCursor);
            setInsetPoint(mMouseStart);
         }
      }

      mMouseCurrent = mMouseStart;
   }

   if (bSuccess == true)
   {
      e->accept();
      updateGL();
   }
   else
   {
      ViewImp::mousePressEvent(e);
   }
}

void PerspectiveViewImp::mouseMoveEvent(QMouseEvent* e)
{
   bool bSuccess = false;
   if (e != NULL)
   {
      QPoint ptMouse = e->pos();
      ptMouse.setY(height() - e->pos().y());

      if (isInsetEnabled() == true)
      {
         setInsetPoint(ptMouse);
         updateGL();
      }

      string mouseMode = "";

      const MouseMode* pMouseMode = getCurrentMouseMode();
      if (pMouseMode != NULL)
      {
         pMouseMode->getName(mouseMode);
      }

      Qt::MouseButtons buttons = e->buttons();
      if (buttons & Qt::LeftButton)
      {
         if (mouseMode == "PanMode")
         {
            Service<DesktopServices> pDesktop;

            PanModeType panMode = pDesktop->getPanMode();
            if (panMode == PAN_INSTANT)
            {
               ViewImp::pan(ptMouse, mMouseCurrent);
            }

            bSuccess = true;
         }
         else if (mouseMode == "RotateMode")
         {
            double centerX = width() / 2.0;
            double centerY = height() / 2.0;

            double x1 = mMouseCurrent.x() - centerX;
            double y1 = mMouseCurrent.y() - centerY;
            double x2 = ptMouse.x() - centerX;
            double y2 = ptMouse.y() - centerY;

            double angle1 = atan2(x1, y1) * 180.0 / PI;
            double angle2 = atan2(x2, y2) * 180.0 / PI;

            double dRotationAngle = angle2 - angle1;
            rotateBy(dRotationAngle);

            bSuccess = true;
         }
         else if (mouseMode == "ZoomBoxMode")
         {
            setSelectionBox(mMouseStart, ptMouse);
            bSuccess = true;
         }
      }

      if (buttons & Qt::MidButton)
      {
         QPoint diff = ptMouse - mMouseCurrent;
         double mag = diff.x() + diff.y();
         if (ConfigurationSettings::getSettingAlternateMouseWheelZoom())
         {
            mag = -mag;
         }

         // prevent extreme changes in zoom when tile generation occurs
         // during direct zoom
         if (fabs(mag) > 50.0)
         {
            mag = mag > 0.0 ? 50 : -50;
         }
         double zoomMult = 1.0 + mag / 100.0;
         double zoomFact = getZoomPercentage() * zoomMult;

         zoomAboutPoint(mMouseStart, zoomFact);
         bSuccess = true;
      }

      mMouseCurrent = ptMouse;
      updateStatusBar(mMouseCurrent);
   }

   if (bSuccess == true)
   {
      e->accept();
      updateGL();
   }
   else
   {
      ViewImp::mouseMoveEvent(e);
   }
}

void PerspectiveViewImp::mouseReleaseEvent(QMouseEvent* e)
{
   mDisplayContextMenu = true;

   bool bSuccess = false;
   if (e != NULL)
   {
      QPoint ptMouse = e->pos();
      ptMouse.setY(height() - e->pos().y());

      string mouseMode = "";

      const MouseModeImp* pMouseMode = dynamic_cast<const MouseModeImp*>(getCurrentMouseMode());
      if (pMouseMode != NULL)
      {
         pMouseMode->getName(mouseMode);
      }

      if ((e->button() == Qt::MidButton) || (isInsetEnabled() == true))
      {
         // Restore the cursor
         QCursor mouseCursor(Qt::ArrowCursor);
         if (pMouseMode != NULL)
         {
            mouseCursor = pMouseMode->getCursor();
         }

         setCursor(mouseCursor);

         // Disable the inset
         if (isInsetEnabled() == true)
         {
            enableInset(false);
            mDisplayContextMenu = false;
            bSuccess = true;
         }
      }
      else if (e->button() == Qt::LeftButton)
      {
         if (mouseMode == "PanMode")
         {
            Service<DesktopServices> pDesktop;

            PanModeType panMode = pDesktop->getPanMode();
            if (panMode == PAN_DELAY)
            {
               ViewImp::pan(ptMouse, mMouseStart);
            }
            else if (panMode == PAN_INSTANT)
            {
               endUndoGroup();
            }

            // Restore the cursor
            QCursor mouseCursor(Qt::OpenHandCursor);
            if (pMouseMode != NULL)
            {
               mouseCursor = pMouseMode->getCursor();
            }

            setCursor(mouseCursor);
            bSuccess = true;
         }
         else if (mouseMode == "RotateMode")
         {
            endUndoGroup();
            bSuccess = true;
         }
         else if (mouseMode == "ZoomBoxMode")
         {
            setSelectionBox(QRect());
            ViewImp::zoomToBox(mMouseStart, ptMouse);
            bSuccess = true;
         }
      }

      mCurrentModifiers = Qt::NoModifier;
   }

   if (bSuccess == true)
   {
      e->accept();
      updateGL();
   }
   else
   {
      ViewImp::mouseReleaseEvent(e);
   }
}

void PerspectiveViewImp::contextMenuEvent(QContextMenuEvent* pEvent)
{
   if (mDisplayContextMenu == true)
   {
      ViewImp::contextMenuEvent(pEvent);
   }
}

void PerspectiveViewImp::wheelEvent(QWheelEvent* e)
{
   if (e != NULL)
   {
      bool zoomIn = e->delta() > 0;
      if (ConfigurationSettings::getSettingAlternateMouseWheelZoom())
      {
         zoomIn = !zoomIn;
      }
      if (isInsetEnabled())
      {
         zoomInset(zoomIn);
      }
      else
      {
         double dZoom = getZoomPercentage();
         if (zoomIn)
         {
            dZoom *= 1.25;
         }
         else
         {
            dZoom /= 1.25;
         }

         QPoint ptMouse = e->pos();
         ptMouse.setY(height() - e->pos().y());

         zoomAboutPoint(ptMouse, dZoom);
      }

      updateGL();

      e->accept();
      return;
   }

   ViewImp::wheelEvent(e);
}

void PerspectiveViewImp::leaveEvent(QEvent* e)
{
   updateStatusBar(QPoint());
   ViewImp::leaveEvent(e);
}

void PerspectiveViewImp::updateMatrices(int width, int height)
{
   if ((width <= 0) || (height <= 0))
   {
      return;
   }

   GlContextSave contextSave(this);

   // Save current matrices
   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   // Update eyepoint
   PointType eyepoint;
   eyepoint.x = (mDist * cos((mHeading - 90.0) * PI / 180.0)) * cos(mPitch * PI / 180.0) + mCenter.mX;
   eyepoint.y = (mDist * sin((mHeading - 90.0) * PI / 180.0)) * cos(mPitch * PI / 180.0) + mCenter.mY;
   eyepoint.z = mDist * sin(mPitch * PI / 180.0);
   eyepoint.pitch = -mPitch;
   eyepoint.heading = 360.0 + mHeading;
   eyepoint.roll = 0.0;

   // Viewport
   glViewport(0, 0, width, height);

   // Projection matrix
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(mFov, static_cast<double>(width) / static_cast<double>(height), mFrontPlane, mBackPlane);

   // Model matrix
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glRotatef(-90.0, 1, 0, 0);                   // now the y-axis is forward and z-axis points up
   glRotatef(-1 * eyepoint.roll, 0, 1, 0);      // roll about the y-axis
   glRotatef(-1 * eyepoint.pitch, 1, 0, 0);     // pitch about the x-axis degrees
   glRotatef(-1 * eyepoint.heading, 0, 0, 1);   // heading changes about the z-axis
   if (mPixelAspect > 1)
   {
      glScalef(mPixelAspect, 1, 1);
   }
   else
   {
      glScalef(1, 1/mPixelAspect, 1);
   }
   glTranslatef(-1 * eyepoint.x, -1 * eyepoint.y, -1 * eyepoint.z);

   // Update the member matrix values
   glGetIntegerv(GL_VIEWPORT, mViewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, mProjMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, mModelMatrix);

   // Restore current matrices
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(projectionMatrix);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd(modelMatrix);
   glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);

   emit displayAreaChanged();
   notify(SIGNAL_NAME(PerspectiveView, DisplayAreaChanged), boost::any());
}

void PerspectiveViewImp::drawInset()
{
   if (isInsetEnabled() == false)
   {
      return;
   }

   setupWorldMatrices();

   // Unlink all views to prevent changing their zoom levels
   vector<pair<View*, LinkType> > linkedViews = getLinkedViews();

   vector<pair<View*, LinkType> >::iterator iter = linkedViews.begin();
   while (iter != linkedViews.end())
   {
      View* pView = iter->first;
      if (pView != NULL)
      {
         unlinkView(pView);
      }

      ++iter;
   }

   // Get the inset parameters from the Options
   int insetSize = static_cast<int>(View::getSettingInsetSize());
   double zoomMultiplier = static_cast<double>(View::getSettingInsetZoom());
   InsetZoomMode zoomMode = View::getSettingInsetZoomMode();

   // Adjust the zoom multiplier if the zoom inset mode is relative
   double zoomFactor = mFullDistance / mDist;
   if (zoomMode == RELATIVE_MODE)
   {
      zoomMultiplier *= zoomFactor;
   }

   // Save the current matrices
   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   // Get the inset location in screen coordinates
   LocationType insetLocation = getInsetLocation();

   double dScreenX = 0.0;
   double dScreenY = 0.0;
   GLdouble winZ;
   gluProject(insetLocation.mX, insetLocation.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
      &dScreenX, &dScreenY, &winZ);

   int writellx = dScreenX - insetSize / 2;
   int writelly = dScreenY - insetSize / 2;

   // Prevent undo actions from being registered
   UndoLock lock(dynamic_cast<View*>(this));

   // Prevent signals from being emitted
   blockSignals(true);

   // View contents
   zoomAboutPoint(insetLocation, zoomMultiplier * 100.0);

   glEnable(GL_SCISSOR_TEST);
   glScissor(writellx, writelly, insetSize, insetSize);
   glDrawBuffer(GL_BACK);
   glClear(GL_COLOR_BUFFER_BIT);
   drawContents();
   glDisable(GL_SCISSOR_TEST);

   zoomAboutPoint(insetLocation, zoomFactor * 100.0);

   // Setup the matrices
   glLineWidth(1.0);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // Black border
   const double one = 0.999;

   qglColor(Qt::black);
   glViewport(writellx, writelly, insetSize, insetSize);
   gluOrtho2D(0.0, 1.0, 0.0, 1.0);
   glBegin(GL_LINE_LOOP);
   glVertex2d(0.0, 0.0);
   glVertex2d(one, 0.0);
   glVertex2d(one, one);
   glVertex2d(0.0, one);
   glEnd();

   // White border
   qglColor(Qt::white);
   glViewport(writellx + 1, writelly + 1, insetSize - 2, insetSize - 2);
   glBegin(GL_LINE_LOOP);
   glVertex2d(0.0, 0.0);
   glVertex2d(one, 0.0);
   glVertex2d(one, one);
   glVertex2d(0.0, one);
   glEnd();

   // Cross hair
   const double XH_SIZE = 0.025;

   glColor3ub(0xff, 0xaf, 0x7f);
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
   glBegin(GL_LINES);
   glVertex2d(0.5 - XH_SIZE, 0.5);
   glVertex2d(0.5 + XH_SIZE, 0.5);
   glVertex2d(0.5, 0.5 - XH_SIZE);
   glVertex2d(0.5, 0.5 + XH_SIZE);
   glEnd();
   glDisable(GL_BLEND);

   // Zoom label
   QString strZoom;
   strZoom.sprintf("%d%c", static_cast<int>(zoomMultiplier * 100.0), '%');

   int screenX = writellx + 4;
   int screenY = writelly + 3;

   // Map the scissor box screen coordinates from the product view to the
   // spatial data view if the inset text is drawn in a product
   if (dynamic_cast<ViewImp*>(parentWidget()) != NULL)
   {
      QPoint screenCoord = mapFromParent(QPoint(screenX, screenY));
      screenX = screenCoord.x();
      screenY = screenCoord.y();
   }

   glLineWidth(2.0);
   qglColor(Qt::black);
   renderText(screenX, height() - screenY, strZoom);

   screenX = writellx + 3;
   screenY = writelly + 4;

   // Map the scissor box screen coordinates from the product view to the
   // spatial data view if the inset text is drawn in a product
   if (dynamic_cast<ViewImp*>(parentWidget()) != NULL)
   {
      QPoint screenCoord = mapFromParent(QPoint(screenX, screenY));
      screenX = screenCoord.x();
      screenY = screenCoord.y();
   }

   glLineWidth(1.0);
   qglColor(Qt::white);
   renderText(screenX, height() - screenY, strZoom);

   // Restore matrices
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(projectionMatrix);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd(modelMatrix);
   glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);

   // Restore signals
   blockSignals(false);

   // Relink the views
   iter = linkedViews.begin();
   while (iter != linkedViews.end())
   {
      View* pView = iter->first;
      if (pView != NULL)
      {
         linkView(pView, iter->second);
      }

      ++iter;
   }
}

double PerspectiveViewImp::getPixelAspect() const
{
   return mPixelAspect;
}

void PerspectiveViewImp::setPixelAspect(double aspect)
{
   // Before updating the matrices, we must check if the absolute value of the difference between 
   // the old pixel aspect and the new one is less than an epsilon which happens to be 0.001 aka 1e-3. 
   // This if check must be done to minimize the amount of zooming and panning that occur when linking 
   // two views via a two-way link. 1e-3 was chosen because it seems to be the best value to make the 
   // previously mentioned zooming and panning less noticeable. However, there is no found value that 
   // will prevent it completely. 
   if (fabs(mPixelAspect - aspect) < 1e-3)
   {
      if (mbLinking == false)
      {
         mPixelAspect = aspect;
         updateMatrices(); // updateMatricies does notify(SIGNAL_NAME(Subject, Modified))

         executeOnLinks<PerspectiveViewImp>(boost::bind(&PerspectiveViewImp::setPixelAspect, _1, aspect), 
            GeocoordLinkFunctor(this));
      }

   }
}

double PerspectiveViewImp::limitZoomPercentage(double dPercent)
{
   // restrict the distance to be between mFrontPlane
   // and mFullDistance * 100
   if (dPercent > 100.0 * mFullDistance / mFrontPlane)
   {
      dPercent = 100.0 * mFullDistance / mFrontPlane;
   }

   if (dPercent < 1.0)
   {
      dPercent = 1.0;
   }

   return dPercent;
}

LocationType PerspectiveViewImp::limitPanCenter(LocationType center)
{
   return center;
}

bool PerspectiveViewImp::toXml(XMLWriter* pXml) const
{
   if (!ViewImp::toXml(pXml))
   {
      return false;
   }

   stringstream buf;
   pXml->addAttr("distance", mDist);
   pXml->addAttr("fullDistance", mFullDistance);
   pXml->addAttr("center", mCenter);
   pXml->addAttr("heading", mHeading);
   pXml->addAttr("pitch", mPitch);
   pXml->addAttr("fieldOfView", mFov);
   pXml->addAttr("frontPlane", mFrontPlane);
   pXml->addAttr("backPlane", mBackPlane);
   pXml->addAttr("pixelAspect", mPixelAspect);
   return true;
}

bool PerspectiveViewImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !ViewImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   mDist = StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("distance"))));
   mFullDistance = StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("fullDistance"))));
   mCenter = StringUtilities::fromXmlString<LocationType>(A(pElem->getAttribute(X("center"))));
   mHeading = StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("heading"))));
   mPitch = StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("pitch"))));
   mFov = StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("fieldOfView"))));
   mFrontPlane = StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("frontPlane"))));
   mBackPlane = StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("backPlane"))));
   mPixelAspect = StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("pixelAspect"))));

   updateMatrices();
   return true;
}

void PerspectiveViewImp::zoomInsetFactor()
{
   if ((isInsetEnabled() == false) || (sKeyboardNumber == 0))
   {
      return;
   }

   zoomInsetTo(sKeyboardNumber);
   sKeyboardNumber = 0;

   refresh();
}

void PerspectiveViewImp::zoomToEnabledInset()
{
   if (isInsetEnabled() == true)
   {
      zoomToInset();
      refresh();
   }
}

void PerspectiveViewImp::togglePixelCoordinates()
{
   toggleShowCoordinates();
   refresh();
}
