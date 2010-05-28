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
#include <QtGui/QFileDialog>
#include <QtGui/QIcon>
#include <QtGui/QInputDialog>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QWidgetAction>

#include "AppVerify.h"
#include "AppVersion.h"
#include "BandBadValuesDlg.h"
#include "ClassificationAdapter.h"
#include "ColormapEditor.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "HistogramAdapter.h"
#include "HistogramPlotImp.h"
#include "HistogramPlotAdapter.h"
#include "LocatorAdapter.h"
#include "MouseMode.h"
#include "PropertiesHistogramPlot.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterLayerAdapter.h"
#include "RasterUtilities.h"
#include "RegionObjectAdapter.h"
#include "SecurityMarkingsDlg.h"
#include "SessionManager.h"
#include "Statistics.h"
#include "StringUtilities.h"
#include "ThresholdLayerAdapter.h"
#include "Undo.h"
#include "UtilityServicesImp.h"
#include "XercesIncludes.h"
#include "xmlreader.h"

#include <vector>
using namespace std;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMNode;
using XERCES_CPP_NAMESPACE_QUALIFIER XMLString;

static void ShowRangeValues(char* pFormat, double lower, double upper = 0.0);

HistogramPlotImp::HistogramPlotImp(const string& id, const string& viewName, QGLContext* pDrawContext,
                                   QWidget* pParent) :
   CartesianPlotImp(id, viewName, pDrawContext, pParent),
   mpHistogram(NULL),
   mpRegion(NULL),
   mpRegion2(NULL),
   mpAlternateColormap(NULL),
   mUpdater(this),
   meSelectedValue(NO_VALUE),
   mRasterChannelType(GRAY),
   mpLayer(NULL),
   mpElement(NULL),
   mAutoZoom(true),
   mpBelowAction(NULL),
   mpAboveAction(NULL),
   mpBetweenAction(NULL),
   mpOutsideAction(NULL),
   mpLinearAction(NULL),
   mpExponentialAction(NULL),
   mpLogarithmicAction(NULL),
   mpEqualizationAction(NULL),
   mpRawAction(NULL),
   mpPercentageAction(NULL),
   mpPercentileAction(NULL),
   mpStdDevAction(NULL),
   mpMagnitudeAction(NULL),
   mpPhaseAction(NULL),
   mpInPhaseAction(NULL),
   mpQuadratureAction(NULL),
   mpPassAreaMenu(NULL),
   mpStretchUnitsMenu(NULL),
   mpStretchTypeMenu(NULL),
   mpStretchResetMenu(NULL),
   mpElementMenu(NULL),
   mpElementList(NULL),
   mpBandMenu(NULL),
   mpBandList(NULL),
   mpComplexDataMenu(NULL),
   mpColorMapMenu(NULL),
   mpColorMapList(NULL),
   mpSaveAction(NULL),
   mpBadValuesAction(NULL),
   mpThresholdSeparatorAction(NULL),
   mpRasterSeparatorAction(NULL),
   mpAutoZoomAction(NULL),
   mpRasterMenusSeparatorAction(NULL),
   mpSamplingAction(NULL),
   mpEndSeparatorAction(NULL)
{
   // Context menu actions
   Service<DesktopServices> pDesktop;
   string shortcutContext = "Histogram Plot";

   // Pass area menu
   mpPassAreaMenu = new QMenu("&PassArea", this);
   QActionGroup* pPassAreaGroup = new QActionGroup(mpPassAreaMenu);
   pPassAreaGroup->setExclusive(true);

   string passAreaContext = shortcutContext + string("/Threshold Pass Area");

   mpBelowAction = pPassAreaGroup->addAction("&Below");
   mpBelowAction->setAutoRepeat(false);
   mpBelowAction->setCheckable(true);
   pDesktop->initializeAction(mpBelowAction, passAreaContext);

   mpAboveAction = pPassAreaGroup->addAction("&Above");
   mpAboveAction->setAutoRepeat(false);
   mpAboveAction->setCheckable(true);
   pDesktop->initializeAction(mpAboveAction, passAreaContext);

   mpBetweenAction = pPassAreaGroup->addAction("Be&tween");
   mpBetweenAction->setAutoRepeat(false);
   mpBetweenAction->setCheckable(true);
   pDesktop->initializeAction(mpBetweenAction, passAreaContext);

   mpOutsideAction = pPassAreaGroup->addAction("&Outside");
   mpOutsideAction->setAutoRepeat(false);
   mpOutsideAction->setCheckable(true);
   pDesktop->initializeAction(mpOutsideAction, passAreaContext);

   mpPassAreaMenu->addActions(pPassAreaGroup->actions());
   VERIFYNR(connect(pPassAreaGroup, SIGNAL(triggered(QAction*)), this, SLOT(setThresholdMode(QAction*))));

   // Stretch type menu
   mpStretchTypeMenu = new QMenu("Stretch &Type", this);
   QActionGroup* pStretchTypeGroup = new QActionGroup(mpStretchTypeMenu);
   pStretchTypeGroup->setExclusive(true);

   string stretchTypeContext = shortcutContext + string("/Stretch Type");

   mpLinearAction = pStretchTypeGroup->addAction("&Linear");
   mpLinearAction->setAutoRepeat(false);
   mpLinearAction->setCheckable(true);
   pDesktop->initializeAction(mpLinearAction, stretchTypeContext);

   mpExponentialAction = pStretchTypeGroup->addAction("&Exponential");
   mpExponentialAction->setAutoRepeat(false);
   mpExponentialAction->setCheckable(true);
   pDesktop->initializeAction(mpExponentialAction, stretchTypeContext);

   mpLogarithmicAction = pStretchTypeGroup->addAction("L&ogarithmic");
   mpLogarithmicAction->setAutoRepeat(false);
   mpLogarithmicAction->setCheckable(true);
   pDesktop->initializeAction(mpLogarithmicAction, stretchTypeContext);

   mpEqualizationAction = pStretchTypeGroup->addAction("H&istogram Equalization");
   mpEqualizationAction->setAutoRepeat(false);
   mpEqualizationAction->setCheckable(true);
   pDesktop->initializeAction(mpEqualizationAction, stretchTypeContext);

   mpStretchTypeMenu->addActions(pStretchTypeGroup->actions());
   VERIFYNR(connect(pStretchTypeGroup, SIGNAL(triggered(QAction*)), this, SLOT(setStretchMode(QAction*))));

   // Stretch units menu
   mpStretchUnitsMenu = new QMenu("Stretch &Units", this);
   QActionGroup* pStretchUnitsGroup = new QActionGroup(mpStretchUnitsMenu);
   pStretchUnitsGroup->setExclusive(true);

   string stretchUnitsContext = shortcutContext + string("/Stretch Units");

   mpRawAction = pStretchUnitsGroup->addAction("&Raw Values");
   mpRawAction->setAutoRepeat(false);
   mpRawAction->setCheckable(true);
   pDesktop->initializeAction(mpRawAction, stretchUnitsContext);

   mpPercentageAction = pStretchUnitsGroup->addAction("Percenta&ge");
   mpPercentageAction->setAutoRepeat(false);
   mpPercentageAction->setCheckable(true);
   pDesktop->initializeAction(mpPercentageAction, stretchUnitsContext);

   mpPercentileAction = pStretchUnitsGroup->addAction("Percenti&le");
   mpPercentileAction->setAutoRepeat(false);
   mpPercentileAction->setCheckable(true);
   pDesktop->initializeAction(mpPercentileAction, stretchUnitsContext);

   mpStdDevAction = pStretchUnitsGroup->addAction("&Standard Deviation");
   mpStdDevAction->setAutoRepeat(false);
   mpStdDevAction->setCheckable(true);
   pDesktop->initializeAction(mpStdDevAction, stretchUnitsContext);

   mpStretchUnitsMenu->addActions(pStretchUnitsGroup->actions());
   VERIFYNR(connect(pStretchUnitsGroup, SIGNAL(triggered(QAction*)), this, SLOT(setStretchUnits(QAction*))));

   // Stretch menu
   mpStretchResetMenu = new QMenu("Stretch &Reset", this);
   string stretchResetContext = shortcutContext + string("/Stretch Reset");

   QAction* pResetThisChannelAction = mpStretchResetMenu->addAction("&This Channel");
   pResetThisChannelAction->setAutoRepeat(false);
   pDesktop->initializeAction(pResetThisChannelAction, stretchResetContext);
   VERIFYNR(connect(pResetThisChannelAction, SIGNAL(triggered()), this, SLOT(stretchResetThisChannel())));

   QAction* pResetAllChannelsAction = mpStretchResetMenu->addAction(QIcon(":/icons/ResetStretch"), "&All Channels");
   pResetAllChannelsAction->setAutoRepeat(false);
   pDesktop->initializeAction(pResetAllChannelsAction, stretchResetContext);
   VERIFYNR(connect(pResetAllChannelsAction, SIGNAL(triggered()), this, SLOT(stretchResetAllChannels())));

   // Complex data menu
   mpComplexDataMenu = new QMenu("Comple&x Data", this);
   QActionGroup* pComplexDataGroup = new QActionGroup(mpComplexDataMenu);
   pComplexDataGroup->setExclusive(true);

   string complexDataContext = shortcutContext + string("/Complex Data");

   mpMagnitudeAction = pComplexDataGroup->addAction("&Magnitude");
   mpMagnitudeAction->setAutoRepeat(false);
   mpMagnitudeAction->setCheckable(true);
   pDesktop->initializeAction(mpMagnitudeAction, complexDataContext);

   mpPhaseAction = pComplexDataGroup->addAction("&Phase");
   mpPhaseAction->setAutoRepeat(false);
   mpPhaseAction->setCheckable(true);
   pDesktop->initializeAction(mpPhaseAction, complexDataContext);

   mpInPhaseAction = pComplexDataGroup->addAction("&In-Phase");
   mpInPhaseAction->setAutoRepeat(false);
   mpInPhaseAction->setCheckable(true);
   pDesktop->initializeAction(mpInPhaseAction, complexDataContext);

   mpQuadratureAction = pComplexDataGroup->addAction("&Quadrature");
   mpQuadratureAction->setAutoRepeat(false);
   mpQuadratureAction->setCheckable(true);
   pDesktop->initializeAction(mpQuadratureAction, complexDataContext);

   mpComplexDataMenu->addActions(pComplexDataGroup->actions());
   VERIFYNR(connect(pComplexDataGroup, SIGNAL(triggered(QAction*)), this, SLOT(setComplexComponent(QAction*))));

   // Colormap menu
   mpColorMapMenu = new QMenu("&Color Map", this);
   QAction* pColorMapLoadAction = mpColorMapMenu->addAction("&Load Color Map...", this, SLOT(setColorMapFromFile()));
   pColorMapLoadAction->setAutoRepeat(false);
   pColorMapLoadAction->setStatusTip("Sets the color map of a raster layer from a file");
   pDesktop->initializeAction(pColorMapLoadAction, shortcutContext);

   QAction* pColorMapSaveAction = mpColorMapMenu->addAction("&Save Color Map...", this,
      SLOT(saveColorMapToFile()));
   pColorMapSaveAction->setAutoRepeat(false);
   pColorMapSaveAction->setStatusTip("Saves the color map of a raster layer to a file");
   pDesktop->initializeAction(pColorMapSaveAction, shortcutContext);

   QAction* pColorMapCreateAction = mpColorMapMenu->addAction("&Create/Edit Color Map...", this,
      SLOT(createColorMap()));
   pColorMapCreateAction->setAutoRepeat(false);
   pColorMapCreateAction->setStatusTip("Creates a new color map or edits an existing one");
   pDesktop->initializeAction(pColorMapCreateAction, shortcutContext);

   mpColorMapMenu->addSeparator();

   mpColorMapList = new MenuListWidget(mpColorMapMenu);
   mpColorMapList->setSelectionMode(QAbstractItemView::SingleSelection);
   preloadColormaps();
   VERIFYNR(connect(mpColorMapList, SIGNAL(itemClicked(QListWidgetItem*)), this,
      SLOT(setPreloadedColorMap(QListWidgetItem*))));

   QWidgetAction* pColorMapListAction = new QWidgetAction(mpColorMapMenu);
   pColorMapListAction->setDefaultWidget(mpColorMapList);
   mpColorMapMenu->addAction(pColorMapListAction);

   // Displayed element menu
   mpElementMenu = new QMenu("Displayed &Element", this);
   mpElementList = new MenuListWidget(mpElementMenu);
   mpElementList->setSelectionMode(QAbstractItemView::SingleSelection);
   VERIFYNR(connect(mpElementList, SIGNAL(itemClicked(QListWidgetItem*)), this,
      SLOT(setDisplayedElement(QListWidgetItem*))));

   QWidgetAction* pElementListAction = new QWidgetAction(mpElementMenu);
   pElementListAction->setDefaultWidget(mpElementList);
   mpElementMenu->addAction(pElementListAction);

   VERIFYNR(connect(mpElementMenu, SIGNAL(aboutToShow()), this, SLOT(initializeElementList())));

   // Band menu
   mpBandMenu = new QMenu("Displayed &Band", this);
   mpBandList = new MenuListWidget(mpBandMenu);
   mpBandList->setSelectionMode(QAbstractItemView::SingleSelection);
   VERIFYNR(connect(mpBandList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(setBand(QListWidgetItem*))));

   QWidgetAction* pBandListAction = new QWidgetAction(mpBandMenu);
   pBandListAction->setDefaultWidget(mpBandList);
   mpBandMenu->addAction(pBandListAction);

   VERIFYNR(connect(mpBandMenu, SIGNAL(aboutToShow()), this, SLOT(initializeBandList())));

   // Save histogram action
   mpSaveAction = new QAction(QIcon(":/icons/SaveHistogram"), "&Save...", this);
   mpSaveAction->setAutoRepeat(false);
   mpSaveAction->setShortcut(QKeySequence("Ctrl+S"));
   VERIFYNR(connect(mpSaveAction, SIGNAL(triggered()), this, SLOT(saveHistogram())));
   pDesktop->initializeAction(mpSaveAction, shortcutContext);

   // Create the layer actions but do not yet add them to the menu
   mpBadValuesAction = new QAction("Bad Values...", this);
   mpBadValuesAction->setAutoRepeat(false);
   VERIFYNR(connect(mpBadValuesAction, SIGNAL(triggered()), this, SLOT(setBadValues())));
   pDesktop->initializeAction(mpBadValuesAction, shortcutContext);

   mpThresholdSeparatorAction = new QAction(this);
   mpThresholdSeparatorAction->setSeparator(true);

   mpRasterSeparatorAction = new QAction(this);
   mpRasterSeparatorAction->setSeparator(true);

   mpAutoZoomAction = new QAction("Auto Zoom", this);
   mpAutoZoomAction->setAutoRepeat(false);
   mpAutoZoomAction->setCheckable(true);
   mpAutoZoomAction->setChecked(true);
   VERIFYNR(connect(mpAutoZoomAction, SIGNAL(triggered(bool)), this, SLOT(enableAutoZoom(bool))));
   pDesktop->initializeAction(mpAutoZoomAction, shortcutContext);

   mpRasterMenusSeparatorAction = new QAction(this);
   mpRasterMenusSeparatorAction->setSeparator(true);

   mpSamplingAction = new QAction("Set S&patial Sampling...", this);
   mpSamplingAction->setAutoRepeat(false);
   VERIFYNR(connect(mpSamplingAction, SIGNAL(triggered()), this, SLOT(setResolution())));
   pDesktop->initializeAction(mpSamplingAction, shortcutContext);

   mpEndSeparatorAction = new QAction(this);
   mpEndSeparatorAction->setSeparator(true);

   // Plot objects
   mpRegion = static_cast<RegionObjectAdapter*>(addObject(REGION, false));
   mpRegion2 = static_cast<RegionObjectAdapter*>(addObject(REGION, false));
   mpHistogram = static_cast<HistogramAdapter*>(addObject(HISTOGRAM, true));

   VERIFYNR(connect(this, SIGNAL(extentsChanged(double, double, double, double)), this, SLOT(zoomExtents())));
   VERIFYNR(connect(this, SIGNAL(displayAreaChanged()), this, SLOT(updateHistogramRegionExtents())));

   Locator* pLocator = getMouseLocator();
   if (pLocator != NULL)
   {
      pLocator->setStyle(Locator::VERTICAL_LOCATOR);
   }

   // Initialization
   const MouseMode* pSelectMode = getMouseMode("SelectionMode");
   if (pSelectMode != NULL)
   {
      enableMouseMode(pSelectMode, false);
   }

   const MouseMode* pLocateMode = getMouseMode("LocatorMode");
   if (pLocateMode != NULL)
   {
      setMouseMode(pLocateMode);
   }

   list<ContextMenuAction> actionList = getContextMenuActions();
   for (list<ContextMenuAction>::iterator iter = actionList.begin(); iter != actionList.end(); ++iter)
   {
      ContextMenuAction action = *iter;
      if (action.mpAction != NULL)
      {
         QMenu* pMenu = action.mpAction->menu();
         if (pMenu != NULL)
         {
            QList<QAction*> menuActions = pMenu->actions();

            int i = 0;
            for (i = 0; i < menuActions.count(); ++i)
            {
               QAction* pMenuAction = menuActions[i];
               if (pMenuAction != NULL)
               {
                  QString strActionText = pMenuAction->text();
                  if (strActionText == "&Locate Point")
                  {
                     pMenuAction->setText("&Set Value");
                     pMenuAction->setStatusTip("Sets a threshold or contrast stretch value");
                     break;
                  }
               }
            }

            if (i < menuActions.count())
            {
               break;
            }
         }
      }
   }

   Service<UtilityServices> pUtilities;
   QString strClassification = QString::fromStdString(pUtilities->getDefaultClassification());
   setClassificationText(strClassification);

   addPropertiesPage(PropertiesHistogramPlot::getName());

   mpElement.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &HistogramPlotImp::elementDeleted));
   mpElement.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &HistogramPlotImp::elementModified));
}

