/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AddQueriesDlg.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "ColorType.h"
#include "ConfigurationSettings.h"
#include "CustomTreeWidget.h"
#include "DisplayQueryOptions.h"
#include "DisplaySelectionWidget.h"
#include "FeatureClass.h"
#include "Filename.h"
#include "FileResource.h"
#include "ObjectResource.h"
#include "QueryOptions.h"
#include "xmlwriter.h"
#include "xmlreader.h"

#include <QtCore/QStringList>
#include <QtGui/QComboBox>
#include <QtGui/QFileDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include <algorithm>
#include <memory>

DisplaySelectionWidget::DisplaySelectionWidget(QWidget* pParent) :
   QWidget(pParent),
   mpFeatureClass(NULL)
{
   QLabel* pQueriesLabel = new QLabel("Queries:", this);

   //make the combo boxes
   mpAttributeCombo = new QComboBox(this);
   QString toolTipStr("Use ':' for ranges");
   mpAttributeCombo->setToolTip(toolTipStr);
   VERIFYNR(connect(mpAttributeCombo, SIGNAL(currentIndexChanged(const QString&)), this,
      SLOT(fieldSelected(const QString&))));
   mpAttributeCombo->hide();

   mpMenu = new QMenu(this);
   VERIFYNR(connect(mpMenu, SIGNAL(aboutToShow()), this, SLOT(populateContextMenu())));

   mpEditButton = new QPushButton("Edit", this);
   mpEditButton->setMenu(mpMenu);

   //make the list
   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Field");
   columnNames.append("Value");

   mpTree = new CustomTreeWidget(this);

   mpTree->setColumnCount(columnNames.count());
   mpTree->setHeaderLabels(columnNames);
   mpTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpTree->setSortingEnabled(false);
   mpTree->setRootIsDecorated(false);
   mpTree->setFullCellColor(true);
   mpTree->setContextMenuPolicy(Qt::CustomContextMenu);

   createAllObjectsEntry();

   VERIFYNR(connect(mpTree, SIGNAL(cellTextChanged(QTreeWidgetItem*, int)), this,
      SLOT(textModified(QTreeWidgetItem*, int))));
   VERIFYNR(connect(mpTree, SIGNAL(cellCheckChanged(QTreeWidgetItem*, int)), this,
      SLOT(treeItemCheckChanged(QTreeWidgetItem*, int))));
   VERIFYNR(connect(mpTree, SIGNAL(itemSelectionChanged()), this, SLOT(querySelectionChanged())));
   VERIFYNR(connect(mpTree, SIGNAL(customContextMenuRequested(const QPoint&)), this,
      SLOT(displayContextMenu(const QPoint&))));

   QHeaderView* pHeader = mpTree->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(true);
      pHeader->setSortIndicatorShown(true);
   }

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pQueriesLabel, 0, 0);
   pLayout->addWidget(mpEditButton, 0, 1, Qt::AlignRight);
   pLayout->addWidget(mpTree, 1, 0, 1, 2);
   pLayout->setRowStretch(1, 10);
   pLayout->setColumnStretch(1, 10);
}

DisplaySelectionWidget::~DisplaySelectionWidget()
{}

void DisplaySelectionWidget::setDefaultQuery(const QueryOptions& query)
{
   mDefaultQuery = query;
}

const QueryOptions& DisplaySelectionWidget::getDefaultQuery()
{
   return mDefaultQuery;
}

