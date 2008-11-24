/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <Qt3Support/Q3MimeSourceFactory>
#include <QtGui/QHelpEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QToolTip>

#include "WizardView.h"

#include "PlugInManagerServices.h"
#include "StringUtilities.h"
#include "Subject.h"
#include "WizardCanvasItem.h"
#include "WizardNodeImp.h"

using namespace std;

int WizardView::miItemZ = 1000;
int WizardView::miLineZ = 2000;
int WizardView::miNodeZ = 10000;

WizardView::WizardView(Q3Canvas* pViewing, QWidget* pParent) :
   Q3CanvasView(pViewing, pParent),
   mpDragRect(NULL),
   mpDragConnectLine(NULL)
{
   setFocusPolicy(Qt::StrongFocus);

   // Create a dummy corner widget
   QWidget* pCorner = new QWidget(this);
   if (pCorner != NULL)
   {
      QPalette cornerPalette = pCorner->palette();
      cornerPalette.setColor(QPalette::Window, Qt::lightGray);
      pCorner->setPalette(cornerPalette);
      pCorner->setAutoFillBackground(true);

      setCornerWidget(pCorner);
   }
}

WizardView::~WizardView()
{
}

void WizardView::attached(Subject &subject, const string &signal, const Slot &slot)
{
   WizardItem* pItem = dynamic_cast<WizardItem*>(&subject);
   if (pItem != NULL)
   {
      if (getCanvasItem(pItem) == NULL)
      {
         itemAttached(subject, signal, boost::any());
      }
   }
   else if (dynamic_cast<WizardNode*>(&subject) != NULL)
   {
      nodeAttached(subject, signal, boost::any());
   }
}

void WizardView::itemAttached(Subject &subject, const string &signal, const boost::any &data)
{
   WizardItemImp* pItem = dynamic_cast<WizardItemImp*>(&subject);
   if (pItem == NULL)
   {
      return;
   }

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   string itemType = pItem->getType();

   // Position
   double dXCoord = pItem->getXPosition();
   double dYCoord = pItem->getYPosition();

   // Execution order
   int iItems = getNumCanvasItems() + 1;

   WizardCanvasItem* pCanvasItem = new WizardCanvasItem(pItem, pCanvas);
   if (pCanvasItem != NULL)
   {
      // Background color
      QColor clrItem = Qt::white;

      if (itemType == PlugInManagerServices::AlgorithmType())
      {
         clrItem.setRgb(200, 255, 200);
      }
      else if (itemType == PlugInManagerServices::ExporterType())
      {
         clrItem.setRgb(200, 200, 255);
      }
      else if (itemType == PlugInManagerServices::GeoreferenceType())
      {
         clrItem.setRgb(200, 255, 255);
      }
      else if (itemType == PlugInManagerServices::ImporterType())
      {
         clrItem.setRgb(255, 200, 200);
      }
      else if (itemType == PlugInManagerServices::InterpreterType())
      {
         clrItem.setRgb(255, 225, 175);
      }
      else if (itemType == PlugInManagerServices::ViewerType())
      {
         clrItem.setRgb(175, 225, 255);
      }
      else if (itemType == PlugInManagerServices::WizardType())
      {
         clrItem.setRgb(225, 175, 225);
      }
      else
      {
         clrItem.setRgb(255, 255, 200);
      }

      pCanvasItem->setColor(clrItem);
      pCanvasItem->setZ(miItemZ++);
      pCanvasItem->move(dXCoord, dYCoord);
      pCanvasItem->setExecutionOrder(iItems);
      pCanvasItem->show();

      expandCanvasToItem(pCanvasItem);
   }

   pCanvas->update();
}

void WizardView::itemModified(Subject &subject, const string &signal, const boost::any &data)
{
   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   WizardItemImp* pItem = dynamic_cast<WizardItemImp*>(&subject);
   VERIFYNRV(pItem != NULL);

   // Position
   WizardCanvasItem* pCanvasItem = getCanvasItem(pItem);
   if (pCanvasItem != NULL)
   {
      WizardItemImp::WizardItemChangeType* peChange = boost::any_cast<WizardItemImp::WizardItemChangeType*>(data);
      if (peChange != NULL)
      {
         if (*peChange == WizardItemImp::ItemName)
         {
            pCanvasItem->setVisible(false);     // Force a redraw of the item
            pCanvasItem->updateDimensions();
            updateConnectionPosition(pCanvasItem, false);
            expandCanvasToItem(pCanvasItem);
            pCanvasItem->setVisible(true);
         }
         else if (*peChange == WizardItemImp::ItemPosition)
         {
            // Update the item and connected lines position
            double dXCoord = pItem->getXPosition();
            double dYCoord = pItem->getYPosition();

            if ((dXCoord != pCanvasItem->x()) || (dYCoord != pCanvasItem->y()))
            {
               QRect rcItem = pCanvasItem->boundingRect();
               pCanvas->setChanged(rcItem);

               pCanvasItem->move(dXCoord, dYCoord);
               updateConnectionPosition(pCanvasItem, false);
               expandCanvasToItem(pCanvasItem);

               rcItem = pCanvasItem->boundingRect();
               pCanvas->setChanged(rcItem);
            }
         }
         else if (*peChange == WizardItemImp::ItemBatchMode)
         {
            QRect rcItem = pCanvasItem->boundingRect();
            pCanvas->setChanged(rcItem);
         }
      }
   }

   pCanvas->update();
}

void WizardView::itemDeleted(Subject &subject, const string &signal, const boost::any &data)
{
   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   WizardItemImp* pItem = dynamic_cast<WizardItemImp*>(&subject);
   VERIFYNRV(pItem != NULL);

   WizardCanvasItem* pCanvasItem = NULL;
   pCanvasItem = getCanvasItem(pItem);
   if (pCanvasItem != NULL)
   {
      deselectItem(pCanvasItem, false);
      removeItemConnections(pCanvasItem, false);
      delete pCanvasItem;
   }

   pCanvas->update();
}

void WizardView::nodeAttached(Subject &subject, const string &signal, const boost::any &data)
{
   WizardNodeImp* pNode = dynamic_cast<WizardNodeImp*>(&subject);
   if (pNode == NULL)
   {
      return;
   }

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   WizardCanvasItem* pCanvasItem = NULL;
   pCanvasItem = getCanvasItem(pNode);
   if (NN(pCanvasItem))
   {
      pCanvasItem->setVisible(false);      // Force a redraw of the item

      pCanvasItem->updateDimensions();
      expandCanvasToItem(pCanvasItem);

      pCanvasItem->setVisible(true);
   }

   pCanvas->update();
}

