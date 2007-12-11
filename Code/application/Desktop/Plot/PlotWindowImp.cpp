/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QContextMenuEvent>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>

#include "Axis.h"
#include "ContextMenuImp.h"
#include "DataVariant.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "PlotWindowImp.h"
#include "InfoBar.h"
#include "MessageLogResource.h"
#include "PlotView.h"
#include "PlotWidget.h"
#include "PlotWindow.h"
#include "PlotWindowImp.h"
#include "PointSet.h"
#include "SessionItemSerializer.h"
#include "Signature.h"

#include <algorithm>
XERCES_CPP_NAMESPACE_USE
using namespace std;

PlotWindowImp::PlotWindowImp(const string& id, const string& windowName, QWidget* parent) :
   DockWindowImp(id, windowName, parent)
{
   mpInfoBar = NULL;
   mpStack = NULL;

   // Dock window widget
   QWidget* pWindowWidget = new QWidget(this);

   // Popup menu
   QMenu* pMenu = new QMenu(NULL);

   // Info bar
   mpInfoBar = new InfoBar(pWindowWidget);
   mpInfoBar->setBackgroundColor(Qt::darkGray);
   mpInfoBar->setTitleColor(Qt::white);
   mpInfoBar->setTitleFont(QFont("Arial", 12, QFont::Bold));
   mpInfoBar->setMenu(pMenu);

   // Widget stack
   mpStack = new QStackedWidget(pWindowWidget);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(pWindowWidget);
   pLayout->setMargin(2);
   pLayout->setSpacing(2);
   pLayout->addWidget(mpInfoBar);
   pLayout->addWidget(mpStack, 10);

   // Initialization
   setWidget(pWindowWidget);
   pWindowWidget->show();    // Must call show() to show the dock window widget if a plug-in creates the window

   // Connections
   connect(mpInfoBar, SIGNAL(titleChanged(const QString&)), this, SLOT(setCurrentPlotSet(const QString&)));
}

PlotWindowImp::~PlotWindowImp()
{
   disconnect(mpInfoBar, SIGNAL(titleChanged(const QString&)), this, SLOT(setCurrentPlotSet(const QString&)));
   clear();
}

list<ContextMenuAction> PlotWindowImp::getContextMenuActions() const
{
   list<ContextMenuAction> menuActions = DockWindowImp::getContextMenuActions();

   PlotSet* pPlotSet = getCurrentPlotSet();
   if (pPlotSet != NULL)
   {
      list<ContextMenuAction> plotSetActions = pPlotSet->getContextMenuActions();
      copy(plotSetActions.begin(), plotSetActions.end(), back_inserter(menuActions));
   }

   return menuActions;
}

const string& PlotWindowImp::getObjectType() const
{
   static string type("PlotWindowImp");
   return type;
}

bool PlotWindowImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlotWindow"))
   {
      return true;
   }

   return DockWindowImp::isKindOf(className);
}

WindowType PlotWindowImp::getWindowType() const
{
   return PLOT_WINDOW;
}

PlotSet* PlotWindowImp::createPlotSet(const QString& strPlotSet)
{
   if (strPlotSet.isEmpty() == true)
   {
      return NULL;
   }

   // Do not create the plot set if one with the same name already exists
   PlotSetAdapter* pPlotSet = NULL;
   pPlotSet = (PlotSetAdapter*) getPlotSet(strPlotSet);
   if (pPlotSet != NULL)
   {
      return NULL;
   }

   // Create the plot set
   pPlotSet = new PlotSetAdapter(SessionItemImp::generateUniqueId(), strPlotSet.toStdString(),
      dynamic_cast<PlotWindow*>(this), mpStack);
   if (pPlotSet == NULL)
   {
      return NULL;
   }

   pPlotSet->setTabPosition(QTabWidget::South);
   pPlotSet->setTabShape(QTabWidget::Triangular);
   pPlotSet->installEventFilter(this);
   mPlotSets.push_back((PlotSet*) pPlotSet);

   // Add the plot set to the widget stack
   mpStack->addWidget(pPlotSet);

   // Add the plot set name to the list box
   QMenu* pMenu = mpInfoBar->getMenu();
   if (pMenu != NULL)
   {
      pMenu->addAction(strPlotSet);
   }

   // Notify connected and attached objects
   emit plotSetAdded((PlotSet*) pPlotSet);
   notify(SIGNAL_NAME(PlotWindow, PlotSetAdded), boost::any(static_cast<PlotSet*>(pPlotSet)));

   // Activate the plot set
   setCurrentPlotSet(pPlotSet);
   return pPlotSet;
}

