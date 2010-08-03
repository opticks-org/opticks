/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QGraphicsScene>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QWheelEvent>

#include "AppVersion.h"
#include "BatchWizard.h"
#include "ConfigurationSettings.h"
#include "DataVariant.h"
#include "DataVariantEditor.h"
#include "FileResource.h"
#include "NameTypeValueDlg.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "Subject.h"
#include "SystemServicesImp.h"
#include "WizardGraphicsItem.h"
#include "WizardItem.h"
#include "WizardItemImp.h"
#include "WizardLine.h"
#include "WizardNode.h"
#include "WizardNodeImp.h"
#include "WizardUtilities.h"
#include "WizardView.h"

#include <algorithm>

int WizardView::sItemZ = 1000;
int WizardView::sLineZ = 2000;
const double WizardView::sZoomIn = 1.25;              // Zoom in by 25%
const double WizardView::sZoomOut = 1.0 / sZoomIn;    // Zoom out based on the inverse of zoom in
const double WizardView::sZoomMin = 0.1;              // Zoom to 10% of default item width
const double WizardView::sZoomMax = 1.0;              // Zoom to 100% of default item width

WizardView::WizardView(QGraphicsScene* pScene, QWidget* pParent) :
   QGraphicsView(pScene, pParent),
   mpWizard(NULL),
   mFilename(QString()),
   mpZoomRect(NULL),
   mpConnectionLine(NULL),
   mModified(false)
{
   // Initialization
   setFocusPolicy(Qt::StrongFocus);
   setAcceptDrops(true);
   setAlignment(Qt::AlignLeft | Qt::AlignTop);
   setDragMode(QGraphicsView::RubberBandDrag);
   setRubberBandSelectionMode(Qt::ContainsItemShape);
   setResizeAnchor(QGraphicsView::NoAnchor);
   setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

   // Corner widget
   QWidget* pCorner = new QWidget(this);
   pCorner->setAutoFillBackground(true);

   QPalette cornerPalette = pCorner->palette();
   cornerPalette.setColor(QPalette::Window, Qt::lightGray);
   pCorner->setPalette(cornerPalette);

   setCornerWidget(pCorner);

   // Connections
   mpWizard.addSignal(SIGNAL_NAME(WizardObjectImp, ItemAdded), Slot(this, &WizardView::itemAdded));
   mpWizard.addSignal(SIGNAL_NAME(WizardObjectImp, ItemRemoved), Slot(this, &WizardView::itemRemoved));
   mpWizard.addSignal(SIGNAL_NAME(WizardObjectImp, ExecutionOrderChanged),
      Slot(this, &WizardView::executionOrderChanged));
   mpWizard.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &WizardView::setModified));
}

WizardView::~WizardView()
{
   // Prompt the user to save the wizard if necessary
   if ((mModified == true) && (mpWizard.get() != NULL))
   {
      QString name = "Untitled";

      const std::string& wizardName = mpWizard->getName();
      if (wizardName.empty() == false)
      {
         name = QString::fromStdString(wizardName);
      }

      int returnValue = QMessageBox::question(this, "Wizard Builder", "Do you want to save the changes to " +
         name + "?", QMessageBox::Yes | QMessageBox::No);
      if (returnValue == QMessageBox::Yes)
      {
         save();
      }
   }

   // Destroy the items
   clear();
}

bool WizardView::setWizard(const QString& filename)
{
   if (filename == mFilename)
   {
      if (isModified() == false)
      {
         QMessageBox::warning(this, "Wizard Builder", "Reloading the wizard file that is currently open will not "
            "change the wizard from its current state since no modifications have been made.  The file will not be "
            "loaded.");
         return false;
      }

      int retValue =  QMessageBox::question(this, "Wizard Builder", "Reloading the wizard file that is currently open "
         "will cause all modifications to be lost.  Do you want to continue?", QMessageBox::Yes | QMessageBox::No);
      if (retValue == QMessageBox::No)
      {
         return false;
      }
   }

   // Load the wizard from the file
   WizardObject* pWizard = NULL;
   if (filename.isEmpty() == false)
   {
      FactoryResource<WizardObject> pWizardResource(WizardUtilities::readWizard(filename.toStdString()));
      if (pWizardResource.get() == NULL)
      {
         QMessageBox::critical(this, "Wizard Builder", "Could not load the wizard from the file!");
         return false;
      }

      pWizard = pWizardResource.release();
   }

   // Initialize the view from the wizard
   setWizard(pWizard);

   // Update the member filename
   mFilename = filename;
   emit filenameChanged(mFilename);

   return true;
}

void WizardView::setWizard(WizardObject* pWizard)
{
   WizardObjectAdapter* pWizardAdapter = dynamic_cast<WizardObjectAdapter*>(pWizard);
   if (pWizardAdapter == mpWizard.get())
   {
      return;
   }

   // Reset the current wizard
   clear();
   resetMatrix();
   mpWizard.reset(pWizardAdapter);
   mFilename.clear();
   emit filenameChanged(mFilename);

   // Add the new wizard items
   if (mpWizard.get() != NULL)
   {
      const std::vector<WizardItem*>& wizardItems = mpWizard->getItems();
      addItems(wizardItems);
   }

   // Scroll to the upper left corner of the items' bounding box
   QGraphicsScene* pScene = scene();
   if (pScene != NULL)
   {
      QRectF itemRect = pScene->itemsBoundingRect();
      QPointF itemsOrigin = itemRect.topLeft();
      ensureVisible(QRectF(itemsOrigin, itemsOrigin + QPointF(1.0, 1.0)));
   }

   // Update the modified flag
   mModified = false;
}

WizardObject* WizardView::getWizard()
{
   return mpWizard.get();
}

const WizardObject* WizardView::getWizard() const
{
   return mpWizard.get();
}

void WizardView::selectItem(WizardItem* pItem)
{
   WizardGraphicsItem* pGraphicsItem = getGraphicsItem(pItem);
   if (pGraphicsItem != NULL)
   {
      pGraphicsItem->setSelected(true);
   }
}

void WizardView::deselectItem(WizardItem* pItem)
{
   WizardGraphicsItem* pGraphicsItem = getGraphicsItem(pItem);
   if (pGraphicsItem != NULL)
   {
      pGraphicsItem->setSelected(false);
   }
}

std::vector<WizardItem*> WizardView::getSelectedItems(const std::string& type) const
{
   std::vector<WizardItem*> selectedItems;

   QList<QGraphicsItem*> allItems = items();
   for (int i = 0; i < allItems.count(); ++i)
   {
      WizardGraphicsItem* pGraphicsItem = dynamic_cast<WizardGraphicsItem*>(allItems[i]);
      if ((pGraphicsItem != NULL) && (pGraphicsItem->isSelected() == true))
      {
         WizardItem* pItem = pGraphicsItem->getWizardItem();
         VERIFYRV(pItem != NULL, std::vector<WizardItem*>());
         if (type.empty() == true || type == pItem->getType())
         {
            selectedItems.push_back(pItem);
         }
      }
   }

   return selectedItems;
}

bool WizardView::isItemSelected(WizardItem* pItem) const
{
   WizardGraphicsItem* pGraphicsItem = getGraphicsItem(pItem);
   if (pGraphicsItem != NULL)
   {
      return pGraphicsItem->isSelected();
   }

   return false;
}

bool WizardView::editItems()
{
   std::vector<WizardItem*> selectedItems = getSelectedItems("Value");
   if (selectedItems.empty() == true)
   {
      QMessageBox::warning(this, "Error", "Please select one or more value items to edit.");
      return false;
   }

   if (selectedItems.size() == 1)
   {
      return editItem(selectedItems.front());
   }

   return WizardUtilities::editItems(selectedItems, this);
}

