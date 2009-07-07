/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QLayout>

#include "AppVerify.h"
#include "WizardGraphicsItem.h"
#include "WizardItemProperties.h"
#include "WizardNodeImp.h"
#include "WizardObject.h"
#include "WizardObjectImp.h"

#include <vector>

WizardItemProperties::WizardItemProperties(QWidget* pParent) :
   QWidget(pParent),
   mpItemNameLabel(NULL),
   mpItemTypeLabel(NULL),
   mpItemOrderLabel(NULL),
   mpItemModeCombo(NULL),
   mpItemInputsTree(NULL),
   mpItemOutputsTree(NULL),
   mpItem(NULL)
{
   // Name
   QLabel* pItemName = new QLabel("Name:", this);
   mpItemNameLabel = new QLabel(this);

   // Type
   QLabel* pItemType = new QLabel("Type:", this);
   mpItemTypeLabel = new QLabel(this);

   // Execution order
   QLabel* pItemOrder = new QLabel("Execution Order:", this);
   mpItemOrderLabel = new QLabel(this);

   mpIncreaseOrderButton = new QToolButton(this);
   mpIncreaseOrderButton->setIcon(QIcon(":/icons/Increase"));
   mpIncreaseOrderButton->setAutoRaise(true);
   mpIncreaseOrderButton->setToolTip("Increase Order");
   mpIncreaseOrderButton->setEnabled(false);

   mpDecreaseOrderButton = new QToolButton(this);
   mpDecreaseOrderButton->setIcon(QIcon(":/icons/Decrease"));
   mpDecreaseOrderButton->setAutoRaise(true);
   mpDecreaseOrderButton->setToolTip("Decrease Order");
   mpDecreaseOrderButton->setEnabled(false);

   // Execution mode
   QLabel* pItemMode = new QLabel("Execution Mode:", this);
   mpItemModeCombo = new QComboBox(this);
   mpItemModeCombo->addItem("Batch");
   mpItemModeCombo->addItem("Interactive");
   mpItemModeCombo->setEnabled(false);

   // Inputs
   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Type");
   columnNames.append("Description");

   QLabel* pItemInputs = new QLabel("Inputs:", this);
   mpItemInputsTree = new QTreeWidget(this);
   mpItemInputsTree->setColumnCount(columnNames.count());
   mpItemInputsTree->setHeaderLabels(columnNames);
   mpItemInputsTree->setSortingEnabled(false);
   mpItemInputsTree->setSelectionMode(QAbstractItemView::NoSelection);
   mpItemInputsTree->setRootIsDecorated(false);
   mpItemInputsTree->setEnabled(false);

   // Outputs
   QLabel* pItemOutputs = new QLabel("Outputs:", this);
   mpItemOutputsTree = new QTreeWidget(this);
   mpItemOutputsTree->setColumnCount(columnNames.count());
   mpItemOutputsTree->setHeaderLabels(columnNames);
   mpItemOutputsTree->setSortingEnabled(false);
   mpItemOutputsTree->setSelectionMode(QAbstractItemView::NoSelection);
   mpItemOutputsTree->setRootIsDecorated(false);
   mpItemOutputsTree->setEnabled(false);

   // Layout
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(0);
   pButtonLayout->addWidget(mpIncreaseOrderButton);
   pButtonLayout->addWidget(mpDecreaseOrderButton);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(5);
   pGrid->setSpacing(5);
   pGrid->addWidget(pItemName, 0, 0);
   pGrid->addWidget(mpItemNameLabel, 0, 1, 1, 3);
   pGrid->addWidget(pItemType, 1, 0);
   pGrid->addWidget(mpItemTypeLabel, 1, 1, 1, 3);
   pGrid->addWidget(pItemOrder, 2, 0);
   pGrid->addWidget(mpItemOrderLabel, 2, 1);
   pGrid->addLayout(pButtonLayout, 2, 3, Qt::AlignLeft);
   pGrid->addWidget(pItemMode, 3, 0);
   pGrid->addWidget(mpItemModeCombo, 3, 1, 1, 3);
   pGrid->addWidget(pItemInputs, 4, 0);
   pGrid->addWidget(mpItemInputsTree, 5, 0, 1, 4);
   pGrid->addWidget(pItemOutputs, 6, 0);
   pGrid->addWidget(mpItemOutputsTree, 7, 0, 1, 4);
   pGrid->setRowStretch(5, 10);
   pGrid->setRowStretch(7, 10);
   pGrid->setColumnStretch(3, 10);
   pGrid->setColumnMinimumWidth(2, 10);

   // Connections
   mpItem.addSignal(SIGNAL_NAME(WizardItemImp, Renamed), Slot(this, &WizardItemProperties::itemRenamed));
   mpItem.addSignal(SIGNAL_NAME(WizardItemImp, BatchModeChanged),
      Slot(this, &WizardItemProperties::itemExecutionModeChanged));
   mpItem.addSignal(SIGNAL_NAME(WizardItemImp, NodeAdded), Slot(this, &WizardItemProperties::nodeAdded));
   mpItem.addSignal(SIGNAL_NAME(WizardItemImp, NodeRemoved), Slot(this, &WizardItemProperties::nodeRemoved));

   VERIFYNR(connect(mpIncreaseOrderButton, SIGNAL(clicked()), this, SLOT(increaseItemOrder())));
   VERIFYNR(connect(mpDecreaseOrderButton, SIGNAL(clicked()), this, SLOT(decreaseItemOrder())));
   VERIFYNR(connect(mpItemModeCombo, SIGNAL(activated(int)), this, SLOT(setItemExecutionMode(int))));
}

