/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QAction>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>

#include "AppVerify.h"
#include "ContextMenu.h"
#include "ContextMenuActions.h"
#include "DesktopServices.h"
#include "DockWindow.h"
#include "PlotSet.h"
#include "PlotSetGroup.h"
#include "PlotSetImp.h"
#include "PlotView.h"
#include "PlotViewImp.h"
#include "PlotWidgetAdapter.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "xmlreader.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

PlotSetImp::PlotSetImp(const string& id, const string& plotSetName, QWidget* pParent) :
   QTabWidget(pParent),
   SessionItemImp(id, plotSetName),
   mpExplorer(Service<SessionExplorer>().get(), SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu),
      Slot(this, &PlotSetImp::updateContextMenu)),
   mpAssociatedView(SIGNAL_NAME(View, Renamed), Slot(this, &PlotSetImp::viewRenamed))
{
   // Initialization
   setTabPosition(QTabWidget::South);
   setTabShape(QTabWidget::Triangular);

   // Connections
   VERIFYNR(connect(this, SIGNAL(currentChanged(int)), this, SLOT(activatePlot(int))));
   VERIFYNR(Service<DesktopServices>()->attach(SIGNAL_NAME(DesktopServices, AboutToShowContextMenu),
      Slot(this, &PlotSetImp::updateContextMenu)));
}

PlotSetImp::~PlotSetImp()
{
   // Detach from the view
   setAssociatedView(NULL);

   VERIFYNR(disconnect(this, SIGNAL(currentChanged(int)), this, SLOT(activatePlot(int))));
   VERIFYNR(Service<DesktopServices>()->detach(SIGNAL_NAME(DesktopServices, AboutToShowContextMenu),
      Slot(this, &PlotSetImp::updateContextMenu)));

   clear();
}

const string& PlotSetImp::getObjectType() const
{
   static string type("PlotSetImp");
   return type;
}

bool PlotSetImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlotSet"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void PlotSetImp::updateContextMenu(Subject& subject, const string& signal, const boost::any& value)
{
   ContextMenu* pMenu = boost::any_cast<ContextMenu*>(value);
   if (pMenu == NULL)
   {
      return;
   }

   QObject* pParent = pMenu->getActionParent();
   vector<SessionItem*> items = pMenu->getSessionItems();

   if (items.empty() == true)
   {
      return;
   }

   bool bAddSeparator = false;
   bool bAddActivate = false;
   bool bAddDelete = false;
   string afterId;

   vector<DockWindow*> windowItems = pMenu->getSessionItems<DockWindow>();
   if (windowItems.size() == 1)
   {
      DockWindow* pDockWindow = windowItems.front();
      if (pDockWindow != NULL)
      {
         PlotSetGroup* pPlotSetGroup = dynamic_cast<PlotSetGroup*>(pDockWindow->getWidget());
         if (pPlotSetGroup != NULL)
         {
            if (dynamic_cast<PlotSetImp*>(pPlotSetGroup->getCurrentPlotSet()) == this)
            {
               bAddSeparator = true;
               bAddDelete = true;
            }
         }
      }
   }
   else if (dynamic_cast<SessionExplorer*>(&subject) != NULL)
   {
      // Make sure all of the selected session items for the menu are plot widgets
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
            if (containsPlot(pPlot) == false)
            {
               return;
            }
         }
      }

      // Check for only one selected plot widget item
      if (plots.size() == 1)
      {
         bAddSeparator = true;

         // Add an activate action if the selected plot widget is not currently active
         PlotWidget* pPlot = plots.front();
         if (pPlot != NULL)
         {
            if (pPlot != getCurrentPlot())
            {
               bAddActivate = true;
            }
         }
      }

      bAddDelete = true;
      afterId = APP_PLOTWIDGET_PRINT_ACTION;
   }

   // Separator
   if (bAddSeparator == true)
   {
      QAction* pSeparatorAction = new QAction(pParent);
      pSeparatorAction->setSeparator(true);
      pMenu->addActionAfter(pSeparatorAction, APP_PLOTSET_SEPARATOR_ACTION, afterId);

      if (afterId.empty() == false)
      {
         afterId = APP_PLOTSET_SEPARATOR_ACTION;
      }
   }

   // Activate
   if (bAddActivate == true)
   {
      QAction* pActivateAction = new QAction("&Activate", pParent);
      pActivateAction->setAutoRepeat(false);
      pActivateAction->setStatusTip("Activates the selected plot in the plot set");
      VERIFYNR(connect(pActivateAction, SIGNAL(triggered()), this, SLOT(activateSelectedPlot())));
      pMenu->addActionAfter(pActivateAction, APP_PLOTSET_ACTIVATE_ACTION, afterId);

      if (afterId.empty() == false)
      {
         afterId = APP_PLOTSET_ACTIVATE_ACTION;
      }
   }

   // Delete
   if (bAddDelete == true)
   {
      QAction* pDeleteAction = new QAction(QIcon(":/icons/Delete"), "&Delete", pParent);
      pDeleteAction->setAutoRepeat(false);
      VERIFYNR(connect(pDeleteAction, SIGNAL(triggered()), this, SLOT(destroyCurrentPlot())));
      pMenu->addActionAfter(pDeleteAction, APP_PLOTSET_DELETE_ACTION, afterId);
   }
}

