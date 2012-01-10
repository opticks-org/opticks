/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include "AnimationController.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "ColorType.h"
#include "ConfigurationSettings.h"
#include "ContextMenuActions.h"
#include "ContextMenuImp.h"
#include "DesktopServices.h"
#include "DrawUtil.h"
#include "Endian.h"
#include "FontImp.h"
#include "GeocoordLinkFunctor.h"
#include "glCommon.h"
#include "ImageResolutionWidget.h"
#include "MouseModeImp.h"
#include "PropertiesView.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "StringUtilities.h"
#include "Undo.h"
#include "UndoAction.h"
#include "UndoStack.h"
#include "UtilityServicesImp.h"
#include "View.h"
#include "ViewImp.h"
#include "ViewUndo.h"
#include "xmlreader.h"

#include <QtCore/QEvent>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QCursor>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFontMetrics>
#include <QtOpenGL/QGLFramebufferObject>

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

#include <string>
using namespace std;
XERCES_CPP_NAMESPACE_USE

const QGLWidget* ViewImp::mpShareWidget = NULL;

ViewImp::ViewImp(const string& id, const string& viewName, QGLContext* drawContext, QWidget* parent) :
   QGLWidget(new ViewContext(drawContext, QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)), parent,
      ViewImp::getShareWidget()),
   SessionItemImp(id, viewName),
   mbLinking(false),
   mClassificationColor(Qt::white),
   mClassificationEnabled(ConfigurationSettings::getSettingDisplayClassificationMarkings()),
   mReleaseInfoEnabled(true),
   mpMouseMode(NULL),
   mpOldMouseMode(NULL),
   mMinX(0.0),
   mMinY(0.0),
   mMaxX(1.0),
   mMaxY(1.0),
   mInset(false),
   mCrossHair(false),
   mpAnimationController(NULL),
   mpUndoStack(new UndoStack(this)),
   mUndoBlocked(false),
   mpUndoGroup(NULL),
   mpSubImageIterator(NULL)
{
   ColorType color = View::getSettingBackgroundColor();
   mBackgroundColor = COLORTYPE_TO_QCOLOR(color);
   mCrossHair = View::getSettingDisplayCrosshair();
   mOrigin = View::getSettingDataOrigin();
   mMousePanTimer.setInterval(0);

   mClassificationFont.setFamily("Helvetica");
   mClassificationFont.setPointSize(12);
   mClassificationFont.setBold(true);

   memset(mModelMatrix, 0, 16 * sizeof(double));
   memset(mProjMatrix, 0, 16 * sizeof(double));
   mViewPort[0] = 0;
   mViewPort[1] = 0;
   mViewPort[2] = -1;
   mViewPort[3] = -1;

   // Keyboard shortcut actions
   Service<DesktopServices> pDesktop;

   QAction* pMousePanAction = new QAction("Toggle Mouse Pan", this);
   pMousePanAction->setAutoRepeat(false);
   pMousePanAction->setShortcut(QKeySequence("Ctrl+Shift+E"));
   pMousePanAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pMousePanAction, "View");
   addAction(pMousePanAction);

   // Initialization
   setContextMenuPolicy(Qt::DefaultContextMenu);
   setMouseTracking(true);
   setFocusPolicy(Qt::ClickFocus);
   setAttribute(Qt::WA_DeleteOnClose);
   addPropertiesPage(PropertiesView::getName());
   addPropertiesPage("Classification Properties");

   int undoLimit = static_cast<int>(ConfigurationSettings::getSettingUndoBufferSize());
   mpUndoStack->setUndoLimit(undoLimit);

   // Connections
   VERIFYNR(connect(pMousePanAction, SIGNAL(triggered()), this, SLOT(toggleMousePanByKey())));
   VERIFYNR(connect(&mMousePanTimer, SIGNAL(timeout()), this, SLOT(mousePanTimeout())));
}

ViewImp::~ViewImp()
{
   // Clear the undo stack so that undo objects with pointers to this object
   // have a chance to be deleted gracefully while it is still valid.
   // e.g.: Any objects with a GlTextureResource need the QGLContext* to still be valid.
   clearUndo();

   // Detach all linked views
   vector<pair<View*, LinkType> >::iterator iter;
   for (iter = mLinkedViews.begin(); iter != mLinkedViews.end(); ++iter)
   {
      View* pLinkedView = iter->first;
      if (pLinkedView != NULL)
      {
         pLinkedView->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ViewImp::viewDeleted));
      }
   }

   // Detach from DesktopServices
   Service<DesktopServices> pDesktop;
   detach(SIGNAL_NAME(View, AboutToShowContextMenu),
      Signal(pDesktop.get(), SIGNAL_NAME(DesktopServices, AboutToShowContextMenu)));

   // Destroy the animation controller
   setAnimationController(NULL);

   // Destroy the undo group
   if (mpUndoGroup != NULL)
   {
      delete mpUndoGroup;
   }
}

const string& ViewImp::getObjectType() const
{
   static string type("ViewImp");
   return type;
}

bool ViewImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "View"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void ViewImp::viewDeleted(Subject &subject, const string &signal, const boost::any &v)
{
   View* pView = dynamic_cast<View*>(&subject);
   if (pView != NULL)
   {
      unlinkView(pView);
   }
}

void ViewImp::animationControllerDeleted(Subject &subject, const string &signal, const boost::any &v)
{
   AnimationController* pAnimationController = dynamic_cast<AnimationController*>(&subject);
   if (pAnimationController != NULL)
   {
      setAnimationController(NULL);
   }
}

bool ViewImp::isKindOfView(const string& className)
{
   if ((className == "ViewImp") || (className == "View"))
   {
      return true;
   }

   return false;
}

void ViewImp::getViewTypes(vector<string>& classList)
{
   classList.push_back("View");
}

ViewImp& ViewImp::operator= (const ViewImp& view)
{
   if (this != &view)
   {
      SessionItemImp::operator= (view);

      mBackgroundColor = view.mBackgroundColor;
      mClassification = view.mClassification;
      mClassificationFont = view.mClassificationFont;
      mClassificationColor = view.mClassificationColor;
      mClassificationEnabled = view.mClassificationEnabled;
      mReleaseInfoEnabled = view.mReleaseInfoEnabled;
      mOrigin = view.mOrigin;
      mMinX = view.mMinX;
      mMinY = view.mMinY;
      mMaxX = view.mMaxX;
      mMaxY = view.mMaxY;
      mInsetLocation = view.mInsetLocation;
      mSelectionBox = view.mSelectionBox;
      mInset = view.mInset;
      mCrossHair = view.mCrossHair;
      mMouseStart = view.mMouseStart;

      // Reset all linked views
      while (mLinkedViews.empty() == false)
      {
         VERIFYNR(unlinkView(mLinkedViews.back().first));
      }

      for (vector<pair<View*, LinkType> >::const_iterator iter = view.mLinkedViews.begin();
         iter != view.mLinkedViews.end();
         ++iter)
      {
         VERIFYNR(linkView(iter->first, iter->second));
      }

      setAnimationController(view.mpAnimationController);

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

void ViewImp::setName(const string& viewName)
{
   if (viewName.empty() == true)
   {
      return;
   }

   if (viewName != getName())
   {
      SessionItemImp::setName(viewName);
      emit renamed(QString::fromStdString(viewName));
      notify(SIGNAL_NAME(View, Renamed), boost::any(viewName));
   }
}

void ViewImp::setClassification(const Classification* pClassification)
{
   const ClassificationAdapter* pClassificationAdapter = dynamic_cast<const ClassificationAdapter*>(pClassification);
   if (pClassificationAdapter == NULL)
   {
      return;
   }

   string currentClassification;
   string newClassification;
   mClassification.getClassificationText(currentClassification);
   pClassification->getClassificationText(newClassification);

   if (newClassification != currentClassification)
   {
      mClassification = *pClassificationAdapter;
      emit classificationChanged(&mClassification);
      notify(SIGNAL_NAME(View, ClassificationChanged),
         boost::any(static_cast<const Classification*>(&mClassification)));
   }
}

Classification* ViewImp::getClassification()
{
   return &mClassification;
}

const Classification* ViewImp::getClassification() const
{
   return &mClassification;
}

QString ViewImp::getClassificationText() const
{
   string classificationText;
   mClassification.getClassificationText(classificationText);

   return QString::fromStdString(classificationText);
}

QFont ViewImp::getClassificationFont() const
{
   return mClassificationFont;
}

QColor ViewImp::getClassificationColor() const
{
   return mClassificationColor;
}

QColor ViewImp::getBackgroundColor() const
{
   return mBackgroundColor;
}

DataOrigin ViewImp::getDataOrigin() const
{
   return mOrigin;
}

AnimationController *ViewImp::getAnimationController() const
{
   return mpAnimationController;
}

void ViewImp::setAnimationController(AnimationController *pPlayer)
{
   if (mpAnimationController != pPlayer)
   {
      if (mpAnimationController != NULL)
      {
         mpAnimationController->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ViewImp::animationControllerDeleted));
      }
      mpAnimationController = pPlayer;
      if (mpAnimationController != NULL)
      {
         mpAnimationController->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ViewImp::animationControllerDeleted));
      }
      notify(SIGNAL_NAME(ViewImp, AnimationControllerChanged), boost::any(pPlayer));
   }
}