void DisplaySelectionWidget::initializeFromQueries(const std::vector<DisplayQueryOptions>& queries)
{
   mpTree->clear();
   createAllObjectsEntry();
   bool bAttributeSet = false;
   for (unsigned int i = 0; i < queries.size(); i++)
   {
      bool bAdd = false;
      QTreeWidgetItem* pItem = new QTreeWidgetItem(mpTree);

      ColorType color = queries[i].getLineColor();
      QColor clrClass(color.mRed, color.mGreen, color.mBlue);
      QString query = QString::fromStdString(queries[i].getQueryString());

      QStringList queryStrList = query.split(',');
      QString queryAttribute;
      QString queryDisplay;
      QComboBox* pCombo = NULL;
      if (queryStrList.size() > 1)
      {
         queryAttribute = queryStrList[1];
         int attrIndex = mpAttributeCombo->findText(queryAttribute);
         if (queryStrList.size() == 9)
         {
            //this must be a toleranced float or double
            queryDisplay = queryStrList[8];
         }
         else if (queryStrList.size() == 7)
         {
            //this must be a ranged value comparison
            queryDisplay = queryStrList[3] + QString(":") + queryStrList[6];
         }
         else
         {
            //this must be a single value comparison
            queryDisplay = queryStrList[3];
         }
         if (attrIndex >= 0 && attrIndex < mpAttributeCombo->count())
         {
            pCombo = mpValueCombos[attrIndex];
         }
      }
      pItem->setText(1, queryAttribute);
      pItem->setText(2, queryDisplay);
      pItem->setText(0, QString::fromStdString(queries[i].getUniqueName()));
      pItem->setData(0, QTreeWidgetItem::UserType, QVariant(QString::fromStdString(queries[i].getUniqueName())));

      mpTree->setCellWidgetType(pItem, 0, CustomTreeWidget::LINE_EDIT);
      mpTree->setCellWidgetType(pItem, 1, CustomTreeWidget::COMBO_BOX);
      mpTree->setCellWidgetType(pItem, 2, CustomTreeWidget::COMBO_BOX);
      CustomTreeWidget::CheckState checked = CustomTreeWidget::CHECKED;
      if (queries[i].getQueryActive() == false)
      {
         checked = CustomTreeWidget::UNCHECKED;
      }
      mpTree->setCellCheckState(pItem, 0, checked);

      mpTree->setComboBox(pItem, 2, pCombo);
      mpTree->setComboBox(pItem, 1, mpAttributeCombo);

      mpTree->setFullCellEdit(pItem, 0, false);
      fieldSelected(queryAttribute);
   }
}

void DisplaySelectionWidget::addButtonPressed()
{
   std::string field = "";
   std::string value = "";

   DisplayQueryOptions* pOption = addNewQueryItem(field, value);

   if (pOption != NULL && mpFeatureClass != NULL)
   {
      mpFeatureClass->addDisplayQuery(pOption);
   }
}

DisplayQueryOptions* DisplaySelectionWidget::addNewQueryItem(std::string& field, std::string& value)
{
   DisplayQueryOptions* pOption = NULL;

   if (mpFeatureClass != NULL)
   {
      QTreeWidgetItem* pItem = new QTreeWidgetItem(mpTree);
      QString queryName;
      //make sure we have a unique name when adding
      while (queryName.isEmpty() == true)
      {
         bool bFound = false;
         QString tmpQueryName = QString("Query ") + QString::number(mpFeatureClass->getCurrentQueryIndex());
         for (int i = 0; i < mpTree->topLevelItemCount(); i++)
         {
            if (pItem->text(0) == tmpQueryName)
            {
               mpFeatureClass->incrementCurrentQueryIndex();
               bFound = true;
               break;
            }
         }
         if (bFound == false)
         {
            queryName = tmpQueryName;
         }
      }
      pItem->setText(0, queryName);
      pItem->setData(0, QTreeWidgetItem::UserType, QVariant(queryName));
      mpFeatureClass->incrementCurrentQueryIndex();
      mpTree->setCellWidgetType(pItem, 0, CustomTreeWidget::LINE_EDIT);
      mpTree->setCellWidgetType(pItem, 1, CustomTreeWidget::COMBO_BOX);
      mpTree->setCellWidgetType(pItem, 2, CustomTreeWidget::COMBO_BOX);

      mpTree->setCellCheckState(pItem, 0, CustomTreeWidget::CHECKED);
      QComboBox* pCombo = NULL;

      mpTree->setComboBox(pItem, 1, mpAttributeCombo);
      if (mpAttributeCombo->count() > 0)
      {
         if (!mpValueCombos.empty())
         {
            pCombo = mpValueCombos[0];
         }
         mpAttributeCombo->setCurrentIndex(0);
      }
      mpTree->setComboBox(pItem, 2, pCombo);
      mpTree->setFullCellEdit(pItem, 0, false);
      QString fieldStr = "";
      if (field.empty() == false)
      {
         fieldStr = QString::fromStdString(field);
      }
      else if (mpAttributeCombo->count() > 0)
      {
         fieldStr = mpAttributeCombo->currentText();
      }
      fieldSelected(fieldStr);
      pItem->setText(1, fieldStr);
      
      if (value.empty() == false)
      {
         QString valueStr(QString::fromStdString(value));
         pItem->setText(2, valueStr);
      }
      pOption = createQueryOption(pItem);
   }
   return pOption;
}

