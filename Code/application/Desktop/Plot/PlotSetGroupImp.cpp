/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QMenu>
#include <QtGui/QStackedWidget>

#include "AppVerify.h"
#include "Axis.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "InfoBar.h"
#include "MessageLogResource.h"
#include "PlotSet.h"
#include "PlotSetImp.h"
#include "PlotSetGroup.h"
#include "PlotSetGroupImp.h"
#include "PlotView.h"
#include "PlotWidget.h"
#include "PointSet.h"
#include "SessionManager.h"
#include "Signature.h"
#include "Slot.h"
#include "xmlwriter.h"

#include <algorithm>

XERCES_CPP_NAMESPACE_USE

PlotSetGroupImp::PlotSetGroupImp(QWidget* pParent) :
   QWidget(pParent),
   mpInfoBar(NULL),
   mpStack(NULL)
{
   // Popup menu
   QMenu* pMenu = new QMenu(NULL);

   // Info bar
   mpInfoBar = new InfoBar(this);
   mpInfoBar->setBackgroundColor(Qt::darkGray);
   mpInfoBar->setTitleColor(Qt::white);
   mpInfoBar->setTitleFont(QFont("Arial", 12, QFont::Bold));
   mpInfoBar->setMenu(pMenu);

   // Widget stack
   mpStack = new QStackedWidget(this);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(2);
   pLayout->setSpacing(2);
   pLayout->addWidget(mpInfoBar);
   pLayout->addWidget(mpStack, 10);

   // Connections
   VERIFYNR(connect(pMenu, SIGNAL(triggered(QAction*)), this, SLOT(setCurrentPlotSet(QAction*))));
}

PlotSetGroupImp::~PlotSetGroupImp()
{
   QMenu* pMenu = mpInfoBar->getMenu();
   if (pMenu != NULL)
   {
      VERIFYNR(disconnect(pMenu, SIGNAL(triggered(QAction*)), this, SLOT(setCurrentPlotSet(QAction*))));
   }

   clear();
}

QWidget* PlotSetGroupImp::getWidget()
{
   return this;
}

const QWidget* PlotSetGroupImp::getWidget() const
{
   return this;
}

PlotSet* PlotSetGroupImp::createPlotSet(const QString& plotSetName)
{
   if (plotSetName.isEmpty() == true)
   {
      return NULL;
   }

   // Do not create the plot set if one with the same name already exists
   if (getPlotSet(plotSetName) != NULL)
   {
      return NULL;
   }

   PlotSet* pPlotSet = NULL;
   Service<SessionManager> pSessionManager;
   if (pSessionManager->isSessionLoading() == true)
   {
      // Retrieve the PlotSet
      pPlotSet = dynamic_cast<PlotSet*>(pSessionManager->getSessionItem(plotSetName.toStdString()));
   }

   if (pPlotSet == NULL)
   {
      // Create the plot set
      pPlotSet = Service<DesktopServices>()->createPlotSet(plotSetName.toStdString(), mpStack);
      if (pPlotSet == NULL)
      {
         return NULL;
      }
   }

   VERIFYNR(pPlotSet->attach(SIGNAL_NAME(PlotSet, ViewAssociated),
      Slot(this, &PlotSetGroupImp::plotSetViewAssociated)));
   mPlotSets[pPlotSet] = NULL;

   // Add the plot set to the widget stack
   QWidget* pWidget = pPlotSet->getWidget();
   VERIFYRV(pWidget != NULL, NULL);

   mpStack->addWidget(pWidget);

   // Add the plot set name to the list box
   QMenu* pMenu = mpInfoBar->getMenu();
   if (pMenu != NULL)
   {
      const std::string& actionName = pPlotSet->getDisplayName(true);

      QAction* pAction = new QAction(QString::fromStdString(actionName), this);
      pAction->setData(QVariant::fromValue(pPlotSet));
      pAction->setStatusTip(QString::fromStdString(pPlotSet->getName()));
      pMenu->addAction(pAction);
   }

   // Notify connected and attached objects
   emit plotSetAdded(pPlotSet);
   notify(SIGNAL_NAME(PlotSetGroup, PlotSetAdded), boost::any(pPlotSet));

   // Activate the plot set
   setCurrentPlotSet(pPlotSet);
   return pPlotSet;
}