bool WizardView::editItem(WizardItem* pItem)
{
   WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
   if (pItem == NULL)
   {
      return false;
   }

   const std::string& itemType = pItem->getType();
   if (itemType != "Value")
   {
      return false;
   }

   const std::vector<WizardNode*>& outputNodes = pItemImp->getOutputNodes();
   VERIFY(outputNodes.empty() == false);

   WizardNodeImp* pNode = static_cast<WizardNodeImp*>(outputNodes.front());
   VERIFY(pNode != NULL);

   const std::string& nodeName = pNode->getName();
   const std::string& nodeType = pNode->getType();
   void* pValue = pNode->getValue();

   NameTypeValueDlg dlgValue(this);
   dlgValue.setWindowTitle("Wizard Value Item");
   dlgValue.setValue(QString::fromStdString(nodeName), DataVariant(nodeType, pValue));

   if (dlgValue.exec() == QDialog::Accepted)
   {
      QString strName = dlgValue.getName();
      QString strType = dlgValue.getType();
      if ((strName.isEmpty() == true) || (strType.isEmpty() == true))
      {
         return false;
      }

      std::string newNodeName = strName.toStdString();
      std::string newNodeType = strType.toStdString();
      const DataVariant& newNodeValue = dlgValue.getValue();

      if (newNodeName != nodeName)
      {
         pNode->setName(newNodeName);
         pItemImp->setName(newNodeName);
      }

      if (newNodeType != nodeType)
      {
         std::vector<std::string> validTypes;
         validTypes.push_back(newNodeType);

         pNode->setOriginalType(newNodeType);
         pNode->setType(newNodeType);
         pNode->setValidTypes(validTypes);
         pNode->clearConnectedNodes();
      }

      if (newNodeValue.getPointerToValueAsVoid() != pValue)
      {
         pNode->setValue(newNodeValue.getPointerToValueAsVoid());
      }
      return true;
   }
   return false;
}

bool WizardView::isModified() const
{
   return mModified;
}

void WizardView::selectAllItems()
{
   std::vector<WizardGraphicsItem*> items = getGraphicsItems();
   for (std::vector<WizardGraphicsItem*>::iterator iter = items.begin(); iter != items.end(); ++iter)
   {
      WizardGraphicsItem* pGraphicsItem = *iter;
      if (pGraphicsItem != NULL)
      {
         pGraphicsItem->setSelected(true);
      }
   }
}

void WizardView::deselectAllItems()
{
   std::vector<WizardGraphicsItem*> items = getGraphicsItems();
   for (std::vector<WizardGraphicsItem*>::iterator iter = items.begin(); iter != items.end(); ++iter)
   {
      WizardGraphicsItem* pGraphicsItem = *iter;
      if (pGraphicsItem != NULL)
      {
         pGraphicsItem->setSelected(false);
      }
   }
}

void WizardView::removeSelectedItems()
{
   // Connections
   QList<QGraphicsItem*> graphicsItems = items();
   for (int i = 0; i < graphicsItems.count(); i++)
   {
      WizardLine* pLineItem = dynamic_cast<WizardLine*>(graphicsItems[i]);
      if ((pLineItem != NULL) && (pLineItem->isSelected() == true))
      {
         WizardNodeImp* pOutputNode = static_cast<WizardNodeImp*>(pLineItem->getOutputNode());
         WizardNodeImp* pInputNode = static_cast<WizardNodeImp*>(pLineItem->getInputNode());
         if ((pOutputNode != NULL) && (pInputNode != NULL))
         {
            pOutputNode->removeConnectedNode(pInputNode);
            pInputNode->removeConnectedNode(pOutputNode);
         }
      }
   }

   // Items
   if (mpWizard.get() != NULL)
   {
      std::vector<WizardItem*> selectedItems = getSelectedItems();
      for (std::vector<WizardItem*>::iterator iter = selectedItems.begin(); iter != selectedItems.end(); ++iter)
      {
         WizardItem* pItem = *iter;
         if (pItem != NULL)
         {
            mpWizard->removeItem(pItem);
         }
      }
   }
}

void WizardView::zoomIn()
{
   zoomBy(sZoomIn);
}

void WizardView::zoomOut()
{
   zoomBy(sZoomOut);
}

void WizardView::zoomToFit()
{
   QGraphicsScene* pScene = scene();
   VERIFYNRV(pScene != NULL);

   QRectF itemRect = pScene->itemsBoundingRect();
   zoomTo(itemRect);
}

void WizardView::zoomBy(double factor)
{
   zoomTo(matrix().m11() * factor);
}

void WizardView::zoomTo(double factor)
{
   // Adjust the zoom factor to be within preset limits
   QMatrix currentMatrix = matrix();

   double currentFactor = currentMatrix.m11();
   if (currentFactor < sZoomMin)
   {
      // Do not allow the zoom to be less than the current factor
      factor = std::max(factor, currentFactor);
   }
   else if (currentFactor > sZoomMax)
   {
      // Do not allow the zoom to be greater than the current factor
      factor = std::min(factor, currentFactor);
   }
   else
   {
      // Limit the zoom between the minimum and maximum
      factor = std::min(factor, sZoomMax);
      factor = std::max(factor, sZoomMin);
   }

   // Apply the zoom to the view
   QMatrix zoomedMatrix(factor, currentMatrix.m12(), currentMatrix.m21(), factor, currentMatrix.dx(),
      currentMatrix.dy());
   setMatrix(zoomedMatrix);
}

void WizardView::zoomTo(const QRectF& sceneRect)
{
   if (sceneRect.isValid() == false)
   {
      return;
   }

   // Zoom to the specified area
   fitInView(sceneRect, Qt::KeepAspectRatio);

   // Restrict to the maximum zoom
   if (matrix().m11() > sZoomMax)
   {
      resetZoom();
   }
}

void WizardView::resetZoom()
{
   zoomTo(sZoomMax);
}

void WizardView::print()
{
   QList<QGraphicsItem*> graphicsItems = items();
   if (graphicsItems.empty() == true)
   {
      QMessageBox::warning(this, "Wizard Builder", "The wizard is currently empty and will not be printed.");
      return;
   }

   QGraphicsScene* pScene = scene();
   VERIFYNRV(pScene != NULL);

   QRectF itemRect = pScene->itemsBoundingRect();
   if (itemRect.isNull() == true)
   {
      QMessageBox::warning(this, "Wizard Builder", "The wizard is currently empty and will not be printed.");
      return;
   }

   QPrinter wizardPrinter(QPrinter::ScreenResolution);
   SystemServicesImp::instance()->WriteLogInfo(std::string(APP_NAME) + " is printing a wizard");

   QPrintDialog dlg(&wizardPrinter, this);
   if (dlg.exec() == QDialog::Accepted)
   {
      QPainter painter(&wizardPrinter);

      double itemWidth = itemRect.width();
      double itemHeight = itemRect.height();
      double itemAspect = itemWidth / itemHeight;

      QRect printRect = painter.viewport();
      double printWidth = static_cast<double>(printRect.width());
      double printHeight = static_cast<double>(printRect.height());
      double printAspect = printWidth / printHeight;

      QRectF targetRect(printWidth / 2.0 - (printHeight * itemAspect) / 2.0, printRect.y(),
         printHeight * itemAspect, printHeight);
      if (itemWidth <= printWidth)
      {
         if (itemHeight <= printHeight)
         {
            targetRect.setRect(printWidth / 2.0 - itemWidth / 2.0, printHeight / 2.0 - itemHeight / 2.0,
               itemWidth, itemHeight);
         }
      }
      else if ((itemHeight <= printHeight) || (itemAspect <= printAspect))
      {
         targetRect.setRect(printRect.x(), printHeight / 2.0 - (printWidth / itemAspect) / 2.0,
            printWidth, printWidth / itemAspect);
      }

      pScene->render(&painter, targetRect, itemRect);
   }
}