PlotSet* PlotWindowImp::getPlotSet(const QString& strPlotSet) const
{
   if (strPlotSet.isEmpty() == true)
   {
      return NULL;
   }

   vector<PlotSet*>::const_iterator iter = mPlotSets.begin();
   while (iter != mPlotSets.end())
   {
      PlotSet* pPlotSet = *iter;
      if (pPlotSet != NULL)
      {
         QString strCurrentName = QString::fromStdString(pPlotSet->getName());
         if (strCurrentName == strPlotSet)
         {
            return pPlotSet;
         }
      }

      ++iter;
   }

   return NULL;
}

PlotSet* PlotWindowImp::getPlotSet(PlotWidget* pPlot) const
{
   if (pPlot == NULL)
   {
      return NULL;
   }

   vector<PlotSet*>::const_iterator iter = mPlotSets.begin();
   while (iter != mPlotSets.end())
   {
      PlotSet* pPlotSet = NULL;
      pPlotSet = *iter;
      if (pPlotSet != NULL)
      {
         bool bContains = false;
         bContains = pPlotSet->containsPlot(pPlot);
         if (bContains == true)
         {
            return pPlotSet;
         }
      }

      ++iter;
   }

   return NULL;
}

vector<PlotSet*> PlotWindowImp::getPlotSets() const
{
   vector<PlotSet*> plots;

   vector<PlotSet*>::const_iterator iter = mPlotSets.begin();
   while (iter != mPlotSets.end())
   {
      PlotSet* pPlotSet = NULL;
      pPlotSet = *iter;
      if (pPlotSet != NULL)
      {
         plots.push_back(pPlotSet);
      }

      ++iter;
   }

   return plots;
}

unsigned int PlotWindowImp::getNumPlotSets() const
{
   return mPlotSets.size();
}

bool PlotWindowImp::containsPlotSet(PlotSet* pPlotSet) const
{
   if (pPlotSet == NULL)
   {
      return false;
   }

   vector<PlotSet*>::const_iterator iter = mPlotSets.begin();
   while (iter != mPlotSets.end())
   {
      PlotSet* pCurrentPlotSet = NULL;
      pCurrentPlotSet = *iter;
      if (pCurrentPlotSet == pPlotSet)
      {
         return true;
      }

      ++iter;
   }

   return false;
}

bool PlotWindowImp::setCurrentPlotSet(PlotSet* pPlotSet)
{
   if (pPlotSet == NULL)
   {
      return false;
   }

   if (containsPlotSet(pPlotSet) == false)
   {
      return false;
   }

   // Set the new title
   string plotSetName = pPlotSet->getName();
   if (plotSetName.empty() == false)
   {
      mpInfoBar->setTitle(QString::fromStdString(plotSetName));
   }

   // Activate the tab widget
   mpStack->setCurrentWidget(dynamic_cast<PlotSetImp*>(pPlotSet));

   emit plotSetActivated(pPlotSet);
   notify(SIGNAL_NAME(PlotWindow, PlotSetActivated), boost::any(pPlotSet));
   return true;
}

PlotSet* PlotWindowImp::getCurrentPlotSet() const
{
   PlotSet* pPlotSet = dynamic_cast<PlotSet*>(mpStack->currentWidget());
   return pPlotSet;
}

bool PlotWindowImp::renamePlotSet(PlotSet* pPlotSet, const QString& strNewName)
{
   if ((pPlotSet == NULL) || (strNewName.isEmpty() == true))
   {
      return false;
   }

   string newName = strNewName.toStdString();

   // Check if the new name is the same as the current name
   const string& currentName = pPlotSet->getName();
   if (newName == currentName)
   {
      return false;
   }

   // Check if an existing plot set contains the new name
   vector<PlotSet*>::iterator iter = mPlotSets.begin();
   while (iter != mPlotSets.end())
   {
      PlotSet* pCurrentPlotSet = *iter;
      if (pCurrentPlotSet != NULL)
      {
         string plotSetName = pCurrentPlotSet->getName();
         if (plotSetName == newName)
         {
            return false;
         }
      }

      ++iter;
   }

   // Rename the plot set
   pPlotSet->setName(newName);

   // Update the info bar
   QMenu* pMenu = mpInfoBar->getMenu();
   if (pMenu != NULL)
   {
      QList<QAction*> menuActions = pMenu->actions();
      for (int i = 0; i < menuActions.count(); ++i)
      {
         QAction* pAction = menuActions[i];
         if (pAction != NULL)
         {
            QString strAction = pAction->text();
            if (strAction == QString::fromStdString(currentName))
            {
               pAction->setText(strNewName);
            }
         }
      }
   }

   mpInfoBar->setTitle(strNewName);
   return true;
}