HistogramPlotImp::~HistogramPlotImp()
{
   setHistogram(NULL);
}

list<ContextMenuAction> HistogramPlotImp::getContextMenuActions() const
{
   list<ContextMenuAction> menuActions = CartesianPlotImp::getContextMenuActions();

   ContextMenuAction saveAction(mpSaveAction, APP_HISTOGRAMPLOT_SAVE_ACTION);
   saveAction.mBuddyType = ContextMenuAction::BEFORE;
   saveAction.mBuddyId = APP_PLOTWIDGET_PRINT_ACTION;
   menuActions.push_back(saveAction);

   string beforeId = APP_PLOTVIEW_SECURITY_MARKINGS_ACTION;
   bool bNeedSeparator = false;
   if (mpElement.get() != NULL)
   {
      ContextMenuAction badValuesAction(mpBadValuesAction, APP_HISTOGRAMPLOT_BAD_VALUES_ACTION);
      badValuesAction.mBuddyType = ContextMenuAction::BEFORE;
      badValuesAction.mBuddyId = beforeId;
      menuActions.push_back(badValuesAction);

      ContextMenuAction samplingAction(mpSamplingAction, APP_HISTOGRAMPLOT_SAMPLING_ACTION);
      samplingAction.mBuddyType = ContextMenuAction::BEFORE;
      samplingAction.mBuddyId = beforeId;
      menuActions.push_back(samplingAction);

      bNeedSeparator = true;
   }

   if (dynamic_cast<const ThresholdLayer*>(mpLayer.get()) != NULL)
   {
      if (bNeedSeparator == true)
      {
         ContextMenuAction thresholdSeparatorAction(mpThresholdSeparatorAction,
            APP_HISTOGRAMPLOT_THRESHOLD_SEPARATOR_ACTION);
         thresholdSeparatorAction.mBuddyType = ContextMenuAction::BEFORE;
         thresholdSeparatorAction.mBuddyId = beforeId;
         menuActions.push_back(thresholdSeparatorAction);
      }

      ContextMenuAction passAreaAction(mpPassAreaMenu->menuAction(), APP_HISTOGRAMPLOT_PASS_AREA_MENU_ACTION);
      passAreaAction.mBuddyType = ContextMenuAction::BEFORE;
      passAreaAction.mBuddyId = beforeId;
      menuActions.push_back(passAreaAction);

      mpStretchUnitsMenu->setTitle("Threshold &Units");
      ContextMenuAction regionUnitsAction(mpStretchUnitsMenu->menuAction(), APP_HISTOGRAMPLOT_UNITS_MENU_ACTION);
      regionUnitsAction.mBuddyType = ContextMenuAction::BEFORE;
      regionUnitsAction.mBuddyId = beforeId;
      menuActions.push_back(regionUnitsAction);

      bNeedSeparator = true;
   }

   if (dynamic_cast<const RasterLayerImp*>(mpLayer.get()) != NULL)
   {
      if (bNeedSeparator == true)
      {
         ContextMenuAction rasterSeparatorAction(mpRasterSeparatorAction, APP_HISTOGRAMPLOT_RASTER_SEPARATOR_ACTION);
         rasterSeparatorAction.mBuddyType = ContextMenuAction::BEFORE;
         rasterSeparatorAction.mBuddyId = beforeId;
         menuActions.push_back(rasterSeparatorAction);
      }

      ContextMenuAction autoZoomAction(mpAutoZoomAction, APP_HISTOGRAMPLOT_AUTO_ZOOM_ACTION);
      autoZoomAction.mBuddyType = ContextMenuAction::BEFORE;
      autoZoomAction.mBuddyId = beforeId;
      menuActions.push_back(autoZoomAction);

      ContextMenuAction rasterMenusSeparatorAction(mpRasterMenusSeparatorAction,
         APP_HISTOGRAMPLOT_RASTER_MENUS_SEPARATOR_ACTION);
      rasterMenusSeparatorAction.mBuddyType = ContextMenuAction::BEFORE;
      rasterMenusSeparatorAction.mBuddyId = beforeId;
      menuActions.push_back(rasterMenusSeparatorAction);

      ContextMenuAction colorMapAction(mpColorMapMenu->menuAction(), APP_HISTOGRAMPLOT_COLOR_MAP_MENU_ACTION);
      colorMapAction.mBuddyType = ContextMenuAction::BEFORE;
      colorMapAction.mBuddyId = beforeId;
      menuActions.push_back(colorMapAction);

      ContextMenuAction stretchTypeAction(mpStretchTypeMenu->menuAction(), APP_HISTOGRAMPLOT_STRETCH_TYPE_MENU_ACTION);
      stretchTypeAction.mBuddyType = ContextMenuAction::BEFORE;
      stretchTypeAction.mBuddyId = beforeId;
      menuActions.push_back(stretchTypeAction);

      mpStretchUnitsMenu->setTitle("Stretch &Units");
      ContextMenuAction regionUnitsAction(mpStretchUnitsMenu->menuAction(), APP_HISTOGRAMPLOT_UNITS_MENU_ACTION);
      regionUnitsAction.mBuddyType = ContextMenuAction::BEFORE;
      regionUnitsAction.mBuddyId = beforeId;
      menuActions.push_back(regionUnitsAction);

      ContextMenuAction stretchResetAction(mpStretchResetMenu->menuAction(),
         APP_HISTOGRAMPLOT_STRETCH_RESET_MENU_ACTION);
      stretchResetAction.mBuddyType = ContextMenuAction::BEFORE;
      stretchResetAction.mBuddyId = beforeId;
      menuActions.push_back(stretchResetAction);

      ContextMenuAction elementMenuAction(mpElementMenu->menuAction(), APP_HISTOGRAMPLOT_ELEMENT_MENU_ACTION);
      elementMenuAction.mBuddyType = ContextMenuAction::BEFORE;
      elementMenuAction.mBuddyId = beforeId;
      menuActions.push_back(elementMenuAction);

      ContextMenuAction bandMenuAction(mpBandMenu->menuAction(), APP_HISTOGRAMPLOT_BAND_MENU_ACTION);
      bandMenuAction.mBuddyType = ContextMenuAction::BEFORE;
      bandMenuAction.mBuddyId = beforeId;
      menuActions.push_back(bandMenuAction);

      bNeedSeparator = true;
   }

   if (mpElement.get() != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(mpElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         EncodingType dataType = pDescriptor->getDataType();
         if ((dataType == INT4SCOMPLEX) || (dataType == FLT8COMPLEX))
         {
            ContextMenuAction complexDataAction(mpComplexDataMenu->menuAction(),
               APP_HISTOGRAMPLOT_COMPLEX_DATA_MENU_ACTION);
            complexDataAction.mBuddyType = ContextMenuAction::BEFORE;
            complexDataAction.mBuddyId = beforeId;
            menuActions.push_back(complexDataAction);

            bNeedSeparator = true;
         }
      }
   }

   if (bNeedSeparator == true)
   {
      ContextMenuAction endSeparatorAction(mpEndSeparatorAction, APP_HISTOGRAMPLOT_END_SEPARATOR_ACTION);
      endSeparatorAction.mBuddyType = ContextMenuAction::BEFORE;
      endSeparatorAction.mBuddyId = beforeId;
      menuActions.push_back(endSeparatorAction);
   }

   return menuActions;
}

