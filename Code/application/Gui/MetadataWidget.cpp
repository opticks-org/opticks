/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "DynamicObjectItemModel.h"
#include "MetadataFilterDlg.h"
#include "MetadataWidget.h"
#include "NameTypeValueDlg.h"
#include "Slot.h"

#include <QtCore/QRegExp>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QClipboard>
#include <QtGui/QComboBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QToolButton>
#include <QtGui/QTreeView>

using namespace std;

MetadataWidget::MetadataWidget(QWidget* parent) :
   QWidget(parent),
   mpMetadata(NULL)
{
   // Metadata
   QLabel* pMetadataLabel = new QLabel("Metadata:", this);

   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Type");
   columnNames.append("Value");

   mpMetadataModel = new DynamicObjectItemModel(this, NULL);
   mpMetadataSortingModel = new QSortFilterProxyModel(this);
   mpMetadataSortingModel->setSortCaseSensitivity(Qt::CaseInsensitive);
   mpMetadataSortingModel->setSourceModel(mpMetadataModel);

   mpMetadataTree = new QTreeView(this);
   mpMetadataTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpMetadataTree->setRootIsDecorated(true);
   mpMetadataTree->setSortingEnabled(true);
   mpMetadataTree->setUniformRowHeights(true);
   mpMetadataTree->setModel(mpMetadataSortingModel);
   mpMetadataTree->setSelectionBehavior(QAbstractItemView::SelectRows);
   mpMetadataTree->setAllColumnsShowFocus(true);
   mpMetadataTree->setContextMenuPolicy(Qt::ActionsContextMenu);
   QAction* pCopyIdAction = new QAction("Copy item ID to clipboard", mpMetadataTree);
   mpMetadataTree->addAction(pCopyIdAction);
   QAction* pCopyValueAction = new QAction("Copy item value to clipboard", mpMetadataTree);
   mpMetadataTree->addAction(pCopyValueAction);

   QHeaderView* pHeader = mpMetadataTree->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setSortIndicatorShown(true);
   }

   // Filter
   mpFilterCheck = new QCheckBox("Filter:", this);
   mpFilterCombo = new QComboBox(this);
   mpFilterCombo->setEditable(false);

   // Buttons
   mpCreateFilterButton = new QToolButton(this);
   mpCreateFilterButton->setIcon(QIcon(":/icons/CreateFilter"));
   mpCreateFilterButton->setToolTip("Create Filter");
   mpCreateFilterButton->setAutoRaise(true);

   mpEditFilterButton = new QToolButton(this);
   mpEditFilterButton->setIcon(QIcon(":/icons/EditFilter"));
   mpEditFilterButton->setToolTip("Edit Filter");
   mpEditFilterButton->setAutoRaise(true);

   mpDeleteFilterButton = new QToolButton(this);
   mpDeleteFilterButton->setIcon(QIcon(":/icons/DeleteFilter"));
   mpDeleteFilterButton->setToolTip("Delete Filter");
   mpDeleteFilterButton->setAutoRaise(true);

   mpModifyButton = new QToolButton(this);
   mpModifyButton->setIcon(QIcon(":/icons/ModifyMetadata"));
   mpModifyButton->setToolTip("Modify Values");
   mpModifyButton->setAutoRaise(true);

   mpAddChildButton = new QToolButton(this);
   mpAddChildButton->setIcon(QIcon(":/icons/AddMetadataChild"));
   mpAddChildButton->setToolTip("Add Child");
   mpAddChildButton->setAutoRaise(true);

   mpAddSiblingButton = new QToolButton(this);
   mpAddSiblingButton->setIcon(QIcon(":/icons/AddMetadataSibling"));
   mpAddSiblingButton->setToolTip("Add Sibling");
   mpAddSiblingButton->setAutoRaise(true);

   mpEditButton = new QToolButton(this);
   mpEditButton->setIcon(QIcon(":/icons/EditMetadataValue"));
   mpEditButton->setToolTip("Edit");
   mpEditButton->setAutoRaise(true);

   mpDeleteButton = new QToolButton(this);
   mpDeleteButton->setIcon(QIcon(":/icons/DeleteMetadataValue"));
   mpDeleteButton->setToolTip("Delete");
   mpDeleteButton->setAutoRaise(true);

   mpClearButton = new QToolButton(this);
   mpClearButton->setIcon(QIcon(":/icons/ClearMetadata"));
   mpClearButton->setToolTip("Clear");
   mpClearButton->setAutoRaise(true);

   // Layout
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(0);
   pButtonLayout->addWidget(mpCreateFilterButton);
   pButtonLayout->addWidget(mpEditFilterButton);
   pButtonLayout->addWidget(mpDeleteFilterButton);
   pButtonLayout->addSpacing(15);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(mpModifyButton);
   pButtonLayout->addSpacing(15);
   pButtonLayout->addWidget(mpAddChildButton);
   pButtonLayout->addWidget(mpAddSiblingButton);
   pButtonLayout->addWidget(mpEditButton);
   pButtonLayout->addWidget(mpDeleteButton);
   pButtonLayout->addWidget(mpClearButton);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pMetadataLabel, 0, 0, 1, 2);
   pGrid->addWidget(mpMetadataTree, 1, 0, 1, 2);
   pGrid->addWidget(mpFilterCheck, 2, 0);
   pGrid->addWidget(mpFilterCombo, 2, 1);
   pGrid->addLayout(pButtonLayout, 3, 0, 1, 2);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(1, 10);

   Service<DesktopServices> pDesktop;
   bool editWarning = pDesktop->getSuppressibleMsgDlgState(getEditWarningDialogId());

   // Initialization
   mpFilterCombo->setEnabled(false);
   mpCreateFilterButton->setEnabled(false);
   mpEditFilterButton->setEnabled(false);
   mpDeleteFilterButton->setEnabled(false);
   mpModifyButton->setEnabled(!editWarning);
   mpAddChildButton->setEnabled(editWarning);
   mpAddSiblingButton->setEnabled(false);
   mpEditButton->setEnabled(false);
   mpDeleteButton->setEnabled(false);
   mpClearButton->setEnabled(editWarning);

   // Connections
   if (editWarning == true)
   {
      VERIFYNR(connect(mpMetadataTree->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
         this, SLOT(currentChanged(const QModelIndex&, const QModelIndex&))));
      VERIFYNR(connect(mpMetadataTree, SIGNAL(doubleClicked(const QModelIndex&)), this,
         SLOT(editSelectedValue(const QModelIndex&))));
   }

   VERIFYNR(connect(mpFilterCheck, SIGNAL(toggled(bool)), this, SLOT(enableFilters(bool))));
   VERIFYNR(connect(mpFilterCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(applyFilter(int))));
   VERIFYNR(connect(mpCreateFilterButton, SIGNAL(clicked()), this, SLOT(createFilter())));
   VERIFYNR(connect(mpEditFilterButton, SIGNAL(clicked()), this, SLOT(editFilter())));
   VERIFYNR(connect(mpDeleteFilterButton, SIGNAL(clicked()), this, SLOT(deleteFilter())));
   VERIFYNR(connect(mpModifyButton, SIGNAL(clicked()), this, SLOT(modifyValues())));
   VERIFYNR(connect(mpAddChildButton, SIGNAL(clicked()), this, SLOT(addKey())));
   VERIFYNR(connect(mpAddSiblingButton, SIGNAL(clicked()), this, SLOT(addKey())));
   VERIFYNR(connect(mpEditButton, SIGNAL(clicked()), this, SLOT(editCurrentValue())));
   VERIFYNR(connect(mpDeleteButton, SIGNAL(clicked()), this, SLOT(deleteKey())));
   VERIFYNR(connect(mpClearButton, SIGNAL(clicked()), this, SLOT(clearKeys())));
   VERIFYNR(connect(pCopyIdAction, SIGNAL(triggered()), this, SLOT(copyIdToClipboard())));
   VERIFYNR(connect(pCopyValueAction, SIGNAL(triggered()), this, SLOT(copyValueToClipboard())));
}

