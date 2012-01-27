/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AoiLayer.h"
#include "AppVerify.h"
#include "ContextMenu.h"
#include "ContextMenuActions.h"
#include "GetSubsetDialog.h"
#include "HistogramPlotAdapter.h"
#include "HistogramWindow.h"
#include "HistogramWindowImp.h"
#include "Layer.h"
#include "LayerListAdapter.h"
#include "PlotSetAdapter.h"
#include "PlotSetGroupAdapter.h"
#include "PlotWidget.h"
#include "PlotWidgetImp.h"
#include "ProductView.h"
#include "ProductWindow.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterElementImp.h"
#include "RasterLayerAdapter.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataViewImp.h"
#include "SpatialDataWindow.h"
#include "StatisticsImp.h"
#include "StatisticsWidget.h"
#include "StringUtilities.h"
#include "ThresholdLayer.h"
#include "WorkspaceWindow.h"

#include <QtCore/QEvent>
#include <QtGui/QSplitter>
#include <vector>
using namespace std;
XERCES_CPP_NAMESPACE_USE

HistogramWindowImp::HistogramWindowImp(const string& id, const string& windowName, QWidget* pParent) :
   DockWindowImp(id, windowName, pParent),
   mpDesktop(Service<DesktopServices>().get()),
   mpExplorer(Service<SessionExplorer>().get(), SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu),
      Slot(this, &HistogramWindowImp::updateContextMenu)),
   mpSessionManager(Service<SessionManager>().get()),
   mpPlotSetGroup(NULL),
   mDisplayModeChanging(false),
   mAddingStatisticsPlot(false),
   mUpdater(this)
{
   // Context menu actions
   mpSyncAutoZoomAction = new QAction("Synchronize Auto Zoom", this);
   mpSyncAutoZoomAction->setAutoRepeat(false);
   VERIFYNR(connect(mpSyncAutoZoomAction, SIGNAL(triggered()), this, SLOT(syncAutoZoom())));
   mpDesktop->initializeAction(mpSyncAutoZoomAction, "Histogram Plot");    // Use the plot context since the action
                                                                           // will appear on the plot menu

   mpStatisticsShowAction = new QAction("Statistics", this);
   mpStatisticsShowAction->setAutoRepeat(false);
   mpStatisticsShowAction->setCheckable(true);
   mpStatisticsShowAction->setStatusTip("Toggles the display of the statistics");
   VERIFYNR(connect(mpStatisticsShowAction, SIGNAL(triggered(bool)), this, SLOT(setStatisticsShown(bool))));
   mpDesktop->initializeAction(mpStatisticsShowAction, "Histogram Plot");  // Use the plot context since the action
                                                                           // will appear on the plot menu

   // Plot set
   QIcon histogramIcon(":/icons/HistogramWindow");
   mpPlotSetGroup = new PlotSetGroupAdapter(this);
   mpPlotSetGroup->setInfoBarIcon(histogramIcon);

   // Initialization
   setIcon(histogramIcon);
   setWidget(mpPlotSetGroup->getWidget());

   // Connections
   VERIFYNR(mpPlotSetGroup->attach(SIGNAL_NAME(PlotSetGroup, PlotSetAdded),
      Slot(this, &HistogramWindowImp::plotSetAdded)));
   VERIFYNR(mpPlotSetGroup->attach(SIGNAL_NAME(PlotSetGroup, PlotSetActivated),
      Slot(this, &HistogramWindowImp::plotSetActivated)));
   VERIFYNR(mpPlotSetGroup->attach(SIGNAL_NAME(PlotSetGroup, PlotSetDeleted),
      Slot(this, &HistogramWindowImp::plotSetDeleted)));

   mpDesktop.addSignal(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &HistogramWindowImp::windowAdded));
   mpDesktop.addSignal(SIGNAL_NAME(DesktopServices, WindowRemoved), Slot(this, &HistogramWindowImp::windowRemoved));
   mpSessionManager.addSignal(SIGNAL_NAME(SessionManager, SessionRestored),
      Slot(this, &HistogramWindowImp::sessionRestored));
}

HistogramWindowImp::~HistogramWindowImp()
{}

void HistogramWindowImp::updateContextMenu(Subject& subject, const string& signal, const boost::any& value)
{
   ContextMenu* pMenu = boost::any_cast<ContextMenu*>(value);
   if (pMenu == NULL)
   {
      return;
   }

   bool bAddActions = false;
   bool bRemoveActions = false;
   PlotWidget* pActionWidget = NULL;

   if (dynamic_cast<SessionExplorer*>(&subject) != NULL)
   {
      // Make sure all of the selected session items for the menu are plot widgets
      vector<SessionItem*> items = pMenu->getSessionItems();
      vector<PlotWidget*> plots = pMenu->getSessionItems<PlotWidget>();
      if (plots.size() != items.size())
      {
         return;
      }

      // Make sure all selected plot widget items are contained in this plot set
      vector<PlotWidget*>::iterator iter;
      for (iter = plots.begin(); iter != plots.end(); ++iter)
      {
         PlotWidget* pPlot = *iter;
         if (pPlot != NULL)
         {
            if (mpPlotSetGroup->containsPlot(pPlot) == true)
            {
               if (plots.size() == 1)
               {
                  bAddActions = true;
                  pActionWidget = pPlot;
               }

               HistogramPlotImp* pHistogramPlot = dynamic_cast<HistogramPlotImp*>(pPlot->getPlot());
               if (pHistogramPlot != NULL)
               {
                  if (pHistogramPlot->getLayer() != NULL)
                  {
                     bRemoveActions = true;
                  }
               }
            }
            else
            {
               return;
            }
         }
      }
   }
   else if (dynamic_cast<HistogramWindowImp*>(&subject) == this)
   {
      if (mpPlotSetGroup->getNumPlotSets() > 0)
      {
         bRemoveActions = true;
      }
   }
   else
   {
      PlotWidget* pPlotWidget = dynamic_cast<PlotWidget*>(&subject);
      if ((pPlotWidget != NULL) && (mpPlotSetGroup->containsPlot(pPlotWidget) == true))
      {
         bAddActions = true;
         pActionWidget = pPlotWidget;
      }
   }

   // Add the sync zoom action
   if ((bAddActions == true) && (pActionWidget != NULL))
   {
      HistogramPlotImp* pHistogramPlot = dynamic_cast<HistogramPlotImp*>(pActionWidget->getPlot());
      if (pHistogramPlot != NULL)
      {
         RasterLayer* pLayer = dynamic_cast<RasterLayer*>(pHistogramPlot->getLayer());
         if ((pLayer != NULL) && (pHistogramPlot->getRasterChannelType() != GRAY))
         {
            mpSyncAutoZoomAction->setData(QVariant::fromValue(dynamic_cast<SessionItem*>(pHistogramPlot)));
            pMenu->addActionBefore(mpSyncAutoZoomAction, APP_HISTOGRAMPLOT_SYNCHRONIZE_AUTO_ZOOM_ACTION,
               APP_HISTOGRAMPLOT_RASTER_MENUS_SEPARATOR_ACTION);
         }
      }
   }

   // Remove the delete action
   if (bRemoveActions == true)
   {
      pMenu->removeAction(APP_PLOTSET_DELETE_ACTION);
   }

   // Add statistics actions
   if (bAddActions)
   {
      pMenu->addActionBefore(mpStatisticsShowAction, APP_HISTOGRAMPLOT_STATISTICS_ACTION,
         APP_HISTOGRAMPLOT_REFRESH_STATISTICS_ACTION);
   }
}