PlotSet* PlotSetGroupImp::getPlotSet(const QString& plotSetName) const
{
   if (plotSetName.isEmpty() == true)
   {
      return NULL;
   }

   for (std::map<PlotSet*, View*>::const_iterator iter = mPlotSets.begin(); iter != mPlotSets.end(); ++iter)
   {
      PlotSet* pPlotSet = iter->first;
      if (pPlotSet != NULL)
      {
         if (QString::fromStdString(pPlotSet->getName()) == plotSetName)
         {
            return pPlotSet;
         }
      }
   }

   return NULL;
}

PlotSet* PlotSetGroupImp::getPlotSet(PlotWidget* pPlot) const
{
   if (pPlot == NULL)
   {
      return NULL;
   }

   for (std::map<PlotSet*, View*>::const_iterator iter = mPlotSets.begin(); iter != mPlotSets.end(); ++iter)
   {
      PlotSet* pPlotSet = iter->first;
      if (pPlotSet != NULL)
      {
         if (pPlotSet->containsPlot(pPlot) == true)
         {
            return pPlotSet;
         }
      }
   }

   return NULL;
}

std::vector<PlotSet*> PlotSetGroupImp::getPlotSets() const
{
   std::vector<PlotSet*> plotSets;
   for (std::map<PlotSet*, View*>::const_iterator iter = mPlotSets.begin(); iter != mPlotSets.end(); ++iter)
   {
      PlotSet* pPlotSet = iter->first;
      if (pPlotSet != NULL)
      {
         plotSets.push_back(pPlotSet);
      }
   }

   return plotSets;
}

unsigned int PlotSetGroupImp::getNumPlotSets() const
{
   return mPlotSets.size();
}

bool PlotSetGroupImp::containsPlotSet(PlotSet* pPlotSet) const
{
   if (pPlotSet == NULL)
   {
      return false;
   }

   std::map<PlotSet*, View*>::const_iterator iter = mPlotSets.find(pPlotSet);
   if (iter != mPlotSets.end())
   {
      return true;
   }

   return false;
}

bool PlotSetGroupImp::setCurrentPlotSet(PlotSet* pPlotSet)
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
   const std::string& plotSetName = pPlotSet->getDisplayName(true);
   if (plotSetName.empty() == false)
   {
      mpInfoBar->setTitle(QString::fromStdString(plotSetName));
   }

   // Activate the tab widget
   QWidget* pWidget = pPlotSet->getWidget();
   VERIFY(pWidget != NULL);

   mpStack->setCurrentWidget(pWidget);

   emit plotSetActivated(pPlotSet);
   notify(SIGNAL_NAME(PlotSetGroup, PlotSetActivated), boost::any(pPlotSet));
   return true;
}

PlotSet* PlotSetGroupImp::getCurrentPlotSet() const
{
   PlotSet* pPlotSet = dynamic_cast<PlotSet*>(mpStack->currentWidget());
   return pPlotSet;
}

bool PlotSetGroupImp::renamePlotSet(PlotSet* pPlotSet, const QString& newName)
{
   if ((pPlotSet == NULL) || (newName.isEmpty() == true))
   {
      return false;
   }

   std::string plotName = newName.toStdString();

   // Check if the new name is the same as the current name
   if (plotName == pPlotSet->getName())
   {
      return false;
   }

   // Check if an existing plot set contains the new name
   for (std::map<PlotSet*, View*>::const_iterator iter = mPlotSets.begin(); iter != mPlotSets.end(); ++iter)
   {
      PlotSet* pCurrentPlotSet = iter->first;
      if (pCurrentPlotSet != NULL)
      {
         if (pCurrentPlotSet->getName() == plotName)
         {
            return false;
         }
      }
   }

   // Rename the plot set
   pPlotSet->setName(plotName);

   // Update the info bar
   const std::string& actionName = pPlotSet->getDisplayName(true);

   QMenu* pMenu = mpInfoBar->getMenu();
   if (pMenu != NULL)
   {
      QList<QAction*> menuActions = pMenu->actions();
      for (int i = 0; i < menuActions.count(); ++i)
      {
         QAction* pAction = menuActions[i];
         if (pAction != NULL)
         {
            PlotSet* pCurrentPlotSet = pAction->data().value<PlotSet*>();
            if (pCurrentPlotSet == pPlotSet)
            {
               pAction->setStatusTip(QString::fromStdString(pPlotSet->getName()));
               pAction->setText(QString::fromStdString(actionName));
               break;
            }
         }
      }
   }

   if (getCurrentPlotSet() == pPlotSet)
   {
      mpInfoBar->setTitle(QString::fromStdString(actionName));
   }

   return true;
}

