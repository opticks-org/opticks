/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDataStream>
#include <QtCore/QMimeData>
#include <QtWidgets/QWidget>

#include "AppVerify.h"
#include "ClassificationLayer.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "DockWindow.h"
#include "GraphicElement.h"
#include "GraphicGroup.h"
#include "GraphicGroupImp.h"
#include "GraphicLayer.h"
#include "Layer.h"
#include "LayerList.h"
#include "PlotSet.h"
#include "PlotSetGroup.h"
#include "PlotWidget.h"
#include "ProductView.h"
#include "Slot.h"
#include "SessionManagerImp.h"
#include "SpatialDataView.h"
#include "ToolBar.h"
#include "ViewObject.h"
#include "WindowModel.h"
#include "WorkspaceWindow.h"

using namespace std;

/////////////////
// WindowModel //
/////////////////

WindowModel::WindowModel(QObject* pParent) :
   QSortFilterProxyModel(pParent)
{
   setSourceModel(new WindowSourceModel(this));
   setDynamicSortFilter(true);
}

WindowModel::~WindowModel()
{}

Qt::DropActions WindowModel::supportedDropActions() const
{
   return Qt::MoveAction;
}

bool WindowModel::dropMimeData(const QMimeData* pData, Qt::DropAction action, int row, int column,
                               const QModelIndex& dropIndex)
{
   if ((pData == NULL) || (dropIndex.isValid() == false))
   {
      return false;
   }

   if (pData->hasFormat("application/x-sessionitem-id") == false)
   {
      return false;
   }

   // Get the dragged layer
   Layer* pDropLayer = NULL;

   QByteArray itemIdArray = pData->data("application/x-sessionitem-id");
   QDataStream itemIdStream(&itemIdArray, QIODevice::ReadOnly);

   while (itemIdStream.atEnd() == false)
   {
      QString itemId;
      itemIdStream >> itemId;
      VERIFY(itemId.isEmpty() == false);

      // Can only drop a single layer
      if (pDropLayer != NULL)
      {
         return false;
      }

      int numLayers = rowCount(dropIndex);
      for (int i = 0; i < numLayers; ++i)
      {
         QModelIndex layerIndex = dropIndex.child(i, 0);
         if (layerIndex.isValid() == true)
         {
            Layer* pLayer =
               dynamic_cast<Layer*>(layerIndex.data(SessionItemModel::SessionItemRole).value<SessionItem*>());
            if (pLayer != NULL)
            {
               if (QString::fromStdString(pLayer->getId()) == itemId)
               {
                  pDropLayer = pLayer;
                  break;
               }
            }
         }
      }
   }

   VERIFY(pDropLayer != NULL);

   // Get the view
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pDropLayer->getView());
   VERIFY(pView != NULL);

   // If the user is moving the layer down in the order, subtract one from the new
   // display index to account for the position that the layer currently occupies
   int displayIndex = pView->getLayerDisplayIndex(pDropLayer);
   if (row > displayIndex)
   {
      row--;
   }

   // Update the layer's display index
   bool success = pView->setLayerDisplayIndex(pDropLayer, row);
   if (success == true)
   {
      pView->refresh();
   }

   return success;
}

bool WindowModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
   if ((left.isValid() == true) && (right.isValid() == true))
   {
      // Check for top level items
      if ((left.parent().isValid() == false) && (right.parent().isValid() == false))
      {
         QString strLeft = sourceModel()->data(left).toString();
         QString strRight = sourceModel()->data(right).toString();

         if ((strLeft == "Workspace Windows") || ((strLeft == "Dock Windows") && (strRight == "Toolbars")))
         {
            return true;
         }

         return false;
      }

      // Check for layer items
      SessionItem* pSessionItem = left.data(SessionItemModel::SessionItemRole).value<SessionItem*>();
      Layer* pLeftLayer = dynamic_cast<Layer*>(pSessionItem);
      pSessionItem = right.data(SessionItemModel::SessionItemRole).value<SessionItem*>();
      Layer* pRightLayer = dynamic_cast<Layer*>(pSessionItem);

      if ((pLeftLayer != NULL) && (pRightLayer != NULL))
      {
         SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLeftLayer->getView());
         if (pView != NULL)
         {
            if (pRightLayer->getView() == pView)
            {
               int leftIndex = pView->getLayerDisplayIndex(pLeftLayer);
               int rightIndex = pView->getLayerDisplayIndex(pRightLayer);

               return (leftIndex < rightIndex);
            }
         }
      }
   }

   return QSortFilterProxyModel::lessThan(left, right);
}

///////////////////////
// WindowSourceModel //
///////////////////////

WindowModel::WindowSourceModel::WindowSourceModel(QObject* pParent) :
   SessionItemModel(pParent),
   mpWorkspaceWindowWrapper(NULL),
   mpDockWindowWrapper(NULL),
   mpToolBarWrapper(NULL)
{
   // Connections
   Service<DesktopServices> pDesktop;
   refreshModel(*pDesktop.get(), "", boost::any());
   SessionManagerImp::instance()->attach(SIGNAL_NAME(SessionManager, SessionRestored),
      Slot(this, &WindowSourceModel::refreshModel));
   SessionManagerImp::instance()->attach(SIGNAL_NAME(SessionManager, AboutToRestore),
      Slot(this, &WindowSourceModel::detachModel));
}

void WindowModel::WindowSourceModel::detachModel(Subject &subject, const string &signal, const boost::any &v)
{
   Q_UNUSED(subject);
   Q_UNUSED(signal);
   Q_UNUSED(v);

   Service<DesktopServices> pDesktop;
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &WindowSourceModel::addWindow));
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowRemoved), Slot(this, &WindowSourceModel::removeWindow));
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowActivated), Slot(this, &WindowSourceModel::setCurrentWindow));
}