PlotWidget* HistogramWindowImp::getPlot(Layer* pLayer) const
{
   if (pLayer == NULL)
   {
      return NULL;
   }

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
   if (pRasterLayer != NULL)
   {
      RasterChannelType channel = GRAY;
      if (pRasterLayer->getDisplayMode() == RGB_MODE)
      {
         channel = RED;
      }

      return getPlot(pRasterLayer, channel);
   }

   // Iterate over all histogram plots on all plot sets to find the plot
   vector<PlotWidget*> plots = mpPlotSetGroup->getPlots(HISTOGRAM_PLOT);
   for (vector<PlotWidget*>::iterator iter = plots.begin(); iter != plots.end(); ++iter)
   {
      PlotWidget* pPlot = *iter;
      if (pPlot != NULL)
      {
         HistogramPlotImp* pHistogramPlot = dynamic_cast<HistogramPlotImp*>(pPlot->getPlot());
         if (pHistogramPlot != NULL)
         {
            Layer* pCurrentLayer = pHistogramPlot->getLayer();
            if (pCurrentLayer == pLayer)
            {
               return pPlot;
            }
         }
      }
   }

   return NULL;
}

PlotWidget* HistogramWindowImp::getPlot(Layer* pLayer, const RasterChannelType& eColor) const
{
   if (pLayer == NULL)
   {
      return NULL;
   }

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
   if (pRasterLayer != NULL)
   {
      return getPlot(pRasterLayer, eColor);
   }

   return getPlot(pLayer);
}

PlotWidget* HistogramWindowImp::getPlot(RasterLayer* pLayer, RasterChannelType channel) const
{
   return getPlot(pLayer, channel, false);
}

PlotWidget* HistogramWindowImp::getPlot(RasterLayer* pLayer, RasterChannelType channel, bool ignoreStatisticsPlots) const
{
   if (pLayer == NULL)
   {
      return NULL;
   }

   // Iterate over all histogram plots on all plot sets to find the plot
   vector<PlotWidget*> plots = mpPlotSetGroup->getPlots(HISTOGRAM_PLOT);
   for (vector<PlotWidget*>::iterator iter = plots.begin(); iter != plots.end(); ++iter)
   {
      PlotWidget* pPlot = *iter;
      if (pPlot != NULL)
      {
         HistogramPlotImp* pHistogramPlot = dynamic_cast<HistogramPlotImp*>(pPlot->getPlot());
         if (pHistogramPlot != NULL && (!ignoreStatisticsPlots || !pHistogramPlot->ownsStatistics()))
         {
            RasterLayer* pCurrentLayer = dynamic_cast<RasterLayer*>(pHistogramPlot->getLayer());
            if (pCurrentLayer == pLayer)
            {
               RasterChannelType currentChannel = pHistogramPlot->getRasterChannelType();
               if (currentChannel == channel)
               {
                  return pPlot;
               }
            }
         }
      }
   }

   return NULL;
}

bool HistogramWindowImp::toXml(XMLWriter* pXml) const
{
   if ((pXml == NULL) || (DockWindowImp::toXml(pXml) == false))
   {
      return false;
   }

   if (mpPlotSetGroup->toXml(pXml) == false)
   {
      return false;
   }

   vector<PlotWidget*> plots = mpPlotSetGroup->getPlots(HISTOGRAM_PLOT);
   for (vector<PlotWidget*>::const_iterator iter = plots.begin(); iter != plots.end(); ++iter)
   {
      PlotWidgetImp* pPlotWidget = dynamic_cast<PlotWidgetImp*>(*iter);
      if (pPlotWidget != NULL)
      {
         pXml->pushAddPoint(pXml->addElement("HistogramPlot"));
         pXml->addAttr("id", pPlotWidget->getId());

         HistogramPlotImp* pPlot = dynamic_cast<HistogramPlotImp*>(pPlotWidget->getPlot());
         if (pPlot != NULL)
         {
            Layer* pLayer = pPlot->getLayer();
            if (pLayer != NULL)
            {
               pXml->addAttr("layerId", pLayer->getId());
            }
         }

         bool statsShown = mpStatisticsShowAction->isChecked();

         QSplitter* pSplitter = pPlotWidget->getSplitter();
         if (pSplitter != NULL)
         {
            QWidget* pWidget = pSplitter->widget(2);
            if (pWidget != NULL)
            {
               statsShown = pWidget->isVisibleTo(pSplitter);
            }
         }

         pXml->addAttr("statisticsShown", statsShown);
         pXml->popAddPoint();
      }
   }

   return true;
}

