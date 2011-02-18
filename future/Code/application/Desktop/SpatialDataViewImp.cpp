/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Animation.h"
#include "AnimationController.h"
#include "AnimationServices.h"
#include "AnnotationElementAdapter.h"
#include "AoiElement.h"
#include "AoiElementImp.h"
#include "AoiLayer.h"
#include "AoiLayerImp.h"
#include "AoiToolBar.h"
#include "AppConfig.h"
#include "ApplicationWindow.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "ContextMenu.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "DataDescriptorAdapter.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "Endian.h"
#include "Georeference.h"
#include "glCommon.h"
#include "GraphicGroupImp.h"
#include "HistogramWindow.h"
#include "LatLonLayer.h"
#include "LayerListAdapter.h"
#include "LayerUndo.h"
#include "MathUtil.h"
#include "MeasurementLayerAdapter.h"
#include "ModelServices.h"
#include "MouseModeImp.h"
#include "PlotWindowAdapter.h"
#include "PropertiesSpatialDataView.h"
#include "PseudocolorLayerImp.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayerImp.h"
#include "SessionManagerImp.h"
#include "SpatialDataViewAdapter.h"
#include "SpatialDataViewImp.h"
#include "Statistics.h"
#include "StatusBar.h"
#include "StringUtilities.h"
#include "ThresholdLayerImp.h"
#include "TiePointLayerAdapter.h"
#include "TypesFile.h"
#include "Undo.h"
#include "Units.h"
#include "XercesIncludes.h"

#include <QtCore/QTimer>
#include <QtGui/QActionGroup>
#include <QtGui/QApplication>
#include <QtGui/QColorDialog>
#include <QtGui/QHelpEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QToolTip>
#include <QtOpenGL/QGLFramebufferObject>

#include <boost/bind.hpp>

#include <algorithm>
#include <limits>
#include <math.h>

using namespace std;
XERCES_CPP_NAMESPACE_USE

SpatialDataViewImp::SpatialDataViewImp(const string& id, const string& viewName, QGLContext* drawContext,
                                       QWidget* parent) :
   PerspectiveViewImp(id, viewName, drawContext, parent),
   mpExplorer(Service<SessionExplorer>().get(), SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu),
      Slot(this, &SpatialDataViewImp::updateContextMenu)),
   mTextureMode(TEXTURE_NEAREST_NEIGHBOR),
   mpLayerList(new LayerListAdapter()),
   mpMeasurementsLayer(NULL),
   mShowMeasurements(true),
   mpDrawLayer(NULL),
   mpPanTimer(NULL),
   mPanKey(0),
   mShiftPressed(false),
   mPanLimit(SpatialDataView::getSettingPanLimit()),
   mMinZoom(SpatialDataView::getSettingMinimumZoomPixels()),
   mMaxZoom(SpatialDataView::getSettingMaximumZoomRatio()),
   mpAnnotationAction(NULL),
   mpAoiAction(NULL),
   mpGcpAction(NULL),
   mpTiePointAction(NULL),
   mpOriginUL(NULL),
   mpOriginLL(NULL),
   mpSmoothAction(NULL)
{
   // Measurements layer
   DataDescriptorAdapter measurementsDescriptor("Measurements", "AnnotationElement", NULL);
   mpMeasurementsLayer = new MeasurementLayerAdapter(SessionItemImp::generateUniqueId(), "Measurements",
      new AnnotationElementAdapter(measurementsDescriptor, SessionItemImp::generateUniqueId()));

   // Context menu actions
   string shortcutContext = "View/Spatial Data";
   Service<DesktopServices> pDesktop;

   // Session explorer context menu actions
   mpShowLayersAction = new QAction(QIcon(":/icons/ShowLayers"), "Show All Layers", this);
   mpShowLayersAction->setAutoRepeat(false);
   mpShowLayersAction->setShortcutContext(Qt::WidgetShortcut);
   VERIFYNR(connect(mpShowLayersAction, SIGNAL(triggered()), this, SLOT(showLayers())));
   pDesktop->initializeAction(mpShowLayersAction, shortcutContext);
   addAction(mpShowLayersAction);

   // Hide all
   mpHideLayersAction = new QAction(QIcon(":/icons/HideLayers"), "Hide All Layers", this);
   mpHideLayersAction->setAutoRepeat(false);
   mpHideLayersAction->setShortcutContext(Qt::WidgetShortcut);
   VERIFYNR(connect(mpHideLayersAction, SIGNAL(triggered()), this, SLOT(hideLayers())));
   pDesktop->initializeAction(mpHideLayersAction, shortcutContext);
   addAction(mpHideLayersAction);

   // Select layers of type
   mpSelectLayersOfTypeAction = new QAction("Select Layers of Type", this);
   mpSelectLayersOfTypeAction->setAutoRepeat(false);
   mpSelectLayersOfTypeAction->setShortcutContext(Qt::WidgetShortcut);
   VERIFYNR(connect(mpSelectLayersOfTypeAction, SIGNAL(triggered()), this, SLOT(selectLayersOfType())));
   pDesktop->initializeAction(mpSelectLayersOfTypeAction, shortcutContext);
   addAction(mpSelectLayersOfTypeAction);

   // New layer
   QMenu* pNewLayerMenu = new QMenu("New Layer", this);
   if (pNewLayerMenu != NULL)
   {
      string newLayerContext = shortcutContext + "/New Layer";

      QActionGroup* pGroup = new QActionGroup(pNewLayerMenu);
      pGroup->setExclusive(true);

      mpAnnotationAction = pGroup->addAction(QIcon(":/icons/Annotation"), "Annotation");
      mpAnnotationAction->setAutoRepeat(false);
      mpAnnotationAction->setShortcutContext(Qt::WidgetShortcut);
      pDesktop->initializeAction(mpAnnotationAction, newLayerContext);
      addAction(mpAnnotationAction);

      mpAoiAction = pGroup->addAction(QIcon(":/icons/DrawPixel"), "AOI");
      mpAoiAction->setAutoRepeat(false);
      mpAoiAction->setShortcutContext(Qt::WidgetShortcut);
      pDesktop->initializeAction(mpAoiAction, newLayerContext);
      addAction(mpAoiAction);

      mpGcpAction = pGroup->addAction(QIcon(":/icons/GcpMarker"), "GCP List");
      mpGcpAction->setAutoRepeat(false);
      mpGcpAction->setShortcutContext(Qt::WidgetShortcut);
      pDesktop->initializeAction(mpGcpAction, newLayerContext);
      addAction(mpGcpAction);

      mpTiePointAction = pGroup->addAction(QIcon(":/icons/TiePointMarker"), "Tie Point");
      mpTiePointAction->setAutoRepeat(false);
      mpTiePointAction->setShortcutContext(Qt::WidgetShortcut);
      pDesktop->initializeAction(mpTiePointAction, newLayerContext);
      addAction(mpTiePointAction);

      pNewLayerMenu->addActions(pGroup->actions());
      VERIFYNR(connect(mpAoiAction, SIGNAL(triggered()), this, SLOT(setAoiMode())));
      connect(pGroup, SIGNAL(triggered(QAction*)), this, SLOT(createLayer(QAction*)));
   }

   // Separator
   QAction* pOriginSeparatorAction = new QAction(this);
   pOriginSeparatorAction->setSeparator(true);

   // Origin
   QMenu* pOriginMenu = new QMenu("Origin", this);
   if (pOriginMenu != NULL)
   {
      string originContext = shortcutContext + "/Origin";

      QActionGroup* pGroup = new QActionGroup(pOriginMenu);
      pGroup->setExclusive(true);

      mpOriginUL = pGroup->addAction("Upper Left");
      mpOriginUL->setAutoRepeat(false);
      mpOriginUL->setCheckable(true);
      mpOriginUL->setShortcutContext(Qt::WidgetShortcut);
      pDesktop->initializeAction(mpOriginUL, originContext);
      addAction(mpOriginUL);

      mpOriginLL = pGroup->addAction("Lower Left");
      mpOriginLL->setAutoRepeat(false);
      mpOriginLL->setCheckable(true);
      mpOriginLL->setShortcutContext(Qt::WidgetShortcut);
      pDesktop->initializeAction(mpOriginLL, originContext);
      addAction(mpOriginLL);

      connect(pGroup, SIGNAL(triggered(QAction*)), this, SLOT(setOrigin(QAction*)));
      pOriginMenu->addActions(pGroup->actions());
   }

   // Separator
   QAction* pCrossHairSeparatorAction = new QAction(this);
   pCrossHairSeparatorAction->setSeparator(true);

   // Crosshair
   QAction* pCrossHairAction = new QAction("Crosshair", this);
   pCrossHairAction->setCheckable(true);
   pCrossHairAction->setChecked(View::getSettingDisplayCrosshair());
   pCrossHairAction->setAutoRepeat(false);
   pCrossHairAction->setShortcutContext(Qt::WidgetShortcut);
   pCrossHairAction->setToolTip("Show or hide the crosshair");
   pDesktop->initializeAction(pCrossHairAction, shortcutContext);
   addAction(pCrossHairAction);

   // Smoothing
   mpSmoothAction = new QAction("Smooth", this);
   mpSmoothAction->setCheckable(true);
   TextureMode tMode = getTextureMode();
   mpSmoothAction->setChecked(tMode == TEXTURE_LINEAR);
   mpSmoothAction->setAutoRepeat(false);
   mpSmoothAction->setShortcutContext(Qt::WidgetShortcut);
   mpSmoothAction->setToolTip("Smooth or don't smooth image");
   pDesktop->initializeAction(mpSmoothAction, shortcutContext);
   addAction(mpSmoothAction);

   // Background color
   QAction* pColorAction = new QAction("Background Color...", this);
   pColorAction->setAutoRepeat(false);
   pColorAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pColorAction, shortcutContext);
   addAction(pColorAction);

   // Separator
   QAction* pPropertiesSeparatorAction = new QAction(this);
   pPropertiesSeparatorAction->setSeparator(true);

   // Measurement properties
   QAction* pMeasurementsAction = new QAction("Measurements Properties...", this);
   pMeasurementsAction->setAutoRepeat(false);
   pMeasurementsAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pMeasurementsAction, shortcutContext);
   addAction(pMeasurementsAction);

   // Keyboard shortcut actions
   QAction* pGrayAction = new QAction("Set Gray Band", this);
   pGrayAction->setAutoRepeat(false);
   pGrayAction->setShortcut(QKeySequence(Qt::Key_X));
   pGrayAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pGrayAction, shortcutContext);
   addAction(pGrayAction);

   QAction* pRedAction = new QAction("Set Red Band", this);
   pRedAction->setAutoRepeat(false);
   pRedAction->setShortcut(QKeySequence(Qt::Key_R));
   pRedAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pRedAction, shortcutContext);
   addAction(pRedAction);

   QAction* pGreenAction = new QAction("Set Green Band", this);
   pGreenAction->setAutoRepeat(false);
   pGreenAction->setShortcut(QKeySequence(Qt::Key_G));
   pGreenAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pGreenAction, shortcutContext);
   addAction(pGreenAction);

   QAction* pBlueAction = new QAction("Set Blue Band", this);
   pBlueAction->setAutoRepeat(false);
   pBlueAction->setShortcut(QKeySequence(Qt::Key_B));
   pBlueAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pBlueAction, shortcutContext);
   addAction(pBlueAction);

   QAction* pGrayscaleAction = new QAction("Set Grayscale Mode", this);
   pGrayscaleAction->setAutoRepeat(false);
   pGrayscaleAction->setShortcut(QKeySequence(Qt::Key_O));
   pGrayscaleAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pGrayscaleAction, shortcutContext);
   addAction(pGrayscaleAction);

   QAction* pRgbAction = new QAction("Set RGB Mode", this);
   pRgbAction->setAutoRepeat(false);
   pRgbAction->setShortcut(QKeySequence(Qt::Key_T));
   pRgbAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pRgbAction, shortcutContext);
   addAction(pRgbAction);

   QAction* pNextBandAction = new QAction("Next Band", this);
   pNextBandAction->setAutoRepeat(false);
   pNextBandAction->setShortcut(QKeySequence(Qt::Key_PageUp));
   pNextBandAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pNextBandAction, shortcutContext);
   addAction(pNextBandAction);

   QAction* pPreviousBandAction = new QAction("Previous Band", this);
   pPreviousBandAction->setAutoRepeat(false);
   pPreviousBandAction->setShortcut(QKeySequence(Qt::Key_PageDown));
   pPreviousBandAction->setShortcutContext(Qt::WidgetShortcut);
   pDesktop->initializeAction(pPreviousBandAction, shortcutContext);
   addAction(pPreviousBandAction);

   // Initialization
   if (mpMeasurementsLayer != NULL)
   {
      mpMeasurementsLayer->setView(this);
   }

   addContextMenuAction(ContextMenuAction(pNewLayerMenu->menuAction(), APP_SPATIALDATAVIEW_NEW_LAYER_MENU_ACTION));
   addContextMenuAction(ContextMenuAction(pOriginSeparatorAction, APP_SPATIALDATAVIEW_ORIGIN_SEPARATOR_ACTION));
   addContextMenuAction(ContextMenuAction(pOriginMenu->menuAction(), APP_SPATIALDATAVIEW_ORIGIN_MENU_ACTION));
   addContextMenuAction(ContextMenuAction(pCrossHairSeparatorAction, APP_SPATIALDATAVIEW_CROSSHAIR_SEPARATOR_ACTION));
   addContextMenuAction(ContextMenuAction(pCrossHairAction, APP_SPATIALDATAVIEW_CROSSHAIR_ACTION));
   addContextMenuAction(ContextMenuAction(mpSmoothAction, APP_SPATIALDATAVIEW_SMOOTH_ACTION));
   addContextMenuAction(ContextMenuAction(pColorAction, APP_SPATIALDATAVIEW_BACKGROUND_COLOR_ACTION));
   addContextMenuAction(ContextMenuAction(pPropertiesSeparatorAction, APP_SPATIALDATAVIEW_PROPERTIES_SEPARATOR_ACTION));
   addContextMenuAction(ContextMenuAction(pMeasurementsAction, APP_SPATIALDATAVIEW_MEASUREMENTS_PROPERTIES_ACTION));
   addPropertiesPage(PropertiesSpatialDataView::getName());

   DataOrigin origin = getDataOrigin();
   updateOriginAction(origin);

   setIcon(QIcon(":/icons/SpectralData"));
   setWindowIcon(QIcon(":/icons/SpectralData"));
   addMouseMode(new MouseModeImp("LayerMode", QCursor(Qt::ArrowCursor)));
   addMouseMode(new MouseModeImp("MeasurementMode", QCursor(QPixmap(":/icons/MeasurementCursor"), 1, 14)));
   addMouseMode(new MouseModeImp("PanMode", QCursor(Qt::OpenHandCursor)));
   addMouseMode(new MouseModeImp("RotateMode", QCursor(QPixmap(":/icons/FreeRotateCursor"), 7, 9)));
   addMouseMode(new MouseModeImp("ZoomInMode", QCursor(QPixmap(":/icons/ZoomInCursor"), 0, 0)));
   addMouseMode(new MouseModeImp("ZoomOutMode", QCursor(QPixmap(":/icons/ZoomOutCursor"), 0, 0)));
   addMouseMode(new MouseModeImp("ZoomBoxMode", QCursor(QPixmap(":/icons/ZoomRectCursor"), 0, 0)));

   // Connections
   VERIFYNR(connect(this, SIGNAL(mouseModeChanged(const MouseMode*)),
      this, SLOT(updateMouseCursor(const MouseMode*))));
   VERIFYNR(connect(this, SIGNAL(originChanged(const DataOrigin&)), this, SLOT(updateOriginAction(const DataOrigin&))));
   VERIFYNR(connect(pCrossHairAction, SIGNAL(toggled(bool)), this, SLOT(setCrossHair(bool))));
   VERIFYNR(connect(this, SIGNAL(crossHairDisplayed(bool)), pCrossHairAction, SLOT(setChecked(bool))));
   VERIFYNR(connect(mpSmoothAction, SIGNAL(toggled(bool)), this, SLOT(setSmoothing(bool))));
   VERIFYNR(connect(this, SIGNAL(textureModeChanged(const TextureMode&)),
      this, SLOT(updateSmoothingAction(const TextureMode&))));
   VERIFYNR(connect(pColorAction, SIGNAL(triggered()), this, SLOT(changeBkgColor())));
   VERIFYNR(connect(pMeasurementsAction, SIGNAL(triggered()), this, SLOT(displayMeasurementProperties())));
   VERIFYNR(connect(pGrayAction, SIGNAL(triggered()), this, SLOT(setGrayBand())));
   VERIFYNR(connect(pRedAction, SIGNAL(triggered()), this, SLOT(setRedBand())));
   VERIFYNR(connect(pGreenAction, SIGNAL(triggered()), this, SLOT(setGreenBand())));
   VERIFYNR(connect(pBlueAction, SIGNAL(triggered()), this, SLOT(setBlueBand())));
   VERIFYNR(connect(pGrayscaleAction, SIGNAL(triggered()), this, SLOT(setGrayscaleMode())));
   VERIFYNR(connect(pRgbAction, SIGNAL(triggered()), this, SLOT(setRgbMode())));
   VERIFYNR(connect(pNextBandAction, SIGNAL(triggered()), this, SLOT(nextBand())));
   VERIFYNR(connect(pPreviousBandAction, SIGNAL(triggered()), this, SLOT(previousBand())));

   if (mpLayerList != NULL)
   {
      VERIFYNR(connect(mpLayerList, SIGNAL(layerAdded(Layer*)), this, SIGNAL(layerAdded(Layer*))));
      VERIFYNR(connect(mpLayerList, SIGNAL(layerDeleted(Layer*)), this, SIGNAL(layerDeleted(Layer*))));
   }

   if (mpMeasurementsLayer != NULL)
   {
      VERIFYNR(connect(mpMeasurementsLayer, SIGNAL(modified()), this, SLOT(refresh())));
      VERIFYNR(connect(mpMeasurementsLayer, SIGNAL(modified()), this, SLOT(notifyLayerModified())));
   }

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(pDesktop->getMainWidget());
   if (pAppWindow != NULL)
   {
      VERIFYNR(connect(this, SIGNAL(mouseModeAdded(const MouseMode*)),
         pAppWindow, SLOT(addMouseModeToGroup(const MouseMode*))));
   }
}

