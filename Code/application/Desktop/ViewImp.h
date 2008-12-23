/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VIEWIMP_H
#define VIEWIMP_H

#include "glCommon.h"

#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QImage>
#include <QtGui/QPolygon>
#include <QtOpenGL/QGLContext>
#include <QtOpenGL/QGLWidget>

#include "LocationType.h"
#include "Resource.h"
#include "SessionItemImp.h"
#include "Subject.h"
#include "SubjectImp.h"
#include "TypesFile.h"
#include "View.h"
#include "XercesIncludes.h"
#include "xmlwriter.h"

#include <map>
#include <vector>

class AnimationController;
class Classification;
class MouseMode;
class SessionItemDeserializer;
class SessionItemSerializer;
class UndoAction;
class UndoStack;

class ViewImp : public QGLWidget, public SessionItemImp, public SubjectImp
{
   Q_OBJECT

public:
   class SubImageIteratorImp : public View::SubImageIterator
   {
   public:
      SubImageIteratorImp(ViewImp *pView, const QSize &totalSize, const QSize &subImageSize);
      ~SubImageIteratorImp();
      bool hasNext() const;
      bool next(QImage &image);
      void location(int &x, int &y) const;
      void count(int &x, int &y) const;

   private:
      ViewImp* mpView;
      QSize mTotalSize;
      QSize mSubImageSize;

      int mTotalTilesX;
      int mTotalTilesY;
      int mCurrentTileX;
      int mCurrentTileY;

      LocationType mStartLowerLeft;
      LocationType mRestoreLowerLeft;
      QSize mRestoreViewSize;
      LocationType mRestoreUpperRight;
   };
   friend class SubImageIteratorImp;

   ViewImp(const std::string& id, const std::string& viewName, QGLContext* drawContext = 0, QWidget* parent = 0);
   ~ViewImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void viewDeleted(Subject &subject, const std::string &signal, const boost::any &v);
   void animationControllerDeleted(Subject &subject, const std::string &signal, const boost::any &v);

   static bool isKindOfView(const std::string& className);
   static void getViewTypes(std::vector<std::string>& classList);

   virtual ViewImp& operator= (const ViewImp& view);
   virtual View* copy(QGLContext* drawContext = 0, QWidget* parent = 0) const = 0;
   virtual bool copy(View *pView) const = 0;

   void setName(const std::string& viewName);
   virtual ViewType getViewType() const = 0;

   void setClassification(const Classification* pClassification);
   QString getClassificationText() const;
   QFont getClassificationFont() const;
   QColor getClassificationColor() const;

   QColor getBackgroundColor() const;
   DataOrigin getDataOrigin() const;

   AnimationController *getAnimationController() const;
   void setAnimationController(AnimationController *pPlayer);

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);
   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);

   bool addMouseMode(const MouseMode* pMouseMode);
   bool removeMouseMode(const MouseMode* pMouseMode);
   bool deleteMouseMode(const MouseMode* pMouseMode);
   bool setMouseMode(const MouseMode* pMouseMode);
   bool setMouseMode(const QString& strModeName);
   bool containsMouseMode(const MouseMode* pMouseMode) const;
   const MouseMode* getMouseMode(const QString& strModeName) const;
   const MouseMode* getCurrentMouseMode() const;
   std::vector<const MouseMode*> getMouseModes() const;
   unsigned int getNumMouseModes() const;
   void enableMouseMode(const MouseMode* pMouseMode, bool bEnable);
   bool isMouseModeEnabled(const MouseMode* pMouseMode) const;

   void getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY) const;
   void getVisibleCorners(LocationType& lowerLeft, LocationType& upperLeft, LocationType& upperRight,
      LocationType& lowerRight) const;
   LocationType getVisibleCenter() const;
   void translateWorldToScreen(double dWorldX, double dWorldY, double& dScreenX, double& dScreenY,
      bool* pVisible = NULL) const;
   void translateScreenToWorld(double dScreenX, double dScreenY, double& dWorldX, double& dWorldY) const;
   double getPixelSize(const LocationType& lowerLeft, const LocationType& upperRight) const;

   bool isInsetEnabled() const;
   LocationType getInsetLocation() const;
   bool isCrossHairEnabled() const;

   virtual void draw();
   void renderText(int screenCoordX, int screenCoordY, const QString& strText, const QFont& fnt = QFont());
   QImage getCurrentImage();
   bool getCurrentImage(QImage &image);
   View::SubImageIterator *getSubImageIterator(const QSize &totalSize, const QSize &subImageSize);

   bool linkView(View* pView, LinkType type);
   const std::vector<std::pair<View*, LinkType> > &getLinkedViews() const;
   LinkType getViewLinkType(View* pView) const;
   bool unlinkView(View* pView);

   void setSelectionBox(const std::vector<LocationType>& selectionBox);
   void setSelectionBox(const LocationType& worldLowerLeft, const LocationType& worldUpperRight);
   void setSelectionBox(const QPolygon& selectionBox);
   void setSelectionBox(const QPoint& screenLowerLeft, const QPoint& screenUpperRight);
   void setSelectionBox(const QRect& rcBox);
   const std::vector<LocationType>& getSelectionBox() const;

   static void reorderImage(unsigned int pImage[], int iWidth, int iHeight);

   /**
    * Determine whether this view can be linked with the specified type.
    *
    * @param pView
    *        View with which the link is requested.
    * @param type
    *        The type of link requested.
    * @return True if the link can be established, false otherwise.
    */
   virtual bool canLinkWithView(View *pView, LinkType type);

   UndoStack* getUndoStack() const;
   bool isUndoBlocked() const;
   void startUndoGroup(const QString& text);
   void endUndoGroup();
   bool inUndoGroup() const;
   void addUndoAction(UndoAction* pAction);
   void clearUndo();

   SIGNAL_METHOD(ViewImp, AnimationControllerChanged)