void DisplaySelectionWidget::removeButtonPressed()
{
   std::vector<DisplayQueryOptions*> optionsVecModify;
   std::vector<DisplayQueryOptions*> optionsVecDelete;
   QList<QTreeWidgetItem*> items = mpTree->selectedItems();
   for (int i = 0 ; i < items.size(); i++)
   {
      DisplayQueryOptions* pOption = createQueryOption(items.at(i));
      if (pOption != NULL)
      {
         delete items.at(i);
         optionsVecDelete.push_back(pOption);
      }
   }
   if (mpFeatureClass != NULL)
   {
      mpFeatureClass->removeDisplayQuery(optionsVecDelete);
   }
   for (int i = 0 ; i < mpTree->topLevelItemCount(); i++)
   {
      DisplayQueryOptions* pOption = createQueryOption(mpTree->topLevelItem(i));
      if (pOption != NULL)
      {
         optionsVecModify.push_back(pOption);
      }
   }
   if (mpFeatureClass != NULL)
   {
      mpFeatureClass->modifyDisplayQuery(optionsVecModify, false);
   }
}

void DisplaySelectionWidget::clearQueryElements()
{
   mpTree->selectAll();
   removeButtonPressed();
}

void DisplaySelectionWidget::fieldSelected(const QString& text)
{
   int currentIndex = mpAttributeCombo->currentIndex();
   if (currentIndex >= 0 && currentIndex < static_cast<int>(mFieldTypes.size()))
   {
      //populate the fields values list
      std::string field = text.toStdString();
      if (mFieldValues[currentIndex].size() == 0)
      {
         std::vector<std::string> values;
         if (mpFeatureClass != NULL)
         {
            mpFeatureClass->getFieldValues(field, values);
            populateFieldValues(field, values);
         }
      }
      else
      {
         populateFieldValueCombos(field);
      }
   }
}

void DisplaySelectionWidget::loadRecentQuery(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   QString queryName = pAction->text();
   if (queryName.isEmpty() == false)
   {
      clearQueryElements();
      if (loadQuery(queryName) == true)
      {
         addToRecentlyUsedList(queryName.toStdString());
      }
   }
}

void DisplaySelectionWidget::textModified(QTreeWidgetItem* pItem, int iColumn)
{
   std::vector<DisplayQueryOptions*> pOptions;
   QList<QTreeWidgetItem*> list = mpTree->selectedItems();
   ColorType color;
   if (iColumn == 0)
   {
      QString newName = pItem->text(0);
      QString oldName = pItem->data(0, QTreeWidgetItem::UserType).toString();
      //check if new query name is unique
      bool bUnique = true;
      for (int i = 0; i < mpTree->topLevelItemCount(); i++)
      {
         QTreeWidgetItem* pTraverseItem = mpTree->topLevelItem(i);
         if (pTraverseItem != NULL && pTraverseItem != pItem)
         {
            if (pTraverseItem->text(0) == newName)
            {
               bUnique = false;
               pItem->setText(0, oldName);
               break;
            }
         }
      }
      //change the query name
      if (bUnique == true && mpFeatureClass != NULL)
      {
         pItem->setData(0, QTreeWidgetItem::UserType, QVariant(newName));
         std::string oName = oldName.toStdString();
         std::string nName = newName.toStdString();
         mpFeatureClass->renameDisplayQuery(mDefaultQuery.getQueryName(), oName, nName);
      }
   }
   else
   {
      //update the values list
      if (iColumn == 1)
      {
         int currentIndex = mpAttributeCombo->currentIndex();
         if (currentIndex >= 0 && currentIndex < static_cast<int>(mFieldTypes.size()))
         {
            mpTree->setComboBox(pItem, 2, mpValueCombos[currentIndex]);
         }
      }
      //change the query value
      for (int i = 0; i < list.count(); i++)
      {
         DisplayQueryOptions* pOption = createQueryOption(list.at(i));
         if (pOption != NULL)
         {
            pOptions.push_back(pOption);
         }
      }
      if (mpFeatureClass != NULL)
      {
         mpFeatureClass->modifyDisplayQuery(pOptions, false);
      }
   }
}