SpatialDataViewImp::~SpatialDataViewImp()
{
   // Clean up the layers
   clear();
   delete mpLayerList;

   // Destroy the measurements layer
   if (mpMeasurementsLayer != NULL)
   {
      emit layerDeleted(mpMeasurementsLayer);
      delete mpMeasurementsLayer;
   }

   // Destroy the mouse modes
   deleteMouseMode(getMouseMode("LayerMode"));
   deleteMouseMode(getMouseMode("MeasurementMode"));
   deleteMouseMode(getMouseMode("PanMode"));
   deleteMouseMode(getMouseMode("RotateMode"));
   deleteMouseMode(getMouseMode("ZoomInMode"));
   deleteMouseMode(getMouseMode("ZoomOutMode"));
   deleteMouseMode(getMouseMode("ZoomBoxMode"));
}

const string& SpatialDataViewImp::getObjectType() const
{
   static string sType("SpatialDataViewImp");
   return sType;
}

bool SpatialDataViewImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SpatialDataView"))
   {
      return true;
   }

   return PerspectiveViewImp::isKindOf(className);
}

bool SpatialDataViewImp::isKindOfView(const string& className)
{
   if ((className == "SpatialDataViewImp") || (className == "SpatialDataView"))
   {
      return true;
   }

   return PerspectiveViewImp::isKindOfView(className);
}

void SpatialDataViewImp::getViewTypes(vector<string>& classList)
{
   classList.push_back("SpatialDataView");
   PerspectiveViewImp::getViewTypes(classList);
}

void SpatialDataViewImp::elementDeleted(Subject &subject, const string &signal, const boost::any &v)
{
   DataElement* pElement = dynamic_cast<DataElement*>(&subject);
   if (NN(pElement))
   {
      if (mpLayerList != NULL)
      {
         vector<Layer*> layers;
         mpLayerList->getLayers(layers);

         for (unsigned int i = 0; i < layers.size(); i++)
         {
            Layer* pLayer = layers[i];
            if (pLayer != NULL)
            {
               DataElement* pCurrentElement = pLayer->getDataElement();
               if ((pCurrentElement == pElement) || (pCurrentElement == NULL))
               {
                  if (pLayer == mpPrimaryRasterLayer.get())
                  {
                     Service<DesktopServices> pDesktop;
                     pDesktop->deleteView(dynamic_cast<View*>(this));
                  }
                  else
                  {
                     deleteLayer(pLayer);
                  }

                  break;
               }
            }
         }
      }
   }
}

SpatialDataViewImp& SpatialDataViewImp::operator= (const SpatialDataViewImp& spatialDataView)
{
   if (this != &spatialDataView)
   {
      PerspectiveViewImp::operator= (spatialDataView);

      mTextureMode = spatialDataView.getTextureMode();
      mShowMeasurements = spatialDataView.mShowMeasurements;
      mPanLimit = spatialDataView.mPanLimit;
      mMinZoom = spatialDataView.mMinZoom;
      mMaxZoom = spatialDataView.mMaxZoom;

      if ((mpMeasurementsLayer != NULL) && (spatialDataView.mpMeasurementsLayer != NULL))
      {
         *mpMeasurementsLayer = *(spatialDataView.mpMeasurementsLayer);
         // Copy the measurement objects from the view's layer to this layer
         list<GraphicObject*> objects;
         spatialDataView.mpMeasurementsLayer->getObjects(objects);

         for (list<GraphicObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
         {
            GraphicObject* pObject = *iter;
            if (pObject != NULL)
            {
               GraphicObjectType objectType = pObject->getGraphicObjectType();

               GraphicObject* pNewObject = mpMeasurementsLayer->addObject(objectType);
               if (pNewObject != NULL)
               {
                  GraphicObjectImp* pNewObjectImp = dynamic_cast<GraphicObjectImp*>(pNewObject);
                  if (pNewObjectImp != NULL)
                  {
                     pNewObjectImp->replicateObject(pObject);
                  }
               }
            }
         }
      }

      clear();

      LayerList* pLayerList = spatialDataView.getLayerList();
      if (pLayerList != NULL)
      {
         setPrimaryRasterElement(pLayerList->getPrimaryRasterElement());

         vector<Layer*> layers;
         pLayerList->getLayers(layers);
         for (vector<Layer*>::size_type i = 0; i < layers.size(); ++i)
         {
            Layer* pLayer = layers[i];
            if (pLayer != NULL)
            {
               DataElement* pParent = pLayerList->getPrimaryRasterElement();

               DataElement* pElement = pLayer->getDataElement();
               if (pElement != NULL)
               {
                  pParent = pElement->getParent();
               }

               Layer* pNewLayer = pLayer->copy(string(), false, pParent);
               if (pNewLayer != NULL)
               {
                  addLayer(pNewLayer);

                  // Layers are shown by default when they are initialized,
                  // so hide the layer if the original layer is hidden
                  if (spatialDataView.isLayerDisplayed(pLayer) == false)
                  {
                     hideLayer(pNewLayer);
                  }

                  // Activate the layer if necessary
                  if (spatialDataView.getActiveLayer() == pLayer)
                  {
                     setActiveLayer(pNewLayer);
                  }
               }
            }
         }
      }

      notify(SIGNAL_NAME(Subject, Modified));
   }
   return *this;
}

View* SpatialDataViewImp::copy(QGLContext* drawContext, QWidget* parent) const
{
   string viewName = getName();

   SpatialDataViewAdapter* pView = new SpatialDataViewAdapter(SessionItemImp::generateUniqueId(), viewName,
      drawContext, parent);
   if (pView != NULL)
   {
      UndoLock lock(pView);
      *dynamic_cast<SpatialDataViewImp*>(pView) = *this;
   }

   return static_cast<View*>(pView);
}

bool SpatialDataViewImp::copy(View *pView) const
{
   SpatialDataViewImp* pViewImp = dynamic_cast<SpatialDataViewImp*>(pView);
   if (pViewImp != NULL)
   {
      UndoLock lock(pView);
      *pViewImp = *this;
   }

   return pViewImp != NULL;
}

ViewType SpatialDataViewImp::getViewType() const
{
   return SPATIAL_DATA_VIEW;
}

bool SpatialDataViewImp::setPrimaryRasterElement(RasterElement* pRasterElement)
{
   bool bSuccess = false;
   if ((mpLayerList != NULL) && (mpPrimaryRasterLayer.get() == NULL))
   {
      bSuccess = mpLayerList->setPrimaryRasterElement(pRasterElement);
      if (bSuccess == true)
      {
         // Set the primary raster layer
         vector<Layer*> layers;
         mpLayerList->getLayers(RASTER, layers);
         for (vector<Layer*>::iterator iter = layers.begin(); iter != layers.end(); ++iter)
         {
            RasterLayer* pRasterLayer = static_cast<RasterLayer*>(*iter);
            if (pRasterLayer != NULL)
            {
               RasterElement* pCurrentRasterElement = dynamic_cast<RasterElement*>(pRasterLayer->getDataElement());
               if (pCurrentRasterElement == pRasterElement)
               {
                  mpPrimaryRasterLayer.reset(pRasterLayer);
                  break;
               }
            }
         }
      }
   }

   return bSuccess;
}

TextureMode SpatialDataViewImp::getTextureMode() const
{
   return mTextureMode;
}

LayerList* SpatialDataViewImp::getLayerList() const
{
   return mpLayerList;
}

void SpatialDataViewImp::notifyLayerModified()
{
   Layer* pLayer = dynamic_cast<Layer*>(sender());
   if (pLayer != NULL)
   {
      emit layerModified(pLayer);
   }
}

Layer* SpatialDataViewImp::createLayer(const LayerType& layerType)
{
   return createLayer(layerType, NULL, QString());
}

Layer* SpatialDataViewImp::createLayer(const LayerType& layerType, const QString& strLayerName)
{
   return createLayer(layerType, NULL, strLayerName);
}

Layer* SpatialDataViewImp::createLayer(const LayerType& layerType, DataElement* pElement)
{
   return createLayer(layerType, pElement, QString());
}

Layer* SpatialDataViewImp::createLayer(const LayerType& layerType, DataElement* pElement,
                                       const QString& strLayerName)
{
   Layer* pLayer = NULL;
   if (mpLayerList != NULL)
   {
      // Attach the element
      if (pElement != NULL)
      {
         pElement->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &SpatialDataViewImp::elementDeleted));
      }

      pLayer = mpLayerList->newLayer(layerType, pElement, strLayerName);
      if (pLayer != NULL)
      {
         bool bSuccess = addLayer(pLayer);
         if (bSuccess == true)
         {
            // Set the primary raster layer
            if ((mpPrimaryRasterLayer.get() == NULL) && (pElement == mpLayerList->getPrimaryRasterElement()))
            {
               mpPrimaryRasterLayer.reset(dynamic_cast<RasterLayer*>(pLayer));
               if (mpPrimaryRasterLayer.get() != NULL)
               {
                  UndoLock lock(dynamic_cast<View*>(this));

                  // Send the layer to the back
                  setBackLayer(pLayer);

                  // Zoom extents
                  resetZoom();
               }
            }
         }
      }
   }

   return pLayer;
}

bool SpatialDataViewImp::addLayer(Layer* pLayer)
{
   if ((pLayer == NULL) || (mpLayerList == NULL))
   {
      return false;
   }

   if (mpLayerList->containsLayer(pLayer) == true)
   {
      return false;
   }

   // Attach the element
   DataElement* pElement = pLayer->getDataElement();
   if (pElement != NULL)
   {
      pElement->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &SpatialDataViewImp::elementDeleted));
   }

   // Set the layer to this view
   LayerImp* pLayerImp = dynamic_cast<LayerImp*>(pLayer);
   if (pLayerImp != NULL)
   {
      pLayerImp->setView(this);
   }

   // Add the layer
   bool bSuccess = mpLayerList->addLayer(pLayer);
   if (bSuccess == true)
   {
      // Notify of a display index change
      emit layerDisplayIndexesChanged();
      notify(SIGNAL_NAME(SpatialDataView, LayerDisplayIndexesChanged));

      // Add the undo action
      addUndoAction(new AddLayer(dynamic_cast<SpatialDataView*>(this), pLayer));

      // Set the view to refresh when the layer is modified
      if (pLayerImp != NULL)
      {
         VERIFYNR(connect(pLayerImp, SIGNAL(modified()), this, SLOT(refresh())));
         VERIFYNR(connect(pLayerImp, SIGNAL(modified()), this, SLOT(notifyLayerModified())));
         VERIFYNR(connect(pLayerImp, SIGNAL(extentsModified()), this, SLOT(updateExtents())));
      }

      RasterLayerImp* pRasterLayer = dynamic_cast<RasterLayerImp*>(pLayer);
      if (pRasterLayer != NULL)
      {
         VERIFYNR(connect(pRasterLayer, SIGNAL(displayedBandChanged(RasterChannelType, DimensionDescriptor)),
            this, SLOT(updateStatusBar())));
      }

      // Set the view classification to that of the layer's element if this is the first layer added
      if ((mpLayerList->getNumLayers() == 1) && (pElement != NULL))
      {
         Service<SessionManager> pManager;
         if (pManager->isSessionLoading() == false)
         {
            const Classification* pClassification = pElement->getClassification();
            if (pClassification != NULL)
            {
               setClassification(pClassification);
            }
         }
      }

      // Show the layer
      UndoLock lock(dynamic_cast<View*>(this));
      showLayer(pLayer);

      // Update the extents
      updateExtents();
   }
   else if (pElement != NULL)
   {
      pElement->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &SpatialDataViewImp::elementDeleted));
   }

   return bSuccess;
}

bool SpatialDataViewImp::addLayerList(const LayerList* pLayerList)
{
   if (pLayerList == NULL)
   {
      return false;
   }

   vector<Layer*> layers;
   pLayerList->getLayers(layers);
   for (unsigned int i = 0; i < layers.size(); i++)
   {
      Layer* pLayer = NULL;
      pLayer = layers.at(i);
      if (pLayer != NULL)
      {
         addLayer(pLayer);
      }
   }

   return true;
}

vector<LayerType> SpatialDataViewImp::getDerivedLayerTypes(const Layer* pLayer) const
{
   vector<LayerType> derivedTypes;
   if (pLayer != NULL)
   {
      LayerType layerType = pLayer->getLayerType();
      derivedTypes = getDerivedLayerTypes(layerType);
   }

   return derivedTypes;
}

vector<LayerType> SpatialDataViewImp::getDerivedLayerTypes(LayerType layerType) const
{
   vector<LayerType> derivedTypes;
   switch (layerType)
   {
      case ANNOTATION:
         derivedTypes.push_back(AOI_LAYER);
         break;

      case AOI_LAYER:
         derivedTypes.push_back(ANNOTATION);
         break;

      case PSEUDOCOLOR:
         derivedTypes.push_back(AOI_LAYER);
         derivedTypes.push_back(RASTER);
         derivedTypes.push_back(THRESHOLD);
         break;

      case RASTER:
         derivedTypes.push_back(PSEUDOCOLOR);
         derivedTypes.push_back(THRESHOLD);
         break;

      case THRESHOLD:
         derivedTypes.push_back(AOI_LAYER);
         derivedTypes.push_back(PSEUDOCOLOR);
         derivedTypes.push_back(RASTER);
         break;

      default:
         break;
   }

   return derivedTypes;
}

