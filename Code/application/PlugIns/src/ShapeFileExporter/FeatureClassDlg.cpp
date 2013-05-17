/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ConfigurationSettings.h"
#include "FeatureClassDlg.h"

#include <QtCore/QRegExp>
#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleValidator>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QIntValidator>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QRegExpValidator>
#include <QtGui/QToolButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>

#include <string>
#include <vector>

#define FEATURE_CLASS_SETTING "ShapeFileExporter/FeatureClasses"

FeatureClassDlg::FeatureClassDlg(QWidget* pParent) :
   QDialog(pParent)
{
   // Classes
   QLabel* pClassLabel = new QLabel("Feature Classes:", this);

   QToolButton* pAddFeatureButton = new QToolButton(this);
   pAddFeatureButton->setAutoRaise(true);
   pAddFeatureButton->setIcon(QIcon(":/icons/New"));
   pAddFeatureButton->setToolTip("Add Feature");

   QToolButton* pRemoveFeatureButton = new QToolButton(this);
   pRemoveFeatureButton->setAutoRaise(true);
   pRemoveFeatureButton->setIcon(QIcon(":/icons/Delete"));
   pRemoveFeatureButton->setToolTip("Remove Feature");

   mpClassList = new QListWidget(this);
   mpClassList->setFixedWidth(150);
   mpClassList->setSelectionMode(QAbstractItemView::SingleSelection);
   mpClassList->setSortingEnabled(true);
   mpClassList->sortItems(Qt::AscendingOrder);

   // Fields
   QLabel* pFieldLabel = new QLabel("Fields:", this);

   QToolButton* pAddFieldButton = new QToolButton(this);
   pAddFieldButton->setAutoRaise(true);
   pAddFieldButton->setIcon(QIcon(":/icons/New"));
   pAddFieldButton->setToolTip("Add Field");

   QToolButton* pRemoveFieldButton = new QToolButton(this);
   pRemoveFieldButton->setAutoRaise(true);
   pRemoveFieldButton->setIcon(QIcon(":/icons/Delete"));
   pRemoveFieldButton->setToolTip("Remove Field");

   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Type");
   columnNames.append("Default Value");

   mpFieldTree = new QTreeWidget(this);
   mpFieldTree->setColumnCount(3);
   mpFieldTree->setHeaderLabels(columnNames);
   mpFieldTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpFieldTree->setRootIsDecorated(false);
   mpFieldTree->setAllColumnsShowFocus(true);
   mpFieldTree->setSortingEnabled(true);
   mpFieldTree->sortByColumn(0, Qt::AscendingOrder);
   mpFieldTree->setItemDelegateForColumn(0, new FieldNameDelegate(mpFieldTree));
   mpFieldTree->setItemDelegateForColumn(1, new FieldTypeDelegate(mpFieldTree));
   mpFieldTree->setItemDelegateForColumn(2, new FieldValueDelegate(mpFieldTree));

   QHeaderView* pHeader = mpFieldTree->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
      pHeader->setSortIndicatorShown(true);
   }

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Dialog buttons
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, this);

   // Layout
   QHBoxLayout* pFeatureButtonLayout = new QHBoxLayout();
   pFeatureButtonLayout->setMargin(0);
   pFeatureButtonLayout->setSpacing(0);
   pFeatureButtonLayout->addWidget(pClassLabel);
   pFeatureButtonLayout->addSpacing(10);
   pFeatureButtonLayout->addStretch();
   pFeatureButtonLayout->addWidget(pAddFeatureButton);
   pFeatureButtonLayout->addWidget(pRemoveFeatureButton);

   QVBoxLayout* pFeatureClassLayout = new QVBoxLayout();
   pFeatureClassLayout->setMargin(0);
   pFeatureClassLayout->setSpacing(2);
   pFeatureClassLayout->addLayout(pFeatureButtonLayout);
   pFeatureClassLayout->addWidget(mpClassList, 10);

   QHBoxLayout* pFieldButtonLayout = new QHBoxLayout();
   pFieldButtonLayout->setMargin(0);
   pFieldButtonLayout->setSpacing(0);
   pFieldButtonLayout->addWidget(pFieldLabel);
   pFieldButtonLayout->addSpacing(10);
   pFieldButtonLayout->addStretch();
   pFieldButtonLayout->addWidget(pAddFieldButton);
   pFieldButtonLayout->addWidget(pRemoveFieldButton);

   QVBoxLayout* pFieldLayout = new QVBoxLayout();
   pFieldLayout->setMargin(0);
   pFieldLayout->setSpacing(2);
   pFieldLayout->addLayout(pFieldButtonLayout);
   pFieldLayout->addWidget(mpFieldTree, 10);

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addLayout(pFeatureClassLayout, 0, 0);
   pLayout->addLayout(pFieldLayout, 0, 1);
   pLayout->addWidget(pLine, 1, 0, 1, 2);
   pLayout->addWidget(pButtonBox, 2, 0, 1, 2);
   pLayout->setRowStretch(0, 10);
   pLayout->setColumnStretch(1, 10);

   // Initialization
   setWindowTitle("Feature Classes");
   resize(550, 300);
   loadFromSettings();
   updateFields();

   // Connections
   VERIFYNR(connect(pAddFeatureButton, SIGNAL(clicked()), this, SLOT(addFeatureClass())));
   VERIFYNR(connect(pRemoveFeatureButton, SIGNAL(clicked()), this, SLOT(removeFeatureClass())));
   VERIFYNR(connect(mpClassList, SIGNAL(itemChanged(QListWidgetItem*)), this,
      SLOT(setFeatureClassData(QListWidgetItem*))));
   VERIFYNR(connect(mpClassList, SIGNAL(itemSelectionChanged()), this, SLOT(updateFields())));
   VERIFYNR(connect(pAddFieldButton, SIGNAL(clicked()), this, SLOT(addField())));
   VERIFYNR(connect(pRemoveFieldButton, SIGNAL(clicked()), this, SLOT(removeField())));
   VERIFYNR(connect(mpFieldTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(setFieldData(QTreeWidgetItem*, int))));
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
}