bool HistogramPlotImp::canRename() const
{
   return false;
}

const string& HistogramPlotImp::getObjectType() const
{
   static string sType("HistogramPlotImp");
   return sType;
}

bool HistogramPlotImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "HistogramPlot"))
   {
      return true;
   }

   return CartesianPlotImp::isKindOf(className);
}

bool HistogramPlotImp::isKindOfView(const string& className)
{
   if ((className == "HistogramPlotImp") || (className == "HistogramPlot"))
   {
      return true;
   }

   return CartesianPlotImp::isKindOfView(className);
}

void HistogramPlotImp::getViewTypes(vector<string>& classList)
{
   classList.push_back("HistogramPlot");
   CartesianPlotImp::getViewTypes(classList);
}

HistogramPlotImp& HistogramPlotImp::operator=(const HistogramPlotImp& histogramPlot)
{
   if (this != &histogramPlot)
   {
      VERIFYNR(disconnect(this, SIGNAL(extentsChanged(double, double, double, double)), this, SLOT(zoomExtents())));
      CartesianPlotImp::operator= (histogramPlot);
      VERIFYNR(connect(this, SIGNAL(extentsChanged(double, double, double, double)), this, SLOT(zoomExtents())));

      mpHistogram = NULL;
      mpRegion = NULL;
      mpRegion2 = NULL;

      list<PlotObject*> objects = getObjects();
      for (list<PlotObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
      {
         PlotObject* pObject = *iter;
         if (pObject != NULL)
         {
            PlotObjectType objectType = pObject->getType();
            if ((objectType == HISTOGRAM) && (mpHistogram == NULL))
            {
               mpHistogram = dynamic_cast<HistogramImp*>(pObject);
            }
            else if (objectType == REGION)
            {
               // Set region one first since it was added first in the constructor
               if (mpRegion == NULL)
               {
                  mpRegion = dynamic_cast<RegionObjectAdapter*>(pObject);
               }
               else if (mpRegion2 == NULL)
               {
                  mpRegion2 = dynamic_cast<RegionObjectAdapter*>(pObject);
               }
            }
         }
      }

      meSelectedValue = histogramPlot.meSelectedValue;

      // Setting the histogram from the layer also updates the raster element
      setHistogram(const_cast<Layer*>(histogramPlot.mpLayer.get()), histogramPlot.mRasterChannelType);

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

View* HistogramPlotImp::copy(QGLContext* pDrawContext, QWidget* pParent) const
{
   string viewName = getName();

   HistogramPlotAdapter* pView = new HistogramPlotAdapter(SessionItemImp::generateUniqueId(), viewName,
      pDrawContext, pParent);
   if (pView != NULL)
   {
      UndoLock lock(pView);
      *(static_cast<HistogramPlotImp*>(pView)) = *this;
   }

   return pView;
}

bool HistogramPlotImp::copy(View *pView) const
{
   HistogramPlotImp* pViewImp = dynamic_cast<HistogramPlotImp*>(pView);
   if (pViewImp != NULL)
   {
      UndoLock lock(pView);
      *pViewImp = *this;
   }

   return pViewImp != NULL;
}

PlotType HistogramPlotImp::getPlotType() const
{
   return HISTOGRAM_PLOT;
}

void HistogramPlotImp::elementDeleted(Subject& subject, const string& signal, const boost::any& v)
{
   if (!NN(dynamic_cast<RasterElement*>(&subject)))
   {
      return;
   }

   // Update the histogram for a values change
   updateHistogramValues();
}

void HistogramPlotImp::elementModified(Subject& subject, const string& signal, const boost::any& v)
{
   RasterElement* pElement = dynamic_cast<RasterElement*>(&subject);
   if (!NN(pElement))
   {
      return;
   }

   if (pElement == mpElement.get())
   {
      // Update the security markings
      const Classification* pClass = mpElement->getClassification();
      if (pClass != NULL)
      {
         setClassification(pClass);
      }

      // Update the histogram for a values change
      updateHistogramValues();
   }
}

void HistogramPlotImp::setName(const string& name)
{
   if (name.empty() == true)
   {
      return;
   }

   string displayName;
   string displayText;
   string nameExtension;

   if (mpLayer.get() != NULL)
   {
      displayName = mpLayer->getDisplayName();
      displayText = mpLayer->getDisplayText();

      if (dynamic_cast<RasterLayer*>(mpLayer.get()) != NULL)
      {
         nameExtension = " - " + StringUtilities::toDisplayString(mRasterChannelType);
      }
   }

   // Get the plot name
   string plotName = name;
   if (nameExtension.empty() == false)
   {
      plotName += nameExtension;
   }

   if (plotName == getName())
   {
      return;
   }

   // If appending to the name, do not attempt to adjust the display name and text
   setFilenameDisplay(nameExtension.empty());

   // Set the display name
   if ((displayName.empty() == false) && (nameExtension.empty() == false))
   {
      displayName += nameExtension;
   }

   setDisplayName(displayName);

   // Set the display text
   if ((displayText.empty() == false) && (nameExtension.empty() == false))
   {
      displayText += nameExtension;
   }

   setDisplayText(displayText);

   // Set the histogram object name so that the name appears correctly in a plot legend
   if (mpHistogram != NULL)
   {
      string histogramName = displayName;
      if (histogramName.empty() == true)
      {
         histogramName = plotName;
      }

      mpHistogram->setObjectName(QString::fromStdString(histogramName));
   }

   // Set the plot name after the display name and text so they will be available for connected objects
   PlotViewImp::setName(plotName);
}

bool HistogramPlotImp::setHistogram(Layer* pLayer)
{
   return setHistogram(pLayer, GRAY);
}

bool HistogramPlotImp::setHistogram(Layer* pLayer, RasterChannelType color)
{
   if ((pLayer == mpLayer.get()) && (color == mRasterChannelType))
   {
      return true;
   }

   if (pLayer != NULL)
   {
      if ((pLayer->isKindOf("RasterLayer") == false) && (pLayer->isKindOf("ThresholdLayer") == false))
      {
         return false;
      }
   }

   // Disconnect the current layer and element
   LayerImp* pLayerImp = dynamic_cast<LayerImp*>(mpLayer.get());
   if (pLayerImp != NULL)
   {
      VERIFYNR(disconnect(pLayerImp, SIGNAL(nameChanged(const QString&)), this, SLOT(updateHistogramName())));
      VERIFYNR(disconnect(pLayerImp, SIGNAL(modified()), this, SLOT(updateHistogramRegions())));

      RasterLayerImp* pRasterLayer = dynamic_cast<RasterLayerImp*>(pLayerImp);
      if (pRasterLayer != NULL)
      {
         VERIFYNR(disconnect(pRasterLayer, SIGNAL(complexComponentChanged(const ComplexComponent&)), this,
            SLOT(setComplexComponent(const ComplexComponent&))));
         VERIFYNR(disconnect(pRasterLayer, SIGNAL(displayedBandChanged(RasterChannelType, DimensionDescriptor)), this,
            SLOT(updateElement())));
         VERIFYNR(disconnect(pRasterLayer, SIGNAL(colorMapChanged(const ColorMap&)), this,
            SLOT(updateSelectedColorMap())));
      }
   }

   // Set the new layer and channel
   mpLayer.reset(pLayer);
   mRasterChannelType = color;

   // Set the displayed element
   updateElement();

   if (mpLayer.get() != NULL)
   {
      // Update the name
      const string& name = mpLayer->getName();
      if (name.empty() == false)
      {
         setName(name);
      }

      // Set the current pass area menu action
      ThresholdLayer* pThresholdLayer = dynamic_cast<ThresholdLayer*>(mpLayer.get());
      if (pThresholdLayer != NULL)
      {
         PassArea ePassArea = pThresholdLayer->getPassArea();
         if (ePassArea == LOWER)
         {
            mpBelowAction->setChecked(true);
         }
         else if (ePassArea == UPPER)
         {
            mpAboveAction->setChecked(true);
         }
         else if (ePassArea == MIDDLE)
         {
            mpBetweenAction->setChecked(true);
         }
         else if (ePassArea == OUTSIDE)
         {
            mpOutsideAction->setChecked(true);
         }

         // Set the current stretch unit menu action
         updateStretchUnitAction();
      }

      RasterLayerImp* pRasterLayer = dynamic_cast<RasterLayerImp*>(mpLayer.get());
      if (pRasterLayer != NULL)
      {
         // Set the current stretch type menu action
         updateStretchTypeAction();

         // Set the current stretch unit menu action
         updateStretchUnitAction();

         // Update the color map menu
         updateSelectedColorMap();

         // Set the current complex component menu action
         if (mpElement.get() != NULL)
         {
            const RasterDataDescriptor* pDescriptor =
               dynamic_cast<const RasterDataDescriptor*>(mpElement->getDataDescriptor());
            if (pDescriptor != NULL)
            {
               EncodingType dataType = pDescriptor->getDataType();
               if ((dataType == INT4SCOMPLEX) || (dataType == FLT8COMPLEX))
               {
                  ComplexComponent eComponent = pRasterLayer->getComplexComponent();
                  setComplexComponent(eComponent);
               }
            }
         }
      }

      // Initialization
      updateHistogramRegions();

      // Connections
      LayerImp* pNewLayerImp = dynamic_cast<LayerImp*>(mpLayer.get());
      if (pNewLayerImp != NULL)
      {
         VERIFYNR(connect(pNewLayerImp, SIGNAL(nameChanged(const QString&)), this, SLOT(updateHistogramName())));
         VERIFYNR(connect(pNewLayerImp, SIGNAL(modified()), this, SLOT(updateHistogramRegions())));
      }

      if (pRasterLayer != NULL)
      {
         VERIFYNR(connect(pRasterLayer, SIGNAL(complexComponentChanged(const ComplexComponent&)), this,
            SLOT(setComplexComponent(const ComplexComponent&))));
         VERIFYNR(connect(pRasterLayer, SIGNAL(displayedBandChanged(RasterChannelType, DimensionDescriptor)), this,
            SLOT(updateElement())));
         VERIFYNR(connect(pRasterLayer, SIGNAL(colorMapChanged(const ColorMap&)), this,
            SLOT(updateSelectedColorMap())));
      }
   }

   notify(SIGNAL_NAME(Subject, Modified));
   return true;
}

void HistogramPlotImp::showEvent(QShowEvent* pEvent)
{
   QWidget::showEvent(pEvent);
   mUpdater.update();
}

bool HistogramPlotImp::setHistogram(unsigned int binCount, const double* pBinCenters, const double* pValues,
                                    const double* pBinWidths, bool bAbove)
{
   bool bSuccess = false;
   if (mpHistogram != NULL)
   {
      bSuccess = mpHistogram->setHistogramData(binCount, pBinCenters, pValues, pBinWidths, bAbove);
      if (bSuccess == true)
      {
         // Update the extents if only one bin
         unsigned int numBins = mpHistogram->getNumBins();
         if (numBins == 1)
         {
            double dMinX = 0.0;
            double dMaxX = 0.0;
            double dMinY = 0.0;
            double dMaxY = 0.0;

            getExtents(dMinX, dMinY, dMaxX, dMaxY);
            setExtents(dMinX - ((dMaxX - dMinX) * 100), dMinY, dMaxX + ((dMaxX - dMinX) * 100), dMaxY);
         }

         // Notify attached objects of the change
         notify(SIGNAL_NAME(Subject, Modified));
      }
   }

   return bSuccess;
}

QColor HistogramPlotImp::getHistogramColor() const
{
   QColor clrHistogram;
   if (mpHistogram != NULL)
   {
      clrHistogram = mpHistogram->HistogramImp::getColor();
   }

   return clrHistogram;
}

void HistogramPlotImp::enableAutoZoom(bool enable)
{
   mAutoZoom = enable;
   mpAutoZoomAction->setChecked(mAutoZoom);

   if (mAutoZoom == true)
   {
      connect(this, SIGNAL(extentsChanged(double, double, double, double)), this, SLOT(zoomExtents()));
   }
   else
   {
      disconnect(this, SIGNAL(extentsChanged(double, double, double, double)), this, SLOT(zoomExtents()));
   }
}

void HistogramPlotImp::setHistogramColor(const QColor& clrHistogram)
{
   if ((clrHistogram.isValid() == false) || (mpHistogram == NULL))
   {
      return;
   }

   QColor clrCurrent = mpHistogram->getColor();
   if (clrHistogram != clrCurrent)
   {
      mpHistogram->setColor(clrHistogram);
      refresh();
   }
}

void HistogramPlotImp::setComplexComponent(const ComplexComponent& eComponent)
{
   if (eComponent == COMPLEX_MAGNITUDE)
   {
      mpMagnitudeAction->activate(QAction::Trigger);
   }
   else if (eComponent == COMPLEX_PHASE)
   {
      mpPhaseAction->activate(QAction::Trigger);
   }
   else if (eComponent == COMPLEX_INPHASE)
   {
      mpInPhaseAction->activate(QAction::Trigger);
   }
   else if (eComponent == COMPLEX_QUADRATURE)
   {
      mpQuadratureAction->activate(QAction::Trigger);
   }

   updateHistogramValues();
}

static void ShowRangeValues(char* pFormat, double lower, double upper)
{
   char message[1024];
   sprintf(message, pFormat, lower, upper);

   Service<DesktopServices> pDesktop;
   pDesktop->setStatusBarMessage(message);
}

void HistogramPlotImp::mousePressEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if ((pEvent->button() == Qt::LeftButton) && (mpLayer.get() != NULL))
      {
         string mouseMode;

         const MouseMode* pCurrentMouseMode = getCurrentMouseMode();
         if (pCurrentMouseMode != NULL)
         {
            pCurrentMouseMode->getName(mouseMode);
         }

         if (mouseMode == "LocatorMode")
         {
            QPoint ptMouse = pEvent->pos();
            ptMouse.setY(height() - pEvent->pos().y());

            double dXValue = 0.0;
            double dYValue = 0.0;
            translateScreenToData(ptMouse.x(), ptMouse.y(), dXValue, dYValue);
            float locatorValue = dXValue;
            QString locatorValueUnit;

            double lowerLimit = 0.0;
            double upperLimit = 0.0;
            getDataLowerUpper(lowerLimit, upperLimit);

            double tmpLowerLimit = lowerLimit;
            double tmpUpperLimit = upperLimit;

            double minValue = 0.0;
            double maxValue = 0.0;
            getDataMinMax(minValue, maxValue);
            LocationType llCorner;
            LocationType ulCorner;
            LocationType urCorner;
            LocationType lrCorner;
            getVisibleCorners(llCorner, ulCorner, urCorner, lrCorner);

            PassArea ePassArea = getLayerPassArea();
            QColor regionColor = getLayerColor();

            bool showRegions = true;

            RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
            if (pRasterLayer != NULL)
            {
               RegionUnits eUnits = pRasterLayer->getStretchUnits(mRasterChannelType);
               if (pRasterLayer->getStretchType(pRasterLayer->getDisplayMode()) == EQUALIZATION)
               {
                  showRegions = false;
               }
               else
               {
                  tmpLowerLimit = max(llCorner.mX, lowerLimit);
                  tmpUpperLimit = min(lrCorner.mX, upperLimit);
                  if (fabs(tmpUpperLimit - dXValue) < fabs(tmpLowerLimit - dXValue))
                  {
                     meSelectedValue = UPPER_VALUE;
                     upperLimit = dXValue;
                  }
                  else
                  {
                     meSelectedValue = LOWER_VALUE;
                     lowerLimit = dXValue;
                  }

                  ShowRangeValues("Contrast stretch: %g - %g", lowerLimit, upperLimit);

                  locatorValue = pRasterLayer->convertStretchValue(mRasterChannelType, RAW_VALUE, dXValue, eUnits);
                  locatorValueUnit =
                     dynamic_cast<RasterLayerImp*>(pRasterLayer)->getStretchUnitsAsString(mRasterChannelType);
               }
            }

            ThresholdLayer* pThresholdLayer = dynamic_cast<ThresholdLayer*>(mpLayer.get());
            if (pThresholdLayer != NULL)
            {
               if ((ePassArea == LOWER) || (ePassArea == UPPER))
               {
                  meSelectedValue = LOWER_VALUE;
                  lowerLimit = dXValue;
               }
               else if (ePassArea == MIDDLE)
               {
                  tmpLowerLimit = max(llCorner.mX, lowerLimit);
                  tmpUpperLimit = min(lrCorner.mX, upperLimit);
                  if (fabs(tmpUpperLimit - dXValue) < fabs(tmpLowerLimit - dXValue))
                  {
                     meSelectedValue = UPPER_VALUE;
                     upperLimit = dXValue;
                  }
                  else
                  {
                     meSelectedValue = LOWER_VALUE;
                     lowerLimit = dXValue;
                  }
               }
               else // OUTSIDE
               {
                  if (lowerLimit < minValue)
                  {
                     tmpLowerLimit = minValue;
                  }
                  if (upperLimit > maxValue)
                  {
                     tmpUpperLimit = maxValue;
                  }
                  if (fabs(dXValue - tmpLowerLimit) < fabs(dXValue - tmpUpperLimit))
                  {
                     meSelectedValue = LOWER_VALUE;
                     lowerLimit = dXValue;
                  }
                  else
                  {
                     meSelectedValue = UPPER_VALUE;
                     upperLimit = dXValue;
                  }
               }

               // Correct swapping of lower and upper limit
               if (lowerLimit > upperLimit)
               {
                  double tmp = lowerLimit;
                  upperLimit = lowerLimit;
                  lowerLimit = tmp;
               }

               if ((ePassArea == LOWER) || (ePassArea == UPPER))
               {
                  ShowRangeValues("Threshold: %g", dXValue);
               }
               else
               {
                  ShowRangeValues("Threshold: %g - %g", lowerLimit, upperLimit);
               }

               RegionUnits eUnits = pThresholdLayer->getRegionUnits();
               locatorValue = pThresholdLayer->convertThreshold(RAW_VALUE, dXValue, eUnits);
               locatorValueUnit = dynamic_cast<ThresholdLayerImp*>(pThresholdLayer)->getRegionUnitsAsString();

               double lowerLimitStretched = pThresholdLayer->convertThreshold(RAW_VALUE, lowerLimit, eUnits);
               double upperLimitStretched = pThresholdLayer->convertThreshold(RAW_VALUE, upperLimit, eUnits);

               pThresholdLayer->setFirstThreshold(lowerLimit);
               pThresholdLayer->setSecondThreshold(upperLimit);
            }

            updateHistogramRegions(lowerLimit, upperLimit, minValue, maxValue, ePassArea, regionColor, showRegions);

            LocatorImp* pLocator = dynamic_cast<LocatorImp*>(getMouseLocator());
            if (pLocator != NULL && showRegions)
            {
               pLocator->setVisible(true);
               pLocator->setLocation(LocationType(dXValue, dYValue), false);
               pLocator->setText(QString::number(locatorValue) + " " + locatorValueUnit, QString());
               pLocator->setColor(Qt::red);
               pLocator->setLineStyle(SOLID_LINE);
            }

            if (showRegions)
            {
               refresh();
            }

            return;
         }
      }
   }

   CartesianPlotImp::mousePressEvent(pEvent);
}

void HistogramPlotImp::mouseMoveEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if ((pEvent->buttons() == Qt::LeftButton) && (mpLayer.get() != NULL))
      {
         string mouseMode;

         const MouseMode* pCurrentMouseMode = getCurrentMouseMode();
         if (pCurrentMouseMode != NULL)
         {
            pCurrentMouseMode->getName(mouseMode);
         }

         if (mouseMode == "LocatorMode")
         {
            QPoint ptMouse = pEvent->pos();
            ptMouse.setY(height() - pEvent->pos().y());

            double dXValue = 0.0;
            double dYValue = 0.0;

            translateScreenToData(ptMouse.x(), ptMouse.y(), dXValue, dYValue);
            float locatorValue = dXValue;
            QString locatorValueUnit;

            double lowerLimit = 0.0;
            double upperLimit = 0.0;
            getDataLowerUpper(lowerLimit, upperLimit);

            double minValue = 0.0;
            double maxValue = 0.0;
            getDataMinMax(minValue, maxValue);

            PassArea ePassArea = getLayerPassArea();
            QColor regionColor = getLayerColor();

            bool showRegions = true;

            if (meSelectedValue == LOWER_VALUE)
            {
               lowerLimit = dXValue;
            }
            else
            {
               upperLimit = dXValue;
            }

            RasterLayerImp* pRasterLayer = dynamic_cast<RasterLayerImp*>(mpLayer.get());
            if (pRasterLayer != NULL)
            {
               RegionUnits eUnits = pRasterLayer->getStretchUnits(mRasterChannelType);
               locatorValue = pRasterLayer->convertStretchValue(mRasterChannelType, RAW_VALUE, dXValue, eUnits);
               locatorValueUnit = pRasterLayer->getStretchUnitsAsString(mRasterChannelType);
               if (pRasterLayer->getStretchType(pRasterLayer->getDisplayMode()) == EQUALIZATION)
               {
                  showRegions = false;
               }
               else
               {
                  ShowRangeValues("Contrast stretch: %g - %g", lowerLimit, upperLimit);
               }
            }

            if (mpLayer->isKindOf("ThresholdLayer") == true)
            {
               if ((ePassArea == LOWER) || (ePassArea == UPPER))
               {
                  ShowRangeValues("Threshold: %g", meSelectedValue == LOWER_VALUE ? lowerLimit : upperLimit);
               }
               else
               {
                  ShowRangeValues("Threshold: %g - %g", lowerLimit, upperLimit);
               }

               ThresholdLayer* pLayer = static_cast<ThresholdLayer*>(mpLayer.get());

               RegionUnits eUnits = pLayer->getRegionUnits();
               locatorValue = pLayer->convertThreshold(RAW_VALUE, dXValue, eUnits);
               locatorValueUnit = dynamic_cast<ThresholdLayerImp*>(pLayer)->getRegionUnitsAsString();

               double lowerLimitStretched = pLayer->convertThreshold(RAW_VALUE, lowerLimit, eUnits);
               double upperLimitStretched = pLayer->convertThreshold(RAW_VALUE, upperLimit, eUnits);

               pLayer->setFirstThreshold(lowerLimit);
               pLayer->setSecondThreshold(upperLimit);
            }

            updateHistogramRegions(lowerLimit, upperLimit, minValue, maxValue, ePassArea, regionColor, showRegions);

            LocatorImp* pLocator = dynamic_cast<LocatorImp*>(getMouseLocator());
            if (pLocator != NULL && showRegions)
            {
               pLocator->setLocation(LocationType(dXValue, dYValue), false);
               pLocator->setText(QString::number(locatorValue) + " " + locatorValueUnit, QString());
            }

            if (showRegions)
            {
               refresh();
            }

            return;
         }
      }
   }

   CartesianPlotImp::mouseMoveEvent(pEvent);
}