Layer* SpatialDataViewImp::deriveLayer(const Layer* pLayer, const LayerType& newLayerType)
{
   if (pLayer == NULL)
   {
      return NULL;
   }

   QString strLayerName = QString::fromStdString(pLayer->getName());
   DataElement* pElement = pLayer->getDataElement();

   DataElement* pParentElement = NULL;
   if (pElement != NULL)
   {
      pParentElement = pElement->getParent();
   }

   // Check for a copying a layer of the same type
   LayerType layerType = pLayer->getLayerType();
   if (layerType == newLayerType)
   {
      // Get a unique name
      QString strNewLayerName = mpLayerList->getUniqueLayerName(strLayerName, newLayerType);
      if (strNewLayerName.isEmpty() == true)
      {
         return NULL;
      }

      // Copy the layer and pass in the parent element because Annotation and AOI layers are always copied
      Layer* pNewLayer = pLayer->copy(strNewLayerName.toStdString(), false, pParentElement);
      if (pNewLayer != NULL)
      {
         addLayer(pNewLayer);
      }

      return pNewLayer;
   }

   // Check that the layer supports deriving to the new layer type
   vector<LayerType> derivedTypes = getDerivedLayerTypes(pLayer);
   vector<LayerType>::iterator iter = std::find(derivedTypes.begin(), derivedTypes.end(), newLayerType);
   if (iter == derivedTypes.end())
   {
      return NULL;
   }

   // Get the name and data element for the new layer
   if ((newLayerType == AOI_LAYER) || (newLayerType == ANNOTATION))
   {
      Service<ModelServices> pModel;
      if (newLayerType == AOI_LAYER)
      {
         // Create a new element
         AoiElement* pAoi = static_cast<AoiElement*>(pModel->getElement(strLayerName.toStdString(),
            "AoiElement", pParentElement));
         if (pAoi != NULL)
         {
            QMessageBox::warning(this, APP_NAME, "An AOI layer already exists with the current layer name.  "
               "Please select a unique name for the new AOI layer.");

            strLayerName = mpLayerList->getUniqueLayerName(strLayerName, newLayerType);
            if (strLayerName.isEmpty() == true)
            {
               return NULL;
            }
         }

         pAoi = static_cast<AoiElement*>(pModel->createElement(strLayerName.toStdString(), "AoiElement",
            pParentElement));
         if (pAoi == NULL)
         {
            return NULL;
         }

         // Populate the data
         if (layerType == ANNOTATION)
         {
            GraphicElement* pGraphicElement = dynamic_cast<GraphicElement*>(pElement);
            if (pGraphicElement != NULL)
            {
               const GraphicGroup* pGroup = pGraphicElement->getGroup();
               GraphicGroupImp* pNewGroup = dynamic_cast<GraphicGroupImp*>(pAoi->getGroup());

               if ((pGroup != NULL) && (pNewGroup != NULL))
               {
                  pNewGroup->replicateObject(pGroup);
               }
            }
         }
         else if ((layerType == PSEUDOCOLOR) || (layerType == THRESHOLD))
         {
            const BitMask* pMask = NULL;
            if (layerType == PSEUDOCOLOR)
            {
               pMask = dynamic_cast<const PseudocolorLayerImp*>(pLayer)->getSelectedPixels();
            }
            else
            {
               pMask = dynamic_cast<const ThresholdLayerImp*>(pLayer)->getSelectedPixels();
            }

            if (pMask != NULL)
            {
               pAoi->addPoints(pMask);
            }
            else
            {
               QMessageBox::warning(this, APP_NAME, "Could not access the current layer's bit mask.");
               pModel->destroyElement(pAoi);
               return NULL;
            }
         }

         pElement = pAoi;
      }
      else if (newLayerType == ANNOTATION)
      {
         // Create a new element
         AnnotationElement* pAnnotation = static_cast<AnnotationElement*>(pModel->getElement(strLayerName.toStdString(),
            "AnnotationElement", pParentElement));
         if (pAnnotation != NULL)
         {
            QMessageBox::warning(this, APP_NAME, "An Annotation layer already exists with the current layer name.  "
               "Please select a unique name for the new Annotation layer.");

            strLayerName = mpLayerList->getUniqueLayerName(strLayerName, newLayerType);
            if (strLayerName.isEmpty() == true)
            {
               return NULL;
            }
         }

         pAnnotation = static_cast<AnnotationElement*>(pModel->createElement(strLayerName.toStdString(),
            "AnnotationElement", pParentElement));

         // Populate the data
         if (layerType == AOI_LAYER)
         {
            GraphicElement* pGraphicElement = dynamic_cast<GraphicElement*>(pElement);
            if (pGraphicElement != NULL)
            {
               const GraphicGroup* pGroup = pGraphicElement->getGroup();
               GraphicGroupImp* pNewGroup = dynamic_cast<GraphicGroupImp*>(pAnnotation->getGroup());

               if ((pGroup != NULL) && (pNewGroup != NULL))
               {
                  pNewGroup->replicateObject(pGroup);
               }
            }
         }

         pElement = pAnnotation;
      }
   }

   // Create the layer
   Layer* pNewLayer = createLayer(newLayerType, pElement, strLayerName);
   if (pNewLayer != NULL)
   {
      pNewLayer->setXOffset(pLayer->getXOffset());
      pNewLayer->setYOffset(pLayer->getYOffset());
      pNewLayer->setXScaleFactor(pLayer->getXScaleFactor());
      pNewLayer->setYScaleFactor(pLayer->getYScaleFactor());
      updateGL();
   }

   return pNewLayer;
}

Layer* SpatialDataViewImp::convertLayer(Layer* pLayer, const LayerType& newLayerType)
{
   if ((pLayer == NULL) || (pLayer == mpPrimaryRasterLayer.get()))
   {
      return NULL;
   }

   LayerType layerType = pLayer->getLayerType();
   if (layerType == newLayerType)
   {
      return NULL;
   }

   UndoGroup group(dynamic_cast<View*>(this), "Convert Layer");

   // Derive a new layer
   Layer* pNewLayer = deriveLayer(pLayer, newLayerType);
   if (pNewLayer != NULL)
   {
      // Delete the original layer
      deleteLayer(pLayer);

      // Create a histogram plot if necessary because it may not have been created
      // if it already existed when the new layer was created
      Service<DesktopServices> pDesktop;

      HistogramWindow* pHistWindow =
         dynamic_cast<HistogramWindow*>(pDesktop->getWindow("Histogram Window", PLOT_WINDOW));
      if (pHistWindow != NULL)
      {
         pHistWindow->createPlot(pNewLayer);
      }
   }

   return pNewLayer;
}

vector<Layer*> SpatialDataViewImp::getDisplayedLayers() const
{
   return mpLayerList->getDisplayedLayers();
}

bool SpatialDataViewImp::isLayerDisplayed(Layer* pLayer) const
{
   return mpLayerList->isLayerDisplayed(pLayer);
}

Layer* SpatialDataViewImp::getTopMostLayer(const LayerType& layerType) const
{
   return mpLayerList->getTopMostLayer(layerType);
}

Layer* SpatialDataViewImp::getTopMostLayer() const
{
   return mpLayerList->getTopMostLayer();
}

DataElement* SpatialDataViewImp::getTopMostElement(LayerType layerType) const
{
   return mpLayerList->getTopMostElement(layerType);
}

DataElement* SpatialDataViewImp::getTopMostElement() const
{
   return mpLayerList->getTopMostElement();
}

DataElement* SpatialDataViewImp::getTopMostElement(const string& elementType) const
{
   return mpLayerList->getTopMostElement(elementType);
}

void SpatialDataViewImp::setActiveLayer(Layer* pLayer)
{
   LayerImp* pLayerImp = dynamic_cast<LayerImp*>(pLayer);
   if (pLayer != mpActiveLayer.get())
   {
      if ((pLayer == NULL) ||
         ((mpLayerList->containsLayer(pLayer) == true) && (pLayerImp->acceptsMouseEvents() == true)))
      {
         mpActiveLayer.reset(pLayer);
         emit layerActivated(pLayer);
         notify(SIGNAL_NAME(SpatialDataView, LayerActivated), boost::any(pLayer));

         const MouseMode* pMouseMode = getCurrentMouseMode();
         updateMouseCursor(pMouseMode);
      }
   }
}

Layer* SpatialDataViewImp::getActiveLayer() const
{
   return const_cast<Layer*>(mpActiveLayer.get());
}

bool SpatialDataViewImp::setLayerDisplayIndex(Layer* pLayer, int iIndex)
{
   if ((pLayer == NULL) || (iIndex < 0))
   {
      return false;
   }

   if (isListLayer(pLayer) == false)
   {
      return false;
   }

   int iCurrentIndex = mpLayerList->getLayerDisplayIndex(pLayer);
   if (iCurrentIndex == iIndex)
   {
      return true;
   }

   bool bSuccess = mpLayerList->setLayerDisplayIndex(pLayer, iIndex);
   if (bSuccess == true)
   {
      addUndoAction(new SetLayerDisplayIndex(pLayer, iCurrentIndex, iIndex));

      emit layerDisplayIndexesChanged();
      notify(SIGNAL_NAME(SpatialDataView, LayerDisplayIndexesChanged));
      refresh();
   }

   return bSuccess;
}

int SpatialDataViewImp::getLayerDisplayIndex(Layer* pLayer) const
{
   return mpLayerList->getLayerDisplayIndex(pLayer);
}

bool SpatialDataViewImp::deleteLayer(Layer* pLayer, bool bClearUndo)
{
   if ((pLayer == NULL) || (mpLayerList == NULL))
   {
      return false;
   }

   string actionText = "Delete Layer";
   if (pLayer != NULL)
   {
      actionText = "Delete " + StringUtilities::toDisplayString(pLayer->getLayerType()) + " Layer";
   }

   UndoGroup group(dynamic_cast<View*>(this), actionText);

   // Disconnect
   LayerImp* pLayerImp = dynamic_cast<LayerImp*>(pLayer);
   disconnect(pLayerImp, SIGNAL(modified()), this, SLOT(refresh()));
   disconnect(pLayerImp, SIGNAL(modified()), this, SLOT(notifyLayerModified()));
   disconnect(pLayerImp, SIGNAL(extentsModified()), this, SLOT(updateExtents()));

   RasterLayerImp* pRasterLayer = dynamic_cast<RasterLayerImp*>(pLayer);
   if (pRasterLayer != NULL)
   {
      VERIFYNR(disconnect(pRasterLayer, SIGNAL(displayedBandChanged(RasterChannelType, DimensionDescriptor)),
         this, SLOT(updateStatusBar())));
   }

   // Detach the element
   DataElement* pElement = pLayer->getDataElement();
   if (pElement != NULL)
   {
      pElement->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &SpatialDataViewImp::elementDeleted));
   }

   // Get the layer type
   LayerType layerType = pLayer->getLayerType();

   // Get the currently active layer
   Layer* pActiveLayer = getActiveLayer();

   // Query whether the layer will delete the element
   bool bUniqueElement = pLayer->hasUniqueElement();

   // Remove the layer
   bool bSuccess = mpLayerList->deleteLayer(pLayer);
   if (bSuccess == true)
   {
      // Reset the active layer if necessary
      if (pLayer == pActiveLayer)
      {
         Layer* pNewActiveLayer = getTopMostLayer(layerType);
         setActiveLayer(pNewActiveLayer);
      }

      // Notify of a display index change
      emit layerDisplayIndexesChanged();
      notify(SIGNAL_NAME(SpatialDataView, LayerDisplayIndexesChanged));

      // Clear all undo stacks if the layer's element is deleted
      if ((bUniqueElement == true) && (bClearUndo == true))
      {
         Service<DesktopServices> pDesktop;

         ApplicationWindow* pAppWindow = dynamic_cast<ApplicationWindow*>(pDesktop->getMainWidget());
         if (pAppWindow != NULL)
         {
            pAppWindow->clearUndoStacks();
         }
      }

      updateExtents();
      refresh();
   }

   return bSuccess;
}

void SpatialDataViewImp::clear()
{
   if (mpLayerList == NULL)
   {
      return;
   }

   vector<Layer*> layers;
   mpLayerList->getLayers(layers);

   // We can't simply iterate over the list of layers and delete them one at a time because the
   // deletion of a layer could also cause the deletion of the associated data element. This
   // could cause the deletion of a child element which could cause the deletion of a layer in
   // the list before the iterator reaches the now invalid layer pointer. The attempt to delete the 
   // invalid pointer would cause Opticks to crash.
   while (layers.empty() == false)
   {
      // delete the primary raster layer last
      if (layers.front() == mpPrimaryRasterLayer.get())
      {
         deleteLayer(layers.back());
      }
      else
      {
         deleteLayer(layers.front());
      }
      mpLayerList->getLayers(layers);
   }
}

void SpatialDataViewImp::clearMarkings()
{
   if (mpLayerList == NULL)
   {
      return;
   }

   // Get the raster element
   RasterElement* pRasterElement = mpLayerList->getPrimaryRasterElement();
   if (pRasterElement == NULL)
   {
      return;
   }

   // Delete all layers except the raster layer and the Corner Coordinates GCP list
   LayerImp* pRasterLayer = NULL;
   LayerImp* pGcpLayer = NULL;

   vector<Layer*> layers;
   mpLayerList->getLayers(layers);

   for (unsigned int i = 0; i < layers.size(); i++)
   {
      Layer* pLayer = layers[i];
      if (pLayer != NULL)
      {
         DataElement* pElement = pLayer->getDataElement();
         if (pElement != pRasterElement)
         {
            string layerName = pLayer->getName();

            LayerType eLayerType = pLayer->getLayerType();

            if ((eLayerType == GCP_LAYER) && (layerName == "Corner Coordinates"))
            {
               pGcpLayer = dynamic_cast<LayerImp*> (pLayer);
            }
            else
            {
               deleteLayer(pLayer);
            }
         }
         else
         {
            pRasterLayer = dynamic_cast<LayerImp*> (pLayer);
         }
      }
   }

   // Reset the raster layer properties to the initial values
   if (pRasterLayer != NULL)
   {
      pRasterLayer->reset();
   }

   // Reset the Corner Coordinates layer properties to the initial values
   if (pGcpLayer != NULL)
   {
      pGcpLayer->reset();
   }

   reset();
   clearUndo();
}

Layer* SpatialDataViewImp::getMeasurementsLayer() const
{
   return mpMeasurementsLayer;
}

void SpatialDataViewImp::showMeasurementsLayer(bool bShow)
{
   mShowMeasurements = bShow;
}

bool SpatialDataViewImp::isMeasurementsLayerShown() const
{
   return mShowMeasurements;
}