void WizardView::execute()
{
   if (mpWizard.get() != NULL)
   {
      WizardUtilities::runWizard(mpWizard.get());
   }
}

bool WizardView::save()
{
   bool success = false;
   if (mFilename.isEmpty() == false)
   {
      success = save(mFilename);
      if (success == true)
      {
         mModified = false;
      }
   }
   else
   {
      success = saveAs();
   }

   return success;
}

bool WizardView::saveAs()
{
   if (mpWizard.get() == NULL)
   {
      return false;
   }

   // Get the default wizard directory and create a default filename
   QString defaultDir = QDir::currentPath();
   QString defaultFile;

   if (mFilename.isEmpty() == false)
   {
      QFileInfo fileInfo = QFileInfo(mFilename);
      defaultDir = fileInfo.absolutePath();
      defaultFile = fileInfo.completeBaseName();
   }
   else
   {
      const Filename* pWizardPath = ConfigurationSettings::getSettingWizardPath();
      if (pWizardPath != NULL)
      {
         defaultDir = QString::fromStdString(pWizardPath->getFullPathAndName());
      }

      std::string wizardName = mpWizard->getName();
      if (wizardName.empty() == false)
      {
         defaultFile = QString::fromStdString(wizardName);
      }
   }

   if (defaultFile.isEmpty() == false)
   {
      defaultFile += ".wiz";
   }

   // Invoke a file selection dialog and get the save filename
   QFileDialog dlg(this, "Save Wizard", defaultDir, "Wizard Files (*.wiz);;All Files (*)");
   dlg.setAcceptMode(QFileDialog::AcceptSave);
   dlg.setFileMode(QFileDialog::AnyFile);
   dlg.setConfirmOverwrite(true);
   dlg.setDefaultSuffix("wiz");
   dlg.selectFile(defaultFile);

   QString filename;
   if (dlg.exec() == QDialog::Accepted)
   {
      filename = dlg.selectedFiles().front();
   }

   if (filename.isEmpty() == true)
   {
      return false;
   }

   // Save the wizard
   bool success = save(filename);
   if (success == true)
   {
      // Set the member filename
      if (filename != mFilename)
      {
         mFilename = filename;
         emit filenameChanged(mFilename);
      }

      mModified = false;
   }

   return success;
}

void WizardView::keyPressEvent(QKeyEvent* pEvent)
{
   if ((pEvent != NULL) && (pEvent->key() == Qt::Key_Delete))
   {
      removeSelectedItems();
   }

   if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
   {
      setCursor(QCursor(QPixmap(":/icons/ZoomRectCursor"), 0, 0));
   }

   QGraphicsView::keyPressEvent(pEvent);
}

void WizardView::keyReleaseEvent(QKeyEvent* pEvent)
{
   setCursor(Qt::ArrowCursor);
}

void WizardView::dragEnterEvent(QDragEnterEvent* pEvent)
{
   dragMoveEvent(pEvent);
}

void WizardView::dragMoveEvent(QDragMoveEvent* pEvent)
{
   if (pEvent != NULL)
   {
      const QMimeData* pData = pEvent->mimeData();
      if ((pData != NULL) && (pData->hasFormat("text/x-wizarditem") == true))
      {
         if (pEvent->proposedAction() == Qt::CopyAction)
         {
            pEvent->acceptProposedAction();
         }
         else
         {
            pEvent->setDropAction(Qt::CopyAction);
            pEvent->accept();
         }
      }
   }
}

void WizardView::dropEvent(QDropEvent* pEvent)
{
   if ((mpWizard.get() == NULL) || (pEvent == NULL))
   {
      return;
   }

   const QMimeData* pData = pEvent->mimeData();
   if (pData == NULL)
   {
      return;
   }

   QString dataText = QString::fromAscii(pData->data("text/x-wizarditem"));
   if (dataText.isEmpty() == true)
   {
      return;
   }

   int itemsAdded = 0;
   QPointF scenePos = mapToScene(pEvent->pos());

   QStringList itemNames = dataText.split("@@!!@@");
   for (int i = 0; i < itemNames.count(); ++i)
   {
      QString itemText = itemNames[i];
      QString itemName = itemText;
      QString itemType;

      int pos = itemText.indexOf("/");
      if (pos != -1)
      {
         itemName = itemText.mid(pos + 1);
         itemType = itemText.left(pos);
      }

      WizardItemImp* pItem = NULL;
      if (itemType == "Value")
      {
         NameTypeValueDlg dlgValue(this);
         dlgValue.setWindowTitle("Wizard Value Item");
         dlgValue.setEmptyValue(QString(), itemName);
         if (dlgValue.exec() == QDialog::Accepted)
         {
            pItem = static_cast<WizardItemImp*>(mpWizard->addValueItem(dlgValue.getName().toStdString(),
               dlgValue.getValue()));
         }
      }
      else
      {
         pItem = static_cast<WizardItemImp*>(mpWizard->addPlugInItem(itemName.toStdString(), itemType.toStdString()));
      }

      if (pItem != NULL)
      {
         // Set the item position
         pItem->setPosition(scenePos.x(), scenePos.y());
         scenePos += QPointF(20.0, 20.0);     // Cascade mulitple items

         ++itemsAdded;
      }
   }

   // Accept the drop
   if (itemsAdded > 0)
   {
      if (pEvent->proposedAction() == Qt::CopyAction)
      {
         pEvent->acceptProposedAction();
      }
      else if (pEvent->dropAction() == Qt::CopyAction)
      {
         pEvent->accept();
      }
   }
}

void WizardView::enterEvent(QEvent* pEvent)
{
   QGraphicsView::enterEvent(pEvent);
   if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
   {
      setCursor(QCursor(QPixmap(":/icons/ZoomRectCursor"), 0, 0));
   }
}

void WizardView::mousePressEvent(QMouseEvent* pEvent)
{
   mpZoomRect.reset(NULL);
   mpConnectionLine.reset(NULL);

   QGraphicsScene* pScene = scene();
   if ((pEvent != NULL) && (pScene != NULL))
   {
      mMousePressPoint = pEvent->pos();
      QPointF scenePos = mapToScene(mMousePressPoint);

      if (pEvent->button() == Qt::LeftButton)
      {
         if (pEvent->modifiers() & Qt::ShiftModifier)
         {
            mpZoomRect.reset(new QRubberBand(QRubberBand::Rectangle, this));
            mpZoomRect->setGeometry(QRect(mMousePressPoint, QSize()));
            mpZoomRect->show();
            return;
         }
         else
         {
            WizardGraphicsItem* pGraphicsItem = getGraphicsItem(mMousePressPoint);
            if (pGraphicsItem != NULL)
            {
               WizardNode* pNode = pGraphicsItem->getNode(scenePos);
               if (pNode != NULL)
               {
                  // Get the line points
                  QPointF connectPos = pGraphicsItem->getNodeConnectionPoint(pNode);
                  if (connectPos.isNull() == false)
                  {
                     bool outputNode = true;

                     WizardItemImp* pItem = static_cast<WizardItemImp*>(pGraphicsItem->getWizardItem());
                     if (pItem != NULL)
                     {
                        outputNode = pItem->isOutputNode(pNode);
                     }

                     QColor lineColor = WizardGraphicsItem::getNodeColor(pNode);

                     // Create the connection line
                     mpConnectionLine.reset(new WizardLine(outputNode ? pNode : NULL, outputNode ? NULL : pNode));
                     mpConnectionLine->setPen(QPen(lineColor, 3));
                     mpConnectionLine->setLine(QLineF(connectPos, connectPos));
                     mpConnectionLine->setZValue(sLineZ++);
                     pScene->addItem(mpConnectionLine.get());
                     return;
                  }
               }
            }
         }
      }
      else if (pEvent->button() == Qt::RightButton)
      {
         // Check if the user clicked on an item
         QList<QGraphicsItem*> itemsAtPos = items(mMousePressPoint);
         for (int i = 0; i < itemsAtPos.count(); ++i)
         {
            QGraphicsItem* pGraphicsItem = itemsAtPos[i];
            if (pGraphicsItem != NULL)
            {
               // Update the selected items and block the event since the default behavior is to clear
               // the selection when the right mouse button is clicked
               if (pGraphicsItem->isSelected() == false)
               {
                  if ((pEvent->modifiers() & Qt::ControlModifier) == 0)
                  {
                     pScene->clearSelection();
                  }

                  pGraphicsItem->setSelected(true);
               }

               return;
            }
         }
      }
   }

   QGraphicsView::mousePressEvent(pEvent);
}