void WizardView::nodeModified(Subject &subject, const string &signal, const boost::any &data)
{
   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   WizardNodeImp* pNode = dynamic_cast<WizardNodeImp*>(&subject);
   VERIFYNRV(pNode != NULL);

   WizardCanvasItem* pCanvasItem = NULL;
   pCanvasItem = getCanvasItem(pNode);
   if (pCanvasItem != NULL)
   {
      pCanvasItem->setVisible(false);      // Force a redraw of the item
      WizardNodeImp::WizardNodeChangeType* peChange = 
         boost::any_cast<WizardNodeImp::WizardNodeChangeType*>(data);

      if (peChange != NULL)
      {
         if ((*peChange == WizardNodeImp::ConnectedNodeAdded) ||
            (*peChange == WizardNodeImp::ConnectedNodeRemoved))
         {
            // Rebuild the node connections
            buildNodeConnections(pNode, false);
         }
      }
      pCanvasItem->setVisible(true);
   }

   pCanvas->update();
}

void WizardView::nodeDeleted(Subject &subject, const string &signal, const boost::any &data)
{
   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   WizardNodeImp* pNode = dynamic_cast<WizardNodeImp*>(&subject);
   VERIFYNRV(pNode != NULL);

   removeNodeConnections(pNode, false);

   pCanvas->update();
}

void WizardView::selectItem(WizardItem* pItem, bool bRefresh)
{
   Q3CanvasItem* pCanvasItem = getCanvasItem(pItem);
   selectItem(pCanvasItem, bRefresh);
}

void WizardView::selectItems(const vector<WizardItem*>& items, bool bRefresh)
{
   for (unsigned int i = 0; i < items.size(); i++)
   {
      WizardItem* pItem = items[i];
      if (pItem != NULL)
      {
         selectItem(pItem, false);
      }
   }

   if (bRefresh == true)
   {
      Q3Canvas* pCanvas = canvas();
      if (pCanvas != NULL)
      {
         pCanvas->update();
      }
   }
}

void WizardView::selectAllItems(bool bRefresh)
{
   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   Q3CanvasItemList lstItems = pCanvas->allItems();
   int iSelected = 0;

   for (int i = 0; i < lstItems.count(); i++)
   {
      Q3CanvasItem* pItem = lstItems[i];
      if (pItem != NULL)
      {
         bool bSelected = pItem->isSelected();
         if (bSelected == false)
         {
            pItem->setSelected(true);
            iSelected++;
         }
      }
   }

   if (iSelected > 0)
   {
      emit selectionChanged();
      if (bRefresh == true)
      {
         pCanvas->update();
      }
   }
}

bool WizardView::isItemSelected(WizardItem* pItem) const
{
   Q3CanvasItem* pCanvasItem = getCanvasItem(pItem);
   if (pCanvasItem == NULL)
   {
      return false;
   }

   bool bSelected = pCanvasItem->isSelected();
   return bSelected;
}

vector<WizardItem*> WizardView::getSelectedItems() const
{
   vector<WizardItem*> selectedItems;

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return selectedItems;
   }

   Q3CanvasItemList lstAllItems = pCanvas->allItems();
   for (int i = 0; i < lstAllItems.count(); i++)
   {
      Q3CanvasItem* pCanvasItem = lstAllItems[i];
      if (pCanvasItem != NULL)
      {
         bool bSelected = pCanvasItem->isSelected();
         if ((bSelected == true) && (pCanvasItem->rtti() == 2))
         {
            WizardCanvasItem* pWizardItem = static_cast<WizardCanvasItem*>(pCanvasItem);

            WizardItem* pItem = pWizardItem->getWizardItem();
            if (pItem != NULL)
            {
               selectedItems.push_back(pItem);
            }
         }
      }
   }

   return selectedItems;
}

int WizardView::getNumSelectedItems() const
{
   vector<WizardItem*> selectedItems = getSelectedItems();

   int iCount = selectedItems.size();
   return iCount;
}

void WizardView::deselectItem(WizardItem* pItem, bool bRefresh)
{
   Q3CanvasItem* pCanvasItem = getCanvasItem(pItem);
   if (pCanvasItem != NULL)
   {
      deselectItem(pCanvasItem, bRefresh);
   }
}

void WizardView::deselectAllItems(bool bRefresh)
{
   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   // Deselect all items
   Q3CanvasItemList lstItems = pCanvas->allItems();
   for (int i = 0; i < lstItems.count(); i++)
   {
      Q3CanvasItem* pItem = lstItems[i];
      if (pItem != NULL)
      {
         pItem->setSelected(false);
      }
   }

   emit selectionChanged();
   if (bRefresh == true)
   {
      pCanvas->update();
   }
}

vector<WizardConnection> WizardView::getSelectedConnections() const
{
   vector<WizardConnection> selectedConnections;

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return selectedConnections;
   }

   Q3CanvasItemList lstAllItems = pCanvas->allItems();
   for (int i = 0; i < lstAllItems.count(); i++)
   {
      Q3CanvasItem* pCanvasItem = lstAllItems[i];
      if (pCanvasItem != NULL)
      {
         bool bSelected = pCanvasItem->isSelected();
         if ((bSelected == true) && (pCanvasItem->rtti() == 7))
         {
            WizardLine* pLine = static_cast<WizardLine*>(pCanvasItem);

            WizardNode* pOutputNode = pLine->getOutputNode();
            WizardNode* pInputNode = pLine->getInputNode();

            WizardConnection currentConnection;
            currentConnection.mpOutputNode = pOutputNode;
            currentConnection.mpInputNode = pInputNode;

            selectedConnections.push_back(currentConnection);
         }
      }
   }

   return selectedConnections;
}

int WizardView::getNumSelectedConnections() const
{
   vector<WizardConnection> selectedConnections = getSelectedConnections();

   int iCount = selectedConnections.size();
   return iCount;
}

void WizardView::updateOrder(vector<WizardItem*> items)
{
   for (unsigned int i = 0; i < items.size(); i++)
   {
      WizardItem* pItem = items[i];
      if (pItem != NULL)
      {
         WizardCanvasItem* pCanvasItem = getCanvasItem(pItem);
         if (pCanvasItem != NULL)
         {
            pCanvasItem->setExecutionOrder(i + 1);
         }
      }
   }

   if (items.empty() == false)
   {
      Q3Canvas* pCanvas = canvas();
      if (pCanvas != NULL)
      {
         pCanvas->setAllChanged();
         pCanvas->update();
      }
   }
}