public slots:
   virtual void setClassificationText(const QString& strClassification);
   virtual void setClassificationFont(const QFont& classificationFont);
   virtual void setClassificationColor(const QColor& clrClassification);
   virtual void enableClassification(bool bEnable);
   virtual void enableReleaseInfo(bool bEnable);
   virtual void setBackgroundColor(const QColor& clrBackground);
   virtual void setDataOrigin(const DataOrigin& dataOrigin);
   virtual bool setExtents(double dMinX, double dMinY, double dMaxX, double dMaxY);
   virtual void zoomExtents() = 0;
   virtual void zoomToBox(const QPoint& screenLowerLeft, const QPoint& screenUpperRight);
   virtual void zoomToBox(const LocationType& worldLowerLeft, const LocationType& worldUpperRight) = 0;
   virtual void zoomToInset() = 0;
   virtual void zoomInset(bool in);
   virtual void zoomInsetTo(unsigned int zoomMultiplier);
   virtual void pan(const QPoint& screenBegin, const QPoint& screenEnd);
   virtual void pan(const LocationType& worldBegin, const LocationType& worldEnd) = 0;
   virtual void panTo(const QPoint& screenCoord);
   virtual void panTo(const LocationType& worldCoord);
   virtual void panToCenter();
   virtual void panBy(int iScreenDeltaX, int iScreenDeltaY);
   virtual void panBy(double dWorldDeltaX, double dWorldDeltaY);
   virtual bool enableInset(bool bEnable);
   virtual void setInsetPoint(const QPoint& screenCoord);
   virtual void setInsetPoint(const LocationType &worldCoord);
   virtual bool enableCrossHair(bool bEnable);
   virtual void refresh();

   void blockUndo();
   void unblockUndo();

signals:
   void renamed(const QString& strViewName);
   void classificationChanged(const QString& strClassification);
   void classificationFontChanged(const QFont& classificationFont);
   void classificationColorChanged(const QColor& clrClassification);
   void backgroundColorChanged(const QColor& clrBackground);
   void originChanged(const DataOrigin& dataOrigin);
   void crossHairDisplayed(bool bDisplayed);
   void mouseModeAdded(const MouseMode* pMouseMode);
   void mouseModeRemoved(const MouseMode* pMouseMode);
   void mouseModeChanged(const MouseMode* pMouseMode);
   void mouseModeEnabled(const MouseMode* pMouseMode, bool bEnable);
   void extentsChanged(double dMinX, double dMinY, double dMaxX, double dMaxY);
   void selectionBoxChanged(const std::vector<LocationType>& selectionBox);
   void displayAreaChanged();