QImage SpatialDataViewImp::getLayerImage(Layer* pLayer, ColorType& transparent, int bbox[4])
{
   LayerImp* pLayerImp = dynamic_cast<LayerImp*>(pLayer);
   if (pLayerImp == NULL)
   {
      return QImage();
   }

   QColor clrBackground = getBackgroundColor();
   if (!transparent.isValid())
   {
      vector<ColorType> layerColors = pLayerImp->getColors();
      transparent = ColorType::getUniqueColor(layerColors);
   }
   if (transparent.isValid())
   {
      clrBackground = COLORTYPE_TO_QCOLOR(transparent);
   }

   // Save matrices
   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   if (QGLFramebufferObject::hasOpenGLFramebufferObjects())
   {
      // Set to draw to an off screen buffer
      makeCurrent();
      int w = width();
      int h = height();
      if (bbox[0] > 0 && bbox[0] <= w && bbox[1] > 0 && bbox[1] <= h)
      {
         w = bbox[0];
         h = bbox[1];
      }
      QGLFramebufferObject fbo(w, h);
      fbo.bind();

      // Draw the layer
      setupWorldMatrices();

      glViewport(0, 0, w, h);
      glGetIntegerv(GL_VIEWPORT, mViewPort);

      glFlush();
      qglClearColor(clrBackground);
      glClear(GL_COLOR_BUFFER_BIT);

      pLayerImp->draw();

      fbo.release();

      // Restore matrices
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixd(projectionMatrix);
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixd(modelMatrix);
      glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);

      // Read the pixels from the draw buffer
      int iWidth = w;
      int iHeight = h;

      QImage img = fbo.toImage();

      // Initialize the bounding box
      bbox[0] = iWidth;
      bbox[1] = iHeight;
      bbox[2] = 0;
      bbox[3] = 0;

      // Scan the buffer to find the actual bounding box that contains all pixels not equal to the background color
      unsigned int backgroundCpack = clrBackground.red() + (clrBackground.green() << 8) + (clrBackground.blue() << 16);
      if (Endian::getSystemEndian() == BIG_ENDIAN_ORDER)
      {
         backgroundCpack = (clrBackground.red() << 24) + (clrBackground.green() << 16) + (clrBackground.blue() << 8);
      }

      for (int i = 0; i < iHeight; i++)
      {
         for (int j = 0; j < iWidth; j++)
         {
            unsigned int pixel = img.pixel(j, i) & 0xffffff;
            if (Endian::getSystemEndian() == BIG_ENDIAN_ORDER)
            {
               pixel = img.pixel(j, i) & 0xffffff00;
            }

            if (pixel != backgroundCpack)
            {
               if (bbox[0] > j)
               {
                  bbox[0] = j;
               }
               if (bbox[1] > i)
               {
                  bbox[1] = i;
               }
               if (bbox[2] < j)
               {
                  bbox[2] = j;
               }
               if (bbox[3] < i)
               {
                  bbox[3] = i;
               }
            }
         }
      }

      // If no pixels found that were not the background color, return a NULL image
      if ((bbox[0] > bbox[2]) || (bbox[1] > bbox[3]))
      {
         return QImage();
      }

      // Get the pixel data of just the bounding box area
      iWidth = bbox[2] - bbox[0] + 1;
      iHeight = bbox[3] - bbox[1] + 1;

      return img.copy(bbox[0], bbox[1], iWidth, iHeight).convertToFormat(QImage::Format_ARGB32);
   }
   else
   {
      // Set to draw the layer in the back buffer
      glDrawBuffer(GL_BACK);
      glReadBuffer(GL_BACK);

      // Draw the layer
      setupWorldMatrices();

      glViewport(0, 0, width(), height());
      glGetIntegerv(GL_VIEWPORT, mViewPort);

      glFlush();
      qglClearColor(clrBackground);
      glClear(GL_COLOR_BUFFER_BIT);

      pLayerImp->draw();

      // Restore matrices
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixd(projectionMatrix);
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixd(modelMatrix);
      glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);

      // Read the pixels from the draw buffer
      int iWidth = width();
      int iHeight = height();

      vector<unsigned int> pixels(iWidth * iHeight);
      glReadPixels(0, 0, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) &pixels[0]);

      // Initialize the bounding box
      bbox[0] = iWidth;
      bbox[1] = iHeight;
      bbox[2] = 0;
      bbox[3] = 0;

      // Scan the buffer to find the actual bounding box that contains all pixels not equal to the background color
      unsigned int backgroundCpack = clrBackground.red() + (clrBackground.green() << 8) + (clrBackground.blue() << 16);
      if (Endian::getSystemEndian() == BIG_ENDIAN_ORDER)
      {
         backgroundCpack = (clrBackground.red() << 24) + (clrBackground.green() << 16) + (clrBackground.blue() << 8);
      }

      for (int i = 0; i < iHeight; i++)
      {
         for (int j = 0; j < iWidth; j++)
         {
            unsigned int pixel = pixels[i * iWidth + j] & 0xffffff;
            if (Endian::getSystemEndian() == BIG_ENDIAN_ORDER)
            {
               pixel = pixels[i * iWidth + j] & 0xffffff00;
            }

            if (pixel != backgroundCpack)
            {
               if (bbox[0] > j)
               {
                  bbox[0] = j;
               }
               if (bbox[1] > i)
               {
                  bbox[1] = i;
               }
               if (bbox[2] < j)
               {
                  bbox[2] = j;
               }
               if (bbox[3] < i)
               {
                  bbox[3] = i;
               }
            }
         }
      }

      // If no pixels found that were not the background color, return a NULL image
      if ((bbox[0] > bbox[2]) || (bbox[1] > bbox[3]))
      {
         return QImage();
      }

      // Get the pixel data of just the bounding box area
      iWidth = bbox[2] - bbox[0] + 1;
      iHeight = bbox[3] - bbox[1] + 1;

      vector<unsigned int> bbPixels(iWidth * iHeight);
      glReadPixels(bbox[0], bbox[1], iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) &bbPixels[0]);

      ViewImp::reorderImage(&bbPixels[0], iWidth, iHeight);

      // Create the image from the pixel data
      QImage image(iWidth, iHeight, QImage::Format_ARGB32);

      unsigned char* pBits = NULL;
      pBits = image.bits();
      if (pBits != NULL)
      {
         memcpy(pBits, &bbPixels[0], iWidth * iHeight * 4);
      }
      return image;
   }
}

void SpatialDataViewImp::generateFullImage()
{
   RasterLayerImp* pRasterLayer = dynamic_cast<RasterLayerImp*>(getTopMostLayer(RASTER));
   if (pRasterLayer)
   {
      pRasterLayer->generateFullImage();
   }
}

QSize SpatialDataViewImp::sizeHint() const
{
   // Set the size based on the extents
   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;
   getExtents(dMinX, dMinY, dMaxX, dMaxY);

   QSize szExtents(static_cast<int>(dMaxX - dMinX), static_cast<int>(dMaxY - dMinY));
   return szExtents;
}

Animation* SpatialDataViewImp::createDefaultAnimation()
{
   Service<AnimationServices> pAnimationServices;

   AnimationController* pController = getAnimationController();
   if (pController == NULL)
   {
      pController = pAnimationServices->getAnimationController(getName());
   }

   if (pController != NULL)
   {
      if (pController->getNumAnimations() == 0)
      {
         if (pController->getFrameType() == FRAME_TIME)
         {
            pAnimationServices->destroyAnimationController(pController);
            pController = NULL;
         }
      }
      else if (pController->getFrameType() == FRAME_TIME)
      {
         return NULL;
      }
   }

   if (pController == NULL)
   {
      pController = pAnimationServices->createAnimationController(getName(), FRAME_ID);
      if (pController == NULL)
      {
         return NULL;
      }

      // Default to one frame per second
      pController->setMinimumFrameRate(boost::rational<int>(1, 1));
   }

   // Set the animation controller in the view
   setAnimationController(pController);

   // Get the animation name from the active cube layer
   RasterLayer* pRasterLayer = static_cast<RasterLayer*>(getTopMostLayer(RASTER));
   if (pRasterLayer != NULL)
   {
      const string& animationName = pRasterLayer->getName();
      if (animationName.empty() == false)
      {
         // Create the animation
         Animation* pAnimation = pController->createAnimation(animationName);
         if (pAnimation != NULL)
         {
            unsigned int numFrames = 0;

            RasterElement* pRasterElement = dynamic_cast<RasterElement*>(pRasterLayer->getDataElement());
            if (pRasterElement != NULL)
            {
               const RasterDataDescriptor* pDescriptor =
                  dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
               if (pDescriptor != NULL)
               {
                  numFrames = pDescriptor->getBandCount();
               }
            }

            vector<AnimationFrame> frames;
            for (unsigned int i = 0; i < numFrames; ++i)
            {
               AnimationFrame frame;
               frame.mFrameNumber = i;

               frames.push_back(frame);
            }

            pAnimation->setFrames(frames);

            // Set the animation in the raster layer
            pRasterLayer->setAnimation(pAnimation);

            return pAnimation;
         }
      }
   }

   return NULL;
}

void SpatialDataViewImp::setTextureMode(const TextureMode& textureMode)
{
   if (textureMode == TEXTURE_LINEAR && isSmoothingAvailable() == false)
   {
      return;
   }

   if (textureMode != mTextureMode)
   {
      mTextureMode = textureMode;
      emit textureModeChanged(mTextureMode);
      notify(SIGNAL_NAME(SpatialDataView, TextureModeChanged), boost::any(mTextureMode));
   }
}

bool SpatialDataViewImp::showLayer(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   // Only show the layer if it is not already displayed
   if (isLayerDisplayed(pLayer) == true)
   {
      return false;
   }

   bool bSuccess = mpLayerList->showLayer(pLayer);
   if (bSuccess == true)
   {
      addUndoAction(new ShowLayer(pLayer));

      emit layerShown(pLayer);
      notify(SIGNAL_NAME(SpatialDataView, LayerShown), boost::any(pLayer));

      UndoLock lock(dynamic_cast<View*>(this));
      updateExtents();
      refresh();
   }

   return bSuccess;
}

bool SpatialDataViewImp::hideLayer(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   if (isLayerDisplayed(pLayer) == false)
   {
      return false;
   }

   bool bSuccess = mpLayerList->hideLayer(pLayer);
   if (bSuccess == true)
   {
      addUndoAction(new HideLayer(pLayer));

      emit layerHidden(pLayer);
      notify(SIGNAL_NAME(SpatialDataView, LayerHidden), boost::any(pLayer));

      UndoLock lock(dynamic_cast<View*>(this));
      updateExtents();
      refresh();
   }

   return bSuccess;
}

bool SpatialDataViewImp::setFrontLayer(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   int iCurrentIndex = getLayerDisplayIndex(pLayer);
   if (iCurrentIndex == 0)
   {
      return true;
   }

   bool bSuccess = mpLayerList->setFrontLayer(pLayer);
   if (bSuccess == true)
   {
      addUndoAction(new SetLayerDisplayIndex(pLayer, iCurrentIndex, 0));

      emit layerDisplayIndexesChanged();
      notify(SIGNAL_NAME(SpatialDataView, LayerDisplayIndexesChanged));
      refresh();
   }

   return bSuccess;
}

bool SpatialDataViewImp::setBackLayer(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   int iCurrentIndex = getLayerDisplayIndex(pLayer);

   int numDisplayedLayers = static_cast<int>(getDisplayedLayers().size());
   if (iCurrentIndex == numDisplayedLayers - 1)
   {
      return true;
   }

   bool bSuccess = mpLayerList->setBackLayer(pLayer);
   if (bSuccess == true)
   {
      addUndoAction(new SetLayerDisplayIndex(pLayer, iCurrentIndex, numDisplayedLayers - 1));

      emit layerDisplayIndexesChanged();
      notify(SIGNAL_NAME(SpatialDataView, LayerDisplayIndexesChanged));
      refresh();
   }

   return bSuccess;
}

bool SpatialDataViewImp::bringLayerForward(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   int iCurrentIndex = getLayerDisplayIndex(pLayer);
   if (iCurrentIndex == 0)
   {
      return true;
   }

   bool bSuccess = mpLayerList->bringLayerForward(pLayer);
   if (bSuccess == true)
   {
      addUndoAction(new SetLayerDisplayIndex(pLayer, iCurrentIndex, iCurrentIndex - 1));

      emit layerDisplayIndexesChanged();
      notify(SIGNAL_NAME(SpatialDataView, LayerDisplayIndexesChanged));
      refresh();
   }

   return bSuccess;
}

bool SpatialDataViewImp::sendLayerBackward(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   int iCurrentIndex = getLayerDisplayIndex(pLayer);

   int numDisplayedLayers = static_cast<int>(getDisplayedLayers().size());
   if (iCurrentIndex == numDisplayedLayers - 1)
   {
      return true;
   }

   bool bSuccess = mpLayerList->sendLayerBackward(pLayer);
   if (bSuccess == true)
   {
      addUndoAction(new SetLayerDisplayIndex(pLayer, iCurrentIndex, iCurrentIndex + 1));

      emit layerDisplayIndexesChanged();
      notify(SIGNAL_NAME(SpatialDataView, LayerDisplayIndexesChanged));
      refresh();
   }

   return bSuccess;
}

void SpatialDataViewImp::updateStatusBarGeocoords(StatusBar *pBar, LocationType geoCoord)
{
   if (pBar == NULL)
   {
      return;
   }

   GeocoordType geocoordType = LatLonLayer::getSettingGeocoordType();
   DmsFormatType dmsFormat = LatLonLayer::getSettingFormat();

   LatLonLayer* pLatLonLayer = dynamic_cast<LatLonLayer*>(getTopMostLayer(LAT_LONG));
   if (pLatLonLayer != NULL)
   {
      geocoordType = pLatLonLayer->getGeocoordType();
      dmsFormat = pLatLonLayer->getLatLonFormat();
   }

   pBar->setGeoCoords(geoCoord, geocoordType, dmsFormat);
}

void SpatialDataViewImp::updateStatusBar(const QPoint& screenCoord)
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

   pBar->clearValues();
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

      LocationType dataCoord;
      LocationType geoCoord;
      if (mpPrimaryRasterLayer.get() != NULL)
      {
         RasterElement* pPrimaryRasterElement = mpLayerList->getPrimaryRasterElement();
         if (pPrimaryRasterElement != NULL)
         {
            mpPrimaryRasterLayer->translateWorldToData(dX, dY, dataCoord.mX, dataCoord.mY);
            bool geoValid(false);
            geoCoord = pPrimaryRasterElement->convertPixelToGeocoord(dataCoord, false, &geoValid);
            if (geoValid)
            {
               updateStatusBarGeocoords(pBar, geoCoord);
            }
         }
      }

      if ((dX >= dMinX) && (dX <= dMaxX) && (dY >= dMinY) && (dY <= dMaxY))
      {
         vector<Layer*> displayedLayers = mpLayerList->getDisplayedLayers();
         for (unsigned int i = 0; i < displayedLayers.size(); i++)
         {
            Layer* pLayer = displayedLayers[i];
            if (pLayer != NULL)
            {
               LocationType dataCoord;
               pLayer->translateWorldToData(dX, dY, dataCoord.mX, dataCoord.mY);

               DataElement* pElement = pLayer->getDataElement();
               if (pElement != NULL)
               {
                  RasterElement* pRaster = dynamic_cast<RasterElement*>(pElement);
                  if (pRaster != NULL)
                  {
                     // Get the original pixel coordinates
                     const RasterDataDescriptor* pDescriptor =
                        dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
                     if (pDescriptor != NULL)
                     {
                        const vector<DimensionDescriptor>& activeRows = pDescriptor->getRows();
                        const vector<DimensionDescriptor>& activeColumns = pDescriptor->getColumns();
                        if ((dataCoord.mY >= 0) &&
                           (activeRows.size() > static_cast<unsigned int>(dataCoord.mY)) &&
                           (activeRows[static_cast<int>(dataCoord.mY)].isValid()) &&
                           (dataCoord.mX >= 0) &&
                           (activeColumns.size() > static_cast<unsigned int>(dataCoord.mX)) &&
                           (activeColumns[static_cast<int>(dataCoord.mX)].isValid()))
                        {
                           if (pRaster == mpLayerList->getPrimaryRasterElement())
                           {
                              RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
                              if (pRasterLayer == NULL)
                              {
                                 continue;
                              }

                              DimensionDescriptor rowDim = activeRows[static_cast<int>(dataCoord.mY)];
                              DimensionDescriptor columnDim = activeColumns[static_cast<int>(dataCoord.mX)];

                              int originalSceneX = columnDim.getOriginalNumber();
                              int originalSceneY = rowDim.getOriginalNumber();
                              pBar->setPixelCoords(originalSceneX, originalSceneY);

                              ComplexComponent eComponent = pRasterLayer->getComplexComponent();
                              DisplayMode eMode = pRasterLayer->getDisplayMode();

                              const RasterElement* pTerrain = pRaster->getTerrain();
                              if (pTerrain != NULL)
                              {
                                 // Do not need a valid band for elevation information
                                 double dValue = pTerrain->getPixelValue(columnDim, rowDim, DimensionDescriptor(),
                                    eComponent);

                                 const Units* pElevationUnits = NULL;

                                 // Multiply by the units scale factor
                                 const RasterDataDescriptor* pTerrainDescriptor =
                                    dynamic_cast<const RasterDataDescriptor*>(pTerrain->getDataDescriptor());
                                 if (pTerrainDescriptor != NULL)
                                 {
                                    pElevationUnits = pTerrainDescriptor->getUnits();
                                    if (pElevationUnits != NULL)
                                    {
                                       dValue *= pElevationUnits->getScaleFromStandard();
                                    }
                                 }

                                 pBar->setElevationValue(dValue, pElevationUnits);
                              }

                              const Units* pUnits = pDescriptor->getUnits();

                              if (eMode == GRAYSCALE_MODE)
                              {
                                 DimensionDescriptor bandDim = pRasterLayer->getDisplayedBand(GRAY);
                                 if (bandDim.isValid())
                                 {
                                    // Get the value from the displayed raster element
                                    double dValue = 0.0;

                                    RasterElement* pDisplayedRaster = pRasterLayer->getDisplayedRasterElement(GRAY);
                                    if (pDisplayedRaster != NULL)
                                    {
                                       dValue = pDisplayedRaster->getPixelValue(columnDim, rowDim, bandDim, eComponent);
                                    }

                                    // Multiply the value by the units scale factor
                                    if (pUnits != NULL)
                                    {
                                       dValue *= pUnits->getScaleFromStandard();
                                    }

                                    // Set the value in the status bar
                                    pBar->setCubeValue(dValue);
                                 }
                                 else
                                 {
                                    pBar->clearCubeValue();
                                 }
                              }
                              else // eMode == RGB_MODE
                              {
                                 QString redValue = "N/A";
                                 QString greenValue = "N/A";
                                 QString blueValue = "N/A";

                                 // Get the red band value
                                 DimensionDescriptor redBand = pRasterLayer->getDisplayedBand(RED);
                                 if (redBand.isValid())
                                 {
                                    // Get the value from the displayed raster element
                                    double dRedValue = 0.0;

                                    RasterElement* pDisplayedRaster = pRasterLayer->getDisplayedRasterElement(RED);
                                    if (pDisplayedRaster != NULL)
                                    {
                                       dRedValue = pDisplayedRaster->getPixelValue(columnDim, rowDim, redBand,
                                          eComponent);
                                    }

                                    // Multiply the value by the units scale factor
                                    if (pUnits != NULL)
                                    {
                                       dRedValue *= pUnits->getScaleFromStandard();
                                    }

                                    redValue = QString::number(dRedValue, 'g', numeric_limits<double>::digits10);
                                 }

                                 // Get the green band value
                                 DimensionDescriptor greenBand = pRasterLayer->getDisplayedBand(GREEN);
                                 if (greenBand.isValid())
                                 {
                                    // Get the value from the displayed raster element
                                    double dGreenValue = 0.0;

                                    RasterElement* pDisplayedRaster = pRasterLayer->getDisplayedRasterElement(GREEN);
                                    if (pDisplayedRaster != NULL)
                                    {
                                       dGreenValue = pDisplayedRaster->getPixelValue(columnDim, rowDim, greenBand,
                                          eComponent);
                                    }

                                    // Multiply the value by the units scale factor
                                    if (pUnits != NULL)
                                    {
                                       dGreenValue *= pUnits->getScaleFromStandard();
                                    }

                                    greenValue = QString::number(dGreenValue, 'g', numeric_limits<double>::digits10);
                                 }

                                 // Get the blue band value
                                 DimensionDescriptor blueBand = pRasterLayer->getDisplayedBand(BLUE);
                                 if (blueBand.isValid())
                                 {
                                    // Get the value from the displayed raster element
                                    double dBlueValue = 0.0;

                                    RasterElement* pDisplayedRaster = pRasterLayer->getDisplayedRasterElement(BLUE);
                                    if (pDisplayedRaster != NULL)
                                    {
                                       dBlueValue = pDisplayedRaster->getPixelValue(columnDim, rowDim, blueBand,
                                          eComponent);
                                    }

                                    // Multiply the value by the units scale factor
                                    if (pUnits != NULL)
                                    {
                                       dBlueValue *= pUnits->getScaleFromStandard();
                                    }

                                    blueValue = QString::number(dBlueValue, 'g', numeric_limits<double>::digits10);
                                 }

                                 if ((redBand.isValid()) || (greenBand.isValid()) || (blueBand.isValid()))
                                 {
                                    pBar->setCubeValue(redValue, greenValue, blueValue);
                                 }
                                 else
                                 {
                                    pBar->clearCubeValue();
                                 }
                              }
                           }
                           else
                           {
                              // not the primary RasterElement
                              ComplexComponent eComponent = RasterLayer::getSettingComplexComponent();
                              DimensionDescriptor bandDim;

                              RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
                              if (pRasterLayer != NULL)
                              {
                                 eComponent = pRasterLayer->getComplexComponent();
                                 bandDim = pRasterLayer->getDisplayedBand(GRAY);
                              }

                              string layerName = pLayer->getName();
                              QString strLayerName = QString::fromStdString(layerName);

                              int numRows = pDescriptor->getRowCount();
                              int numCols = pDescriptor->getColumnCount();

                              vector<int> badValues;

                              Statistics* pStatistics = pRaster->getStatistics(bandDim);
                              if (pStatistics != NULL)
                              {
                                 badValues = pStatistics->getBadValues();
                              }

                              if ((dataCoord.mX >= 0) && (dataCoord.mX < numCols) && (dataCoord.mY >= 0) &&
                                 (dataCoord.mY < numRows))
                              {
                                 DimensionDescriptor column = pDescriptor->getActiveColumn(
                                    static_cast<unsigned int>(dataCoord.mX));
                                 DimensionDescriptor row = pDescriptor->getActiveRow(
                                    static_cast<unsigned int>(dataCoord.mY));

                                 double dValue = pRaster->getPixelValue(column, row, bandDim, eComponent);
                                 if (std::find(badValues.begin(), badValues.end(),
                                    roundDouble(dValue)) == badValues.end())
                                 {
                                    pBar->setResultValue(strLayerName, dValue, pDescriptor->getUnits());
                                 }
                                 else
                                 {
                                    pBar->clearResultValue();
                                 }
                              }
                              else
                              {
                                 pBar->clearResultValue();
                              }

                           }
                        }
                     }
                  }
               }
            }
         }

         double rotation = getRotation();
         pBar->setRotationValue(rotation);
      }
   }
}

