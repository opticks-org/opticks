/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SPATIALDATAVIEWIMP_H
#define SPATIALDATAVIEWIMP_H

#include "AttachmentPtr.h"
#include "ColorType.h"
#include "Layer.h"
#include "Observer.h"
#include "PerspectiveViewImp.h"
#include "RasterLayer.h"
#include "SessionExplorer.h"
#include "SpatialDataView.h"
#include "TypesFile.h"

#include <QtCore/QTimer>

#include <string>
#include <vector>

class DataElement;
class LayerImp;
class LayerList;
class LayerListAdapter;
class MeasurementLayerAdapter;
class MouseMode;
class RasterElement;
class QMenu;
class QHelpEvent;
class StatusBar;

class SpatialDataViewImp : public PerspectiveViewImp, public Observer
{
   Q_OBJECT

public:
   SpatialDataViewImp(const std::string& id, const std::string& viewName, QGLContext* drawContext = 0,
      QWidget* parent = 0);
   ~SpatialDataViewImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void elementModified(Subject &subject, const std::string &signal, const boost::any &v);
   void elementDeleted(Subject &subject, const std::string &signal, const boost::any &v);
   void attached(Subject &subject, const std::string &signal, const Slot &slot);

   static bool isKindOfView(const std::string& className);
   static void getViewTypes(std::vector<std::string>& classList);

   SpatialDataViewImp& operator= (const SpatialDataViewImp& spatialDataView);
   View* copy(QGLContext* drawContext = 0, QWidget* parent = 0) const;
   bool copy(View *pView) const;

   ViewType getViewType() const;
   using ViewImp::setName;

   bool setPrimaryRasterElement(RasterElement* pRasterElement);
   TextureMode getTextureMode() const;
   LayerList* getLayerList() const;
   Layer* createLayer(const LayerType& layerType);
   Layer* createLayer(const LayerType& layerType, const QString& strLayerName);
   Layer* createLayer(const LayerType& layerType, DataElement* pElement);
   Layer* createLayer(const LayerType& layerType, DataElement* pElement, const QString& strLayerName);
   bool addLayer(Layer* pLayer);
   bool addLayerList(const LayerList* pLayerList);
   std::vector<LayerType> getDerivedLayerTypes(const Layer* pLayer) const;
   std::vector<LayerType> getDerivedLayerTypes(LayerType layerType) const;
   Layer* deriveLayer(const Layer* pLayer, const LayerType& newLayerType);
   Layer* convertLayer(Layer* pLayer, const LayerType& newLayerType);
   std::vector<Layer*> getDisplayedLayers() const;
   bool isLayerDisplayed(Layer* pLayer) const;
   Layer* getTopMostLayer(const LayerType& layerType) const;
   Layer* getTopMostLayer() const;
   DataElement* getTopMostElement(LayerType layerType) const;
   DataElement* getTopMostElement() const;
   DataElement* getTopMostElement(const std::string& elementType) const;
   void setActiveLayer(Layer* pLayer);
   Layer* getActiveLayer() const;
   bool setLayerDisplayIndex(Layer* pLayer, int iIndex);
   int getLayerDisplayIndex(Layer* pLayer) const;
   bool deleteLayer(Layer* pLayer, bool bClearUndo = true);
   void clear();
   void clearMarkings();

   Layer* getMeasurementsLayer() const;
   void showMeasurementsLayer(bool bShow);
   bool isMeasurementsLayerShown() const;
   QImage getLayerImage(Layer* pLayer, ColorType& transparent, int bbox[4]);

   void generateFullImage();

   QSize sizeHint() const;

   using PerspectiveViewImp::setInsetPoint;
   void setInsetPoint(const LocationType &worldCoord);
   void setInsetPointGeo(const LocationType &geoCoord);

   bool canLinkWithView(View *pView, LinkType type);

