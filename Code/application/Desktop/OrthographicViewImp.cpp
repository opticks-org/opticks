/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtGui/QResizeEvent>

#include "GlContextSave.h"
#include "OrthographicView.h"
#include "OrthographicViewImp.h"
#include "xmlreader.h"

#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

OrthographicViewImp::OrthographicViewImp(const string& id, const string& viewName, QGLContext* drawContext,
                                         QWidget* parent) :
   ViewImp(id, viewName, drawContext, parent),
   mDisplayMinX(0.0),
   mDisplayMinY(0.0),
   mDisplayMaxX(1.0),
   mDisplayMaxY(1.0),
   mLockRatio(false)
{
}

OrthographicViewImp::~OrthographicViewImp()
{
}

const string& OrthographicViewImp::getObjectType() const
{
   static string type("OrthographicViewImp");
   return type;
}

bool OrthographicViewImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "OrthographicView"))
   {
      return true;
   }

   return ViewImp::isKindOf(className);
}

bool OrthographicViewImp::isKindOfView(const string& className)
{
   if ((className == "OrthographicViewImp") || (className == "OrthographicView"))
   {
      return true;
   }

   return ViewImp::isKindOfView(className);
}

void OrthographicViewImp::getViewTypes(vector<string>& classList)
{
   classList.push_back("OrthographicView");
   ViewImp::getViewTypes(classList);
}