void SpatialDataViewImp::updateExtents()
{
   vector<Layer*> layers;
   switch (mPanLimit)
   {
   case NO_LIMIT:       // fall through
   case MAX_EXTENTS:
   default:
      layers = getDisplayedLayers();   // get extents from all displayed layers
      break;

   case CUBE_EXTENTS:    // get extents of cube layer only
      Layer* pLayer = NULL;
      if (mpLayerList != NULL)
      {
         RasterElement* pElem = mpLayerList->getPrimaryRasterElement();
         pLayer = mpLayerList->getLayer(RASTER, pElem);
      }

      if (pLayer != NULL)
      {
         layers.push_back(pLayer);
      }
      break;
   }

   double dMinX = 1e38;
   double dMinY = 1e38;
   double dMaxX = -1e38;
   double dMaxY = -1e38;
   double x1;
   double y1;
   double x4;
   double y4;

   if (layers.size() == 0)
   {
      return;
   }

   vector<Layer*>::iterator it = layers.begin();
   for (; it != layers.end(); ++it)
   {
      LayerImp* pImp = dynamic_cast<LayerImp*>(*it);
      VERIFYNRV(pImp != NULL);
      if (pImp->getExtents(x1, y1, x4, y4))
      {
         dMinX = min(dMinX, x1);
         dMinY = min(dMinY, y1);
         dMaxX = max(dMaxX, x4);
         dMaxY = max(dMaxY, y4);
      }
   }

   if (dMinX == dMaxX)
   {
      dMinX -= 1.0;
      dMaxX += 1.0;
   }

   if (dMinY == dMaxY)
   {
      dMinY -= 1.0;
      dMaxY += 1.0;
   }

   setExtents(dMinX, dMinY, dMaxX, dMaxY);
}

void SpatialDataViewImp::drawContents()
{
   // Layers
   drawLayers();

   // Measurement layer
   if ((mpMeasurementsLayer != NULL) && (mShowMeasurements == true))
   {
      setupWorldMatrices();
      mpMeasurementsLayer->draw();
   }

   // Origin
   drawOrigin();

   // Axis
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);

   qglColor(Qt::black);
   drawAxis(viewPort[0] + 31, viewPort[1] + viewPort[3] - 31);
   qglColor(Qt::white);
   drawAxis(viewPort[0] + 30, viewPort[1] + viewPort[3] - 30);
}

void SpatialDataViewImp::drawLayers()
{
   setupWorldMatrices();

   vector<Layer*> displayedLayers = mpLayerList->getDisplayedLayers();
   for (unsigned int i = 0; i < displayedLayers.size(); i++)
   {
      mpDrawLayer = dynamic_cast<LayerImp*>(displayedLayers[i]);
      if (mpDrawLayer != NULL)
      {
         double xOffset = mpDrawLayer->getXOffset();
         double yOffset = mpDrawLayer->getYOffset();
         double xScale = mpDrawLayer->getXScaleFactor();
         double yScale = mpDrawLayer->getYScaleFactor();

         glPushMatrix();
         glTranslated(xOffset, yOffset, 0);
         glScaled(xScale, yScale, 1);
         mpDrawLayer->draw();
         glPopMatrix();
      }
   }

   mpDrawLayer = NULL;
}

void SpatialDataViewImp::drawOrigin()
{
   setupWorldMatrices();

   qglColor(Qt::white);
   glLineWidth (1.0);
   glBegin(GL_LINES);
   glVertex2f(0.0, 0.0);
   glVertex2f(0.0, -10.0);
   glVertex2f(0.0, 0.0);
   glVertex2f(-10.0, 0.0);
   glEnd();
}

void SpatialDataViewImp::drawAxis(float fX, float fY)
{
   setupScreenMatrices();

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glTranslatef(fX, fY, 0.0);

   glLineWidth(1.0);

   double dHeading = getRotation();
   double dPitch = getPitch();

   int flippedMultiplier = 0;

   // Pitch < 0 means that the user has flipped the dataset vertically.
   if (dPitch < 0)
   {
      dHeading -= 180;
      flippedMultiplier = -1;
   }
   else
   {
      dHeading *= -1;
      flippedMultiplier = 1;
   }

   glRotatef(dHeading, 0, 0, 1);

   glBegin(GL_LINES);

   // draw the arrow for the x axis
   glVertex2f(0.0, 0.0);
   glVertex2f(15.0 * flippedMultiplier, 0.0);
   glVertex2f(10.0 * flippedMultiplier, -3.0 * flippedMultiplier);
   glVertex2f(15.0 * flippedMultiplier, 0.0);
   glVertex2f(10.0 * flippedMultiplier, 3.0 * flippedMultiplier);
   glVertex2f(15.0 * flippedMultiplier, 0.0);

   // draw an arrow for y axis
   glVertex2f(0.0, 0.0);
   glVertex2f(0.0, 15.0);
   glVertex2f(-3.0, 10.0);
   glVertex2f(0.0, 15.0);
   glVertex2f(3.0, 10.0);
   glVertex2f(0.0, 15.0);
   glEnd();

   // Draw the X and Y labels
   QFont widgetFont = font();
   QFontMetrics fontMetrics(widgetFont);
   QRect rectX = fontMetrics.tightBoundingRect("X");
   QRect rectY = fontMetrics.tightBoundingRect("Y");

   int textWidthX = rectX.width();
   int textWidthY = rectY.width();
   int textHeightX = rectX.height();
   int textHeightY = rectY.height();

   // Get the arrow tip location
   double dRotation = getRotation();
   if (getPitch() >= 0.0)
   {
      dRotation *= -1.0;
   }

   LocationType xCoord(fX + (15.0 * cos(dRotation * PI / 180.0)),
      fY + (15.0 * sin(dRotation * PI / 180.0)));
   LocationType yCoord(fX + (15.0 * cos((dRotation + (90.0 * flippedMultiplier)) * PI / 180.0)),
      fY + (15.0 * sin((dRotation + (90.0 * flippedMultiplier)) * PI / 180.0)));

   // Calculate the text rotation offset
   double dSpacing = 3.0;

   LocationType offsetX;
   offsetX.mX = (textWidthX / 2.0 * cos(dRotation * PI / 180.0)) - (textWidthX / 2.0) +
      (dSpacing * cos(dRotation * PI / 180.0));
   offsetX.mY = (textHeightX / 2.0 * sin(dRotation * PI / 180.0)) - (textHeightX / 2.0) +
      (dSpacing * sin(dRotation * PI / 180.0));

   LocationType offsetY;
   offsetY.mX = (textWidthY / 2.0 * cos((dRotation + (90.0 * flippedMultiplier)) * PI / 180.0)) -
      (textWidthY / 2.0) + (dSpacing * cos((dRotation + (90.0 * flippedMultiplier)) * PI / 180.0));
   offsetY.mY = (textHeightY / 2.0 * sin((dRotation + (90.0 * flippedMultiplier)) * PI / 180.0)) -
      (textHeightY / 2.0) + (dSpacing * sin((dRotation + (90.0 * flippedMultiplier)) * PI / 180.0));

   LocationType xLocation = xCoord + offsetX;
   LocationType yLocation = yCoord + offsetY;

   // Draw the X character
   int screenX = static_cast<int>(xLocation.mX);
   int screenY = static_cast<int>(xLocation.mY);

   // If drawing the axis in a view object in a product, map the product
   // view screen coordinates to spatial data view coordinates
   if (dynamic_cast<ViewImp*>(parentWidget()) != NULL)
   {
      QPoint screenCoord = mapFromParent(QPoint(screenX, parentWidget()->height() - screenY));
      screenX = screenCoord.x();
      screenY = screenCoord.y();
   }
   else
   {
      screenY = height() - screenY;
   }

   renderText(screenX, screenY, "X", widgetFont);

   // Draw the Y character
   screenX = static_cast<int>(yLocation.mX);
   screenY = static_cast<int>(yLocation.mY);

   // If drawing the axis in a view object in a product, map the product
   // view screen coordinates to spatial data view coordinates
   if (dynamic_cast<ViewImp*>(parentWidget()) != NULL)
   {
      QPoint screenCoord = mapFromParent(QPoint(screenX, parentWidget()->height() - screenY));
      screenX = screenCoord.x();
      screenY = screenCoord.y();
   }
   else
   {
      screenY = height() - screenY;
   }

   renderText(screenX, screenY, "Y", widgetFont);

   glPopMatrix();
}

bool SpatialDataViewImp::isListLayer(Layer* pLayer) const
{
   if ((pLayer == NULL) || (mpLayerList == NULL))
   {
      return false;
   }

   return mpLayerList->containsLayer(pLayer);
}

bool SpatialDataViewImp::event(QEvent* pEvent)
{
   if (pEvent->type() == QEvent::ToolTip)
   {
      toolTipEvent(static_cast<QHelpEvent*>(pEvent));
      return true;
   }
   else if (pEvent->type() == QEvent::Polish)
   {
      attach(SIGNAL_NAME(View, AboutToShowContextMenu), Slot(this, &SpatialDataViewImp::updateContextMenu));
   }

   return PerspectiveViewImp::event(pEvent);
}

void SpatialDataViewImp::toolTipEvent(QHelpEvent* pEvent)
{
   bool showGeoCoordToolTip = SpatialDataView::getSettingGeoCoordTooltip();
   if (!showGeoCoordToolTip)
   {
      return;
   }

   double dX = 0.0;
   double dY = 0.0;
   translateScreenToWorld(pEvent->pos().x(), height() - pEvent->pos().y(), dX, dY);

   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;
   getExtents(dMinX, dMinY, dMaxX, dMaxY);

   if ((dX < dMinX) || (dX > dMaxX) || (dY < dMinY) || (dY > dMaxY))
   {
      //outside the extents, so clear any existing tooltip
      QToolTip::showText(pEvent->globalPos(), "", this);
      pEvent->accept();
      return;
   }

   LayerList* pLayerList = getLayerList();
   VERIFYNRV(pLayerList != NULL);

   RasterElement* pRaster = pLayerList->getPrimaryRasterElement();
   if (pRaster != NULL && pRaster->isGeoreferenced())
   {
      LocationType dataCoord(dX, dY);

      Layer* pLayer = pLayerList->getLayer(RASTER, pRaster);
      if (pLayer != NULL)
      {
         pLayer->translateWorldToData(dX, dY, dataCoord.mX, dataCoord.mY);
      }

      GeocoordType geocoordType = LatLonLayer::getSettingGeocoordType();
      DmsFormatType dmsFormat = LatLonLayer::getSettingFormat();
      LatLonLayer* pLatLonLayer = dynamic_cast<LatLonLayer*>(pLayerList->getLayer(LAT_LONG, pRaster));
      if (pLatLonLayer != NULL)
      {
         geocoordType = pLatLonLayer->getGeocoordType();
         dmsFormat = pLatLonLayer->getLatLonFormat();
      }

      // convert to geo coordinate.
      LocationType geoCoord = pRaster->convertPixelToGeocoord(dataCoord);
      LatLonPoint latLonPoint(geoCoord);

      string coordinateText = "";
      if (geocoordType == GEOCOORD_LATLON)
      {
         coordinateText = "Geo: (" + latLonPoint.getText(dmsFormat) + ")";
      }
      else if (geocoordType == GEOCOORD_UTM)
      {
         UtmPoint utmPoint(latLonPoint);
         coordinateText = "UTM: (" + utmPoint.getText() + ")";
      }
      else if (geocoordType == GEOCOORD_MGRS)
      {
         MgrsPoint mgrsPoint(latLonPoint);
         coordinateText = "MGRS: (" + mgrsPoint.getText() + ")";
      }
      QToolTip::showText(pEvent->globalPos(),
                         QString::fromStdString(coordinateText), this);
      pEvent->accept();
   }
}

