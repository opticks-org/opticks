/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTSETGROUPIMP_H
#define PLOTSETGROUPIMP_H

#include "SerializableImp.h"
#include "SubjectImp.h"
#include "TypesFile.h"

#include <QtGui/QWidget>

#include <boost/any.hpp>
#include <map>
#include <string>
#include <vector>

class DataVariant;
class DynamicObject;
class InfoBar;
class PlotSet;
class PlotWidget;
class QAction;
class QIcon;
class QStackedWidget;
class Signature;
class Subject;
class View;

class PlotSetGroupImp : public QWidget, public SubjectImp
{
   Q_OBJECT

public:
   PlotSetGroupImp(QWidget* pParent = NULL);
   virtual ~PlotSetGroupImp();

   QWidget* getWidget();
   const QWidget* getWidget() const;

   PlotSet* createPlotSet(const QString& plotSetName);
   PlotSet* getPlotSet(const QString& plotSetName) const;
   PlotSet* getPlotSet(PlotWidget* pPlot) const;
   std::vector<PlotSet*> getPlotSets() const;
   unsigned int getNumPlotSets() const;
   bool containsPlotSet(PlotSet* pPlotSet) const;
   bool setCurrentPlotSet(PlotSet* pPlotSet);
   PlotSet* getCurrentPlotSet() const;
   bool renamePlotSet(PlotSet* pPlotSet, const QString& newName);
   bool deletePlotSet(PlotSet* pPlotSet);

   std::vector<PlotWidget*> getPlots(PlotType plotType) const;
   std::vector<PlotWidget*> getPlots() const;
   unsigned int getNumPlots() const;
   bool containsPlot(PlotWidget* pPlot) const;
   bool setCurrentPlot(PlotWidget* pPlot);
   PlotWidget* getCurrentPlot() const;

   PlotWidget* plotData(const Signature& sig, const std::string& xAttribute, const std::string& yAttribute,
      const std::string& plotName);
   PlotWidget* plotData(const DynamicObject& obj, const std::string& xAttribute, const std::string& yAttribute,
      const std::string& plotName);

   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& className) const;
   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void clear();
   void setInfoBarIcon(const QIcon& icon);
   void setInfoBarElideMode(Qt::TextElideMode mode);

signals:
   void plotSetAdded(PlotSet* pPlotSet);
   void plotSetActivated(PlotSet* pPlotSet);
   void plotSetDeleted(PlotSet* pPlotSet);

protected:
   void plotSetViewAssociated(Subject& subject, const std::string& signal, const boost::any& value);
   void plotSetViewDeleted(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void setCurrentPlotSet(QAction* pAction);

private:
   PlotSetGroupImp(const PlotSetGroupImp& rhs);
   PlotSetGroupImp& operator=(const PlotSetGroupImp& rhs);

   PlotWidget* plotData(const DataVariant& xRawData, const DataVariant& yRawData, const std::string& xAttribute,
      const std::string& yAttribute, const std::string& plotName);

   InfoBar* mpInfoBar;
   QStackedWidget* mpStack;
   std::map<PlotSet*, View*> mPlotSets;
};

#define PLOTSETGROUPADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES \
   SERIALIZABLEADAPTEREXTENSION_CLASSES

#define PLOTSETGROUPADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   SERIALIZABLEADAPTER_METHODS(impClass) \
   QWidget* getWidget() \
   { \
      return impClass::getWidget(); \
   } \
   const QWidget* getWidget() const \
   { \
      return impClass::getWidget(); \
   } \
   PlotSet* createPlotSet(const std::string& plotSetName) \
   { \
      return impClass::createPlotSet(QString::fromStdString(plotSetName)); \
   } \
   PlotSet* getPlotSet(const std::string& plotSetName) const \
   { \
      return impClass::getPlotSet(QString::fromStdString(plotSetName)); \
   } \
   PlotSet* getPlotSet(PlotWidget* pPlot) const \
   { \
      return impClass::getPlotSet(pPlot); \
   } \
   std::vector<PlotSet*> getPlotSets() const \
   { \
      return impClass::getPlotSets(); \
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
   bool renamePlotSet(PlotSet* pPlotSet, const std::string& newName) \
   { \
      return impClass::renamePlotSet(pPlotSet, QString::fromStdString(newName)); \
   } \
   bool deletePlotSet(PlotSet* pPlotSet) \
   { \
      return impClass::deletePlotSet(pPlotSet); \
   } \
   std::vector<PlotWidget*> getPlots(PlotType plotType) const \
   { \
      return impClass::getPlots(plotType); \
   } \
   std::vector<PlotWidget*> getPlots() const \
   { \
      return impClass::getPlots(); \
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
   PlotWidget* plotData(const Signature& sig, const std::string& xAttribute, const std::string& yAttribute, \
      const std::string& plotName) \
   { \
      return impClass::plotData(sig, xAttribute, yAttribute, plotName); \
   } \
   PlotWidget* plotData(const DynamicObject& obj, const std::string& xAttribute, const std::string& yAttribute, \
      const std::string& plotName) \
   { \
      return impClass::plotData(obj, xAttribute, yAttribute, plotName); \
   } \
   void clear() \
   { \
      impClass::clear(); \
   } \
   void setInfoBarIcon(const QIcon& icon) \
   { \
      impClass::setInfoBarIcon(icon); \
   }

#endif