WizardItemProperties::~WizardItemProperties()
{
   setWizardItem(NULL);
}

void WizardItemProperties::setWizardItem(WizardItem* pItem)
{
   WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
   if (pItemImp == mpItem.get())
   {
      return;
   }

   if (mpItem.get() != NULL)
   {
      WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(mpItem->getWizard());
      if (pWizard != NULL)
      {
         VERIFYNRV(pWizard->detach(SIGNAL_NAME(WizardObjectImp, BatchModeChanged),
            Slot(this, &WizardItemProperties::wizardExecutionModeChanged)));
         VERIFYNRV(pWizard->detach(SIGNAL_NAME(WizardObjectImp, ExecutionOrderChanged),
            Slot(this, &WizardItemProperties::executionOrderChanged)));
      }
   }

   while (mNodes.empty() == false)
   {
      QMap<WizardNodeImp*, QTreeWidgetItem*>::iterator iter = mNodes.begin();

      WizardNodeImp* pNode = iter.key();
      if (pNode != NULL)
      {
         removeNode(pNode);
      }
   }

   mpItem.reset(pItemImp);
   if (mpItem.get() == NULL)
   {
      mpItemNameLabel->clear();
      mpItemTypeLabel->clear();
      mpItemOrderLabel->clear();
      mpIncreaseOrderButton->setEnabled(false);
      mpDecreaseOrderButton->setEnabled(false);
      mpItemModeCombo->setCurrentIndex(0);
      mpItemModeCombo->setEnabled(false);
      mpItemInputsTree->clear();
      mpItemInputsTree->setEnabled(false);
      mpItemOutputsTree->clear();
      mpItemOutputsTree->setEnabled(false);
      return;
   }

   // Name
   QString itemName = QString::fromStdString(mpItem->getName());
   mpItemNameLabel->setText(itemName);

   // Type
   QString itemType = QString::fromStdString(mpItem->getType());
   mpItemTypeLabel->setText(itemType);

   // Execution order
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(mpItem->getWizard());
   if (pWizard != NULL)
   {
      int order = pWizard->getExecutionOrder(mpItem.get());
      if (order > 0)
      {
         mpItemOrderLabel->setText(QString::number(order));
      }

      VERIFYNRV(pWizard->attach(SIGNAL_NAME(WizardObjectImp, BatchModeChanged),
         Slot(this, &WizardItemProperties::wizardExecutionModeChanged)));
      VERIFYNRV(pWizard->attach(SIGNAL_NAME(WizardObjectImp, ExecutionOrderChanged),
         Slot(this, &WizardItemProperties::executionOrderChanged)));
   }

   mpIncreaseOrderButton->setEnabled(true);
   mpDecreaseOrderButton->setEnabled(true);

   // Execution mode
   if (mpItem->getBatchMode() == true)
   {
      mpItemModeCombo->setCurrentIndex(0);
   }
   else
   {
      mpItemModeCombo->setCurrentIndex(1);
   }

   bool disableCombo = false;
   if (pWizard != NULL)
   {
      disableCombo = pWizard->isBatch();
   }

   mpItemModeCombo->setDisabled(disableCombo);

   // Input nodes
   const std::vector<WizardNode*>& inputNodes = mpItem->getInputNodes();
   for (std::vector<WizardNode*>::const_iterator iter = inputNodes.begin(); iter != inputNodes.end(); ++iter)
   {
      WizardNodeImp* pNode = static_cast<WizardNodeImp*>(*iter);
      if (pNode != NULL)
      {
         addNode(pNode);
      }
   }

   mpItemInputsTree->setEnabled(true);

   // Output nodes
   const std::vector<WizardNode*>& outputNodes = mpItem->getOutputNodes();
   for (std::vector<WizardNode*>::const_iterator iter = outputNodes.begin(); iter != outputNodes.end(); ++iter)
   {
      WizardNodeImp* pNode = static_cast<WizardNodeImp*>(*iter);
      if (pNode != NULL)
      {
         addNode(pNode);
      }
   }

   mpItemOutputsTree->setEnabled(true);
}

