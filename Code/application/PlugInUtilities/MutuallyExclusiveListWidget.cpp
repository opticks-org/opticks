/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "MutuallyExclusiveListWidget.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>

MutuallyExclusiveListWidget::MutuallyExclusiveListWidget(QWidget* pParent) :
   QWidget(pParent),
   mpAvailableList(NULL),
   mpSelectedList(NULL)
{
   // Available items
   mpAvailableLabel = new QLabel("Available Items:", this);
   mpAvailableList = new QListWidget(this);

   // Selected items
   mpSelectedLabel = new QLabel("Selected Items:", this);
   mpSelectedList = new QListWidget(this);

   // Buttons
   QPushButton* pAddButton = new QPushButton("Add ->", this);
   QPushButton* pRemoveButton = new QPushButton("<- Remove", this);

   // Layout
   QVBoxLayout* pButtonLayout = new QVBoxLayout();
   pButtonLayout->setMargin(5);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(pAddButton);
   pButtonLayout->addWidget(pRemoveButton);
   pButtonLayout->addStretch(10);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(mpAvailableLabel, 0, 0);
   pGrid->addWidget(mpAvailableList, 1, 0);
   pGrid->addLayout(pButtonLayout, 0, 1, 2, 1);
   pGrid->addWidget(mpSelectedLabel, 0, 2);
   pGrid->addWidget(mpSelectedList, 1, 2);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(0, 5);
   pGrid->setColumnStretch(2, 5);

   // Connections
   VERIFYNR(connect(pAddButton, SIGNAL(clicked()), this, SLOT(addSelectedItem())));
   VERIFYNR(connect(pRemoveButton, SIGNAL(clicked()), this, SLOT(removeSelectedItem())));
}

MutuallyExclusiveListWidget::~MutuallyExclusiveListWidget()
{}

void MutuallyExclusiveListWidget::setAvailableItemsLabel(const QString& label)
{
   if (label.isEmpty() == false)
   {
      mpAvailableLabel->setText(label);
   }
}

QString MutuallyExclusiveListWidget::getAvailableItemsLabel() const
{
   return mpAvailableLabel->text();
}

void MutuallyExclusiveListWidget::setAvailableItems(const QStringList& items)
{
   mpAvailableList->clear();
   mpSelectedList->clear();
   mpAvailableList->addItems(items);
}

QStringList MutuallyExclusiveListWidget::getAvailableItems() const
{
   QStringList availableItems;
   for (int i = 0; i < mpAvailableList->count(); ++i)
   {
      QListWidgetItem* pItem = mpAvailableList->item(i);
      if ((pItem != NULL) && (pItem->isHidden() == false))
      {
         availableItems.append(pItem->text());
      }
   }

   return availableItems;
}

void MutuallyExclusiveListWidget::setSelectedItemsLabel(const QString& label)
{
   if (label.isEmpty() == false)
   {
      mpSelectedLabel->setText(label);
   }
}

QString MutuallyExclusiveListWidget::getSelectedItemsLabel() const
{
   return mpSelectedLabel->text();
}

QStringList MutuallyExclusiveListWidget::getSelectedItems() const
{
   QStringList selectedItems;
   for (int i = 0; i < mpSelectedList->count(); ++i)
   {
      QListWidgetItem* pItem = mpSelectedList->item(i);
      if (pItem != NULL)
      {
         selectedItems.append(pItem->text());
      }
   }

   return selectedItems;
}

void MutuallyExclusiveListWidget::selectItems(const QStringList& items)
{
   bool itemsSelected = false;
   for (int i = 0; i < items.count(); ++i)
   {
      QList<QListWidgetItem*> availableItems = mpAvailableList->findItems(items[i], Qt::MatchExactly);
      if (availableItems.empty() == false)
      {
         VERIFYNRV(availableItems.size() == 1);

         QListWidgetItem* pItem = availableItems.front();
         if ((pItem != NULL) && (pItem->isHidden() == false))
         {
            selectAvailableItem(pItem);
            itemsSelected = true;
         }
      }
   }

   if (itemsSelected == true)
   {
      mpAvailableList->clearSelection();
   }
}

void MutuallyExclusiveListWidget::removeItems(const QStringList& items)
{
   for (int i = 0; i < items.count(); ++i)
   {
      QList<QListWidgetItem*> selectedItems = mpSelectedList->findItems(items[i], Qt::MatchExactly);
      if (selectedItems.empty() == false)
      {
         VERIFYNRV(selectedItems.size() == 1);
         removeSelectedItem(selectedItems.front());
      }
   }
}

void MutuallyExclusiveListWidget::addSelectedItem()
{
   QList<QListWidgetItem*> availableItems = mpAvailableList->selectedItems();
   if (availableItems.empty() == true)
   {
      return;
   }

   VERIFYNRV(availableItems.size() == 1);
   selectAvailableItem(availableItems.front());
   mpAvailableList->clearSelection();
}

void MutuallyExclusiveListWidget::removeSelectedItem()
{
   QList<QListWidgetItem*> selectedItems = mpSelectedList->selectedItems();
   if (selectedItems.empty() == true)
   {
      return;
   }

   VERIFYNRV(selectedItems.size() == 1);
   removeSelectedItem(selectedItems.front());
}

void MutuallyExclusiveListWidget::selectAvailableItem(QListWidgetItem* pAvailableItem)
{
   if (pAvailableItem == NULL)
   {
      return;
   }

   if (pAvailableItem->listWidget() != mpAvailableList)
   {
      return;
   }

   if (pAvailableItem->isHidden() == true)
   {
      return;
   }

   QString text = pAvailableItem->text();

   // Hide the item in the available items list
   pAvailableItem->setHidden(true);

   // Add the item to the selected items list
   mpSelectedList->addItem(text);

   // Notify connected objects
   emit itemSelected(text);
}

void MutuallyExclusiveListWidget::removeSelectedItem(QListWidgetItem* pSelectedItem)
{
   if (pSelectedItem == NULL)
   {
      return;
   }

   if (pSelectedItem->listWidget() != mpSelectedList)
   {
      return;
   }

   QString text = pSelectedItem->text();

   // Show the item in the available items list
   QList<QListWidgetItem*> availableItems = mpAvailableList->findItems(text, Qt::MatchExactly);
   VERIFYNRV(availableItems.size() == 1);

   QListWidgetItem* pAvailableItem = availableItems.front();
   if (pAvailableItem != NULL)
   {
      pAvailableItem->setHidden(false);
   }

   // Delete the item from the selected items list
   delete pSelectedItem;

   // Notify connected objects
   emit itemRemoved(text);
}
