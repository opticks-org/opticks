/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HISTOGRAMWINDOWIMP_H
#define HISTOGRAMWINDOWIMP_H

#include <QtGui/QAction>

#include "AttachmentPtr.h"
#include "PlotWindowImp.h"
#include "RasterLayer.h"
#include "SessionExplorer.h"

#include <boost/shared_ptr.hpp>
#include <set>

class Layer;
class PlotWidget;
class PlotWidgetImp;

class HistogramWindowImp : public PlotWindowImp
{
   Q_OBJECT

public:
   HistogramWindowImp(const std::string& id, const std::string& windowName, QWidget* pParent = 0);
   ~HistogramWindowImp();

   using SessionItemImp::setIcon;
   void updateContextMenu(Subject& subject, const std::string& signal, const boost::any& value);
   void activateLayerPlot(Subject& subject, const std::string& signal, const boost::any& value);

   PlotSet* createPlotSet(const QString& strPlotSet);
   bool deletePlotSet(PlotSet* pPlotSet);

   PlotWidget* getPlot(Layer* pLayer) const;
   PlotWidget* getPlot(Layer* pLayer, const RasterChannelType& eColor) const;    // Deprecated
   PlotWidget* getPlot(RasterLayer* pLayer, RasterChannelType channel) const;
   using PlotWindowImp::setCurrentPlot;

   void createSubsetPlot(Layer* pLayer);

public slots:
   PlotWidget* createPlot(Layer* pLayer);
   PlotWidget* createPlot(Layer* pLayer, PlotSet* pPlotSet);
   PlotWidget* createPlot(RasterLayer* pLayer, RasterChannelType channel);
   PlotWidget* createPlot(RasterLayer* pLayer, RasterChannelType channel, PlotSet* pPlotSet);
   void setCurrentPlot(Layer* pLayer);
   bool setCurrentPlot(Layer* pLayer, const RasterChannelType& eColor);          // Deprecated
   bool setCurrentPlot(RasterLayer* pLayer, RasterChannelType channel);
   void deletePlot(Layer* pLayer);
   void deletePlot(RasterLayer* pLayer, RasterChannelType channel);

signals:
   void plotActivated(Layer* pLayer, const RasterChannelType& color);

protected:
   bool event(QEvent* pEvent);
   void showEvent(QShowEvent * pEvent);

   void createPlots(RasterLayer* pLayer, DisplayMode displayMode);
   void createPlots(RasterLayer* pLayer, DisplayMode displayMode, PlotSet* pPlotSet);
   void deletePlots(RasterLayer* pLayer, DisplayMode displayMode);
   void updatePlotInfo(RasterLayer* pLayer, RasterChannelType channel);

protected slots:
   void setCurrentPlot(const DisplayMode& displayMode);
   void activateLayer(PlotWidget* pPlot);
   void updatePlotInfo(RasterChannelType channel);
   void syncAutoZoom();
   void setStatisticsShown(bool shown);
   void setStatisticsShowActionState(PlotSet* pPlotSet);

private:
   void setStatisticsShowActionState(PlotWidgetImp* pPlot);
   void deleteStatisticsPlots(Layer* pLayer);

   AttachmentPtr<SessionExplorer> mpExplorer;
   bool mDisplayModeChanging;
   QAction* mpSyncAutoZoomAction;
   QAction* mpStatisticsShowAction;

   class HistogramUpdater
   {
   public:
      HistogramUpdater(HistogramWindowImp *pWindow);

      void initialize(RasterLayer* pLayer, RasterChannelType channel);
      void update();
   private:
      class UpdateMomento
      {
      public:
         UpdateMomento(HistogramWindowImp *pWindow, RasterLayer* pLayer, RasterChannelType channel);
         bool operator<(const UpdateMomento &rhs) const;
         void update() const;
      private:
         HistogramWindowImp* mpWindow;
         boost::shared_ptr<SafePtr<RasterLayer> > mpRasterLayer;
         RasterChannelType mChannel;
      };
      std::set<UpdateMomento> mUpdatesPending;
      HistogramWindowImp* mpWindow;
   } mUpdater;
};

#define HISTOGRAMWINDOWADAPTEREXTENSION_CLASSES \
   PLOTWINDOWADAPTEREXTENSION_CLASSES

#define HISTOGRAMWINDOWADAPTER_METHODS(impClass) \
   PLOTWINDOWADAPTER_METHODS(impClass) \
   PlotWidget* createPlot(Layer* pLayer, PlotSet* pPlotSet) \
   { \
      return impClass::createPlot(pLayer, pPlotSet); \
   } \
   PlotWidget* getPlot(Layer* pLayer) const \
   { \
      return impClass::getPlot(pLayer); \
   } \
   PlotWidget* getPlot(Layer* pLayer, const RasterChannelType& eColor) const \
   { \
      return impClass::getPlot(pLayer, eColor); \
   } \
   void setCurrentPlot(Layer* pLayer) \
   { \
      impClass::setCurrentPlot(pLayer); \
   } \
   bool setCurrentPlot(Layer* pLayer, const RasterChannelType& eColor) \
   { \
      return impClass::setCurrentPlot(pLayer, eColor); \
   } \
   void deletePlot(Layer* pLayer) \
   { \
      impClass::deletePlot(pLayer); \
   } \
   void createSubsetPlot(Layer* pLayer) \
   { \
      impClass::createSubsetPlot(pLayer); \
   }

#endif