void WizardView::buildView(vector<WizardItem*> items, bool bClear)
{
   if (bClear == true)
   {
      clear();
   }

   // Items
   unsigned int numItems = items.size();
   if (numItems == 0)
   {
      return;
   }

   unsigned int i = 0;
   for (i = 0; i < numItems; i++)
   {
      WizardItemImp* pItem = static_cast<WizardItemImp*>(items[i]);
      if (pItem != NULL)
      {
         pItem->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &WizardView::itemModified));
         pItem->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardView::itemDeleted));

         unsigned int j = 0;

         const vector<WizardNode*>& inputNodes = pItem->getInputNodes();
         for (vector<WizardNode*>::const_iterator iter = inputNodes.begin(); iter != inputNodes.end(); ++iter)
         {
            WizardNodeImp* pNode = static_cast<WizardNodeImp*>(*iter);
            if (pNode != NULL)
            {
               pNode->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &WizardView::nodeModified));
               pNode->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardView::nodeDeleted));
            }
         }

         const vector<WizardNode*>& outputNodes = pItem->getOutputNodes();
         for (vector<WizardNode*>::const_iterator iter = outputNodes.begin(); iter != outputNodes.end(); ++iter)
         {
            WizardNodeImp* pNode = static_cast<WizardNodeImp*>(*iter);
            if (pNode != NULL)
            {
               pNode->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &WizardView::nodeModified));
               pNode->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardView::nodeDeleted));
            }
         }
      }
   }

   // Connections
   for (i = 0; i < numItems; i++)
   {
      WizardItem* pOutputItem = items[i];
      if (pOutputItem != NULL)
      {
         const vector<WizardNode*>& outputNodes = pOutputItem->getOutputNodes();

         unsigned int numOutputNodes = outputNodes.size();
         for (unsigned int j = 0; j < numOutputNodes; j++)
         {
            WizardNode* pOutputNode = outputNodes[j];
            if (pOutputNode == NULL)
            {
               continue;
            }

            const vector<WizardNode*>& connectedNodes = pOutputNode->getConnectedNodes();

            unsigned int numConnectedNodes = connectedNodes.size();
            for (unsigned int k = 0; k < numConnectedNodes; ++k)
            {
               WizardNode* pConnectedNode = connectedNodes[k];
               if (pConnectedNode != NULL)
               {
                  // Connect the nodes
                  addConnection(pOutputNode, pConnectedNode, false);
               }
            }
         }
      }
   }

   Q3Canvas* pCanvas = canvas();
   if (pCanvas != NULL)
   {
      pCanvas->update();
   }
}

void WizardView::drawView(QPainter* p)
{
   if (p == NULL)
   {
      return;
   }

   // Get the bounding rect for the entire wizard
   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   QRect rcView;
   rcView.setX(pCanvas->width());
   rcView.setY(pCanvas->height());

   Q3CanvasItemList lstItems = pCanvas->allItems();
   for (int i = 0; i < lstItems.count(); i++)
   {
      Q3CanvasItem* pItem = lstItems[i];
      if (pItem != NULL)
      {
         QRect rcItem = pItem->boundingRect();

         if (rcItem.left() < rcView.left())
         {
            rcView.setLeft(rcItem.left());
         }

         if (rcItem.top() < rcView.top())
         {
            rcView.setTop(rcItem.top());
         }

         if (rcItem.right() > rcView.right())
         {
            rcView.setRight(rcItem.right());
         }

         if (rcItem.bottom() > rcView.bottom())
         {
            rcView.setBottom(rcItem.bottom());
         }
      }
   }

   // Draw the view
   if (rcView.isNull() == false)
   {
      double dAspect = static_cast<double>(rcView.width()) / static_cast<double>(rcView.height());

      QRect rcViewport = p->viewport();

      int iPrintWidth = rcViewport.width();
      int iPrintHeight = rcViewport.height();
      double pAspect = static_cast<double>(iPrintWidth) / static_cast<double>(iPrintHeight);
      double ratioAspects = pAspect / dAspect;
      if (ratioAspects > 1.0)
      {
         iPrintWidth = iPrintWidth / ratioAspects;
      }
      else
      {
         iPrintHeight = iPrintHeight * ratioAspects;
      }

      if ((rcView.width() * 6 < iPrintWidth) || (rcView.height() * 6 < iPrintHeight))
      {
         iPrintWidth = rcView.width() * 6;
         iPrintHeight = rcView.height() * 6;
      }

      p->setWindow(rcView.x(), rcView.y(), rcView.width(), rcView.height());
      p->setViewport((rcViewport.width() - iPrintWidth) / 2,
         (rcViewport.height() - iPrintHeight) / 2, iPrintWidth, iPrintHeight);

      drawContents(p, rcView.x(), rcView.y(), rcView.width(), rcView.height());
   }
}

void WizardView::clear()
{
   deselectAllItems(false);

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   Q3CanvasItemList lstItems = pCanvas->allItems();
   while (lstItems.count() > 0)
   {
      Q3CanvasItem* pCanvasItem = lstItems.back();
      if (pCanvasItem != NULL)
      {
         if (pCanvasItem->rtti() == 2)
         {
            WizardCanvasItem* pWizardItem = static_cast<WizardCanvasItem*>(pCanvasItem);
            removeItemConnections(pWizardItem, false);
            delete pWizardItem;
         }
         else if (pCanvasItem->rtti() == 7)
         {
            WizardLine* pLine = static_cast<WizardLine*>(pCanvasItem);
            removeConnection(pLine, false);
         }
         else
         {
            delete pCanvasItem;
         }
      }

      // Get the list of items again in case multiple items were deleted
      lstItems = pCanvas->allItems();
   }

   pCanvas->update();
}