void DisplaySelectionWidget::populateAttributeList(const std::vector<std::string>& names,
                                                   const std::vector<std::string>& types)
{
   mpAttributeCombo->clear();
   mpValueCombos.clear();
   mFieldTypes.clear();
   for (unsigned int i = 0; i < names.size(); i++)
   {
      mFieldTypes.push_back(types[i]);
      mFieldValues.push_back(std::vector<std::string>());
      mpValueCombos.push_back(new QComboBox(this));
      mpValueCombos[i]->setEditable(true);
      mpValueCombos[i]->setHidden(true);
      mpAttributeCombo->addItem(QString::fromStdString(names[i]));
   }
}

void DisplaySelectionWidget::querySelectionChanged()
{
   QList<QTreeWidgetItem*> list = mpTree->selectedItems();
   std::vector<DisplayQueryOptions*> pOptions;
   if (list.count() == 0)
   {
      QTreeWidgetItem* pItem = mpTree->topLevelItem(0);
      mpTree->setItemSelected(pItem, true);
   }
   else
   {
      for (int i = 0; i < list.count(); i++)
      {
         DisplayQueryOptions* pOption = createQueryOption(list.at(i));
         if (pOption != NULL)
         {
            pOptions.push_back(pOption);
         }
      }

      emit selectDisplayQuery(pOptions);
   }
}

DisplayQueryOptions* DisplaySelectionWidget::createQueryOption(QTreeWidgetItem* pItem)
{
   DisplayQueryOptions* pOption = NULL;

   std::string uniqueName = pItem->text(0).toStdString();
   if (uniqueName != "All Objects")
   {
      pOption = new DisplayQueryOptions(mDefaultQuery);
      pOption->setUniqueName(pItem->text(0).toStdString());
      QString itemText = pItem->text(2);
      QString attributeText = pItem->text(1);
      pOption->setQueryString(createQueryString(attributeText, itemText).toStdString());
      CustomTreeWidget::CheckState checkState = mpTree->getCellCheckState(pItem, 0);
      pOption->setQueryActive(checkState == CustomTreeWidget::CHECKED);
      pOption->setOrder(mpTree->indexOfTopLevelItem(pItem));
   }
   return pOption;
}

QString DisplaySelectionWidget::createQueryString(QString& field, QString& value)
{
   QString query;
   QStringList list = value.split(QChar(':'), QString::SkipEmptyParts);
   if (list.empty() == true)
   {
      return QString();
   }

   int index = mpAttributeCombo->findText(field);
   if (index >= 0 && index < mpAttributeCombo->count())
   {
      if (mFieldTypes[index] != "String" && list.size() > 1)
      {
         query += "and," + field + ",>=," + list[0] + "," + field + ",<=," + list[1];
      }
      else if ((mFieldTypes[index] == "Float" || mFieldTypes[index] == "Double") && !list.empty())
      {
         //if we are dealing with a match for a float or a double, add a 
         //tolerance based on significant digits
         double digits = 0;
         QStringList decimalList = list[0].split(QChar('.'));
         if (decimalList.size() > 1)
         {
            digits = static_cast<double>(decimalList[1].size());
         }
         double tolerance = 5 / pow(10, digits + 1);
         double value1 = list[0].toDouble() - tolerance;
         double value2 = list[0].toDouble() + tolerance;
         query += "and," + field + ",>=," + QString::number(value1) + "," + field + ",<=," + 
            QString::number(value2) +",tolerance," + list[0];
      }
      else
      {
         query += "and," + field + ",=," + value;
      }
   }
   return query;
}