void WindowModel::WindowSourceModel::refreshModel(Subject &subject, const string &signal, const boost::any &v)
{
   Q_UNUSED(subject);
   Q_UNUSED(signal);
   Q_UNUSED(v);

   Service<DesktopServices> pDesktop;
   pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &WindowSourceModel::addWindow));
   pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowRemoved), Slot(this, &WindowSourceModel::removeWindow));
   pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowActivated), Slot(this, &WindowSourceModel::setCurrentWindow));

   clear();

   // Add top level items
   SessionItemWrapper* pRootWrapper = getRootWrapper();
   if (pRootWrapper != NULL)
   {
      mpWorkspaceWindowWrapper = new SessionItemWrapper(this, NULL, pRootWrapper);
      if (mpWorkspaceWindowWrapper != NULL)
      {
         mpWorkspaceWindowWrapper->setDisplayName("Workspace Windows");
         pRootWrapper->addChild(mpWorkspaceWindowWrapper);
      }

      mpDockWindowWrapper = new SessionItemWrapper(this, NULL, pRootWrapper);
      if (mpDockWindowWrapper != NULL)
      {
         mpDockWindowWrapper->setDisplayName("Dock Windows");
         pRootWrapper->addChild(mpDockWindowWrapper);
      }

      mpToolBarWrapper = new SessionItemWrapper(this, NULL, pRootWrapper);
      if (mpToolBarWrapper != NULL)
      {
         mpToolBarWrapper->setDisplayName("Toolbars");
         pRootWrapper->addChild(mpToolBarWrapper);
      }
   }

   // Initialize the model
   vector<Window*> windows;
   pDesktop->getWindows(windows);
   for (vector<DataElement*>::size_type i = 0; i < windows.size(); ++i)
   {
      Window* pWindow = windows[i];
      if (pWindow != NULL)
      {
         addWindowItem(pWindow);
         WorkspaceWindow* pWorkWin = dynamic_cast<WorkspaceWindow*>(pWindow);
         if (pWorkWin != NULL && pDesktop->getCurrentWorkspaceWindow() == pWorkWin)
         {
            activateItem(pWindow);
         }
      }
   }
}

WindowModel::WindowSourceModel::~WindowSourceModel()
{
   // Detach from desktop services
   Service<DesktopServices> pDesktop;
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &WindowSourceModel::addWindow));
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowRemoved), Slot(this, &WindowSourceModel::removeWindow));
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowActivated), Slot(this, &WindowSourceModel::setCurrentWindow));
}

Qt::ItemFlags WindowModel::WindowSourceModel::flags(const QModelIndex& index) const
{
   Qt::ItemFlags itemFlags = SessionItemModel::flags(index);
   if (index.isValid() == true)
   {
      QModelIndex parentIndex = index.parent();
      if (parentIndex.isValid() == true)
      {
         itemFlags |= Qt::ItemIsDragEnabled;
      }

      Layer* pLayer = dynamic_cast<Layer*>(index.data(SessionItemModel::SessionItemRole).value<SessionItem*>());
      if (pLayer != NULL)
      {
         SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
         if (pView != NULL)
         {
            itemFlags |= Qt::ItemIsUserCheckable;
         }
      }
      else if (rowCount(index) > 0)
      {
         QModelIndex childIndex = index.child(0, 0);
         if (childIndex.isValid() == true)
         {
            SessionItem* pItem = childIndex.data(SessionItemModel::SessionItemRole).value<SessionItem*>();
            if (dynamic_cast<Layer*>(pItem) != NULL)
            {
               itemFlags |= Qt::ItemIsDropEnabled;
            }
         }
      }

      ToolBar* pToolbar = dynamic_cast<ToolBar*>(index.data(SessionItemModel::SessionItemRole).value<SessionItem*>());
      if (pToolbar != NULL)
      {
         itemFlags |= Qt::ItemIsUserCheckable;
      }

      DockWindow* pDock = dynamic_cast<DockWindow*>(index.data(SessionItemModel::SessionItemRole).value<SessionItem*>());
      if (pDock != NULL)
      {
         itemFlags |= Qt::ItemIsUserCheckable;
      }
   }

   return itemFlags;
}

bool WindowModel::WindowSourceModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
   if (index.isValid() == false)
   {
      return false;
   }

   if (SessionItemModel::setData(index, value, role) == true)
   {
      return true;
   }

   SessionItemWrapper* pWrapper = reinterpret_cast<SessionItemWrapper*>(index.internalPointer());
   if (pWrapper == NULL)
   {
      return false;
   }

   if (role == Qt::CheckStateRole)
   {
      bool checkboxClicked = false;
      Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());

      Layer* pLayer = dynamic_cast<Layer*>(pWrapper->getSessionItem());
      if (pLayer != NULL)
      {
         SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
         if (pView == NULL)
         {
            return false;
         }

         checkboxClicked = true;
         if (checkState == Qt::Checked)
         {
            pView->showLayer(pLayer);
         }
         else
         {
            pView->hideLayer(pLayer);
         }

         pView->refresh();
      }

      ToolBar* pToolbar = dynamic_cast<ToolBar*>(pWrapper->getSessionItem());
      if (pToolbar != NULL)
      {
         checkboxClicked = true;
         if (checkState == Qt::Checked)
         {
            pToolbar->show();
         }
         else
         {
            pToolbar->hide();
         }
      }

      DockWindow* pDock = dynamic_cast<DockWindow*>(pWrapper->getSessionItem());
      if (pDock != NULL)
      {
         checkboxClicked = true;
         if (checkState == Qt::Checked)
         {
            pDock->show();
         }
         else
         {
            pDock->hide();
         }
      }

      if (checkboxClicked == false)
      {
         return false;
      }
   }
   else
   {
      return false;
   }

   emit dataChanged(index, index);
   return true;
}

void WindowModel::WindowSourceModel::addWindow(Subject& subject, const string& signal, const boost::any& value)
{
   Window* pWindow = boost::any_cast<Window*>(value);
   if (pWindow != NULL)
   {
      addWindowItem(pWindow);
   }
}

void WindowModel::WindowSourceModel::removeWindow(Subject& subject, const string& signal, const boost::any& value)
{
   Window* pWindow = boost::any_cast<Window*>(value);
   if (pWindow != NULL)
   {
      removeWindowItem(pWindow);
   }
}

void WindowModel::WindowSourceModel::setCurrentWindow(Subject& subject, const string& signal, const boost::any& value)
{
   WorkspaceWindow* pWindow = boost::any_cast<WorkspaceWindow*>(value);
   if (pWindow != NULL)
   {
      activateItem(pWindow);
   }
}