   PanLimitType getPanLimit() const;
   void setPanLimit(PanLimitType ePanLimit);
   double getMaximumZoom() const;
   void setMaximumZoom(double dMaxZoom);
   double getMinimumZoom() const;
   void setMinimumZoom(double dMinZoom);

   Animation* createDefaultAnimation();

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

   bool linkView(View *pView, LinkType type);
   bool isSmoothingAvailable() const;

public slots:
   void setTextureMode(const TextureMode& textureMode);
   bool showLayer(Layer* pLayer);
   bool hideLayer(Layer* pLayer);
   bool setFrontLayer(Layer* pLayer);
   bool setBackLayer(Layer* pLayer);
   bool bringLayerForward(Layer* pLayer);
   bool sendLayerBackward(Layer* pLayer);
   void updateExtents();

signals:
   void textureModeChanged(const TextureMode& textureMode);
   void layerModified(Layer* pLayer);
   void layerAdded(Layer* pLayer);
   void layerShown(Layer* pLayer);
   void layerHidden(Layer* pLayer);
   void layerActivated(Layer* pLayer);
   void layerDeleted(Layer* pLayer);
   void layerDisplayIndexesChanged();

protected:
   void enableMousePan(bool enabled);
   bool isMousePanEnabled() const;
   void setMousePanPos(const QPoint& globalScreenCoord);

   void updateStatusBar(const QPoint& screenCoord);
   void drawContents();
   void drawLayers();
   void drawOrigin();
   void drawAxis(float fX, float fY);
   void drawMousePanAnchor();
   bool isListLayer(Layer* pLayer) const;

   bool event(QEvent* pEvent);
   bool eventFilter(QObject* pWatched, QEvent* pEvent);
   void toolTipEvent(QHelpEvent* pEvent);
   void keyPressEvent(QKeyEvent* pEvent);
   void keyReleaseEvent(QKeyEvent* pEvent);
   void mousePressEvent(QMouseEvent* pEvent);
   void mouseMoveEvent(QMouseEvent* pEvent);
   void mouseReleaseEvent(QMouseEvent* pEvent);
   void mouseDoubleClickEvent(QMouseEvent* pEvent);

   virtual double limitZoomPercentage(double dPercent);
   virtual LocationType limitPanCenter(LocationType center);

   void updateContextMenu(Subject& subject, const std::string& signal, const boost::any& value);

   void updateStatusBarGeocoords(StatusBar *pBar, LocationType geoCoord);

protected slots:
   void keyPan();
   void updateMouseCursor(const MouseMode* pMouseMode);
   void disableMousePan();
   void updateStatusBar();
   void notifyLayerModified();
   void createLayer(QAction* pAction);
   void copyLayer();
   void convertLayer();
   void setActiveLayer();
   void showLayers();
   void hideLayers();
   void deleteLayer();
   void setOrigin(QAction* pAction);
   void updateOriginAction(const DataOrigin& origin);
   void setCrossHair(bool bEnabled);
   void setSmoothing(bool enabled);
   void updateSmoothingAction(const TextureMode& textureMode);
   void changeBkgColor();
   void displayMeasurementProperties();
   void setAoiMode();
   void toggleMousePanByKey();

private:
   AttachmentPtr<SessionExplorer> mpExplorer;

   TextureMode mTextureMode;
   LayerListAdapter* mpLayerList;
   SafePtr<RasterLayer> mpPrimaryRasterLayer;
   MeasurementLayerAdapter* mpMeasurementsLayer;
   bool mShowMeasurements;
   SafePtr<Layer> mpActiveLayer;
   LayerImp* mpDrawLayer;
   QTimer* mpPanTimer;
   int mPanKey;
   bool mShiftPressed;
   PanLimitType mPanLimit;
   double mMinZoom;
   double mMaxZoom;
   QPoint mMousePanAnchor;
   QPoint mMousePanCurrentPos;
   QTimer mMousePanTimer;
   QPoint mMousePanEngagePos;
   const MouseMode* mpOldMouseMode;