void SpatialDataViewImp::keyPressEvent(QKeyEvent* pEvent)
{
   if (pEvent == NULL)
   {
      return;
   }

   Qt::KeyboardModifiers modifiers = pEvent->modifiers();

   switch (pEvent->key())
   {
      case Qt::Key_Up: // fall through to next case
      case Qt::Key_Down: // fall through to next case
      case Qt::Key_Left: // fall through to next case
      case Qt::Key_Right:
         mPanKey = pEvent->key();
         break;
      case Qt::Key_Shift:
         mShiftPressed = true;
         break;
      default:
         break;
   }

   if (pEvent->key() == Qt::Key_Control)
   {
      if (mpPanTimer == NULL)
      {
         mpPanTimer = new QTimer;
         connect(mpPanTimer, SIGNAL(timeout()), this, SLOT(keyPan()));
         mpPanTimer->start(0);
      }
      if (mPanKey == 0)
      {
         PerspectiveViewImp::keyPressEvent(pEvent);
      }
   }
   else if (modifiers & Qt::ControlModifier)
   {
      if (mPanKey == 0)
      {
         PerspectiveViewImp::keyPressEvent(pEvent);
      }
   }
   else
   {
      PerspectiveViewImp::keyPressEvent(pEvent);

      if (pEvent->key() == Qt::Key_Delete)
      {
         const MouseMode* pMouseMode = getCurrentMouseMode();
         if (pMouseMode != NULL)
         {
            GraphicLayerImp* pLayer = NULL;

            string mouseMode = "";
            pMouseMode->getName(mouseMode);
            if (mouseMode == "LayerMode")
            {
               pLayer = dynamic_cast<GraphicLayerImp*>(getActiveLayer());
            }
            else if (mouseMode == "MeasurementMode")
            {
               pLayer = dynamic_cast<GraphicLayerImp*> (getMeasurementsLayer());
            }

            if (pLayer != NULL)
            {
               pLayer->deleteSelectedObjects();
               refresh();
            }
         }
      }
   }
}

void SpatialDataViewImp::keyReleaseEvent(QKeyEvent* pEvent)
{
   if (pEvent == NULL)
   {
      return;
   }

   if (pEvent->key() == Qt::Key_Shift)
   {
      mShiftPressed = false;
   }

   if (pEvent->key() == Qt::Key_Control && mpPanTimer)
   {
      delete mpPanTimer;
      mpPanTimer = NULL;
   }

   if (pEvent->key() == mPanKey && pEvent->isAutoRepeat() == false)
   {
      mPanKey = 0;
   }
}

void SpatialDataViewImp::keyPan()
{   
   int distance = 0;
   if (mShiftPressed)
   {
      distance = SpatialDataView::getSettingFastPanSpeed();
   }
   else
   {
      distance = SpatialDataView::getSettingSlowPanSpeed();
   }

   switch (mPanKey)
   {
      case Qt::Key_Up:
         ViewImp::pan(QPoint(0, 0), QPoint(0, distance));
         updateGL();
         break;

      case Qt::Key_Down:
         ViewImp::pan(QPoint(0, 0), QPoint(0, -distance));
         updateGL();
         break;

      case Qt::Key_Left:
         ViewImp::pan(QPoint(0, 0), QPoint(-distance, 0));
         updateGL();
         break;

      case Qt::Key_Right:
         ViewImp::pan(QPoint(0, 0), QPoint(distance, 0));
         updateGL();
         break;

      default:
         break;
   }
}

void SpatialDataViewImp::mousePressEvent(QMouseEvent* pEvent)
{
   bool bSuccess = false;
   if (pEvent != NULL)
   {
      QPoint ptMouse = pEvent->pos();
      ptMouse.setY(height() - pEvent->pos().y());

      string mouseMode = "";

      const MouseMode* pMouseMode = getCurrentMouseMode();
      if (pMouseMode != NULL)
      {
         pMouseMode->getName(mouseMode);
      }

      LayerImp* pActiveLayer = dynamic_cast<LayerImp*>(mpActiveLayer.get());
      if ((mouseMode == "LayerMode") && (pActiveLayer != NULL))
      {
         bSuccess = pActiveLayer->processMousePress(ptMouse, pEvent->button(), pEvent->buttons(),
            pEvent->modifiers());

         GraphicLayerImp* pGraphicLayer = dynamic_cast<GraphicLayerImp*>(mpActiveLayer.get());
         AoiLayer* pAoiLayer = dynamic_cast<AoiLayer*>(mpActiveLayer.get());
         if (bSuccess == true && pGraphicLayer != NULL && pGraphicLayer->getNumSelectedObjects() == 0 &&
            (pAoiLayer == NULL || pAoiLayer->getMode() == AOI_MOVE))
         {
            // No objects selected, so activate another GraphicLayer
            vector<Layer*> displayedLayers = getDisplayedLayers();
            for (vector<Layer*>::reverse_iterator iter = displayedLayers.rbegin();
                 iter != displayedLayers.rend();
                 ++iter)
            {
               GraphicLayerImp* pCurrentLayer = dynamic_cast<GraphicLayerImp*>(*iter);
               if ((pCurrentLayer != NULL) && (pCurrentLayer != pGraphicLayer))
               {
                  AoiLayerImp* pCurrentAoiLayer = dynamic_cast<AoiLayerImp*>(*iter);

                  // If ptMouse hits an object within pCurrentLayer or if ptMouse hits the label of pCurrentAoiLayer,
                  // then pCurrentLayer should be activated.
                  if ((pCurrentLayer->hit(ptMouse) != NULL) ||
                     (pCurrentAoiLayer != NULL && pCurrentAoiLayer->hitLabel(ptMouse) == true))
                  {
                     setActiveLayer(dynamic_cast<Layer*>(pCurrentLayer));
                     if (pCurrentAoiLayer != NULL)
                     {
                        pCurrentAoiLayer->setMode(AOI_MOVE);
                     }

                     pCurrentLayer->processMousePress(ptMouse,
                        pEvent->button(), pEvent->buttons(), pEvent->modifiers());
                     bSuccess = true;
                     break;
                  }
               }
            }
         }
      }
      else if ((mouseMode == "MeasurementMode") && (mpMeasurementsLayer != NULL) && (mShowMeasurements == true))
      {
         bSuccess = mpMeasurementsLayer->processMousePress(ptMouse,
            pEvent->button(), pEvent->buttons(), pEvent->modifiers());
      }

      if (bSuccess == true)
      {
         pEvent->accept();
         refresh();
      }
   }

   if (bSuccess == false)
   {
      PerspectiveViewImp::mousePressEvent(pEvent);
   }
}

void SpatialDataViewImp::mouseMoveEvent(QMouseEvent* pEvent)
{
   bool bSuccess = false;
   if (pEvent != NULL)
   {
      QPoint ptMouse = pEvent->pos();
      ptMouse.setY(height() - pEvent->pos().y());

      string mouseMode = "";

      const MouseMode* pMouseMode = getCurrentMouseMode();
      if (pMouseMode != NULL)
      {
         pMouseMode->getName(mouseMode);
      }

      LayerImp* pActiveLayer = dynamic_cast<LayerImp*>(mpActiveLayer.get());
      if ((mouseMode == "LayerMode") && (pActiveLayer != NULL))
      {
         bSuccess = pActiveLayer->processMouseMove(ptMouse, pEvent->button(), pEvent->buttons(), pEvent->modifiers());
      }
      else if ((mouseMode == "MeasurementMode") && (mpMeasurementsLayer != NULL) && (mShowMeasurements == true))
      {
         bSuccess = mpMeasurementsLayer->processMouseMove(ptMouse,
            pEvent->button(), pEvent->buttons(), pEvent->modifiers());
      }

      if (bSuccess == true)
      {
         pEvent->accept();
         updateStatusBar(ptMouse);
         updateGL();
      }
   }

   if (bSuccess == false)
   {
      PerspectiveViewImp::mouseMoveEvent(pEvent);
   }
}

void SpatialDataViewImp::mouseReleaseEvent(QMouseEvent* pEvent)
{
   bool bSuccess = false;
   if (pEvent != NULL)
   {
      QPoint ptMouse = pEvent->pos();
      ptMouse.setY(height() - pEvent->pos().y());

      string mouseMode = "";

      const MouseMode* pMouseMode = getCurrentMouseMode();
      if (pMouseMode != NULL)
      {
         pMouseMode->getName(mouseMode);
      }

      LayerImp* pActiveLayer = dynamic_cast<LayerImp*>(mpActiveLayer.get());
      if ((mouseMode == "LayerMode") && (pActiveLayer != NULL))
      {
         bSuccess = pActiveLayer->processMouseRelease(ptMouse, pEvent->button(), pEvent->buttons(),
            pEvent->modifiers());
      }
      else if ((mouseMode == "MeasurementMode") && (mpMeasurementsLayer != NULL) && (mShowMeasurements == true))
      {
         bSuccess = mpMeasurementsLayer->processMouseRelease(ptMouse, pEvent->button(), pEvent->buttons(),
            pEvent->modifiers());
      }

      if (bSuccess == true)
      {
         pEvent->accept();
         refresh();
      }
   }

   if (bSuccess == false)
   {
      PerspectiveViewImp::mouseReleaseEvent(pEvent);
      if (pEvent->button() == Qt::MidButton && !isMousePanEnabled())
      {
         updateMouseCursor(getCurrentMouseMode());
      }
   }
}

void SpatialDataViewImp::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
   bool bSuccess = false;
   if (pEvent != NULL)
   {
      QPoint ptMouse = pEvent->pos();
      ptMouse.setY(height() - pEvent->pos().y());

      string mouseMode = "";

      const MouseMode* pMouseMode = getCurrentMouseMode();
      if (pMouseMode != NULL)
      {
         pMouseMode->getName(mouseMode);
      }

      LayerImp* pActiveLayer = dynamic_cast<LayerImp*>(mpActiveLayer.get());
      if ((mouseMode == "LayerMode") && (pActiveLayer != NULL))
      {
         bSuccess = pActiveLayer->processMouseDoubleClick(ptMouse, pEvent->button(), pEvent->buttons(),
            pEvent->modifiers());
      }
      else if ((mouseMode == "MeasurementMode") && (mpMeasurementsLayer != NULL) && (mShowMeasurements == true))
      {
         bSuccess = mpMeasurementsLayer->processMouseDoubleClick(ptMouse, pEvent->button(), pEvent->buttons(),
            pEvent->modifiers());
      }

      if (bSuccess == true)
      {
         pEvent->accept();
         refresh();
      }
   }

   if (bSuccess == false)
   {
      PerspectiveViewImp::mouseDoubleClickEvent(pEvent);
   }
}

void SpatialDataViewImp::setInsetPoint(const LocationType &worldCoord)
{
   if (mbLinking == false)
   {
      PerspectiveViewImp::setInsetPoint(worldCoord);
      LayerList* pLayerList = getLayerList();
      VERIFYNRV(pLayerList != NULL);
      RasterElement* pRaster = pLayerList->getPrimaryRasterElement();
      if (pRaster != NULL && pRaster->isGeoreferenced())
      {
         LocationType geoCoord = pRaster->convertPixelToGeocoord(worldCoord);
         void (ViewImp::*func)(const LocationType&) = &ViewImp::setInsetPoint;
         executeOnLinks<ViewImp, SpatialDataViewImp>(boost::bind(func, _1, worldCoord),
            boost::bind(&SpatialDataViewImp::setInsetPointGeo, _1, geoCoord));

      }
   }
}

void SpatialDataViewImp::setInsetPointGeo(const LocationType &geoCoord)
{
   if (mbLinking == false)
   {
      LocationType worldCoord;
      LayerList* pLayerList = getLayerList();
      VERIFYNRV(pLayerList != NULL);
      RasterElement* pRaster = pLayerList->getPrimaryRasterElement();
      if (pRaster != NULL && pRaster->isGeoreferenced())
      {
         worldCoord = pRaster->convertGeocoordToPixel(geoCoord);
         ViewImp::setInsetPoint(worldCoord);
      }
      void (ViewImp::*func)(const LocationType&) = &ViewImp::setInsetPoint;
      executeOnLinks<ViewImp, SpatialDataViewImp>(boost::bind(func, _1, worldCoord),
         boost::bind(&SpatialDataViewImp::setInsetPointGeo, _1, geoCoord));
   }
}

bool SpatialDataViewImp::canLinkWithView(View *pView, LinkType type)
{
   if (type == GEOCOORD_LINK)
   {
      SpatialDataViewImp* pSpatial = dynamic_cast<SpatialDataViewImp*>(pView);
      if (pSpatial != NULL)
      {
         LayerList* pLayerList = getLayerList();
         LayerList* pOtherLayerList = pSpatial->getLayerList();
         VERIFY(pOtherLayerList != NULL && pLayerList != NULL);

         RasterElement* pRaster = pLayerList->getPrimaryRasterElement();
         RasterElement* pOtherRaster = pLayerList->getPrimaryRasterElement();

         if (pRaster != NULL && pRaster->isGeoreferenced() && 
            pOtherRaster != NULL && pOtherRaster->isGeoreferenced())
         {
            return true;
         }
      }
   }
   return PerspectiveViewImp::canLinkWithView(pView, type);
}

double SpatialDataViewImp::getMaximumZoom() const
{
   return mMaxZoom;
}

void SpatialDataViewImp::setMaximumZoom(double dMaxZoom)
{
   if (dMaxZoom == mMaxZoom)
   {
      return;
   }

   mMaxZoom = dMaxZoom;
   notify(SIGNAL_NAME(Subject, Modified));
}

double SpatialDataViewImp::getMinimumZoom() const
{
   return mMinZoom;
}

void SpatialDataViewImp::setMinimumZoom(double dMinZoom)
{
   if (dMinZoom == mMinZoom)
   {
      return;
   }

   mMinZoom = dMinZoom;
   notify(SIGNAL_NAME(Subject, Modified));
}

PanLimitType SpatialDataViewImp::getPanLimit() const
{
   return mPanLimit;
}

void SpatialDataViewImp::setPanLimit(PanLimitType ePanLimit)
{
   if (ePanLimit == mPanLimit)
   {
      return;
   }

   mPanLimit = ePanLimit;
   updateExtents();
}

double SpatialDataViewImp::limitZoomPercentage(double dPercent)
{
   if (isInsetEnabled())
   {
      return dPercent;
   }

   dPercent = PerspectiveViewImp::limitZoomPercentage(dPercent);

   // dPercent/100 is basically the number of screen pixels per scene pixel.
   // Use it to calculate the number of scene pixels in each dimension that will
   // be visible in the view for this zoom percentage. Limit check to be sure
   // no less that mMinZoom pixels or more than mMaxZoom * short dimension pixels.
   double newPixSize = dPercent * 0.01;

   // get scene size
   double minX;
   double minY;
   double maxX;
   double maxY;
   getExtents(minX, minY, maxX, maxY);   // this will depend on the current pan limit
   double sceneWidth = abs(maxX-minX+1);
   double sceneHeight = abs(maxY-minY+1);

   // get screen size
   double screenWidth = width();
   double screenHeight = height();

   // ensure we can zoom down to 100% even on a tiny scene
   sceneWidth = max(sceneWidth, screenWidth/2);
   sceneHeight = max(sceneHeight, screenHeight/2);

   // calc max limits
   double maxSceneWidth = numeric_limits<double>::max();
   double maxSceneHeight = numeric_limits<double>::max();

   if (mMaxZoom != 0)
   {
      maxSceneWidth = sceneWidth / mMaxZoom;
      maxSceneHeight = sceneHeight / mMaxZoom;
   }

   // calc number scene pixels in view for dPercent based on x and y pixel size
   double xPixelSize = 1.0;
   double yPixelSize = 1.0;
   if (mpPrimaryRasterLayer.get() != NULL)
   {
      xPixelSize = mpPrimaryRasterLayer->getXScaleFactor();
      yPixelSize = mpPrimaryRasterLayer->getYScaleFactor();
   }

   double scenePixsInViewWidth = screenWidth / (newPixSize * xPixelSize);
   double scenePixsInViewHeight = screenHeight / (newPixSize * yPixelSize);

   // check limits
   if (scenePixsInViewWidth > maxSceneWidth)
   {
      scenePixsInViewWidth = maxSceneWidth;
   }
   else if (scenePixsInViewWidth < mMinZoom)
   {
      scenePixsInViewWidth = mMinZoom;
   }

   if (scenePixsInViewHeight > maxSceneHeight)
   {
      scenePixsInViewHeight = maxSceneHeight;
   }
   else if (scenePixsInViewHeight < mMinZoom)
   {
      scenePixsInViewHeight = mMinZoom;
   }

   // calc screen pixel size for limit checked view
   double limitedPixSizeX = screenWidth / (scenePixsInViewWidth * xPixelSize);
   double limitedPixSizeY = screenHeight / (scenePixsInViewHeight * yPixelSize);

   // set dPecent to limit checked zoom percentage
   dPercent = min(limitedPixSizeX, limitedPixSizeY) * 100.0;

   return dPercent;
}