void WindowModel::WindowSourceModel::addView(Subject& subject, const string& signal, const boost::any& value)
{
   View* pView = boost::any_cast<View*>(value);
   if (pView != NULL)
   {
      ViewObject* pViewObject = dynamic_cast<ViewObject*>(&subject);
      if (pViewObject != NULL)
      {
         SessionItemWrapper* pViewObjectWrapper = getWrapper(pViewObject);
         if (pViewObjectWrapper != NULL)
         {
            addViewItem(pViewObjectWrapper, pView);
         }
      }
   }
}

void WindowModel::WindowSourceModel::removeView(Subject& subject, const string& signal, const boost::any& value)
{
   View* pView = boost::any_cast<View*>(value);
   if (pView != NULL)
   {
      ViewObject* pViewObject = dynamic_cast<ViewObject*>(&subject);
      if (pViewObject != NULL)
      {
         SessionItemWrapper* pViewObjectWrapper = getWrapper(pViewObject);
         if (pViewObjectWrapper != NULL)
         {
            removeViewItem(pViewObjectWrapper, pView);
         }
      }
   }
}

void WindowModel::WindowSourceModel::addLayer(Subject& subject, const string& signal, const boost::any& value)
{
   Layer* pLayer = boost::any_cast<Layer*>(value);
   if (pLayer != NULL)
   {
      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         SessionItemWrapper* pParentLayerWrapper = getWrapper(pView);
         if (pParentLayerWrapper == NULL)
         {
            QWidget* pViewWidget = pView->getWidget();
            if (pViewWidget != NULL)
            {
               Window* pWindow = dynamic_cast<Window*>(pViewWidget->parentWidget());
               if (pWindow != NULL)
               {
                  pParentLayerWrapper = getWrapper(pWindow);
               }
            }
         }

         if (pParentLayerWrapper != NULL)
         {
            addLayerItem(pParentLayerWrapper, pLayer);
         }
      }
   }
}

void WindowModel::WindowSourceModel::removeLayer(Subject& subject, const string& signal, const boost::any& value)
{
   Layer* pLayer = boost::any_cast<Layer*>(value);
   if (pLayer != NULL)
   {
      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         SessionItemWrapper* pParentLayerWrapper = getWrapper(pView);
         if (pParentLayerWrapper == NULL)
         {
            QWidget* pViewWidget = pView->getWidget();
            if (pViewWidget != NULL)
            {
               Window* pWindow = dynamic_cast<Window*>(pViewWidget->parentWidget());
               if (pWindow != NULL)
               {
                  pParentLayerWrapper = getWrapper(pWindow);
               }
            }
         }

         if (pParentLayerWrapper != NULL)
         {
            removeLayerItem(pParentLayerWrapper, pLayer);
         }
      }
   }
}

void WindowModel::WindowSourceModel::updateLayerOrder(Subject& subject, const string& signal, const boost::any& value)
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(&subject);
   if (pView == NULL)
   {
      return;
   }

   // Get the index from one of the layers in the view
   LayerList* pLayerList = pView->getLayerList();
   VERIFYNRV(pLayerList != NULL);

   vector<Layer*> layers;
   pLayerList->getLayers(layers);
   if (layers.empty() == true)
   {
      return;
   }

   QModelIndex layerIndex = index(layers.front());
   VERIFYNRV(layerIndex.isValid() == true);

   // Get the index of the first and last layer items
   QModelIndex parentIndex = layerIndex.parent();
   VERIFYNRV(parentIndex.isValid() == true);

   int numLayers = rowCount(parentIndex);
   QModelIndex beginIndex = parentIndex.child(0, 0);
   QModelIndex endIndex = parentIndex.child(numLayers - 1, 0);

   // Notify of data change for all layers in the view
   emit dataChanged(beginIndex, endIndex);
}

void WindowModel::WindowSourceModel::updateLayerDisplay(Subject& subject, const string& signal, const boost::any& value)
{
   Layer* pLayer = boost::any_cast<Layer*>(value);
   if (pLayer != NULL)
   {
      SessionItemWrapper* pWrapper = getWrapper(pLayer);
      if (pWrapper != NULL)
      {
         Qt::CheckState checkState = Qt::Checked;
         if (signal == "SpatialDataView::LayerHidden")
         {
            checkState = Qt::Unchecked;
         }

         pWrapper->setCheckState(checkState);

         QModelIndex layerIndex = index(pLayer);
         emit dataChanged(layerIndex, layerIndex);
      }
   }
}

void WindowModel::WindowSourceModel::updateToolbarDisplay(Subject& subject, const string& signal,
                                                          const boost::any& value)
{
   ToolBar* pToolbar = dynamic_cast<ToolBar*>(&subject);
   if (pToolbar != NULL)
   {
      SessionItemWrapper* pWrapper = getWrapper(pToolbar);
      if (pWrapper != NULL)
      {
         Qt::CheckState checkState = Qt::Checked;
         if (signal == "ToolBar::Hidden")
         {
            checkState = Qt::Unchecked;
         }

         pWrapper->setCheckState(checkState);

         QModelIndex toolbarIndex = index(pToolbar);
         emit dataChanged(toolbarIndex, toolbarIndex);
      }
   }
}

void WindowModel::WindowSourceModel::updateDockDisplay(Subject& subject, const string& signal, const boost::any& value)
{
   DockWindow* pPlot = dynamic_cast<DockWindow*>(&subject);
   if (pPlot != NULL)
   {
      SessionItemWrapper* pWrapper = getWrapper(pPlot);
      if (pWrapper != NULL)
      {
         Qt::CheckState checkState = Qt::Checked;
         if (signal == "DockWindow::Hidden")
         {
            checkState = Qt::Unchecked;
         }

         pWrapper->setCheckState(checkState);

         QModelIndex plotIndex = index(pPlot);
         emit dataChanged(plotIndex, plotIndex);
      }
   }
}

void WindowModel::WindowSourceModel::activateLayer(Subject& subject, const string& signal, const boost::any& value)
{
   Layer* pLayer = boost::any_cast<Layer*>(value);
   activateItem(pLayer);
}