void PlotSetImp::setName(const string& name)
{
   if (name.empty() == true)
   {
      return;
   }

   if (name != getName())
   {
      SessionItemImp::setName(name);
      emit renamed(QString::fromStdString(name));
      notify(SIGNAL_NAME(PlotSet, Renamed), boost::any(name));
   }
}

QWidget* PlotSetImp::getWidget()
{
   return this;
}

const QWidget* PlotSetImp::getWidget() const
{
   return this;
}

void PlotSetImp::setAssociatedView(View* pView)
{
   if (pView == mpAssociatedView.get())
   {
      return;
   }

   mpAssociatedView.reset(pView);
   notify(SIGNAL_NAME(PlotSet, ViewAssociated), boost::any(pView));

   if (mpAssociatedView.get() != NULL)
   {
      const string& viewName = mpAssociatedView->getName();
      if (viewName.empty() == false)
      {
         setName(viewName);
      }
   }
}

View* PlotSetImp::getAssociatedView()
{
   return mpAssociatedView.get();
}

const View* PlotSetImp::getAssociatedView() const
{
   return mpAssociatedView.get();
}

PlotWidget* PlotSetImp::createPlot(const QString& strPlotName, const PlotType& plotType)
{
   if (strPlotName.isEmpty() == true)
   {
      return NULL;
   }

   // Do not create the plot if a plot with the given name already exists
   PlotWidget* pPlot = getPlot(strPlotName);
   if (pPlot != NULL)
   {
      return NULL;
   }

   // Create the plot widget
   pPlot = new PlotWidgetAdapter(SessionItemImp::generateUniqueId(), strPlotName.toStdString(), plotType,
      dynamic_cast<PlotSet*>(this), this);
   if (pPlot != NULL)
   {
      // Block undo actions from being added to the plot view
      PlotViewImp* pPlotView = dynamic_cast<PlotViewImp*>(pPlot->getPlot());
      if (pPlotView != NULL)
      {
         pPlotView->blockUndo();
      }

      // Add the plot widget to the plot set
      addPlot(pPlot);
   }

   return pPlot;
}

PlotWidget* PlotSetImp::getPlot(const QString& strPlotName) const
{
   if (strPlotName.isEmpty() == true)
   {
      return NULL;
   }

   for (int i = 0; i < count(); i++)
   {
      PlotWidget* pPlotWidget = dynamic_cast<PlotWidget*>(widget(i));
      if (pPlotWidget != NULL)
      {
         QString strCurrentName = QString::fromStdString(pPlotWidget->getName());
         if (strCurrentName == strPlotName)
         {
            return pPlotWidget;
         }
      }
   }

   return NULL;
}