MetadataWidget::~MetadataWidget()
{
   setMetadata(NULL);
}

void MetadataWidget::setMetadata(DynamicObject* pMetadata)
{
   if (pMetadata == mpMetadata)
   {
      return;
   }

   if (mpMetadata != NULL)
   {
      mpMetadata->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &MetadataWidget::metadataDeleted));
   }

   mpMetadata = pMetadata;
   mpMetadataModel->setDynamicObject(mpMetadata);

   if (mpMetadata != NULL)
   {
      mpMetadata->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &MetadataWidget::metadataDeleted));
   }

   if (mpFilterCheck->isChecked() == true)
   {
      int filterIndex = mpFilterCombo->currentIndex();
      applyFilter(filterIndex);
   }
}

const string& MetadataWidget::getEditWarningDialogId()
{
   static string editWarningDialogId = "{A70B4321-44D8-491a-B78C-5ABDA44605F8}";
   return editWarningDialogId;
}

QSize MetadataWidget::sizeHint() const
{
   return QSize(575, 325);
}

void MetadataWidget::enableFilters(bool enable)
{
   mpFilterCombo->setEnabled(enable);
   mpCreateFilterButton->setEnabled(enable);

   int filterIndex = -1;
   if (enable == true)
   {
      filterIndex = mpFilterCombo->currentIndex();
   }

   applyFilter(filterIndex);
}