void WindowModel::WindowSourceModel::addGraphicObject(Subject& subject, const string& signal, const boost::any& value)
{
   GraphicGroup* pGroup = dynamic_cast<GraphicGroup*>(&subject);
   if (pGroup == NULL)
   {
      return;
   }

   GraphicObject* pObject = boost::any_cast<GraphicObject*>(value);
   if (pObject == NULL)
   {
      return;
   }

   // Get the parent group's wrapper in which to add the item
   SessionItemWrapper* pWrapper = getWrapper(pGroup);
   if (pWrapper == NULL)
   {
      // The parent group's wrapper is not available so the object is added to the layer's wrapper
      GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*>(pObject);
      if (pObjectImp != NULL)
      {
         GraphicLayer* pLayer = pObjectImp->getLayer();
         if (pLayer != NULL)
         {
            pWrapper = getWrapper(pLayer);
         }
      }
   }

   // Add the graphic object item
   if (pWrapper != NULL)
   {
      addGraphicObjectItem(pWrapper, pObject);
   }
}

void WindowModel::WindowSourceModel::removeGraphicObject(Subject& subject, const string& signal,
                                                         const boost::any& value)
{
   GraphicGroup* pGroup = dynamic_cast<GraphicGroup*>(&subject);
   if (pGroup == NULL)
   {
      return;
   }

   GraphicObject* pObject = boost::any_cast<GraphicObject*>(value);
   if (pObject == NULL)
   {
      return;
   }

   // Get the parent group's wrapper from which to remove the item
   SessionItemWrapper* pWrapper = getWrapper(pGroup);
   if (pWrapper == NULL)
   {
      // The parent group's wrapper is not available so the object is removed from the layer's wrapper
      GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*>(pObject);
      if (pObjectImp != NULL)
      {
         GraphicLayer* pLayer = pObjectImp->getLayer();
         if (pLayer != NULL)
         {
            pWrapper = getWrapper(pLayer);
         }
      }
   }

   // Remove the graphic object item
   if (pWrapper != NULL)
   {
      removeGraphicObjectItem(pWrapper, pObject);
   }
}

void WindowModel::WindowSourceModel::addPlotSets(Subject& subject, const string& signal, const boost::any& value)
{
   DockWindow* pDockWindow = dynamic_cast<DockWindow*>(&subject);
   if (pDockWindow == NULL)
   {
      return;
   }

   PlotSetGroup* pPlotSetGroup = dynamic_cast<PlotSetGroup*>(pDockWindow->getWidget());
   if (pPlotSetGroup != NULL)
   {
      SessionItemWrapper* pDockWindowWrapper = getWrapper(pDockWindow);
      if (pDockWindowWrapper != NULL)
      {
         // Add plot set items
         const vector<PlotSet*>& plotSets = pPlotSetGroup->getPlotSets();
         for (vector<PlotSet*>::const_iterator iter = plotSets.begin(); iter != plotSets.end(); ++iter)
         {
            PlotSet* pPlotSet = *iter;
            if (pPlotSet != NULL)
            {
               addPlotSetItem(pDockWindowWrapper, pPlotSet);
               if (pPlotSetGroup->getCurrentPlotSet() == pPlotSet)
               {
                  activateItem(pPlotSet);
               }
            }
         }

         // Connections
         pPlotSetGroup->attach(SIGNAL_NAME(PlotSetGroup, PlotSetAdded), Slot(this, &WindowSourceModel::addPlotSet));
         pPlotSetGroup->attach(SIGNAL_NAME(PlotSetGroup, PlotSetDeleted),
            Slot(this, &WindowSourceModel::removePlotSet));
         pPlotSetGroup->attach(SIGNAL_NAME(PlotSetGroup, PlotSetActivated),
            Slot(this, &WindowSourceModel::setCurrentPlotSet));
      }
   }
}

void WindowModel::WindowSourceModel::removePlotSets(Subject& subject, const string& signal, const boost::any& value)
{
   DockWindow* pDockWindow = dynamic_cast<DockWindow*>(&subject);
   if (pDockWindow == NULL)
   {
      return;
   }

   PlotSetGroup* pPlotSetGroup = dynamic_cast<PlotSetGroup*>(pDockWindow->getWidget());
   if (pPlotSetGroup != NULL)
   {
      SessionItemWrapper* pDockWindowWrapper = getWrapper(pDockWindow);
      if (pDockWindowWrapper != NULL)
      {
         // Detach the plot window
         pPlotSetGroup->detach(SIGNAL_NAME(PlotSetGroup, PlotSetAdded), Slot(this, &WindowSourceModel::addPlotSet));
         pPlotSetGroup->detach(SIGNAL_NAME(PlotSetGroup, PlotSetDeleted),
            Slot(this, &WindowSourceModel::removePlotSet));
         pPlotSetGroup->detach(SIGNAL_NAME(PlotSetGroup, PlotSetActivated),
            Slot(this, &WindowSourceModel::setCurrentPlotSet));

         // Remove the plot set items
         const vector<PlotSet*>& plotSets = pPlotSetGroup->getPlotSets();
         for (vector<PlotSet*>::const_iterator iter = plotSets.begin(); iter != plotSets.end(); ++iter)
         {
            PlotSet* pPlotSet = *iter;
            if (pPlotSet != NULL)
            {
               removePlotSetItem(pDockWindowWrapper, pPlotSet);
            }
         }
      }
   }
}

void WindowModel::WindowSourceModel::addPlotSet(Subject& subject, const string& signal, const boost::any& value)
{
   PlotSetGroup* pPlotSetGroup = dynamic_cast<PlotSetGroup*>(&subject);
   if (pPlotSetGroup == NULL)
   {
      return;
   }

   QWidget* pWidget = pPlotSetGroup->getWidget();
   if (pWidget == NULL)
   {
      return;
   }

   DockWindow* pDockWindow = dynamic_cast<DockWindow*>(pWidget->parentWidget());
   if (pDockWindow != NULL)
   {
      PlotSet* pPlotSet = boost::any_cast<PlotSet*>(value);
      if (pPlotSet != NULL)
      {
         SessionItemWrapper* pDockWindowWrapper = getWrapper(pDockWindow);
         if (pDockWindowWrapper != NULL)
         {
            addPlotSetItem(pDockWindowWrapper, pPlotSet);
         }
      }
   }
}