bool ViewImp::addMouseMode(const MouseMode* pMouseMode)
{
   if (pMouseMode == NULL)
   {
      return false;
   }

   if (containsMouseMode(pMouseMode) == true)
   {
      return false;
   }

   // TODO: Add checks for non-supported modes based on the name?

   mMouseModes[pMouseMode] = true;
   emit mouseModeAdded(pMouseMode);
   return true;
}

bool ViewImp::removeMouseMode(const MouseMode* pMouseMode)
{
   if (pMouseMode != NULL)
   {
      if (containsMouseMode(pMouseMode) == true)
      {
         if (pMouseMode == mpMouseMode)
         {
            setMouseMode(NULL);
         }

         mMouseModes.erase(pMouseMode);
         emit mouseModeRemoved(pMouseMode);
         return true;
      }
   }

   return false;
}

bool ViewImp::deleteMouseMode(const MouseMode* pMouseMode)
{
   bool bSuccess = removeMouseMode(pMouseMode);
   if (bSuccess == true)
   {
      delete (static_cast<const MouseModeImp*>(pMouseMode));
   }

   return bSuccess;
}

bool ViewImp::setMouseMode(const MouseMode* pMouseMode)
{
   if (pMouseMode == mpMouseMode)
   {
      return false;
   }

   if (pMouseMode != NULL)
   {
      if (isMouseModeEnabled(pMouseMode) == false)
      {
         return false;
      }
   }

   mpMouseMode = pMouseMode;

   QCursor mouseCursor(Qt::ArrowCursor);
   if (mpMouseMode != NULL)
   {
      const MouseModeImp* pMouseModeImp = dynamic_cast<const MouseModeImp*>(mpMouseMode);
      if (pMouseModeImp != NULL)
      {
         mouseCursor = pMouseModeImp->getCursor();
      }
   }

   setCursor(mouseCursor);
   emit mouseModeChanged(mpMouseMode);
   notify(SIGNAL_NAME(View, MouseModeChanged), boost::any(mpMouseMode));

   enableMousePan(false);
   return true;
}

bool ViewImp::setMouseMode(const QString& strModeName)
{
   const MouseMode* pMouseMode = getMouseMode(strModeName);
   return setMouseMode(pMouseMode);
}

bool ViewImp::containsMouseMode(const MouseMode* pMouseMode) const
{
   if (pMouseMode == NULL)
   {
      return false;
   }

   map<const MouseMode*, bool>::const_iterator iter = mMouseModes.find(pMouseMode);
   return (iter != mMouseModes.end());
}

const MouseMode* ViewImp::getMouseMode(const QString& strModeName) const
{
   if (strModeName.isEmpty() == true)
   {
      return NULL;
   }

   map<const MouseMode*, bool>::const_iterator iter = mMouseModes.begin();
   while (iter != mMouseModes.end())
   {
      const MouseModeImp* pMode = static_cast<const MouseModeImp*>(iter->first);
      if (pMode != NULL)
      {
         QString strCurrentName = pMode->getName();
         if (strCurrentName == strModeName)
         {
            return pMode;
         }
      }

      ++iter;
   }

   return NULL;
}

const MouseMode* ViewImp::getCurrentMouseMode() const
{
   return mpMouseMode;
}

vector<const MouseMode*> ViewImp::getMouseModes() const
{
   vector<const MouseMode*> mouseModes;

   map<const MouseMode*, bool>::const_iterator iter = mMouseModes.begin();
   while (iter != mMouseModes.end())
   {
      const MouseMode* pMode = iter->first;
      if (pMode != NULL)
      {
         mouseModes.push_back(pMode);
      }

      ++iter;
   }

   return mouseModes;
}

unsigned int ViewImp::getNumMouseModes() const
{
   return mMouseModes.size();
}

void ViewImp::enableMouseMode(const MouseMode* pMouseMode, bool bEnable)
{
   if (pMouseMode == NULL)
   {
      return;
   }

   map<const MouseMode*, bool>::iterator iter = mMouseModes.find(pMouseMode);
   if (iter != mMouseModes.end())
   {
      if (bEnable != iter->second)
      {
         if ((pMouseMode == getCurrentMouseMode()) && (bEnable == false))
         {
            setMouseMode(NULL);
         }

         mMouseModes[pMouseMode] = bEnable;
         emit mouseModeEnabled(pMouseMode, bEnable);
      }
   }
}

bool ViewImp::isMouseModeEnabled(const MouseMode* pMouseMode) const
{
   bool bEnabled = false;

   map<const MouseMode*, bool>::const_iterator iter = mMouseModes.find(pMouseMode);
   if (iter != mMouseModes.end())
   {
      bEnabled = iter->second;
   }

   return bEnabled;
}

void ViewImp::enableMousePan(bool enabled)
{
   if (enabled == mMousePanTimer.isActive())
   {
      return;
   }

   if (enabled)
   {
      mMousePanAnchor = QCursor::pos();
      mpOldMouseMode = getCurrentMouseMode();
      setMouseMode(NULL);
      setCursor(Qt::SizeAllCursor);
      enableInset(false);
      QApplication::instance()->installEventFilter(this);
      startUndoGroup("Pan");
      mMousePanTimer.start();
      notify(SIGNAL_NAME(View, MousePanEnabled), true);
   }
   else
   {
      QApplication::instance()->removeEventFilter(this);
      mMousePanTimer.stop();
      endUndoGroup();
      setMouseMode(mpOldMouseMode);
      if (mpOldMouseMode == NULL)
      {
         setCursor(Qt::ArrowCursor);
      }
      refresh();
      notify(SIGNAL_NAME(View, MousePanEnabled), false);
   }
}

bool ViewImp::isMousePanEnabled() const
{
   return mMousePanTimer.isActive();
}

void ViewImp::setMousePanPos(const QPoint& globalScreenCoord)
{
   mMousePanCurrentPos = globalScreenCoord;
}

void ViewImp::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY) const
{
   dMinX = mMinX;
   dMinY = mMinY;
   dMaxX = mMaxX;
   dMaxY = mMaxY;
}

void ViewImp::getVisibleCorners(LocationType& lowerLeft, LocationType& upperLeft, LocationType& upperRight,
      LocationType& lowerRight) const
{
   translateScreenToWorld(0, 0, lowerLeft.mX, lowerLeft.mY);
   translateScreenToWorld(0, height(), upperLeft.mX, upperLeft.mY);
   translateScreenToWorld(width(), height(), upperRight.mX, upperRight.mY);
   translateScreenToWorld(width(), 0, lowerRight.mX, lowerRight.mY);
}

LocationType ViewImp::getVisibleCenter() const
{
   LocationType center;
   translateScreenToWorld(width() / 2.0, height() / 2.0, center.mX, center.mY);

   return center;
}

void ViewImp::translateWorldToScreen(double dWorldX, double dWorldY, double& dScreenX, double& dScreenY,
                                     bool* pVisible) const
{
   GLdouble winX;
   GLdouble winY;
   GLdouble winZ;
   gluProject(dWorldX, dWorldY, 0.0, mModelMatrix, mProjMatrix, mViewPort, &winX, &winY, &winZ);

   dScreenX = winX;
   dScreenY = winY;

   if (pVisible != NULL)
   {
      *pVisible = ((winZ > 0) && (winZ < 1));
   }
}

void ViewImp::translateScreenToWorld(double dScreenX, double dScreenY, double& dWorldX, double& dWorldY) const
{
   DrawUtil::unProjectToZero(dScreenX, dScreenY, mModelMatrix, mProjMatrix, mViewPort, &dWorldX, &dWorldY);
}

inline double magdiff2(LocationType temp1, LocationType temp2)
{
   double tempX = temp1.mX-temp2.mX; 
   double tempY = temp1.mY-temp2.mY;
   return tempX * tempX + tempY * tempY;
}