void MetadataWidget::applyFilter(int filterIndex)
{
   // Get the new name and value filters
   QRegExp nameFilter;
   QRegExp valueFilter;

   if (filterIndex > -1)
   {
      QVariant filter = mpFilterCombo->itemData(filterIndex);
      QMap<QString, QVariant> filterMap = filter.toMap();
      if (filterMap.isEmpty() == false)
      {
         nameFilter = filterMap["Name"].toRegExp();
         valueFilter = filterMap["Value"].toRegExp();
      }
   }

   // Hide and show rows in the tree view according to the filters
   QModelIndex index = mpMetadataTree->rootIndex();

   int numRows = mpMetadataSortingModel->rowCount(index);
   for (int i = 0; i < numRows; ++i)
   {
      applyFilter(i, index, nameFilter, valueFilter);
   }

   // Enable the edit and delete filter buttons
   mpEditFilterButton->setEnabled(filterIndex > -1);
   mpDeleteFilterButton->setEnabled(filterIndex > -1);
}

bool MetadataWidget::applyFilter(int row, const QModelIndex& parent, const QRegExp& nameFilter,
                                 const QRegExp& valueFilter, bool parentMatch)
{
   // Get the attribute name and value
   QModelIndex nameIndex = mpMetadataSortingModel->index(row, 0, parent);
   QModelIndex valueIndex = mpMetadataSortingModel->index(row, 2, parent);

   // Determine whether this attribute should be shown because its parent is shown
   bool attributeMatch = parentMatch;

   // If the attribute does not have a parent or if the parent is not shown,
   // check whether this attribute matches the filter and should be shown
   if (attributeMatch == false)
   {
      // Name
      bool nameMatch = true;
      if (nameFilter.isEmpty() == false)
      {
         QString name = nameIndex.data().toString();
         nameMatch = nameFilter.exactMatch(name);
      }

      // Value
      bool valueMatch = true;
      if (valueFilter.isEmpty() == false)
      {
         QString value = valueIndex.data().toString();
         if (value.isEmpty() == false)
         {
            valueMatch = valueFilter.exactMatch(value);
         }
         else
         {
            valueMatch = false;
         }
      }

      if ((nameMatch == true) && (valueMatch == true))
      {
         attributeMatch = true;
      }
   }

   // Show or hide the child attributes
   bool childMatch = false;

   int numRows = mpMetadataSortingModel->rowCount(nameIndex);
   for (int i = 0; i < numRows; ++i)
   {
      // Force child attributes to be shown if this attribute is shown
      if (applyFilter(i, nameIndex, nameFilter, valueFilter, attributeMatch) == true)
      {
         childMatch = true;
      }
   }

   // If at least one child attribute matches the filter, ensure that this attribute is shown
   if (childMatch == true)
   {
      attributeMatch = true;
   }

   // Show or hide this attribute
   mpMetadataTree->setRowHidden(row, parent, !attributeMatch);

   // Return whether this attribute needs to be shown, which would cause a parent attribute to be shown
   return attributeMatch;
}