void WizardView::mouseMoveEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      QPoint viewPos = pEvent->pos();
      if (mpZoomRect.get() != NULL)
      {
         QRect viewRect = QRect(std::min(mMousePressPoint.x(), viewPos.x()),
            std::min(mMousePressPoint.y(), viewPos.y()), abs(mMousePressPoint.x() - viewPos.x()) + 1,
            abs(mMousePressPoint.y() - viewPos.y()) + 1);
         mpZoomRect->setGeometry(viewRect);
      }

      if (mpConnectionLine.get() != NULL)
      {
         QPointF connectPos = mpConnectionLine->line().p1();
         QPointF scenePos = mapToScene(viewPos);
         mpConnectionLine->setLine(QLineF(connectPos, scenePos));
      }
   }

   QGraphicsView::mouseMoveEvent(pEvent);
}

void WizardView::mouseReleaseEvent(QMouseEvent* pEvent)
{
   if (mpZoomRect.get() != NULL)
   {
      QPolygonF scenePolygon = mapToScene(mpZoomRect->geometry());
      QRectF sceneRect = scenePolygon.boundingRect();
      zoomTo(sceneRect);
      centerOn(sceneRect.center());
      mpZoomRect.reset(NULL);
   }

   if (mpConnectionLine.get() != NULL)
   {
      if (pEvent != NULL)
      {
         QPoint viewPos = pEvent->pos();
         QPointF scenePos = mapToScene(viewPos);

         WizardGraphicsItem* pGraphicsItem = getGraphicsItem(viewPos);
         if (pGraphicsItem != NULL)
         {
            WizardNodeImp* pNode = static_cast<WizardNodeImp*>(pGraphicsItem->getNode(scenePos));
            if (pNode != NULL)
            {
               // Get the input and output nodes
               WizardNodeImp* pOutputNode = static_cast<WizardNodeImp*>(mpConnectionLine->getOutputNode());
               if (pOutputNode == NULL)
               {
                  pOutputNode = pNode;
               }

               WizardNodeImp* pInputNode = static_cast<WizardNodeImp*>(mpConnectionLine->getInputNode());
               if (pInputNode == NULL)
               {
                  pInputNode = pNode;
               }

               // Check for possible connection errors
               if ((pOutputNode != pInputNode) && (pOutputNode != NULL) && (pInputNode != NULL))
               {
                  WizardItemImp* pInputItem = static_cast<WizardItemImp*>(pInputNode->getItem());
                  WizardItemImp* pOutputItem = static_cast<WizardItemImp*>(pOutputNode->getItem());
                  VERIFYNRV((pInputItem != NULL) && (pOutputItem != NULL));

                  bool connectNodes = true;

                  // Check for connections between two input nodes or two output nodes
                  if ((pInputItem->isInputNode(pInputNode) == false) ||
                     (pOutputItem->isOutputNode(pOutputNode) == false))
                  {
                     QMessageBox::warning(this, "Wizard Builder", "An input node cannot be connected to another "
                        "input node and an output node cannot be connected to another output node.  Please connect "
                        "an input node to an output node.");
                     connectNodes = false;
                  }

                  // Check for circular connections between the two items
                  if ((connectNodes == true) && (pOutputItem->isItemConnected(pInputItem, true) == true))
                  {
                     QMessageBox::warning(this, "Wizard Builder", "These nodes cannot be connected "
                        "since a circular connection between the two items would be created.");
                     connectNodes = false;
                  }

                  // Check if the nodes are already connected
                  if ((connectNodes == true) && (pOutputNode->isNodeConnected(pInputNode) == true))
                  {
                     connectNodes = false;
                  }

                  // Check for multiple output nodes connected to the same input node
                  if ((connectNodes == true) && (pInputNode->getNumConnectedNodes() > 0))
                  {
                     QMessageBox::warning(this, "Wizard Builder", "This node already has a connection. "
                        "The connection will not be made.");
                     connectNodes = false;
                  }

                  // Connect the nodes
                  if (connectNodes == true)
                  {
                     pOutputNode->addConnectedNode(pInputNode);
                     pInputNode->addConnectedNode(pOutputNode);
                  }
               }
            }
         }
      }

      // Remove the connection line
      mpConnectionLine.reset(NULL);
   }

   QGraphicsView::mouseReleaseEvent(pEvent);
}

void WizardView::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
   if (pEvent == NULL)
   {
      return;
   }

   QPoint viewPos = pEvent->pos();

   WizardGraphicsItem* pGraphicsItem = getGraphicsItem(viewPos);
   if (pGraphicsItem == NULL)
   {
      return;
   }

   if (pEvent->button() == Qt::LeftButton)
   {
      WizardItemImp* pItem = static_cast<WizardItemImp*>(pGraphicsItem->getWizardItem());
      if (pItem != NULL)
      {
         const std::string& itemType = pItem->getType();
         if (itemType != "Value")
         {
            QPointF scenePos = mapToScene(viewPos);
            WizardNodeImp* pNode = static_cast<WizardNodeImp*>(pGraphicsItem->getNode(scenePos));
            if (pNode != NULL && pItem->isInputNode(pNode))
            {
               if (!pNode->getConnectedNodes().empty())
               {
                  QMessageBox::warning(this, "Wizard Builder",
                     "Unable to create a connection since the node already has an existing connection.");
                  return;
               }

               const std::string& nodeName = pNode->getName();
               const std::string& nodeType = pNode->getType();

               WizardNodeImp* pNewNode = NULL;
               WizardItemImp* pNewItem = NULL;
               if (DataVariantEditor::hasDelegate(nodeType))
               {
                  // create a new value item
                  void* pNodeValue = pNode->getValue();

                  // If the wizard has already been executed, the node value will be reset to NULL,
                  // so use the default value from the plug-in arg as the initial value for the new value item
                  PlugInDescriptor* pPlugInDesc =
                     Service<PlugInManagerServices>()->getPlugInDescriptor(pItem->getName());
                  if (pPlugInDesc != NULL)
                  {
                     const PlugInArgList* pList = pItem->getBatchMode() ?
                                    pPlugInDesc->getBatchInputArgList() :
                                    pPlugInDesc->getInteractiveInputArgList();
                     if (pList != NULL)
                     {
                        PlugInArg* pArg = NULL;
                        if (pList->getArg(nodeName, pArg) && pArg != NULL && pArg->isDefaultSet())
                        {
                           pNodeValue = pArg->getDefaultValue();
                        }
                     }
                  }

                  DataVariant initialValue(nodeType, pNodeValue);
                  if (pNodeValue == NULL)
                  {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This should error; " \
   "fix this when we can have a DataVariant with a type and an invalid value (tclarke)")
                     QString type = QString::fromStdString(nodeType);
                     if ((type.contains("char") ||
                          type.contains("short") ||
                          type.contains("int") ||
                          type.contains("long") ||
                          type.contains("float") ||
                          type.contains("double")) && !type.contains("vector"))
                     {
                        initialValue.fromDisplayString(nodeType, "0");
                     }
                  }

                  pNewItem = static_cast<WizardItemImp*>(mpWizard->addValueItem(nodeName, initialValue));
                  if (pNewItem != NULL)
                  {
                     pNewNode = static_cast<WizardNodeImp*>(pNewItem->getOutputNodes().front());
                  }
               }
               else
               {
                  // create a GetSessionItem
                  pNewItem = static_cast<WizardItemImp*>(mpWizard->addPlugInItem("Get " + nodeType, "Wizard"));
                  if (pNewItem != NULL)
                  {
                     pNewNode = static_cast<WizardNodeImp*>(pNewItem->getOutputNode("Session Item", nodeType));
                  }
               }

               if (pNewItem == NULL || pNewNode == NULL)
               {
                  mpWizard->removeItem(pNewItem);
                  QMessageBox::warning(this, "Wizard Builder", "Unable to automatically create a connection.");
               }
               else
               {
                  WizardGraphicsItem* pNewGraphicsItem = getGraphicsItem(pNewItem);
                  WizardGraphicsItem* pGraphicsItem = getGraphicsItem(pItem);
                  VERIFYNRV(pNewGraphicsItem && pGraphicsItem);

                  QPointF nodeSceneLocation = pGraphicsItem->getNodeConnectionPoint(pNode);
                  QPointF valueNodeOffset = pNewGraphicsItem->mapFromScene(
                     pNewGraphicsItem->getNodeConnectionPoint(pNewNode));
                  valueNodeOffset += QPointF(60, 0); // magic number based on visual trials
                  nodeSceneLocation -= valueNodeOffset;
                  pNewItem->setPosition(nodeSceneLocation.x(), nodeSceneLocation.y());

                  // connect the value item output node to the input node
                  pNode->addConnectedNode(pNewNode);
                  pNewNode->addConnectedNode(pNode);

                  // edit value items now that the wizard item is in place and connected
                  if (pNewItem->getType() == "Value" && editItem(pNewItem) == false)
                  {
                     mpWizard->removeItem(pNewItem);
                  }
               }
            }
            else
            {
               QGraphicsView::mouseDoubleClickEvent(pEvent);
            }
         }
         else
         {
            editItem(pItem);
         }
      }
   }
}