WizardItem* WizardItemProperties::getWizardItem()
{
   return mpItem.get();
}

const WizardItem* WizardItemProperties::getWizardItem() const
{
   return mpItem.get();
}

QSize WizardItemProperties::sizeHint() const
{
   QSize widgetSize = QWidget::sizeHint();
   widgetSize.setWidth(315);     // This width provides enough space to not display a horizontal scroll bar by default

   return widgetSize;
}

void WizardItemProperties::wizardExecutionModeChanged(Subject& subject, const std::string& signal,
                                                      const boost::any& data)
{
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(&subject);
   VERIFYNRV(pWizard != NULL);

   bool batch = pWizard->isBatch();
   mpItemModeCombo->setDisabled(batch);
}

void WizardItemProperties::executionOrderChanged(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(&subject);
   VERIFYNRV(pWizard != NULL);

   QString orderText;

   int order = pWizard->getExecutionOrder(mpItem.get());
   if (order > -1)
   {
      orderText = QString::number(order);
   }

   mpItemOrderLabel->setText(orderText);
}

void WizardItemProperties::itemRenamed(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardItemImp* pItem = dynamic_cast<WizardItemImp*>(&subject);
   VERIFYNRV(pItem != NULL);
   VERIFYNRV(pItem == mpItem.get());

   const std::string& itemName = mpItem->getName();
   mpItemNameLabel->setText(QString::fromStdString(itemName));
}

void WizardItemProperties::itemExecutionModeChanged(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardItemImp* pItem = dynamic_cast<WizardItemImp*>(&subject);
   VERIFYNRV(pItem != NULL);
   VERIFYNRV(pItem == mpItem.get());

   if (mpItem->getBatchMode() == true)
   {
      mpItemModeCombo->setCurrentIndex(0);
   }
   else
   {
      mpItemModeCombo->setCurrentIndex(1);
   }
}

void WizardItemProperties::nodeAdded(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardNodeImp* pNode = static_cast<WizardNodeImp*>(boost::any_cast<WizardNode*>(data));
   if (pNode != NULL)
   {
      addNode(pNode);
   }
}

void WizardItemProperties::nodeRemoved(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardNodeImp* pNode = static_cast<WizardNodeImp*>(boost::any_cast<WizardNode*>(data));
   if (pNode != NULL)
   {
      removeNode(pNode);
   }
}

void WizardItemProperties::nodeRenamed(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardNodeImp* pNode = dynamic_cast<WizardNodeImp*>(&subject);
   VERIFYNRV(pNode != NULL);

   QMap<WizardNodeImp*, QTreeWidgetItem*>::iterator iter = mNodes.find(pNode);
   if (iter != mNodes.end())
   {
      QTreeWidgetItem* pTreeItem = iter.value();
      if (pTreeItem != NULL)
      {
         const std::string& nodeName = pNode->getName();
         pTreeItem->setText(0, QString::fromStdString(nodeName));
      }
   }
}

void WizardItemProperties::nodeTypeChanged(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardNodeImp* pNode = dynamic_cast<WizardNodeImp*>(&subject);
   VERIFYNRV(pNode != NULL);

   QMap<WizardNodeImp*, QTreeWidgetItem*>::iterator iter = mNodes.find(pNode);
   if (iter != mNodes.end())
   {
      QTreeWidgetItem* pTreeItem = iter.value();
      if (pTreeItem != NULL)
      {
         QPixmap nodePix = WizardGraphicsItem::getNodePixmap(pNode);
         const std::string& nodeType = pNode->getType();

         pTreeItem->setIcon(1, QIcon(nodePix));
         pTreeItem->setText(1, QString::fromStdString(nodeType));
      }
   }
}