bool MetadataWidget::isFilterNameUnique(const QString& filterName, int ignoreIndex) const
{
   if (filterName.isEmpty() == true)
   {
      return false;
   }

   for (int i = 0; i < mpFilterCombo->count(); ++i)
   {
      if (i == ignoreIndex)
      {
         continue;
      }

      QString currentFilterName = mpFilterCombo->itemText(i);
      if (currentFilterName == filterName)
      {
         return false;
      }
   }

   return true;
}

void MetadataWidget::createFilter()
{
   MetadataFilterDlg filterDlg(this);
   filterDlg.setWindowTitle("Create Filter");
   filterDlg.setFilterName("Filter " + QString::number(mpFilterCombo->count() + 1));

   QString filterName;

   bool uniqueName = false;
   while (uniqueName == false)
   {
      if (filterDlg.exec() == QDialog::Rejected)
      {
         return;
      }

      filterName = filterDlg.getFilterName();   // The dialog ensures that the filter name is not empty
      uniqueName = isFilterNameUnique(filterName);
      if (uniqueName == false)
      {
         QMessageBox::warning(this, APP_NAME, "Another filter exists with the same name.  "
            "Please select a unique name for the filter.");
      }
   }

   QMap<QString, QVariant> filter;
   filter.insert("Name", QVariant(filterDlg.getNameFilter()));
   filter.insert("Value", QVariant(filterDlg.getValueFilter()));

   mpFilterCombo->addItem(filterName, filter);
   mpFilterCombo->setCurrentIndex(mpFilterCombo->count() - 1);    // Automatically updates the tree view
}

void MetadataWidget::editFilter()
{
   int filterIndex = mpFilterCombo->currentIndex();
   QString filterName = mpFilterCombo->currentText();
   QMap<QString, QVariant> filter = mpFilterCombo->itemData(filterIndex).toMap();
   VERIFYNRV(filter.isEmpty() == false);

   MetadataFilterDlg filterDlg(this);
   filterDlg.setWindowTitle("Edit Filter");
   filterDlg.setFilterName(filterName);
   filterDlg.setNameFilter(filter["Name"].toRegExp());
   filterDlg.setValueFilter(filter["Value"].toRegExp());

   bool uniqueName = false;
   while (uniqueName == false)
   {
      if (filterDlg.exec() == QDialog::Rejected)
      {
         return;
      }

      filterName = filterDlg.getFilterName();   // The dialog ensures that the filter name is not empty
      uniqueName = isFilterNameUnique(filterName, filterIndex);
      if (uniqueName == false)
      {
         QMessageBox::warning(this, APP_NAME, "Another filter exists with the same name.  "
            "Please select a unique name for the filter.");
      }
   }

   if (filterName.isEmpty() == false)
   {
      mpFilterCombo->setItemText(filterIndex, filterName);
   }

   filter["Name"] = QVariant(filterDlg.getNameFilter());
   filter["Value"] = QVariant(filterDlg.getValueFilter());
   mpFilterCombo->setItemData(filterIndex, QVariant(filter));

   applyFilter(filterIndex);
}

void MetadataWidget::deleteFilter()
{
   int filterIndex = mpFilterCombo->currentIndex();
   mpFilterCombo->removeItem(filterIndex);      // Automatically updates the tree view
}

void MetadataWidget::modifyValues()
{
   Service<DesktopServices> pDesktop;

   pDesktop->showSuppressibleMsgDlg(APP_NAME, "Modifying the metadata values could have adverse effects since "
         "some plug-ins may require the existence of specific metadata key-value pairs or may ignore changes made "
         "in this dialog. For example, the NITF exporter will not export any changes made in this dialog to the "
         "security marking values in the metadata. Also, other widgets in "
         "this same dialog displaying portions of the metadata contents may not update properly.", MESSAGE_WARNING,
         getEditWarningDialogId(), this);

   mpMetadataTree->selectionModel()->clear();
   mpModifyButton->setEnabled(false);
   mpAddChildButton->setEnabled(true);
   mpAddSiblingButton->setEnabled(false);
   mpDeleteButton->setEnabled(false);
   mpClearButton->setEnabled(mpMetadata != NULL);

   VERIFYNR(connect(mpMetadataTree->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
      this, SLOT(currentChanged(const QModelIndex&, const QModelIndex&))));
   VERIFYNR(connect(mpMetadataTree, SIGNAL(doubleClicked(const QModelIndex&)), this,
      SLOT(editSelectedValue(const QModelIndex&))));
}