void WizardView::contextMenuEvent(QContextMenuEvent* pEvent)
{
   if ((pEvent == NULL) || (mpWizard.get() == NULL))
   {
      return;
   }

   QGraphicsScene* pScene = scene();
   VERIFYNRV(pScene != NULL);

   std::vector<WizardGraphicsItem*> items = getGraphicsItems();
   std::vector<WizardItem*> selectedWizardItems = getSelectedItems();
   QList<QGraphicsItem*> selectedItems = pScene->selectedItems();

   if (selectedWizardItems.empty() == false)
   {
      // Create the selected items menu
      QMenu contextMenu(this);

      QAction* pBatchAction = contextMenu.addAction("Set &Batch Mode");
      QAction* pInteractiveAction = contextMenu.addAction("Set &Interactive Mode");
      contextMenu.addSeparator();

      if (mpWizard->isBatch() == true)
      {
         pBatchAction->setEnabled(false);
         pInteractiveAction->setEnabled(false);
      }

      bool valueItems = false;
      bool connections = false;
      for (std::vector<WizardItem*>::iterator iter = selectedWizardItems.begin();
         iter != selectedWizardItems.end();
         ++iter)
      {
         WizardItemImp* pItem = static_cast<WizardItemImp*>(*iter);
         if (pItem != NULL)
         {
            if (pItem->getType() == "Value")
            {
               valueItems = true;
               if (connections == true)
               {
                  break;
               }
            }

            std::vector<WizardItem*> connectedItems;
            pItem->getConnectedItems(true, connectedItems);
            if (connectedItems.empty() == false)
            {
               connections = true;
               if (valueItems == true)
               {
                  break;
               }
            }

            pItem->getConnectedItems(false, connectedItems);
            if (connectedItems.empty() == false)
            {
               connections = true;
               if (valueItems == true)
               {
                  break;
               }
            }
         }
      }

      QAction* pEditAction = NULL;
      if (valueItems == true)
      {
         pEditAction = contextMenu.addAction(QIcon(":/icons/ValueEdit"), "&Edit...");
         contextMenu.addSeparator();
      }

      QAction* pRemoveConnectionsAction = NULL;
      if (connections == true)
      {
         pRemoveConnectionsAction = contextMenu.addAction("&Remove Connections");
      }

      QAction* pDeleteAction = contextMenu.addAction(QIcon(":/icons/Delete"), "&Delete");

      // Invoke the menu
      QAction* pInvokedAction = contextMenu.exec(pEvent->globalPos());
      if (pInvokedAction != NULL)
      {
         if (pInvokedAction == pDeleteAction)
         {
            removeSelectedItems();
         }
         else if (pInvokedAction == pEditAction)
         {
            editItems();
         }
         else
         {
            for (std::vector<WizardItem*>::iterator iter = selectedWizardItems.begin();
               iter != selectedWizardItems.end();
               ++iter)
            {
               WizardItemImp* pItem = static_cast<WizardItemImp*>(*iter);
               if (pItem != NULL)
               {
                  if (pInvokedAction == pBatchAction)
                  {
                     pItem->setBatchMode(true);
                  }
                  else if (pInvokedAction == pInteractiveAction)
                  {
                     pItem->setBatchMode(false);
                  }
                  else if (pInvokedAction == pRemoveConnectionsAction)
                  {
                     // Input nodes
                     std::vector<WizardNode*> inputNodes = pItem->getInputNodes();
                     for (unsigned int i = 0; i < inputNodes.size(); i++)
                     {
                        WizardNodeImp* pNode = static_cast<WizardNodeImp*>(inputNodes[i]);
                        if (pNode != NULL)
                        {
                           pNode->clearConnectedNodes();
                        }
                     }

                     // Output nodes
                     std::vector<WizardNode*> outputNodes = pItem->getOutputNodes();
                     for (unsigned int i = 0; i < outputNodes.size(); i++)
                     {
                        WizardNodeImp* pNode = static_cast<WizardNodeImp*>(outputNodes[i]);
                        if (pNode != NULL)
                        {
                           pNode->clearConnectedNodes();
                        }
                     }
                  }
               }
            }
         }
      }
   }
   else if (selectedItems.empty() == false)
   {
      // Create the selected connections menu
      QMenu contextMenu(this);
      QAction* pDeleteAction = contextMenu.addAction(QIcon(":/icons/Delete"), "&Delete");

      // Invoke the menu
      QAction* pInvokedAction = contextMenu.exec(pEvent->globalPos());
      if ((pInvokedAction != NULL) && (pInvokedAction == pDeleteAction))
      {
         removeSelectedItems();
      }
   }
   else
   {
      // Create the wizard menu
      QMenu contextMenu(this);

      QAction* pSelectAllAction = contextMenu.addAction("Se&lect All", this, SLOT(selectAllItems()));
      contextMenu.addSeparator();
      QAction* pBatchAction = contextMenu.addAction("&Batch Mode");
      QAction* pInteractiveAction = contextMenu.addAction("&Interactive Mode");
      contextMenu.addSeparator();
      QAction* pZoomInAction = contextMenu.addAction(QIcon(":/icons/ZoomIn"), "&Zoom In", this, SLOT(zoomIn()));
      QAction* pZoomOutAction = contextMenu.addAction(QIcon(":/icons/ZoomOut"), "Zoom &Out", this, SLOT(zoomOut()));
      QAction* pZoomToFitAction = contextMenu.addAction(QIcon(":/icons/ZoomToFit"), "Zoom to &Fit",
         this, SLOT(zoomToFit()));
      contextMenu.addSeparator();
      QAction* pExecuteAction = contextMenu.addAction("&Execute", this, SLOT(execute()));
      contextMenu.addSeparator();
      QAction* pClearAction = contextMenu.addAction("&Clear");
      contextMenu.addSeparator();
      QAction* pSaveAction = contextMenu.addAction(QIcon(":/icons/Save"), "&Save", this, SLOT(save()));
      QAction* pPrintAction = contextMenu.addAction(QIcon(":/icons/Print"), "&Print", this, SLOT(print()));

      pBatchAction->setCheckable(true);
      pInteractiveAction->setCheckable(true);

      if (mpWizard->isBatch() == true)
      {
         pBatchAction->setChecked(true);
      }
      else
      {
         pInteractiveAction->setChecked(true);
      }

      // Invoke the menu
      QAction* pInvokedAction = contextMenu.exec(pEvent->globalPos());
      if (pInvokedAction == pBatchAction)
      {
         mpWizard->setBatch(true);
      }
      else if (pInvokedAction == pInteractiveAction)
      {
         mpWizard->setBatch(false);
      }
      else if (pInvokedAction == pClearAction)
      {
         mpWizard->clear();
      }
   }
}