bool PlotWindowImp::deletePlotSet(PlotSet* pPlotSet)
{
   PlotSetImp* pPlotSetImp = dynamic_cast<PlotSetImp*>(pPlotSet);
   if (pPlotSetImp == NULL)
   {
      return false;
   }

   // Get the name of the plot set that is being removed
   QString strPlotSetName = QString::fromStdString(pPlotSetImp->getName());

   // Remove the plot set from the vector
   vector<PlotSet*>::iterator iter = mPlotSets.begin();
   while (iter != mPlotSets.end())
   {
      PlotSet* pCurrentPlotSet = *iter;
      if (pCurrentPlotSet == pPlotSet)
      {
         mPlotSets.erase(iter);
         mpStack->removeWidget(pPlotSetImp);
         emit plotSetDeleted(pPlotSet);
         notify(SIGNAL_NAME(PlotWindow, PlotSetDeleted), boost::any(pPlotSet));
         delete pPlotSetImp;
         break;
      }

      ++iter;
   }

   // Update the info bar title with the new current plot set
   bool bSuccess = false;

   PlotSet* pDisplayPlotSet = dynamic_cast<PlotSet*>(mpStack->currentWidget());
   if (pDisplayPlotSet != NULL)
   {
      bSuccess = setCurrentPlotSet(pDisplayPlotSet);
   }

   if (bSuccess == false)
   {
      mpInfoBar->setTitle(QString());
   }

   // Remove the action from the info bar menu
   QMenu* pMenu = mpInfoBar->getMenu();
   if (pMenu != NULL)
   {
      QList<QAction*> menuActions = pMenu->actions();
      for (int i = 0; i < menuActions.count(); ++i)
      {
         QAction* pAction = menuActions[i];
         if (pAction != NULL)
         {
            QString strAction = pAction->text();
            if (strAction == strPlotSetName)
            {
               pMenu->removeAction(pAction);
            }
         }
      }
   }

   return true;
}

vector<PlotWidget*> PlotWindowImp::getPlots(const PlotType& plotType) const
{
   vector<PlotWidget*> plots;

   vector<PlotSet*>::const_iterator iter;
   for (iter = mPlotSets.begin(); iter != mPlotSets.end(); ++iter)
   {
      PlotSet* pPlotSet = *iter;
      if (pPlotSet != NULL)
      {
         vector<PlotWidget*> currentPlots;
         pPlotSet->getPlots(currentPlots);

         vector<PlotWidget*>::const_iterator widgetIter;
         for (widgetIter = currentPlots.begin(); widgetIter != currentPlots.end(); ++widgetIter)
         {
            PlotWidget* pPlot = *widgetIter;
            if (pPlot != NULL)
            {
               PlotView* pPlotView = pPlot->getPlot();
               if (pPlotView != NULL)
               {
                  PlotType currentType = pPlotView->getPlotType();
                  if (currentType == plotType)
                  {
                     plots.push_back(pPlot);
                  }
               }
            }
         }
      }
   }

   return plots;
}

vector<PlotWidget*> PlotWindowImp::getPlots() const
{
   vector<PlotWidget*> plots;

   vector<PlotSet*>::const_iterator iter = mPlotSets.begin();
   while (iter != mPlotSets.end())
   {
      PlotSet* pPlotSet = *iter;
      if (pPlotSet != NULL)
      {
         vector<PlotWidget*> currentPlots;
         pPlotSet->getPlots(currentPlots);

         vector<PlotWidget*>::const_iterator widgetIter = currentPlots.begin();
         while (widgetIter != currentPlots.end())
         {
            PlotWidget* pPlot = *widgetIter;
            if (pPlot != NULL)
            {
               plots.push_back(pPlot);
            }

            ++widgetIter;
         }
      }

      ++iter;
   }

   return plots;
}