OrthographicViewImp& OrthographicViewImp::operator= (const OrthographicViewImp& orthographicView)
{
   if (this != &orthographicView)
   {
      ViewImp::operator= (orthographicView);

      mDisplayMinX = orthographicView.mDisplayMinX;
      mDisplayMinY = orthographicView.mDisplayMinY;
      mDisplayMaxX = orthographicView.mDisplayMaxX;
      mDisplayMaxY = orthographicView.mDisplayMaxY;
      mLockRatio = orthographicView.mLockRatio;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

void OrthographicViewImp::lockAspectRatio(bool bLock)
{
   mLockRatio = bLock;
}

bool OrthographicViewImp::isAspectRatioLocked() const
{
   return mLockRatio;
}

inline double magdiff2(const LocationType& temp1, const LocationType& temp2)
{
   double tempX = temp1.mX - temp2.mX; 
   double tempY = temp1.mY - temp2.mY;
   return tempX * tempX + tempY * tempY;
}

LocationType OrthographicViewImp::getPixelSize() const
{
   LocationType pixelSize;
   LocationType temp1, temp2;

   translateWorldToScreen(0.0, 0.0, temp1.mX, temp1.mY);
   translateWorldToScreen(1.0, 0.0, temp2.mX, temp2.mY);
   pixelSize.mX = sqrt(magdiff2(temp1, temp2));

   translateWorldToScreen(0.0, 1.0, temp2.mX, temp2.mY);
   pixelSize.mY = sqrt(magdiff2(temp1, temp2));

   return pixelSize;
}

void OrthographicViewImp::zoomExtents()
{
   // Get the extents
   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;
   getExtents(dMinX, dMinY, dMaxX, dMaxY);

   // Update the display extents
   mDisplayMinX = dMinX;
   mDisplayMaxX = dMaxX;
   mDisplayMinY = dMinY;
   mDisplayMaxY = dMaxY;

   // Adjust the display extents for the aspect ratio
   if (mLockRatio == true)
   {
      double xRatio = width() / fabs(dMaxX - dMinX);
      double yRatio = height() / fabs(dMaxY - dMinY);

      if (yRatio > xRatio)
      {
         double newDisplayHeight = height() / xRatio;
         mDisplayMinY = (dMinY + dMaxY - newDisplayHeight) / 2.0;
         mDisplayMaxY = (dMinY + dMaxY + newDisplayHeight) / 2.0;
      }
      else if (xRatio > yRatio)
      {
         double newDisplayWidth = width() / yRatio;
         mDisplayMinX = (dMinX + dMaxX - newDisplayWidth) / 2.0;
         mDisplayMaxX = (dMinX + dMaxX + newDisplayWidth) / 2.0;
      }
   }

   // Update the matrices
   updateMatrices();
}

void OrthographicViewImp::zoomToBox(const LocationType& worldLowerLeft, const LocationType& worldUpperRight)
{
   // Update the display extents
   double dBeginX = worldLowerLeft.mX;
   double dBeginY = worldLowerLeft.mY;
   double dEndX = worldUpperRight.mX;
   double dEndY = worldUpperRight.mY;

   if (dEndX < dBeginX)
   {
      double dTemp = dEndX;
      dEndX = dBeginX;
      dBeginX = dTemp;
   }

   if (dEndY < dBeginY)
   {
      double dTemp = dEndY;
      dEndY = dBeginY;
      dBeginY = dTemp;
   }

   mDisplayMinX = dBeginX;
   mDisplayMaxX = dEndX;
   mDisplayMinY = dBeginY;
   mDisplayMaxY = dEndY;

   // Adjust the display extents for the aspect ratio
   if (mLockRatio == true)
   {
      double xRatio = width() / fabs(dEndX - dBeginX);
      double yRatio = height() / fabs(dEndY - dBeginY);

      if (yRatio > xRatio)
      {
         double newDisplayHeight = height() / xRatio;
         mDisplayMinY = (dBeginY + dEndY - newDisplayHeight) / 2.0;
         mDisplayMaxY = (dBeginY + dEndY + newDisplayHeight) / 2.0;
      }
      else if (xRatio > yRatio)
      {
         double newDisplayWidth = width() / yRatio;
         mDisplayMinX = (dBeginX + dEndX - newDisplayWidth) / 2.0;
         mDisplayMaxX = (dBeginX + dEndX + newDisplayWidth) / 2.0;
      }
   }

   // Update the matrices
   updateMatrices();
}

void OrthographicViewImp::zoomOnPoint(const QPoint& anchor, const QPoint& delta)
{
   // Update the display extents
   const double zoomRatio = 0.01;
   double deltaX = delta.x();
   double deltaY = delta.y();

   if (mLockRatio == true)
   {
      double dist = sqrt(pow(deltaX, 2) + pow(deltaY, 2));

      double dSign = 1.0;
      if (fabs(deltaX) > fabs(deltaY))
      {
         if (deltaX < 0.0)
         {
            dSign = -1.0;
         }
      }
      else if (fabs(deltaY) > fabs(deltaX))
      {
         if (deltaY < 0.0)
         {
            dSign = -1.0;
         }
      }

      deltaX = dist * dSign;
      deltaY = dist * dSign;
   }

   double xZoom = zoomRatio * deltaX;
   double yZoom = zoomRatio * deltaY;

   double xFrac = (double) anchor.x() / (double) width();
   double yFrac = (double) anchor.y() / (double) height();

   double xDelta = mDisplayMaxX - mDisplayMinX;
   double yDelta = mDisplayMaxY - mDisplayMinY;

   double xPos = xDelta * xFrac + mDisplayMinX;
   double yPos = yDelta * yFrac + mDisplayMinY;

   mDisplayMinX = mDisplayMinX - (xPos - mDisplayMinX) * xZoom;
   mDisplayMinY = mDisplayMinY - (yPos - mDisplayMinY) * yZoom;
   mDisplayMaxX = mDisplayMaxX + (mDisplayMaxX - xPos) * xZoom;
   mDisplayMaxY = mDisplayMaxY + (mDisplayMaxY - yPos) * yZoom;

   // Adjust the display extents for the aspect ratio
   if (mLockRatio == true)
   {
      double dMinX = mDisplayMinX;
      double dMinY = mDisplayMinY;
      double dMaxX = mDisplayMaxX;
      double dMaxY = mDisplayMaxY;

      double xRatio = width() / fabs(dMaxX - dMinX);
      double yRatio = height() / fabs(dMaxY - dMinY);

      if (yRatio > xRatio)
      {
         double newDisplayHeight = height() / xRatio;
         mDisplayMinY = (dMinY + dMaxY - newDisplayHeight) / 2.0;
         mDisplayMaxY = (dMinY + dMaxY + newDisplayHeight) / 2.0;
      }
      else if (xRatio > yRatio)
      {
         double newDisplayWidth = width() / yRatio;
         mDisplayMinX = (dMinX + dMaxX - newDisplayWidth) / 2.0;
         mDisplayMaxX = (dMinX + dMaxX + newDisplayWidth) / 2.0;
      }
   }

   // Update the matrices
   updateMatrices();
}

void OrthographicViewImp::zoomToInset()
{
   // TODO: Zoom to the inset zoom level
}

void OrthographicViewImp::pan(const LocationType& worldBegin, const LocationType& worldEnd)
{
   // Update the display extents
   mDisplayMinX += (worldEnd.mX - worldBegin.mX);
   mDisplayMaxX += (worldEnd.mX - worldBegin.mX);
   mDisplayMinY += (worldEnd.mY - worldBegin.mY);
   mDisplayMaxY += (worldEnd.mY - worldBegin.mY);

   // Update the matrices
   updateMatrices();
}

void OrthographicViewImp::resizeEvent(QResizeEvent *e)
{
   // Do not call the base class method since it now does nothing

   if (e == NULL)
   {
      return;
   }

   if ((width() <= 0) || (height() <= 0))
   {
      return;
   }

   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;
   getExtents(dMinX, dMinY, dMaxX, dMaxY);

   if ((dMaxX == dMinX) || (dMaxY == dMinY))
   {
      return;
   }

   // Adjust the display extents for the aspect ratio
   if (mLockRatio == true)
   {
      double dMinX = mDisplayMinX;
      double dMinY = mDisplayMinY;
      double dMaxX = mDisplayMaxX;
      double dMaxY = mDisplayMaxY;

      double xRatio = width() / fabs(dMaxX - dMinX);
      double yRatio = height() / fabs(dMaxY - dMinY);

      int iDeltaHeight = abs(e->oldSize().height() - height());
      int iDeltaWidth = abs(e->oldSize().width() - width());

      if (iDeltaHeight > iDeltaWidth)
      {
         double newDisplayHeight = height() / xRatio;
         mDisplayMinY = (dMinY + dMaxY - newDisplayHeight) / 2.0;
         mDisplayMaxY = (dMinY + dMaxY + newDisplayHeight) / 2.0;
      }
      else if (iDeltaWidth > iDeltaHeight)
      {
         double newDisplayWidth = width() / yRatio;
         mDisplayMinX = (dMinX + dMaxX - newDisplayWidth) / 2.0;
         mDisplayMaxX = (dMinX + dMaxX + newDisplayWidth) / 2.0;
      }
   }

   // Update the matrices
   updateMatrices();
}

void OrthographicViewImp::updateMatrices(int width, int height)
{
   if ((width <= 0) || (height <= 0))
   {
      return;
   }

   { // save the context for a time
      GlContextSave contextSave;
      makeCurrent();

      // Save current matrices
      double lmodelMatrix[16], lprojectionMatrix[16];
      int lviewPort[4];
      glGetIntegerv(GL_VIEWPORT, lviewPort);
      glGetDoublev(GL_PROJECTION_MATRIX, lprojectionMatrix);
      glGetDoublev(GL_MODELVIEW_MATRIX, lmodelMatrix);

      // Viewport
      glViewport(0, 0, width, height);

      // Projection matrix
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluOrtho2D(mDisplayMinX, mDisplayMaxX, mDisplayMinY, mDisplayMaxY);

      // Model matrix
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      // Update the member matrix values
      glGetIntegerv(GL_VIEWPORT, mViewPort);
      glGetDoublev(GL_MODELVIEW_MATRIX, mModelMatrix);
      glGetDoublev(GL_PROJECTION_MATRIX, mProjMatrix);

      // Restore current matrices
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixd(lprojectionMatrix);
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixd(lmodelMatrix);
      glViewport(lviewPort[0], lviewPort[1], lviewPort[2], lviewPort[3]);
   }

   emit displayAreaChanged();
   notify(SIGNAL_NAME(OrthographicView, DisplayAreaChanged), boost::any());
}

void OrthographicViewImp::drawInset()
{
   // TODO: Draw the inset
}

bool OrthographicViewImp::toXml(XMLWriter* pXml) const
{
   if (!ViewImp::toXml(pXml))
   {
      return false;
   }
   stringstream buf;
   buf << mDisplayMinX << " " << mDisplayMinY;
   pXml->addAttr("displayMin", buf.str().c_str());
   buf.str("");
   buf << mDisplayMaxX << " " << mDisplayMaxY;
   pXml->addAttr("displayMax", buf.str().c_str());
   pXml->addAttr("lockRatio", mLockRatio);
   return true;
}

bool OrthographicViewImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (!ViewImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement *pElem = static_cast<DOMElement*>(pDocument);
   if (pElem == NULL)
   {
      return false;
   }

   LocationType locXY;
   if (XmlReader::StrToLocation(pElem->getAttribute(X("displayMin")), locXY))
   {
      mDisplayMinX = locXY.mX;
      mDisplayMinY = locXY.mY;
   }

   if (XmlReader::StrToLocation(pElem->getAttribute(X("displayMax")), locXY))
   {
      mDisplayMaxX = locXY.mX;
      mDisplayMaxY = locXY.mY;
   }

   mLockRatio = StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("lockRatio"))));

   updateMatrices();
   return true;
}