void WindowModel::WindowSourceModel::removePlotSet(Subject& subject, const string& signal, const boost::any& value)
{
   PlotSetGroup* pPlotSetGroup = dynamic_cast<PlotSetGroup*>(&subject);
   if (pPlotSetGroup == NULL)
   {
      return;
   }

   QWidget* pWidget = pPlotSetGroup->getWidget();
   if (pWidget == NULL)
   {
      return;
   }

   DockWindow* pDockWindow = dynamic_cast<DockWindow*>(pWidget->parentWidget());
   if (pDockWindow != NULL)
   {
      PlotSet* pPlotSet = boost::any_cast<PlotSet*>(value);
      if (pPlotSet != NULL)
      {
         SessionItemWrapper* pDockWindowWrapper = getWrapper(pDockWindow);
         if (pDockWindowWrapper != NULL)
         {
            removePlotSetItem(pDockWindowWrapper, pPlotSet);
         }
      }
   }
}

void WindowModel::WindowSourceModel::setCurrentPlotSet(Subject& subject, const string& signal, const boost::any& value)
{
   PlotSet* pPlotSet = boost::any_cast<PlotSet*>(value);
   if (pPlotSet != NULL)
   {
      activateItem(pPlotSet);
   }
}

void WindowModel::WindowSourceModel::addPlotWidget(Subject& subject, const string& signal, const boost::any& value)
{
   PlotSet* pPlotSet = dynamic_cast<PlotSet*>(&subject);
   if (pPlotSet != NULL)
   {
      PlotWidget* pPlotWidget = boost::any_cast<PlotWidget*>(value);
      if (pPlotWidget != NULL)
      {
         SessionItemWrapper* pPlotSetWrapper = getWrapper(pPlotSet);
         if (pPlotSetWrapper != NULL)
         {
            pPlotSetWrapper->addChild(pPlotWidget);
         }
      }
   }
}

void WindowModel::WindowSourceModel::removePlotWidget(Subject& subject, const string& signal, const boost::any& value)
{
   PlotSet* pPlotSet = dynamic_cast<PlotSet*>(&subject);
   if (pPlotSet != NULL)
   {
      PlotWidget* pPlotWidget = boost::any_cast<PlotWidget*>(value);
      if (pPlotWidget != NULL)
      {
         SessionItemWrapper* pPlotSetWrapper = getWrapper(pPlotSet);
         if (pPlotSetWrapper != NULL)
         {
            pPlotSetWrapper->removeChild(pPlotWidget);
         }
      }
   }
}

void WindowModel::WindowSourceModel::setCurrentPlotWidget(Subject& subject, const string& signal,
                                                          const boost::any& value)
{
   PlotWidget* pPlotWidget = boost::any_cast<PlotWidget*>(value);
   if (pPlotWidget != NULL)
   {
      activateItem(pPlotWidget);
   }
}

WindowModel::WindowSourceModel::SessionItemWrapper* WindowModel::WindowSourceModel::addWindowItem(Window* pWindow)
{
   if (pWindow == NULL)
   {
      return NULL;
   }

   // Add the window item
   SessionItemWrapper* pWrapper = NULL;
   if (dynamic_cast<WorkspaceWindow*>(pWindow) != NULL)
   {
      pWrapper = mpWorkspaceWindowWrapper;
   }
   else if (dynamic_cast<DockWindow*>(pWindow) != NULL)
   {
      pWrapper = mpDockWindowWrapper;
   }
   else if (dynamic_cast<ToolBar*>(pWindow) != NULL)
   {
      pWrapper = mpToolBarWrapper;
   }

   if (pWrapper == NULL)
   {
      return NULL;
   }

   SessionItemWrapper* pWindowWrapper = pWrapper->addChild(pWindow);
   if (pWindowWrapper != NULL)
   {
      WorkspaceWindow* pWorkspaceWindow = dynamic_cast<WorkspaceWindow*>(pWindow);
      if (pWorkspaceWindow != NULL)
      {
         View* pView = pWorkspaceWindow->getView();
         if (pView != NULL)
         {
            addViewItem(pWindowWrapper, pView, false);
         }
      }

      ToolBar* pToolbar = dynamic_cast<ToolBar*>(pWindow);
      if (pToolbar != NULL)
      {
         bool toolbarDisplayed = pToolbar->isShown();
         pWindowWrapper->setCheckState(toolbarDisplayed ? Qt::Checked : Qt::Unchecked);

         // Connections
         pToolbar->attach(SIGNAL_NAME(ToolBar, Shown), Slot(this, &WindowSourceModel::updateToolbarDisplay));
         pToolbar->attach(SIGNAL_NAME(ToolBar, Hidden), Slot(this, &WindowSourceModel::updateToolbarDisplay));
      }

      DockWindow* pDockWindow = dynamic_cast<DockWindow*>(pWindow);
      if (pDockWindow != NULL)
      {
         bool dockWindowDisplayed = pDockWindow->isShown();
         pWindowWrapper->setCheckState(dockWindowDisplayed ? Qt::Checked : Qt::Unchecked);

         // Connections
         pDockWindow->attach(SIGNAL_NAME(ViewWindow, AboutToSetWidget), Slot(this, &WindowSourceModel::removePlotSets));
         pDockWindow->attach(SIGNAL_NAME(ViewWindow, WidgetSet), Slot(this, &WindowSourceModel::addPlotSets));
         pDockWindow->attach(SIGNAL_NAME(DockWindow, Shown), Slot(this, &WindowSourceModel::updateDockDisplay));
         pDockWindow->attach(SIGNAL_NAME(DockWindow, Hidden), Slot(this, &WindowSourceModel::updateDockDisplay));

         // Add plot set items
         addPlotSets(*pDockWindow, SIGNAL_NAME(ViewWindow, WidgetSet), boost::any(pDockWindow->getWidget()));
      }
   }

   return pWindowWrapper;
}