unsigned int PlotWindowImp::getNumPlots() const
{
   vector<PlotWidget*> plots = getPlots();
   return plots.size();
}

bool PlotWindowImp::containsPlot(PlotWidget* pPlot) const
{
   if (pPlot == NULL)
   {
      return false;
   }

   vector<PlotWidget*> plots = getPlots();

   vector<PlotWidget*>::const_iterator iter = plots.begin();
   while (iter != plots.end())
   {
      PlotWidget* pCurrentPlot = *iter;
      if (pCurrentPlot == pPlot)
      {
         return true;
      }

      ++iter;
   }

   return false;
}

bool PlotWindowImp::setCurrentPlot(PlotWidget* pPlot)
{
   if (pPlot == NULL)
   {
      return false;
   }

   PlotSet* pPlotSet = getPlotSet(pPlot);
   if (pPlotSet != NULL)
   {
      if (pPlotSet != getCurrentPlotSet())
      {
         setCurrentPlotSet(pPlotSet);
      }

      return pPlotSet->setCurrentPlot(pPlot);
   }

   return false;
}

PlotWidget* PlotWindowImp::getCurrentPlot() const
{
   PlotSet* pPlotSet = getCurrentPlotSet();
   if (pPlotSet != NULL)
   {
      PlotWidget* pPlotWidget = pPlotSet->getCurrentPlot();
      return pPlotWidget;
   }

   return NULL;
}

void PlotWindowImp::clear()
{
   vector<PlotSet*> plotSets = getPlotSets();
   for (unsigned int i = 0; i < plotSets.size(); i++)
   {
      PlotSet* pPlotSet = plotSets[i];
      if (pPlotSet != NULL)
      {
         deletePlotSet(pPlotSet);
      }
   }
}

bool PlotWindowImp::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if (pEvent != NULL)
   {
      PlotSetImp* pPlotSet = dynamic_cast<PlotSetImp*>(getCurrentPlotSet());
      if ((pObject != NULL) && (pObject == pPlotSet))
      {
         if (pEvent->type() == QEvent::ContextMenu)
         {
            QContextMenuEvent* pMenuEvent = static_cast<QContextMenuEvent*>(pEvent);
            contextMenuEvent(pMenuEvent);
            return true;
         }
      }
   }

   return DockWindowImp::eventFilter(pObject, pEvent);
}

void PlotWindowImp::contextMenuEvent(QContextMenuEvent* pEvent)
{
   if (pEvent != NULL)
   {
      // Create the context menu
      const QPoint& mouseLocation = pEvent->globalPos();
      list<ContextMenuAction> defaultActions = getContextMenuActions();

      vector<SessionItem*> sessionItems;
      sessionItems.push_back(dynamic_cast<SessionItem*>(this));
      sessionItems.push_back(dynamic_cast<SessionItem*>(getCurrentPlotSet()));

      ContextMenuImp menu(sessionItems, mouseLocation, defaultActions, this);

      // Notify to allow additional actions to be added
      notify(SIGNAL_NAME(DockWindow, AboutToShowContextMenu), boost::any(static_cast<ContextMenu*>(&menu)));

      // Invoke the menu
      if (menu.show() == true)
      {
         return;
      }
   }

   DockWindowImp::contextMenuEvent(pEvent);
}

InfoBar* PlotWindowImp::getInfoBar() const
{
   return mpInfoBar;
}

void PlotWindowImp::setCurrentPlotSet(const QString& strPlotSet)
{
   if (strPlotSet.isEmpty() == true)
   {
      return;
   }

   PlotSet* pPlotSet = getPlotSet(strPlotSet);
   if (pPlotSet != NULL)
   {
      setCurrentPlotSet(pPlotSet);
   }
}

namespace
{
   template<class T>
   void extractFromVariant(const DataVariant &var, vector<double> &data)
   {
      const vector<T>* pVec = var.getPointerToValue<vector<T> >();
      if (pVec != NULL)
      {
         data.reserve(pVec->size());
         copy(pVec->begin(), pVec->end(), back_inserter(data));
      }
   }
   void convertRawData(const DataVariant &var, vector<double> &data)
   {
      // list<types> typeList = char, unsigned char, short, ...
      // for each type in typeList
      //    data = extractFromVariant<type>(var);
      //    if (!data.empty) return data;
      extractFromVariant<char>(var, data); if (!data.empty()) return;
      extractFromVariant<unsigned char>(var, data); if (!data.empty()) return;
      extractFromVariant<short>(var, data); if (!data.empty()) return;
      extractFromVariant<unsigned short>(var, data); if (!data.empty()) return;
      extractFromVariant<int>(var, data); if (!data.empty()) return;
      extractFromVariant<unsigned int>(var, data); if (!data.empty()) return;
      extractFromVariant<long>(var, data); if (!data.empty()) return;
      extractFromVariant<unsigned long>(var, data); if (!data.empty()) return;
      extractFromVariant<float>(var, data); if (!data.empty()) return;
      extractFromVariant<double>(var, data);
   }

}