void HistogramPlotImp::mouseReleaseEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if ((pEvent->button() == Qt::LeftButton) && (mpLayer.get() != NULL))
      {
         string mouseMode;

         const MouseMode* pCurrentMouseMode = getCurrentMouseMode();
         if (pCurrentMouseMode != NULL)
         {
            pCurrentMouseMode->getName(mouseMode);
         }

         if (mouseMode == "LocatorMode")
         {
            QPoint ptMouse = pEvent->pos();
            ptMouse.setY(height() - pEvent->pos().y());

            double dXValue = 0.0;
            double dYValue = 0.0;
            translateScreenToData(ptMouse.x(), ptMouse.y(), dXValue, dYValue);

            double lowerLimit = 0.0;
            double upperLimit = 0.0;
            getDataLowerUpper(lowerLimit, upperLimit);

            double minValue = 0.0;
            double maxValue = 0.0;
            getDataMinMax(minValue, maxValue);

            PassArea ePassArea = getLayerPassArea();
            QColor regionColor = getLayerColor();
            bool showRegions = true;

            if (meSelectedValue == LOWER_VALUE)
            {
               lowerLimit = dXValue;
            }
            else
            {
               upperLimit = dXValue;
            }

            RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
            if (pRasterLayer != NULL)
            {
               RegionUnits eUnits = pRasterLayer->getStretchUnits(mRasterChannelType);

               double lowerLimitStretched = pRasterLayer->convertStretchValue(mRasterChannelType, RAW_VALUE,
                  lowerLimit, eUnits);
               double upperLimitStretched = pRasterLayer->convertStretchValue(mRasterChannelType, RAW_VALUE,
                  upperLimit, eUnits);

               if (pRasterLayer->getStretchType(pRasterLayer->getDisplayMode()) == EQUALIZATION)
               {
                  showRegions = false;
               }
               else
               {
                  pRasterLayer->setStretchValues(mRasterChannelType, lowerLimitStretched, upperLimitStretched);
               }
            }

            ThresholdLayer* pThresholdLayer = dynamic_cast<ThresholdLayer*>(mpLayer.get());
            if (pThresholdLayer != NULL)
            {
               RegionUnits eUnits = pThresholdLayer->getRegionUnits();

               double lowerLimitStretched = pThresholdLayer->convertThreshold(RAW_VALUE, lowerLimit, eUnits);
               double upperLimitStretched = pThresholdLayer->convertThreshold(RAW_VALUE, upperLimit, eUnits);

               pThresholdLayer->setFirstThreshold(lowerLimit);
               pThresholdLayer->setSecondThreshold(upperLimit);
            }

            updateHistogramRegions(lowerLimit, upperLimit, minValue, maxValue, ePassArea, regionColor, showRegions);

            Locator* pLocator = getMouseLocator();
            if (pLocator != NULL)
            {
               pLocator->setVisible(false);
            }

            meSelectedValue = NO_VALUE;

            if (showRegions)
            {
               refresh();
            }
         }
      }
   }

   CartesianPlotImp::mouseReleaseEvent(pEvent);
}