void DisplaySelectionWidget::treeItemCheckChanged(QTreeWidgetItem* pItem, int iColumn)
{
   std::vector<DisplayQueryOptions*> options;
   QList<QTreeWidgetItem*> items = mpTree->selectedItems();
   for (int i = 0 ; i < items.size(); i++)
   {
      DisplayQueryOptions* pOption = createQueryOption(items.at(i));
      if (pOption != NULL)
      {
         options.push_back(pOption);
         if (pItem != items.at(i))
         {
            mpTree->setCellCheckState(items.at(i), 0, mpTree->getCellCheckState(pItem, 0));
         }
      }
   }
   if (mpFeatureClass != NULL)
   {
      mpFeatureClass->modifyDisplayQuery(options, false);
   }
}

void DisplaySelectionWidget::openFilePressed()
{
   Service<ConfigurationSettings> pSettings;
   QString defaultPath;
   const Filename* pImportPath = pSettings->getSettingImportPath();
   if (pImportPath != NULL)
   {
      defaultPath = QString::fromStdString(pImportPath->getFullPathAndName());
   }
   QString strFilename = QFileDialog::getOpenFileName(this, "Get Query Group Filename", defaultPath, "All Files (*.*)");
   if (strFilename.isEmpty() == false)
   {
      clearQueryElements();
      if (loadQuery(strFilename) == true)
      {
         addToRecentlyUsedList(strFilename.toStdString());
      }
   }
}

void DisplaySelectionWidget::saveFilePressed()
{
   Service<ConfigurationSettings> pSettings;
   QString defaultPath;
   const Filename* pExportPath = pSettings->getSettingExportPath();
   if (pExportPath != NULL)
   {
      defaultPath = QString::fromStdString(pExportPath->getFullPathAndName());
   }
   QString strFilename = QFileDialog::getSaveFileName(this, "Set Query Group Filename", defaultPath, "All Files (*.*)");

   std::string fileName = strFilename.toStdString();
   if (strFilename.isEmpty() == false)
   {
      saveQuery(strFilename);
      //write the location of the file to the user configurations
      addToRecentlyUsedList(fileName);
   }
}

void DisplaySelectionWidget::addQueriesButtonPressed()
{
   std::vector<std::string> fields;
   for (int i = 0; i < mpAttributeCombo->count(); i++)
   {
      if (mFieldTypes[i] != "Float" && mFieldTypes[i] != "Double")
      {
         fields.push_back(mpAttributeCombo->itemText(i).toStdString());
      }
   }
   AddQueriesDlg dlg(fields, this);
   if (dlg.exec() == QDialog::Accepted)
   {
      std::string attribute = dlg.getAttribute();
      std::string queryName = mDefaultQuery.getQueryName();
      if (mpFeatureClass != NULL)
      {
         mpFeatureClass->populateDisplayQueries(queryName, attribute, dlg.isLineColorUnique(), dlg.isFillColorUnique());
         //now that the queries are in the feature class,
         //add them to the display
         const std::vector<DisplayQueryOptions> queries = mpFeatureClass->getDisplayQueryOptions(queryName);
         initializeFromQueries(queries);
      }
   }
}

void DisplaySelectionWidget::moveItems(int index)
{
   std::vector<DisplayQueryOptions*> optionsVec;
   QList<QTreeWidgetItem*> items = mpTree->selectedItems();
   QList<QTreeWidgetItem*> itemsToMove;
   for (int i = 0; i < items.size(); i++)
   {
      std::auto_ptr<DisplayQueryOptions> pOption(createQueryOption(items.at(i)));
      if (pOption.get() != NULL)
      {
         QTreeWidgetItem* pItem = mpTree->takeTopLevelItem(mpTree->indexOfTopLevelItem(items.at(i)));
         itemsToMove.push_back(pItem);
      }
   }
   mpTree->insertTopLevelItems(index, itemsToMove);
   for (int i = 0; i < mpTree->topLevelItemCount(); i++)
   {
      DisplayQueryOptions* pOption = createQueryOption(mpTree->topLevelItem(i));
      if (pOption != NULL)
      {
         optionsVec.push_back(pOption);
      }
   }
   if (mpFeatureClass != NULL)
   {
      mpFeatureClass->modifyDisplayQuery(optionsVec, false);
   }
}

