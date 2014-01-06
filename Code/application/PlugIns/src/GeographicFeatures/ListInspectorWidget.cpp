/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ListInspectorWidget.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSplitter>

#include <sstream>

ListInspectorWidget::ListInspectorWidget(QWidget *pInspector, QWidget *pParent) : 
   QWidget(pParent), mpInspector(pInspector), mpDisplayedItem(NULL), mDeleteRowOnSave(false)
{
   mpSplitter = new QSplitter(this);
   mpSplitter->setHandleWidth(10);

   QHBoxLayout* pLayout = new QHBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);

   pLayout->addWidget(mpSplitter);

   mpInspector->setDisabled(true);
   QLayout* pInspectorLayout = mpInspector->layout();
   if (pInspectorLayout != NULL)
   {
      pInspectorLayout->setMargin(0);
   }

   QWidget* pListCompositeWidget = new QWidget(mpSplitter);
   mpListWidget = new QListWidget(pListCompositeWidget);
   VERIFYNR(connect(mpListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), 
      this, SLOT(setDisplayedItem(QListWidgetItem*))));

   QGridLayout* pListLayout = new QGridLayout(pListCompositeWidget);
   pListLayout->setMargin(0);
   pListLayout->setSpacing(5);
   pListLayout->addWidget(mpListWidget, 0, 0, 1, 3);

   QPushButton* pAddButton = new QPushButton("Add", pListCompositeWidget);
   QPushButton* pRemoveButton = new QPushButton("Remove", pListCompositeWidget);
   connect(pAddButton, SIGNAL(clicked()), this, SIGNAL(addItems()));
   connect(pRemoveButton, SIGNAL(clicked()), this, SLOT(removeDisplayedItem()));

   pListLayout->addWidget(pAddButton, 1, 1);
   pListLayout->addWidget(pRemoveButton, 1, 2);

   pListLayout->setColumnStretch(0, 10);

   mpSplitter->addWidget(pListCompositeWidget);
   mpSplitter->addWidget(mpInspector);
   mpSplitter->setStretchFactor(1, 10);

}

ListInspectorWidget::~ListInspectorWidget()
{
}

QListWidgetItem *ListInspectorWidget::addItem(const std::string &text)
{
   QListWidgetItem* pItem = new QListWidgetItem(QString::fromStdString(text), mpListWidget);
   mpListWidget->addItem(pItem);
   return pItem;
}

void ListInspectorWidget::setCurrentItem(QListWidgetItem* pItem)
{
   mpListWidget->setCurrentItem(pItem);
}

void ListInspectorWidget::applyChanges()
{
   emit saveInspector(mpInspector, mpDisplayedItem);
}

void ListInspectorWidget::removeDisplayedItem()
{
   if (mpDisplayedItem == NULL)
   {
      return;
   }

   mDeleteRowOnSave = true;

   emit removeItem(mpDisplayedItem);

   delete mpDisplayedItem;
}

void ListInspectorWidget::setDisplayedItem(QListWidgetItem *pDisplayedItem)
{
   VERIFYNRV(mpInspector != NULL);

   if (!mDeleteRowOnSave)
   {
      emit saveInspector(mpInspector, mpDisplayedItem);
   }
   mDeleteRowOnSave = false;

   if (pDisplayedItem == NULL)
   {
      mpDisplayedItem = NULL;
      mpInspector->setDisabled(!mpListWidget->isHidden());
      return;
   }
   else
   {
      mpInspector->setDisabled(false);
   }

   mpDisplayedItem = pDisplayedItem;

   emit loadInspector(mpInspector, pDisplayedItem);
}

QWidget *ListInspectorWidget::getInspector()
{
   return mpInspector;
}

std::string ListInspectorWidget::getUniqueName(const std::string &name) const
{
   QString newName = QString::fromStdString(name);

   int counter = 2; // start with "blah 2"
   while (mpListWidget->findItems(newName, Qt::MatchExactly).empty() == false)
   {
      std::stringstream newStream;
      newStream << name << " " << counter;
      ++counter;
      newName = QString::fromStdString(newStream.str());
   }

   return newName.toStdString();
}

void ListInspectorWidget::setHideList(bool hidden)
{
   if (hidden)
   {
      if (mpDisplayedItem == NULL)
      {
         // no item is currently selected.  Since there won't be a chance for
         // the user to select one with the list hidden, select the first one
         setDisplayedItem(mpListWidget->item(0));
      }
   }

   QWidget* pWidget = mpSplitter->widget(0);
   VERIFYNRV(pWidget != NULL);
   pWidget->setHidden(hidden);
}

void ListInspectorWidget::clearList()
{
   setDisplayedItem(NULL);  
   mpListWidget->clear();
}
