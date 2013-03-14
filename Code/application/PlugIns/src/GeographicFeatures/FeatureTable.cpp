/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AlphaNumericSortFilterProxyModel.h"
#include "DesktopServices.h"
#include "FeatureClass.h"
#include "FeatureTable.h"
#include "GraphicElement.h"
#include "GraphicGroup.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "PerspectiveView.h"
#include "SignalBlocker.h"

#include <QtGui/QMenu>
#include <QtGui/QScrollBar>

#include <list>
#include <string>
#include <vector>

FeatureTable::FeatureTable(FeatureClass* pFeatureClass, GraphicLayer* pFeatureLayer, QWidget* pParent) :
   QTableView(pParent),
   mpFeatureLayer(pFeatureLayer)
{
   setContextMenuPolicy(Qt::CustomContextMenu);

   // Allow multiple uncontiguous selections.
   setSelectionBehavior(QAbstractItemView::SelectRows);
   setSelectionMode(QAbstractItemView::ExtendedSelection);

   // Alphanumerically sort string types.
   setSortingEnabled(true);
   QSortFilterProxyModel* pProxyModel = new AlphaNumericSortFilterProxyModel();
   pProxyModel->setSourceModel(pFeatureClass);
   pProxyModel->setDynamicSortFilter(true);
   setModel(pProxyModel);

   // Want to select rows in the table, when features are selected from the view's layer.
   mpFeatureLayer.addSignal(SIGNAL_NAME(GraphicLayer, ObjectsSelected), Slot(this, &FeatureTable::selectRows));

   // Want to select graphic objects in the view, when rows are selected in the table.
   // However do not want the above connection to be utilized when the graphic objects are selected, so instead of
   // overriding the selectionChanged method, a new slot is introduced which will prevent any duplicate graphic object
   // selection/deselection.
   if (selectionModel() != NULL)
   {
      VERIFYNR(connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this,
         SLOT(selectGraphicObjects(const QItemSelection&, const QItemSelection&))));
   }

   // Select rows in the table, if features are already selected in the view.
   if (mpFeatureLayer.get() != NULL)
   {
      std::list<GraphicObject*> selectedObjects;
      mpFeatureLayer->getSelectedObjects(selectedObjects);
      selectRows(*(mpFeatureLayer.get()), GraphicLayer::signalObjectsSelected(), boost::any(selectedObjects));
   }

   VERIFYNR(connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
      this, SLOT(displayContextMenu(const QPoint&))));
}

FeatureTable::~FeatureTable()
{}

GraphicLayer* FeatureTable::getFeatureLayer()
{
   return mpFeatureLayer.get();
}

const GraphicLayer* FeatureTable::getFeatureLayer() const
{
   return mpFeatureLayer.get();
}

void FeatureTable::displayContextMenu(const QPoint& pos)
{
   if (!selectedIndexes().empty())
   {
      QMenu contextMenu(this);
      contextMenu.addAction("Zoom To", this, SLOT(zoomToSelected()));
      contextMenu.addAction("Export Selected", this, SLOT(exportSelected()));
      contextMenu.exec(viewport()->mapToGlobal(pos));
   }
}