void DisplaySelectionWidget::moveUpButtonPressed()
{
   QList<QTreeWidgetItem*> items = mpTree->selectedItems();
   if (!items.empty())
   {
      int insertIndex = mpTree->indexOfTopLevelItem(items.front());
      if (insertIndex > 1)
      {
         insertIndex -= 1;
         moveItems(insertIndex);
      }
   }
}

void DisplaySelectionWidget::moveDownButtonPressed()
{
   QList<QTreeWidgetItem*> items = mpTree->selectedItems();
   if (!items.empty())
   {
      int insertIndex = mpTree->indexOfTopLevelItem(items.front());
      if (insertIndex < mpTree->topLevelItemCount() - 1)
      {
         insertIndex += 1;
         moveItems(insertIndex);
      }
   }
}

void DisplaySelectionWidget::moveTopButtonPressed()
{
   moveItems(1);
}

void DisplaySelectionWidget::moveBottomButtonPressed()
{
   moveItems(mpTree->topLevelItemCount() - 1);
}

void DisplaySelectionWidget::checkAllButtonPressed()
{
   int count = mpTree->topLevelItemCount();
   std::vector<DisplayQueryOptions*> options;
   for (int i = 0; i < count; i++)
   {
      DisplayQueryOptions* pOption = createQueryOption(mpTree->topLevelItem(i));
      if (pOption != NULL)
      {
         mpTree->setCellCheckState(mpTree->topLevelItem(i), 0, CustomTreeWidget::CHECKED);
         pOption->setQueryActive(true);
         options.push_back(pOption);
      }
   }
   if (mpFeatureClass != NULL)
   {
      mpFeatureClass->modifyDisplayQuery(options, false);
   }
}

void DisplaySelectionWidget::unCheckAllButtonPressed()
{
   int count = mpTree->topLevelItemCount();
   std::vector<DisplayQueryOptions*> options;
   for (int i = 0; i < count; i++)
   {
      DisplayQueryOptions* pOption = createQueryOption(mpTree->topLevelItem(i));
      if (pOption != NULL)
      {
         mpTree->setCellCheckState(mpTree->topLevelItem(i), 0, CustomTreeWidget::UNCHECKED);
         pOption->setQueryActive(false);
         options.push_back(pOption);
      }
   }
   if (mpFeatureClass != NULL)
   {
      mpFeatureClass->modifyDisplayQuery(options, false);
   }
}

void DisplaySelectionWidget::createAllObjectsEntry()
{
   QTreeWidgetItem* pItem = new QTreeWidgetItem(mpTree);
   pItem->setText(0, QString("All Objects"));
   mpTree->setItemSelected(pItem, true);
}

void DisplaySelectionWidget::displayContextMenu(const QPoint& menuPoint)
{
   mpMenu->exec(mpTree->viewport()->mapToGlobal(menuPoint));
}