double ViewImp::getPixelSize(const LocationType& lowerLeft, const LocationType& upperRight) const
{
   double pixelSize = 0.0;
   LocationType temp1;
   LocationType temp2;

   translateWorldToScreen(lowerLeft.mX, lowerLeft.mY, temp1.mX, temp1.mY);
   translateWorldToScreen(lowerLeft.mX + 1.0, lowerLeft.mY, temp2.mX, temp2.mY);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   translateWorldToScreen(lowerLeft.mX, lowerLeft.mY + 1.0, temp2.mX, temp2.mY);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   translateWorldToScreen(lowerLeft.mX, upperRight.mY, temp1.mX, temp1.mY);
   translateWorldToScreen(lowerLeft.mX + 1.0, upperRight.mY, temp2.mX, temp2.mY);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   translateWorldToScreen(lowerLeft.mX, upperRight.mY + 1.0, temp2.mX, temp2.mY);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   translateWorldToScreen(upperRight.mX, lowerLeft.mY, temp1.mX, temp1.mY);
   translateWorldToScreen(upperRight.mX + 1.0, lowerLeft.mY, temp2.mX, temp2.mY);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   translateWorldToScreen(upperRight.mX, lowerLeft.mY + 1.0, temp2.mX, temp2.mY);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   translateWorldToScreen(upperRight.mX, upperRight.mY, temp1.mX, temp1.mY);
   translateWorldToScreen(upperRight.mX + 1.0, upperRight.mY, temp2.mX, temp2.mY);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   translateWorldToScreen(upperRight.mX, upperRight.mY + 1.0, temp2.mX, temp2.mY);
   pixelSize = max(pixelSize, magdiff2(temp1, temp2));

   return sqrt(pixelSize);
}

bool ViewImp::isInsetEnabled() const
{
   return mInset;
}

LocationType ViewImp::getInsetLocation() const
{
   return mInsetLocation;
}

bool ViewImp::isCrossHairEnabled() const
{
   return mCrossHair;
}

void ViewImp::draw()
{
   GlContextSave contextSave(this);
   drawContents();
   drawMousePanAnchor();
   drawCrossHair();
   drawSelectionBox();
   drawInset();
   drawClassification();
}

void ViewImp::renderText(int screenCoordX, int screenCoordY, const QString& strText, const QFont& fnt)
{
   // Check to see if the view's parent widget is a view, which indicates that the text is being drawn in a product
   ViewImp* pParentView = dynamic_cast<ViewImp*>(parentWidget());
   if (pParentView != NULL)
   {
      // Map the coordinates to the parent view's coordinates that is drawing the text
      QPoint textCoord = mapToParent(QPoint(screenCoordX, screenCoordY));
      pParentView->renderText(textCoord.x(), textCoord.y(), strText, fnt);
   }
   else
   {
      QGLWidget::renderText(screenCoordX, screenCoordY, strText, fnt);
   }
}

QImage ViewImp::getCurrentImage()
{
   QImage image;
   getCurrentImage(image);
   return image;
}

bool ViewImp::getCurrentImage(QImage &image)
{
   if (QGLFramebufferObject::hasOpenGLFramebufferObjects())
   {
      int curWidth = width();
      int curHeight = height();
      int iWidth = curWidth;
      int iHeight = curHeight;
      if (!image.isNull())
      {
         iWidth = image.width();
         iHeight = image.height();
      }

      LocationType ll;
      LocationType ul;
      LocationType ur;
      LocationType lr;

      if ((curWidth != iWidth) || (curHeight != iHeight))
      {
         getVisibleCorners(ll, ul, ur, lr);
         resize(iWidth, iHeight);
         zoomToBox(ll, ur);
      }

      GlContextSave contextSave(this);
      QGLFramebufferObject fbo(iWidth, iHeight, QGLFramebufferObject::CombinedDepthStencil);
      fbo.bind();
      drawImage(iWidth, iHeight);
      fbo.release();
      if (image.isNull())
      {
         image = fbo.toImage();
      }
      else
      {
         image.convertToFormat(QImage::Format_ARGB32);
         QImage tmpImage = fbo.toImage().convertToFormat(image.format());
         memcpy(image.bits(), tmpImage.bits(), image.numBytes());
      }

      if ((curWidth != iWidth) || (curHeight != iHeight))
      {
         resize(curWidth, curHeight);
         zoomToBox(ll, ur);
      }

      return !image.isNull();
   }
   else
   {
      int curWidth = width();
      int curHeight = height();
      if (image.isNull() == true)
      {
         image = QImage(width(), height(), QImage::Format_ARGB32);
      }
      if (image.format() != QImage::Format_ARGB32)
      {
         image.convertToFormat(QImage::Format_ARGB32);
      }

      int iWidth = image.width();
      int iHeight = image.height();
      LocationType ll;
      LocationType ul;
      LocationType ur;
      LocationType lr;

      if ((curWidth != iWidth) || (curHeight != iHeight))
      {
         getVisibleCorners(ll, ul, ur, lr);
         resize(iWidth, iHeight);
         zoomToBox(ll, ur);
      }

      GlContextSave contextSave(this);

      // Draw the current image in the back buffer
      glDrawBuffer(GL_BACK);
      glReadBuffer(GL_BACK);
      drawImage(iWidth, iHeight);

      // Read the pixels from the draw buffer
      vector<unsigned int> pixels(iWidth * iHeight);
      glReadPixels(0, 0, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*>(&pixels.front()));

      reorderImage(&pixels.front(), iWidth, iHeight);

      unsigned char* pBits = image.bits();
      if (pBits != NULL)
      {
         memcpy(pBits, &pixels.front(), iWidth * iHeight * 4);
      }

      if ((curWidth != iWidth) || (curHeight != iHeight))
      {
         resize(curWidth, curHeight);
         zoomToBox(ll, ur);
      }

      return (pBits != NULL);
   }
}

ViewImp::SubImageIteratorImp::SubImageIteratorImp(ViewImp *pView, const QSize &totalSize, const QSize &subImageSize) :
         mpView(pView), mTotalSize(totalSize), mSubImageSize(subImageSize), mTotalTilesX(0), mTotalTilesY(0),
         mCurrentTileX(-1), mCurrentTileY(0)
{
   // save the current view settings
   //
   LocationType dummy;
   mpView->getVisibleCorners(mRestoreLowerLeft, dummy, mRestoreUpperRight, dummy);
   mRestoreViewSize = mpView->size();

   if (subImageSize.width() == 0 || subImageSize.height() == 0 || totalSize.width() == 0 || totalSize.height() == 0)
   {
      return;
   }

   // Calculate the total number of sub-images.
   //
   double totalTilesX = static_cast<double>(totalSize.width()) / static_cast<double>(subImageSize.width());
   double totalTilesY = static_cast<double>(totalSize.height()) / static_cast<double>(subImageSize.height());
   mTotalTilesX = static_cast<int>(ceil(totalTilesX));
   mTotalTilesY = static_cast<int>(ceil(totalTilesY));

   // resize the view to the requested sub-image size (pixel coordinates)
   //
   mpView->resize(mSubImageSize);

   // move the view to the first sub-image
   //

   // Calculate the origin location based on the current visible corners
   mStartLowerLeft = LocationType(mRestoreLowerLeft.mX, mRestoreLowerLeft.mY);

   double inputImageAspect = fabs(mRestoreUpperRight.mX - mRestoreLowerLeft.mX) /
      fabs(mRestoreUpperRight.mY - mRestoreLowerLeft.mY);
   double outputImageAspect = static_cast<double>(totalSize.width()) / static_cast<double>(totalSize.height());

   if (inputImageAspect < outputImageAspect)
   {
      // The image will be drawn to the left, so center in the x-direction
      double deltaX = (outputImageAspect * fabs(mRestoreUpperRight.mY - mRestoreLowerLeft.mY) -
         fabs(mRestoreUpperRight.mX - mRestoreLowerLeft.mX)) / 2.0;
      mStartLowerLeft.mX -= deltaX;
   }
   else
   {
      // The image will be drawn along the top, so center in the y-direction
      double deltaY = ((fabs(mRestoreUpperRight.mX - mRestoreLowerLeft.mX) / outputImageAspect) -
         fabs(mRestoreUpperRight.mY - mRestoreLowerLeft.mY)) / 2.0;
      mStartLowerLeft.mY += deltaY;
   }

   // Calculate the size of each tile in world pixels
   LocationType worldTileSize(fabs(mRestoreUpperRight.mX - mRestoreLowerLeft.mX) / totalTilesX,
      fabs(mRestoreUpperRight.mY - mRestoreLowerLeft.mY) / totalTilesY);

   double tileAspect = worldTileSize.mX / worldTileSize.mY;
   double subImageAspect = static_cast<double>(subImageSize.width()) / static_cast<double>(subImageSize.height());

   if (tileAspect < subImageAspect)
   {
      worldTileSize.mX = subImageAspect * worldTileSize.mY;
   }
   else
   {
      worldTileSize.mY = worldTileSize.mX / subImageAspect;
   }

   worldTileSize.mY *= -1.0;

   LocationType upperRight = mStartLowerLeft + worldTileSize;
   mpView->zoomToBox(mStartLowerLeft, upperRight);
}

ViewImp::SubImageIteratorImp::~SubImageIteratorImp()
{
   // restore the view
   //
   mpView->resize(mRestoreViewSize);
   mpView->zoomToBox(mRestoreLowerLeft, mRestoreUpperRight);
   mpView->mpSubImageIterator = NULL;
}