   // Context menu actions
   QAction* mpAnnotationAction;
   QAction* mpAoiAction;
   QAction* mpGcpAction;
   QAction* mpTiePointAction;
   QAction* mpOriginUL;
   QAction* mpOriginLL;
   QAction* mpSmoothAction;

private slots:
   void setGrayBand();
   void setRedBand();
   void setGreenBand();
   void setBlueBand();
   void setBand(RasterChannelType channel);
   void setGrayscaleMode();
   void setRgbMode();
   void nextBand();
   void previousBand();
   void mousePanTimeout();
};

#define SPATIALDATAVIEWADAPTEREXTENSION_CLASSES \
   PERSPECTIVEVIEWADAPTEREXTENSION_CLASSES

#define SPATIALDATAVIEWADAPTER_METHODS(impClass) \
   PERSPECTIVEVIEWADAPTER_METHODS(impClass) \
   PanLimitType getPanLimit() const \
   { \
      return impClass::getPanLimit(); \
   } \
   void setPanLimit(PanLimitType ePanLimit) \
   { \
      return impClass::setPanLimit(ePanLimit); \
   } \
   double getMaximumZoom() const \
   { \
      return impClass::getMaximumZoom(); \
   } \
   void setMaximumZoom(double dMaxZoom) \
   { \
      return impClass::setMaximumZoom(dMaxZoom); \
   } \
   double getMinimumZoom() const \
   { \
      return impClass::getMinimumZoom(); \
   } \
   void setMinimumZoom(double dMinZoom) \
   { \
      return impClass::setMinimumZoom(dMinZoom); \
   } \
   bool setPrimaryRasterElement(RasterElement* pRasterElement) \
   { \
      return impClass::setPrimaryRasterElement(pRasterElement); \
   } \
   void setTextureMode(const TextureMode& textureMode) \
   { \
      return impClass::setTextureMode(textureMode); \
   } \
   TextureMode getTextureMode() const \
   { \
      return impClass::getTextureMode(); \
   } \
   LayerList* getLayerList() const \
   { \
      return impClass::getLayerList(); \
   } \
   Layer* createLayer(const LayerType& layerType, DataElement* pElement) \
   { \
      return impClass::createLayer(layerType, pElement); \
   } \
   Layer* createLayer(const LayerType& layerType, DataElement* pElement, const std::string& layerName) \
   { \
      QString strLayerName; \
      if (layerName.empty() == false) \
      { \
         strLayerName = QString::fromStdString(layerName); \
      } \
      \
      return impClass::createLayer(layerType, pElement, strLayerName); \
   } \
   bool addLayer(Layer* pLayer) \
   { \
      return impClass::addLayer(pLayer); \
   } \
   bool addLayerList(const LayerList* pLayerList) \
   { \
      return impClass::addLayerList(pLayerList); \
   } \
   std::vector<LayerType> getDerivedLayerTypes(const Layer* pLayer) const \
   { \
      return impClass::getDerivedLayerTypes(pLayer); \
   } \
   std::vector<LayerType> getDerivedLayerTypes(LayerType layerType) const \
   { \
      return impClass::getDerivedLayerTypes(layerType); \
   } \
   Layer* deriveLayer(const Layer* pLayer, const LayerType& newLayerType) \
   { \
      return impClass::deriveLayer(pLayer, newLayerType); \
   } \
   Layer* convertLayer(Layer* pLayer, const LayerType& newLayerType) \
   { \
      return impClass::convertLayer(pLayer, newLayerType); \
   } \
   bool showLayer(Layer* pLayer) \
   { \
      return impClass::showLayer(pLayer); \
   } \
   bool hideLayer(Layer* pLayer) \
   { \
      return impClass::hideLayer(pLayer); \
   } \
   void getDisplayedLayers(std::vector<Layer*>& displayedLayers) const \
   { \
      displayedLayers = impClass::getDisplayedLayers(); \
   } \
   bool isLayerDisplayed(Layer* pLayer) const \
   { \
      return impClass::isLayerDisplayed(pLayer); \
   } \
   Layer* getTopMostLayer() const \
   { \
      return impClass::getTopMostLayer(); \
   } \
   Layer* getTopMostLayer(const LayerType& layerType) const \
   { \
      return impClass::getTopMostLayer(layerType); \
   } \
   DataElement* getTopMostElement() const \
   { \
      return impClass::getTopMostElement(); \
   } \
   DataElement* getTopMostElement(LayerType layerType) const \
   { \
      return impClass::getTopMostElement(layerType); \
   } \
   DataElement* getTopMostElement(const std::string& elementType) const \
   { \
      return impClass::getTopMostElement(elementType); \
   } \
   void setActiveLayer(Layer* pLayer) \
   { \
      impClass::setActiveLayer(pLayer); \
   } \
   Layer* getActiveLayer() const \
   { \
      return impClass::getActiveLayer(); \
   } \
   bool setFrontLayer(Layer* pLayer) \
   { \
      return impClass::setFrontLayer(pLayer); \
   } \
   bool setBackLayer(Layer* pLayer) \
   { \
      return impClass::setBackLayer(pLayer); \
   } \
   bool bringLayerForward(Layer* pLayer) \
   { \
      return impClass::bringLayerForward(pLayer); \
   } \
   bool sendLayerBackward(Layer* pLayer) \
   { \
      return impClass::sendLayerBackward(pLayer); \
   } \
   bool setLayerDisplayIndex(Layer* pLayer, int iIndex) \
   { \
      return impClass::setLayerDisplayIndex(pLayer, iIndex); \
   } \
   int getLayerDisplayIndex(Layer* pLayer) const \
   { \
      return impClass::getLayerDisplayIndex(pLayer); \
   } \
   bool deleteLayer(Layer* pLayer) \
   { \
      return impClass::deleteLayer(pLayer); \
   } \
   void clear() \
   { \
      return impClass::clear(); \
   } \
   void clearMarkings() \
   { \
      return impClass::clearMarkings(); \
   } \
   bool getLayerImage(Layer* pLayer, QImage &image, ColorType& transparent, int bbox[4]) \
   { \
      image = impClass::getLayerImage(pLayer, transparent, bbox); \
      return !image.isNull(); \
   } \
   bool getLayerImage(Layer* pLayer, unsigned char*& pImage, int& iWidth, int& iHeight, \
      int& iDepth, int& iNumColors, ColorType& transparent, int bbox[4]) \
   { \
      pImage = NULL; \
      iWidth = 0; \
      iHeight = 0; \
      iDepth = 0; \
      iNumColors = 0; \
      \
      QImage image = SpatialDataViewImp::getLayerImage(pLayer, transparent, bbox); \
      if (image.isNull() == true) \
      { \
         return false; \
      } \
      \
      /* Delete the previous image */ \
      static unsigned char* spBuffer = NULL; \
      if (spBuffer != NULL) \
      { \
         delete spBuffer; \
      } \
      \
      /* Set the image data */ \
      unsigned char* pBits = image.bits(); \
      int iBytes = image.numBytes(); \
      \
      spBuffer = new (std::nothrow) unsigned char[iBytes]; \
      if ((pBits == NULL) || (spBuffer == NULL)) \
      { \
         return false; \
      } \
      \
      memcpy(spBuffer, pBits, iBytes); \
      \
      /* Populate the parameter values */ \
      pImage = spBuffer; \
      iWidth = image.width(); \
      iHeight = image.height(); \
      iDepth = image.depth(); \
      iNumColors = image.numColors(); \
      \
      return true; \
   } \
   Animation* createDefaultAnimation() \
   { \
      return impClass::createDefaultAnimation(); \
   }

#endif
