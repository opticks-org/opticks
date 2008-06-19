/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTSETIMP_H
#define PLOTSETIMP_H

#include <QtGui/QTabWidget>

#include "AttachmentPtr.h"
#include "Observer.h"
#include "SessionExplorer.h"
#include "SessionItemImp.h"
#include "SubjectImp.h"
#include "TypesFile.h"
#include "xmlwriter.h"

#include <string>
#include <vector>

#include "XercesIncludes.h"

class PlotWidget;
class PlotWindow;
class SessionItemDeserializer;
class SessionItemSerializer;
class View;

class PlotSetImp : public QTabWidget, public SessionItemImp, public SubjectImp, public Observer
{
   Q_OBJECT

public:
   PlotSetImp(const std::string& id, const std::string& plotSetName, PlotWindow* pPlotWindow, QWidget* parent = 0);
   ~PlotSetImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void elementDeleted(Subject &subject, const std::string &signal, const boost::any &v);
   void elementModified(Subject &subject, const std::string &signal, const boost::any &v);
   void updateContextMenu(Subject& subject, const std::string& signal, const boost::any& value);
   void attached(Subject &subject, const std::string &signal, const Slot &slot);

   void setName(const std::string& name);
   void setAssociatedView(View* pView);
   View* getAssociatedView() const;

   PlotWidget* createPlot(const QString& strPlotName, const PlotType& plotType);
   PlotWidget* getPlot(const QString& strPlotName) const;
   std::vector<PlotWidget*> getPlots(const PlotType& plotType) const;
   std::vector<PlotWidget*> getPlots() const;
   unsigned int getNumPlots() const;
   bool containsPlot(PlotWidget* pPlot) const;
   PlotWidget* getCurrentPlot() const;
   bool renamePlot(PlotWidget* pPlot, const QString& strNewName);
   QString renamePlot(PlotWidget* pPlot);
   bool deletePlot(PlotWidget* pPlot);

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer); 
   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   bool setCurrentPlot(PlotWidget* pPlot);
   void renameCurrentPlot();
   void destroyCurrentPlot();
   void activateSelectedPlot();
   void renameSelectedPlot();
   void destroySelectedPlots();
   void clear();

signals:
   void plotAdded(PlotWidget* pPlot);
   void plotActivated(PlotWidget* pPlot);
   void plotDeleted(PlotWidget* pPlot);
   void renamed(const QString& strName);

protected:
   void addPlot(PlotWidget* pPlot);

protected slots:
   void updatePlotName();

private:
   AttachmentPtr<SessionExplorer> mpExplorer;
   PlotWindow* mpPlotWindow;
   View* mpAssociatedView;

private slots:
   void activatePlot(int iIndex);
};

#define PLOTSETADAPTEREXTENSION_CLASSES \
   SESSIONITEMADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES

#define PLOTSETADAPTER_METHODS(impClass) \
   SESSIONITEMADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   void setName(const std::string& name) \
   { \
      impClass::setName(name); \
   } \
   void setAssociatedView(View* pView) \
   { \
      impClass::setAssociatedView(pView); \
   } \
   View* getAssociatedView() const \
   { \
      return impClass::getAssociatedView(); \
   } \
   PlotWidget* createPlot(const std::string& plotName, const PlotType& plotType) \
   { \
      return impClass::createPlot(QString::fromStdString(plotName), plotType); \
   } \
   PlotWidget* getPlot(const std::string& plotName) const \
   { \
      return impClass::getPlot(QString::fromStdString(plotName)); \
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
   std::string renamePlot(PlotWidget* pPlot) \
   { \
      return impClass::renamePlot(pPlot).toStdString(); \
   } \
   bool renamePlot(PlotWidget* pPlot, const std::string& newPlotName) \
   { \
      return impClass::renamePlot(pPlot, QString::fromStdString(newPlotName)); \
   } \
   bool deletePlot(PlotWidget* pPlot) \
   { \
      return impClass::deletePlot(pPlot); \
   } \
   void clear() \
   { \
      impClass::clear(); \
   }

#endif
