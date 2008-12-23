/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>

#include "AppAssert.h"
#include "AppVerify.h"
#include "ContextMenu.h"
#include "ContextMenuActions.h"
#include "DesktopServices.h"
#include "HistogramPlotAdapter.h"
#include "HistogramWindow.h"
#include "HistogramWindowImp.h"
#include "Icons.h"
#include "InfoBar.h"
#include "Layer.h"
#include "LayerListAdapter.h"
#include "PlotSetAdapter.h"
#include "PlotWidget.h"
#include "PlotWidgetImp.h"
#include "ProductView.h"
#include "ProductWindow.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayerAdapter.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "WorkspaceWindow.h"

#include <vector>
using namespace std;

HistogramWindowImp::HistogramWindowImp(const string& id, const string& windowName, QWidget* pParent) :
   PlotWindowImp(id, windowName, pParent),
   mpExplorer(Service<SessionExplorer>().get(), SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu),
      Slot(this, &HistogramWindowImp::updateContextMenu)),
   mDisplayModeChanging(false),
   mpSyncAutoZoomAction(NULL),
   mUpdater(this)
{
   Service<DesktopServices> pDesktop;

   mpSyncAutoZoomAction = new QAction("Synchronize Auto Zoom", this);
   mpSyncAutoZoomAction->setAutoRepeat(false);
   VERIFYNR(connect(mpSyncAutoZoomAction, SIGNAL(triggered()), this, SLOT(syncAutoZoom())));
   pDesktop->initializeAction(mpSyncAutoZoomAction, "Histogram Plot");     // Use the plot context since the action
                                                                           // will appear on the plot menu

   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);

   InfoBar* pInfoBar = getInfoBar();
   if (pInfoBar != NULL)
   {
      pInfoBar->setInfoIcon(pIcons->mHistogram);
   }

   setIcon(pIcons->mHistogram);
}

HistogramWindowImp::~HistogramWindowImp()
{
}

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
            if (containsPlot(pPlot) == true)
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
      vector<PlotSet*> plotSetItems = pMenu->getSessionItems<PlotSet>();
      if (plotSetItems.empty() == false)
      {
         PlotSet* pPlotSet = plotSetItems.front();
         if (pPlotSet != NULL)
         {
            bRemoveActions = containsPlotSet(pPlotSet);
         }
      }
   }
   else
   {
      PlotWidget* pPlotWidget = dynamic_cast<PlotWidget*>(&subject);
      if ((pPlotWidget != NULL) && (containsPlot(pPlotWidget) == true))
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
}

PlotSet* HistogramWindowImp::createPlotSet(const QString& strPlotSet)
{
   if (strPlotSet.isEmpty() == true)
   {
      return NULL;
   }

   PlotSet* pPlotSet = PlotWindowImp::createPlotSet(strPlotSet);
   if (pPlotSet != NULL)
   {
      VERIFYNR(connect(dynamic_cast<PlotSetImp*>(pPlotSet), SIGNAL(plotActivated(PlotWidget*)), this,
         SLOT(activateLayer(PlotWidget*))));
   }

   return pPlotSet;
}

bool HistogramWindowImp::deletePlotSet(PlotSet* pPlotSet)
{
   if (pPlotSet != NULL)
   {
      if (containsPlotSet(pPlotSet) == true)
      {
         VERIFYNR(disconnect(dynamic_cast<PlotSetImp*>(pPlotSet), SIGNAL(plotActivated(PlotWidget*)), this,
            SLOT(activateLayer(PlotWidget*))));
      }
   }

   return PlotWindowImp::deletePlotSet(pPlotSet);
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
   vector<PlotWidget*> plots = getPlots(HISTOGRAM_PLOT);
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
   if (pLayer == NULL)
   {
      return NULL;
   }

   // Iterate over all histogram plots on all plot sets to find the plot
   vector<PlotWidget*> plots = getPlots(HISTOGRAM_PLOT);
   for (vector<PlotWidget*>::iterator iter = plots.begin(); iter != plots.end(); ++iter)
   {
      PlotWidget* pPlot = *iter;
      if (pPlot != NULL)
      {
         HistogramPlotImp* pHistogramPlot = dynamic_cast<HistogramPlotImp*>(pPlot->getPlot());
         if (pHistogramPlot != NULL)
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
            QString strViewName = QString::fromStdString(viewName);

            pPlotSet = getPlotSet(strViewName);
            if (pPlotSet == NULL)
            {
               pPlotSet = createPlotSet(strViewName);
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
      // Set the histogram data in the plot
      HistogramPlotImp* pHistogramPlot = dynamic_cast<HistogramPlotImp*>(pPlot->getPlot());
      if (pHistogramPlot != NULL)
      {
         pHistogramPlot->setHistogram(pLayer);
      }

      // Activate the plot
      PlotWindowImp::setCurrentPlot(pPlot);
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
   if (getPlot(pLayer, channel) != NULL)
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
            QString strViewName = QString::fromStdString(viewName);

            pPlotSet = getPlotSet(strViewName);
            if (pPlotSet == NULL)
            {
               pPlotSet = createPlotSet(strViewName);
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
      // Set the histogram data in the plot
      HistogramPlotImp* pHistogramPlot = dynamic_cast<HistogramPlotImp*>(pPlot->getPlot());
      if (pHistogramPlot != NULL)
      {
         pHistogramPlot->setHistogram(pLayer, channel);
      }

      // Update the displayed plot information
      updatePlotInfo(pLayer, channel);

      // Activate the plot
      PlotWindowImp::setCurrentPlot(pPlot);

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
            QString strViewName = QString::fromStdString(viewName);

            pPlotSet = getPlotSet(strViewName);
            if (pPlotSet == NULL)
            {
               pPlotSet = createPlotSet(strViewName);
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

   if (pPlot == getCurrentPlot())
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

   if (pPlot == getCurrentPlot())
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
      if (pPlotWidget == getCurrentPlot())
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
         pPlotSet->deletePlot(pPlotWidget);
         if (pPlotSet->getNumPlots() == 0)
         {
            deletePlotSet(pPlotSet);
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

   PlotWidget* pPlotWidget = getPlot(pLayer, channel);
   if (pPlotWidget != NULL)
   {
      PlotSet* pPlotSet = pPlotWidget->getPlotSet();
      if (pPlotSet != NULL)
      {
         pPlotWidget->detach(SIGNAL_NAME(PlotWidget, AboutToShowContextMenu),
            Slot(this, &HistogramWindowImp::updateContextMenu));

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
            deletePlotSet(pPlotSet);
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

   return PlotWindowImp::event(pEvent);
}

void HistogramWindowImp::setCurrentPlot(const DisplayMode& displayMode)
{
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

void HistogramWindowImp::activateLayer(PlotWidget* pPlot)
{
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
               if (pLayerList->getPrimaryRasterElement() == pElement)
               {
                  activate = false;
               }
            }
         }
      }

      if (activate)
      {
         Service<DesktopServices> pDesktop;
         WorkspaceWindow* pWindow = pDesktop->getCurrentWorkspaceWindow();
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
               pView->setFrontLayer(pLayer);
            }
         }
      }
   }

   emit plotActivated(pLayer, channel);
}

void HistogramWindowImp::showEvent(QShowEvent* pEvent)
{
   PlotWindowImp::showEvent(pEvent);
   mUpdater.update();
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

   PlotWidgetImp* pPlotWidget = dynamic_cast<PlotWidgetImp*>(getPlot(pLayer, channel));
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
   mpRasterLayer(new AttachmentPtr<RasterLayer>(pLayer)),
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