bool HistogramWindowImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if ((pDocument == NULL) || (DockWindowImp::fromXml(pDocument, version) == false))
   {
      return false;
   }

   if (mpPlotSetGroup->fromXml(pDocument, version) == false)
   {
      return false;
   }

   vector<Window*> windows;
   mpDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);
   for (vector<Window*>::const_iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(*iter);
      if (pWindow != NULL)
      {
         SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
         if (pView != NULL)
         {
            VERIFYNR(connect(pView, SIGNAL(layerAdded(Layer*)), this, SLOT(createPlot(Layer*))));
            VERIFYNR(connect(pView, SIGNAL(layerActivated(Layer*)), this, SLOT(setCurrentPlot(Layer*))));
            VERIFYNR(pView->attach(SIGNAL_NAME(SpatialDataView, LayerShown),
               Slot(this, &HistogramWindowImp::layerShown)));
         }
      }
   }

   mShowStatistics.clear();

   for (DOMNode* pChild = pDocument->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X("HistogramPlot")))
      {
         DOMElement* pElement = static_cast<DOMElement*>(pChild);

         // Attach the plot widget
         PlotWidget* pPlotWidget = dynamic_cast<PlotWidget*>(mpSessionManager->getSessionItem(
            A(pElement->getAttribute(X("id")))));
         if (pPlotWidget != NULL)
         {
            VERIFYNR(pPlotWidget->attach(SIGNAL_NAME(PlotWidget, AboutToShowContextMenu),
               Slot(this, &HistogramWindowImp::updateContextMenu)));
         }

         // Attach the layer
         Layer* pLayer = dynamic_cast<Layer*>(mpSessionManager->getSessionItem(
            A(pElement->getAttribute(X("layerId")))));

         RasterLayerImp* pRasterLayer = dynamic_cast<RasterLayerImp*>(pLayer);
         if (pRasterLayer != NULL)
         {
            VERIFYNR(connect(pRasterLayer, SIGNAL(displayModeChanged(const DisplayMode&)), this,
               SLOT(setCurrentPlot(const DisplayMode&))));
            VERIFYNR(connect(pRasterLayer, SIGNAL(displayedBandChanged(RasterChannelType, DimensionDescriptor)),
               this, SLOT(updatePlotInfo(RasterChannelType))));
            pRasterLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &HistogramWindowImp::layerDeleted));
         }

         ThresholdLayer* pThresholdLayer = dynamic_cast<ThresholdLayer*>(pLayer);
         if (pThresholdLayer != NULL)
         {
            VERIFYNR(pThresholdLayer->attach(SIGNAL_NAME(Subject, Deleted),
               Slot(this, &HistogramWindowImp::layerDeleted)));
         }

         // Store the statistics widget visibility state so that it can be set properly when the widget is created
         if (pPlotWidget != NULL)
         {
            mShowStatistics[pPlotWidget] =
               StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("statisticsShown"))));
         }
      }
   }

   return true;
}

PlotWidget* HistogramWindowImp::createPlot(Layer* pLayer)
{
   return createPlot(pLayer, NULL);
}

PlotWidget* HistogramWindowImp::createPlot(Layer* pLayer, PlotSet* pPlotSet)
{
   if (pLayer == NULL)
   {
      return NULL;
   }

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
   if (pRasterLayer != NULL)
   {
      DisplayMode displayMode = pRasterLayer->getDisplayMode();
      if (displayMode == RGB_MODE)
      {
         createPlots(pRasterLayer, displayMode, pPlotSet);
         return getPlot(pRasterLayer, RED);
      }

      return createPlot(pRasterLayer, GRAY, pPlotSet);
   }

   if (pLayer->isKindOf("ThresholdLayer") == false)
   {
      return NULL;
   }

   // Do not add the plot if it already exists
   if (getPlot(pLayer) != NULL)
   {
      return NULL;
   }

   // Get or create a plot set if necessary
   if (pPlotSet == NULL)
   {
      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         string viewName = pView->getName();
         if (viewName.empty() == false)
         {
            pPlotSet = mpPlotSetGroup->getPlotSet(viewName);
            if (pPlotSet == NULL)
            {
               pPlotSet = mpPlotSetGroup->createPlotSet(viewName);
            }
         }
      }
   }

   if (pPlotSet == NULL)
   {
      return NULL;
   }

   // Add the plot to the plot set
   PlotWidget* pPlot = pPlotSet->createPlot(pLayer->getName(), HISTOGRAM_PLOT);
   if (pPlot != NULL)
   {
      // Attach to the Layer::Deleted signal before calling setHistogram() so that the
      // layerDeleted() slot is called before the histogram plot pointer is NULLed
      VERIFYNR(pLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &HistogramWindowImp::layerDeleted)));

      // Set the histogram data in the plot
      HistogramPlotImp* pHistogramPlot = dynamic_cast<HistogramPlotImp*>(pPlot->getPlot());
      if (pHistogramPlot != NULL)
      {
         pHistogramPlot->setHistogram(pLayer);
      }

      // Activate the plot
      mpPlotSetGroup->setCurrentPlot(pPlot);
   }

   return pPlot;
}

PlotWidget* HistogramWindowImp::createPlot(RasterLayer* pLayer, RasterChannelType channel)
{
   return createPlot(pLayer, channel, NULL);
}

PlotWidget* HistogramWindowImp::createPlot(RasterLayer* pLayer, RasterChannelType channel, PlotSet* pPlotSet)
{
   if (pLayer == NULL)
   {
      return NULL;
   }

   // Do not add the plot if it already exists
   if (getPlot(pLayer, channel, true) != NULL)
   {
      return NULL;
   }

   // Get or create a plot set if necessary
   if (pPlotSet == NULL)
   {
      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         string viewName = pView->getName();
         if (viewName.empty() == false)
         {
            pPlotSet = mpPlotSetGroup->getPlotSet(viewName);
            if (pPlotSet == NULL)
            {
               pPlotSet = mpPlotSetGroup->createPlotSet(viewName);
            }
         }
      }
   }

   if (pPlotSet == NULL)
   {
      return NULL;
   }

   // Add the plot to the plot set
   PlotWidget* pPlot = pPlotSet->createPlot(pLayer->getName(), HISTOGRAM_PLOT);
   if (pPlot != NULL)
   {
      // Attach to the Layer::Deleted signal before calling setHistogram() so that the
      // layerDeleted() slot is called before the histogram plot pointer is NULLed
      pLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &HistogramWindowImp::layerDeleted));

      // Set the histogram data in the plot
      HistogramPlotImp* pHistogramPlot = dynamic_cast<HistogramPlotImp*>(pPlot->getPlot());
      if (pHistogramPlot != NULL)
      {
         pHistogramPlot->setHistogram(pLayer, channel);
      }

      // Update the displayed plot information
      updatePlotInfo(pLayer, channel);

      // Activate the plot
      mpPlotSetGroup->setCurrentPlot(pPlot);

      // Connections
      VERIFYNR(pPlot->attach(SIGNAL_NAME(PlotWidget, AboutToShowContextMenu),
         Slot(this, &HistogramWindowImp::updateContextMenu)));

      RasterLayerImp* pLayerImp = dynamic_cast<RasterLayerImp*>(pLayer);
      if (pLayerImp != NULL)
      {
         VERIFYNR(connect(pLayerImp, SIGNAL(displayModeChanged(const DisplayMode&)), this,
            SLOT(setCurrentPlot(const DisplayMode&))));
         VERIFYNR(connect(pLayerImp, SIGNAL(displayedBandChanged(RasterChannelType, DimensionDescriptor)), this,
            SLOT(updatePlotInfo(RasterChannelType))));
      }
   }

   return pPlot;
}