void MetadataWidget::addKey()
{
   // Get the dynamic object
   DynamicObject* pDynamicObject = NULL;
   QModelIndex parent;

   QObject* pButton = sender();
   if (pButton == mpAddChildButton)
   {
      parent = mpMetadataSortingModel->mapToSource(mpMetadataTree->currentIndex());
   }
   else if (pButton == mpAddSiblingButton)
   {
      parent = mpMetadataSortingModel->mapToSource(mpMetadataTree->currentIndex().parent());
   }

   if (parent.isValid() == true)
   {
      DataVariant* pValue = parent.data(DataVariantRole).value<DataVariant*>();
      if ((pValue != NULL) && (pValue->isValid() == true))
      {
         if (pValue->getTypeName() == "DynamicObject")
         {
            pDynamicObject = dv_cast<DynamicObject>(pValue);
         }
      }
   }
   else
   {
      pDynamicObject = mpMetadata;
   }

   if (pDynamicObject == NULL)
   {
      return;
   }

   // Get the new name and value from the user
   NameTypeValueDlg dlg(this);
   dlg.setWindowTitle("Add Metadata Value");
   if (dlg.exec() == QDialog::Accepted)
   {
      string key = dlg.getName().toStdString();
      const DataVariant& var = dlg.getValue();

      // Update the value
      pDynamicObject->setAttribute(key, var);

      // Expand the parent dynamic object item to see the new child attribute
      if (parent.isValid() == true)
      {
         mpMetadataTree->expand(mpMetadataSortingModel->mapFromSource(parent));
      }
   }
}

void MetadataWidget::editCurrentValue()
{
   QModelIndex selectedItem = mpMetadataTree->currentIndex();
   editSelectedValue(selectedItem);
}