bool WizardView::event(QEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (pEvent->type() == QEvent::ToolTip)
      {
         QHelpEvent* pHelpEvent = static_cast<QHelpEvent*>(pEvent);

         Q3Canvas* pCanvas = canvas();
         if (pCanvas != NULL)
         {
            QPoint ptContents = viewportToContents(pHelpEvent->pos());

            Q3CanvasItemList lstItems = pCanvas->collisions(ptContents);
            for (int i = 0; i < lstItems.count(); i++)
            {
               Q3CanvasItem* pCanvasItem = lstItems[i];
               if (pCanvasItem != NULL)
               {
                  if (pCanvasItem->rtti() == 2)
                  {
                     WizardCanvasItem* pWizardItem = static_cast<WizardCanvasItem*>(pCanvasItem);

                     WizardNodeImp* pNode = static_cast<WizardNodeImp*>(pWizardItem->getNode(ptContents));
                     if (pNode == NULL)
                     {
                        return false;
                     }

                     QRect rcNode = pWizardItem->getNodeRect(pNode);
                     if (rcNode.isValid() == false)
                     {
                        return false;
                     }

                     QString strName;
                     QString strOriginalType;
                     QString strType;
                     QStringList strlValidTypes;

                     const string& nodeName = pNode->getName();
                     if (nodeName.empty() == false)
                     {
                        strName = QString::fromStdString(nodeName);
                     }

                     const string& originalType = pNode->getOriginalType();
                     if (originalType.empty() == false)
                     {
                        strOriginalType = QString::fromStdString(originalType);
                        strOriginalType.replace("<", "&lt;");
                        strOriginalType.replace(">", "&gt;");

                        QString strImageType = strOriginalType;
                        if (Q3MimeSourceFactory::defaultFactory()->data(QString::fromStdString(originalType)) == NULL)
                        {
                           strImageType = "Unknown";
                        }

                        strOriginalType = "<img source=\"" + strImageType + "\">" + strOriginalType;
                     }

                     const string& nodeType = pNode->getType();
                     if (nodeType.empty() == false)
                     {
                        strType = QString::fromStdString(nodeType);
                        strType.replace("<", "&lt;");
                        strType.replace(">", "&gt;");

                        QString strImageType = strType;
                        if (Q3MimeSourceFactory::defaultFactory()->data(QString::fromStdString(nodeType)) == NULL)
                        {
                           strImageType = "Unknown";
                        }

                        strType = "<img source=\"" + strImageType + "\">" + strType;
                     }

                     const vector<string>& validTypes = pNode->getValidTypes();
                     for (unsigned int j = 0; j < validTypes.size(); j++)
                     {
                        string currentType = validTypes[j];
                        if (currentType.empty() == false)
                        {
                           QString strValidType = QString::fromStdString(currentType);
                           strValidType.replace("<", "&lt;");
                           strValidType.replace(">", "&gt;");

                           QString strImageType = strValidType;
                           if (Q3MimeSourceFactory::defaultFactory()->data(QString::fromStdString(currentType)) == NULL)
                           {
                              strImageType = "Unknown";
                           }

                           strValidType = "<img source=\"" + strImageType + "\">" + strValidType;

                           strlValidTypes.append(strValidType);
                        }
                     }

                     QString strTip = "<qt><table width=235 cellspacing=0><tr><td width=90><b>Name:</b></td>"
                        "<td width=145>" + strName + "</td></tr><tr><td width=90><b>Type:</b></td><td width=145>" +
                        strOriginalType + "</td></tr></table><hr><table width=235 cellspacing=0><tr>"
                        "<td width=90><b>Current Type:</b></td><td width=145>" + strType + "</td></tr><tr>"
                        "<td width=90><b>Valid Types:</b></td><td width=145>" + strlValidTypes.join("<br>") +
                        "</td></tr></table></qt>";

                     QToolTip::showText(pHelpEvent->globalPos(), strTip);
                     break;
                  }
               }
            }
         }
      }
   }

   return Q3CanvasView::event(pEvent);
}

void WizardView::viewportMousePressEvent(QMouseEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   // Set the focus to the view
   setFocus();

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   // Reset the drag items
   if (mpDragRect != NULL)
   {
      delete mpDragRect;
      mpDragRect = NULL;
      pCanvas->update();
   }

   if (mpDragConnectLine != NULL)
   {
      delete mpDragConnectLine;
      mpDragConnectLine = NULL;
      pCanvas->update();
   }

   QPoint ptMouse = e->pos();
   QPoint ptContents = viewportToContents(ptMouse);
   Q3CanvasItemList lstItems = pCanvas->collisions(ptContents);

   int i = 0;
   int iCount = lstItems.count();
   for (i = 0; i < iCount; i++)
   {
      Q3CanvasItem* pCanvasItem = lstItems[i];
      if (pCanvasItem != NULL)
      {
         if ((pCanvasItem->rtti() == 2) || (pCanvasItem->rtti() == 7))
         {
            if (e->button() == Qt::LeftButton)
            {
               mptMoveStart = ptMouse;
               mptMoveValid = ptContents;

               bool bSelected = false;
               bSelected = pCanvasItem->isSelected();
               if (bSelected == false)
               {
                  if ((e->modifiers() != Qt::ShiftModifier) && (e->modifiers() != Qt::ControlModifier))
                  {
                     deselectAllItems(false);
                  }

                  selectItem(pCanvasItem, false);
                  pCanvas->update();
               }
               else
               {
                  if ((e->modifiers() == Qt::ShiftModifier) || (e->modifiers() == Qt::ControlModifier))
                  {
                     deselectItem(pCanvasItem, false);
                     pCanvas->update();
                  }
               }
            }

            if (pCanvasItem->rtti() == 2)
            {
               WizardCanvasItem* pWizardCanvasItem = static_cast<WizardCanvasItem*>(pCanvasItem);

               // Get the canvas item
               WizardItem* pItem = pWizardCanvasItem->getWizardItem();
               if (pItem != NULL)
               {
                  // Get the node at the clicked point
                  WizardNode* pNode = pWizardCanvasItem->getNode(ptContents);
                  if (pNode != NULL)
                  {
                     // Set the active node in the canvas item
                     if (e->button() == Qt::LeftButton)
                     {
                        QRect rcButton = pWizardCanvasItem->getNodeToggleRect(pNode);
                        if (rcButton.contains(ptContents) == true)
                        {
                           pWizardCanvasItem->setActiveNode(pNode);
                        }
                     }

                     // Start the drag line if necessary
                     if (e->button() == Qt::MidButton)
                     {
                        QRect rcNode = pWizardCanvasItem->getNodeRect(pNode);
                        if (rcNode.contains(ptContents) == true)
                        {
                           // Get the line points
                           QPoint ptNode = pWizardCanvasItem->getNodeConnectionPoint(pNode);
                           if (ptNode.isNull() == false)
                           {
                              QColor clrLine = pWizardCanvasItem->getNodeColor(pNode);

                              // Create the drag line
                              mpDragConnectLine = new Q3CanvasLine(pCanvas);
                              mpDragConnectLine->setPen(QPen(clrLine, 3));
                              mpDragConnectLine->setPoints(ptNode.x(), ptNode.y(), ptNode.x(), ptNode.y());
                              mpDragConnectLine->setZ(miLineZ++);
                              mpDragConnectLine->show();
                           }
                        }
                     }
                  }
               }

               // Emit the signal
               emit itemPressed(e, pItem);
               return;
            }
         }
      }
   }

   // Start the drag rectangle if necessary
   if ((iCount == 0) && (e->button() == Qt::LeftButton))
   {
      if ((e->modifiers() != Qt::ShiftModifier) && (e->modifiers() != Qt::ControlModifier))
      {
         deselectAllItems(false);
         pCanvas->update();
      }

      QPen pnRect = QPen(Qt::black, 1, Qt::DashLine);
      QBrush brRect = QBrush(Qt::NoBrush);

      // Create the drag rectangle
      mpDragRect = new Q3CanvasRectangle(ptContents.x(), ptContents.y(), 0, 0, pCanvas);
      mpDragRect->setPen(pnRect);
      mpDragRect->setBrush(brRect);
      mpDragRect->setZ(25000);
      mpDragRect->show();
   }
}