FeatureClassDlg::~FeatureClassDlg()
{}

void FeatureClassDlg::accept()
{
   saveToSettings();
   QDialog::accept();
}

void FeatureClassDlg::loadFromSettings()
{
   mpClassList->clear();

   Service<ConfigurationSettings> pSettings;
   const DataVariant& classes = pSettings->getSetting(FEATURE_CLASS_SETTING);

   const DynamicObject* pClasses = classes.getPointerToValue<DynamicObject>();
   if (pClasses == NULL)
   {
      return;
   }

   mpFeatureClasses->merge(pClasses);

   std::vector<std::string> classNames;
   mpFeatureClasses->getAttributeNames(classNames);
   for (std::vector<std::string>::const_iterator iter = classNames.begin(); iter != classNames.end(); ++iter)
   {
      QString className = QString::fromStdString(*iter);
      if (className.isEmpty() == false)
      {
         QListWidgetItem* pItem = new QListWidgetItem(className, mpClassList);
         pItem->setData(Qt::UserRole, QVariant(className));
         pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
         mpClassList->addItem(pItem);
      }
   }

   QListWidgetItem* pItem = mpClassList->item(0);
   if (pItem != NULL)
   {
      pItem->setSelected(true);
   }
}

void FeatureClassDlg::saveToSettings() const
{
   Service<ConfigurationSettings> pSettings;
   pSettings->setSetting(FEATURE_CLASS_SETTING, *(mpFeatureClasses.get()));
}

void FeatureClassDlg::addFeatureClass()
{
   // Assign a unique default name for the feature class
   int featureClassNumber = 1;
   QString className = "Feature Class " + QString::number(featureClassNumber);
   while (mpClassList->findItems(className, Qt::MatchExactly).empty() == false)
   {
      className = "Feature Class " + QString::number(++featureClassNumber);
   }

   // Add the feature class to the member DynamicObject
   FactoryResource<DynamicObject> pFields;
   mpFeatureClasses->setAttribute(className.toStdString(), *(pFields.get()));

   // Create a new list widget item and select it
   QListWidgetItem* pItem = new QListWidgetItem(className, mpClassList);
   pItem->setData(Qt::UserRole, QVariant(className));
   pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);

   mpClassList->addItem(pItem);
   mpClassList->setCurrentItem(pItem);
}