void HistogramWindowImp::createPlots(RasterLayer* pLayer, DisplayMode displayMode)
{
   createPlots(pLayer, displayMode, NULL);
}

void HistogramWindowImp::createPlots(RasterLayer* pLayer, DisplayMode displayMode, PlotSet* pPlotSet)
{
   if (pLayer == NULL)
   {
      return;
   }

   // Get or create a plot set if necessary
   if (pPlotSet == NULL)
   {
      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         string viewName = pView->getName();
         if (viewName.empty() == false)
         {
            pPlotSet = mpPlotSetGroup->getPlotSet(viewName);
            if (pPlotSet == NULL)
            {
               pPlotSet = mpPlotSetGroup->createPlotSet(viewName);
            }
         }
      }
   }

   if (pPlotSet == NULL)
   {
      return;
   }

   if (displayMode == RGB_MODE)
   {
      VERIFYNR(disconnect(dynamic_cast<PlotSetImp*>(pPlotSet), SIGNAL(plotActivated(PlotWidget*)), this,
         SLOT(activateLayer(PlotWidget*))));

      createPlot(pLayer, RED, pPlotSet);
      createPlot(pLayer, GREEN, pPlotSet);
      createPlot(pLayer, BLUE, pPlotSet);

      VERIFYNR(connect(dynamic_cast<PlotSetImp*>(pPlotSet), SIGNAL(plotActivated(PlotWidget*)), this,
         SLOT(activateLayer(PlotWidget*))));

      setCurrentPlot(pLayer, RED);
   }
   else
   {
      createPlot(pLayer, GRAY, pPlotSet);
   }
}

void HistogramWindowImp::setCurrentPlot(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return;
   }

   PlotWidget* pPlot = getPlot(pLayer);
   if (pPlot == NULL)
   {
      return;
   }

   if (pPlot == mpPlotSetGroup->getCurrentPlot())
   {
      return;
   }

   PlotSet* pPlotSet = pPlot->getPlotSet();
   if (pPlotSet != NULL)
   {
      pPlotSet->setCurrentPlot(pPlot);
   }
}

bool HistogramWindowImp::setCurrentPlot(Layer* pLayer, const RasterChannelType& eColor)
{
   if (pLayer == NULL)
   {
      return false;
   }

   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
   if (pRasterLayer != NULL)
   {
      return setCurrentPlot(pRasterLayer, eColor);
   }

   PlotWidget* pPlot = getPlot(pLayer);
   if (pPlot == NULL)
   {
      return false;
   }

   if (pPlot == mpPlotSetGroup->getCurrentPlot())
   {
      return true;
   }

   bool bSuccess = false;

   PlotSet* pPlotSet = pPlot->getPlotSet();
   if (pPlotSet != NULL)
   {
      bSuccess = pPlotSet->setCurrentPlot(pPlot);
   }

   return bSuccess;
}

bool HistogramWindowImp::setCurrentPlot(RasterLayer* pLayer, RasterChannelType channel)
{
   if (pLayer == NULL)
   {
      return false;
   }

   PlotWidget* pPlotWidget = getPlot(pLayer, channel);
   if (pPlotWidget != NULL)
   {
      if (pPlotWidget == mpPlotSetGroup->getCurrentPlot())
      {
         return true;
      }

      PlotSet* pPlotSet = pPlotWidget->getPlotSet();
      if (pPlotSet != NULL)
      {
         return pPlotSet->setCurrentPlot(pPlotWidget);
      }
   }

   return false;
}

void HistogramWindowImp::deletePlot(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return;
   }

   deleteStatisticsPlots(pLayer);
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
   if (pRasterLayer != NULL)
   {
      deletePlots(pRasterLayer, pRasterLayer->getDisplayMode());
      return;
   }

   PlotWidget* pPlotWidget = getPlot(pLayer);
   if (pPlotWidget != NULL)
   {
      PlotSet* pPlotSet = pPlotWidget->getPlotSet();
      if (pPlotSet != NULL)
      {
         VERIFYNR(pLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &HistogramWindowImp::layerDeleted)));

         pPlotSet->deletePlot(pPlotWidget);
         if (pPlotSet->getNumPlots() == 0)
         {
            mpPlotSetGroup->deletePlotSet(pPlotSet);
         }
      }
   }
}

void HistogramWindowImp::deletePlot(RasterLayer* pLayer, RasterChannelType channel)
{
   if (pLayer == NULL)
   {
      return;
   }

   PlotWidget* pPlotWidget = getPlot(pLayer, channel, true);
   if (pPlotWidget != NULL)
   {
      PlotSet* pPlotSet = pPlotWidget->getPlotSet();
      if (pPlotSet != NULL)
      {
         pPlotWidget->detach(SIGNAL_NAME(PlotWidget, AboutToShowContextMenu),
            Slot(this, &HistogramWindowImp::updateContextMenu));
         pLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &HistogramWindowImp::layerDeleted));

         RasterLayerImp* pRasterLayerImp = dynamic_cast<RasterLayerImp*>(pLayer);
         if (pRasterLayerImp != NULL)
         {
            disconnect(pRasterLayerImp, SIGNAL(displayModeChanged(const DisplayMode&)), this,
               SLOT(setCurrentPlot(const DisplayMode&)));
            disconnect(pRasterLayerImp, SIGNAL(displayedBandChanged(RasterChannelType, DimensionDescriptor)),
               this, SLOT(updatePlotInfo(RasterChannelType)));
         }

         pPlotSet->deletePlot(pPlotWidget);
         if (pPlotSet->getNumPlots() == 0)
         {
            mpPlotSetGroup->deletePlotSet(pPlotSet);
         }
      }
   }
}