LocationType SpatialDataViewImp::limitPanCenter(LocationType center)
{
   if (mPanLimit == NO_LIMIT || isInsetEnabled())
   {
      return center;
   }

   LocationType mins;
   LocationType maxs;
   getExtents(mins.mX, mins.mY, maxs.mX, maxs.mY);

   center.clampMinimum(mins);
   center.clampMaximum(maxs);

   return center;
}

void SpatialDataViewImp::updateContextMenu(Subject& subject, const string& signal, const boost::any& value)
{
   ContextMenu* pMenu = boost::any_cast<ContextMenu*>(value);
   if (pMenu == NULL)
   {
      return;
   }

   // Disable smoothing if a layer is using gpu images since it does not work on GL_FLOAT with NV extensions
   mpSmoothAction->setEnabled(isSmoothingAvailable());

   QObject* pParent = pMenu->getActionParent();

   // Check if the user clicked in this view
   if (dynamic_cast<SpatialDataViewImp*>(&subject) == this)
   {
      // Add a separator action
      QAction* pSeparatorAction = new QAction(pParent);
      pSeparatorAction->setSeparator(true);
      pMenu->addActionBefore(pSeparatorAction, APP_SPATIALDATAVIEW_LAYER_SEPARATOR_ACTION,
         APP_SPATIALDATAVIEW_NEW_LAYER_MENU_ACTION);

      // Add layer actions on submenus
      vector<Layer*> layers;
      mpLayerList->getLayers(layers);
      for (vector<Layer*>::iterator iter = layers.begin(); iter != layers.end(); ++iter)
      {
         Layer* pLayer = *iter;
         if (pLayer != NULL)
         {
            list<ContextMenuAction> layerActions = pLayer->getContextMenuActions();
            if (layerActions.empty() == false)
            {
               QMenu* pLayerMenu = new QMenu(QString::fromStdString(pLayer->getName()),
                  const_cast<SpatialDataViewImp*>(this));

               // Layer actions
               list<ContextMenuAction>::iterator actionIter;
               for (actionIter = layerActions.begin(); actionIter != layerActions.end(); ++actionIter)
               {
                  QAction* pAction = actionIter->mpAction;
                  if (pAction != NULL)
                  {
                     pLayerMenu->addAction(pAction);
                  }
               }

               // Separator
               pLayerMenu->addSeparator();

               // Activate
               if (pLayer != getActiveLayer())
               {
                  LayerImp* pLayerImp = dynamic_cast<LayerImp*>(pLayer);
                  if ((pLayerImp != NULL) && (pLayerImp->acceptsMouseEvents() == true))
                  {
                     QAction* pActivateAction = new QAction("Activate", pParent);
                     pActivateAction->setAutoRepeat(false);
                     pActivateAction->setData(QVariant::fromValue(pLayer));
                     VERIFYNR(connect(pActivateAction, SIGNAL(triggered()), this, SLOT(setActiveLayer())));
                     pLayerMenu->addAction(pActivateAction);
                     pLayerMenu->addSeparator();
                  }
               }

               // Copy
               QAction* pCopyAction = new QAction("Copy", pParent);
               pCopyAction->setAutoRepeat(false);
               pCopyAction->setData(QVariant::fromValue(pLayer));
               VERIFYNR(connect(pCopyAction, SIGNAL(triggered()), this, SLOT(copyLayer())));
               pLayerMenu->addAction(pCopyAction);

               // Convert
               if (getDerivedLayerTypes(pLayer).empty() == false)
               {
                  QAction* pConvertAction = new QAction("Convert...", pParent);
                  pConvertAction->setAutoRepeat(false);
                  pConvertAction->setData(QVariant::fromValue(pLayer));
                  VERIFYNR(connect(pConvertAction, SIGNAL(triggered()), this, SLOT(convertLayer())));
                  pLayerMenu->addAction(pConvertAction);
               }

               // Delete
               QAction* pDeleteAction = new QAction(QIcon(":/icons/Delete"), "Delete", pParent);
               pDeleteAction->setAutoRepeat(false);
               pDeleteAction->setData(QVariant::fromValue(pLayer));
               VERIFYNR(connect(pDeleteAction, SIGNAL(triggered()), this, SLOT(deleteLayer())));
               pLayerMenu->addAction(pDeleteAction);

               // Separator
               pLayerMenu->addSeparator();

               // Export
               QAction* pExportAction = new QAction(QIcon(":/icons/Save"), "Export", pParent);
               pExportAction->setAutoRepeat(false);
               pExportAction->setData(QVariant::fromValue(pLayer));
               VERIFYNR(connect(pExportAction, SIGNAL(triggered()), this, SLOT(exportLayer())));
               pLayerMenu->addAction(pExportAction);

               // Properties
               QAction* pPropertiesAction = new QAction(QIcon(QIcon(":/icons/Properties")), "Properties...", pParent);
               pPropertiesAction->setAutoRepeat(false);
               pPropertiesAction->setData(QVariant::fromValue(pLayer));
               VERIFYNR(connect(pPropertiesAction, SIGNAL(triggered()), this, SLOT(displayLayerProperties())));
               pLayerMenu->addAction(pPropertiesAction);

               // Add the layer submenu to the context menu
               pMenu->addActionBefore(pLayerMenu->menuAction(), APP_SPATIALDATAVIEW_LAYER_MENU_ACTION(pLayer),
                  APP_SPATIALDATAVIEW_LAYER_SEPARATOR_ACTION);
            }
         }
      }
   }
   else if (dynamic_cast<SessionExplorer*>(&subject) != NULL)
   {
      vector<SessionItem*> items = pMenu->getSessionItems();

      unsigned int numItems = items.size();
      if (numItems == 1)
      {
         Layer* pLayer = dynamic_cast<Layer*>(items.front());
         if ((pLayer != NULL) && (mpLayerList->containsLayer(pLayer) == true))
         {
            string beforeAction = APP_SESSIONEXPLORER_RENAME_ACTION;

            // Separator
            QAction* pSeparatorAction = new QAction(pParent);
            pSeparatorAction->setSeparator(true);
            pMenu->addActionBefore(pSeparatorAction, APP_SPATIALDATAVIEW_LAYER_DELETE_SEPARATOR_ACTION, beforeAction);
            beforeAction = APP_SPATIALDATAVIEW_LAYER_DELETE_SEPARATOR_ACTION;

            // Delete
            QAction* pDeleteAction = new QAction(QIcon(":/icons/Delete"), "Delete", pParent);
            pDeleteAction->setAutoRepeat(false);
            pDeleteAction->setData(QVariant::fromValue(pLayer));
            VERIFYNR(connect(pDeleteAction, SIGNAL(triggered()), this, SLOT(deleteLayer())));
            pMenu->addActionBefore(pDeleteAction, APP_SPATIALDATAVIEW_LAYER_DELETE_ACTION, beforeAction);
            beforeAction = APP_SPATIALDATAVIEW_LAYER_DELETE_ACTION;

            // Convert
            if (getDerivedLayerTypes(pLayer).empty() == false)
            {
               QAction* pConvertAction = new QAction("Convert...", pParent);
               pConvertAction->setAutoRepeat(false);
               pConvertAction->setData(QVariant::fromValue(pLayer));
               VERIFYNR(connect(pConvertAction, SIGNAL(triggered()), this, SLOT(convertLayer())));
               pMenu->addActionBefore(pConvertAction, APP_SPATIALDATAVIEW_LAYER_CONVERT_ACTION, beforeAction);
               beforeAction = APP_SPATIALDATAVIEW_LAYER_CONVERT_ACTION;
            }

            // Copy
            QAction* pCopyAction = new QAction("Copy", pParent);
            pCopyAction->setAutoRepeat(false);
            pCopyAction->setData(QVariant::fromValue(pLayer));
            VERIFYNR(connect(pCopyAction, SIGNAL(triggered()), this, SLOT(copyLayer())));
            pMenu->addActionBefore(pCopyAction, APP_SPATIALDATAVIEW_LAYER_COPY_ACTION, beforeAction);
            beforeAction = APP_SPATIALDATAVIEW_LAYER_COPY_ACTION;

            // Select layers of type
            pMenu->addActionBefore(mpSelectLayersOfTypeAction, APP_SPATIALDATAVIEW_SELECT_LAYERS_OF_TYPE_ACTION,
               beforeAction);
            beforeAction = APP_SPATIALDATAVIEW_SELECT_LAYERS_OF_TYPE_ACTION;

            // Activate
            if (pLayer != getActiveLayer())
            {
               LayerImp* pLayerImp = dynamic_cast<LayerImp*>(pLayer);
               if ((pLayerImp != NULL) && (pLayerImp->acceptsMouseEvents() == true))
               {
                  // Separator
                  QAction* pActivateSeparatorAction = new QAction(pParent);
                  pActivateSeparatorAction->setSeparator(true);
                  pMenu->addActionBefore(pActivateSeparatorAction, APP_SPATIALDATAVIEW_LAYER_ACTIVATE_SEPARATOR_ACTION,
                     beforeAction);
                  beforeAction = APP_SPATIALDATAVIEW_LAYER_ACTIVATE_SEPARATOR_ACTION;

                  // Activate
                  QAction* pActivateAction = new QAction("Activate", pParent);
                  pActivateAction->setAutoRepeat(false);
                  pActivateAction->setData(QVariant::fromValue(pLayer));
                  VERIFYNR(connect(pActivateAction, SIGNAL(triggered()), this, SLOT(setActiveLayer())));
                  pMenu->addActionBefore(pActivateAction, APP_SPATIALDATAVIEW_LAYER_ACTIVATE_ACTION, beforeAction);
                  beforeAction = APP_SPATIALDATAVIEW_LAYER_ACTIVATE_ACTION;
               }
            }

            // Separator
            QAction* pLayerSeparatorAction = new QAction(pParent);
            pLayerSeparatorAction->setSeparator(true);
            pMenu->addActionBefore(pLayerSeparatorAction, APP_SPATIALDATAVIEW_LAYER_SEPARATOR_ACTION, beforeAction);
         }
      }
      else if (numItems > 1)
      {
         vector<Layer*> layers = pMenu->getSessionItems<Layer>();
         if (layers.size() == numItems)
         {
            // Show all
            pMenu->addAction(mpShowLayersAction, APP_SPATIALDATAVIEW_SHOW_LAYERS_ACTION);

            // Hide all
            pMenu->addActionAfter(mpHideLayersAction, APP_SPATIALDATAVIEW_HIDE_LAYERS_ACTION,
               APP_SPATIALDATAVIEW_SHOW_LAYERS_ACTION);

            // Select layers of type
            pMenu->addActionAfter(mpSelectLayersOfTypeAction, APP_SPATIALDATAVIEW_SELECT_LAYERS_OF_TYPE_ACTION,
               APP_SPATIALDATAVIEW_HIDE_LAYERS_ACTION);
         }
      }
   }
}

void SpatialDataViewImp::updateMouseCursor(const MouseMode* pMouseMode)
{
   const MouseModeImp* pMouseModeImp = static_cast<const MouseModeImp*>(pMouseMode);
   if (pMouseModeImp != NULL)
   {
      QString strModeName = pMouseModeImp->getName();
      if (strModeName == "LayerMode")
      {
         QCursor mouseCursor = pMouseModeImp->getCursor();

         LayerImp* pActiveLayer = dynamic_cast<LayerImp*>(mpActiveLayer.get());
         if (pActiveLayer != NULL)
         {
            mouseCursor = pActiveLayer->getMouseCursor();
         }

         setCursor(mouseCursor);
      }
   }
}

void SpatialDataViewImp::updateStatusBar()
{
   QPoint globalPoint = QCursor::pos();
   QPoint viewPoint = mapFromGlobal(globalPoint);

   if ((viewPoint.x() >= 0) && (viewPoint.x() < width()) && (viewPoint.y() >= 0) && (viewPoint.y() < height()))
   {
      viewPoint.setY(height() - viewPoint.y());
      updateStatusBar(viewPoint);
   }
}

void SpatialDataViewImp::createLayer(QAction* pAction)
{
   Layer* pLayer = NULL;
   if (pAction == mpAnnotationAction)
   {
      pLayer = createLayer(ANNOTATION);
   }
   else if (pAction == mpAoiAction)
   {
      pLayer = createLayer(AOI_LAYER);
   }
   else if (pAction == mpGcpAction)
   {
      pLayer = createLayer(GCP_LAYER);
   }
   else if (pAction == mpTiePointAction)
   {
      pLayer = createLayer(TIEPOINT_LAYER);
   }

   if (pLayer != NULL)
   {
      setMouseMode("LayerMode");
      setActiveLayer(pLayer);
   }
   else
   {
      QMessageBox::critical(this, windowTitle(), "Could not create the new layer!");
   }
}

void SpatialDataViewImp::copyLayer()
{
   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction != NULL)
   {
      Layer* pLayer = pAction->data().value<Layer*>();
      if (pLayer != NULL)
      {
         LayerType layerType = pLayer->getLayerType();
         if (deriveLayer(pLayer, layerType) == NULL)
         {
            QMessageBox::critical(this, windowTitle(), "Could not copy the layer!");
         }
      }
   }
}

void SpatialDataViewImp::convertLayer()
{
   Layer* pLayer = NULL;

   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction != NULL)
   {
      pLayer = pAction->data().value<Layer*>();
   }

   if (pLayer == NULL)
   {
      return;
   }

   // Prevent converting the primary raster layer
   if (pLayer == mpPrimaryRasterLayer.get())
   {
      QMessageBox::critical(this, windowTitle(), "The primary raster layer cannot be converted!");
      return;
   }

   // Get the list of supported conversion types from the layer
   QStringList layerTypes;

   vector<LayerType> derivedTypes = getDerivedLayerTypes(pLayer);
   for (vector<LayerType>::iterator iter = derivedTypes.begin(); iter != derivedTypes.end(); ++iter)
   {
      LayerType layerType = *iter;

      string typeName = StringUtilities::toDisplayString(layerType);
      if (typeName.empty() == false)
      {
         layerTypes.append(QString::fromStdString(typeName));
      }
   }

   if (layerTypes.empty() == true)
   {
      return;
   }

   // Get the conversion type from the user
   bool bSuccess = false;

   QString strLayerType = QInputDialog::getItem(this, "Convert Layer", "Layer Type:", layerTypes, 0, false, &bSuccess);
   if ((bSuccess == false) || (strLayerType.isEmpty() == true))
   {
      return;
   }

   LayerType newlayerType =
      StringUtilities::fromDisplayString<LayerType>(strLayerType.toStdString());

   // Convert the layer
   Layer* pNewLayer = convertLayer(pLayer, newlayerType);
   if (pNewLayer == NULL)
   {
      QMessageBox::critical(this, windowTitle(), "Could not convert the layer to a new " + strLayerType + " layer!");
   }
}

void SpatialDataViewImp::setActiveLayer()
{
   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction != NULL)
   {
      Layer* pLayer = pAction->data().value<Layer*>();
      if (pLayer != NULL)
      {
         setMouseMode("LayerMode");
         setActiveLayer(pLayer);
      }
   }
}

void SpatialDataViewImp::showLayers()
{
   UndoGroup group(dynamic_cast<View*>(this), "Show Layers");
   std::vector<Layer*> layers = Service<SessionExplorer>()->getSelectedSessionItems<Layer>();
   for (std::vector<Layer*>::iterator iter = layers.begin(); iter != layers.end(); ++iter)
   {
      Layer* pLayer = *iter;
      if (pLayer != NULL && mpLayerList->containsLayer(pLayer))
      {
         showLayer(pLayer);
      }
   }
   refresh();
}

void SpatialDataViewImp::hideLayers()
{
   UndoGroup group(dynamic_cast<View*>(this), "Hide Layers");
   std::vector<Layer*> layers = Service<SessionExplorer>()->getSelectedSessionItems<Layer>();
   for (std::vector<Layer*>::iterator iter = layers.begin(); iter != layers.end(); ++iter)
   {
      Layer* pLayer = *iter;
      if (pLayer != NULL && mpLayerList->containsLayer(pLayer))
      {
         hideLayer(pLayer);
      }
   }
   refresh();
}