bool PlotSetGroupImp::deletePlotSet(PlotSet* pPlotSet)
{
   if (pPlotSet == NULL)
   {
      return false;
   }

   // Get the name of the plot set that is being removed
   QString plotSetName = QString::fromStdString(pPlotSet->getName());
   QWidget* pWidget = pPlotSet->getWidget();

   // Delete the plot set
   std::map<PlotSet*, View*>::iterator iter = mPlotSets.find(pPlotSet);
   if (iter != mPlotSets.end())
   {
      VERIFYNR(pPlotSet->detach(SIGNAL_NAME(PlotSet, ViewAssociated),
         Slot(this, &PlotSetGroupImp::plotSetViewAssociated)));
      mPlotSets.erase(iter);
      mpStack->removeWidget(pWidget);
      emit plotSetDeleted(pPlotSet);
      notify(SIGNAL_NAME(PlotSetGroup, PlotSetDeleted), boost::any(pPlotSet));
      delete dynamic_cast<PlotSetImp*>(pPlotSet);
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
            PlotSet* pCurrentPlotSet = pAction->data().value<PlotSet*>();
            if (pCurrentPlotSet == pPlotSet)
            {
               pMenu->removeAction(pAction);
               break;
            }
         }
      }
   }

   return true;
}

std::vector<PlotWidget*> PlotSetGroupImp::getPlots(PlotType plotType) const
{
   std::vector<PlotWidget*> plots;
   for (std::map<PlotSet*, View*>::const_iterator iter = mPlotSets.begin(); iter != mPlotSets.end(); ++iter)
   {
      PlotSet* pPlotSet = iter->first;
      if (pPlotSet != NULL)
      {
         std::vector<PlotWidget*> currentPlots;
         pPlotSet->getPlots(currentPlots);

         for (std::vector<PlotWidget*>::const_iterator widgetIter = currentPlots.begin();
            widgetIter != currentPlots.end();
            ++widgetIter)
         {
            PlotWidget* pPlot = *widgetIter;
            if (pPlot != NULL)
            {
               PlotView* pPlotView = pPlot->getPlot();
               if (pPlotView != NULL)
               {
                  if (pPlotView->getPlotType() == plotType)
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

std::vector<PlotWidget*> PlotSetGroupImp::getPlots() const
{
   std::vector<PlotWidget*> plots;
   for (std::map<PlotSet*, View*>::const_iterator iter = mPlotSets.begin(); iter != mPlotSets.end(); ++iter)
   {
      PlotSet* pPlotSet = iter->first;
      if (pPlotSet != NULL)
      {
         std::vector<PlotWidget*> currentPlots;
         pPlotSet->getPlots(currentPlots);

         for (std::vector<PlotWidget*>::const_iterator widgetIter = currentPlots.begin();
            widgetIter != currentPlots.end();
            ++widgetIter)
         {
            PlotWidget* pPlot = *widgetIter;
            if (pPlot != NULL)
            {
               plots.push_back(pPlot);
            }
         }
      }
   }

   return plots;
}

unsigned int PlotSetGroupImp::getNumPlots() const
{
   std::vector<PlotWidget*> plots = getPlots();
   return plots.size();
}

bool PlotSetGroupImp::containsPlot(PlotWidget* pPlot) const
{
   if (pPlot == NULL)
   {
      return false;
   }

   std::vector<PlotWidget*> plots = getPlots();
   if (std::find(plots.begin(), plots.end(), pPlot) != plots.end())
   {
      return true;
   }

   return false;
}

bool PlotSetGroupImp::setCurrentPlot(PlotWidget* pPlot)
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

PlotWidget* PlotSetGroupImp::getCurrentPlot() const
{
   PlotSet* pPlotSet = getCurrentPlotSet();
   if (pPlotSet != NULL)
   {
      PlotWidget* pPlotWidget = pPlotSet->getCurrentPlot();
      return pPlotWidget;
   }

   return NULL;
}

void PlotSetGroupImp::clear()
{
   std::vector<PlotSet*> plotSets = getPlotSets();
   for (unsigned int i = 0; i < plotSets.size(); ++i)
   {
      PlotSet* pPlotSet = plotSets[i];
      if (pPlotSet != NULL)
      {
         deletePlotSet(pPlotSet);
      }
   }
}

void PlotSetGroupImp::setInfoBarIcon(const QIcon& icon)
{
   mpInfoBar->setInfoIcon(icon.pixmap(16));
}

void PlotSetGroupImp::setInfoBarElideMode(Qt::TextElideMode mode)
{
   mpInfoBar->setElideMode(mode);
}

void PlotSetGroupImp::plotSetViewAssociated(Subject& subject, const std::string& signal, const boost::any& value)
{
   PlotSet* pPlotSet = dynamic_cast<PlotSet*>(&subject);
   if (pPlotSet != NULL)
   {
      View* pView = boost::any_cast<View*>(value);
      if (pView != NULL)
      {
         mPlotSets[pPlotSet] = pView;
         pView->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &PlotSetGroupImp::plotSetViewDeleted));
      }
   }
}

void PlotSetGroupImp::plotSetViewDeleted(Subject& subject, const std::string& signal, const boost::any& value)
{
   View* pView = dynamic_cast<View*>(&subject);
   if (pView != NULL)
   {
      VERIFYNR(pView->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &PlotSetGroupImp::plotSetViewDeleted)));

      for (std::map<PlotSet*, View*>::iterator iter = mPlotSets.begin(); iter != mPlotSets.end(); ++iter)
      {
         PlotSet* pPlotSet = iter->first;
         if ((pPlotSet != NULL) && (iter->second == pView))
         {
            deletePlotSet(pPlotSet);
            return;
         }
      }
   }
}

void PlotSetGroupImp::setCurrentPlotSet(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   PlotSet* pPlotSet = pAction->data().value<PlotSet*>();
   if (pPlotSet != NULL)
   {
      setCurrentPlotSet(pPlotSet);
   }
}

namespace
{
   template<class T>
   void extractFromVariant(const DataVariant& var, std::vector<double>& data)
   {
      const std::vector<T>* pVec = var.getPointerToValue<std::vector<T> >();
      if (pVec != NULL)
      {
         data.reserve(pVec->size());
         copy(pVec->begin(), pVec->end(), back_inserter(data));
      }
   }
   void convertRawData(const DataVariant& var, std::vector<double>& data)
   {
      // list<types> typeList = char, unsigned char, short, ...
      // for each type in typeList
      //    data = extractFromVariant<type>(var);
      //    if (!data.empty) return data;
      extractFromVariant<char>(var, data);
      if (!data.empty())
      {
         return;
      }

      extractFromVariant<unsigned char>(var, data);
      if (!data.empty())
      {
         return;
      }

      extractFromVariant<short>(var, data);
      if (!data.empty())
      {
         return;
      }

      extractFromVariant<unsigned short>(var, data);
      if (!data.empty())
      {
         return;
      }

      extractFromVariant<int>(var, data);
      if (!data.empty())
      {
         return;
      }

      extractFromVariant<unsigned int>(var, data);
      if (!data.empty())
      {
         return;
      }

      extractFromVariant<long>(var, data);
      if (!data.empty())
      {
         return;
      }

      extractFromVariant<unsigned long>(var, data);
      if (!data.empty())
      {
         return;
      }

      extractFromVariant<Int64>(var, data);
      if (!data.empty())
      {
         return;
      }

      extractFromVariant<UInt64>(var, data);
      if (!data.empty())
      {
         return;
      }

      extractFromVariant<float>(var, data);
      if (!data.empty())
      {
         return;
      }

      extractFromVariant<double>(var, data);
   }
}

PlotWidget* PlotSetGroupImp::plotData(const DataVariant& xRawData, const DataVariant& yRawData,
                                      const std::string& xAttribute, const std::string& yAttribute,
                                      const std::string& plotName)
{
   std::vector<double> xData;
   convertRawData(xRawData, xData);
   std::vector<double> yData;
   convertRawData(yRawData, yData);
   if (xData.size() != yData.size() || xData.size() == 0 || yData.size() == 0)
   {
      std::string message = "The selected attributes are not plottable:\n" + yAttribute + " vs. " + xAttribute;
      QMessageBox::critical(this, "Unable to plot", message.c_str(), QMessageBox::Ok, 0);
      MessageResource msg("Plotting", "app", "7137567E-0EF3-4e46-9706-2F2F9764AD1C");
      msg->addProperty("Message", message);
      return NULL;
   }

   PlotSet* pSet = getCurrentPlotSet();
   if (pSet == NULL)
   {
      pSet = createPlotSet(QString::fromStdString(plotName));
      VERIFYRV(pSet != NULL, NULL);
   }

   PlotWidget* pPlotWidget = NULL;
   pPlotWidget = pSet->getPlot(plotName);

   if (pPlotWidget == NULL)
   {
      pPlotWidget = pSet->createPlot(plotName, CARTESIAN_PLOT);

      Axis* pAxis = pPlotWidget->getAxis(AXIS_BOTTOM);
      VERIFYRV(pAxis != NULL, NULL);
      pAxis->setTitle(xAttribute);
      pAxis = pPlotWidget->getAxis(AXIS_LEFT);
      VERIFYRV(pAxis != NULL, NULL);
      pAxis->setTitle(yAttribute);
   }
   else
   {
      Axis* pAxis = pPlotWidget->getAxis(AXIS_BOTTOM);
      VERIFYRV(pAxis != NULL, NULL);
      std::string xText = pAxis->getTitle();
      pAxis = pPlotWidget->getAxis(AXIS_LEFT);
      VERIFYRV(pAxis != NULL, NULL);
      std::string yText = pAxis->getTitle();
      if (xText != xAttribute || yText != yAttribute)
      {
         std::stringstream message;
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

   PlotView* pPlotView = pPlotWidget->getPlot();
   PointSet* pPointSet = dynamic_cast<PointSet*>(pPlotView->addObject(POINT_SET, true));
   for (unsigned int i = 0; i < xData.size(); ++i)
   {
      pPointSet->addPoint(xData[i], yData[i]);
   }

   return pPlotWidget;
}

PlotWidget* PlotSetGroupImp::plotData(const Signature& sig, const std::string& xAttribute,
                                      const std::string& yAttribute, const std::string& plotName)
{
   const DataVariant& xRawData = sig.getData(xAttribute);
   const DataVariant& yRawData = sig.getData(yAttribute);
   return plotData(xRawData, yRawData, xAttribute, yAttribute, plotName);
}

PlotWidget* PlotSetGroupImp::plotData(const DynamicObject& obj, const std::string& xAttribute,
                                      const std::string& yAttribute, const std::string& plotName)
{
   const DataVariant& xRawData = obj.getAttribute(xAttribute);
   const DataVariant& yRawData = obj.getAttribute(yAttribute);
   return plotData(xRawData, yRawData, xAttribute, yAttribute, plotName);
}

const std::string& PlotSetGroupImp::getObjectType() const
{
   static std::string sType("PlotSetGroupImp");
   return sType;
}

bool PlotSetGroupImp::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "PlotSetGroup"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

bool PlotSetGroupImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   if (mpInfoBar != NULL)
   {
      pXml->addAttr("infoBarTitle", mpInfoBar->getTitle().toStdString());
      pXml->addAttr("infoBarTitleColor", mpInfoBar->getTitleColor().name().toStdString());
      pXml->addAttr("infoBarTitleFont", mpInfoBar->getTitleFont().toString().toStdString());
   }

   PlotSet* pCurrentSet = getCurrentPlotSet();
   if (pCurrentSet != NULL)
   {
      pXml->addAttr("currentPlotSetId", pCurrentSet->getId());
   }

   PlotWidget* pPlot = getCurrentPlot();
   if (pPlot != NULL)
   {
      pXml->addAttr("currentPlotId", pPlot->getId());
   }

   if (mPlotSets.empty() == false)
   {
      pXml->pushAddPoint(pXml->addElement("PlotSets"));
      for (std::map<PlotSet*, View*>::const_iterator iter = mPlotSets.begin(); iter != mPlotSets.end(); ++iter)
      {
         PlotSet* pPlotSet = iter->first;
         if (pPlotSet != NULL)
         {
            pXml->pushAddPoint(pXml->addElement("PlotSet"));
            pXml->addAttr("plotSetId", pPlotSet->getId());

            View* pView = iter->second;
            if (pView != NULL)
            {
               pXml->addAttr("viewId", pView->getId());
            }

            pXml->popAddPoint();
         }
      }

      pXml->popAddPoint();
   }

   return true;
}

bool PlotSetGroupImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   // Delete existing plots
   clear();

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   if (pElem == NULL)
   {
      return false;
   }

   // Info bar
   if (mpInfoBar != NULL)
   {
      std::string infoBarTitle = A(pElem->getAttribute(X("infoBarTitle")));
      if (infoBarTitle.empty() == false)
      {
         mpInfoBar->setTitle(QString::fromStdString(infoBarTitle));
      }

      std::string infoBarTitleColor = A(pElem->getAttribute(X("infoBarTitleColor")));
      if (infoBarTitleColor.empty() == false)
      {
         QColor color(QString::fromStdString(infoBarTitleColor));
         mpInfoBar->setTitleColor(color);
      }

      std::string infoBarTitleFont = A(pElem->getAttribute(X("infoBarTitleFont")));
      if (infoBarTitleFont.empty() == false)
      {
         QFont font;
         if (font.fromString(QString::fromStdString(infoBarTitleFont)) == true)
         {
            mpInfoBar->setTitleFont(font);
         }
      }
   }

   // Current plot set and plot
   std::string currentPlotSetId = A(pElem->getAttribute(X("currentPlotSetId")));
   std::string currentPlotId = A(pElem->getAttribute(X("currentPlotId")));

   // Plot sets
   for (DOMNode* pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      std::string name = A(pChld->getNodeName());
      if (name == "PlotSets")
      {
         for (DOMNode* pGChld = pChld->getFirstChild(); pGChld != NULL; pGChld = pGChld->getNextSibling())
         {
            std::string name = A(pGChld->getNodeName());
            if (name == "PlotSet")
            {
               DOMElement* pPlotSetElement = static_cast<DOMElement*>(pGChld);

               std::string plotSetId = A(pPlotSetElement->getAttribute(X("plotSetId")));
               std::string viewId = A(pPlotSetElement->getAttribute(X("viewId")));

               PlotSet* pPlotSet = createPlotSet(QString::fromStdString(plotSetId));
               if (pPlotSet == NULL)
               {
                  return false;
               }

               View* pView = dynamic_cast<View*>(Service<SessionManager>()->getSessionItem(viewId));
               if (pView != NULL)
               {
                  mPlotSets[pPlotSet] = pView;
                  pView->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &PlotSetGroupImp::plotSetViewDeleted));
               }
            }
         }

         // Set the current plot set
         PlotSet* pPlotSet = dynamic_cast<PlotSet*>(Service<SessionManager>()->getSessionItem(currentPlotSetId));
         if ((pPlotSet != NULL) && (containsPlotSet(pPlotSet) == true))
         {
            setCurrentPlotSet(pPlotSet);
         }
      }
   }

   // Set the current plot
   setCurrentPlot(dynamic_cast<PlotWidget*>(Service<SessionManager>()->getSessionItem(currentPlotId)));
   return true;
}