void HistogramWindowImp::deletePlots(RasterLayer* pLayer, DisplayMode displayMode)
{
   if (pLayer == NULL)
   {
      return;
   }

   if (displayMode == RGB_MODE)
   {
      PlotSetImp* pPlotSet = NULL;

      PlotWidget* pPlotWidget = getPlot(pLayer);
      if (pPlotWidget != NULL)
      {
         pPlotSet = dynamic_cast<PlotSetImp*>(pPlotWidget->getPlotSet());
      }

      if (pPlotSet != NULL)
      {
         VERIFYNR(disconnect(pPlotSet, SIGNAL(plotActivated(PlotWidget*)), this, SLOT(activateLayer(PlotWidget*))));
      }

      deletePlot(pLayer, RED);
      deletePlot(pLayer, GREEN);

      if (pPlotSet != NULL)
      {
         VERIFYNR(connect(pPlotSet, SIGNAL(plotActivated(PlotWidget*)), this, SLOT(activateLayer(PlotWidget*))));
      }

      deletePlot(pLayer, BLUE);
   }
   else
   {
      deletePlot(pLayer, GRAY);
   }
}

void HistogramWindowImp::deleteStatisticsPlots(Layer* pLayer)
{
   std::vector<PlotWidget*> plots = mpPlotSetGroup->getPlots();
   for (std::vector<PlotWidget*>::iterator plot = plots.begin(); plot != plots.end(); ++plot)
   {
      HistogramPlotImp* pHistPlot = dynamic_cast<HistogramPlotImp*>((*plot)->getPlot());
      if (pHistPlot->getLayer() == pLayer && pHistPlot->ownsStatistics())
      {
         PlotSet* pPlotSet = (*plot)->getPlotSet();
         if (pPlotSet != NULL)
         {
            pPlotSet->deletePlot(*plot);
            if (pPlotSet->getNumPlots() == 0)
            {
               mpPlotSetGroup->deletePlotSet(pPlotSet);
            }
         }
      }
   }
}

bool HistogramWindowImp::event(QEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (pEvent->type() == QEvent::Polish)
      {
         // Attach to the dock window
         attach(SIGNAL_NAME(DockWindow, AboutToShowContextMenu), Slot(this, &HistogramWindowImp::updateContextMenu));
      }
   }

   return DockWindowImp::event(pEvent);
}

void HistogramWindowImp::setCurrentPlot(const DisplayMode& displayMode)
{
   if (mAddingStatisticsPlot)
   {
      return;
   }
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(sender());
   if (pLayer != NULL)
   {
      mDisplayModeChanging = true;
      if (pLayer->getDisplayMode() == RGB_MODE)
      {
         deletePlots(pLayer, GRAYSCALE_MODE);
         createPlots(pLayer, RGB_MODE);
      }
      else
      {
         deletePlots(pLayer, RGB_MODE);
         createPlots(pLayer, GRAYSCALE_MODE);
      }

      mDisplayModeChanging = false;

      if (pLayer->getDisplayMode() == RGB_MODE)
      {
         emit plotActivated(pLayer, RED);
      }
      else
      {
         emit plotActivated(pLayer, GRAY);
      }
   }
}

void HistogramWindowImp::createStatisticsWidget(PlotWidget* pPlotWidget)
{
   PlotWidgetImp* pPlotWidgetImp = dynamic_cast<PlotWidgetImp*>(pPlotWidget);
   if (pPlotWidgetImp == NULL)
   {
      return;
   }

   QSplitter* pSplitter = pPlotWidgetImp->getSplitter();
   VERIFYNRV(pSplitter != NULL);

   if (pSplitter->widget(2) != NULL)
   {
      return;
   }

   // Create the statistics widget
   HistogramPlotImp* pHistPlot = dynamic_cast<HistogramPlotImp*>(pPlotWidgetImp->getPlot());
   StatisticsWidget* pStatisticsWidget = new StatisticsWidget(pHistPlot, this);

   // Initialization
   bool showStats = mpStatisticsShowAction->isChecked();

   map<PlotWidget*, bool>::const_iterator iter = mShowStatistics.find(pPlotWidget);
   if (iter != mShowStatistics.end())
   {
      showStats = iter->second;
      mShowStatistics.erase(iter);
   }

   pStatisticsWidget->setVisible(showStats);

   // Add the statistics widget to the plot widget after setting the visibility to ensure proper initialization
   pSplitter->insertWidget(2, pStatisticsWidget);

   // Connections
   VERIFYNR(connect(pHistPlot, SIGNAL(histogramUpdated()), pStatisticsWidget, SLOT(updateStatistics())));
}