bool HistogramPlotImp::getDataMinMax(double& minValue, double& maxValue) const
{
   Statistics* pStatistics = getStatistics();
   if (pStatistics != NULL)
   {
      ComplexComponent eComponent = RasterLayer::getSettingComplexComponent();

      const RasterLayer* pRasterLayer = dynamic_cast<const RasterLayer*>(mpLayer.get());
      if (pRasterLayer != NULL)
      {
         eComponent = pRasterLayer->getComplexComponent();
      }

      minValue = pStatistics->getMin(eComponent);
      maxValue = pStatistics->getMax(eComponent);
      return true;
   }

   return false;
}

bool HistogramPlotImp::getDataLowerUpper(double& lowerLimit, double& upperLimit) const
{
   if (mpLayer.get() != NULL)
   {
      const RasterLayer* pRasterLayer = dynamic_cast<const RasterLayer*>(mpLayer.get());
      if (pRasterLayer != NULL)
      {
         // NOTE: getStretchValues returns stretched (Not Raw) values
         pRasterLayer->getStretchValues(mRasterChannelType, lowerLimit, upperLimit);

         RegionUnits eUnits = pRasterLayer->getStretchUnits(mRasterChannelType);
         if (eUnits != RAW_VALUE)
         {
            lowerLimit = pRasterLayer->convertStretchValue(mRasterChannelType, lowerLimit, RAW_VALUE);
            upperLimit = pRasterLayer->convertStretchValue(mRasterChannelType, upperLimit, RAW_VALUE);
         }

         return true;
      }

      const ThresholdLayer* pThresholdLayer = dynamic_cast<const ThresholdLayer*>(mpLayer.get());
      if (pThresholdLayer != NULL)
      {
         lowerLimit = pThresholdLayer->getFirstThreshold();
         upperLimit = pThresholdLayer->getSecondThreshold();
         return true;
      }
   }

   return false;
}

Statistics* HistogramPlotImp::getStatistics() const
{
   if (mpElement.get() != NULL)
   {
      const RasterLayer* pRasterLayer = dynamic_cast<const RasterLayer*>(mpLayer.get());
      if (pRasterLayer != NULL)
      {
         // Only get the statistics if the displayed band is valid
         DimensionDescriptor bandDim = pRasterLayer->getDisplayedBand(mRasterChannelType);
         if (bandDim.isValid())
         {
            return mpElement->getStatistics(bandDim);
         }
      }
      else
      {
         return mpElement->getStatistics();
      }
   }

   return NULL;
}

PassArea HistogramPlotImp::getLayerPassArea() const
{
   PassArea ePassArea = MIDDLE;

   const ThresholdLayer* pThresholdLayer = dynamic_cast<const ThresholdLayer*>(mpLayer.get());
   if (pThresholdLayer != NULL)
   {
      ePassArea = pThresholdLayer->getPassArea();
   }

   return ePassArea;
}

QColor HistogramPlotImp::getLayerColor() const
{
   QColor layerColor = Qt::white;
   if (mpLayer.get() != NULL)
   {
      LayerType eLayerType = mpLayer->getLayerType();
      switch (eLayerType)
      {
         case RASTER:
         {
            switch (mRasterChannelType)
            {
               case GRAY:
                  layerColor = Qt::white;
                  break;

               case RED:
                  layerColor = Qt::red;
                  break;

               case GREEN:
                  layerColor = Qt::green;
                  break;

               case BLUE:
                  layerColor = Qt::blue;
                  break;

               default:
                  break;
            }

            break;
         }

         case THRESHOLD:
         {
            const ThresholdLayerImp* pThresholdLayer = dynamic_cast<const ThresholdLayerImp*>(mpLayer.get());
            if (pThresholdLayer != NULL)
            {
               layerColor = pThresholdLayer->getColor();
            }
            break;
         }

         default:
            break;
      }
   }

   return layerColor;
}

void HistogramPlotImp::updateStretchTypeAction()
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if (pRasterLayer != NULL)
   {
      DisplayMode eMode = mRasterChannelType == GRAY ? GRAYSCALE_MODE : RGB_MODE;
      StretchType eType = pRasterLayer->getStretchType(eMode);
      if (eType == LINEAR)
      {
         if (mpLinearAction != NULL)
         {
            mpLinearAction->setChecked(true);
         }
      }
      else if (eType == EXPONENTIAL)
      {
         if (mpExponentialAction != NULL)
         {
            mpExponentialAction->setChecked(true);
         }
      }
      else if (eType == LOGARITHMIC)
      {
         if (mpLogarithmicAction != NULL)
         {
            mpLogarithmicAction->setChecked(true);
         }
      }
      else if (eType == EQUALIZATION)
      {
         if (mpEqualizationAction != NULL)
         {
            mpEqualizationAction->setChecked(true);
         }
      }
   }
}

void HistogramPlotImp::updateStretchUnitAction()
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   bool updateAction = false;
   RegionUnits eUnits;
   if (pRasterLayer != NULL)
   {
      eUnits = pRasterLayer->getStretchUnits(mRasterChannelType);
      updateAction = true;
   }
   ThresholdLayer* pThresholdLayer = dynamic_cast<ThresholdLayer*>(mpLayer.get());
   if (pThresholdLayer != NULL)
   {
      eUnits = pThresholdLayer->getRegionUnits();
      updateAction = true;
   }
   if (!updateAction)
   {
      return;
   }

   if (eUnits == RAW_VALUE)
   {
      if (mpRawAction != NULL)
      {
         mpRawAction->setChecked(true);
      }
   }
   else if (eUnits == PERCENTAGE)
   {
      if (mpPercentageAction != NULL)
      {
         mpPercentageAction->setChecked(true);
      }
   }
   else if (eUnits == PERCENTILE)
   {
      if (mpPercentileAction != NULL)
      {
         mpPercentileAction->setChecked(true);
      }
   }
   else if (eUnits == STD_DEV)
   {
      if (mpStdDevAction != NULL)
      {
         mpStdDevAction->setChecked(true);
      }
   }
}

void HistogramPlotImp::setAlternateColormap(const std::vector<ColorType>* pColormap)
{
   mpAlternateColormap = pColormap;
   updateHistogramRegions();
}