void DisplaySelectionWidget::populateContextMenu()
{
   mpMenu->clear();
   mpMenu->addAction(QIcon(":/icons/Open"), "&Open", this, SLOT(openFilePressed()));
   mpMenu->addAction(QIcon(":/icons/Save"), "&Save", this, SLOT(saveFilePressed()));
   mpMenu->addSeparator();

   mpMenu->addAction("&Add Query", this, SLOT(addButtonPressed()));
   //check if we should have an add Queries in the list
   bool bFieldsPopulated = false;
   for (unsigned int i = 0; i < mFieldTypes.size(); i++)
   {
      if (mFieldValues[i].size() > 0)
      {
         bFieldsPopulated = true;
         break;
      }
   }
   if (bFieldsPopulated)
   {
      mpMenu->addAction("Add Queries...", this, SLOT(addQueriesButtonPressed()));
   }

   std::vector<std::string> queries;
   getRecentlyUsedList(queries);
   if (queries.empty() == false)
   {
      QMenu* pQueriesMenu = mpMenu->addMenu("Recent Queries");
      for (std::vector<std::string>::iterator iter = queries.begin(); iter != queries.end(); ++iter)
      {
         QString queryName = QString::fromStdString(*iter);
         VERIFYNRV(queryName.isEmpty() == false);
         pQueriesMenu->addAction(queryName);
      }

      VERIFYNR(connect(pQueriesMenu, SIGNAL(triggered(QAction*)), this, SLOT(loadRecentQuery(QAction*))));
   }

   int numSelected = mpTree->selectedItems().count();
   if (numSelected > 0)
   {
      mpMenu->addAction(QIcon(":/icons/Delete"), "&Remove", this, SLOT(removeButtonPressed()));
   }
   mpMenu->addAction("&Clear", this, SLOT(clearQueryElements()));
   mpMenu->addSeparator();
   if (numSelected > 0)
   {
      mpMenu->addAction(QIcon(":/icons/Increase"), "Move &Up", this, SLOT(moveUpButtonPressed()));
      mpMenu->addAction(QIcon(":/icons/Decrease"), "Move &Down", this, SLOT(moveDownButtonPressed()));
      mpMenu->addAction("Move to &Top", this, SLOT(moveTopButtonPressed()));
      mpMenu->addAction("Move to &Bottom", this, SLOT(moveBottomButtonPressed()));
      mpMenu->addSeparator();
   }
   mpMenu->addAction("Check All", this, SLOT(checkAllButtonPressed()));
   mpMenu->addAction("Uncheck All", this, SLOT(unCheckAllButtonPressed()));
}

void DisplaySelectionWidget::populateFieldValues(const std::string& field, const std::vector<std::string>& values)
{
   int fieldIndex = mpAttributeCombo->findText(QString::fromStdString(field));
   if (fieldIndex >= 0 && fieldIndex < static_cast<int>(mFieldTypes.size()))
   {
      mFieldValues[fieldIndex].clear();
      for (unsigned int i = 0; i < values.size(); i++)
      {
         mFieldValues[fieldIndex].push_back(values[i]);
      }
      int currentIndex = mpAttributeCombo->currentIndex();
      if (currentIndex == fieldIndex)
      {
         populateFieldValueCombos(field);
      }
   }
}

void DisplaySelectionWidget::populateFieldValueCombos(std::string field)
{
   int fieldIndex = mpAttributeCombo->findText(QString::fromStdString(field));
   if (fieldIndex >= 0 && fieldIndex < static_cast<int>(mFieldTypes.size()))
   {
      mpValueCombos[fieldIndex]->clear();
      if (fieldIndex < static_cast<int>(mpValueCombos.size()))
      {
         for (unsigned int i = 0 ; i < mFieldValues[fieldIndex].size(); i++)
         {
            mpValueCombos[fieldIndex]->addItem(QString::fromStdString(mFieldValues[fieldIndex].at(i)));
         }
      }
   }
}

void DisplaySelectionWidget::getRecentlyUsedList(std::vector<std::string>& list)
{
   Service<ConfigurationSettings> pSettings;
   DataVariant queryFiles = pSettings->getSetting("DisplayQueryOptionsList");
   if (queryFiles.isValid() && queryFiles.getTypeName() == "vector<string>")
   {
      std::vector<std::string>* pStrVec = queryFiles.getPointerToValue<std::vector<std::string> >();
      if (pStrVec != NULL)
      {
         for (unsigned int i = 0; i < pStrVec->size(); i++)
         {
            if (i < pStrVec->size())
            {
               list.push_back(pStrVec->at(i));
            }
         }
      }
   }
}

void DisplaySelectionWidget::addToRecentlyUsedList(std::string filename)
{
   Service<ConfigurationSettings> pSettings;
   std::vector<std::string> queryFilesVector;
   queryFilesVector.push_back(filename);
   std::vector<std::string> origList;
   getRecentlyUsedList(origList);
   for (unsigned int i = 0; i < 4; i++)
   {
      if (i < origList.size() && origList[i] != filename)
      {
         queryFilesVector.push_back(origList[i]);
      }
   }
   pSettings->setSetting("DisplayQueryOptionsList", queryFilesVector);
}