void WizardView::viewportMouseDoubleClickEvent(QMouseEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   QPoint ptMouse = e->pos();
   QPoint ptContents = viewportToContents(ptMouse);

   Q3CanvasItemList lstItems = pCanvas->collisions(ptContents);
   for (int i = 0; i < lstItems.count(); ++i)
   {
      Q3CanvasItem* pCanvasItem = lstItems[i];
      if (pCanvasItem != NULL)
      {
         if (pCanvasItem->rtti() == 2)
         {
            WizardCanvasItem* pWizardCanvasItem = static_cast<WizardCanvasItem*>(pCanvasItem);

            WizardItem* pItem = pWizardCanvasItem->getWizardItem();
            emit itemDoubleClicked(e, pItem);
            return;
         }
      }
   }
}

void WizardView::viewportMouseMoveEvent(QMouseEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   QPoint ptMouse = e->pos();
   QPoint ptContents = viewportToContents(ptMouse);

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   if (mpDragRect != NULL)
   {
      // Set the drag rectangle coordinates
      QRect rcDrag = mpDragRect->rect();
      int iWidth = ptContents.x() - rcDrag.x();
      int iHeight = ptContents.y() - rcDrag.y();

      mpDragRect->setSize(iWidth, iHeight);
      pCanvas->update();
   }
   else if (e->buttons() == Qt::LeftButton)
   {
      QRect rcSelected = getSelectedItemsRect();

      bool bMove = true;
      if (rcSelected.left() - (mptMoveValid.x() - ptContents.x()) < -6)
      {
         bMove = false;
      }

      if (rcSelected.top() - (mptMoveValid.y() - ptContents.y()) < -6)
      {
         bMove = false;
      }

      if (bMove == true)
      {
         mptMoveValid = ptContents;

         vector<WizardItem*> selectedItems = getSelectedItems();
         for (unsigned int i = 0; i < selectedItems.size(); i++)
         {
            WizardItem* pItem = selectedItems[i];
            if (pItem != NULL)
            {
               WizardCanvasItem* pWizardItem = getCanvasItem(pItem);
               if (pWizardItem != NULL)
               {
                  if (pWizardItem->getActiveNode() == NULL)
                  {
                     // Move the item
                     pWizardItem->moveBy(ptMouse.x() - mptMoveStart.x(), ptMouse.y() - mptMoveStart.y());

                     // Notify of a position change
                     double dX = pWizardItem->x();
                     double dY = pWizardItem->y();
                     emit itemPositionChanged(pItem, dX, dY);

                     // Move the connected lines
                     updateConnectionPosition(pWizardItem, false);

                     // Increase the canvas size if necessary
                     expandCanvasToItem(pWizardItem);
                  }
               }
            }
         }

         mptMoveStart = ptMouse;
         pCanvas->update();
      }
   }

   if (mpDragConnectLine != NULL)
   {
      // Set the drag line coordinates
      QPoint ptStart = mpDragConnectLine->startPoint();
      mpDragConnectLine->setPoints(ptStart.x(), ptStart.y(), ptContents.x(), ptContents.y());
      pCanvas->update();
   }
}

void WizardView::viewportMouseReleaseEvent(QMouseEvent* e)
{
   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   // Update the node type and reset the active node in the canvas items
   vector<WizardCanvasItem*> items = getCanvasItems();
   for (unsigned int i = 0; i < items.size(); i++)
   {
      WizardCanvasItem* pItem = items[i];
      if (pItem != NULL)
      {
         WizardNodeImp* pActiveNode = static_cast<WizardNodeImp*>(pItem->getActiveNode());
         if (pActiveNode != NULL)
         {
            // Break the node connections
            pActiveNode->clearConnectedNodes();

            // Update the current type
            string type = pActiveNode->getType();
            const vector<string>& validTypes = pActiveNode->getValidTypes();

            unsigned int uiIndex = 0;
            for (unsigned int j = 0; j < (validTypes.size() - 1); j++)
            {
               string currentType = validTypes.at(j);
               if (currentType == type)
               {
                  uiIndex = j + 1;
                  break;
               }
            }

            string newType = validTypes.at(uiIndex);
            if ((newType.empty() == false) && (newType != type))
            {
               pActiveNode->setType(newType);
               emit nodeTypeChanged(pActiveNode);
            }

            // Reset the active node
            pItem->setActiveNode(NULL);
         }
      }
   }

   if (mpDragRect != NULL)
   {
      // Select all items completely within the rectangle
      QRect rcSelection = mpDragRect->rect();
      rcSelection = rcSelection.normalized();
      selectItems(rcSelection, false);

      // Remove the drag rectangle
      delete mpDragRect;
      mpDragRect = NULL;

      // Update the canvas
      pCanvas->update();
   }

   if (mpDragConnectLine != NULL)
   {
      // Get the start and end points
      QPoint ptStart = mpDragConnectLine->startPoint();
      QPoint ptEnd = mpDragConnectLine->endPoint();

      // Get the items at the points
      WizardCanvasItem* pStartItem = NULL;

      int j = 0;

      Q3CanvasItemList lstItems = pCanvas->collisions(ptStart);
      for (j = 0; j < lstItems.count(); ++j)
      {
         Q3CanvasItem* pCanvasItem = lstItems[j];
         if (pCanvasItem != NULL)
         {
            if (pCanvasItem->rtti() == 2)
            {
               pStartItem = static_cast<WizardCanvasItem*>(pCanvasItem);
            }
         }
      }

      WizardCanvasItem* pEndItem = NULL;
      lstItems.clear();
      lstItems = pCanvas->collisions(ptEnd);

      for (j = 0; j < lstItems.count(); ++j)
      {
         Q3CanvasItem* pCanvasItem = lstItems[j];
         if (pCanvasItem != NULL)
         {
            if (pCanvasItem->rtti() == 2)
            {
               pEndItem = static_cast<WizardCanvasItem*>(pCanvasItem);
            }
         }
      }

      // Get the nodes
      if ((pStartItem != pEndItem) && (pStartItem != NULL) && (pEndItem != NULL))
      {
         WizardNodeImp* pStartNode = static_cast<WizardNodeImp*>(pStartItem->getNode(ptStart));
         WizardNodeImp* pEndNode = static_cast<WizardNodeImp*>(pEndItem->getNode(ptEnd));

         // Add the connection
         if ((pStartNode != pEndNode) && (pStartNode != NULL) && (pEndNode != NULL))
         {
            // Get the wizard items
            WizardItemImp* pInputItem = NULL;
            WizardItemImp* pOutputItem = NULL;

            WizardItemImp* pItem = static_cast<WizardItemImp*>(pStartItem->getWizardItem());
            if (pItem != NULL)
            {
               if (pItem->isInputNode(pStartNode) == true)
               {
                  pInputItem = pItem;
               }
               else if (pItem->isOutputNode(pStartNode) == true)
               {
                  pOutputItem = pItem;
               }
            }

            pItem = static_cast<WizardItemImp*>(pEndItem->getWizardItem());
            if (pItem != NULL)
            {
               if (pItem->isInputNode(pEndNode) == true)
               {
                  pInputItem = pItem;
               }
               else if (pItem->isOutputNode(pEndNode) == true)
               {
                  pOutputItem = pItem;
               }
            }

            // Check for circular connections between the two items
            if ((pInputItem != NULL) && (pOutputItem != NULL))
            {
               bool bConnected = pOutputItem->isItemConnected(pInputItem, true);
               if (bConnected == false)
               {
                  // Check if the nodes are already connected
                  bConnected = pStartNode->isNodeConnected(pEndNode);
                  if (bConnected == false)
                  {
                     // Connect the nodes
                     pStartNode->addConnectedNode(pEndNode);
                     pEndNode->addConnectedNode(pStartNode);
                  }
               }
               else
               {
                  QMessageBox::critical(this, "Wizard Builder", "These nodes cannot be connected "
                     "since a circular connection between the two items would be created!");
               }
            }
         }
      }

      // Remove the drag line
      delete mpDragConnectLine;
      mpDragConnectLine = NULL;

      // Update the canvas
      pCanvas->update();
   }
}