protected:
   bool event(QEvent* pEvent);
   void contextMenuEvent(QContextMenuEvent* pEvent);
   void initializeGL();
   void paintGL();

   void setupScreenMatrices();
   void setupWorldMatrices();
   virtual void updateMatrices();
   virtual void updateMatrices(int width, int height) = 0;

   virtual void drawContents() = 0;
   virtual void drawCrossHair();
   virtual void drawSelectionBox();
   virtual void drawInset() = 0;
   virtual void drawClassification();

   template<typename V, typename T>
   void executeOnLinks(T &functor)
   {
      executeOnLinks<V, V>(functor, functor);
   }

   template<typename V, typename T1, typename T2>
   void executeOnLinks(T1 &mirrorFunctor, T2 &geoFunctor)
   {
      executeOnLinks<V, V>(mirrorFunctor, geoFunctor);
   }

   template<typename VMirror, typename VGeo, typename T1, typename T2>
   void executeOnLinks(T1 &mirrorFunctor, T2 &geoFunctor)
   {
      if (mbLinking)
      {
         return;
      }

      ResetVariableOnDestroy<bool> setter(mbLinking, true);

      for (std::vector<std::pair<View*, LinkType> >::const_iterator iter = getLinkedViews().begin();
         iter != getLinkedViews().end(); ++iter)
      {
         View *pView = iter->first;
         if (pView != NULL)
         {
            ViewImp *pViewImp = dynamic_cast<ViewImp*>(pView);
            if (pViewImp != NULL)
            {
               if (pViewImp->mbLinking)
               {
                  continue;
               }
            }

            VMirror *pViewMirror = NULL;
            VGeo *pViewGeo = NULL;
            LinkType type = iter->second;

            switch (iter->second)
            {
               case MIRRORED_LINK:
                  pViewMirror = dynamic_cast<VMirror*>(pView);
                  if (pViewMirror != NULL)
                  {
                     mirrorFunctor(pViewMirror);
                  }
                  break;
               case GEOCOORD_LINK:
                  pViewGeo = dynamic_cast<VGeo*>(pView);
                  if (pViewGeo != NULL)
                  {
                     geoFunctor(pViewGeo);
                  }
                  break;
               case AUTOMATIC_LINK:
                  if (canLinkWithView(pView, GEOCOORD_LINK))
                  {
                     pViewGeo = dynamic_cast<VGeo*>(pView);
                     if (pViewGeo != NULL)
                     {
                        geoFunctor(pViewGeo);
                     }
                  }
                  else if (canLinkWithView(pView, MIRRORED_LINK))
                  {
                     pViewMirror = dynamic_cast<VMirror*>(pView);
                     if (pViewMirror != NULL)
                     {
                        mirrorFunctor(pViewMirror);
                     }
                  }
                  break;
               default:
                  break;
            };
            pView->refresh();
         }
      }
   }

protected:
   GLdouble mModelMatrix[16];
   GLdouble mProjMatrix[16];
   GLint mViewPort[4];

   bool mbLinking;

   QPoint mMouseStart;
   QPoint mMouseCurrent;
   QPoint mMouseEnd;

private:
   static const QGLWidget* mpShareWidget;

   QColor mBackgroundColor;
   QString mClassificationText;
   QFont mClassificationFont;
   QColor mClassificationColor;
   bool mClassificationEnabled;
   bool mReleaseInfoEnabled;
   DataOrigin mOrigin;
   const MouseMode* mpMouseMode;
   std::map<const MouseMode*, bool> mMouseModes;
   double mMinX;
   double mMinY;
   double mMaxX;
   double mMaxY;
   LocationType mInsetLocation;
   std::vector<LocationType> mSelectionBox;
   bool mInset;
   bool mCrossHair;
   std::vector<std::pair<View*, LinkType> > mLinkedViews;
   AnimationController* mpAnimationController;
   UndoStack* mpUndoStack;
   bool mUndoBlocked;
   std::vector<UndoAction*>* mpUndoGroup;
   QString mUndoGroupText;

   View::SubImageIterator* mpSubImageIterator;

   class ViewContext : public QGLContext
   {
   public:
      ViewContext(QGLContext* pDrawContext, const QGLFormat& format) :
         QGLContext(format),
         mpDrawContext(pDrawContext)
      {
      }

      void makeCurrent()
      {
         if (mpDrawContext != NULL)
         {
            mpDrawContext->makeCurrent();
         }
         else
         {
            QGLContext::makeCurrent();
         }
      }

   private:
      QGLContext* mpDrawContext;
   };

private:
   static const QGLWidget* getShareWidget();
   void drawImage();
   void drawImage(int width, int height);
};

#define VIEWADAPTEREXTENSION_CLASSES \
   SESSIONITEMADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES

