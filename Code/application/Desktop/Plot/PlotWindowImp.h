/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTWINDOWIMP_H
#define PLOTWINDOWIMP_H

#include <QtGui/QStackedWidget>

#include "DockWindowImp.h"
#include "PlotSetAdapter.h"
#include "TypesFile.h"

#include <vector>

class DataVariant;
class DynamicObject;
class InfoBar;
class PlotWidget;
class Signature;

/**
 *  A widget to manage multiple plots.
 *
 *  The plot window contains one or more plot sets, which are tabbed widgets
 *  that manage multiple plot widgets.  The list of plot sets is available in
 *  an information bar at the top of the window.  Each plot set can contain
 *  one or more plots, which appear as a tab with the plot name along the
 *  bottom of the window.
 *
 *  @see       DockWindowImp, PlotSetImp, PlotWidgetImp
 */
class PlotWindowImp : public DockWindowImp
{
   Q_OBJECT

public:
   PlotWindowImp(const std::string& id, const std::string& windowName, QWidget* parent = 0);
   ~PlotWindowImp();

   using SessionItemImp::setIcon;
   std::list<ContextMenuAction> getContextMenuActions() const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   WindowType getWindowType() const;
   using WindowImp::setName;

   virtual PlotSet* createPlotSet(const QString& strPlotSet);
   PlotSet* getPlotSet(const QString& strPlotSet) const;
   PlotSet* getPlotSet(PlotWidget* pPlot) const;
   std::vector<PlotSet*> getPlotSets() const;
   unsigned int getNumPlotSets() const;
   bool containsPlotSet(PlotSet* pPlotSet) const;
   bool setCurrentPlotSet(PlotSet* pPlotSet);
   PlotSet* getCurrentPlotSet() const;
   bool renamePlotSet(PlotSet* pPlotSet, const QString& strNewName);
   virtual bool deletePlotSet(PlotSet* pPlotSet);

   std::vector<PlotWidget*> getPlots(const PlotType& plotType) const;
   std::vector<PlotWidget*> getPlots() const;
   unsigned int getNumPlots() const;
   bool containsPlot(PlotWidget* pPlot) const;
   bool setCurrentPlot(PlotWidget* pPlot);
   PlotWidget* getCurrentPlot() const;

   PlotWidget* plotData(const Signature &sig, const std::string &xAttribute,
      const std::string &yAttribute, const std::string &plotName);
   PlotWidget* plotData(const DynamicObject &obj, const std::string &xAttribute,
      const std::string &yAttribute, const std::string &plotName);
   PlotWidget *plotData(const DataVariant &xRawData, const DataVariant &yRawData,
      const std::string &xAttribute, const std::string &yAttribute, const std::string &plotName);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void clear();

signals:
   void plotSetAdded(PlotSet* pPlotSet);
   void plotSetActivated(PlotSet* pPlotSet);
   void plotSetDeleted(PlotSet* pPlotSet);

protected:
   bool eventFilter(QObject* pObject, QEvent* pEvent);
   void contextMenuEvent(QContextMenuEvent* pEvent);

   /**
    *  Returns a pointer to the info bar above the tab widget.
    *
    *  @return   A pointer to the info bar.  NULL is returned if an error occurred.
    */
   InfoBar* getInfoBar() const;

protected slots:
   /**
    *  Sets the active plot set.
    *
    *  @param   strPlotSet
    *           The name of the plot set to activate.  Cannot be empty.
    */
   void setCurrentPlotSet(const QString& strPlotSet);

private:
   InfoBar* mpInfoBar;
   QStackedWidget* mpStack;
   std::vector<PlotSet*> mPlotSets;
};

#define PLOTWINDOWADAPTER_METHODS(impClass) \
   DOCKWINDOWADAPTER_METHODS(impClass) \
   PlotSet* createPlotSet(const std::string& plotSet) \
   { \
      return impClass::createPlotSet(QString::fromStdString(plotSet)); \
   } \
   PlotSet* getPlotSet(const std::string& plotSet) const \
   { \
      return impClass::getPlotSet(QString::fromStdString(plotSet)); \
   } \
   PlotSet* getPlotSet(PlotWidget* pPlot) const \
   { \
      return impClass::getPlotSet(pPlot); \
   } \
   void getPlotSets(std::vector<PlotSet*>& plotSets) const \
   { \
      plotSets = impClass::getPlotSets(); \
   } \
   unsigned int getNumPlotSets() const \
   { \
      return impClass::getNumPlotSets(); \
   } \
   bool containsPlotSet(PlotSet* pPlotSet) const \
   { \
      return impClass::containsPlotSet(pPlotSet); \
   } \
   bool setCurrentPlotSet(PlotSet* pPlotSet) \
   { \
      return impClass::setCurrentPlotSet(pPlotSet); \
   } \
   PlotSet* getCurrentPlotSet() const \
   { \
      return impClass::getCurrentPlotSet(); \
   } \
   bool renamePlotSet(PlotSet* pPlotSet, const std::string& newPlotSetName) \
   { \
      return impClass::renamePlotSet(pPlotSet, QString::fromStdString(newPlotSetName)); \
   } \
   bool deletePlotSet(PlotSet* pPlotSet) \
   { \
      return impClass::deletePlotSet(pPlotSet); \
   } \
   void getPlots(const PlotType& plotType, std::vector<PlotWidget*>& plots) const \
   { \
      plots = impClass::getPlots(plotType); \
   } \
   void getPlots(std::vector<PlotWidget*>& plots) const \
   { \
      plots = impClass::getPlots(); \
   } \
   unsigned int getNumPlots() const \
   { \
      return impClass::getNumPlots(); \
   } \
   bool containsPlot(PlotWidget* pPlot) const \
   { \
      return impClass::containsPlot(pPlot); \
   } \
   bool setCurrentPlot(PlotWidget* pPlot) \
   { \
      return impClass::setCurrentPlot(pPlot); \
   } \
   PlotWidget* getCurrentPlot() const \
   { \
      return impClass::getCurrentPlot(); \
   } \
   void clear() \
   { \
      impClass::clear(); \
   } \
   PlotWidget* plotData(const Signature& sig, const std::string& xAttribute, const std::string& yAttribute, \
      const std::string& plotName) \
   { \
      return impClass::plotData(sig, xAttribute, yAttribute, plotName); \
   } \
   PlotWidget* plotData(const DynamicObject& obj, const std::string& xAttribute, const std::string& yAttribute, \
      const std::string& plotName) \
   { \
      return impClass::plotData(obj, xAttribute, yAttribute, plotName); \
   }

#endif