vector<PlotWidget*> PlotSetImp::getPlots(const PlotType& plotType) const
{
   vector<PlotWidget*> plots;
   for (int i = 0; i < count(); i++)
   {
      PlotWidget* pPlot = dynamic_cast<PlotWidget*>(widget(i));
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

   return plots;
}

vector<PlotWidget*> PlotSetImp::getPlots() const
{
   vector<PlotWidget*> plots;
   for (int i = 0; i < count(); i++)
   {
      PlotWidget* pPlot = dynamic_cast<PlotWidget*>(widget(i));
      if (pPlot != NULL)
      {
         plots.push_back(pPlot);
      }
   }

   return plots;
}

unsigned int PlotSetImp::getNumPlots() const
{
   return count();
}

bool PlotSetImp::containsPlot(PlotWidget* pPlot) const
{
   if (pPlot == NULL)
   {
      return false;
   }

   vector<PlotWidget*> plots = getPlots();
   for (unsigned int i = 0; i < plots.size(); i++)
   {
      PlotWidget* pCurrentPlot = NULL;
      pCurrentPlot = plots.at(i);
      if (pCurrentPlot == pPlot)
      {
         return true;
      }
   }

   return false;
}

PlotWidget* PlotSetImp::getCurrentPlot() const
{
   PlotWidget* pPlot = dynamic_cast<PlotWidget*>(currentWidget());
   return pPlot;
}

bool PlotSetImp::renamePlot(PlotWidget* pPlot, const QString& strNewName)
{
   if ((pPlot == NULL) || (strNewName.isEmpty() == true))
   {
      return false;
   }

   // Check if the new name is the same as the current name
   string newName = strNewName.toStdString();
   string currentName = pPlot->getName();

   if (newName == currentName)
   {
      return false;
   }

   // Check if a plot already exists with the new name
   if (getPlot(strNewName) != NULL)
   {
      return false;
   }

   // Rename the plot
   PlotWidgetImp* pPlotWidgetImp = dynamic_cast<PlotWidgetImp*>(pPlot);
   if (pPlotWidgetImp != NULL)
   {
      pPlotWidgetImp->setName(newName);
   }

   return true;
}

QString PlotSetImp::renamePlot(PlotWidget* pPlot)
{
   QString strOldName;

   PlotView* pPlotView = pPlot->getPlot();
   if (pPlotView != NULL)
   {
      strOldName = QString::fromStdString(pPlotView->getName());
   }

   QString strNewName;
   while (strNewName.isEmpty() == true)
   {
      bool bSuccess = false;
      strNewName = QInputDialog::getText(this, QString::fromStdString(getName()), "Name:", QLineEdit::Normal,
         strOldName, &bSuccess);
      if (bSuccess == false)
      {
         return QString();
      }

      if (strNewName.isEmpty() == true)
      {
         QMessageBox::warning(this, QString::fromStdString(getName()),
            "The plot name is invalid.  Please enter a unique name.");
      }
      else
      {
         // Do not change the name to that of an existing plot
         if (getPlot(strNewName) == NULL)
         {
            bSuccess = renamePlot(pPlot, strNewName);
            if (bSuccess == false)
            {
               return QString();
            }
         }
         else
         {
            QMessageBox::warning(this, QString::fromStdString(getName()), "The '" + strNewName +
               "' plot name already exists.  Please enter a unique name.");
            strNewName.clear();
         }
      }
   }

   return strNewName;
}

bool PlotSetImp::deletePlot(PlotWidget* pPlot)
{
   if (pPlot == NULL)
   {
      return false;
   }

   if (containsPlot(pPlot) == false)
   {
      return false;
   }

   PlotWidgetImp* pPlotImp = dynamic_cast<PlotWidgetImp*>(pPlot);
   if (pPlotImp == NULL)
   {
      return false;
   }

   int tabIndex = indexOf(pPlotImp);
   if (tabIndex == -1)
   {
      return false;
   }

   removeTab(tabIndex);
   emit plotDeleted(pPlot);
   notify(SIGNAL_NAME(PlotSet, PlotDeleted), boost::any(pPlot));
   delete pPlotImp;
   return true;
}

bool PlotSetImp::setCurrentPlot(PlotWidget* pPlot)
{
   if (pPlot == NULL)
   {
      return false;
   }

   if (containsPlot(pPlot) == false)
   {
      return false;
   }

   setCurrentWidget(dynamic_cast<PlotWidgetImp*>(pPlot));
   emit plotActivated(pPlot);
   notify(SIGNAL_NAME(PlotSet, Activated), boost::any(pPlot));
   return true;
}

void PlotSetImp::renameCurrentPlot()
{
   PlotWidget* pPlot = getCurrentPlot();
   if (pPlot != NULL)
   {
      renamePlot(pPlot);
   }
}

void PlotSetImp::destroyCurrentPlot()
{
   PlotWidget* pPlot = getCurrentPlot();
   if (pPlot != NULL)
   {
      deletePlot(pPlot);
   }
}

void PlotSetImp::activateSelectedPlot()
{
   Service<SessionExplorer> pExplorer;

   vector<SessionItem*> selectedItems = pExplorer->getSelectedSessionItems();
   if (selectedItems.size() == 1)
   {
      PlotWidget* pPlot = dynamic_cast<PlotWidget*>(selectedItems.front());
      if (pPlot != NULL)
      {
         setCurrentPlot(pPlot);
      }
   }
}

void PlotSetImp::renameSelectedPlot()
{
   Service<SessionExplorer> pExplorer;

   vector<SessionItem*> selectedItems = pExplorer->getSelectedSessionItems();
   if (selectedItems.size() == 1)
   {
      PlotWidget* pPlot = dynamic_cast<PlotWidget*>(selectedItems.front());
      if (pPlot != NULL)
      {
         renamePlot(pPlot);
      }
   }
}

void PlotSetImp::destroySelectedPlots()
{
   // Get the selected plots
   Service<SessionExplorer> pExplorer;
   vector<PlotWidget*> selectedPlots = pExplorer->getSelectedSessionItems<PlotWidget>();

   // Destroy the selected plots
   vector<PlotWidget*>::iterator iter;
   for (iter = selectedPlots.begin(); iter != selectedPlots.end(); ++iter)
   {
      PlotWidget* pPlot = *iter;
      if (pPlot != NULL)
      {
         deletePlot(pPlot);
      }
   }
}

void PlotSetImp::clear()
{
   vector<PlotWidget*> plots = getPlots();
   for (unsigned int i = 0; i < plots.size(); i++)
   {
      PlotWidget* pPlot = NULL;
      pPlot = plots.at(i);
      if (pPlot != NULL)
      {
         deletePlot(pPlot);
      }
   }
}

void PlotSetImp::viewRenamed(Subject& subject, const string& signal, const boost::any& value)
{
   if (&subject == mpAssociatedView.get())
   {
      string viewName = boost::any_cast<string>(value);
      if (viewName.empty() == false)
      {
         setName(viewName);
      }
   }
}

void PlotSetImp::addPlot(PlotWidget* pPlot)
{
   if (pPlot == NULL)
   {
      return;
   }

   if (containsPlot(pPlot) == true)
   {
      return;
   }

   // Get the plot view
   PlotViewImp* pView = dynamic_cast<PlotViewImp*>(pPlot->getPlot());
   if (pView == NULL)
   {
      return;
   }

   // Get the plot name
   const string& plotName = pView->getDisplayName(true);
   if (plotName.empty() == true)
   {
      return;
   }

   // Add the new tab
   PlotWidgetImp* pPlotImp = dynamic_cast<PlotWidgetImp*>(pPlot);
   if (pPlotImp != NULL)
   {
      VERIFYNR(connect(pView, SIGNAL(renamed(const QString&)), this, SLOT(updatePlotName())));
      addTab(pPlotImp, QString::fromStdString(plotName));
      emit plotAdded(pPlot);
      notify(SIGNAL_NAME(PlotSet, PlotAdded), boost::any(pPlot));
      setCurrentPlot(pPlot);
   }
}

void PlotSetImp::updatePlotName()
{
   PlotViewImp* pPlot = dynamic_cast<PlotViewImp*>(sender());
   if (pPlot != NULL)
   {
      for (int i = 0; i < count(); ++i)
      {
         PlotWidgetImp* pPlotWidget = dynamic_cast<PlotWidgetImp*>(widget(i));
         if (pPlotWidget != NULL)
         {
            PlotViewImp* pCurrentPlot = dynamic_cast<PlotViewImp*>(pPlotWidget->getPlot());
            if (pCurrentPlot == pPlot)
            {
               const string& name = pPlot->getDisplayName(true);
               if (name.empty() == false)
               {
                  int index = indexOf(pPlotWidget);
                  if (index != -1)
                  {
                     setTabText(index, QString::fromStdString(name));
                  }
               }

               break;
            }
         }
      }
   }
}

void PlotSetImp::activatePlot(int iIndex)
{
   PlotWidget* pPlot = NULL;
   if (iIndex != -1)
   {
      pPlot = dynamic_cast<PlotWidget*>(widget(iIndex));
   }

   emit plotActivated(pPlot);
   notify(SIGNAL_NAME(PlotSet, Activated), boost::any(pPlot));
}

bool PlotSetImp::serialize(SessionItemSerializer &serializer) const
{
   XMLWriter xml(getObjectType().c_str());

   if (!toXml(&xml))
   {
      return false;
   }

   return serializer.serialize(xml);
}

bool PlotSetImp::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader xml(NULL, false);
   DOMNode* pRoot = deserializer.deserialize(xml, getObjectType().c_str());
   return fromXml(pRoot, XmlBase::VERSION);
}