void HistogramPlotImp::updateHistogramRegions(double lowerLimit, double upperLimit, double minValue, double maxValue,
                                              const PassArea& eArea, const QColor& layerColor, bool showRegions)
{
   if (showRegions == false)
   {
      mpRegion->setVisible(false);
      mpRegion2->setVisible(false);
      refresh();
      return;
   }
   else
   {
      mpRegion->setVisible(true);
      mpRegion2->setVisible(true);
   }

   if (mpRegion2 != NULL)
   {
      mpRegion2->setVisible(false);
   }

   if (mpRegion != NULL)
   {
      bool bUseColorMap = false;
      bool bUseColorGradient = false;
      RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());

      // find whether it is a colormap, gradient, or other
      if (pRasterLayer != NULL)
      {
         bUseColorGradient = true;

         RasterLayerImp* pRasterImp =
            dynamic_cast<RasterLayerImp*> (pRasterLayer);

         if ((mpAlternateColormap ? ColorMap("Custom", *mpAlternateColormap) : pRasterImp->getColorMap()).isDefault() 
            == false)
         {
            bUseColorMap = true;
         }

         if (pRasterLayer->getDisplayMode() == RGB_MODE)
         {
            bUseColorMap = false;
         }
      }

      // colormap
      if (bUseColorMap && pRasterLayer != NULL)
      {
         RasterLayerImp* pRasterLayerImp = dynamic_cast<RasterLayerImp*>(pRasterLayer);
         if (pRasterLayerImp != NULL)
         {
            const ColorMap& colormap = pRasterLayerImp->getColorMap();
            const vector<ColorType> colors = mpAlternateColormap ? *mpAlternateColormap : colormap.getTable();

            int opacity = 0;
            for (unsigned int i = 0; i < colors.size(); ++i)
            {
               opacity = max(opacity, colors[i].mAlpha);
            }

            if (opacity == 0)
            {
               opacity = 255 * 0.75;
            }
            else
            {
               opacity = min(255, static_cast<int>(255 * (0.75*255 / opacity)));
            }

            mpRegion->setColors(colors);
            mpRegion->setTransparency(opacity);
            mpRegion->setDrawBorder(true);

            if (mpRegion2 != NULL)
            {
               mpRegion2->setColors(colors);
               mpRegion2->setTransparency(opacity);
               mpRegion2->setDrawBorder(true);
            }
         }
      }
      // gradient
      else if (bUseColorGradient)
      {
         vector<QColor> colors;
         colors.push_back(Qt::black);
         colors.push_back(layerColor);

         (static_cast<RegionObjectImp*> (mpRegion))->setColors(colors);
         mpRegion->setTransparency(0.75*255);
         mpRegion->setDrawBorder(true);

         if (mpRegion2 != NULL)
         {
            (static_cast<RegionObjectImp*> (mpRegion2))->setColors(colors);
            mpRegion2->setTransparency(0.75 * 255);
            mpRegion2->setDrawBorder(true);
         }
      }
      // other
      else
      {
         (static_cast<RegionObjectImp*> (mpRegion))->setColor(layerColor);
         mpRegion->setTransparency(255);
         mpRegion->setDrawBorder(false);

         if (mpRegion2 != NULL)
         {
            (static_cast<RegionObjectImp*> (mpRegion2))->setColor(layerColor);
            mpRegion2->setTransparency(255);
            mpRegion2->setDrawBorder(false);
         }
      }

      double dMinX = 0.0;
      double dMinY = 0.0;
      double dMaxX = 0.0;
      double dMaxY = 0.0;

      translateScreenToData(0, 0, dMinX, dMinY);
      translateScreenToData(width(), height(), dMaxX, dMaxY);

      switch (eArea)
      {
         case LOWER:
            mpRegion->setRegion(minValue, dMinY, lowerLimit, dMaxY);
            break;

         case UPPER:
            mpRegion->setRegion(lowerLimit, dMinY, maxValue, dMaxY);
            break;

         case MIDDLE:
            mpRegion->setRegion(lowerLimit, dMinY, upperLimit, dMaxY);
            break;

         case OUTSIDE:
            mpRegion->setRegion(minValue, dMinY, lowerLimit, dMaxY);
            if (mpRegion2 != NULL)
            {
               mpRegion2->setVisible(true);
               mpRegion2->setRegion(upperLimit, dMinY, maxValue, dMaxY);
            }

            break;

         default:
            break;
      }
   }

   refresh();
}

void HistogramPlotImp::saveHistogram()
{
   vector<double> centers;
   vector<double> counts;
   vector<double> widths;

   if (mpHistogram != NULL)
   {
      mpHistogram->getHistogramData(centers, counts, widths);
   }

   unsigned int uiBins = centers.size();
   if (uiBins == 0)
   {
      QMessageBox::critical(this, QString::fromStdString(getName()),
         "There is no data in the histogram!  It will not be saved.");
      return;
   }

   QString strFilename = QFileDialog::getSaveFileName(this, QString(), QString(),
      "Histogram Files (*.hst);;All Files (*)", NULL, QFileDialog::DontConfirmOverwrite);
   if (strFilename.isEmpty() == true)
   {
      return;
   }

   QFileInfo saveFileInfo = QFileInfo(strFilename);
   if (saveFileInfo.suffix() != "hst")
   {
      strFilename += ".hst";
      saveFileInfo.setFile(strFilename);
   }

   // Prompt for overwrite
   if (saveFileInfo.exists() == true)
   {
      int iReplace = QMessageBox::warning(this, APP_NAME, strFilename + " already exists.\n"
         "Do you want to replace it?", QMessageBox::Yes, QMessageBox::No);
      if (iReplace == QMessageBox::No)
      {
         return;
      }
   }

   string filename = strFilename.toStdString();

   FILE* stream = fopen(filename.c_str(), "wt");
   if (stream == NULL)
   {
      QMessageBox::critical(this, APP_NAME, "Unable to write to file " + strFilename);
      return;
   }

   for (unsigned int i = 0; i < uiBins; i++)
   {
      fprintf(stream, "%g %g\n", centers.at(i), counts.at(i));
   }

   fclose(stream);
}

void HistogramPlotImp::setSecurityMarkings()
{
   QString strMarkings;
   ClassificationAdapter newClassification;

   if (mpElement.get() == NULL)
   {
      strMarkings = getClassificationText();
   }
   else
   {
      const Classification* pClassification = mpElement->getClassification();
      if (pClassification != NULL)
      {
         newClassification.setClassification(pClassification);
      }
   }

   SecurityMarkingsDlg dlg(this, strMarkings, &newClassification);

   int iReturn = -1;
   iReturn = dlg.exec();
   if (iReturn == QDialog::Accepted)
   {
      QString strNewMarkings = dlg.getSecurityMarkings();
      if (strNewMarkings.isEmpty() == false)
      {
         setClassificationText(strNewMarkings);
      }

      if (mpElement.get() != NULL)
      {
         DataDescriptor* pDescriptor = mpElement->getDataDescriptor();
         if (pDescriptor != NULL)
         {
            pDescriptor->setClassification(&newClassification);
         }
      }
   }
}

void HistogramPlotImp::preloadColormaps()
{
   if (mpColorMapList == NULL)
   {
      return;
   }

   ColorMap defaultMap;

   const string& defaultName = defaultMap.getName();
   if (defaultName.empty() == false)
   {
      mpColorMapList->addItem(QString::fromStdString(defaultName));
   }

   QString colorMapDir;
   const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
   if (pSupportFilesPath != NULL)
   {
      colorMapDir = QString::fromStdString(pSupportFilesPath->getFullPathAndName() + SLASH + "ColorTables");
   }

   QDir dir(colorMapDir, "*.clu *.cgr");
   QString strDirectory = dir.absolutePath() + "/";

   for (unsigned int i = 0; i < dir.count(); ++i)
   {
      QString strFilename = strDirectory + dir[i];
      if (strFilename.isEmpty() == false)
      {
         string filename = strFilename.toStdString();
         try
         {
            ColorMap cmap(filename);
            mPreloadedColorMaps.insert(pair<string, string>(cmap.getName(), filename));
            mpColorMapList->addItem(QString::fromStdString(cmap.getName()));
         }
         catch (...) // bad color map file
         {
         }
      }
   }
}

void HistogramPlotImp::setPreloadedColorMap(QListWidgetItem* pItem)
{
   if ((pItem == NULL) || (mpLayer.get() == NULL))
   {
      return;
   }

   QString strColorMap = pItem->text();
   if (strColorMap.isEmpty() == true)
   {
      return;
   }

   RasterLayerImp* pRasterLayer = dynamic_cast<RasterLayerImp*>(mpLayer.get());
   if (pRasterLayer == NULL)
   {
      return;
   }

   // Close the context menu by simulating a mouse click on the menu
   if (mpColorMapMenu != NULL)
   {
      QMouseEvent mousePress(QEvent::MouseButtonPress, mpColorMapMenu->mapFromGlobal(QCursor::pos()),
         Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
      QApplication::sendEvent(mpColorMapMenu, &mousePress);

      QMouseEvent mouseRelease(QEvent::MouseButtonRelease, mpColorMapMenu->mapFromGlobal(QCursor::pos()),
         Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
      QApplication::sendEvent(mpColorMapMenu, &mouseRelease);
   }

   ColorMap defaultMap;

   QString strDefaultName = QString::fromStdString(defaultMap.getName());
   if (strColorMap == strDefaultName)
   {
      pRasterLayer->setColorMap(defaultMap);
      return;
   }

   map<string, string>::iterator iter = mPreloadedColorMaps.find(strColorMap.toStdString());
   if (iter != mPreloadedColorMaps.end())
   {
      setColorMapFromFile(QString::fromStdString((*iter).second));
   }
   else
   {
      QMessageBox::critical(this, "Bad Color Map", "Error HistogramPlot001: Missing ColorMap in list");
   }
}

void HistogramPlotImp::updateSelectedColorMap()
{
   if (mpColorMapList == NULL)
   {
      return;
   }

   mpColorMapList->clearSelection();
   mpColorMapList->setCurrentItem(NULL);

   RasterLayerImp* pRasterLayer = dynamic_cast<RasterLayerImp*>(mpLayer.get());
   if (pRasterLayer == NULL)
   {
      return;
   }

   const ColorMap& colorMap = pRasterLayer->getColorMap();

   const string& colorMapName = colorMap.getName();
   if (colorMapName.empty() == false)
   {
      QList<QListWidgetItem*> items = mpColorMapList->findItems(QString::fromStdString(colorMapName), Qt::MatchExactly);
      if (items.count() == 1)
      {
         QListWidgetItem* pItem = items.front();
         if (pItem != NULL)
         {
            mpColorMapList->setItemSelected(pItem, true);
            mpColorMapList->setCurrentItem(pItem);
         }
      }
   }
}

bool HistogramPlotImp::setColorMapFromFile(const QString& filename)
{
   if (filename.isEmpty() == true)
   {
      return false;
   }

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if (pRasterLayer == NULL)
   {
      return false;
   }

   try
   {
      ColorMap colorMap(filename.toStdString());
      pRasterLayer->setColorMap(colorMap.getName(), colorMap.getTable());
      return true;
   }
   catch (const std::exception&)
   {
      QMessageBox::critical(this, "Bad Color Table",
         QString("The file selected was not a valid %1 color table file").arg(APP_NAME));
   }

   return false;
}

void HistogramPlotImp::setColorMapFromFile()
{
   QString strFilename = QFileDialog::getOpenFileName(this, "Select ColorMap File...", QString(), "*.clu *.cgr");
   if (strFilename.isEmpty() == false)
   {
      setColorMapFromFile(strFilename);
   }
}

void HistogramPlotImp::saveColorMapToFile()
{
   RasterLayerImp* pRasterLayer = dynamic_cast<RasterLayerImp*> (mpLayer.get());
   if (pRasterLayer == NULL)
   {
      return;
   }

   bool goodSave = false;
   while (!goodSave)
   {
      QString strFilename = QFileDialog::getSaveFileName(this, "Save Color Map File...", QString(), "*.clu");
      if (strFilename.isEmpty() == true)
      {
         break;
      }

      if (strFilename.endsWith(".clu") == false)
      {
         strFilename += ".clu";
      }

      goodSave = pRasterLayer->getColorMap().saveToFile(strFilename.toStdString());
   }
}

void HistogramPlotImp::createColorMap()
{
   RasterLayerImp* pRasterLayer = dynamic_cast<RasterLayerImp*> (mpLayer.get());
   if (pRasterLayer == NULL)
   {
      return;
   }

   if (pRasterLayer->getDisplayMode() != GRAYSCALE_MODE)
   {
      int response = QMessageBox::warning(this, "Change to Grayscale?", "The image is not currently in grayscale "
         "mode. It must be\nin grayscale mode in order to apply a colormap. Do you\nwish to switch to grayscale "
         "mode and proceed?", QMessageBox::Yes, QMessageBox::No);
      if (response == QMessageBox::No)
      {
         return;
      }

      pRasterLayer->setDisplayMode(GRAYSCALE_MODE);
   }

   ColormapEditor* pEditor = new ColormapEditor(*this);
   pEditor->show();
}

void HistogramPlotImp::setThresholdMode(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   ThresholdLayer* pThresholdLayer = dynamic_cast<ThresholdLayer*>(mpLayer.get());
   if (pThresholdLayer == NULL)
   {
      return;
   }

   PassArea passArea = pThresholdLayer->getPassArea();
   if (pAction == mpBelowAction)
   {
      passArea = LOWER;
   }
   else if (pAction == mpAboveAction)
   {
      passArea = UPPER;
   }
   else if (pAction == mpBetweenAction)
   {
      passArea = MIDDLE;
   }
   else if (pAction == mpOutsideAction)
   {
      passArea = OUTSIDE;
   }

   pThresholdLayer->setPassArea(passArea);
}

void HistogramPlotImp::setStretchUnits(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if (pRasterLayer != NULL)
   {
      RegionUnits eCurrentUnits = pRasterLayer->getStretchUnits(mRasterChannelType);
      RegionUnits eNewUnits;

      if (pAction == mpRawAction)
      {
         eNewUnits = RAW_VALUE;
      }
      else if (pAction == mpPercentageAction)
      {
         eNewUnits = PERCENTAGE;
      }
      else if (pAction == mpPercentileAction)
      {
         eNewUnits = PERCENTILE;
      }
      else if (pAction == mpStdDevAction)
      {
         eNewUnits = STD_DEV;
      }

      double lower;
      double upper;
      pRasterLayer->getStretchValues(mRasterChannelType, lower, upper);
      lower = pRasterLayer->convertStretchValue(mRasterChannelType, eCurrentUnits, lower, eNewUnits);
      upper = pRasterLayer->convertStretchValue(mRasterChannelType, eCurrentUnits, upper, eNewUnits);
      pRasterLayer->setStretchValues(mRasterChannelType, lower, upper);
      pRasterLayer->setStretchUnits(mRasterChannelType, eNewUnits);
   }

   ThresholdLayer* pThresholdLayer = dynamic_cast<ThresholdLayer*>(mpLayer.get());
   if (pThresholdLayer != NULL)
   {
      RegionUnits regionUnits;
      if (pAction == mpRawAction)
      {
         regionUnits = RAW_VALUE;
      }
      else if (pAction == mpPercentageAction)
      {
         regionUnits = PERCENTAGE;
      }
      else if (pAction == mpPercentileAction)
      {
         regionUnits = PERCENTILE;
      }
      else if (pAction == mpStdDevAction)
      {
         regionUnits = STD_DEV;
      }

      pThresholdLayer->setRegionUnits(regionUnits);
   }
}

void HistogramPlotImp::stretchResetThisChannel()
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if (pRasterLayer != NULL)
   {
      pRasterLayer->resetStretchType(mRasterChannelType == GRAY ? GRAYSCALE_MODE : RGB_MODE);
      pRasterLayer->resetStretchUnits(mRasterChannelType);
      pRasterLayer->resetStretchValues(mRasterChannelType);
   }
}

void HistogramPlotImp::stretchResetAllChannels()
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if (pRasterLayer != NULL)
   {
      pRasterLayer->resetStretch();
   }
}