bool ViewImp::SubImageIteratorImp::hasNext() const
{
   return !((mCurrentTileY >= (mTotalTilesY - 1)) && (mCurrentTileX >= (mTotalTilesX - 1)));
}

bool ViewImp::SubImageIteratorImp::next(QImage &image)
{
   if (!hasNext())
   {
      return false;
   }
   mCurrentTileX++;
   if (mCurrentTileX >= mTotalTilesX)
   {
      mCurrentTileY++;
      mCurrentTileX = 0;
   }

   // move the view if not on the first tile
   //
   if (mCurrentTileX > 0 || mCurrentTileY > 0)
   {
      LocationType visLl;
      LocationType visUr;
      LocationType dummy;
      mpView->getVisibleCorners(visLl, dummy, visUr, dummy);
      double deltaX = 0;
      double deltaY = 0;
      if (mCurrentTileX == 0)
      {
         // reset the x direction and move the y direction
         //
         deltaX = mStartLowerLeft.mX - visLl.mX;
         deltaY = visUr.mY - visLl.mY;
      }
      else
      {
         // move the x direction
         //
         deltaX = visUr.mX - visLl.mX;
      }
      mpView->panBy(deltaX, deltaY);
   }

   // grab the image
   //
   return mpView->getCurrentImage(image);
}

void ViewImp::SubImageIteratorImp::location(int &x, int &y) const
{
   x = mCurrentTileX;
   y = mCurrentTileY;
}

void ViewImp::SubImageIteratorImp::count(int &x, int &y) const
{
   x = mTotalTilesX;
   y = mTotalTilesY;
}

View::SubImageIterator *ViewImp::getSubImageIterator(const QSize &totalSize, const QSize &subImageSize)
{
   if (mpSubImageIterator == NULL)
   {
      mpSubImageIterator = new SubImageIteratorImp(this, totalSize, subImageSize);
   }
   return mpSubImageIterator;
}

bool ViewImp::linkView(View* pView, LinkType type)
{
   if ((pView == NULL) || (dynamic_cast<ViewImp*>(pView) == this))
   {
      return false;
   }

   if (type == NO_LINK)
   {
      unlinkView(pView);
      return true;
   }

   // Do not add the linked view if it is already linked
   if (getViewLinkType(pView) != NO_LINK)
   {
      return false;
   }


   // Add the window to the linked list
   mLinkedViews.push_back(pair<View*, LinkType>(pView, type));

   pView->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ViewImp::viewDeleted));

   if (type == GEOCOORD_LINK || 
      (type == AUTOMATIC_LINK && canLinkWithView(pView, GEOCOORD_LINK)))
   {
      // bring the views into geocoord alignment
      mbLinking = true;
      GeocoordLinkFunctor(this)(dynamic_cast<ViewImp*>(pView));
      mbLinking = false;
   }

   return true;
}

const vector<pair<View*, LinkType> > &ViewImp::getLinkedViews() const
{
   return mLinkedViews;
}

LinkType ViewImp::getViewLinkType(View* pView) const
{
   if ((pView == NULL) || (dynamic_cast<ViewImp*>(pView) == this))
   {
      return NO_LINK;
   }

   for (vector<pair<View*, LinkType> >::const_iterator iter = mLinkedViews.begin();
      iter != mLinkedViews.end(); ++iter)
   {
      View* pLinkedView = iter->first;
      if (pLinkedView == pView)
      {
         return iter->second;
      }
   }

   return NO_LINK;
}

bool ViewImp::unlinkView(View* pView)
{
   if ((pView == NULL) || (dynamic_cast<ViewImp*>(pView) == this))
   {
      return false;
   }

   vector<pair<View*, LinkType> >::iterator iter = mLinkedViews.begin();
   while (iter != mLinkedViews.end())
   {
      View* pLinkedView = iter->first;
      if (pLinkedView == pView)
      {
         pView->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ViewImp::viewDeleted));
         mLinkedViews.erase(iter);
         return true;
      }

      ++iter;
   }

   return false;
}

void ViewImp::setSelectionBox(const vector<LocationType>& selectionBox)
{
   if (selectionBox == mSelectionBox)
   {
      return;
   }

   mSelectionBox = selectionBox;
   emit selectionBoxChanged(mSelectionBox);
   notify(SIGNAL_NAME(View, SelectionBoxChanged), boost::any(mSelectionBox));
}

void ViewImp::setSelectionBox(const LocationType& worldLowerLeft, const LocationType& worldUpperRight)
{
   vector<LocationType> selectionBox;
   selectionBox.push_back(worldLowerLeft);
   selectionBox.push_back(LocationType(worldUpperRight.mX, worldLowerLeft.mY));
   selectionBox.push_back(worldUpperRight);
   selectionBox.push_back(LocationType(worldLowerLeft.mX, worldUpperRight.mY));

   setSelectionBox(selectionBox);
}

void ViewImp::setSelectionBox(const QPolygon& selectionBox)
{
   vector<LocationType> newBox;
   for (int i = 0; i < selectionBox.count(); i++)
   {
      QPoint point = selectionBox.at(i);

      LocationType pixelCoord;
      translateScreenToWorld(point.x(), point.y(), pixelCoord.mX, pixelCoord.mY);

      newBox.push_back(pixelCoord);
   }

   setSelectionBox(newBox);
}

void ViewImp::setSelectionBox(const QPoint& screenLowerLeft, const QPoint& screenUpperRight)
{
   QRect rcBox;
   rcBox.setBottomLeft(screenLowerLeft);
   rcBox.setTopRight(screenUpperRight);

   setSelectionBox(rcBox);
}

void ViewImp::setSelectionBox(const QRect& rcBox)
{
   QPolygon selectionBox;
   if (rcBox.isNull() == false)
   {
      int iBeginX = rcBox.left();
      int iBeginY = rcBox.bottom();
      int iEndX = rcBox.right();
      int iEndY = rcBox.top();

      if (iEndX < iBeginX)
      {
         int iTemp = iEndX;
         iEndX = iBeginX;
         iBeginX = iTemp;
      }

      if (iEndY < iBeginY)
      {
         int iTemp = iEndY;
         iEndY = iBeginY;
         iBeginY = iTemp;
      }

      selectionBox.resize(4);
      selectionBox.setPoint(0, iBeginX, iBeginY);
      selectionBox.setPoint(1, iEndX, iBeginY);
      selectionBox.setPoint(2, iEndX, iEndY);
      selectionBox.setPoint(3, iBeginX, iEndY);
   }

   setSelectionBox(selectionBox);
}

const vector<LocationType>& ViewImp::getSelectionBox() const
{
   return mSelectionBox;
}

void ViewImp::reorderImage(unsigned int pImage[], int iWidth, int iHeight)
{
   int numPixels = iWidth * iHeight;
   int i;
   int j;

   if (Endian::getSystemEndian() == BIG_ENDIAN_ORDER)
   {
      for (i = 0; i < numPixels; i++)
      {
         pImage[i] >>= 8;
      }
   }
   else
   {
      unsigned char tempColor;
      unsigned char* pPixel = NULL;
      for (i = 0; i < numPixels; i++)
      {
         pPixel = reinterpret_cast<unsigned char*>(&pImage[i]);
         tempColor = *pPixel;
         *pPixel = pPixel[2];
         pPixel[2] = tempColor;
      }
   }

   unsigned int* pRow1 = NULL;
   unsigned int* pRow2 = NULL;
   int rows = iHeight;
   int rowWidth = iWidth;
   int swapRowCount = rows / 2;
   unsigned int tempPixel;
   for (i = 0; i < swapRowCount; i++)
   {
      pRow1 = &pImage[iWidth * i];
      pRow2 = &pImage[iWidth * (rows - i - 1)];
      for (j = 0; j < rowWidth; j++)
      {
         tempPixel = pRow1[j];
         pRow1[j] = pRow2[j];
         pRow2[j] = tempPixel;
      }
   }
}

void ViewImp::setClassificationFont(const QFont& classificationFont)
{
   if (classificationFont != mClassificationFont)
   {
      addUndoAction(new SetViewClassificationFont(dynamic_cast<View*>(this), mClassificationFont, classificationFont));

      mClassificationFont = classificationFont;
      emit classificationFontChanged(mClassificationFont);
      notify(SIGNAL_NAME(View, ClassificationFontChanged), boost::any(mClassificationFont));
   }
}

void ViewImp::setClassificationColor(const QColor& clrClassification)
{
   if (clrClassification.isValid() == false)
   {
      return;
   }

   if (clrClassification != mClassificationColor)
   {
      addUndoAction(new SetViewClassificationColor(dynamic_cast<View*>(this), mClassificationColor, clrClassification));

      mClassificationColor = clrClassification;
      emit classificationColorChanged(mClassificationColor);
      notify(SIGNAL_NAME(View, ClassificationColorChanged), boost::any(
         ColorType(mClassificationColor.red(), mClassificationColor.green(),
         mClassificationColor.blue())));
   }
}