#define VIEWADAPTER_METHODS(impClass) \
   SESSIONITEMADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   void setName(const std::string& viewName) \
   { \
      impClass::setName(viewName); \
   } \
   ViewType getViewType() const \
   { \
      return impClass::getViewType(); \
   } \
   QWidget* getWidget() const \
   { \
      return const_cast<QWidget*>(static_cast<const QWidget*>(this)); \
   } \
   void setBackgroundColor(const ColorType& backgroundColor) \
   { \
      QColor clrBackground; \
      if (backgroundColor.isValid() == true) \
      { \
         clrBackground.setRgb(backgroundColor.mRed, backgroundColor.mGreen, backgroundColor.mBlue); \
      } \
      \
      impClass::setBackgroundColor(clrBackground); \
   } \
   ColorType getBackgroundColor() const \
   { \
      ColorType backgroundColor; \
      \
      QColor clrBackground = impClass::getBackgroundColor(); \
      if (clrBackground.isValid() == true) \
      { \
         backgroundColor.mRed = clrBackground.red(); \
         backgroundColor.mGreen = clrBackground.green(); \
         backgroundColor.mBlue = clrBackground.blue(); \
      } \
      \
      return backgroundColor; \
   } \
   void setClassification(const Classification* pClassification) \
   { \
      return impClass::setClassification(pClassification); \
   } \
   void setClassificationText(const std::string& classificationText) \
   { \
      QString strClassification; \
      if (classificationText.empty() == false) \
      { \
         strClassification = QString::fromStdString(classificationText); \
      } \
      \
      impClass::setClassificationText(strClassification); \
   } \
   void getClassificationText(std::string& classificationText) const \
   { \
      classificationText.erase(); \
      \
      QString strClassification = impClass::getClassificationText(); \
      if (strClassification.isEmpty() == false) \
      { \
         classificationText = strClassification.toStdString(); \
      } \
   } \
   void setDataOrigin(const DataOrigin& dataOrigin) \
   { \
      return impClass::setDataOrigin(dataOrigin); \
   } \
   DataOrigin getDataOrigin() const \
   { \
      return impClass::getDataOrigin(); \
   } \
   AnimationController *getAnimationController() const \
   { \
      return impClass::getAnimationController(); \
   } \
   void setAnimationController(AnimationController *pPlayer) \
   { \
      impClass::setAnimationController(pPlayer); \
   } \
   bool addMouseMode(MouseMode* pMouseMode) \
   { \
      return impClass::addMouseMode(pMouseMode); \
   } \
   bool setMouseMode(MouseMode* pMouseMode) \
   { \
      return impClass::setMouseMode(pMouseMode); \
   } \
   bool setMouseMode(const std::string& modeName) \
   { \
      QString strModeName; \
      if (modeName.empty() == false) \
      { \
         strModeName = QString::fromStdString(modeName); \
      } \
      \
      return impClass::setMouseMode(strModeName); \
   } \
   bool containsMouseMode(MouseMode* pMouseMode) const \
   { \
      return impClass::containsMouseMode(pMouseMode); \
   } \
   MouseMode* getMouseMode(const std::string& modeName) const \
   { \
      QString strModeName; \
      if (modeName.empty() == false) \
      { \
         strModeName = QString::fromStdString(modeName); \
      } \
      \
      return (const_cast<MouseMode*>(impClass::getMouseMode(strModeName))); \
   } \
   MouseMode* getCurrentMouseMode() const \
   { \
      return const_cast<MouseMode*>(impClass::getCurrentMouseMode()); \
   } \
   void getMouseModes(std::vector<MouseMode*>& modes) const \
   { \
      modes.clear(); \
      \
      std::vector<const MouseMode*> mouseModes = impClass::getMouseModes(); \
      \
      std::vector<const MouseMode*>::const_iterator iter = mouseModes.begin(); \
      while (iter != mouseModes.end()) \
      { \
         MouseMode* pMouseMode = const_cast<MouseMode*>(*iter); \
         if (pMouseMode != NULL) \
         { \
            modes.push_back(pMouseMode); \
         } \
         \
         ++iter; \
      } \
   } \
   unsigned int getNumMouseModes() const \
   { \
      return impClass::getNumMouseModes(); \
   } \
   void getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY) const \
   { \
      return impClass::getExtents(dMinX, dMinY, dMaxX, dMaxY); \
   } \
   void getVisibleCorners(LocationType& lowerLeft, LocationType& upperLeft, LocationType& upperRight, \
      LocationType& lowerRight) const \
   { \
      return impClass::getVisibleCorners(lowerLeft, upperLeft, upperRight, lowerRight); \
   } \
   LocationType getVisibleCenter() const \
   { \
      return impClass::getVisibleCenter(); \
   } \
   void zoomExtents() \
   { \
      return impClass::zoomExtents(); \
   } \
   void zoomToBox(const LocationType& lowerLeft, const LocationType& upperRight) \
   { \
      return impClass::zoomToBox(lowerLeft, upperRight); \
   } \
   void panTo(const LocationType& worldCoord) \
   { \
      return impClass::panTo(worldCoord); \
   } \
   void panToCenter() \
   { \
      return impClass::panToCenter(); \
   } \
   void panBy(double dDeltaX, double dDeltaY) \
   { \
      return impClass::panBy(dDeltaX, dDeltaY); \
   } \
   bool enableInset(bool bEnable) \
   { \
      return impClass::enableInset(bEnable); \
   } \
   bool isInsetEnabled() const \
   { \
      return impClass::isInsetEnabled(); \
   } \
   void setInsetPoint(const LocationType& worldCoord) \
   { \
      return impClass::setInsetPoint(worldCoord); \
   } \
   bool enableCrossHair(bool bEnable) \
   { \
      return impClass::enableCrossHair(bEnable); \
   } \
   bool isCrossHairEnabled() const \
   { \
      return impClass::isCrossHairEnabled(); \
   } \
   void refresh() \
   { \
      return impClass::refresh(); \
   } \
   bool getCurrentImage(QImage &image) \
   { \
      return impClass::getCurrentImage(image); \
   } \
   View::SubImageIterator *getSubImageIterator(const QSize &totalSize, const QSize &subImageSize) \
   { \
      return impClass::getSubImageIterator(totalSize, subImageSize); \
   } \
   bool linkView(View* pView, LinkType type) \
   { \
      return impClass::linkView(pView, type); \
   } \
   void getLinkedViews(std::vector<std::pair<View*, LinkType> > &linkedViews) const \
   { \
      linkedViews = impClass::getLinkedViews(); \
   } \
   LinkType getViewLinkType(View* pView) const \
   { \
      return impClass::getViewLinkType(pView); \
   } \
   bool unlinkView(View* pView) \
   { \
      return impClass::unlinkView(pView); \
   } \
   View* copy() const  \
   { \
      return impClass::copy(); \
   } \
   bool copy(View *pView) const \
   { \
      return impClass::copy(pView); \
   } \
   bool removeMouseMode(const MouseMode* pMouseMode) \
   { \
      return impClass::removeMouseMode(pMouseMode); \
   } \
   void enableMouseMode(const MouseMode* pMouseMode, bool bEnable) \
   { \
      return impClass::enableMouseMode(pMouseMode, bEnable); \
   } \
   bool isMouseModeEnabled(const MouseMode* pMouseMode) const \
   { \
      return impClass::isMouseModeEnabled(pMouseMode); \
   } \
   void translateWorldToScreen(double dWorldX, double dWorldY, double& dScreenX, double& dScreenY, \
      bool* pVisible = NULL) const \
   { \
      return impClass::translateWorldToScreen(dWorldX, dWorldY, dScreenX, dScreenY, pVisible); \
   } \
   void translateScreenToWorld(double dScreenX, double dScreenY, double& dWorldX, double& dWorldY) const \
   { \
      return impClass::translateScreenToWorld(dScreenX, dScreenY, dWorldX, dWorldY); \
   } \
   double getPixelSize(const LocationType& lowerLeft, const LocationType& upperRight) const \
   { \
      return impClass::getPixelSize(lowerLeft, upperRight); \
   } \
   bool isUndoBlocked() const \
   { \
      return impClass::isUndoBlocked(); \
   } \
   bool inUndoGroup() const \
   { \
      return impClass::inUndoGroup(); \
   } \
   void addUndoAction(UndoAction* pAction) \
   { \
      impClass::addUndoAction(pAction); \
   } \
   void clearUndo() \
   { \
      impClass::clearUndo(); \
   } \
   void blockUndo() \
   { \
      impClass::blockUndo(); \
   } \
   void unblockUndo() \
   { \
      impClass::unblockUndo(); \
   } \
   void startUndoGroup(const std::string& text) \
   { \
      impClass::startUndoGroup(QString::fromStdString(text)); \
   } \
   void endUndoGroup() \
   { \
      impClass::endUndoGroup(); \
   } \
   void enableClassification(bool bEnable) \
   { \
      impClass::enableClassification(bEnable); \
   }

#endif