bool WizardView::selectItem(Q3CanvasItem* pItem, bool bRefresh)
{
   if (pItem == NULL)
   {
      return false;
   }

   if (pItem->isSelected() == true)
   {
      return false;
   }

   pItem->setSelected(true);
   emit selectionChanged();

   if (bRefresh == true)
   {
      Q3Canvas* pCanvas = canvas();
      if (pCanvas != NULL)
      {
         pCanvas->update();
      }
   }

   return true;
}

void WizardView::selectItems(QRect rcSelection, bool bRefresh)
{
   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   int iSelected = 0;

   Q3CanvasItemList lstItems = pCanvas->allItems();
   for (int i = 0; i < lstItems.count(); i++)
   {
      Q3CanvasItem* pItem = lstItems[i];
      if (pItem != NULL)
      {
         QRect rcItem = pItem->boundingRect();

         bool bSelect = rcSelection.contains(rcItem);
         if (bSelect == false)
         {
            continue;
         }

         bool bSelected = pItem->isSelected();
         if (bSelected == false)
         {
            pItem->setSelected(true);
            iSelected++;
         }
      }
   }

   if (iSelected > 0)
   {
      emit selectionChanged();
      if (bRefresh == true)
      {
         pCanvas->update();
      }
   }
}

bool WizardView::deselectItem(Q3CanvasItem* pItem, bool bRefresh)
{
   if (pItem == NULL)
   {
      return false;
   }

   if (pItem->isSelected() == false)
   {
      return false;
   }

   pItem->setSelected(false);
   emit selectionChanged();

   if (bRefresh == true)
   {
      Q3Canvas* pCanvas = canvas();
      if (pCanvas != NULL)
      {
         pCanvas->update();
      }
   }

   return true;
}

QRect WizardView::getSelectedItemsRect() const
{
   QRect rcSelected;

   vector<WizardItem*> selectedItems = getSelectedItems();
   for (unsigned int i = 0; i < selectedItems.size(); i++)
   {
      WizardItem* pItem = selectedItems[i];
      if (pItem != NULL)
      {
         WizardCanvasItem* pWizardItem = getCanvasItem(pItem);
         if (pWizardItem != NULL)
         {
            QRect rcItem = pWizardItem->boundingRect();

            if (i == 0)
            {
               rcSelected = rcItem;
            }
            else
            {
               if (rcItem.top() < rcSelected.top())
               {
                  rcSelected.setTop(rcItem.top());
               }

               if (rcItem.left() < rcSelected.left())
               {
                  rcSelected.setLeft(rcItem.left());
               }

               if (rcItem.right() > rcSelected.right())
               {
                  rcSelected.setRight(rcItem.right());
               }

               if (rcItem.bottom() > rcSelected.bottom())
               {
                  rcSelected.setBottom(rcItem.bottom());
               }
            }
         }
      }
   }

   return rcSelected;
}

void WizardView::expandCanvasToItem(Q3CanvasItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return;
   }

   QRect rcItem = pItem->boundingRect();

   bool bResize = false;
   int iWidth = pCanvas->width();
   int iHeight = pCanvas->height();

   if (rcItem.right() > iWidth)
   {
      iWidth = rcItem.right();
      bResize = true;
   }

   if (rcItem.bottom() > iHeight)
   {
      iHeight = rcItem.bottom();
      bResize = true;
   }

   if (bResize == true)
   {
      pCanvas->resize(iWidth, iHeight);
      pCanvas->update();
   }
}