void FeatureTable::zoomToSelected()
{
   // Get pointer to model.
   QAbstractItemModel* pModel = model();

   // Get the indexes of the cells which are selected in the table, including the hidden cells.
   QModelIndexList selectedIndexList = selectedIndexes();

   // If at least one row selected.
   int numSelected = selectedIndexList.count();
   GraphicLayer* pFeatureLayer = getFeatureLayer();
   if (numSelected > 0 && pFeatureLayer != NULL)
   {
      std::vector<LocationType> allExtents;
      // Get the graphic objects for the feature layer.
      GraphicElement* pAnnElement = dynamic_cast<GraphicElement*>(pFeatureLayer->getDataElement());
      if (pAnnElement != NULL)
      {
         GraphicGroup* pGraphicGroup = pAnnElement->getGroup();
         if (pGraphicGroup != NULL)
         {
            const std::list<GraphicObject*>& graphicObjects = pGraphicGroup->getObjects();
            if (!graphicObjects.empty())
            {
               // For each selected row.
               for (int i = 0; i < numSelected; ++i)
               {
                  // Find graphic object which corresponds to table row.
                  QModelIndex index = selectedIndexList.at(i);
                  if (index.column() == 0)
                  {
                     QVariant variant = pModel->data(index, Qt::UserRole);
                     std::string id = variant.toString().toStdString();

                     for (std::list<GraphicObject*>::const_iterator graphicObjectsIter = graphicObjects.begin();
                        graphicObjectsIter != graphicObjects.end();
                        ++graphicObjectsIter)
                     {
                        GraphicObject* pGraphicObject = *graphicObjectsIter;
                        if (pGraphicObject != NULL)
                        {
                           // Store world coordinate rotated extents.
                           if (id == pGraphicObject->getId())
                           {
                              std::vector<LocationType> extents;
                              pGraphicObject->getRotatedExtents(extents);
                              for (std::vector<LocationType>::iterator extentsIter = extents.begin();
                                 extentsIter != extents.end();
                                 ++extentsIter)
                              {
                                 pFeatureLayer->translateDataToWorld(extentsIter->mX, extentsIter->mY,
                                    extentsIter->mX, extentsIter->mY);
                                 allExtents.push_back(*extentsIter);
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }

      // Zoom to the extents of the graphic objects corresponding to the selected rows.
      PerspectiveView* pView = dynamic_cast<PerspectiveView*>(pFeatureLayer->getView());
      if (pView != NULL)
      {
         pView->zoomToArea(allExtents);
         pView->refresh();
      }
   }
}

void FeatureTable::exportSelected()
{
   if (mpFeatureLayer.get() != NULL)
   {
      Service<DesktopServices>()->exportSessionItem(mpFeatureLayer.get());
   }
}

void FeatureTable::selectGraphicObjects(const QItemSelection& selected, const QItemSelection& deselected)
{
   selectGraphicObjects(selected);
   selectGraphicObjects(deselected, false);
}

void FeatureTable::selectGraphicObjects(const QItemSelection& selection, bool selected)
{
   QAbstractItemModel* pModel = model();

   GraphicLayer* pFeatureLayer = getFeatureLayer();
   VERIFYNRV(pFeatureLayer != NULL);
   GraphicElement* pAnnElement = dynamic_cast<GraphicElement*>(pFeatureLayer->getDataElement());
   VERIFYNRV(pAnnElement != NULL);
   GraphicGroup* pGraphicGroup = pAnnElement->getGroup();
   VERIFYNRV(pGraphicGroup != NULL);
   const std::list<GraphicObject*>& graphicObjects = pGraphicGroup->getObjects();

   int numSelected = selection.count();
   if (numSelected > 0 && !graphicObjects.empty())
   {
      for (int i = 0; i < numSelected; ++i)
      {
         QItemSelectionRange contiguousSelection = selection[i];
         QModelIndexList indexes = contiguousSelection.indexes();

         for (int j = 0; j < indexes.count(); ++j)
         {
            QModelIndex index = indexes.at(j);
            if (index.column() == 0)
            {
               QVariant variant = pModel->data(index, Qt::UserRole);
               std::string id = variant.toString().toStdString();

               for (std::list<GraphicObject*>::const_iterator graphicObjectsIter = graphicObjects.begin();
                  graphicObjectsIter != graphicObjects.end();
                  ++graphicObjectsIter)
               {
                  GraphicObject* pGraphicObject = *graphicObjectsIter;
                  if (pGraphicObject != NULL)
                  {
                     if (pGraphicObject->getId() == id)
                     {
                        GraphicLayer* pLayer = pGraphicObject->getLayer();
                        if (pLayer != NULL)
                        {
                           if (selected != pLayer->isObjectSelected(pGraphicObject))
                           {
                              // Prevent cyclical calls
                              GraphicLayer* pFeatureLayer = mpFeatureLayer.get();
                              mpFeatureLayer.reset();
                              if (selected)
                              {
                                 pLayer->selectObject(pGraphicObject);
                              }
                              else
                              {
                                 pLayer->deselectObject(pGraphicObject);
                              }
                              mpFeatureLayer.reset(pFeatureLayer);
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

int FeatureTable::getObjectRow(const GraphicObject* pObject) const
{
   if (pObject == NULL)
   {
      return -1;
   }

   std::string objectId = pObject->getId();
   VERIFYRV(objectId.empty() == false, -1);

   QAbstractItemModel* pModel = model();
   VERIFYRV(pModel != NULL, -1);

   int numRows = pModel->rowCount();
   for (int i = 0; i < numRows; ++i)
   {
      QModelIndex index = pModel->index(i, 0);
      QVariant variant = pModel->data(index, Qt::UserRole);
      std::string currentObjectId = variant.toString().toStdString();
      if (currentObjectId == objectId)
      {
         return i;
      }
   }

   return -1;
}

void FeatureTable::selectRows(Subject& subject, const std::string& signal, const boost::any& data)
{
   QAbstractItemModel* pModel = model();
   VERIFYNR(pModel != NULL);

   QItemSelectionModel* pSelModel = selectionModel();
   VERIFYNR(pSelModel != NULL);

   VERIFYNR(disconnect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this,
      SLOT(selectGraphicObjects(const QItemSelection&, const QItemSelection&))));

   // Clear any currently selected rows
   clearSelection();

   // Select the table rows of the newly selected graphic objects
   std::list<GraphicObject*> selectedObjects = boost::any_cast<std::list<GraphicObject*> >(data);
   for (std::list<GraphicObject*>::iterator iter = selectedObjects.begin(); iter != selectedObjects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         int row = getObjectRow(pObject);
         if (row > -1)
         {
            // Need to select the rows in the item selection model instead of calling QTableView::selectRow()
            // because it uses the current keyboard modifiers with the extended selection.  Since multiple graphic
            // objects are selected with the Shift key, selectRow() interprets this as selecting a range of rows.
            QModelIndex index = pModel->index(row, 0);
            pSelModel->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);

            // Ensure that the last selected row is visible
            if (pObject == selectedObjects.back())
            {
               QScrollBar* pScroll = verticalScrollBar();
               VERIFYNRV(pScroll != NULL);

               pScroll->setSliderPosition(row);
            }
         }
      }
   }

   VERIFYNR(connect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this,
      SLOT(selectGraphicObjects(const QItemSelection&, const QItemSelection&))));
}