void WizardItemProperties::nodeDescriptionChanged(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardNodeImp* pNode = dynamic_cast<WizardNodeImp*>(&subject);
   VERIFYNRV(pNode != NULL);

   QMap<WizardNodeImp*, QTreeWidgetItem*>::iterator iter = mNodes.find(pNode);
   if (iter != mNodes.end())
   {
      QTreeWidgetItem* pTreeItem = iter.value();
      if (pTreeItem != NULL)
      {
         const std::string& nodeDescription = pNode->getDescription();
         pTreeItem->setText(2, QString::fromStdString(nodeDescription));
      }
   }
}

void WizardItemProperties::addNode(WizardNodeImp* pNode)
{
   if (pNode == NULL)
   {
      return;
   }

   if (mNodes.find(pNode) != mNodes.end())
   {
      return;
   }

   // Create the tree widget item
   QTreeWidget* pTreeWidget = NULL;

   WizardItemImp* pItem = static_cast<WizardItemImp*>(pNode->getItem());
   if (pItem != NULL)
   {
      if (pItem->isInputNode(pNode) == true)
      {
         pTreeWidget = mpItemInputsTree;
      }
      else if (pItem->isOutputNode(pNode) == true)
      {
         pTreeWidget = mpItemOutputsTree;
      }
   }

   if (pTreeWidget == NULL)
   {
      return;
   }

   QString nodeName = QString::fromStdString(pNode->getName());
   QString nodeType = QString::fromStdString(pNode->getType());
   QString nodeDescription = QString::fromStdString(pNode->getDescription());
   QPixmap nodePix = WizardGraphicsItem::getNodePixmap(pNode);

   QTreeWidgetItem* pTreeItem = new QTreeWidgetItem(pTreeWidget);
   pTreeItem->setText(0, nodeName);
   pTreeItem->setIcon(1, QIcon(nodePix));
   pTreeItem->setText(1, nodeType);
   pTreeItem->setText(2, nodeDescription);

   // Add the node to the map
   mNodes[pNode] = pTreeItem;

   // Attach to the node
   VERIFYNR(pNode->attach(SIGNAL_NAME(WizardNodeImp, Renamed), Slot(this, &WizardItemProperties::nodeRenamed)));
   VERIFYNR(pNode->attach(SIGNAL_NAME(WizardNodeImp, TypeChanged), Slot(this, &WizardItemProperties::nodeTypeChanged)));
   VERIFYNR(pNode->attach(SIGNAL_NAME(WizardNodeImp, DescriptionChanged),
      Slot(this, &WizardItemProperties::nodeDescriptionChanged)));
}

void WizardItemProperties::removeNode(WizardNodeImp* pNode)
{
   if (pNode == NULL)
   {
      return;
   }

   QMap<WizardNodeImp*, QTreeWidgetItem*>::iterator iter = mNodes.find(pNode);
   if (iter == mNodes.end())
   {
      return;
   }

   // Detach from the node
   VERIFYNR(pNode->detach(SIGNAL_NAME(WizardNodeImp, Renamed), Slot(this, &WizardItemProperties::nodeRenamed)));
   VERIFYNR(pNode->detach(SIGNAL_NAME(WizardNodeImp, TypeChanged),
      Slot(this, &WizardItemProperties::nodeTypeChanged)));
   VERIFYNR(pNode->detach(SIGNAL_NAME(WizardNodeImp, DescriptionChanged),
      Slot(this, &WizardItemProperties::nodeDescriptionChanged)));

   // Delete the tree widget item
   QTreeWidgetItem* pTreeItem = iter.value();
   if (pTreeItem != NULL)
   {
      delete pTreeItem;
   }

   // Remove the node from the map
   mNodes.remove(pNode);
}

void WizardItemProperties::increaseItemOrder()
{
   if (mpItem.get() == NULL)
   {
      return;
   }

   WizardObjectImp* pObject = dynamic_cast<WizardObjectImp*>(mpItem->getWizard());
   if (pObject != NULL)
   {
      pObject->increaseItemOrder(mpItem.get());
   }
}

void WizardItemProperties::decreaseItemOrder()
{
   if (mpItem.get() == NULL)
   {
      return;
   }

   WizardObjectImp* pObject = dynamic_cast<WizardObjectImp*>(mpItem->getWizard());
   if (pObject != NULL)
   {
      pObject->decreaseItemOrder(mpItem.get());
   }
}

void WizardItemProperties::setItemExecutionMode(int modeIndex)
{
   if (mpItem.get() != NULL)
   {
      if (modeIndex == 0)
      {
         mpItem->setBatchMode(true);
      }
      else if (modeIndex == 1)
      {
         mpItem->setBatchMode(false);
      }
   }
}