bool WizardView::addConnection(WizardNode* pOutputNode, WizardNode* pInputNode, bool bRefresh)
{
   if ((pOutputNode == NULL) || (pInputNode == NULL))
   {
      return false;
   }

   WizardCanvasItem* pOutputCanvasItem = getCanvasItem(pOutputNode);
   WizardCanvasItem* pInputCanvasItem = getCanvasItem(pInputNode);

   if ((pOutputCanvasItem == NULL) || (pInputCanvasItem == NULL))
   {
      return false;
   }

   // Check for valid nodes
   WizardItem* pOutputItem = pOutputCanvasItem->getWizardItem();
   WizardItem* pInputItem = pInputCanvasItem->getWizardItem();

   if ((pOutputItem == NULL) || (pInputItem == NULL))
   {
      return false;
   }

   bool bContains = false;
   unsigned int i = 0;

   const vector<WizardNode*>& outputNodes = pOutputItem->getOutputNodes();
   for (i = 0; i < outputNodes.size(); ++i)
   {
      WizardNode* pNode = outputNodes[i];
      if (pNode == pOutputNode)
      {
         bContains = true;
      }
   }

   if (bContains == false)
   {
      return false;
   }

   bContains = false;

   const vector<WizardNode*>& inputNodes = pInputItem->getInputNodes();
   for (i = 0; i < inputNodes.size(); ++i)
   {
      WizardNode* pNode = inputNodes[i];
      if (pNode == pInputNode)
      {
         bContains = true;
      }
   }

   if (bContains == false)
   {
      return false;
   }

   // Get the connection locations
   QPoint ptOutput = pOutputCanvasItem->getNodeConnectionPoint(pOutputNode);
   QPoint ptInput = pInputCanvasItem->getNodeConnectionPoint(pInputNode);
   QColor clrNode = pOutputCanvasItem->getNodeColor(pOutputNode);

   // Create the connection line
   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return false;
   }

   WizardLine* pLine = new WizardLine(pOutputNode, pInputNode, pCanvas);
   if (pLine != NULL)
   {
      pLine->setPen(QPen(clrNode, 3));
      pLine->setPoints(ptOutput.x(), ptOutput.y(), ptInput.x(), ptInput.y());
      pLine->setZ(miLineZ++);
      pLine->show();

      emit itemsConnected(pOutputItem, pInputItem);

      if (bRefresh == true)
      {
         pCanvas->update();
      }

      return true;
   }

   return false;
}

bool WizardView::buildNodeConnections(WizardNode* pNode, bool bRefresh)
{
   if (pNode == NULL)
   {
      return false;
   }

   removeNodeConnections(pNode, false);

   const vector<WizardNode*>& connectedNodes = pNode->getConnectedNodes();
   for (unsigned int i = 0; i < connectedNodes.size(); i++)
   {
      WizardNode* pConnectedNode = connectedNodes[i];
      if (pConnectedNode != NULL)
      {
         bool bAdded = addConnection(pNode, pConnectedNode, false);
         if (bAdded == false)
         {
            addConnection(pConnectedNode, pNode, false);
         }
      }
   }

   return true;
}

bool WizardView::updateConnectionPosition(WizardNode* pOutputNode, WizardNode* pInputNode, bool bRefresh)
{
   if ((pOutputNode == NULL) || (pInputNode == NULL))
   {
      return false;
   }

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return false;
   }

   WizardLine* pLine = NULL;

   Q3CanvasItemList lstAllItems = pCanvas->allItems();
   for (int j = 0; j < lstAllItems.count(); j++)
   {
      Q3CanvasItem* pCanvasItem = lstAllItems[j];
      if (pCanvasItem != NULL)
      {
         if (pCanvasItem->rtti() == 7)
         {
            WizardLine* pCurrentLine = static_cast<WizardLine*>(pCanvasItem);
            if (pCurrentLine != NULL)
            {
               WizardNode* pOutputLineNode = pCurrentLine->getOutputNode();
               WizardNode* pInputLineNode = pCurrentLine->getInputNode();

               if ((pOutputLineNode == pOutputNode) && (pInputLineNode == pInputNode))
               {
                  pLine = pCurrentLine;
                  break;
               }
            }
         }
      }
   }

   if (pLine == NULL)
   {
      return false;
   }

   WizardCanvasItem* pOutputItem = getCanvasItem(pOutputNode);
   WizardCanvasItem* pInputItem = getCanvasItem(pInputNode);

   if ((pOutputItem == NULL) || (pInputItem == NULL))
   {
      return false;
   }

   QPoint ptOutput = pOutputItem->getNodeConnectionPoint(pOutputNode);
   QPoint ptInput = pInputItem->getNodeConnectionPoint(pInputNode);
   pLine->setOutputPoint(ptOutput.x(), ptOutput.y());
   pLine->setInputPoint(ptInput.x(), ptInput.y());

   if (bRefresh == true)
   {
      pCanvas->update();
   }

   return true;
}

void WizardView::updateConnectionPosition(WizardCanvasItem* pWizardItem, bool bRefresh)
{
   if (pWizardItem == NULL)
   {
      return;
   }

   WizardItem* pItem = pWizardItem->getWizardItem();
   if (pItem == NULL)
   {
      return;
   }

   unsigned int i = 0;

   const vector<WizardNode*>& inputNodes = pItem->getInputNodes();
   for (i = 0; i < inputNodes.size(); i++)
   {
      WizardNode* pInputNode = inputNodes[i];
      if (pInputNode != NULL)
      {
         const vector<WizardNode*>& connectedNodes = pInputNode->getConnectedNodes();
         for (unsigned int j = 0; j < connectedNodes.size(); ++j)
         {
            WizardNode* pOutputNode = connectedNodes[j];
            if (pOutputNode != NULL)
            {
               updateConnectionPosition(pOutputNode, pInputNode, false);
            }
         }
      }
   }

   const vector<WizardNode*>& outputNodes = pItem->getOutputNodes();
   for (i = 0; i < outputNodes.size(); ++i)
   {
      WizardNode* pOutputNode = outputNodes[i];
      if (pOutputNode != NULL)
      {
         const vector<WizardNode*>& connectedNodes = pOutputNode->getConnectedNodes();
         for (unsigned int j = 0; j < connectedNodes.size(); ++j)
         {
            WizardNode* pInputNode = connectedNodes[j];
            if (pInputNode != NULL)
            {
               updateConnectionPosition(pOutputNode, pInputNode, false);
            }
         }
      }
   }

   if (bRefresh == true)
   {
      Q3Canvas* pCanvas = canvas();
      if (pCanvas != NULL)
      {
         pCanvas->update();
      }
   }
}

bool WizardView::removeConnection(WizardLine* pLine, bool bRefresh)
{
   if (pLine == NULL)
   {
      return false;
   }

   WizardNode* pInputNode = pLine->getInputNode();
   WizardNode* pOutputNode = pLine->getOutputNode();

   bool bSuccess = removeConnection(pOutputNode, pInputNode, bRefresh);
   return bSuccess;
}

