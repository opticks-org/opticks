/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ORTHOGRAPHICVIEWIMP_H
#define ORTHOGRAPHICVIEWIMP_H

#include "ViewImp.h"

#include <string>

class OrthographicViewImp : public ViewImp
{
   Q_OBJECT

public:
   OrthographicViewImp(const std::string& id, const std::string& viewName, QGLContext* drawContext = 0,
      QWidget* parent = 0);
   ~OrthographicViewImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfView(const std::string& className);
   static void getViewTypes(std::vector<std::string>& classList);

   OrthographicViewImp& operator= (const OrthographicViewImp& orthographicView);
   using ViewImp::setName;

   void lockAspectRatio(bool bLock);
   bool isAspectRatioLocked() const;

   LocationType getPixelSize() const;
   using ViewImp::getPixelSize;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void zoomExtents();
   void zoomToBox(const LocationType& worldLowerLeft, const LocationType& worldUpperRight);
   void zoomOnPoint(const QPoint& anchor, const QPoint& delta);
   void zoomToInset();
   void pan(const LocationType& worldBegin, const LocationType& worldEnd);

protected:
   void resizeEvent(QResizeEvent* e);
   using ViewImp::updateMatrices;
   void updateMatrices(int width, int height);
   void drawInset();

private:
   double mDisplayMinX;
   double mDisplayMinY;
   double mDisplayMaxX;
   double mDisplayMaxY;
   bool mLockRatio;
};

#define ORTHOGRAPHICVIEWADAPTEREXTENSION_CLASSES \
   VIEWADAPTEREXTENSION_CLASSES

#define ORTHOGRAPHICVIEWADAPTER_METHODS(impClass) \
   VIEWADAPTER_METHODS(impClass) \
   void lockAspectRatio(bool bLock) \
   { \
      return impClass::lockAspectRatio(bLock); \
   } \
   bool isAspectRatioLocked() const \
   { \
      return impClass::isAspectRatioLocked(); \
   } \
   LocationType getPixelSize() const \
   { \
      return impClass::getPixelSize(); \
   }

#endif