void WizardView::wheelEvent(QWheelEvent* pEvent)
{
   if (pEvent == NULL)
   {
      return;
   }

   if (pEvent->modifiers() & Qt::ControlModifier)
   {
      QGraphicsView::wheelEvent(pEvent);
      return;
   }

   double factor = pEvent->delta() / 120.0;
   if (factor < 0.0)
   {
      if (ConfigurationSettings::getSettingAlternateMouseWheelZoom())
      {
         factor *= -sZoomIn;
      }
      else
      {
         factor *= -sZoomOut;
      }
   }
   else
   {
      if (ConfigurationSettings::getSettingAlternateMouseWheelZoom())
      {
         factor *= sZoomOut;
      }
      else
      {
         factor *= sZoomIn;
      }
   }

   zoomBy(factor);
}

void WizardView::drawItems(QPainter* pPainter, int numItems, QGraphicsItem* items[],
                           const QStyleOptionGraphicsItem options[])
{
   if (numItems > 0)
   {
      std::vector<QStyleOptionGraphicsItem> styleOptions(numItems);
      for (int i = 0; i < numItems; ++i)
      {
         QStyleOptionGraphicsItem option = options[i];
         option.state &= ~(QStyle::State_Selected);

         styleOptions[i] = option;
      }

      QGraphicsView::drawItems(pPainter, numItems, items, &styleOptions[0]);
      return;
   }

   QGraphicsView::drawItems(pPainter, numItems, items, options);
}

void WizardView::itemAdded(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(&subject);
   VERIFYNRV(pWizard != NULL);
   VERIFYNRV(pWizard == mpWizard.get());

   WizardItem* pItem = boost::any_cast<WizardItem*>(data);
   VERIFYNRV(pItem != NULL);

   // Add the item to the view
   addItem(pItem);
}

void WizardView::itemRemoved(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(&subject);
   VERIFYNRV(pWizard != NULL);
   VERIFYNRV(pWizard == mpWizard.get());

   WizardItem* pItem = boost::any_cast<WizardItem*>(data);
   VERIFYNRV(pItem != NULL);

   // Remove the item from the view
   removeItem(pItem);
}

void WizardView::executionOrderChanged(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(&subject);
   VERIFYNRV(pWizard != NULL);
   VERIFYNRV(pWizard == mpWizard.get());

   // Update the execution order on all graphic items
   updateExecutionOrder();
}

void WizardView::itemPositionChanged(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardItem* pItem = dynamic_cast<WizardItem*>(&subject);
   if (pItem != NULL)
   {
      updateConnectionPosition(pItem);
   }
}

void WizardView::nodeAdded(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardNodeImp* pNode = static_cast<WizardNodeImp*>(boost::any_cast<WizardNode*>(data));
   if (pNode != NULL)
   {
      VERIFYNR(pNode->attach(SIGNAL_NAME(WizardNodeImp, NodeConnected), Slot(this, &WizardView::nodeConnected)));
      VERIFYNR(pNode->attach(SIGNAL_NAME(WizardNodeImp, NodeDisconnected), Slot(this, &WizardView::nodeDisconnected)));
      VERIFYNR(pNode->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &WizardView::setModified)));
   }
}

void WizardView::nodeRemoved(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardNodeImp* pNode = static_cast<WizardNodeImp*>(boost::any_cast<WizardNode*>(data));
   if (pNode != NULL)
   {
      VERIFYNR(pNode->detach(SIGNAL_NAME(WizardNodeImp, NodeConnected), Slot(this, &WizardView::nodeConnected)));
      VERIFYNR(pNode->detach(SIGNAL_NAME(WizardNodeImp, NodeDisconnected), Slot(this, &WizardView::nodeDisconnected)));
      VERIFYNR(pNode->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &WizardView::setModified)));

      const std::vector<WizardNode*>& connectedNodes = pNode->getConnectedNodes();
      for (std::vector<WizardNode*>::const_iterator iter = connectedNodes.begin(); iter != connectedNodes.end(); ++iter)
      {
         WizardNode* pConnectedNode = *iter;
         if (pConnectedNode != NULL)
         {
            WizardItemImp* pItem = static_cast<WizardItemImp*>(pNode->getItem());
            if (pItem != NULL)
            {
               if (pItem->isInputNode(pNode) == true)
               {
                  removeConnection(pConnectedNode, pNode);
               }
               else if (pItem->isOutputNode(pNode) == true)
               {
                  removeConnection(pNode, pConnectedNode);
               }
            }
         }
      }
   }
}

void WizardView::nodeConnected(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardNode* pNode = dynamic_cast<WizardNode*>(&subject);
   WizardNode* pConnectedNode = boost::any_cast<WizardNode*>(data);

   if ((pNode != NULL) && (pConnectedNode != NULL))
   {
      addConnection(pNode, pConnectedNode);
   }
}

void WizardView::nodeDisconnected(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardNode* pNode = dynamic_cast<WizardNode*>(&subject);
   WizardNode* pConnectedNode = boost::any_cast<WizardNode*>(data);

   if ((pNode != NULL) && (pConnectedNode != NULL))
   {
      WizardItemImp* pItem = static_cast<WizardItemImp*>(pNode->getItem());
      if (pItem != NULL)
      {
         if (pItem->isInputNode(pNode) == true)
         {
            removeConnection(pConnectedNode, pNode);
         }
         else if (pItem->isOutputNode(pNode) == true)
         {
            removeConnection(pNode, pConnectedNode);
         }
      }
   }
}

void WizardView::setModified(Subject& subject, const std::string& signal, const boost::any& data)
{
   mModified = true;
}

void WizardView::addItem(WizardItem* pItem)
{
   if ((pItem == NULL) || (getGraphicsItem(pItem) != NULL))
   {
      return;
   }

   QGraphicsScene* pScene = scene();
   VERIFYNRV(pScene != NULL);

   WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
   VERIFYNRV(pItemImp != NULL);

   // Create the graphics item
   WizardGraphicsItem* pGraphicsItem = new WizardGraphicsItem(pItem);
   pScene->addItem(pGraphicsItem);     // Add the item to the scene before initializing it so that the
                                       // scene can properly update when setting the item properties

   pGraphicsItem->setZValue(sItemZ++);
   pGraphicsItem->setPos(pItemImp->getXPosition(), pItemImp->getYPosition());
   pGraphicsItem->setExecutionOrder(getNumItems());

   // Connections
   VERIFYNR(connect(pGraphicsItem, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged())));
   VERIFYNR(pItemImp->attach(SIGNAL_NAME(WizardItemImp, Renamed), Slot(this, &WizardView::itemPositionChanged)));
   VERIFYNR(pItemImp->attach(SIGNAL_NAME(WizardItemImp, PositionChanged),
      Slot(this, &WizardView::itemPositionChanged)));
   VERIFYNR(pItemImp->attach(SIGNAL_NAME(WizardItemImp, NodeAdded), Slot(this, &WizardView::nodeAdded)));
   VERIFYNR(pItemImp->attach(SIGNAL_NAME(WizardItemImp, NodeRemoved), Slot(this, &WizardView::nodeRemoved)));
   VERIFYNR(pItemImp->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &WizardView::setModified)));

   const std::vector<WizardNode*>& inputNodes = pItem->getInputNodes();
   const std::vector<WizardNode*>& outputNodes = pItem->getOutputNodes();

   for (std::vector<WizardNode*>::size_type i = 0; i < inputNodes.size() + outputNodes.size(); ++i)
   {
      WizardNodeImp* pNode = NULL;
      if (i < inputNodes.size())
      {
         pNode = static_cast<WizardNodeImp*>(inputNodes[i]);
      }
      else
      {
         pNode = static_cast<WizardNodeImp*>(outputNodes[i - inputNodes.size()]);
      }

      if (pNode != NULL)
      {
         VERIFYNR(pNode->attach(SIGNAL_NAME(WizardNodeImp, NodeConnected), Slot(this, &WizardView::nodeConnected)));
         VERIFYNR(pNode->attach(SIGNAL_NAME(WizardNodeImp, NodeDisconnected),
            Slot(this, &WizardView::nodeDisconnected)));
         VERIFYNR(pNode->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &WizardView::setModified)));
      }
   }
}

