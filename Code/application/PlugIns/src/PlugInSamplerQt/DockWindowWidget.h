/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DOCKWINDOWWIDGET_H
#define DOCKWINDOWWIDGET_H

#include <boost/any.hpp>
#include <QtGui/QCheckBox>
#include <QtGui/QListWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QWidget>

#include "DesktopServices.h"

class DockWindow;
class ElidedLabel;
class PlotSet;
class PlotWidget;
class Subject;

class DockWindowWidget : public QWidget
{
   Q_OBJECT

public:
   DockWindowWidget(QWidget* pParent = 0);
   virtual ~DockWindowWidget();

protected:
   virtual void hideEvent(QHideEvent* pEvent);

   void windowAdded(Subject& subject, const std::string& signal, const boost::any& value);
   void windowActivated(Subject& subject, const std::string& signal, const boost::any& value);
   void windowRemoved(Subject& subject, const std::string& signal, const boost::any& value);
   void dockWindowWidgetSet(Subject& subject, const std::string& signal, const boost::any& value);
   void sessionRestored(Subject& subject, const std::string& signal, const boost::any& value);

   void setSelectedPlotWindow(DockWindow* pWindow);
   void setSelectedPlotSet(PlotSet* pPlotSet);
   void setSelectedPlot(PlotWidget* pPlot);
   DockWindow* getSelectedPlotWindow() const;
   PlotWidget* getSelectedPlot() const;

protected slots:
   void addPlotWindow();
   void deletePlotWindow();
   void showPlotWindow();
   void hidePlotWindow();
   void updatePlotWindowList();
   void deleteAllPlotWindows();
   void addPlot();
   void deletePlot();
   void updatePlotList();
   void activatePlot();
   void renamePlot();
   void editPlotProperties();

private:
   DockWindowWidget(const DockWindowWidget& rhs);
   DockWindowWidget& operator=(const DockWindowWidget& rhs);
   Service<DesktopServices> mpDesktop;

   QListWidget* mpWindowList;
   QCheckBox* mpDeleteWindowCheck;
   QTreeWidget* mpPlotTree;
   ElidedLabel* mpActiveWindowLabel;
};

#endif