void FeatureClassDlg::removeFeatureClass()
{
   QList<QListWidgetItem*> items = mpClassList->selectedItems();
   for (int i = 0; i < items.count(); ++i)
   {
      QListWidgetItem* pItem = items[i];
      if (pItem != NULL)
      {
         QString className = pItem->text();

         // Remove the feature class from the member dynamic object
         mpFeatureClasses->removeAttribute(className.toStdString());

         // Remove the item from the list widget
         delete pItem;
      }
   }

   // The selected item was deleted, so just select the first item in the list
   QListWidgetItem* pItem = mpClassList->item(0);
   if (pItem != NULL)
   {
      pItem->setSelected(true);
   }
}

void FeatureClassDlg::setFeatureClassData(QListWidgetItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   // This slot gets called whenever anything on the item changes (e.g. selection state),
   // so check to be sure that the user actually renamed the item
   QString oldClassName = pItem->data(Qt::UserRole).toString();
   QString newClassName = pItem->text();

   if (newClassName == oldClassName)
   {
      return;
   }

   if (mpClassList->findItems(newClassName, Qt::MatchExactly).count() > 1)
   {
      QMessageBox::warning(this, windowTitle(), "Another feature class exists with the same name.  "
         "Please choose a unique name for the feature class.");
      pItem->setText(oldClassName);
      return;
   }

   DataVariant fields = mpFeatureClasses->getAttribute(oldClassName.toStdString());
   mpFeatureClasses->removeAttribute(oldClassName.toStdString());
   mpFeatureClasses->setAttribute(newClassName.toStdString(), fields);

   pItem->setData(Qt::UserRole, QVariant(newClassName));
}

void FeatureClassDlg::addField()
{
   // Assign a unique default name for the field
   int fieldNumber = 1;
   QString fieldName = "Field_" + QString::number(fieldNumber);
   while (mpFieldTree->findItems(fieldName, Qt::MatchExactly, 0).empty() == false)
   {
      fieldName = "Field_" + QString::number(++fieldNumber);
   }

   // Add the field to the member DynamicObject
   DynamicObject* pClass = getCurrentFeatureClass();
   if (pClass != NULL)
   {
      pClass->setAttribute(fieldName.toStdString(), std::string());
   }

   // Create a new tree widget item and select it
   QTreeWidgetItem* pFieldItem = new QTreeWidgetItem();
   pFieldItem->setData(0, Qt::UserRole, QVariant(fieldName));
   pFieldItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
   pFieldItem->setText(0, fieldName);
   pFieldItem->setText(1, "string");

   mpFieldTree->addTopLevelItem(pFieldItem);
   mpFieldTree->setCurrentItem(pFieldItem);
}

void FeatureClassDlg::removeField()
{
   QList<QTreeWidgetItem*> items = mpFieldTree->selectedItems();
   for (int i = 0; i < items.count(); ++i)
   {
      QTreeWidgetItem* pItem = items[i];
      if (pItem != NULL)
      {
         QString fieldName = pItem->text(0);

         // Remove the field from the member DynamicObject
         DynamicObject* pClass = getCurrentFeatureClass();
         if (pClass != NULL)
         {
            pClass->removeAttribute(fieldName.toStdString());
         }

         // Remove the item from the tree widget
         delete pItem;
      }
   }
}