void ViewImp::enableClassification(bool bEnable)
{
   mClassificationEnabled = bEnable;
}

void ViewImp::enableReleaseInfo(bool bEnable)
{
   mReleaseInfoEnabled = bEnable;
}

void ViewImp::setBackgroundColor(const QColor& clrBackground)
{
   if (clrBackground.isValid() == false)
   {
      return;
   }

   if (clrBackground != mBackgroundColor)
   {
      addUndoAction(new SetViewBackgroundColor(dynamic_cast<View*>(this), mBackgroundColor, clrBackground));

      mBackgroundColor = clrBackground;
      emit backgroundColorChanged(mBackgroundColor);
      notify(SIGNAL_NAME(View, BackgroundColorChanged), boost::any(
         ColorType(mBackgroundColor.red(), mBackgroundColor.green(), mBackgroundColor.blue())));
   }
}

void ViewImp::setDataOrigin(const DataOrigin& dataOrigin)
{
   if (dataOrigin != mOrigin)
   {
      addUndoAction(new SetViewDataOrigin(dynamic_cast<View*>(this), mOrigin, dataOrigin));

      mOrigin = dataOrigin;

      UndoLock lock(dynamic_cast<View*>(this));
      emit originChanged(mOrigin);
      notify(SIGNAL_NAME(View, OriginChanged), boost::any(mOrigin));
   }
}

bool ViewImp::setExtents(double dMinX, double dMinY, double dMaxX, double dMaxY)
{
   if ((dMinX == mMinX) && (dMinY == mMinY) && (dMaxX == mMaxX) && (dMaxY == mMaxY))
   {
      return false;
   }

   mMinX = dMinX;
   mMinY = dMinY;
   mMaxX = dMaxX;
   mMaxY = dMaxY;

   emit extentsChanged(dMinX, dMinY, dMaxX, dMaxY);
   notify(SIGNAL_NAME(View, ExtentsChanged), boost::any(
      boost::tuple<double, double, double, double>(dMinX, dMinY, dMaxX, dMaxY)));
   return true;
}

void ViewImp::zoomToBox(const QPoint& screenLowerLeft, const QPoint& screenUpperRight)
{
   // Prevent large zoom levels by checking for a box at least three screen pixels in size
   QPoint boxSize = screenUpperRight - screenLowerLeft;
   if ((abs(boxSize.x()) < 3) || (abs(boxSize.y()) < 3))
   {
      return;
   }

   LocationType lowerLeft;
   LocationType upperRight;
   translateScreenToWorld(screenLowerLeft.x(), screenLowerLeft.y(), lowerLeft.mX, lowerLeft.mY);
   translateScreenToWorld(screenUpperRight.x(), screenUpperRight.y(), upperRight.mX, upperRight.mY);

   zoomToBox(lowerLeft, upperRight);
}

void ViewImp::zoomInset(bool in)
{
   unsigned int zoomMultiplier = View::getSettingInsetZoom();

   if (in)
   {
      zoomMultiplier *= 2;
   }
   else
   {
      zoomMultiplier /= 2;
   }

   // ensure 1 <= zoomMultiplier <= 256
   zoomMultiplier = max(zoomMultiplier, static_cast<unsigned int>(1));
   zoomMultiplier = min(zoomMultiplier, static_cast<unsigned int>(256));

   View::setSettingInsetZoom(zoomMultiplier);

   // Refresh the linked views
   executeOnLinks<View>(boost::bind(&View::refresh, _1));
}

void ViewImp::zoomInsetTo(unsigned int zoomMultiplier)
{
   if (zoomMultiplier < 1)
   {
      zoomMultiplier = 1;
   }
   else if (zoomMultiplier > 256)
   {
      zoomMultiplier = 256;
   }

   View::setSettingInsetZoom(zoomMultiplier);

   // Refresh the linked views
   executeOnLinks<View>(boost::bind(&View::refresh, _1));
}

void ViewImp::pan(const QPoint& screenBegin, const QPoint& screenEnd)
{
   LocationType worldBegin;
   LocationType worldEnd;
   translateScreenToWorld(screenBegin.x(), screenBegin.y(), worldBegin.mX, worldBegin.mY);
   translateScreenToWorld(screenEnd.x(), screenEnd.y(), worldEnd.mX, worldEnd.mY);

   pan(worldBegin, worldEnd);
}

void ViewImp::panTo(const QPoint& screenCoord)
{
   QPoint center(width() / 2, height() / 2);
   pan(center, screenCoord);
}

void ViewImp::panTo(const LocationType& worldCoord)
{
   QPointF screenCenter(width() / 2.0, height() / 2.0);

   LocationType center;
   translateScreenToWorld(screenCenter.x(), screenCenter.y(), center.mX, center.mY);

   pan(center, worldCoord);
}

void ViewImp::panToCenter()
{
   LocationType worldCenter((mMaxX + mMinX) / 2.0, (mMaxY - mMinY) / 2.0);
   panTo(worldCenter);
}

void ViewImp::panBy(int iScreenDeltaX, int iScreenDeltaY)
{
   QPoint center(width() / 2, height() / 2);
   QPoint newCenter(center.x() + iScreenDeltaX, center.y() + iScreenDeltaY);

   pan(center, newCenter);
}

void ViewImp::panBy(double dWorldDeltaX, double dWorldDeltaY)
{
   double dCenterX;
   double dCenterY;
   translateScreenToWorld(width() / 2.0, height() / 2.0, dCenterX, dCenterY);

   LocationType newCenter(dCenterX + dWorldDeltaX, dCenterY + dWorldDeltaY);
   panTo(newCenter);
}

bool ViewImp::enableInset(bool bEnable)
{
   if (mbLinking == false)
   {
      mInset = bEnable;

      // Update the linked views
      executeOnLinks<View>(boost::bind(&View::enableInset, _1, bEnable));
      return true;
   }

   return false;
}

void ViewImp::setInsetPoint(const QPoint& screenCoord)
{
   LocationType insetPoint;
   translateScreenToWorld(screenCoord.x(), screenCoord.y(), insetPoint.mX, insetPoint.mY);

   setInsetPoint(insetPoint);
}

void ViewImp::setInsetPoint(const LocationType &worldCoord)
{
   if (mbLinking == false)
   {
      mInsetLocation = worldCoord;

      void (ViewImp::*func)(const LocationType&) = &ViewImp::setInsetPoint;
      executeOnLinks<ViewImp>(boost::bind(func, _1, worldCoord));
   }
}

bool ViewImp::enableCrossHair(bool bEnable)
{
   if (bEnable != mCrossHair)
   {
      mCrossHair = bEnable;
      emit crossHairDisplayed(mCrossHair);
      return true;
   }

   return false;
}

void ViewImp::refresh()
{
   update();
}

bool ViewImp::event(QEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (pEvent->type() == QEvent::Polish)
      {
         Service<DesktopServices> pDesktop;
         attach(SIGNAL_NAME(View, AboutToShowContextMenu),
            Signal(pDesktop.get(), SIGNAL_NAME(DesktopServices, AboutToShowContextMenu)));

         updateMatrices();
      }
   }

   return QGLWidget::event(pEvent);
}

bool ViewImp::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if (isMousePanEnabled())
   {
      if (pEvent->type() == QEvent::FocusOut)
      {
         enableMousePan(false);
      }

      if (pEvent->type() != QEvent::MouseMove && pEvent->type() != QEvent::MouseButtonPress &&
         pEvent->type() != QEvent::MouseButtonRelease)
      {
         return QGLWidget::eventFilter(pObject, pEvent);
      }

      QMouseEvent* pOrgEvent = static_cast<QMouseEvent*>(pEvent);
      QPoint globalPos = QCursor::pos();
      QPoint widgetPos = mapFromGlobal(globalPos);
      QMouseEvent newMouseEvent(pOrgEvent->type(), widgetPos, globalPos, pOrgEvent->button(), pOrgEvent->buttons(),
         pOrgEvent->modifiers());

      if (pEvent->type() == QEvent::MouseMove)
      {
         mouseMoveEvent(&newMouseEvent);
      }
      else if (pEvent->type() == QEvent::MouseButtonPress)
      {
         mousePressEvent(&newMouseEvent);
      }
      else if (pEvent->type() == QEvent::MouseButtonRelease)
      {
         mouseReleaseEvent(&newMouseEvent);
      }

      return true;
   }

   return QGLWidget::eventFilter(pObject, pEvent);
}

void ViewImp::mousePressEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (isMousePanEnabled() == true)
      {
         enableMousePan(false);
         pEvent->accept();
         refresh();
      }
      else if (pEvent->button() == Qt::MidButton)
      {
         mMousePanEngagePos = pEvent->globalPos();
      }
   }
}

void ViewImp::mouseMoveEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (isMousePanEnabled() == true)
      {
         setMousePanPos(pEvent->globalPos());
         pEvent->accept();
      }
   }
}