void HistogramPlotImp::setStretchMode(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if (pRasterLayer == NULL)
   {
      return;
   }

   DisplayMode eMode = mRasterChannelType == GRAY ? GRAYSCALE_MODE : RGB_MODE;
   StretchType eType = pRasterLayer->getStretchType(eMode);

   if (pAction == mpLinearAction)
   {
      eType = LINEAR;
   }
   else if (pAction == mpExponentialAction)
   {
      eType = EXPONENTIAL;
   }
   else if (pAction == mpLogarithmicAction)
   {
      eType = LOGARITHMIC;
   }
   else if (pAction == mpEqualizationAction)
   {
      eType = EQUALIZATION;
   }

   pRasterLayer->setStretchType(eMode, eType);
}

void HistogramPlotImp::setComplexComponent(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if (pRasterLayer == NULL)
   {
      return;
   }

   ComplexComponent component = pRasterLayer->getComplexComponent();
   if (pAction == mpMagnitudeAction)
   {
      component = COMPLEX_MAGNITUDE;
   }
   else if (pAction == mpPhaseAction)
   {
      component = COMPLEX_PHASE;
   }
   else if (pAction == mpInPhaseAction)
   {
      component = COMPLEX_INPHASE;
   }
   else if (pAction == mpQuadratureAction)
   {
      component = COMPLEX_QUADRATURE;
   }

   pRasterLayer->setComplexComponent(component);
}

void HistogramPlotImp::setDisplayedElement(QListWidgetItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if (pRasterLayer == NULL)
   {
      return;
   }

   // Close the context menu by simulating a mouse click on the menu
   if (mpElementMenu != NULL)
   {
      QMouseEvent mousePress(QEvent::MouseButtonPress, mpElementMenu->mapFromGlobal(QCursor::pos()),
         Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
      QApplication::sendEvent(mpElementMenu, &mousePress);

      QMouseEvent mouseRelease(QEvent::MouseButtonRelease, mpElementMenu->mapFromGlobal(QCursor::pos()),
         Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
      QApplication::sendEvent(mpElementMenu, &mouseRelease);
   }

   // Get the displayed element from the list widget item
   RasterElement* pRasterElement = pItem->data(Qt::UserRole).value<RasterElement*>();
   if (pRasterElement != pRasterLayer->getDisplayedRasterElement(mRasterChannelType))
   {
      // Set the displayed band to the first band
      DimensionDescriptor band;

      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         const vector<DimensionDescriptor>& activeBands = pDescriptor->getBands();
         if (activeBands.empty() == false)
         {
            band = activeBands.front();
         }
      }

      pRasterLayer->setDisplayedBand(mRasterChannelType, band, pRasterElement);
   }
}

void HistogramPlotImp::updateElement()
{
   // Update the member element
   RasterElement* pElement = NULL;

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if (pRasterLayer != NULL)
   {
      pElement = pRasterLayer->getDisplayedRasterElement(mRasterChannelType);
   }
   else if (mpLayer.get() != NULL)
   {
      pElement = dynamic_cast<RasterElement*>(mpLayer->getDataElement());
   }

   if (pElement != mpElement.get())
   {
      // Reset the element pointer, which updates the histogram values
      mpElement.reset(pElement);
   }
   else
   {
      // Since updateElement() is called when the displayed band changes in the raster layer,
      // update the histogram values even if the displayed element does not change
      updateHistogramValues();
   }
}

void HistogramPlotImp::initializeElementList()
{
   if (mpElementList == NULL)
   {
      return;
   }

   mpElementList->clear();

   if (mpLayer.get() == NULL)
   {
      return;
   }

   // Get the displayed element
   RasterElement* pDisplayedRasterElement = NULL;

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if (pRasterLayer != NULL)
   {
      pDisplayedRasterElement = pRasterLayer->getDisplayedRasterElement(mRasterChannelType);
   }

   // Populate the list with all displayable elements
   RasterElement* pRasterElement = dynamic_cast<RasterElement*>(mpLayer->getDataElement());
   if (pRasterElement != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         unsigned int rows = pDescriptor->getRowCount();
         unsigned int columns = pDescriptor->getColumnCount();

         Service<ModelServices> pModel;
         vector<DataElement*> rasterElements = pModel->getElements("RasterElement");

         for (unsigned int i = 0; i < rasterElements.size(); ++i)
         {
            RasterElement* pCurrentRasterElement = dynamic_cast<RasterElement*>(rasterElements[i]);
            if (pCurrentRasterElement != NULL)
            {
               unsigned int currentRows = 0;
               unsigned int currentColumns = 0;

               const RasterDataDescriptor* pCurrentDescriptor =
                  dynamic_cast<const RasterDataDescriptor*>(pCurrentRasterElement->getDataDescriptor());
               if (pCurrentDescriptor != NULL)
               {
                  currentRows = pCurrentDescriptor->getRowCount();
                  currentColumns = pCurrentDescriptor->getColumnCount();
               }

               if ((currentRows == rows) && (currentColumns == columns))
               {
                  string displayName = pCurrentRasterElement->getDisplayName();
                  if (displayName.empty() == true)
                  {
                     displayName = pCurrentRasterElement->getName();
                  }

                  if (displayName.empty() == false)
                  {
                     QListWidgetItem* pItem = new QListWidgetItem(QString::fromStdString(displayName), mpElementList);
                     pItem->setData(Qt::UserRole, QVariant::fromValue(pCurrentRasterElement));

                     // Select the displayed element
                     if (pCurrentRasterElement == pDisplayedRasterElement)
                     {
                        pItem->setSelected(true);
                        mpElementList->setCurrentItem(pItem);
                     }
                  }
               }
            }
         }
      }
   }
}