void FeatureClassDlg::setFieldData(QTreeWidgetItem* pItem, int column)
{
   if (pItem == NULL)
   {
      return;
   }

   DynamicObject* pClass = getCurrentFeatureClass();
   if (pClass == NULL)
   {
      return;
   }

   QString fieldName = pItem->text(0);
   if (column == 0)     // Name
   {
      QString oldFieldName = pItem->data(column, Qt::UserRole).toString();
      if (fieldName == oldFieldName)
      {
         return;
      }

      if (mpFieldTree->findItems(fieldName, Qt::MatchExactly, column).count() > 1)
      {
         QMessageBox::warning(this, windowTitle(), "Another field exists with the same name.  "
            "Please choose a unique name for the field.");
         pItem->setText(column, oldFieldName);
         return;
      }

      DataVariant field = pClass->getAttribute(oldFieldName.toStdString());
      pClass->removeAttribute(oldFieldName.toStdString());
      pClass->setAttribute(fieldName.toStdString(), field);

      pItem->setData(column, Qt::UserRole, QVariant(fieldName));
   }
   else                 // Type or Value
   {
      QString fieldType = pItem->text(1);
      QString fieldValue = pItem->text(2);

      if (column == 1)  // Type
      {
         // If the type changed, ensure that the value is valid with the new type and reset the value if necessary
         QString validateString = fieldValue;
         int pos = 0;

         if (fieldType == "int")
         {
            QIntValidator validator(this);
            if (validator.validate(validateString, pos) != QValidator::Acceptable)
            {
               pItem->setText(2, "0");
               return;     // Return since this method will be called again as a result to changing the value text
            }
         }
         else if (fieldType == "double")
         {
            QDoubleValidator validator(this);
            if (validator.validate(fieldValue, pos) != QValidator::Acceptable)
            {
               pItem->setText(2, "0.0");
               return;     // Return since this method will be called again as a result to changing the value text
            }
         }
      }

      DataVariant field;
      if (fieldType == "int")
      {
         int intValue = fieldValue.toInt();
         field = DataVariant(intValue);
      }
      else if (fieldType == "double")
      {
         double doubleValue = fieldValue.toDouble();
         field = DataVariant(doubleValue);
      }
      else if (fieldType == "string")
      {
         std::string stringValue = fieldValue.toStdString();
         field = DataVariant(stringValue);
      }

      pClass->setAttribute(fieldName.toStdString(), field);
   }
}

void FeatureClassDlg::updateFields()
{
   mpFieldTree->clear();

   DynamicObject* pClass = getCurrentFeatureClass();
   if (pClass == NULL)
   {
      return;
   }

   std::vector<std::string> fieldNames;
   pClass->getAttributeNames(fieldNames);
   for (std::vector<std::string>::const_iterator iter = fieldNames.begin(); iter != fieldNames.end(); ++iter)
   {
      std::string fieldName = *iter;
      if (fieldName.empty() == false)
      {
         const DataVariant& field = pClass->getAttribute(fieldName);
         QString fieldType = QString::fromStdString(field.getTypeName());
         QString fieldValue = QString::fromStdString(field.toDisplayString());

         QTreeWidgetItem* pFieldItem = new QTreeWidgetItem(mpFieldTree);
         pFieldItem->setData(0, Qt::UserRole, QVariant(QString::fromStdString(fieldName)));
         pFieldItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
         pFieldItem->setText(0, QString::fromStdString(fieldName));
         pFieldItem->setText(1, fieldType);
         pFieldItem->setText(2, fieldValue);
      }
   }
}

DynamicObject* FeatureClassDlg::getCurrentFeatureClass()
{
   QList<QListWidgetItem*> items = mpClassList->selectedItems();
   if (items.empty() == true)
   {
      return NULL;
   }

   QListWidgetItem* pFeatureItem = items.front();
   if (pFeatureItem == NULL)
   {
      return NULL;
   }

   std::string className = pFeatureItem->text().toStdString();
   DataVariant& featureClass = mpFeatureClasses->getAttribute(className);

   return featureClass.getPointerToValue<DynamicObject>();
}

FieldNameDelegate::FieldNameDelegate(QObject* pParent) :
   QStyledItemDelegate(pParent)
{}

FieldNameDelegate::~FieldNameDelegate()
{}

QWidget* FieldNameDelegate::createEditor(QWidget* pParent, const QStyleOptionViewItem& option,
                                         const QModelIndex& index) const
{
   if (index.isValid() == false)
   {
      return NULL;
   }

   QLineEdit* pNameEdit = new QLineEdit(pParent);
   pNameEdit->setFrame(false);
   pNameEdit->setValidator(new QRegExpValidator(QRegExp("[\\d\\w]{1,11}"), pNameEdit));

   return pNameEdit;
}