bool WizardView::removeConnection(WizardNode* pOutputNode, WizardNode* pInputNode, bool bRefresh)
{
   if ((pOutputNode == NULL) || (pInputNode == NULL))
   {
      return false;
   }

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return false;
   }

   Q3CanvasItemList lstItems = pCanvas->allItems();
   for (int i = 0; i < lstItems.count(); i++)
   {
      Q3CanvasItem* pCanvasItem = lstItems[i];
      if (pCanvasItem != NULL)
      {
         if (pCanvasItem->rtti() == 7)
         {
            WizardLine* pLine = static_cast<WizardLine*>(pCanvasItem);
            if (pLine != NULL)
            {
               WizardNode* pCurrentOutputNode = pLine->getOutputNode();
               WizardNode* pCurrentInputNode = pLine->getInputNode();

               if ((pCurrentOutputNode == pOutputNode) && (pCurrentInputNode == pInputNode))
               {
                  if (pLine->isSelected() == true)
                  {
                     deselectItem(pLine, false);
                  }

                  delete pLine;
                  return true;
               }
            }
         }
      }
   }

   return false;
}

bool WizardView::removeNodeConnections(WizardNode* pNode, bool bRefresh)
{
   if (pNode == NULL)
   {
      return false;
   }

   Q3Canvas* pCanvas = canvas();
   if (pCanvas == NULL)
   {
      return false;
   }

   int i = 0;

   QList<WizardLine*> lstNodeLines;

   Q3CanvasItemList lstItems = pCanvas->allItems();
   for (i = 0; i < lstItems.count(); ++i)
   {
      Q3CanvasItem* pCanvasItem = lstItems[i];
      if (pCanvasItem != NULL)
      {
         if (pCanvasItem->rtti() == 7)
         {
            WizardLine* pLine = static_cast<WizardLine*>(pCanvasItem);
            if (pLine != NULL)
            {
               WizardNode* pOutputNode = pLine->getOutputNode();
               WizardNode* pInputNode = pLine->getInputNode();

               if ((pOutputNode == pNode) || (pInputNode == pNode))
               {
                  lstNodeLines.append(pLine);
               }
            }
         }
      }
   }

   // Cycle through the lines only since removing a connection also removes the selection nodes
   for (i = 0; i < lstNodeLines.count(); ++i)
   {
      WizardLine* pLine = lstNodeLines[i];
      if (pLine != NULL)
      {
         removeConnection(pLine, false);
      }
   }

   if ((lstNodeLines.empty() == false) && (bRefresh == true))
   {
      pCanvas->update();
   }

   return true;
}

bool WizardView::removeItemConnections(WizardCanvasItem* pWizardItem, bool bRefresh)
{
   if (pWizardItem == NULL)
   {
      return false;
   }

   WizardItem* pItem = pWizardItem->getWizardItem();
   if (pItem == NULL)
   {
      return false;
   }

   // Remove the input node connections
   unsigned int i = 0;

   const vector<WizardNode*>& inputNodes = pItem->getInputNodes();
   for (i = 0; i < inputNodes.size(); ++i)
   {
      WizardNode* pInputNode = inputNodes[i];
      if (pInputNode != NULL)
      {
         const vector<WizardNode*>& connectedNodes = pInputNode->getConnectedNodes();
         for (unsigned int j = 0; j < connectedNodes.size(); ++j)
         {
            WizardNode* pOutputNode = connectedNodes[j];
            if (pOutputNode != NULL)
            {
               removeConnection(pOutputNode, pInputNode, false);
            }
         }
      }
   }

   // Remove the output node connections
   const vector<WizardNode*>& outputNodes = pItem->getOutputNodes();
   for (i = 0; i < outputNodes.size(); ++i)
   {
      WizardNode* pOutputNode = outputNodes[i];
      if (pOutputNode != NULL)
      {
         const vector<WizardNode*>& connectedNodes = pOutputNode->getConnectedNodes();
         for (unsigned int j = 0; j < connectedNodes.size(); ++j)
         {
            WizardNode* pInputNode = connectedNodes[j];
            if (pInputNode != NULL)
            {
               removeConnection(pOutputNode, pInputNode, false);
            }
         }
      }
   }

   if (bRefresh == true)
   {
      // Update the canvas
      Q3Canvas* pCanvas = canvas();
      if (pCanvas != NULL)
      {
         pCanvas->update();
      }
   }

   return true;
}

WizardCanvasItem* WizardView::getCanvasItem(WizardItem* pItem) const
{
   if (pItem == NULL)
   {
      return NULL;
   }

   vector<WizardCanvasItem*> items = getCanvasItems();
   for (unsigned int i = 0; i < items.size(); i++)
   {
      WizardCanvasItem* pWizardCanvasItem = items[i];
      if (pWizardCanvasItem != NULL)
      {
         WizardItem* pCurrentItem = pWizardCanvasItem->getWizardItem();
         if (pCurrentItem == pItem)
         {
            return pWizardCanvasItem;
         }
      }
   }

   return NULL;
}

WizardCanvasItem* WizardView::getCanvasItem(WizardNode* pNode) const
{
   if (pNode == NULL)
   {
      return NULL;
   }

   vector<WizardCanvasItem*> items = getCanvasItems();
   for (unsigned int i = 0; i < items.size(); i++)
   {
      WizardCanvasItem* pWizardCanvasItem = items[i];
      if (pWizardCanvasItem != NULL)
      {
         WizardItemImp* pItem = static_cast<WizardItemImp*>(pWizardCanvasItem->getWizardItem());
         if (pItem != NULL)
         {
            bool bSuccess = pItem->isInputNode(pNode);
            if (bSuccess == false)
            {
               bSuccess = pItem->isOutputNode(pNode);
            }

            if (bSuccess == true)
            {
               return pWizardCanvasItem;
            }
         }
      }
   }

   return NULL;
}

vector<WizardCanvasItem*> WizardView::getCanvasItems() const
{
   vector<WizardCanvasItem*> items;

   Q3Canvas* pCanvas = canvas();
   if (pCanvas != NULL)
   {
      Q3CanvasItemList allItems = pCanvas->allItems();
      for (int i = 0; i < allItems.count(); i++)
      {
         Q3CanvasItem* pItem = allItems[i];
         if (pItem != NULL)
         {
            if (pItem->rtti() == 2)
            {
               items.push_back(static_cast<WizardCanvasItem*>(pItem));
            }
         }
      }
   }

   return items;
}

unsigned int WizardView::getNumCanvasItems() const
{
   vector<WizardCanvasItem*> items = getCanvasItems();
   unsigned int uiItems = items.size();

   return uiItems;
}