PlotWidget *PlotWindowImp::plotData(const DataVariant &xRawData, const DataVariant &yRawData,
   const string &xAttribute, const string &yAttribute, const string &plotName)
{
   vector<double> xData;
   convertRawData(xRawData, xData);
   vector<double> yData;
   convertRawData(yRawData, yData);
   if (xData.size() != yData.size() || xData.size() == 0 || yData.size() == 0)
   {
      string message = "The selected attributes are not plottable:\n" +
         yAttribute + " vs. " + xAttribute;
      QMessageBox::critical(this, "Unable to plot", message.c_str(), QMessageBox::Ok, 0);
      MessageResource msg("Plotting", "app", "7137567E-0EF3-4e46-9706-2F2F9764AD1C");
      msg->addProperty("Message", message);
      return NULL;
   }

   PlotSet *pSet = getCurrentPlotSet();
   if (pSet == NULL)
   {
      pSet = this->createPlotSet(QString::fromStdString(plotName));
      VERIFYRV(pSet != NULL, NULL);
   }

   PlotWidget *pPlotWidget = NULL;
   pPlotWidget = pSet->getPlot(plotName);

   if (pPlotWidget == NULL)
   {
      pPlotWidget = pSet->createPlot(plotName, CARTESIAN_PLOT);

      Axis *pAxis = pPlotWidget->getAxis(AXIS_BOTTOM);
      VERIFYRV(pAxis != NULL, NULL);
      pAxis->setTitle(xAttribute);
      pAxis = pPlotWidget->getAxis(AXIS_LEFT);
      VERIFYRV(pAxis != NULL, NULL);
      pAxis->setTitle(yAttribute);
   }
   else
   {
      Axis *pAxis = pPlotWidget->getAxis(AXIS_BOTTOM);
      VERIFYRV(pAxis != NULL, NULL);
      string xText = pAxis->getTitle();
      pAxis = pPlotWidget->getAxis(AXIS_LEFT);
      VERIFYRV(pAxis != NULL, NULL);
      string yText = pAxis->getTitle();
      if (xText != xAttribute || yText != yAttribute)
      {
         stringstream message;
         message << "The attributes of the data don't match the axes of the plot:\n" <<
            "Data = " << yAttribute << " vs. " << xAttribute << "\n" <<
            "Axes = " << yText << " vs. " << xText << "\n" <<
            "Do you wish to add this data to this plot anyway?";
         int response = QMessageBox::warning(this, "Axis mismatch", message.str().c_str(), 
            QMessageBox::Yes, QMessageBox::No);
         if (response == QMessageBox::No)
         {
            return NULL;
         }
      }
   }

   PlotView *pPlotView = pPlotWidget->getPlot();
   PointSet *pPointSet = dynamic_cast<PointSet*>(pPlotView->addObject(POINT_SET, true));
   for (unsigned int i=0; i<xData.size(); ++i)
   {
      pPointSet->addPoint(xData[i], yData[i]);
   }

   return pPlotWidget;
}

PlotWidget* PlotWindowImp::plotData(const Signature &sig, const std::string &xAttribute,
   const std::string &yAttribute, const std::string &plotName)
{
   const DataVariant &xRawData = sig.getData(xAttribute);
   const DataVariant &yRawData = sig.getData(yAttribute);
   return plotData(xRawData, yRawData, xAttribute, yAttribute, plotName);
}

PlotWidget* PlotWindowImp::plotData(const DynamicObject &obj, const std::string &xAttribute,
   const std::string &yAttribute, const std::string &plotName)
{
   const DataVariant &xRawData = obj.getAttribute(xAttribute);
   const DataVariant &yRawData = obj.getAttribute(yAttribute);
   return plotData(xRawData, yRawData, xAttribute, yAttribute, plotName);
}