void HistogramPlotImp::setBand(QListWidgetItem* pItem)
{
   if ((pItem == NULL) || (mpBandList == NULL))
   {
      return;
   }

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if (pRasterLayer == NULL)
   {
      return;
   }

   // Close the context menu by simulating a mouse click on the menu
   if (mpBandMenu != NULL)
   {
      QMouseEvent mousePress(QEvent::MouseButtonPress, mpBandMenu->mapFromGlobal(QCursor::pos()),
         Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
      QApplication::sendEvent(mpBandMenu, &mousePress);

      QMouseEvent mouseRelease(QEvent::MouseButtonRelease, mpBandMenu->mapFromGlobal(QCursor::pos()),
         Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
      QApplication::sendEvent(mpBandMenu, &mouseRelease);
   }

   // Get the band based on the active band number
   if (mpElement.get() != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(mpElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         int activeNumber = mpBandList->row(pItem);

         DimensionDescriptor bandDim = pDescriptor->getActiveBand(activeNumber);
         pRasterLayer->setDisplayedBand(mRasterChannelType, bandDim, mpElement.get());
      }
   }
}

void HistogramPlotImp::initializeBandList()
{
   if (mpBandList == NULL)
   {
      return;
   }

   mpBandList->clear();

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
   if ((pRasterLayer == NULL) || (mpElement.get() == NULL))
   {
      return;
   }

   const RasterDataDescriptor* pDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(mpElement->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   // Get all band names
   vector<string> bandNames = RasterUtilities::getBandNames(pDescriptor);

   // Get the displayed band name
   QString displayedBand;

   DimensionDescriptor band = pRasterLayer->getDisplayedBand(mRasterChannelType);
   if (band.isActiveNumberValid() == true)
   {
      unsigned int activeNumber = band.getActiveNumber();
      if (activeNumber < bandNames.size())
      {
         displayedBand = QString::fromStdString(bandNames[activeNumber]);
      }
   }

   // Populate the list with all band names
   const vector<DimensionDescriptor>& activeBands = pDescriptor->getBands();
   for (unsigned int i = 0; i < activeBands.size(); ++i)
   {
      QString bandName;

      const DimensionDescriptor& bandDim = activeBands[i];
      if (i < bandNames.size())
      {
         bandName = QString::fromStdString(bandNames[i]);
      }

      QListWidgetItem* pItem = new QListWidgetItem(bandName, mpBandList);

      // Select the displayed band
      if (bandName == displayedBand)
      {
         pItem->setSelected(true);
         mpBandList->setCurrentItem(pItem);
      }
   }
}

void HistogramPlotImp::updateHistogramName()
{
   updateHistogramName(false);
}

void HistogramPlotImp::updateHistogramName(bool force)
{
   if (force || isVisible())
   {
      if (mpLayer.get() != NULL)
      {
         const string& layerName = mpLayer->getName();
         if (layerName.empty() == false)
         {
            setName(layerName);
         }
      }
   }
   else
   {
      mUpdater.initialize();
   }
}

void HistogramPlotImp::updateHistogramValues()
{
   updateHistogramValues(false);
}

void HistogramPlotImp::updateHistogramValues(bool force)
{
   if (mpLayer.get() == NULL)
   {
      return;
   }

   if (force || isVisible())
   {
      // Get the histogram data
      const unsigned int* pHistogramCounts = NULL;
      const double* pHistogramLocations = NULL;

      Statistics* pStatistics = getStatistics();
      if (pStatistics != NULL)
      {
         ComplexComponent eComponent = RasterLayer::getSettingComplexComponent();

         RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
         if (pRasterLayer)
         {
            eComponent = pRasterLayer->getComplexComponent();
         }

         pStatistics->getHistogram(pHistogramLocations, pHistogramCounts, eComponent);
      }

      if ((pHistogramLocations != NULL) && (pHistogramCounts != NULL))
      {
         unsigned int uiCount = 256;

         double* pCounts = new double[uiCount];
         if (pCounts != NULL)
         {
            for (unsigned int i = 0; i < uiCount; i++)
            {
               pCounts[i] = static_cast<double>(pHistogramCounts[i]);
            }
         }

         setHistogram(uiCount, pHistogramLocations, pCounts);
         delete [] pCounts;
      }
      else
      {
         setHistogram(0, NULL, NULL);
      }

      updateHistogramRegions(force);
   }
   else
   {
      mUpdater.initialize();
   }
}

void HistogramPlotImp::updateHistogramRegions()
{
   updateHistogramRegions(false);
}

void HistogramPlotImp::updateHistogramRegions(bool force)
{
   if (force || isVisible())
   {
      double lowerLimit = 0.0;
      double upperLimit = 0.0;
      getDataLowerUpper(lowerLimit, upperLimit);

      double minValue = 0.0;
      double maxValue = 0.0;
      getDataMinMax(minValue, maxValue);

      PassArea ePassArea = getLayerPassArea();
      QColor regionColor = getLayerColor();
      bool showRegions = true;

      RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(mpLayer.get());
      if (pRasterLayer != NULL)
      {
         if ((pRasterLayer->getStretchType(pRasterLayer->getDisplayMode()) == EQUALIZATION) ||
            (pRasterLayer->getDisplayedBand(mRasterChannelType).isValid() == false))
         {
            showRegions = false;
         }
      }

      updateHistogramRegions(lowerLimit, upperLimit, minValue, maxValue, ePassArea, regionColor, showRegions);
   }
   else
   {
      mUpdater.initialize();
   }
}

void HistogramPlotImp::updateHistogramRegionExtents()
{
   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;

   translateScreenToData(0, 0, dMinX, dMinY);
   translateScreenToData(width() - 1, height() - 1, dMaxX, dMaxY);

   double dLeft = 0.0;
   double dBottom = 0.0;
   double dRight = 0.0;
   double dTop = 0.0;

   if (mpRegion != NULL)
   {
      bool bSuccess = false;
      bSuccess = mpRegion->getRegion(dLeft, dBottom, dRight, dTop);
      if (bSuccess == true)
      {
         mpRegion->setRegion(dLeft, dMinY, dRight, dMaxY);
      }
   }

   if (mpRegion2 != NULL)
   {
      bool bSuccess = false;
      bSuccess = mpRegion2->getRegion(dLeft, dBottom, dRight, dTop);
      if (bSuccess == true)
      {
         mpRegion2->setRegion(dLeft, dMinY, dRight, dMaxY);
      }
   }

   refresh();
}

bool HistogramPlotImp::isAutoZoomEnabled() const
{
   return mAutoZoom;
}

RasterChannelType HistogramPlotImp::getRasterChannelType() const
{
   return mRasterChannelType;
}

Layer* HistogramPlotImp::getLayer() const
{
   return const_cast<Layer*>(mpLayer.get());
}

void HistogramPlotImp::setResolution()
{
   Statistics* pStatistics = getStatistics();
   if (pStatistics == NULL)
   {
      return;
   }

   int oldResolution = pStatistics->getStatisticsResolution();
   bool bSuccess = false;

   int newResolution = QInputDialog::getInteger(this, "Statistics Resolution",
      "Enter the skip factor for computing the statistics:", oldResolution, 1, 100, 1, &bSuccess);

   if ((bSuccess == true) && (oldResolution != newResolution))
   {
      pStatistics->setStatisticsResolution(newResolution);

      if (mpElement.get() != NULL)
      {
         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(mpElement->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            const vector<DimensionDescriptor>& allBands = pDescriptor->getBands();
            for (unsigned int i = 0; i < allBands.size(); ++i)
            {
               DimensionDescriptor bandDim = allBands[i];
               if (bandDim.isValid())
               {
                  pStatistics = mpElement->getStatistics(bandDim);
                  if (pStatistics != NULL)
                  {
                     pStatistics->setStatisticsResolution(newResolution);
                  }
               }
            }
         }

         mpElement->updateData();
      }

      updateHistogramValues();
   }
}

void HistogramPlotImp::setBadValues()
{
   Statistics* pStatistics = getStatistics();
   if (pStatistics == NULL)
   {
      return;
   }

   vector<int> badValues = pStatistics->getBadValues();

   BandBadValuesDlg bandBadValuesDlg(this);
   bandBadValuesDlg.setModal(true);
   bandBadValuesDlg.setBadValues(badValues);

   if (bandBadValuesDlg.exec() == QDialog::Accepted)
   {
      bandBadValuesDlg.getBadValues(badValues);
      pStatistics->setBadValues(badValues);

      if (mpElement.get() != NULL)
      {
         if (bandBadValuesDlg.getChangeForAllBands() == true)
         {
            const RasterDataDescriptor* pDescriptor =
               dynamic_cast<const RasterDataDescriptor*>(mpElement->getDataDescriptor());
            if (pDescriptor != NULL)
            {
               const vector<DimensionDescriptor>& allBands = pDescriptor->getBands();
               for (unsigned int i = 0; i < allBands.size(); ++i)
               {
                  DimensionDescriptor bandDim = allBands[i];
                  if (bandDim.isValid())
                  {
                     pStatistics = mpElement->getStatistics(bandDim);
                     if (pStatistics != NULL)
                     {
                        pStatistics->setBadValues(badValues);
                     }
                  }
               }
            }
         }

         mpElement->updateData();
      }

      updateHistogramValues();
   }
}

void HistogramPlotImp::attached(Subject& subject, const string& signal, const Slot& slot)
{
   elementModified(subject, signal, boost::any());
}

void HistogramPlotImp::detached(Subject& subject, const string& signal, const Slot& slot)
{
   elementDeleted(subject, signal, boost::any());
}

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Remove QListWidget subclass when QListWidget " \
   "defines an appropriate size hint! (Qt 4.5.2) (dsulgrov)")
QSize HistogramPlotImp::MenuListWidget::sizeHint() const
{
   int iWidth = sizeHintForColumn(0) + 25;   // Add 25 to provide room for the scroll bar
   int iHeight = 150;

   return QSize(iWidth, iHeight);
}

namespace StringUtilities
{
template<>
std::string toXmlString(const HistogramPlotImp::ValuesType& val, bool* pError)
{
   string retValue;
   switch (val)
   {
   case HistogramPlotImp::NO_VALUE:
      retValue = "none";
      break;
   case HistogramPlotImp::LOWER_VALUE:
      retValue = "lower";
      break;
   case HistogramPlotImp::UPPER_VALUE:
      retValue = "upper";
      break;
   default:
      break;
   }
   if (pError != NULL)
   {
      *pError = (retValue.empty());
   }
   return retValue;
}

template<>
HistogramPlotImp::ValuesType fromXmlString<HistogramPlotImp::ValuesType>(string value, bool* pError)
{
   HistogramPlotImp::ValuesType retValue;
   if (value == "none") 
   {
      retValue = HistogramPlotImp::NO_VALUE;
   }
   else if (value == "lower")
   {
      retValue = HistogramPlotImp::LOWER_VALUE;
   }
   else if (value == "upper")
   {
      retValue = HistogramPlotImp::UPPER_VALUE;
   }
   if (pError != NULL)
   {
      *pError = (!retValue.isValid());
   }
   return retValue;
}
}

bool HistogramPlotImp::toXml(XMLWriter* pXml) const
{
   if (!CartesianPlotImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("selectedValue", StringUtilities::toXmlString(meSelectedValue));
   pXml->addAttr("rasterChannelType", mRasterChannelType);
   if (mpLayer.get() != NULL)
   {
      pXml->addAttr("layerId", mpLayer->getId());
   }

   if (mPreloadedColorMaps.empty() == false)
   {
      pXml->pushAddPoint(pXml->addElement("PreloadedColorMaps"));
      map<string, string>::const_iterator it;
      for (it = mPreloadedColorMaps.begin(); it != mPreloadedColorMaps.end(); ++it)
      {
         pXml->pushAddPoint(pXml->addElement("ColorMap"));
         pXml->addAttr("name", it->first);
         pXml->addAttr("filename", it->second);
         pXml->popAddPoint();
      }
      pXml->popAddPoint();
   }
   pXml->pushAddPoint(pXml->addElement("Histogram"));
   mpHistogram->toXml(pXml);
   pXml->popAddPoint();
   pXml->pushAddPoint(pXml->addElement("Region1"));
   mpRegion->toXml(pXml);
   pXml->popAddPoint();
   pXml->pushAddPoint(pXml->addElement("Region2"));
   mpRegion2->toXml(pXml);
   pXml->popAddPoint();

   return true;
}

bool HistogramPlotImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (!CartesianPlotImp::fromXml(pDocument, version))
   {
      return false;
   }
   DOMElement* pElement = static_cast<DOMElement*>(pDocument);
   meSelectedValue = StringUtilities::fromXmlString<HistogramPlotImp::ValuesType>(
      A(pElement->getAttribute(X("selectedValue"))));
   RasterChannelType channel = StringUtilities::fromXmlString<RasterChannelType>(
      A(pElement->getAttribute(X("rasterChannelType"))));
   Layer* pLayer = NULL;
   mpLayer.reset(NULL);

   if (pElement->hasAttribute(X("layerId")))
   {
      pLayer = dynamic_cast<Layer*>(Service<SessionManager>()->getSessionItem(A(pElement->getAttribute(X("layerId")))));
      if (pLayer == NULL)
      {
         return false;
      }
   }

   if (!setHistogram(pLayer, channel))
   {
      return false;
   }

   mpAlternateColormap = NULL;
   for (DOMNode* pChild = pElement->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X("Histogram")))
      {
         if (!mpHistogram->fromXml(pChild, version))
         {
            return false;
         }
      }
      else if (XMLString::equals(pChild->getNodeName(), X("Region1")))
      {
         for (DOMNode* pGChld = pChild->getFirstChild(); pGChld != NULL; pGChld = pGChld->getNextSibling())
         {
            if (pGChld->getNodeType() == DOMNode::ELEMENT_NODE)
            {
               string name = A(pGChld->getNodeName());
               if (!mpRegion->fromXml(pGChld, version))
               {
                  return false;
               }
            }
         }
      }
      else if (XMLString::equals(pChild->getNodeName(), X("Region2")))
      {
         for (DOMNode* pGChld = pChild->getFirstChild(); pGChld != NULL; pGChld = pGChld->getNextSibling())
         {
            if (pGChld->getNodeType() == DOMNode::ELEMENT_NODE)
            {
               if (!mpRegion2->fromXml(pGChld, version))
               {
                  return false;
               }
            }
         }
      }
   }

   return true;
}

HistogramPlotImp::HistogramUpdater::HistogramUpdater(HistogramPlotImp* pPlot) :
   mpPlot(pPlot),
   mNeedsUpdated(false)
{
}

void HistogramPlotImp::HistogramUpdater::initialize()
{
   mNeedsUpdated = true;
}

void HistogramPlotImp::HistogramUpdater::update()
{
   if (mpPlot != NULL && mNeedsUpdated)
   {
      mpPlot->updateHistogramValues(true);
      mpPlot->updateHistogramRegions(true);
      mpPlot->updateHistogramName(true);
      mNeedsUpdated = false;
   }
}