void SpatialDataViewImp::selectLayersOfType()
{
   std::vector<Layer*> layers = Service<SessionExplorer>()->getSelectedSessionItems<Layer>();
   std::set<std::pair<SpatialDataView*, LayerType> > layerTypes;
   for (std::vector<Layer*>::iterator iter = layers.begin(); iter != layers.end(); ++iter)
   {
      Layer* pLayer = *iter;
      if (pLayer != NULL && mpLayerList->containsLayer(pLayer))
      {
         SpatialDataView* pSdv = dynamic_cast<SpatialDataView*>(pLayer->getView());
         if (pSdv != NULL)
         {
            layerTypes.insert(std::make_pair(pSdv, pLayer->getLayerType()));
         }
      }
   }
   std::vector<SessionItem*> newLayerSelection;
   for (std::set<std::pair<SpatialDataView*, LayerType> >::iterator typeIter = layerTypes.begin();
        typeIter != layerTypes.end();
        ++typeIter)
   {
      std::vector<Layer*> curLayers;
      typeIter->first->getLayerList()->getLayers(typeIter->second, curLayers);
      for (std::vector<Layer*>::iterator curIter = curLayers.begin(); curIter != curLayers.end(); ++curIter)
      {
         newLayerSelection.push_back(*curIter);
      }
   }
   Service<SessionExplorer>()->setSelectedSessionItems(newLayerSelection);
}

void SpatialDataViewImp::deleteLayer()
{
   Layer* pLayer = NULL;

   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction != NULL)
   {
      pLayer = pAction->data().value<Layer*>();
   }

   if (pLayer == NULL)
   {
      return;
   }

   // Prevent deleting the primary raster layer
   if (pLayer == mpPrimaryRasterLayer.get())
   {
      QMessageBox::critical(this, windowTitle(), "The primary raster layer cannot be deleted!");
      return;
   }

   // Display non-undoable layer deletion warning
   bool bUniqueElement = pLayer->hasUniqueElement();
   if ((bUniqueElement == true) && (SpatialDataView::getSettingConfirmLayerDelete() == true))
   {
      string layerName = pLayer->getName();
      VERIFYNRV(layerName.empty() == false);

      QString msg = "Deleting the '" + QString::fromStdString(layerName) +
         "' layer will also delete the associated data element.  This action can not be undone "
         "and will cause all undo history for all views to be cleared.  Do you want to continue?";

      int iReturn = QMessageBox::warning(this, windowTitle(), msg, "Con&tinue", "&Cancel");
      if (iReturn == 1)    // Cancel
      {
         return;
      }
   }

   // Delete the layer
   deleteLayer(pLayer);
}

void SpatialDataViewImp::exportLayer()
{
   Layer* pLayer = NULL;

   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction != NULL)
   {
      pLayer = pAction->data().value<Layer*>();
   }

   if (pLayer == NULL)
   {
      return;
   }

   Service<DesktopServices> pDesktop;

   ApplicationWindow* pAppWindow = dynamic_cast<ApplicationWindow*>(pDesktop->getMainWidget());
   if (pAppWindow != NULL)
   {
      pAppWindow->exportSessionItem(pLayer);
   }
}

void SpatialDataViewImp::displayLayerProperties()
{
   Layer* pLayer = NULL;

   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction != NULL)
   {
      pLayer = pAction->data().value<Layer*>();
   }

   if (pLayer == NULL)
   {
      return;
   }

   Service<DesktopServices> pDesktop;

   ApplicationWindow* pAppWindow = dynamic_cast<ApplicationWindow*>(pDesktop->getMainWidget());
   if (pAppWindow != NULL)
   {
      pAppWindow->displayProperties(pLayer);
   }
}

void SpatialDataViewImp::setOrigin(QAction* pAction)
{
   if (pAction == mpOriginLL)
   {
      if (getDataOrigin() != LOWER_LEFT)
      {
         setDataOrigin(LOWER_LEFT);
         resetOrientation();
         refresh();
      }
   }
   else if (pAction == mpOriginUL)
   {
      if (getDataOrigin() != UPPER_LEFT)
      {
         setDataOrigin(UPPER_LEFT);
         resetOrientation();
         refresh();
      }
   }
}

void SpatialDataViewImp::updateOriginAction(const DataOrigin& origin)
{
   if (origin == UPPER_LEFT)
   {
      mpOriginUL->setChecked(true);
   }
   else if (origin == LOWER_LEFT)
   {
      mpOriginLL->setChecked(true);
   }
}

void SpatialDataViewImp::setCrossHair(bool bEnabled)
{
   if (bEnabled != isCrossHairEnabled())
   {
      enableCrossHair(bEnabled);
      refresh();
   }
}

void SpatialDataViewImp::setSmoothing(bool enabled)
{
   TextureMode currentMode = getTextureMode();
   TextureMode newMode;
   if (enabled && isSmoothingAvailable())
   {
      newMode = TEXTURE_LINEAR;
   }
   else
   {
      newMode = TEXTURE_NEAREST_NEIGHBOR;
   }

   if (newMode != currentMode)
   {
      setTextureMode(newMode);
      refresh();
   }
}

void SpatialDataViewImp::updateSmoothingAction(const TextureMode& textureMode)
{
   mpSmoothAction->setChecked(textureMode == TEXTURE_LINEAR);
}

void SpatialDataViewImp::changeBkgColor()
{
   QColor bkgColor = getBackgroundColor();
   QColor newBkg = QColorDialog::getColor(bkgColor, this);
   if (newBkg.isValid() && newBkg!=bkgColor)
   {
      setBackgroundColor(newBkg);
      refresh();
   }
}

void SpatialDataViewImp::displayMeasurementProperties()
{
   Service<DesktopServices> pDesktop;
   pDesktop->displayProperties(mpMeasurementsLayer);
}

bool SpatialDataViewImp::toXml(XMLWriter* pXml) const
{
   if (!PerspectiveViewImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("textureMode", mTextureMode);
   pXml->addAttr("showMeasurements", mShowMeasurements);
   pXml->addAttr("panKey", mPanKey);
   pXml->addAttr("panLimit", mPanLimit);
   pXml->addAttr("minZoom", mMinZoom);
   pXml->addAttr("maxZoom", mMaxZoom);

   if (mpActiveLayer.get() != NULL)
   {
      pXml->addAttr("activeLayerId", mpActiveLayer->getId());
   }

   if (mpDrawLayer != NULL)
   {
      pXml->addAttr("drawLayerId", mpDrawLayer->getId());
   }

   if (mpMeasurementsLayer != NULL)
   {
      pXml->addAttr("measurementLayerId", mpMeasurementsLayer->getId());
   }

   VERIFY(mpLayerList != NULL);
   RasterElement* pElement = mpLayerList->getPrimaryRasterElement();
   VERIFY(pElement != NULL);
   pXml->addAttr("primary", pElement->getId());
   vector<Layer*> layers;
   mpLayerList->getLayers(layers);
   for (vector<Layer*>::const_iterator lit = layers.begin(); lit != layers.end(); ++lit)
   {
      pXml->pushAddPoint(pXml->addElement("layer"));
      pXml->addAttr("id", (*lit)->getId());
      if (*lit == mpPrimaryRasterLayer.get())
      {
         pXml->addAttr("primary", true);
      }
      pXml->popAddPoint();
   }

   return true;
}

bool SpatialDataViewImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (!PerspectiveViewImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   setTextureMode(StringUtilities::fromXmlString<TextureMode>(A(pElem->getAttribute(X("textureMode")))));
   mShowMeasurements = StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("showMeasurements"))));
   mPanKey = StringUtilities::fromXmlString<int>(A(pElem->getAttribute(X("panKey"))));
   setPanLimit(StringUtilities::fromXmlString<PanLimitType>(A(pElem->getAttribute(X("panLimit")))));
   setMinimumZoom(StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("minZoom")))));
   setMaximumZoom(StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("maxZoom")))));

   if (pElem->hasAttribute(X("activeLayerId")))
   {
      mpActiveLayer.reset(dynamic_cast<Layer*>(
         SessionManagerImp::instance()->getSessionItem(A(pElem->getAttribute(X("activeLayerId"))))));
   }
   else
   {
      mpActiveLayer.reset(NULL);
   }

   if (pElem->hasAttribute(X("drawLayerId")))
   {
      mpDrawLayer = dynamic_cast<LayerImp*>(SessionManagerImp::instance()->getSessionItem(
         A(pElem->getAttribute(X("drawLayerId")))));
   }
   else
   {
      mpDrawLayer = NULL;
   }

   if (pElem->hasAttribute(X("measurementLayerId")))
   {
      MeasurementLayerAdapter* pMeasLayer = dynamic_cast<MeasurementLayerAdapter*>(
         SessionManagerImp::instance()->getSessionItem(A(pElem->getAttribute(X("measurementLayerId")))));
      if (pMeasLayer != NULL)
      {
         VERIFY(mpMeasurementsLayer != NULL);
         *mpMeasurementsLayer = *pMeasLayer;
         GraphicGroupImp* pCurGroup = dynamic_cast<GraphicGroupImp*>(mpMeasurementsLayer->getGroup());  
         GraphicGroup* pRestorGroup = pMeasLayer->getGroup();
         if (pCurGroup != NULL && pRestorGroup != NULL)
         {
            pCurGroup->replicateObject(pRestorGroup);
         }
         delete pMeasLayer;
      }
   }

   VERIFY(mpLayerList != NULL);
   mpLayerList->setPrimaryRasterElement(dynamic_cast<RasterElement*>(
      SessionManagerImp::instance()->getSessionItem(A(pElem->getAttribute(X("primary"))))));
   for (DOMNode *pNode = pElem->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(), X("layer")))
      {
         DOMElement* pLayerElement = static_cast<DOMElement*>(pNode);
         Layer* pLayer = dynamic_cast<Layer*>(
            SessionManagerImp::instance()->getSessionItem(A(pLayerElement->getAttribute(X("id")))));
         if (pLayer != NULL)
         {
            addLayer(pLayer);
            if (pLayerElement->hasAttribute(X("primary")))
            {
               mpPrimaryRasterLayer.reset(dynamic_cast<RasterLayer*>(pLayer));
            }
         }
      }
   }

   return true;
}

void SpatialDataViewImp::setGrayBand()
{
   setBand(GRAY);
}

void SpatialDataViewImp::setRedBand()
{
   setBand(RED);
}

void SpatialDataViewImp::setGreenBand()
{
   setBand(GREEN);
}

void SpatialDataViewImp::setBlueBand()
{
   setBand(BLUE);
}

void SpatialDataViewImp::setBand(RasterChannelType channel)
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(getTopMostLayer(RASTER));
   if (pRasterLayer != NULL)
   {
      if (sKeyboardNumber > 0)
      {
         sKeyboardNumber--;
      }

      DataElement* pElement = pRasterLayer->getDataElement();
      if (pElement != NULL)
      {
         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            const vector<DimensionDescriptor>& activeBands = pDescriptor->getBands();
            for (unsigned int i = 0; i < activeBands.size(); ++i)
            {
               DimensionDescriptor bandDim = activeBands[i];
               if (bandDim.isValid())
               {
                  unsigned int currentNumber = bandDim.getOriginalNumber();
                  if (currentNumber == sKeyboardNumber)
                  {
                     pRasterLayer->setDisplayedBand(channel, bandDim);
                     break;
                  }
               }
            }
         }
      }
   }

   sKeyboardNumber = 0;
}

void SpatialDataViewImp::setGrayscaleMode()
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(getTopMostLayer(RASTER));
   if (pRasterLayer != NULL)
   {
      pRasterLayer->setDisplayMode(GRAYSCALE_MODE);
   }

   sKeyboardNumber = 0;
}

void SpatialDataViewImp::setRgbMode()
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(getTopMostLayer(RASTER));
   if (pRasterLayer != NULL)
   {
      pRasterLayer->setDisplayMode(RGB_MODE);
   }

   sKeyboardNumber = 0;
}

void SpatialDataViewImp::nextBand()
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(getTopMostLayer(RASTER));
   if ((pRasterLayer == NULL) || (pRasterLayer->getDisplayMode() == RGB_MODE))
   {
      return;
   }

   DataElement* pElement = pRasterLayer->getDataElement();
   if (pElement == NULL)
   {
      return;
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   DimensionDescriptor currentBand = pRasterLayer->getDisplayedBand(GRAY);
   DimensionDescriptor newBand;

   const vector<DimensionDescriptor>& activeBands = pDescriptor->getBands();
   if (currentBand.isValid() == false)
   {
      if (activeBands.empty() == false)
      {
         newBand = activeBands.front();
      }
   }
   else
   {
      bool bFound = false;
      for (vector<DimensionDescriptor>::const_iterator iter = activeBands.begin(); iter != activeBands.end(); ++iter)
      {
         DimensionDescriptor activeBand = *iter;
         if (bFound == true)
         {
            newBand = activeBand;
            break;
         }

         if (activeBand == currentBand)
         {
            bFound = true;
         }
      }
   }

   pRasterLayer->setDisplayedBand(GRAY, newBand);
}

void SpatialDataViewImp::previousBand()
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(getTopMostLayer(RASTER));
   if ((pRasterLayer == NULL) || (pRasterLayer->getDisplayMode() == RGB_MODE))
   {
      return;
   }

   DataElement* pElement = pRasterLayer->getDataElement();
   if (pElement == NULL)
   {
      return;
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   DimensionDescriptor currentBand = pRasterLayer->getDisplayedBand(GRAY);
   DimensionDescriptor newBand;

   const vector<DimensionDescriptor>& activeBands = pDescriptor->getBands();
   if (currentBand.isValid() == false)
   {
      if (activeBands.empty() == false)
      {
         newBand = activeBands.back();
      }
   }
   else
   {
      bool bFound = false;

      vector<DimensionDescriptor>::const_reverse_iterator iter;
      for (iter = activeBands.rbegin(); iter != activeBands.rend(); ++iter)
      {
         DimensionDescriptor activeBand = *iter;
         if (bFound == true)
         {
            newBand = activeBand;
            break;
         }

         if (activeBand == currentBand)
         {
            bFound = true;
         }
      }
   }

   pRasterLayer->setDisplayedBand(GRAY, newBand);
}

bool SpatialDataViewImp::linkView(View *pView, LinkType type)
{
   if (type != NO_LINK && getViewLinkType(pView) == NO_LINK)
   {
      SpatialDataView* pSourceView = dynamic_cast<SpatialDataView*>(pView);
      if (pSourceView != NULL)
      {
         if (pSourceView->getMaximumZoom() != 0 || pSourceView->getMinimumZoom() != 0 ||
            pSourceView->getPanLimit() != NO_LIMIT)
         {
            int buttonPressed = QMessageBox::information(this, QString::fromStdString(getDisplayName()),
               "Zoom/Pan limits will be disabled for this " + QString::fromStdString(pSourceView->getDisplayName()) +
               " view. Do you want to continue linking the views?", QMessageBox::Ok, QMessageBox::Cancel);

            if (buttonPressed == QMessageBox::Cancel)
            {
               return false;
            }
         }

         pSourceView->setMaximumZoom(0);
         pSourceView->setMinimumZoom(0);
         pSourceView->setPanLimit(NO_LIMIT);
      }
   }

   return ViewImp::linkView(pView, type);
}

void SpatialDataViewImp::setAoiMode()
{
   Service<DesktopServices> pDesktop;

   AoiToolBar* pToolbar = static_cast<AoiToolBar*>(pDesktop->getWindow("AOI", TOOLBAR));
   if (pToolbar != NULL)
   {
      pToolbar->setSelectionTool(pToolbar->getSelectionTool(), DRAW);
   }
}

double SpatialDataViewImp::getMousePanScaleFactor() const
{
   double zoomFactor = getZoomPercentage() / 100.0;
   double scaleFactor = PerspectiveViewImp::getMousePanScaleFactor() * sqrt(zoomFactor);
   return scaleFactor;
}

bool SpatialDataViewImp::isSmoothingAvailable() const
{
   VERIFY(mpLayerList != NULL);
   vector<Layer*> layers;
   mpLayerList->getLayers(RASTER, layers);
   for (vector<Layer*>::iterator iter = layers.begin(); iter != layers.end(); ++iter)
   {
      RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(*iter);
      if (pRasterLayer != NULL && pRasterLayer->isGpuImageEnabled())
      {
         return false;
      }
   }

   return true;
}

void SpatialDataViewImp::resetStretch()
{
   VERIFYNR(mpLayerList != NULL);
   UndoGroup group(dynamic_cast<View*>(this), "Reset Stretch for Visible Layers");
   vector<Layer*> layers;
   mpLayerList->getLayers(RASTER, layers);
   for (vector<Layer*>::iterator iter = layers.begin(); iter != layers.end(); ++iter)
   {
      RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(*iter);
      if (pRasterLayer != NULL && isLayerDisplayed(pRasterLayer) == true)
      {
         pRasterLayer->resetStretch();
      }
   }
}
