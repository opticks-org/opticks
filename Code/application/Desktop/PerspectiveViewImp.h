/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PERSPECTIVEVIEWIMP_H
#define PERSPECTIVEVIEWIMP_H

#include "ViewImp.h"

class PerspectiveViewImp : public ViewImp
{
   Q_OBJECT

public:
   PerspectiveViewImp(const std::string& id, const std::string& viewName, QGLContext* drawContext = 0,
      QWidget* parent = 0);
   ~PerspectiveViewImp();

   using SessionItemImp::setIcon;

   static bool isKindOfView(const std::string& className);
   static void getViewTypes(std::vector<std::string>& classList);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   PerspectiveViewImp& operator= (const PerspectiveViewImp& perspectiveView);
   using ViewImp::setName;

   double getZoomPercentage() const;
   double getRotation() const;
   double getPitch() const;
   
   double getPixelAspect() const;
   void setPixelAspect(double aspect);

   void setAllowZoomOnResize(bool allow);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void zoomExtents();
   void zoomBy(double dPercent);
   virtual void zoomTo(double dPercent);
   void zoomAboutPoint(const QPoint& screenCoord, double dPercent);
   void zoomAboutPoint(const LocationType& worldCoord, double dPercent);
   void zoomToPoint(const QPoint& screenCoord, double dPercent);
   void zoomToPoint(const LocationType& worldCoord, double dPercent);
   void zoomToCenter(double dPercent);
   virtual void zoomToBox(const LocationType& worldLowerLeft, const LocationType& worldUpperRight);
   void zoomToInset();
   virtual void pan(const LocationType& worldBegin, const LocationType& worldEnd);
   void rotateBy(double dDegrees);
   virtual void rotateTo(double dDegrees);
   void flipBy(double dDegrees);
   virtual void flipTo(double dDegrees);
   void flipHorizontal();
   void flipVertical();
   void resetZoom();
   void resetOrientation();
   void reset();
   void toggleShowCoordinates();

signals:
   void zoomChanged(double dPercent);
   void rotationChanged(double dAngle);
   void pitchChanged(double dPitch);

protected:
   virtual void updateStatusBar(const QPoint& screenCoord);

   void resizeEvent(QResizeEvent* e);
   void keyPressEvent(QKeyEvent* e);
   void mousePressEvent(QMouseEvent* e);
   void mouseMoveEvent(QMouseEvent* e);
   void mouseReleaseEvent(QMouseEvent* e);
   void contextMenuEvent(QContextMenuEvent* pEvent);
   void wheelEvent(QWheelEvent* e);
   void leaveEvent(QEvent* e);

   virtual double limitZoomPercentage(double dPercent);
   virtual LocationType limitPanCenter(LocationType center);
   using ViewImp::updateMatrices;
   void updateMatrices(int width, int height);
   void drawInset();

protected:
   static unsigned int sKeyboardNumber;

private:
   typedef struct
   {
      float x;
      float y;
      float z;
      float heading;
      float pitch;
      float roll;
   } PointType;

   double mDist;           // distance from centerpoint of view display
   double mFullDistance;
   LocationType mCenter;        // centerpoint of view display
   double mHeading;
   double mPitch;
   double mFov;
   double mFrontPlane;     // front clipping plane
   double mBackPlane;      // back clipping plane
   double mPixelAspect; 
   Qt::KeyboardModifiers mCurrentModifiers;
   bool mDisplayContextMenu;
   bool mAllowZoomOnResize;
};

#define PERSPECTIVEVIEWADAPTEREXTENSION_CLASSES \
   VIEWADAPTEREXTENSION_CLASSES

#define PERSPECTIVEVIEWADAPTER_METHODS(impClass) \
   VIEWADAPTER_METHODS(impClass) \
   void zoomBy(double dPercent) \
   { \
      return impClass::zoomBy(dPercent); \
   } \
   void zoomTo(double dPercent) \
   { \
      return impClass::zoomTo(dPercent); \
   } \
   void zoomAboutPoint(const LocationType& worldCoord, double dPercent) \
   { \
      return impClass::zoomAboutPoint(worldCoord, dPercent); \
   } \
   void zoomToPoint(const LocationType& worldCoord, double dPercent) \
   { \
      return impClass::zoomToPoint(worldCoord, dPercent); \
   } \
   void zoomToCenter(double dPercent) \
   { \
      return impClass::zoomToCenter(dPercent); \
   } \
   double getZoomPercentage() const \
   { \
      return impClass::getZoomPercentage(); \
   } \
   void rotateBy(double dDegrees) \
   { \
      return impClass::rotateBy(dDegrees); \
   } \
   void rotateTo(double dDegrees) \
   { \
      return impClass::rotateTo(dDegrees); \
   } \
   double getRotation() const \
   { \
      return impClass::getRotation(); \
   } \
   void flipBy(double dDegrees) \
   { \
      return impClass::flipBy(dDegrees); \
   } \
   void flipTo(double dDegrees) \
   { \
      return impClass::flipTo(dDegrees); \
   } \
   void flipHorizontal() \
   { \
      return impClass::flipHorizontal(); \
   } \
   void flipVertical() \
   { \
      return impClass::flipVertical(); \
   } \
   double getPitch() const \
   { \
      return impClass::getPitch(); \
   } \
   void resetZoom() \
   { \
      return impClass::resetZoom(); \
   } \
   void resetOrientation() \
   { \
      return impClass::resetOrientation(); \
   } \
   void reset() \
   { \
      return impClass::reset(); \
   } \
   void toggleShowCoordinates() \
   { \
      return impClass::toggleShowCoordinates(); \
   } \
   double getPixelAspect() const \
   { \
      return impClass::getPixelAspect(); \
   } \
   void setPixelAspect(double aspect) \
   { \
      return impClass::setPixelAspect(aspect); \
   }

#endif