void HistogramWindowImp::activateLayer(PlotWidget* pPlot)
{
   if (mAddingStatisticsPlot || pPlot == NULL)
   {
      return;
   }
   HistogramPlotImp* pHistogramPlot = dynamic_cast<HistogramPlotImp*>(pPlot->getPlot());
   if ((pHistogramPlot == NULL) || (mDisplayModeChanging))
   {
      return;
   }

   Layer* pLayer = pHistogramPlot->getLayer();
   RasterChannelType channel = pHistogramPlot->getRasterChannelType();

   if (pLayer != NULL)
   {
      bool activate = HistogramWindow::getSettingLayerActivation();
      bool isPrimary = false;
      if (activate)
      {
         // Don't activate the primary raster element
         RasterElement* pElement = dynamic_cast<RasterElement*>(pLayer->getDataElement());
         SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
         if (pView != NULL && pElement != NULL)
         {
            LayerList* pLayerList = pView->getLayerList();
            if (pLayerList != NULL)
            {
               /**
                This prevents the "primary raster layer" from moving to the top of the stack
                when activating a histogram. The "primary raster layer" is a raster layer
                containing the primary raster element.
                **/
               if (pLayerList->getPrimaryRasterElement() == pElement &&
                   pLayer->getLayerType() == RASTER &&
                   pView->getLayerDisplayIndex(pLayer) == static_cast<int>(pLayerList->getNumLayers()) - 1)
               {
                  isPrimary = true;
               }
            }
         }
      }

      if (activate)
      {
         WorkspaceWindow* pWindow = mpDesktop->getCurrentWorkspaceWindow();
         if (pWindow != NULL)
         {
            SpatialDataView* pView = NULL;
            SpatialDataWindow* pSpatialWindow = dynamic_cast<SpatialDataWindow*>(pWindow);
            ProductWindow* pProductWindow = dynamic_cast<ProductWindow*>(pWindow);
            if (pSpatialWindow != NULL)
            {
               pView = pSpatialWindow->getSpatialDataView();
            }
            else if (pProductWindow != NULL)
            {
               ProductView* pProductView = pProductWindow->getProductView();
               if (pProductView != NULL)
               {
                  SpatialDataView* pEditView = dynamic_cast<SpatialDataView*>(pProductView->getActiveEditView());
                  if (pEditView != NULL)
                  {
                     pView = pEditView;
                  }
               }
            }

            if (pView != NULL)
            {
               if (!isPrimary)
               {
                  pView->setFrontLayer(pLayer);
               }
               // ensure the layer is visible
               pView->showLayer(pLayer);
            }
         }
      }
   }

   setStatisticsShowActionState(dynamic_cast<PlotWidgetImp*>(pPlot));

   if (!pHistogramPlot->ownsStatistics())
   {
      emit plotActivated(pLayer, channel);
   }
}

void HistogramWindowImp::setStatisticsShowActionState(PlotSet* pPlotSet)
{
   if (pPlotSet != NULL)
   {
      setStatisticsShowActionState(dynamic_cast<PlotWidgetImp*>(pPlotSet->getCurrentPlot()));
   }
}

void HistogramWindowImp::setStatisticsShowActionState(PlotWidgetImp* pPlot)
{
   QWidget* pWidget = pPlot == NULL ? NULL : pPlot->getSplitter()->widget(2);
   if (pWidget != NULL)
   {
      mpStatisticsShowAction->setChecked(!pWidget->isHidden());
   }
}

void HistogramWindowImp::showEvent(QShowEvent* pEvent)
{
   DockWindowImp::showEvent(pEvent);
   mUpdater.update();
}

void HistogramWindowImp::windowAdded(Subject& subject, const string& signal, const boost::any& value)
{
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(boost::any_cast<Window*>(value));
   if (pWindow != NULL)
   {
      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      if (pView != NULL)
      {
         VERIFYNR(connect(pView, SIGNAL(layerAdded(Layer*)), this, SLOT(createPlot(Layer*))));
         VERIFYNR(connect(pView, SIGNAL(layerActivated(Layer*)), this, SLOT(setCurrentPlot(Layer*))));
         VERIFYNR(pView->attach(SIGNAL_NAME(SpatialDataView, LayerShown), Slot(this, &HistogramWindowImp::layerShown)));
      }
   }
}

void HistogramWindowImp::windowRemoved(Subject& subject, const string& signal, const boost::any& value)
{
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(boost::any_cast<Window*>(value));
   if (pWindow != NULL)
   {
      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      if (pView != NULL)
      {
         VERIFYNR(disconnect(pView, SIGNAL(layerAdded(Layer*)), this, SLOT(createPlot(Layer*))));
         VERIFYNR(disconnect(pView, SIGNAL(layerActivated(Layer*)), this, SLOT(setCurrentPlot(Layer*))));
         VERIFYNR(pView->detach(SIGNAL_NAME(SpatialDataView, LayerShown), Slot(this, &HistogramWindowImp::layerShown)));
      }
   }
}

void HistogramWindowImp::plotSetAdded(Subject& subject, const string& signal, const boost::any& value)
{
   PlotSetImp* pPlotSet = dynamic_cast<PlotSetImp*>(boost::any_cast<PlotSet*>(value));
   if (pPlotSet != NULL)
   {
      VERIFYNR(connect(pPlotSet, SIGNAL(plotAdded(PlotWidget*)), this, SLOT(createStatisticsWidget(PlotWidget*))));
      VERIFYNR(connect(pPlotSet, SIGNAL(plotActivated(PlotWidget*)), this, SLOT(activateLayer(PlotWidget*))));
   }
}

void HistogramWindowImp::plotSetActivated(Subject& subject, const string& signal, const boost::any& value)
{
   PlotSet* pPlotSet = boost::any_cast<PlotSet*>(value);
   if (pPlotSet != NULL)
   {
      setStatisticsShowActionState(pPlotSet);
   }
}

void HistogramWindowImp::plotSetDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   PlotSetImp* pPlotSet = dynamic_cast<PlotSetImp*>(boost::any_cast<PlotSet*>(value));
   if (pPlotSet != NULL)
   {
      VERIFYNR(disconnect(pPlotSet, SIGNAL(plotAdded(PlotWidget*)), this, SLOT(createStatisticsWidget(PlotWidget*))));
      VERIFYNR(disconnect(pPlotSet, SIGNAL(plotActivated(PlotWidget*)), this, SLOT(activateLayer(PlotWidget*))));
   }
}

void HistogramWindowImp::layerShown(Subject& subject, const string& signal, const boost::any& value)
{
   if (isHidden() || !HistogramWindow::getSettingLayerActivation())
   {
      return;
   }

   ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(boost::any_cast<Layer*>(value));
   if (pLayer == NULL)
   {
      return;
   }

   PlotWidget* pPlot = getPlot(pLayer);
   if (pPlot != NULL)
   {
      mpPlotSetGroup->setCurrentPlot(pPlot);
   }
}

void HistogramWindowImp::layerDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   Layer* pLayer = dynamic_cast<Layer*>(&subject);
   if (pLayer != NULL)
   {
      deletePlot(pLayer);
   }
}

void HistogramWindowImp::sessionRestored(Subject& subject, const string& signal, const boost::any& value)
{
   vector<PlotWidget*> plots = mpPlotSetGroup->getPlots(HISTOGRAM_PLOT);
   for (vector<PlotWidget*>::const_iterator iter = plots.begin(); iter != plots.end(); ++iter)
   {
      PlotWidgetImp* pPlotWidget = dynamic_cast<PlotWidgetImp*>(*iter);
      if (pPlotWidget != NULL)
      {
         QSplitter* pSplitter = pPlotWidget->getSplitter();
         VERIFYNRV(pSplitter != NULL);

         StatisticsWidget* pStatsWidget = dynamic_cast<StatisticsWidget*>(pSplitter->widget(2));
         if (pStatsWidget != NULL)
         {
            pStatsWidget->updateStatistics();
         }
      }
   }
}