void WizardView::addItems(const std::vector<WizardItem*>& wizardItems)
{
   if (wizardItems.empty() == true)
   {
      return;
   }

   // Items
   for (std::vector<WizardItem*>::const_iterator iter = wizardItems.begin(); iter != wizardItems.end(); ++iter)
   {
      WizardItem* pItem = *iter;
      if (pItem != NULL)
      {
         addItem(pItem);
      }
   }

   // Node connections
   for (std::vector<WizardItem*>::const_iterator iter = wizardItems.begin(); iter != wizardItems.end(); ++iter)
   {
      WizardItem* pItem = *iter;
      if (pItem != NULL)
      {
         const std::vector<WizardNode*>& outputNodes = pItem->getOutputNodes();
         for (std::vector<WizardNode*>::const_iterator outputIter = outputNodes.begin();
            outputIter != outputNodes.end();
            ++outputIter)
         {
            WizardNode* pOutputNode = *outputIter;
            if (pOutputNode != NULL)
            {
               const std::vector<WizardNode*>& connectedNodes = pOutputNode->getConnectedNodes();
               for (std::vector<WizardNode*>::const_iterator connectedIter = connectedNodes.begin();
                  connectedIter != connectedNodes.end();
                  ++connectedIter)
               {
                  WizardNode* pConnectedNode = *connectedIter;
                  if (pConnectedNode != NULL)
                  {
                     addConnection(pOutputNode, pConnectedNode);
                  }
               }
            }
         }
      }
   }
}

void WizardView::removeItem(WizardItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   // Connections
   WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
   VERIFYNRV(pItemImp != NULL);

   VERIFYNR(pItemImp->detach(SIGNAL_NAME(WizardItemImp, Renamed), Slot(this, &WizardView::itemPositionChanged)));
   VERIFYNR(pItemImp->detach(SIGNAL_NAME(WizardItemImp, PositionChanged),
      Slot(this, &WizardView::itemPositionChanged)));
   VERIFYNR(pItemImp->detach(SIGNAL_NAME(WizardItemImp, NodeAdded), Slot(this, &WizardView::nodeAdded)));
   VERIFYNR(pItemImp->detach(SIGNAL_NAME(WizardItemImp, NodeRemoved), Slot(this, &WizardView::nodeRemoved)));
   VERIFYNR(pItemImp->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &WizardView::setModified)));

   // Remove the node connection items
   const std::vector<WizardNode*>& inputNodes = pItem->getInputNodes();
   const std::vector<WizardNode*>& outputNodes = pItem->getOutputNodes();

   for (std::vector<WizardNode*>::size_type i = 0; i < inputNodes.size() + outputNodes.size(); ++i)
   {
      WizardNodeImp* pNode = NULL;
      if (i < inputNodes.size())
      {
         pNode = static_cast<WizardNodeImp*>(inputNodes[i]);
      }
      else
      {
         pNode = static_cast<WizardNodeImp*>(outputNodes[i - inputNodes.size()]);
      }

      if (pNode != NULL)
      {
         VERIFYNR(pNode->detach(SIGNAL_NAME(WizardNodeImp, NodeConnected), Slot(this, &WizardView::nodeConnected)));
         VERIFYNR(pNode->detach(SIGNAL_NAME(WizardNodeImp, NodeDisconnected),
            Slot(this, &WizardView::nodeDisconnected)));
         VERIFYNR(pNode->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &WizardView::setModified)));

         const std::vector<WizardNode*>& connectedNodes = pNode->getConnectedNodes();
         for (std::vector<WizardNode*>::const_iterator iter = connectedNodes.begin(); iter != connectedNodes.end(); ++iter)
         {
            WizardNode* pConnectedNode = *iter;
            if (pConnectedNode != NULL)
            {
               if (i < inputNodes.size())
               {
                  removeConnection(pConnectedNode, pNode);
               }
               else
               {
                  removeConnection(pNode, pConnectedNode);
               }
            }
         }
      }
   }

   // Remove the wizard item
   WizardGraphicsItem* pGraphicsItem = getGraphicsItem(pItem);
   if (pGraphicsItem != NULL)
   {
      delete pGraphicsItem;
   }

   // Update the execution order
   updateExecutionOrder();
}

unsigned int WizardView::getNumItems() const
{
   std::vector<WizardGraphicsItem*> items = getGraphicsItems();
   return items.size();
}

void WizardView::clear()
{
   std::vector<WizardGraphicsItem*> items = getGraphicsItems();
   for (std::vector<WizardGraphicsItem*>::iterator iter = items.begin(); iter != items.end(); ++iter)
   {
      WizardGraphicsItem* pGraphicsItem = *iter;
      if (pGraphicsItem != NULL)
      {
         WizardItem* pItem = pGraphicsItem->getWizardItem();
         if (pItem != NULL)
         {
            removeItem(pItem);
         }
      }
   }
}

void WizardView::addConnection(WizardNode* pOutputNode, WizardNode* pInputNode)
{
   if ((pOutputNode == NULL) || (pInputNode == NULL))
   {
      return;
   }

   QGraphicsScene* pScene = scene();
   VERIFYNRV(pScene != NULL);

   // Check for valid nodes
   WizardGraphicsItem* pOutputGraphicsItem = getGraphicsItem(pOutputNode);
   WizardGraphicsItem* pInputGraphicsItem = getGraphicsItem(pInputNode);
   if ((pOutputGraphicsItem == NULL) || (pInputGraphicsItem == NULL))
   {
      return;
   }

   WizardItem* pOutputItem = pOutputGraphicsItem->getWizardItem();
   WizardItem* pInputItem = pInputGraphicsItem->getWizardItem();
   if ((pOutputItem == NULL) || (pInputItem == NULL))
   {
      return;
   }

   const std::vector<WizardNode*>& outputNodes = pOutputItem->getOutputNodes();
   if (std::find(outputNodes.begin(), outputNodes.end(), pOutputNode) == outputNodes.end())
   {
      return;
   }

   const std::vector<WizardNode*>& inputNodes = pInputItem->getInputNodes();
   if (std::find(inputNodes.begin(), inputNodes.end(), pInputNode) == inputNodes.end())
   {
      return;
   }

   // Create the connection line
   QPointF outputPoint = pOutputGraphicsItem->getNodeConnectionPoint(pOutputNode);
   QPointF inputPoint = pInputGraphicsItem->getNodeConnectionPoint(pInputNode);
   QColor lineColor = WizardGraphicsItem::getNodeColor(pOutputNode);

   WizardLine* pLine = new WizardLine(pOutputNode, pInputNode);
   pLine->setPen(QPen(lineColor, 3));
   pLine->setLine(QLineF(outputPoint, inputPoint));
   pLine->setZValue(sLineZ++);
   pScene->addItem(pLine);
}