void FieldNameDelegate::setEditorData(QWidget* pEditor, const QModelIndex& index) const
{
   QLineEdit* pNameEdit = dynamic_cast<QLineEdit*>(pEditor);
   if (pNameEdit == NULL)
   {
      return;
   }

   QString name = index.model()->data(index, Qt::EditRole).toString();
   pNameEdit->setText(name);
}

void FieldNameDelegate::setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const
{
   if (pModel == NULL)
   {
      return;
   }

   QLineEdit* pNameEdit = dynamic_cast<QLineEdit*>(pEditor);
   if (pNameEdit == NULL)
   {
      return;
   }

   QString name = pNameEdit->text();
   pModel->setData(index, QVariant(name), Qt::EditRole);
}

void FieldNameDelegate::updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const
{
   if (pEditor != NULL)
   {
      pEditor->setGeometry(option.rect);
   }
}

FieldTypeDelegate::FieldTypeDelegate(QObject* pParent) :
   QStyledItemDelegate(pParent)
{}

FieldTypeDelegate::~FieldTypeDelegate()
{}

QWidget* FieldTypeDelegate::createEditor(QWidget* pParent, const QStyleOptionViewItem& option,
                                         const QModelIndex& index) const
{
   if (index.isValid() == false)
   {
      return NULL;
   }

   QComboBox* pTypeCombo = new QComboBox(pParent);
   pTypeCombo->setEditable(false);
   pTypeCombo->addItem("int");
   pTypeCombo->addItem("double");
   pTypeCombo->addItem("string");

   return pTypeCombo;
}

void FieldTypeDelegate::setEditorData(QWidget* pEditor, const QModelIndex& index) const
{
   QComboBox* pTypeCombo = dynamic_cast<QComboBox*>(pEditor);
   if (pTypeCombo == NULL)
   {
      return;
   }

   QString typeText = index.model()->data(index, Qt::EditRole).toString();
   int typeIndex = pTypeCombo->findText(typeText);
   pTypeCombo->setCurrentIndex(typeIndex);
}

void FieldTypeDelegate::setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const
{
   if (pModel == NULL)
   {
      return;
   }

   QComboBox* pTypeCombo = dynamic_cast<QComboBox*>(pEditor);
   if (pTypeCombo == NULL)
   {
      return;
   }

   QString typeText = pTypeCombo->currentText();
   pModel->setData(index, QVariant(typeText), Qt::EditRole);
}

void FieldTypeDelegate::updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option,
                                             const QModelIndex& index) const
{
   if (pEditor != NULL)
   {
      pEditor->setGeometry(option.rect);
   }
}

FieldValueDelegate::FieldValueDelegate(QObject* pParent) :
   QStyledItemDelegate(pParent)
{}

FieldValueDelegate::~FieldValueDelegate()
{}

QWidget* FieldValueDelegate::createEditor(QWidget* pParent, const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
   if (index.isValid() == false)
   {
      return NULL;
   }

   QLineEdit* pValueEdit = new QLineEdit(pParent);
   pValueEdit->setFrame(false);

   QValidator* pValidator = NULL;

   QString type = index.sibling(index.row(), 1).data(Qt::DisplayRole).toString();
   if (type == "int")
   {
      pValidator = new QIntValidator(pValueEdit);
   }
   else if (type == "double")
   {
      pValidator = new QDoubleValidator(pValueEdit);
   }

   pValueEdit->setValidator(pValidator);

   return pValueEdit;
}

void FieldValueDelegate::setEditorData(QWidget* pEditor, const QModelIndex& index) const
{
   QLineEdit* pValueEdit = dynamic_cast<QLineEdit*>(pEditor);
   if (pValueEdit == NULL)
   {
      return;
   }

   QString valueText = index.model()->data(index, Qt::EditRole).toString();
   pValueEdit->setText(valueText);
}

void FieldValueDelegate::setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const
{
   if (pModel == NULL)
   {
      return;
   }

   QLineEdit* pValueEdit = dynamic_cast<QLineEdit*>(pEditor);
   if (pValueEdit == NULL)
   {
      return;
   }

   QString valueText = pValueEdit->text();
   pModel->setData(index, QVariant(valueText), Qt::EditRole);
}

void FieldValueDelegate::updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const
{
   if (pEditor != NULL)
   {
      pEditor->setGeometry(option.rect);
   }
}