void ViewImp::mouseReleaseEvent(QMouseEvent* pEvent)
{
   bool success = false;
   if (pEvent != NULL)
   {
      if (pEvent->button() == Qt::MidButton)
      {
         if (isMousePanEnabled() == true)
         {
            enableMousePan(false);
            success = true;
         }
         else
         {
            double deltaX = mMousePanEngagePos.x() - pEvent->globalPos().x();
            double deltaY = mMousePanEngagePos.y() - pEvent->globalPos().y();
            if (fabs(deltaX) < 3 && fabs(deltaY) < 3)
            {
               setMousePanPos(pEvent->globalPos());
               enableMousePan(true);
               success = true;
            }
         }
      }
   }

   if (success == true)
   {
      pEvent->accept();
      refresh();
   }
}

void ViewImp::contextMenuEvent(QContextMenuEvent* pEvent)
{
   if (pEvent != NULL)
   {
      // Create the context menu
      const QPoint& mouseLocation = pEvent->globalPos();
      list<ContextMenuAction> defaultActions = getContextMenuActions();

      vector<SessionItem*> sessionItems;
      sessionItems.push_back(dynamic_cast<SessionItem*>(this));

      ContextMenuImp menu(sessionItems, mouseLocation, defaultActions, this);

      // Notify to allow additional actions to be added
      notify(SIGNAL_NAME(View, AboutToShowContextMenu), boost::any(static_cast<ContextMenu*>(&menu)));

      // Invoke the menu
      if (menu.show() == true)
      {
         return;
      }
   }

   QGLWidget::contextMenuEvent(pEvent);
}

void ViewImp::initializeGL()
{
   // Turn off unnecessary OpenGL functions to increase speed
   glDisable(GL_CULL_FACE);
   glShadeModel(GL_FLAT);          // don't need shading
   glDisable(GL_NORMALIZE);        // don't automatically normalize normal
   glDisable(GL_LIGHTING);         // don't need a lighting model
   glDisable(GL_DEPTH_TEST);       // disable z-buffering
   glDisable(GL_STENCIL_TEST);     // disable stencil
   glDisable(GL_ALPHA_TEST);

   setAutoBufferSwap(true);
}

void ViewImp::paintGL()
{
   drawImage();
}

void ViewImp::setupScreenMatrices()
{
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(viewPort[0], viewPort[0] + viewPort[2], viewPort[1], viewPort[1] + viewPort[3]);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

void ViewImp::setupWorldMatrices()
{
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(mProjMatrix);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd(mModelMatrix);
}

void ViewImp::updateMatrices()
{
   updateMatrices(width(), height());
}

void ViewImp::drawMousePanAnchor()
{
   if (!isMousePanEnabled())
   {
      return;
   }

   setupScreenMatrices();

   ViewImp* pView = this;
   ViewImp* pProductView = dynamic_cast<ViewImp*>(parentWidget());
   if (pProductView != NULL)
   {
      pView = pProductView;
   }

   QPoint anchorPos = pView->mapFromGlobal(mMousePanAnchor);
   anchorPos.setY(pView->height() - anchorPos.y());

   const int boxSize = 6;
   const int crossSize = 12;
   int centerX = anchorPos.x();
   int centerY = anchorPos.y();

   glLineWidth(1.0);
   glColor3ub(0xff, 0xaf, 0x7f);
   glPushAttrib(GL_COLOR_BUFFER_BIT);
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
   glBegin(GL_LINES);
   glVertex2f(centerX - boxSize, centerY - boxSize);
   glVertex2f(centerX + boxSize, centerY - boxSize);
   glVertex2f(centerX + boxSize, centerY - boxSize);
   glVertex2f(centerX + boxSize, centerY + boxSize);
   glVertex2f(centerX - boxSize, centerY - boxSize);
   glVertex2f(centerX - boxSize, centerY + boxSize);
   glVertex2f(centerX - boxSize, centerY + boxSize);
   glVertex2f(centerX + boxSize, centerY + boxSize);
   glVertex2f(centerX, centerY + crossSize);
   glVertex2f(centerX, centerY - crossSize);
   glVertex2f(centerX + crossSize, centerY);
   glVertex2f(centerX - crossSize, centerY);
   glEnd();
   glDisable(GL_BLEND);
   glPopAttrib();
}

void ViewImp::drawCrossHair()
{
   if (mCrossHair == false)
   {
      return;
   }

   setupScreenMatrices();

   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);

   const int size = 10;
   int centerX = viewPort[0] + (viewPort[2] / 2);
   int centerY = viewPort[1] + (viewPort[3] / 2);

   glLineWidth(1.0);
   glColor3ub(0xff, 0xaf, 0x7f);

   glPushAttrib(GL_COLOR_BUFFER_BIT);
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
   glBegin(GL_LINES);
   glVertex2f(centerX, centerY + size);
   glVertex2f(centerX, centerY - size);
   glVertex2f(centerX + size, centerY);
   glVertex2f(centerX - size, centerY);
   glEnd();
   glDisable(GL_BLEND);
   glPopAttrib();
}

void ViewImp::drawSelectionBox()
{
   if (mSelectionBox.size() == 0)
   {
      return;
   }

   setupWorldMatrices();

   vector<LocationType> screenCoords;

   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   unsigned int i = 0;
   for (i = 0; i < mSelectionBox.size(); i++)
   {
      LocationType point = mSelectionBox.at(i);
      LocationType screenCoord;

      GLdouble winZ;
      gluProject(point.mX, point.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
         &screenCoord.mX, &screenCoord.mY, &winZ);

      screenCoords.push_back(screenCoord);
   }

   setupScreenMatrices();

   glLineWidth(2.0);

   // Draw the black shadow box
   glColor4ub(0, 0, 0, 0);
   glBegin(GL_LINE_STRIP);

   for (i = 0; i < screenCoords.size(); i++)
   {
      glVertex2f(screenCoords.at(i).mX + 1, screenCoords.at(i).mY - 1);
   }

   glVertex2f(screenCoords.at(0).mX + 1, screenCoords.at(0).mY - 1);
   glEnd();

   // Draw the white selection box
   glColor4ub(255, 255, 255, 255);
   glBegin(GL_LINE_STRIP);

   for (i = 0; i < screenCoords.size(); i++)
   {
      glVertex2f(screenCoords.at(i).mX, screenCoords.at(i).mY);
   }

   glVertex2f(screenCoords.at(0).mX, screenCoords.at(0).mY);
   glEnd();

   glLineWidth(1.0);
}