void HistogramWindowImp::updatePlotInfo(RasterChannelType channel)
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(sender());
   if (pLayer != NULL)
   {
      if (isVisible())
      {
         updatePlotInfo(pLayer, channel);
      }
      else
      {
         mUpdater.initialize(pLayer, channel);
      }
   }
}

void HistogramWindowImp::updatePlotInfo(RasterLayer* pLayer, RasterChannelType channel)
{
   if (pLayer == NULL)
   {
      return;
   }

   PlotWidgetImp* pPlotWidget = dynamic_cast<PlotWidgetImp*>(getPlot(pLayer, channel, true));
   if (pPlotWidget == NULL)
   {
      return;
   }

   // Get the layer name
   string layerName = pLayer->getDisplayName();
   if (layerName.empty() == true)
   {
      layerName = pLayer->getName();
   }

   QString strLayer = QString::fromStdString(layerName);
   VERIFYNRV(strLayer.isEmpty() == false);

   // Get the display channel
   QString strChannel = QString::fromStdString(StringUtilities::toDisplayString(channel));
   VERIFYNRV(strChannel.isEmpty() == false);

   // Get the displayed element and display band names
   QString strElement = "No Element Displayed";
   QString strBand = "No Band Displayed";

   RasterElement* pElement = pLayer->getDisplayedRasterElement(channel);
   if (pElement != NULL)
   {
      string elementName = pElement->getDisplayName();
      if (elementName.empty() == true)
      {
         elementName = pElement->getName();
      }

      strElement = QString::fromStdString(elementName);

      DimensionDescriptor band = pLayer->getDisplayedBand(channel);
      if (band.isActiveNumberValid() == true)
      {
         RasterDataDescriptor* pDescriptor =
            dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            strBand = QString::fromStdString(RasterUtilities::getBandName(pDescriptor, band));
         }
      }
   }

   VERIFYNRV(strElement.isEmpty() == false);
   VERIFYNRV(strBand.isEmpty() == false);

   // Display the element and band names in the plot title
   pPlotWidget->setTitle(strElement + " - " + strBand);
   pPlotWidget->setTitleElideMode(Qt::ElideLeft);

   // Add a tool tip for the plot tab in the window
   PlotSetImp* pPlotSet = dynamic_cast<PlotSetImp*>(pPlotWidget->getPlotSet());
   if (pPlotSet != NULL)
   {
      int index = pPlotSet->indexOf(pPlotWidget);
      if (index != -1)
      {
         QString strTip = "<qt><table cellspacing=0><tr><td width=115><b>Layer:</b></td><td>" + strLayer +
            "</td></tr><tr><td width=115><b>Display Channel:</b></td><td>" + strChannel +
            "</td></tr><tr><td width=115><b>Displayed Element:</b></td><td>" + strElement +
            "</td></tr><tr><td width=115><b>Displayed Band:</b></td><td>" + strBand + "</td></tr></table></qt>";

         pPlotSet->setTabToolTip(index, strTip);
      }
   }
}

void HistogramWindowImp::syncAutoZoom()
{
   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction == NULL)
   {
      return;
   }

   SessionItem* pItem = pAction->data().value<SessionItem*>();
   HistogramPlotImp* pPlot = dynamic_cast<HistogramPlotImp*>(pItem);
   VERIFYNRV(pPlot != NULL);

   Layer* pLayer = pPlot->getLayer();
   VERIFYNRV(pLayer != NULL);

   RasterChannelType channel = pPlot->getRasterChannelType();
   VERIFYNRV(channel != GRAY);

   bool autoZoom = pPlot->isAutoZoomEnabled();
   if (channel != RED)
   {
      PlotWidget* pPlotWidget = getPlot(pLayer, RED);
      if (pPlotWidget != NULL)
      {
         HistogramPlot* pRedPlot = dynamic_cast<HistogramPlot*>(pPlotWidget->getPlot());
         if (pRedPlot != NULL)
         {
            pRedPlot->enableAutoZoom(autoZoom);
         }
      }
   }

   if (channel != GREEN)
   {
      PlotWidget* pPlotWidget = getPlot(pLayer, GREEN);
      if (pPlotWidget != NULL)
      {
         HistogramPlot* pGreenPlot = dynamic_cast<HistogramPlot*>(pPlotWidget->getPlot());
         if (pGreenPlot != NULL)
         {
            pGreenPlot->enableAutoZoom(autoZoom);
         }
      }
   }

   if (channel != BLUE)
   {
      PlotWidget* pPlotWidget = getPlot(pLayer, BLUE);
      if (pPlotWidget != NULL)
      {
         HistogramPlot* pBluePlot = dynamic_cast<HistogramPlot*>(pPlotWidget->getPlot());
         if (pBluePlot != NULL)
         {
            pBluePlot->enableAutoZoom(autoZoom);
         }
      }
   }
}

void HistogramWindowImp::setStatisticsShown(bool shown)
{
   PlotWidgetImp* pPlotImp = dynamic_cast<PlotWidgetImp*>(mpPlotSetGroup->getCurrentPlot());
   if (pPlotImp != NULL)
   {
      QWidget* pWidget = pPlotImp->getSplitter()->widget(2);
      if (pWidget != NULL)
      {
         pWidget->setVisible(shown);
      }
   }
}

