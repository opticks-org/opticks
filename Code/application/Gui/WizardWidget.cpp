/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QLayout>
#include <QtGui/QSplitter>
#include <QtGui/QVBoxLayout>

#include "WizardWidget.h"
#include "AppVerify.h"
#include "DataVariantEditor.h"
#include "BatchWizard.h"

#include <string>
#include <vector>
using namespace std;

WizardWidget::WizardWidget(QWidget* parent) :
   QWidget(parent)
{
   mpWizard = NULL;

   // Splitter
   QSplitter* pSplitter = new QSplitter(Qt::Horizontal, this);
   pSplitter->setOpaqueResize(true);

   // Name
   QWidget* pNameWidget = new QWidget(pSplitter);
   QLabel* pNameLabel = new QLabel("Value Name:", pNameWidget);

   mpNameTree = new QTreeWidget(pNameWidget);
   mpNameTree->setRootIsDecorated(true);
   mpNameTree->setColumnCount(1);
   mpNameTree->setSelectionMode(QAbstractItemView::SingleSelection);

   QTreeWidgetItem* pHeaderItem = mpNameTree->headerItem();
   if (pHeaderItem != NULL)
   {
      mpNameTree->setItemHidden(pHeaderItem, true);
   }

   VERIFYNR(connect(mpNameTree, SIGNAL(itemSelectionChanged()), this, SLOT(showSelectedValues())));

   QGridLayout* pNameGrid = new QGridLayout(pNameWidget);
   pNameGrid->setMargin(0);
   pNameGrid->setSpacing(5);
   pNameGrid->addWidget(pNameLabel, 0, 0);
   pNameGrid->addWidget(mpNameTree, 1, 0);
   pNameGrid->setColumnMinimumWidth(1, 5);
   pNameGrid->setRowStretch(1, 10);
   pNameGrid->setColumnStretch(0, 10);

   // Type
   QWidget* pValueWidget = new QWidget(pSplitter);

   QLabel* pTypeLabel = new QLabel("Type:", pValueWidget);
   mpTypeLabel = new QLabel();

   mpTypeCombo = new QComboBox();
   mpTypeCombo->setEditable(false);
   mpTypeCombo->addItem("Filename");
   mpTypeCombo->addItem("File set");
   VERIFYNR(connect(mpTypeCombo, SIGNAL(activated(const QString&)), this, SLOT(setValueWidgets(const QString&))));

   mpTypeStack = new QStackedWidget(pValueWidget);
   mpTypeStack->addWidget(mpTypeLabel);
   mpTypeStack->addWidget(mpTypeCombo);

   // Value
   mpValueStack = new QStackedWidget(pValueWidget);

   // Value editor
   mpValueEditorWidget = new QWidget(mpValueStack);
   QLabel* pValueEditorLabel = new QLabel("Value:", mpValueEditorWidget);
   mpValueEditor = new DataVariantEditor(mpValueEditorWidget);
   mpValueStack->addWidget(mpValueEditorWidget);

   QVBoxLayout* pValueEditorLayout = new QVBoxLayout(mpValueEditorWidget);
   pValueEditorLayout->setMargin(0);
   pValueEditorLayout->setSpacing(5);
   pValueEditorLayout->addWidget(pValueEditorLabel);
   pValueEditorLayout->addWidget(mpValueEditor);

   // File set widget
   mpFilesetWidget = new QWidget(mpValueStack);
   QLabel* pFilesetLabel = new QLabel("File Set:", mpFilesetWidget);
   mpValueFilesetCombo = new QComboBox(mpFilesetWidget);
   mpValueFilesetCombo->setEditable(false);
   mpValueStack->addWidget(mpFilesetWidget);

   QVBoxLayout* pFilesetLayout = new QVBoxLayout(mpFilesetWidget);
   pFilesetLayout->setMargin(0);
   pFilesetLayout->setSpacing(5);
   pFilesetLayout->addWidget(pFilesetLabel);
   pFilesetLayout->addWidget(mpValueFilesetCombo);
   pFilesetLayout->addStretch();

   QGridLayout* pValueGrid = new QGridLayout(pValueWidget);
   pValueGrid->setMargin(0);
   pValueGrid->setSpacing(5);
   pValueGrid->addWidget(pTypeLabel, 0, 0);
   pValueGrid->addWidget(mpTypeStack, 0, 1);
   pValueGrid->addWidget(mpValueStack, 1, 0, 1, 2);
   pValueGrid->setRowStretch(1, 10);
   pValueGrid->setColumnStretch(1, 10);

   // Repeating fileset
   mpRepeatCheck = new QCheckBox("Repeat wizard using fileset:", this);
   mpFilesetCombo = new QComboBox(this);
   mpFilesetCombo->setEditable(false);
   mpFilesetCombo->setFixedWidth(200);
   VERIFYNR(connect(mpRepeatCheck, SIGNAL(toggled(bool)), mpFilesetCombo, SLOT(setEnabled(bool))));

   QHBoxLayout* pRepeatLayout = new QHBoxLayout();
   pRepeatLayout->setMargin(0);
   pRepeatLayout->setSpacing(5);
   pRepeatLayout->addWidget(mpRepeatCheck);
   pRepeatLayout->addWidget(mpFilesetCombo);
   pRepeatLayout->addStretch();

   // Clean up
   mpCleanupCheck = new QCheckBox("Delete data elements after wizard execution", this);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pSplitter, 0, 0);
   pGrid->addLayout(pRepeatLayout, 1, 0);
   pGrid->addWidget(mpCleanupCheck, 2, 0);

   // Initialization
   setActiveWizard(NULL, QStringList());

   // Connections
   VERIFYNR(connect(this, SIGNAL(modified()), this, SLOT(updateBatchWizardValues())));
}