void ViewImp::drawClassification()
{
   int iWidth = width();
   int iHeight = height();
   int iClassificationWidth = 0;
   int iClassificationHeight = 0;

   const int topMargin = 1;
   const int bottomMargin = 4;
   const int leftMargin = 5;
   const int rightMargin = 5;
   const int shadowOffset = 2;

   // Draw the classification markings
   QString classificationText = getClassificationText();
   if ((mClassificationEnabled == true) && (classificationText.isEmpty() == false))
   {
      // Calculate the screen width and height of the classification markings
      QFontMetrics fontMetrics(mClassificationFont);
      iClassificationWidth = fontMetrics.width(classificationText);
      iClassificationHeight = fontMetrics.ascent();

      // Top markings - Qt has an upper left origin, so there is
      // no need to offset the y-coordinate by the view height
      qglColor(Qt::black);
      int screenX((iWidth / 2) - (iClassificationWidth / 2) + shadowOffset);
      switch (getClassificationPosition())
      {
      case TOP_LEFT_BOTTOM_LEFT:    // fall through
      case TOP_LEFT_BOTTOM_RIGHT:
         screenX = leftMargin + shadowOffset;
         break;

      case TOP_RIGHT_BOTTOM_LEFT:   // fall through
      case TOP_RIGHT_BOTTOM_RIGHT:
         screenX = iWidth - rightMargin - iClassificationWidth + shadowOffset;
         break;

      default:                      // nothing to do, markings centered by default
         break;
      }
      int screenY = topMargin + iClassificationHeight + shadowOffset;
      renderText(screenX, screenY, classificationText, mClassificationFont);

      qglColor(mClassificationColor);
      screenX -= shadowOffset;
      screenY -= shadowOffset;
      renderText(screenX, screenY, classificationText, mClassificationFont);

      // Bottom markings
      qglColor(Qt::black);
      switch (getClassificationPosition())
      {
      case TOP_LEFT_BOTTOM_LEFT:    // fall through
      case TOP_RIGHT_BOTTOM_LEFT:
         screenX = leftMargin + shadowOffset;
         break;

      case TOP_LEFT_BOTTOM_RIGHT:   // fall through
      case TOP_RIGHT_BOTTOM_RIGHT:
         screenX = iWidth - rightMargin - iClassificationWidth + shadowOffset;
         break;

      default:                      // center the markings
         screenX = (iWidth / 2) - (iClassificationWidth / 2) + shadowOffset;
         break;
      }
      screenY = iHeight - bottomMargin;
      renderText(screenX, screenY, classificationText, mClassificationFont);

      qglColor(mClassificationColor);
      screenX -= shadowOffset;
      screenY -= shadowOffset;
      renderText(screenX, screenY, classificationText, mClassificationFont);
   }

   // Draw the release info
   if (mReleaseInfoEnabled == true)
   {
      // Check if the release is a production release
      Service<ConfigurationSettings> pConfigSettings;
      QFontMetrics fontMetrics(mClassificationFont);

      // Calculate the screen width and height of the release info
      QStringList strRelease(QString::fromStdString(
         StringUtilities::toDisplayString(pConfigSettings->getReleaseType())));

      QString strReleaseDescription = QString::fromStdString(pConfigSettings->getReleaseDescription());
      if (strReleaseDescription.isEmpty() == false)
      {
         strRelease << strReleaseDescription.split('\n');
      }

      int iTotalReleaseHeight = 0;
      for (QList<QString>::const_iterator iter = strRelease.begin(); iter != strRelease.end(); ++iter)
      {
         const QString& strReleaseText = *iter;

         if (!strReleaseText.isEmpty())
         {
            int iReleaseWidth = fontMetrics.width(strReleaseText);
            int iReleaseHeight = fontMetrics.ascent();

            // Release markings - Qt has an upper left origin, so there
            // is no need to offset the y-coordinate by the view height
            qglColor(Qt::black);
            int screenX = (iWidth / 2) - (iReleaseWidth / 2) + shadowOffset;
            int screenY = topMargin + iClassificationHeight + shadowOffset +
               topMargin + iReleaseHeight + shadowOffset + iTotalReleaseHeight;
            renderText(screenX, screenY, strReleaseText, mClassificationFont);

            qglColor(mClassificationColor);
            screenX = (iWidth / 2) - (iReleaseWidth / 2);
            screenY = topMargin + iClassificationHeight + shadowOffset +
               topMargin + iReleaseHeight + iTotalReleaseHeight;
            renderText(screenX, screenY, strReleaseText, mClassificationFont);
            iTotalReleaseHeight += iReleaseHeight + topMargin;
         }
      }

      if (pConfigSettings->isProductionRelease() == false)
      {
         QString strProductionText = "Not for Production Use";
         int iProductionWidth = fontMetrics.width(strProductionText);

         // Production markings
         qglColor(Qt::black);
         int screenX = (iWidth / 2) - (iProductionWidth / 2) + shadowOffset;
         int screenY = iHeight - bottomMargin - shadowOffset - iClassificationHeight - topMargin;
         renderText(screenX, screenY, strProductionText, mClassificationFont);

         qglColor(mClassificationColor);
         screenX = (iWidth / 2) - (iProductionWidth / 2);
         screenY = iHeight - bottomMargin - shadowOffset - iClassificationHeight - topMargin - shadowOffset;
         renderText(screenX, screenY, strProductionText, mClassificationFont);
      }
   }
}

const QGLWidget* ViewImp::getShareWidget()
{
   if (mpShareWidget == NULL)
   {
      mpShareWidget = new QGLWidget();
   }

   return mpShareWidget;
}

void ViewImp::drawImage()
{
   // Save matrices
   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   // Draw the view
   glViewport(0, 0, width(), height());
   glGetIntegerv(GL_VIEWPORT, mViewPort);

   glFlush();

   qglClearColor(mBackgroundColor);
   glClear(GL_COLOR_BUFFER_BIT);

   draw();

   // Restore matrices
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(projectionMatrix);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd(modelMatrix);
   glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);
}

void ViewImp::drawImage(int width, int height)
{
   // Save matrices
   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   /* not working
   updateMatrices(width, height);
   */
   // Draw the view
   glViewport(0, 0, width, height);
   glGetIntegerv(GL_VIEWPORT, mViewPort);
   glFlush();

   qglClearColor(mBackgroundColor);
   glClear(GL_COLOR_BUFFER_BIT);

   draw();

   // Restore matrices
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(projectionMatrix);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd(modelMatrix);
   glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);
}

bool ViewImp::canLinkWithView(View *pView, LinkType type)
{
   if (pView == NULL)
   {
      return false;
   }

   if (type == NO_LINK || type == MIRRORED_LINK || type == AUTOMATIC_LINK)
   {
      return true;
   }

   return false;
}

void ViewImp::snapshotSized()
{
   QImage image;
   getCurrentImage(image);
   if (image.isNull() == true)
   {
      return;
   }

   QDialog snapshotDialog(this);

   ImageResolutionWidget* pResolutionWidget = new ImageResolutionWidget(&snapshotDialog);
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(&snapshotDialog);
   pButtonBox->setOrientation(Qt::Horizontal);
   pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

   QVBoxLayout* pLayout = new QVBoxLayout(&snapshotDialog);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pResolutionWidget);
   pLayout->addStretch();
   pLayout->addWidget(pButtonBox, 0, Qt::AlignRight);

   snapshotDialog.setWindowTitle("Copy Snapshot");
   snapshotDialog.resize(200, 200);
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), &snapshotDialog, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), &snapshotDialog, SLOT(reject())));

   unsigned int iWidth = View::getSettingOutputWidth();
   unsigned int iHeight = View::getSettingOutputHeight();
   if (View::getSettingUseViewResolution())
   {
      pResolutionWidget->setResolution(image.width(), image.height());
   }
   else if (View::getSettingAspectRatioLock())
   {
      unsigned int newX = (iHeight * image.width()) / static_cast<double>(image.height());
      unsigned int newY = (iWidth * image.height()) / static_cast<double>(image.width());
      if (newX < iWidth)
      {
         if ((newX % 2) != 0)
         {
            newX++;
         }
         pResolutionWidget->setResolution(newX, iHeight);
      }
      else
      {
         if ((newY % 2) != 0)
         {
            newY++;
         }
         pResolutionWidget->setResolution(iWidth, newY);
      }
   }
   else
   {
      pResolutionWidget->setResolution(iWidth, iHeight);
   }

   if (snapshotDialog.exec() == QDialog::Accepted)
   {
      pResolutionWidget->getResolution(iWidth, iHeight);
      if ((iWidth != 0) && (iHeight != 0))
      {
         QClipboard* pClipboard = QApplication::clipboard();
         pClipboard->setImage(image.scaled(iWidth, iHeight));
      }
   }
}

void ViewImp::snapshot()
{
   QImage image;
   getCurrentImage(image);
   if (image.isNull() == true)
   {
      return;
   }

   unsigned int iWidth = View::getSettingOutputWidth();
   unsigned int iHeight = View::getSettingOutputHeight();
   if (View::getSettingUseViewResolution())
   {
      iWidth = image.width();
      iHeight = image.height();
   }
   else if (View::getSettingAspectRatioLock())
   {
      unsigned int newX = (iHeight * image.width()) / static_cast<double>(image.height());
      unsigned int newY = (iWidth * image.height()) / static_cast<double>(image.width());
      if (newX < iWidth)
      {
         if ((newX % 2) != 0)
         {
            newX++;
         }
         iWidth = newX;
      }
      else
      {
         if ((newY % 2) != 0)
         {
            newY++;
         }
         iHeight = newY;
      }
   }

   if ((iWidth != 0) && (iHeight != 0))
   {
      QClipboard* pClipboard = QApplication::clipboard();
      pClipboard->setImage(image.scaled(iWidth, iHeight));
   }
}

UndoStack* ViewImp::getUndoStack() const
{
   return mpUndoStack;
}

void ViewImp::blockUndo()
{
   mUndoBlocked = true;
}

void ViewImp::unblockUndo()
{
   mUndoBlocked = false;
}

bool ViewImp::isUndoBlocked() const
{
   return mUndoBlocked;
}

void ViewImp::startUndoGroup(const QString& text)
{
   if (isUndoBlocked() == false)
   {
      if (mpUndoGroup == NULL)
      {
         mpUndoGroup = new vector<UndoAction*>();
         mUndoGroupText = text;
      }
   }
}

void ViewImp::endUndoGroup()
{
   if (isUndoBlocked() == false)
   {
      if (mpUndoGroup != NULL)
      {
         if (mpUndoGroup->empty() == false)
         {
            mpUndoStack->beginMacro(mUndoGroupText);

            for (vector<UndoAction*>::iterator iter = mpUndoGroup->begin(); iter != mpUndoGroup->end(); ++iter)
            {
               UndoAction* pAction = *iter;
               if (pAction != NULL)
               {
                  mpUndoStack->push(pAction);
               }
            }

            mpUndoStack->endMacro();
         }

         delete mpUndoGroup;
         mpUndoGroup = NULL;

         mUndoGroupText.clear();
      }
   }
}

bool ViewImp::inUndoGroup() const
{
   return (mpUndoGroup != NULL);
}