void WindowModel::WindowSourceModel::removeWindowItem(Window* pWindow)
{
   if (pWindow == NULL)
   {
      return;
   }

   SessionItemWrapper* pWindowWrapper = getWrapper(pWindow);
   if (pWindowWrapper == NULL)
   {
      return;
   }

   WorkspaceWindow* pWorkspaceWindow = dynamic_cast<WorkspaceWindow*>(pWindow);
   if (pWorkspaceWindow != NULL)
   {
      View* pView = pWorkspaceWindow->getView();
      if (pView != NULL)
      {
         removeViewItem(pWindowWrapper, pView);
      }
   }

   ToolBar* pToolBar = dynamic_cast<ToolBar*>(pWindow);
   if (pToolBar != NULL)
   {
      pToolBar->detach(SIGNAL_NAME(ToolBar, Shown), Slot(this, &WindowSourceModel::updateToolbarDisplay));
      pToolBar->detach(SIGNAL_NAME(ToolBar, Hidden), Slot(this, &WindowSourceModel::updateToolbarDisplay));
   }

   DockWindow* pDockWindow = dynamic_cast<DockWindow*>(pWindow);
   if (pDockWindow != NULL)
   {
      pDockWindow->detach(SIGNAL_NAME(ViewWindow, AboutToSetWidget), Slot(this, &WindowSourceModel::removePlotSets));
      pDockWindow->detach(SIGNAL_NAME(ViewWindow, WidgetSet), Slot(this, &WindowSourceModel::addPlotSets));
      pDockWindow->detach(SIGNAL_NAME(DockWindow, Shown), Slot(this, &WindowSourceModel::updateDockDisplay));
      pDockWindow->detach(SIGNAL_NAME(DockWindow, Hidden), Slot(this, &WindowSourceModel::updateDockDisplay));

      // Remove the plot set items
      removePlotSets(*pDockWindow, SIGNAL_NAME(ViewWindow, AboutToSetWidget), boost::any());
   }

   // Remove the window item
   SessionItemWrapper* pParentWrapper = NULL;
   if (dynamic_cast<WorkspaceWindow*>(pWindow) != NULL)
   {
      pParentWrapper = mpWorkspaceWindowWrapper;
   }
   else if (dynamic_cast<DockWindow*>(pWindow) != NULL)
   {
      pParentWrapper = mpDockWindowWrapper;
   }
   else if (dynamic_cast<ToolBar*>(pWindow) != NULL)
   {
      pParentWrapper = mpToolBarWrapper;
   }

   if (pParentWrapper != NULL)
   {
      pParentWrapper->removeChild(pWindow);
   }
}

WindowModel::WindowSourceModel::SessionItemWrapper*
WindowModel::WindowSourceModel::addViewItem(SessionItemWrapper* pWindowWrapper, View* pView, bool bAddViewItem)
{
   if ((pWindowWrapper == NULL) || (pView == NULL))
   {
      return NULL;
   }

   SessionItemWrapper* pLayerParentWrapper = pWindowWrapper;

   // Add the view item
   SessionItemWrapper* pViewWrapper = NULL;
   if (bAddViewItem == true)
   {
      pViewWrapper = pWindowWrapper->addChild(pView);
      if (pViewWrapper != NULL)
      {
         pLayerParentWrapper = pViewWrapper;
      }
   }

   if (pLayerParentWrapper != NULL)
   {
      // Add the layer items
      SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pView);
      if (pSpatialDataView != NULL)
      {
         LayerList* pLayerList = pSpatialDataView->getLayerList();
         if (pLayerList != NULL)
         {
            // Add the layer items in reverse order so that the top-most layer is added first
            vector<Layer*> layers;
            pLayerList->getLayers(layers);

            for (vector<Layer*>::reverse_iterator iter = layers.rbegin(); iter != layers.rend(); ++iter)
            {
               Layer* pLayer = *iter;
               if (pLayer != NULL)
               {
                  addLayerItem(pLayerParentWrapper, pLayer);
               }
            }

            Layer* pActiveLayer = pSpatialDataView->getActiveLayer();
            if (pActiveLayer != NULL)
            {
               activateItem(pActiveLayer);
            }

            // Connections
            pLayerList->attach(SIGNAL_NAME(LayerList, LayerAdded), Slot(this, &WindowSourceModel::addLayer));
            pLayerList->attach(SIGNAL_NAME(LayerList, LayerDeleted), Slot(this, &WindowSourceModel::removeLayer));
            pSpatialDataView->attach(SIGNAL_NAME(SpatialDataView, LayerDisplayIndexesChanged),
               Slot(this, &WindowSourceModel::updateLayerOrder));
            pSpatialDataView->attach(SIGNAL_NAME(SpatialDataView, LayerShown),
               Slot(this, &WindowSourceModel::updateLayerDisplay));
            pSpatialDataView->attach(SIGNAL_NAME(SpatialDataView, LayerHidden),
               Slot(this, &WindowSourceModel::updateLayerDisplay));
            pSpatialDataView->attach(SIGNAL_NAME(SpatialDataView, LayerActivated),
               Slot(this, &WindowSourceModel::activateLayer));
         }
      }

      ProductView* pProductView = dynamic_cast<ProductView*>(pView);
      if (pProductView != NULL)
      {
         ClassificationLayer* pClassificationLayer = pProductView->getClassificationLayer();
         if (pClassificationLayer != NULL)
         {
            addLayerItem(pLayerParentWrapper, pClassificationLayer);
         }

         AnnotationLayer* pLayoutLayer = pProductView->getLayoutLayer();
         if (pLayoutLayer != NULL)
         {
            addLayerItem(pLayerParentWrapper, pLayoutLayer);
         }

         Layer* pActiveLayer = pProductView->getActiveLayer();
         if (pActiveLayer != NULL)
         {
            activateItem(pActiveLayer);
         }

         // Connections
         pProductView->attach(SIGNAL_NAME(ProductView, LayerActivated),
            Slot(this, &WindowSourceModel::activateLayer));
      }
   }

   return pViewWrapper;
}