WizardWidget::~WizardWidget()
{
}

void WizardWidget::setActiveWizard(BatchWizard* pWizard, QStringList filesetNames)
{
   disconnect(mpFilesetCombo, SIGNAL(activated(const QString&)), this, SIGNAL(modified()));
   disconnect(mpRepeatCheck, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
   disconnect(mpCleanupCheck, SIGNAL(toggled(bool)), this, SIGNAL(modified()));

   bool bEnable = false;
   QString strWizardName;
   bool bRepeat = false;
   bool bCleanup = false;

   mpNameTree->clear();
   mValues.clear();
   mpFilesetCombo->clear();
   mpValueFilesetCombo->clear();

   QTreeWidgetItem* pFirstItem = NULL;
   if (pWizard != NULL)
   {
      bEnable = true;

      // Add the wizard items to the list
      vector<Value*> inputValues = pWizard->getInputValues();

      vector<Value*>::iterator iter;
      for (iter = inputValues.begin(); iter != inputValues.end(); iter++)
      {
         Value* pValue = NULL;
         pValue = *iter;
         if (pValue != NULL)
         {
            string name = "";
            name = pValue->getNodeName();
            if (name.empty() == false)
            {
               QTreeWidgetItem* pItemItem = NULL;

               string itemName = pValue->getItemName();
               if (itemName.empty() == false)
               {
                  pItemItem = getItemItem(QString::fromStdString(itemName));
                  if (pItemItem == NULL)
                  {
                     pItemItem = new QTreeWidgetItem(mpNameTree);
                     pItemItem->setText(0, QString::fromStdString(itemName));
                     pItemItem->setFlags(Qt::ItemIsEnabled);
                  }
               }

               QTreeWidgetItem* pItem = NULL;
               if (pItemItem == NULL)
               {
                  pItem = new QTreeWidgetItem(mpNameTree);
               }
               else
               {
                  pItem = new QTreeWidgetItem(pItemItem);
               }

               if (pItem != NULL)
               {
                  pItem->setText(0, QString::fromStdString(name));
                  mValues.insert(pItem, pValue);
               }

               if (pFirstItem == NULL)
               {
                  pFirstItem = pItem;
               }
            }
         }
      }

      // Repeat
      mpFilesetCombo->addItems(filesetNames);
      mpValueFilesetCombo->addItems(filesetNames);

      string repeatName = "";
      bRepeat = pWizard->isRepeating(repeatName);
      if (repeatName.empty() == false)
      {
         int iIndex = mpFilesetCombo->findText(QString::fromStdString(repeatName));
         if (iIndex != -1)
         {
            mpFilesetCombo->setCurrentIndex(iIndex);
         }
      }

      // Clean up
      bCleanup = pWizard->doesCleanup();
   }

   mpWizard = pWizard;

   // Set the current value
   showValues(pFirstItem);

   if (pFirstItem != NULL)
   {
      QTreeWidgetItem* pParentItem = pFirstItem->parent();
      if (pParentItem != NULL)
      {
         mpNameTree->expandItem(pParentItem);
      }

      mpNameTree->setItemSelected(pFirstItem, true);
   }

   mpRepeatCheck->setChecked(bRepeat);
   mpCleanupCheck->setChecked(bCleanup);

   mpNameTree->setEnabled(bEnable);
   mpTypeStack->setEnabled(bEnable);
   mpValueStack->setEnabled(bEnable);
   mpRepeatCheck->setEnabled(bEnable);
   mpFilesetCombo->setEnabled(bEnable);
   mpCleanupCheck->setEnabled(bEnable);

   if (bEnable == true)
   {
      mpRepeatCheck->setEnabled(!(filesetNames.isEmpty()));
      mpFilesetCombo->setEnabled(bRepeat);
   }

   VERIFYNR(connect(mpFilesetCombo, SIGNAL(activated(const QString&)), this, SIGNAL(modified())));
   VERIFYNR(connect(mpRepeatCheck, SIGNAL(toggled(bool)), this, SIGNAL(modified())));
   VERIFYNR(connect(mpCleanupCheck, SIGNAL(toggled(bool)), this, SIGNAL(modified())));
}

QTreeWidgetItem* WizardWidget::getItemItem(const QString& strItem) const
{
   if (strItem.isEmpty() == true)
   {
      return NULL;
   }

   QTreeWidgetItemIterator iter(mpNameTree);
   while (*iter != NULL)
   {
      QTreeWidgetItem* pItem = *iter;
      if (pItem != NULL)
      {
         QString strCurrentItem = pItem->text(0);
         if (strCurrentItem == strItem)
         {
            return pItem;
         }
      }

      ++iter;
   }

   return NULL;
}

Value* WizardWidget::getCurrentValue() const
{
   QList<QTreeWidgetItem*> selectedItems = mpNameTree->selectedItems();
   if (selectedItems.empty() == false)
   {
      VERIFYRV(selectedItems.count() == 1, NULL);

      QTreeWidgetItem* pItem = selectedItems.front();
      if (pItem != NULL)
      {
         QMap<QTreeWidgetItem*, Value*>::ConstIterator iter = mValues.find(pItem);
         if (iter != mValues.end())
         {
            Value* pValue = iter.value();
            return pValue;
         }
      }
   }

   return NULL;
}

void WizardWidget::showValues(QTreeWidgetItem* pItem)
{
   // Get the value for the tree widget item
   QString strNodeType;
   DataVariant value;

   if (pItem != NULL)
   {
      QMap<QTreeWidgetItem*, Value*>::Iterator iter = mValues.find(pItem);
      if (iter != mValues.end())
      {
         Value* pValue = iter.value();
         if (pValue != NULL)
         {
            strNodeType = QString::fromStdString(pValue->getNodeType());
            value = pValue->getValue();
         }
      }
   }

   disconnect(mpTypeCombo, SIGNAL(activated(const QString&)), this, SIGNAL(modified()));
   disconnect(mpValueEditor, SIGNAL(modified()), this, SIGNAL(modified()));
   disconnect(mpValueFilesetCombo, SIGNAL(activated(const QString&)), this, SIGNAL(modified()));

   // Set the current widgets to display the value
   setValueWidgets(strNodeType);

   // Set the value in the widgets
   if (strNodeType == "File set")
   {
      string filesetName = value.toDisplayString();
      if (filesetName.empty() == false)
      {
         int index = mpValueFilesetCombo->findText(QString::fromStdString(filesetName));
         if (index != -1)
         {
            mpValueFilesetCombo->setCurrentIndex(index);
         }
      }
   }
   else
   {
      mpValueEditor->setValue(value);
   }

   VERIFYNR(connect(mpTypeCombo, SIGNAL(activated(const QString&)), this, SIGNAL(modified())));
   VERIFYNR(connect(mpValueEditor, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(mpValueFilesetCombo, SIGNAL(activated(const QString&)), this, SIGNAL(modified())));
}

void WizardWidget::showSelectedValues()
{
   QList<QTreeWidgetItem*> selectedItems = mpNameTree->selectedItems();
   if (selectedItems.empty() == false)
   {
      VERIFYNRV(selectedItems.count() == 1);

      QTreeWidgetItem* pItem = selectedItems.front();
      if (pItem != NULL)
      {
         showValues(pItem);
      }
   }
}

void WizardWidget::setValueWidgets(const QString& strNodeType)
{
   QWidget* pTypeWidget = mpTypeLabel;
   QWidget* pEditWidget = mpValueEditorWidget;

   if (strNodeType == "Filename")
   {
      if (mpFilesetCombo->count() > 0)
      {
         pTypeWidget = mpTypeCombo;
         mpTypeCombo->setCurrentIndex(0);
      }

      Value* pValue = getCurrentValue();
      if (pValue != NULL)
      {
         if (pValue->getNodeType() == "File set")
         {
            const DataVariant& currentValue = pValue->getValue();

            string filename = currentValue.toDisplayString();
            if (filename.empty() == false)
            {
               DataVariant value;
               value.fromDisplayString("Filename", filename);
               mpValueEditor->setValue(value);
            }
         }
      }
   }
   else if (strNodeType == "File set")
   {
      pTypeWidget = mpTypeCombo;
      pEditWidget = mpFilesetWidget;
      mpTypeCombo->setCurrentIndex(1);
   }

   mpTypeLabel->setText(strNodeType);
   mpTypeStack->setCurrentWidget(pTypeWidget);
   mpValueStack->setCurrentWidget(pEditWidget);
}

void WizardWidget::updateBatchWizardValues()
{
   if (mpWizard == NULL)
   {
      return;
   }

   // Value
   Value* pValue = getCurrentValue();
   if (pValue != NULL)
   {
      QWidget* pCurrentWidget = mpValueStack->currentWidget();
      if (pCurrentWidget == mpFilesetWidget)
      {
         // Node type
         QString strType = mpTypeCombo->currentText();
         if (strType.isEmpty() == false)
         {
            pValue->setNodeType(strType.toStdString());
         }

         // Value
         DataVariant value;

         string filesetName = mpValueFilesetCombo->currentText().toStdString();
         if (filesetName.empty() == false)
         {
            value = DataVariant("string", &filesetName);
         }

         pValue->setValue(value);
      }
      else if (pCurrentWidget == mpValueEditorWidget)
      {
         // Node type
         if (mpTypeStack->currentWidget() == mpTypeCombo)
         {
            QString strType = mpTypeCombo->currentText();
            if (strType.isEmpty() == false)
            {
               pValue->setNodeType(strType.toStdString());
            }
         }

         // Value
         const DataVariant& value = mpValueEditor->getValue();
         pValue->setValue(value);
      }
   }

   // Repeat
   BatchFileset* pRepeatFileset = NULL;

   bool bRepeat = false;
   bRepeat = mpRepeatCheck->isChecked();
   if (bRepeat == true)
   {
      QString strFileset = mpFilesetCombo->currentText();
      if (strFileset.isEmpty() == false)
      {
         pRepeatFileset = mpWizard->getFileset(strFileset.toStdString());
      }
   }

   mpWizard->setRepeatFileset(pRepeatFileset);

   // Cleanup
   bool bCleanup = false;
   bCleanup = mpCleanupCheck->isChecked();

   mpWizard->setCleanup(bCleanup);
}