bool PlotSetImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL || !SessionItemImp::toXml(pXml))
   {
      return false;
   }
   pXml->addAttr("type", getObjectType());

   if (mpAssociatedView.get() != NULL)
   {
      pXml->addAttr("associatedViewId", mpAssociatedView->getId());
   }

   vector<PlotWidget*> widgets = getPlots();
   for (vector<PlotWidget*>::const_iterator it = widgets.begin(); it != widgets.end(); ++it)
   {
      if (*it != NULL)
      {
         pXml->pushAddPoint(pXml->addElement("PlotWidget"));
         pXml->addAttr("id", (*it)->getId());
         pXml->popAddPoint();
      }
   }

   PlotWidget* pPlot = getCurrentPlot();
   if (pPlot != NULL)
   {
      pXml->addAttr("currentPlotId", pPlot->getId());
   }

   return true;
}

bool PlotSetImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !SessionItemImp::fromXml(pDocument, version))
   {
      return false;
   }
   DOMElement* pElem = static_cast<DOMElement*>(pDocument);

   if (pElem->hasAttribute(X("associatedViewId")))
   {
      View* pView = dynamic_cast<View*>(
         Service<SessionManager>()->getSessionItem(A(pElem->getAttribute(X("associatedViewId")))));
      setAssociatedView(pView);
   }

   for (DOMNode *pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("PlotWidget")))
      {
         PlotWidget* pWidget = dynamic_cast<PlotWidget*>(
            Service<SessionManager>()->getSessionItem(A(static_cast<DOMElement*>(pChld)->getAttribute(X("id")))));
         if (pWidget != NULL)
         {
            addPlot(pWidget);
         }
      }
   }

   string currentPlotId = A(pElem->getAttribute(X("currentPlotId")));

   PlotWidget* pPlot = dynamic_cast<PlotWidget*>(Service<SessionManager>()->getSessionItem(currentPlotId));
   if (pPlot != NULL)
   {
      setCurrentPlot(pPlot);
   }

   return true;
}