void HistogramWindowImp::createSubsetPlot(Layer* pLayer)
{
   RasterElement* pElement = pLayer == NULL ? NULL : dynamic_cast<RasterElement*>(pLayer->getDataElement());
   SpatialDataView* pView = pLayer == NULL ? NULL : dynamic_cast<SpatialDataView*>(pLayer->getView());
   if (pView == NULL || pElement == NULL)
   {
      return;
   }
   const RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
   std::vector<Layer*> aoiLayers;
   pView->getLayerList()->getLayers(AOI_LAYER, aoiLayers);
   std::map<std::string, AoiElement*> aoiElements;
   QStringList aoiNames;
   for (std::vector<Layer*>::const_iterator layer = aoiLayers.begin(); layer != aoiLayers.end(); ++layer)
   {
      AoiLayer* pLayer = static_cast<AoiLayer*>(*layer);
      aoiNames << QString::fromStdString(pLayer->getName());
      aoiElements[pLayer->getName()] = static_cast<AoiElement*>(pLayer->getDataElement());
   }
   std::vector<std::string> bandNamesStl = RasterUtilities::getBandNames(pDesc);
   QStringList bandNames;
   for (std::vector<std::string>::const_iterator bandName = bandNamesStl.begin();
         bandName != bandNamesStl.end(); ++bandName)
   {
      bandNames << QString::fromStdString(*bandName);
   }
   std::vector<int> defaultSelection;
   const RasterLayer* pRasterLayer = dynamic_cast<const RasterLayer*>(pLayer);
   if (pRasterLayer != NULL && pRasterLayer->getDisplayMode() == GRAYSCALE_MODE)
   {
      defaultSelection.push_back(pRasterLayer->getDisplayedBand(GRAY).getActiveNumber());
   }
   else if (pRasterLayer != NULL)
   {
      defaultSelection.push_back(pRasterLayer->getDisplayedBand(RED).getActiveNumber());
      defaultSelection.push_back(pRasterLayer->getDisplayedBand(GREEN).getActiveNumber());
      defaultSelection.push_back(pRasterLayer->getDisplayedBand(BLUE).getActiveNumber());
   }
   else
   {
      defaultSelection.push_back(0);
   }
   PlotSet* pPlotSet = mpPlotSetGroup->getCurrentPlotSet();
   VERIFYNRV(pPlotSet);
   std::string baseName = pLayer->getDisplayName(true) + " - Statistics ";
   for (int suffix = 1; suffix < 200; ++suffix) // if we reach 200 of these, we have bigger issues,
                                                // this is an arbitrary max
   {
      std::string name = baseName + StringUtilities::toDisplayString(suffix);
      if (pPlotSet->getPlot(name) == NULL)
      {
         baseName = name;
         break;
      }
   }
   GetSubsetDialog dlg(QString::fromStdString(baseName), aoiNames, bandNames, defaultSelection, this);
   if (dlg.exec() == QDialog::Accepted)
   {
      QString aoi = dlg.getSelectedAoi();
      std::vector<int> bandIndices = dlg.getBandSelectionIndices();
      std::vector<DimensionDescriptor> bands;
      bands.reserve(bandIndices.size());
      for (std::vector<int>::const_iterator index = bandIndices.begin(); index != bandIndices.end(); ++index)
      {
         bands.push_back(pDesc->getActiveBand(*index));
      }
      show();
      mAddingStatisticsPlot = true;
      PlotWidget* pPlot = pPlotSet->createPlot(dlg.getPlotName().toStdString(), HISTOGRAM_PLOT);
      mAddingStatisticsPlot = false;
      if (pPlot == NULL)
      {
         Service<DesktopServices>()->showMessageBox("Can't create plot",
            "Can't create statistics plot with the specified name. It may already exist, try a different name.",
            "Ok");
         return;
      }
      // create a title which shows at least some of the bands
      std::string title = dlg.getPlotName().toStdString() + " (";
      std::string bandNames;
      for (std::vector<DimensionDescriptor>::const_iterator band = bands.begin(); band != bands.end(); ++band)
      {
         if (!bandNames.empty())
         {
            bandNames += ",";
         }
         bandNames += RasterUtilities::getBandName(pDesc, *band);
      }
      title += bandNames + ")";
      pPlot->setTitle(title);
      PlotWidgetImp* pPlotImp = dynamic_cast<PlotWidgetImp*>(pPlot);
      VERIFYNRV(pPlotImp);
      pPlotImp->setTitleElideMode(Qt::ElideRight);

      StatisticsImp* pStatistics = new StatisticsImp(dynamic_cast<RasterElementImp*>(pElement),
         bands, aoiElements[aoi.toStdString()]);
      HistogramPlotImp* pHist = dynamic_cast<HistogramPlotImp*>(pPlot->getPlot());
      pHist->setHistogram(pLayer, pStatistics);

      // Connections
      VERIFYNR(pPlot->attach(SIGNAL_NAME(PlotWidget, AboutToShowContextMenu),
         Slot(this, &HistogramWindowImp::updateContextMenu)));
   }
}

HistogramWindowImp::HistogramUpdater::HistogramUpdater(HistogramWindowImp* pWindow) :
   mpWindow(pWindow)
{
}

void HistogramWindowImp::HistogramUpdater::initialize(RasterLayer* pLayer, RasterChannelType channel)
{
   mUpdatesPending.insert(UpdateMomento(mpWindow, pLayer, channel));
}

void HistogramWindowImp::HistogramUpdater::update()
{
   for_each(mUpdatesPending.begin(), mUpdatesPending.end(), mem_fun_ref(&UpdateMomento::update));
   mUpdatesPending.clear();
}

HistogramWindowImp::HistogramUpdater::UpdateMomento::UpdateMomento(HistogramWindowImp* pWindow,
                                                                   RasterLayer* pLayer,
                                                                   RasterChannelType channel) :
   mpWindow(pWindow),
   mpRasterLayer(new SafePtr<RasterLayer>(pLayer)),
   mChannel(channel)
{
}

void HistogramWindowImp::HistogramUpdater::UpdateMomento::update() const
{
   RasterLayer* pLayer = mpRasterLayer.get() == NULL ? NULL : mpRasterLayer.get()->get();
   if (pLayer == NULL || mpWindow == NULL)
   {
      return;
   }

   mpWindow->updatePlotInfo(pLayer, mChannel);
}

bool HistogramWindowImp::HistogramUpdater::UpdateMomento::operator<(const UpdateMomento& rhs) const
{
   RasterLayer* pLeftLayer = mpRasterLayer.get() == NULL ? NULL : mpRasterLayer.get()->get();
   RasterLayer* pRightLayer = rhs.mpRasterLayer.get() == NULL ? NULL : rhs.mpRasterLayer.get()->get();
   if (pLeftLayer == pRightLayer)
   {
      return this->mChannel < rhs.mChannel;
   }

   return pLeftLayer < pRightLayer;
}