void ViewImp::addUndoAction(UndoAction* pAction)
{
   if (pAction != NULL)
   {
      if (isUndoBlocked() == false)
      {
         VERIFYNR(connect(pAction, SIGNAL(aboutToUndo()), this, SLOT(blockUndo())));
         VERIFYNR(connect(pAction, SIGNAL(undoComplete()), this, SLOT(unblockUndo())));
         VERIFYNR(connect(pAction, SIGNAL(aboutToRedo()), this, SLOT(blockUndo())));
         VERIFYNR(connect(pAction, SIGNAL(redoComplete()), this, SLOT(unblockUndo())));

         if (mpUndoGroup != NULL)
         {
            mpUndoGroup->push_back(pAction);
         }
         else
         {
            mpUndoStack->push(pAction);
         }
      }
      else
      {
         delete pAction;
      }
   }
}

void ViewImp::clearUndo()
{
   if (mpUndoGroup != NULL)
   {
      for (vector<UndoAction*>::iterator iter = mpUndoGroup->begin(); iter != mpUndoGroup->end(); ++iter)
      {
         UndoAction* pAction = *iter;
         if (pAction != NULL)
         {
            delete pAction;
         }
      }

      mpUndoGroup->clear();
   }

   mpUndoStack->clear();
}

bool ViewImp::serialize(SessionItemSerializer &serializer) const
{
   XMLWriter xml(getObjectType().c_str());

   if (!toXml(&xml))
   {
      return false;
   }

   return serializer.serialize(xml);
}

bool ViewImp::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader xml(NULL, false);
   DOMNode* pRoot = deserializer.deserialize(xml, getObjectType().c_str());
   return fromXml(pRoot, XmlBase::VERSION);
}

bool ViewImp::toXml(XMLWriter* pXml) const
{
   pXml->addAttr("name", getName());
   pXml->addAttr("displayName", getDisplayName());
   pXml->addAttr("displayText", getDisplayText());
   pXml->addAttr("type", getObjectType());

   pXml->addAttr("visibleCenter", getVisibleCenter());
   double a;
   double b;
   double c;
   double d;
   getExtents(a, b, c, d);
   stringstream str;
   str << a << " " << b << " " << c << " " << d;
   pXml->addAttr("extents", str.str());
   pXml->addAttr("backgroundColor", mBackgroundColor.name().toStdString());
   pXml->addAttr("classificationPosition",
      StringUtilities::toXmlString<PositionType>(mClassificationPosition));
   pXml->addFontElement("classificationFont", FontImp(mClassificationFont));
   pXml->addAttr("classificationColor", mClassificationColor.name().toStdString());
   pXml->addAttr("classificationEnabled", mClassificationEnabled);
   pXml->addAttr("releaseInfoEnabled", mReleaseInfoEnabled);
   pXml->addAttr("origin", mOrigin);
   pXml->addAttr("crossHair", mCrossHair);

   if (mClassification.toXml(pXml) == false)
   {
      return false;
   }

   if (mpMouseMode != NULL)
   {
      string mouseMode;
      mpMouseMode->getName(mouseMode);
      if (mouseMode.empty() == false)
      {
         pXml->addAttr("mouseMode", mouseMode);
      }
   }

   if (mpAnimationController != NULL)
   {
      pXml->addAttr("animationControllerId", mpAnimationController->getId());
   }

   if (mLinkedViews.size() > 0)
   {
      vector<pair<View*, LinkType> >::const_iterator iter = mLinkedViews.begin();
      for (; iter!=mLinkedViews.end(); ++iter)
      {
         View* pView = iter->first;
         LinkType lType = iter->second;
         if (pView != NULL)
         {
            pXml->pushAddPoint(pXml->addElement("LinkedView"));
            pXml->addAttr("viewId", pView->getId());
            pXml->addAttr("linkType", StringUtilities::toXmlString(lType));
            pXml->popAddPoint();
         }
      }
   }

   return true;
}

bool ViewImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);

   string name(A(pElem->getAttribute(X("name"))));
   setName(name);
   string displayName(A(pElem->getAttribute(X("displayName"))));
   setDisplayName(displayName);
   string displayText(A(pElem->getAttribute(X("displayText"))));
   setDisplayText(displayText);

   double a = 0.0;
   double b = 0.0;
   double c = 1.0;
   double d = 1.0;
   XmlReader::StrToQuadCoord(pElem->getAttribute(X("extents")), a, b, c, d);
   setExtents(a, b, c, d);
   panTo(StringUtilities::fromXmlString<LocationType>(
      A(pElem->getAttribute(X("visibleCenter")))));
   QColor bkgColor(A(pElem->getAttribute(X("backgroundColor"))));
   setBackgroundColor(bkgColor);
   setClassificationPosition(StringUtilities::fromXmlString<PositionType>(
      A(pElem->getAttribute(X("classificationPosition")))));
   FontImp classificationFont;
   readFontElement("classificationFont", pElem, classificationFont);
   setClassificationFont(classificationFont.getQFont());
   QColor classificationColor(A(pElem->getAttribute(X("classificationColor"))));
   setClassificationColor(classificationColor);
   enableClassification(StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("classificationEnabled")))));
   enableReleaseInfo(StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("releaseInfoEnabled")))));
   setDataOrigin(StringUtilities::fromXmlString<DataOrigin>(
      A(pElem->getAttribute(X("origin")))));
   enableCrossHair(StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("crossHair")))));

   if (pElem->hasAttribute(X("mouseMode")))
   {
      string mouseMode = A(pElem->getAttribute(X("mouseMode")));
      setMouseMode(QString::fromStdString(mouseMode));
   }

   if (pElem->hasAttribute(X("animationControllerId")))
   {
      string controllerId(A(pElem->getAttribute(X("animationControllerId"))));
      SessionItem* pItem = Service<SessionManager>()->getSessionItem(controllerId);
      AnimationController* pController = dynamic_cast<AnimationController*>(pItem);
      if (pController != NULL)
      {
         setAnimationController(pController);
      }
   }

   for (DOMNode* pNode = pDocument->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(), X("classification")))
      {
         if (mClassification.fromXml(pNode, version) == false)
         {
            return false;
         }
      }
   }

   // re-establish view links
   for (DOMNode* pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      string name = A(pChld->getNodeName());
      if (name == "LinkedView")
      {
         pElem = static_cast<DOMElement*>(pChld);
         if (pElem != NULL)
         {
            string id = A(pElem->getAttribute(X("viewId")));
            View* pView = dynamic_cast<View*>(Service<SessionManager>()->getSessionItem(id));
            LinkType lType = StringUtilities::fromXmlString<LinkType>(A(pElem->getAttribute(X("linkType"))));
            if (pView != NULL && lType.isValid())
            {
               linkView(pView, lType);
            }
         }
      }
   }

   return true;
}

void ViewImp::toggleMousePanByKey()
{
   if (isMousePanEnabled() == false)
   {
      QPoint globalPos = QCursor::pos();
      QPoint widgetPos = mapFromGlobal(globalPos);
      if (widgetPos.x() < 0 || widgetPos.x() > width() || widgetPos.y() < 0 || widgetPos.y() > height())
      {
         //since mouse pan was toggled on using keyboard
         //while mouse was outside the view, we can't properly
         //drawn the anchor point, so do nothing
         //i.e. leave the mouse pan turned off.
         return;
      }

      setMousePanPos(globalPos);
   }

   enableMousePan(!isMousePanEnabled());
}

double ViewImp::getMousePanScaleFactor() const
{
   double magicFactor = 0.05;    // Determined by trial and error
   double scaleFactor = magicFactor * (View::getSettingMousePanSensitivity() / 100.0);
   return scaleFactor;
}

void ViewImp::mousePanTimeout()
{
   if (abs(mMousePanAnchor.x() - mMousePanCurrentPos.x()) < 6 &&
      abs(mMousePanAnchor.y() - mMousePanCurrentPos.y()) < 6)
   {
      //create dead space based upon screen pixels
      return;
   }

   ViewImp* pView = this;
   ViewImp* pProductView = dynamic_cast<ViewImp*>(parentWidget());
   if (pProductView != NULL)
   {
      pView = pProductView;
   }

   double anchorX;
   double anchorY;
   double currentX;
   double currentY;
   translateScreenToWorld(mMousePanAnchor.x(), mMousePanAnchor.y(), anchorX, anchorY);
   translateScreenToWorld(mMousePanCurrentPos.x(), mMousePanCurrentPos.y(), currentX, currentY);

   double scaleFactor = getMousePanScaleFactor();
   double deltaX = (anchorX - currentX) * scaleFactor * -1;
   double deltaY = (anchorY - currentY) * scaleFactor;

   panBy(deltaX, deltaY);
   pView->refresh();
}

PositionType ViewImp::getClassificationPosition() const
{
   return mClassificationPosition;
}

void ViewImp::setClassificationPosition(PositionType position)
{
   if (position == mClassificationPosition)
   {
      return;
   }

   mClassificationPosition = position;
   emit classificationPositionChanged(position);
   notify(SIGNAL_NAME(Subject, Modified), position);
}