void WindowModel::WindowSourceModel::removeViewItem(SessionItemWrapper* pWindowWrapper, View* pView)
{
   if (pView == NULL)
   {
      return;
   }

   bool bRemoveViewItem = true;

   SessionItemWrapper* pParentLayerWrapper = getWrapper(pView);
   if (pParentLayerWrapper == NULL)
   {
      pParentLayerWrapper = pWindowWrapper;
      bRemoveViewItem = false;
   }

   if (pParentLayerWrapper != NULL)
   {
      SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pView);
      if (pSpatialDataView != NULL)
      {
         LayerList* pLayerList = pSpatialDataView->getLayerList();
         if (pLayerList != NULL)
         {
            // Detach the layer list and view
            pLayerList->detach(SIGNAL_NAME(LayerList, LayerAdded), Slot(this, &WindowSourceModel::addLayer));
            pLayerList->detach(SIGNAL_NAME(LayerList, LayerDeleted), Slot(this, &WindowSourceModel::removeLayer));
            pSpatialDataView->detach(SIGNAL_NAME(SpatialDataView, LayerDisplayIndexesChanged),
               Slot(this, &WindowSourceModel::updateLayerOrder));
            pSpatialDataView->detach(SIGNAL_NAME(SpatialDataView, LayerShown),
               Slot(this, &WindowSourceModel::updateLayerDisplay));
            pSpatialDataView->detach(SIGNAL_NAME(SpatialDataView, LayerHidden),
               Slot(this, &WindowSourceModel::updateLayerDisplay));
            pSpatialDataView->detach(SIGNAL_NAME(SpatialDataView, LayerActivated),
               Slot(this, &WindowSourceModel::activateLayer));

            // Remove the layer items
            vector<Layer*> layers;
            pLayerList->getLayers(layers);
            for (vector<Layer*>::size_type i = 0; i < layers.size(); ++i)
            {
               Layer* pLayer = layers[i];
               if (pLayer != NULL)
               {
                  removeLayerItem(pParentLayerWrapper, pLayer);
               }
            }
         }
      }

      ProductView* pProductView = dynamic_cast<ProductView*>(pView);
      if (pProductView != NULL)
      {
         // Detach the view
         pProductView->detach(SIGNAL_NAME(ProductView, LayerActivated), Slot(this, &WindowSourceModel::activateLayer));

         // Remove the layer items
         ClassificationLayer* pClassificationLayer = pProductView->getClassificationLayer();
         if (pClassificationLayer != NULL)
         {
            removeLayerItem(pParentLayerWrapper, pClassificationLayer);
         }

         AnnotationLayer* pLayoutLayer = pProductView->getLayoutLayer();
         if (pLayoutLayer != NULL)
         {
            removeLayerItem(pParentLayerWrapper, pLayoutLayer);
         }
      }

      // Remove the view item
      if (bRemoveViewItem == true)
      {
         pWindowWrapper->removeChild(pView);
      }
   }
}

WindowModel::WindowSourceModel::SessionItemWrapper*
WindowModel::WindowSourceModel::addLayerItem(SessionItemWrapper* pViewWrapper, Layer* pLayer)
{
   if ((pViewWrapper == NULL) || (pLayer == NULL))
   {
      return NULL;
   }

   // Add the layer item
   SessionItemWrapper* pLayerWrapper = pViewWrapper->addChild(pLayer);
   if (pLayerWrapper != NULL)
   {
      // Add the graphic object items
      GraphicElement* pGraphicElement = dynamic_cast<GraphicElement*>(pLayer->getDataElement());
      if (pGraphicElement != NULL)
      {
         GraphicGroup* pGroup = pGraphicElement->getGroup();
         if (pGroup != NULL)
         {
            const list<GraphicObject*>& objects = pGroup->getObjects();
            for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
            {
               GraphicObject* pObject = *iter;
               if (pObject != NULL)
               {
                  addGraphicObjectItem(pLayerWrapper, pObject);
               }
            }

            // Connections
            pGroup->attach(SIGNAL_NAME(GraphicGroup, ObjectAdded), Slot(this, &WindowSourceModel::addGraphicObject));
            pGroup->attach(SIGNAL_NAME(GraphicGroup, ObjectRemoved),
               Slot(this, &WindowSourceModel::removeGraphicObject));
         }
      }

      // Initialize the wrapper
      bool layerActive = false;

      SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pSpatialDataView != NULL)
      {
         // Determine the active layer to bold the layer name if necessary
         if (pSpatialDataView->getActiveLayer() == pLayer)
         {
            layerActive = true;
         }

         // Check state
         bool layerDisplayed = pSpatialDataView->isLayerDisplayed(pLayer);
         pLayerWrapper->setCheckState(layerDisplayed ? Qt::Checked : Qt::Unchecked);
      }

      ProductView* pProductView = dynamic_cast<ProductView*>(pLayer->getView());
      if (pProductView != NULL)
      {
         // Determine the active layer to bold the layer name if necessary
         if (pProductView->getActiveLayer() == pLayer)
         {
            layerActive = true;
         }
      }

      QFont itemFont = pLayerWrapper->getDisplayFont();
      itemFont.setBold(layerActive);
      pLayerWrapper->setDisplayFont(itemFont);
   }

   return pLayerWrapper;
}

void WindowModel::WindowSourceModel::removeLayerItem(SessionItemWrapper* pViewWrapper, Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return;
   }

   SessionItemWrapper* pLayerWrapper = getWrapper(pLayer);
   if (pLayerWrapper != NULL)
   {
      GraphicElement* pGraphicElement = dynamic_cast<GraphicElement*>(pLayer->getDataElement());
      if (pGraphicElement != NULL)
      {
         GraphicGroup* pGroup = pGraphicElement->getGroup();
         if (pGroup != NULL)
         {
            // Detach from the element's group
            pGroup->detach(SIGNAL_NAME(GraphicGroup, ObjectAdded), Slot(this, &WindowSourceModel::addGraphicObject));
            pGroup->detach(SIGNAL_NAME(GraphicGroup, ObjectRemoved),
               Slot(this, &WindowSourceModel::removeGraphicObject));

            // Remove the graphic object items
            const list<GraphicObject*>& objects = pGroup->getObjects();
            for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
            {
               GraphicObject* pObject = *iter;
               if (pObject != NULL)
               {
                  removeGraphicObjectItem(pLayerWrapper, pObject);
               }
            }
         }
      }
   }

   // Remove the layer item
   if (pViewWrapper != NULL)
   {
      pViewWrapper->removeChild(pLayer);
   }
}