bool PlotWindowImp::toXml(XMLWriter* pXml) const
{
   if (!DockWindowImp::toXml(pXml))
   {
      return false;
   }

   if (mpInfoBar != NULL)
   {
      pXml->addAttr("infoBarTitle", mpInfoBar->getTitle().toStdString());
      pXml->addAttr("infoBarTitleColor", mpInfoBar->getTitleColor().name().toStdString());
      pXml->addAttr("infoBarTitleFont", mpInfoBar->getTitleFont().toString().toStdString());
   }

   PlotSet* pSet = getCurrentPlotSet();
   if (pSet != NULL)
   {
      pXml->addAttr("currentPlotSetId", pSet->getId());
   }

   PlotWidget* pPlot = getCurrentPlot();
   if (pPlot != NULL)
   {
      pXml->addAttr("currentPlotId", pPlot->getId());
   }

   if (mPlotSets.size() > 0)
   {
      pXml->pushAddPoint(pXml->addElement("PlotSets"));
      vector<PlotSet*>::const_iterator it;
      for (it=mPlotSets.begin(); it!=mPlotSets.end(); ++it)
      {
         PlotSet* pSet = *it;
         if (pSet != NULL)
         {
            pXml->pushAddPoint(pXml->addElement("PlotId"));
            pXml->addText(pSet->getId());
            pXml->popAddPoint();
         }
      }
      pXml->popAddPoint();
   }

   return true;
}

bool PlotWindowImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   // remove any leftovers
   clear();

   if (!DockWindowImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement *pElem = static_cast<DOMElement*>(pDocument);
   if (pElem == NULL)
   {
      return false;
   }

   // get attributes
   string currentPlotSetId, currentPlotId;
   if (mpInfoBar != NULL)
   {
      string infoBarTitle = A(pElem->getAttribute(X("infoBarTitle")));
      if (!infoBarTitle.empty())
      {
         mpInfoBar->setTitle(infoBarTitle.c_str());
      }

      string infoBarTitleColor = A(pElem->getAttribute(X("infoBarTitleColor")));
      if (!infoBarTitleColor.empty())
      {
         QColor color(infoBarTitleColor.c_str());
         mpInfoBar->setTitleColor(color);
      }

      string infoBarTitleFont = A(pElem->getAttribute(X("infoBarTitleFont")));
      QFont font;
      if (font.fromString(infoBarTitleFont.c_str()))
      {
         mpInfoBar->setTitleFont(font);
      }

      currentPlotSetId = A(pElem->getAttribute(X("currentPlotSetId")));
      currentPlotId = A(pElem->getAttribute(X("currentPlotId")));
   }

   // get plot sets
   for (DOMNode *pChld = pDocument->getFirstChild();
      pChld != NULL;
      pChld = pChld->getNextSibling())
   {
      string name = A(pChld->getNodeName());
      if(name == "PlotSets")
      {
         for (DOMNode *pGChld = pChld->getFirstChild();
            pGChld != NULL;
            pGChld = pGChld->getNextSibling())
         {
            string name = A(pGChld->getNodeName());
            if(name == "PlotId")
            {
               string setId = A(pGChld->getFirstChild()->getNodeValue());
               PlotSetImp* pSet(NULL);
               SessionItem* pItem = Service<SessionManager>()->getSessionItem(setId);
               pSet = dynamic_cast<PlotSetImp*>(pItem);
               if (pSet != NULL)
               {
                  pSet->setTabPosition(QTabWidget::South);
                  pSet->setTabShape(QTabWidget::Triangular);
                  mPlotSets.push_back(dynamic_cast<PlotSet*>(pSet));
                  mpStack->addWidget(pSet);
                  QMenu* pMenu = mpInfoBar->getMenu();
                  if (pMenu != NULL)
                  {
                     pMenu->addAction(pSet->getName().c_str());
                  }
               }
            }
         }
         // set current plot set - have to wait to set current plot in plotset restore
         SessionItem* pItem = Service<SessionManager>()->getSessionItem(currentPlotSetId);
         if (pItem != NULL)
         {
            if (containsPlotSet(dynamic_cast<PlotSet*>(pItem)))
            {
               setCurrentPlotSet(dynamic_cast<PlotSet*>(pItem));
            }
         }
      }
   }
   setCurrentPlot(dynamic_cast<PlotWidget*>(Service<SessionManager>()->getSessionItem(currentPlotId)));

   return true;
}