bool DisplaySelectionWidget::loadQuery(const QString& fileLocation)
{
   if (mpFeatureClass != NULL)
   {
      FactoryResource<DynamicObject> pDynObj;
      Service<MessageLogMgr> pMsgLog;
      MessageLog* pLog = pMsgLog->getLog();

      XmlReader xml(pLog, false);
      FactoryResource<Filename> pFilename;
      pFilename->setFullPathAndName(fileLocation.toStdString());

      //read the document
      bool bSuccess = false;

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDocument = xml.parse(pFilename.get(), "QueryOptionsList");
      if (pDocument != NULL)
      {
         XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pRootElement = pDocument->getDocumentElement();
         if (pRootElement != NULL)
         {
            unsigned int version = atoi(A(pRootElement->getAttribute(X("version"))));
            if (pRootElement->getFirstElementChild() != NULL)
            {
               bSuccess = pDynObj->fromXml(pRootElement->getFirstElementChild(), version);
            }
         }
      }
      if (bSuccess)
      {
         //we have successfully read in the file, now add the queries
         //to the feature class
         unsigned int numAttributes = pDynObj->getNumAttributes();
         std::vector<std::string> names;
         pDynObj->getAttributeNames(names);
         std::string queryName = mDefaultQuery.getQueryName();
         for (unsigned int i = 0; i < numAttributes; i++)
         {
            DisplayQueryOptions* pOption = new DisplayQueryOptions();
            DataVariant var = pDynObj->getAttribute(names[i]);
            pOption->fromDynamicObject(dynamic_cast<DynamicObject*>(var.getPointerToValue<DynamicObject>()));
            //now we need to override the saved query name to be the current query name
            pOption->setQueryName(queryName);
            mpFeatureClass->addDisplayQuery(pOption);
         }
         //now that the queries are in the feature class,
         //add them to the display
         const std::vector<DisplayQueryOptions> queries = mpFeatureClass->getDisplayQueryOptions(queryName);
         initializeFromQueries(queries);

         return true;
      }
      else
      {
         QMessageBox::critical(this, APP_NAME, "The selected query file could not load properly.");

         // Remove the bad file from the configuration settings
         std::vector<std::string> queries;
         getRecentlyUsedList(queries);

         std::vector<std::string>::iterator iter = std::find(queries.begin(), queries.end(),
            fileLocation.toStdString());
         if (iter != queries.end())
         {
            queries.erase(iter);

            Service<ConfigurationSettings> pSettings;
            pSettings->setSetting("DisplayQueryOptionsList", queries);
         }
      }
   }

   return false;
}

void DisplaySelectionWidget::saveQuery(const QString& fileLocation) const
{
   //retreive the information to save
   std::string queryName = mDefaultQuery.getQueryName();
   VERIFYNRV(mpFeatureClass != NULL);
   const std::vector<DisplayQueryOptions> options = mpFeatureClass->getDisplayQueryOptions(queryName);
   FactoryResource<DynamicObject> pDynObj;
   VERIFYNRV(pDynObj.get() != NULL);
   for (unsigned int i = 0; i < options.size(); i++)
   {
      //write to the file
      DynamicObject* pObject = options[i].toDynamicObject();
      pDynObj->setAttribute(options[i].getUniqueName(), *pObject);
   }
   Service<MessageLogMgr> pLogMgr;
   XMLWriter writer("QueryOptionsList", pLogMgr->getLog());

   bool success = pDynObj->toXml(&writer);
   if (!success)
   {
      return;
   }

   FileResource saveFile(fileLocation.toStdString().c_str(), "wb");
   if (saveFile.get() == NULL)
   {
      return;
   }

   writer.writeToFile(saveFile);
}

void DisplaySelectionWidget::setFeatureClass(FeatureClass* pFeatureClass)
{
   mpFeatureClass = pFeatureClass;
}