void WizardView::removeConnection(WizardNode* pOutputNode, WizardNode* pInputNode)
{
   if ((pOutputNode == NULL) || (pInputNode == NULL))
   {
      return;
   }

   QList<QGraphicsItem*> allItems = items();
   for (int i = 0; i < allItems.count(); ++i)
   {
      WizardLine* pLine = dynamic_cast<WizardLine*>(allItems[i]);
      if (pLine != NULL)
      {
         WizardNode* pCurrentOutputNode = pLine->getOutputNode();
         WizardNode* pCurrentInputNode = pLine->getInputNode();
         if ((pCurrentOutputNode == pOutputNode) && (pCurrentInputNode == pInputNode))
         {
            delete pLine;
            return;
         }
      }
   }
}

void WizardView::updateConnectionPosition(WizardItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   const std::vector<WizardNode*>& inputNodes = pItem->getInputNodes();
   const std::vector<WizardNode*>& outputNodes = pItem->getOutputNodes();

   for (std::vector<WizardNode*>::size_type i = 0; i < inputNodes.size() + outputNodes.size(); ++i)
   {
      WizardNode* pNode = NULL;
      if (i < inputNodes.size())
      {
         pNode = inputNodes[i];
      }
      else
      {
         pNode = outputNodes[i - inputNodes.size()];
      }

      if (pNode != NULL)
      {
         const std::vector<WizardNode*>& connectedNodes = pNode->getConnectedNodes();
         for (std::vector<WizardNode*>::const_iterator iter = connectedNodes.begin(); iter != connectedNodes.end(); ++iter)
         {
            WizardNode* pConnectedNode = *iter;
            if (pConnectedNode != NULL)
            {
               if (i < inputNodes.size())
               {
                  updateConnectionPosition(pConnectedNode, pNode);
               }
               else
               {
                  updateConnectionPosition(pNode, pConnectedNode);
               }
            }
         }
      }
   }
}

void WizardView::updateConnectionPosition(WizardNode* pOutputNode, WizardNode* pInputNode)
{
   if ((pOutputNode == NULL) || (pInputNode == NULL))
   {
      return;
   }

   WizardLine* pLine = NULL;

   QList<QGraphicsItem*> allItems = items();
   for (int i = 0; i < allItems.count(); ++i)
   {
      WizardLine* pCurrentLine = dynamic_cast<WizardLine*>(allItems[i]);
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

   if (pLine == NULL)
   {
      return;
   }

   WizardGraphicsItem* pOutputGraphicsItem = getGraphicsItem(pOutputNode);
   WizardGraphicsItem* pInputGraphicsItem = getGraphicsItem(pInputNode);
   if ((pOutputGraphicsItem == NULL) || (pInputGraphicsItem == NULL))
   {
      return;
   }

   QPointF outputPos = pOutputGraphicsItem->getNodeConnectionPoint(pOutputNode);
   QPointF inputPos = pInputGraphicsItem->getNodeConnectionPoint(pInputNode);
   pLine->setOutputPoint(outputPos);
   pLine->setInputPoint(inputPos);
}

void WizardView::updateExecutionOrder()
{
   if (mpWizard.get() == NULL)
   {
      return;
   }

   const std::vector<WizardItem*>& items = mpWizard->getItems();
   for (std::vector<WizardItem*>::size_type i = 0; i < items.size(); ++i)
   {
      WizardItem* pItem = items[i];
      if (pItem != NULL)
      {
         WizardGraphicsItem* pGraphicsItem = getGraphicsItem(pItem);
         if (pGraphicsItem != NULL)
         {
            pGraphicsItem->setExecutionOrder(i + 1);
            pGraphicsItem->update();
         }
      }
   }
}

WizardGraphicsItem* WizardView::getGraphicsItem(const QPoint& viewPos) const
{
   QList<QGraphicsItem*> itemsAtPos = items(viewPos);
   for (int i = 0; i < itemsAtPos.count(); ++i)
   {
      WizardGraphicsItem* pGraphicsItem = dynamic_cast<WizardGraphicsItem*>(itemsAtPos[i]);
      if (pGraphicsItem != NULL)
      {
         return pGraphicsItem;
      }
   }

   return NULL;
}

WizardGraphicsItem* WizardView::getGraphicsItem(WizardItem* pItem) const
{
   if (pItem == NULL)
   {
      return NULL;
   }

   std::vector<WizardGraphicsItem*> items = getGraphicsItems();
   for (unsigned int i = 0; i < items.size(); i++)
   {
      WizardGraphicsItem* pGraphicsItem = items[i];
      if (pGraphicsItem != NULL)
      {
         WizardItem* pCurrentItem = pGraphicsItem->getWizardItem();
         if (pCurrentItem == pItem)
         {
            return pGraphicsItem;
         }
      }
   }

   return NULL;
}

WizardGraphicsItem* WizardView::getGraphicsItem(WizardNode* pNode) const
{
   if (pNode == NULL)
   {
      return NULL;
   }

   std::vector<WizardGraphicsItem*> items = getGraphicsItems();
   for (unsigned int i = 0; i < items.size(); i++)
   {
      WizardGraphicsItem* pGraphicsItem = items[i];
      if (pGraphicsItem != NULL)
      {
         WizardItemImp* pItem = static_cast<WizardItemImp*>(pGraphicsItem->getWizardItem());
         if (pItem != NULL)
         {
            bool bSuccess = pItem->isInputNode(pNode);
            if (bSuccess == false)
            {
               bSuccess = pItem->isOutputNode(pNode);
            }

            if (bSuccess == true)
            {
               return pGraphicsItem;
            }
         }
      }
   }

   return NULL;
}

std::vector<WizardGraphicsItem*> WizardView::getGraphicsItems() const
{
   std::vector<WizardGraphicsItem*> graphicsItems;

   QList<QGraphicsItem*> allItems = items();
   for (int i = 0; i < allItems.count(); ++i)
   {
      WizardGraphicsItem* pGraphicsItem = dynamic_cast<WizardGraphicsItem*>(allItems[i]);
      if (pGraphicsItem != NULL)
      {
         graphicsItems.push_back(pGraphicsItem);
      }
   }

   return graphicsItems;
}

bool WizardView::save(const QString& filename)
{
   if ((filename.isEmpty() == true) || (mpWizard.get() == NULL))
   {
      return false;
   }

   bool success = false;

   FileResource file(filename.toStdString().c_str(), "wb");
   if (file.get() != NULL)
   {
      XMLWriter xml("Wizard");

      success = mpWizard->toXml(&xml);
      if (success == true)
      {
         xml.writeToFile(file.get());
      }
      else
      {
         file.setDeleteOnClose(true);
      }
   }

   if (success == false)
   {
      QMessageBox::critical(this, "Wizard Builder", "The wizard file '" + filename + "' could not be saved!");
   }
   else
   {
      std::string batchFilename = WizardUtilities::deriveBatchWizardFilename(filename.toStdString());
      bool createdBatchWizard = false;

      std::auto_ptr<BatchWizard> pBatchWizard(WizardUtilities::createBatchWizardFromWizard(mpWizard.get(),
         filename.toStdString()));
      if (pBatchWizard.get() != NULL)
      {
         std::vector<BatchWizard*> batchWizards;
         batchWizards.push_back(pBatchWizard.get());
         createdBatchWizard = WizardUtilities::writeBatchWizard(batchWizards, batchFilename);
      }

      if (createdBatchWizard == false)
      {
         QMessageBox::critical(this, "Wizard Builder", "Could not save the '" +
            QString::fromStdString(batchFilename) + "' file!");
      }
   }

   return success;
}