void MetadataWidget::deleteKey()
{
   // Get the parent dynamic object
   DynamicObject* pDynamicObject = NULL;

   QModelIndex parent = mpMetadataSortingModel->mapToSource(mpMetadataTree->currentIndex().parent());
   if (parent.isValid() == true)
   {
      DataVariant* pValue = parent.data(DataVariantRole).value<DataVariant*>();
      if ((pValue != NULL) && (pValue->isValid() == true))
      {
         if (pValue->getTypeName() == "DynamicObject")
         {
            pDynamicObject = dv_cast<DynamicObject>(pValue);
         }
      }
   }
   else
   {
      pDynamicObject = mpMetadata;
   }

   if (pDynamicObject == NULL)
   {
      return;
   }

   // Prompt the user to delete all child attributes
   QModelIndex index = mpMetadataSortingModel->mapToSource(mpMetadataTree->currentIndex());
   index = index.sibling(index.row(), 0);

   if (mpMetadataModel->rowCount(index) > 0)
   {
      if ((mpFilterCheck->isChecked() == true) && (mpFilterCombo->currentText().isEmpty() == false))
      {
         if (QMessageBox::warning(this, APP_NAME, "This will delete all child attributes, not just "
            "the attributes displayed by the applied filter.  Do you want to continue?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
         {
            return;
         }
      }
   }

   // Remove the attribute
   string key = index.data(Qt::DisplayRole).toString().toStdString();
   pDynamicObject->removeAttribute(key);
}

void MetadataWidget::clearKeys()
{
   if (mpMetadata != NULL)
   {
      if ((mpFilterCheck->isChecked() == true) && (mpFilterCombo->currentText().isEmpty() == false))
      {
         if (QMessageBox::warning(this, APP_NAME, "This will clear all metadata attributes, not just "
            "the attributes displayed by the applied filter.  Do you want to continue?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
         {
            return;
         }
      }

      mpMetadata->clear();
   }
}

void MetadataWidget::currentChanged(const QModelIndex& selectedItem, const QModelIndex& deselectedItem)
{
   QModelIndex selected = mpMetadataSortingModel->mapToSource(selectedItem);

   bool bSelectedItem = selected.isValid();
   mpAddChildButton->setEnabled(true);
   mpAddSiblingButton->setEnabled(bSelectedItem);
   mpEditButton->setEnabled(bSelectedItem);
   mpDeleteButton->setEnabled(bSelectedItem);

   if (bSelectedItem == true)
   {
      DataVariant* pValue = selected.data(DataVariantRole).value<DataVariant*>();
      if ((pValue != NULL) && (pValue->isValid() == true))
      {
         if (pValue->getTypeName() == "DynamicObject")
         {
            mpEditButton->setEnabled(false);
         }
         else
         {
            mpAddChildButton->setEnabled(false);
         }
      }
   }
}

void MetadataWidget::editSelectedValue(const QModelIndex& selectedItem)
{
   // Do not allow editing of a dynamic object item
   QModelIndex index = mpMetadataSortingModel->mapToSource(selectedItem);
   if (index.isValid() == true)
   {
      DataVariant* pValue = index.data(DataVariantRole).value<DataVariant*>();
      if ((pValue != NULL) && (pValue->isValid() == true))
      {
         if (pValue->getTypeName() == "DynamicObject")
         {
            return;
         }
      }
   }
   else
   {
      return;
   }

   // Get the parent dynamic object
   DynamicObject* pDynamicObject = NULL;

   QModelIndex parent = mpMetadataSortingModel->mapToSource(selectedItem.parent());
   if (parent.isValid() == true)
   {
      DataVariant* pValue = parent.data(DataVariantRole).value<DataVariant*>();
      if ((pValue != NULL) && (pValue->isValid() == true))
      {
         if (pValue->getTypeName() == "DynamicObject")
         {
            pDynamicObject = dv_cast<DynamicObject>(pValue);
         }
      }
   }
   else
   {
      pDynamicObject = mpMetadata;
   }

   if (pDynamicObject == NULL)
   {
      return;
   }

   // Get the current name and value
   index = index.sibling(index.row(), 0);

   QString name = index.data(Qt::DisplayRole).toString();
   DataVariant* pValue = index.data(DataVariantRole).value<DataVariant*>();
   VERIFYNRV(pValue != NULL);

   // Get the new name and value from the user
   NameTypeValueDlg dlg(this);
   dlg.setWindowTitle("Edit Metadata Value");
   dlg.setValue(name, *pValue);
   if (dlg.exec() == QDialog::Accepted)
   {
      QString newName = dlg.getName();
      if (newName.isEmpty() == false)
      {
         // Update the value
         const DataVariant& newValue = dlg.getValue();
         pDynamicObject->setAttribute(newName.toStdString(), newValue);
      }
   }
}

void MetadataWidget::copyIdToClipboard()
{
   QModelIndexList selected = mpMetadataTree->selectionModel()->selectedIndexes();
   if (selected.size() >= 1)
   {
      QStringList id;
      QModelIndex idx = selected.front();
      while (idx.isValid())
      {
         QString tmp = idx.data().toString();
         tmp.replace("/", "//");
         id.push_front(tmp);
         idx = idx.parent();
      }
      QApplication::clipboard()->setText(id.join("/"));
   }
}

void MetadataWidget::copyValueToClipboard()
{
   QModelIndexList selected = mpMetadataTree->selectionModel()->selectedIndexes();
   if (selected.size() >= 1)
   {
      QModelIndex idx = selected.front();
      if (idx.isValid())
      {
         DataVariant* pValue = idx.data(DataVariantRole).value<DataVariant*>();
         if (pValue != NULL && pValue->isValid())
         {
            QApplication::clipboard()->setText(QString::fromStdString(pValue->toDisplayString()));
         }
      }
   }
}

void MetadataWidget::metadataDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   setMetadata(NULL);
}