WindowModel::WindowSourceModel::SessionItemWrapper*
WindowModel::WindowSourceModel::addGraphicObjectItem(SessionItemWrapper* pLayerWrapper, GraphicObject* pObject)
{
   if ((pLayerWrapper == NULL) || (pObject == NULL))
   {
      return NULL;
   }

   // Do not add objects that are used in the background
   if (pObject->isVisible() == false)
   {
      return NULL;
   }

   // Add the graphic object item
   SessionItemWrapper* pObjectWrapper = pLayerWrapper->addChild(pObject);
   if (pObjectWrapper != NULL)
   {
      // Add a group's child objects
      GraphicGroup* pGroup = dynamic_cast<GraphicGroup*>(pObject);
      if (pGroup != NULL)
      {
         const list<GraphicObject*>& objects = pGroup->getObjects();
         for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
         {
            GraphicObject* pGroupObject = *iter;
            if (pGroupObject != NULL)
            {
               addGraphicObjectItem(pObjectWrapper, pGroupObject);
            }
         }

         // Connections
         pGroup->attach(SIGNAL_NAME(GraphicGroup, ObjectAdded), Slot(this, &WindowSourceModel::addGraphicObject));
         pGroup->attach(SIGNAL_NAME(GraphicGroup, ObjectRemoved), Slot(this, &WindowSourceModel::removeGraphicObject));
      }

      // Connect to the view object
      ViewObject* pViewObject = dynamic_cast<ViewObject*>(pObject);
      if (pViewObject != NULL)
      {
         // Add the object's view item
         View* pView = pViewObject->getObjectView();
         if (pView != NULL)
         {
            addViewItem(pObjectWrapper, pView);
         }

         pViewObject->attach(SIGNAL_NAME(ViewObject, ViewCreated), Slot(this, &WindowSourceModel::addView));
         pViewObject->attach(SIGNAL_NAME(ViewObject, ViewDeleted), Slot(this, &WindowSourceModel::removeView));
      }
   }

   return pLayerWrapper;
}

void WindowModel::WindowSourceModel::removeGraphicObjectItem(SessionItemWrapper* pLayerWrapper, GraphicObject* pObject)
{
   if (pObject == NULL)
   {
      return;
   }

   SessionItemWrapper* pObjectWrapper = getWrapper(pObject);
   if (pObjectWrapper != NULL)
   {
      ViewObject* pViewObject = dynamic_cast<ViewObject*>(pObject);
      if (pViewObject != NULL)
      {
         // Detach from a view object
         pViewObject->detach(SIGNAL_NAME(ViewObject, ViewCreated), Slot(this, &WindowSourceModel::addView));
         pViewObject->detach(SIGNAL_NAME(ViewObject, ViewDeleted), Slot(this, &WindowSourceModel::removeView));

         // Remove the object's view item
         View* pView = pViewObject->getObjectView();
         if (pView != NULL)
         {
            SessionItemWrapper* pViewObjectWrapper = getWrapper(pViewObject);
            if (pViewObjectWrapper != NULL)
            {
               removeViewItem(pViewObjectWrapper, pView);
            }
         }
      }

      GraphicGroup* pGroup = dynamic_cast<GraphicGroup*>(pObject);
      if (pGroup != NULL)
      {
         // Detach from the group object
         pGroup->detach(SIGNAL_NAME(GraphicGroup, ObjectAdded), Slot(this, &WindowSourceModel::addGraphicObject));
         pGroup->detach(SIGNAL_NAME(GraphicGroup, ObjectRemoved), Slot(this, &WindowSourceModel::removeGraphicObject));

         // Remove the group's child objects
         const list<GraphicObject*>& objects = pGroup->getObjects();
         for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
         {
            GraphicObject* pGroupObject = *iter;
            if (pGroupObject != NULL)
            {
               removeGraphicObjectItem(pObjectWrapper, pGroupObject);
            }
         }
      }
   }

   // Remove the layer item
   if (pLayerWrapper != NULL)
   {
      pLayerWrapper->removeChild(pObject);
   }
}

WindowModel::WindowSourceModel::SessionItemWrapper*
WindowModel::WindowSourceModel::addPlotSetItem(SessionItemWrapper* pDockWindowWrapper, PlotSet* pPlotSet)
{
   if ((pDockWindowWrapper == NULL) || (pPlotSet == NULL))
   {
      return NULL;
   }

   // Add the plot set item
   SessionItemWrapper* pPlotSetWrapper = pDockWindowWrapper->addChild(pPlotSet);
   if (pPlotSetWrapper != NULL)
   {
      // Add the plot widget items
      vector<PlotWidget*> plots;
      pPlotSet->getPlots(plots);
      for (vector<PlotWidget*>::size_type i = 0; i < plots.size(); ++i)
      {
         PlotWidget* pPlotWidget = plots[i];
         if (pPlotWidget != NULL)
         {
            pPlotSetWrapper->addChild(pPlotWidget);
            if (pPlotSet->getCurrentPlot() == pPlotWidget)
            {
               activateItem(pPlotWidget);
            }
         }
      }

      // Connections
      pPlotSet->attach(SIGNAL_NAME(PlotSet, PlotAdded), Slot(this, &WindowSourceModel::addPlotWidget));
      pPlotSet->attach(SIGNAL_NAME(PlotSet, PlotDeleted), Slot(this, &WindowSourceModel::removePlotWidget));
      pPlotSet->attach(SIGNAL_NAME(PlotSet, Activated), Slot(this, &WindowSourceModel::setCurrentPlotWidget));
   }

   return pPlotSetWrapper;
}

void WindowModel::WindowSourceModel::removePlotSetItem(SessionItemWrapper* pDockWindowWrapper, PlotSet* pPlotSet)
{
   if ((pDockWindowWrapper == NULL) || (pPlotSet == NULL))
   {
      return;
   }

   // Detach the plot set
   pPlotSet->detach(SIGNAL_NAME(PlotSet, PlotAdded), Slot(this, &WindowSourceModel::addPlotWidget));
   pPlotSet->detach(SIGNAL_NAME(PlotSet, PlotDeleted), Slot(this, &WindowSourceModel::removePlotWidget));
   pPlotSet->detach(SIGNAL_NAME(PlotSet, Activated), Slot(this, &WindowSourceModel::setCurrentPlotWidget));

   SessionItemWrapper* pPlotSetWrapper = getWrapper(pPlotSet);
   if (pPlotSetWrapper != NULL)
   {
      // Remove the plot widget items
      vector<PlotWidget*> plots;
      pPlotSet->getPlots(plots);
      for (vector<PlotWidget*>::size_type i = 0; i < plots.size(); ++i)
      {
         PlotWidget* pPlotWidget = plots[i];
         if (pPlotWidget != NULL)
         {
            pPlotSetWrapper->removeChild(pPlotWidget);
         }
      }

      // Remove the plot set item
      pDockWindowWrapper->removeChild(pPlotSet);
   }
}
