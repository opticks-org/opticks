/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QRegExp>
#include <QtGui/QBitmap>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>

#include "BatchFileset.h"
#include "CustomTreeWidget.h"
#include "FileBrowser.h"
#include "FilesetWidget.h"
#include "IconImages.h"

#include <string>
using namespace std;

FilesetWidget::FilesetWidget(QWidget* parent) :
   QWidget(parent)
{
   // Directory
   QLabel* pDirLabel = new QLabel("Directory:", this);

   QFont dirFont = pDirLabel->font();
   dirFont.setBold(true);
   pDirLabel->setFont(dirFont);

   mpDirectoryEdit = new QLineEdit(this);

   QPixmap pixOpen(IconImages::OpenIcon);
   pixOpen.setMask(pixOpen.createHeuristicMask());
   QIcon icnBrowse(pixOpen);

   mpBrowseButton = new QPushButton(icnBrowse, QString(), this);
   mpBrowseButton->setFixedWidth(27);

   QHBoxLayout* pDirLayout = new QHBoxLayout();
   pDirLayout->setMargin(0);
   pDirLayout->setSpacing(5);
   pDirLayout->addWidget(pDirLabel);
   pDirLayout->addWidget(mpDirectoryEdit, 10);
   pDirLayout->addWidget(mpBrowseButton);

   // Criteria
   mpCriteriaTree = new CustomTreeWidget(this);

   QStringList columnNames;
   columnNames.append("Search Criteria");
   columnNames.append("Inclusion");

   mpCriteriaTree->setColumnCount(2);
   mpCriteriaTree->setHeaderLabels(columnNames);
   mpCriteriaTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpCriteriaTree->setRootIsDecorated(false);
   mpCriteriaTree->setSortingEnabled(true);
   mpCriteriaTree->setGridlinesShown(Qt::Horizontal | Qt::Vertical, true);

   QHeaderView* pHeader = mpCriteriaTree->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->resizeSection(0, 125);
      pHeader->setSortIndicatorShown(true);
   }

   mpFileBrowser = new FileBrowser(mpCriteriaTree);
   mpFileBrowser->setBrowseCaption("Select File");
   mpFileBrowser->hide();

   mpInclusionCombo = new QComboBox(mpCriteriaTree);
   mpInclusionCombo->setEditable(false);
   mpInclusionCombo->addItem("exclude");
   mpInclusionCombo->addItem("include");
   mpInclusionCombo->hide();

   // Criteria buttons
   mpNewButton = new QPushButton("&New", this);
   mpAddButton = new QPushButton("&Add Files...", this);
   mpDeleteButton = new QPushButton("&Delete", this);

   QVBoxLayout* pButtonLayout = new QVBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addWidget(mpNewButton);
   pButtonLayout->addWidget(mpAddButton);
   pButtonLayout->addWidget(mpDeleteButton);
   pButtonLayout->addStretch();

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(10);
   pGrid->addLayout(pDirLayout, 0, 0, 1, 2);
   pGrid->addWidget(mpCriteriaTree, 1, 0);
   pGrid->addLayout(pButtonLayout, 1, 1);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(0, 10);

   // Connections
   connect(mpDirectoryEdit, SIGNAL(textChanged(const QString&)), mpCriteriaTree,
      SLOT(setBrowseDir(const QString&)));
   connect(mpBrowseButton, SIGNAL(clicked()), this, SLOT(browseForDirectory()));
   connect(mpCriteriaTree, SIGNAL(cellTextChanged(QTreeWidgetItem*, int)), this,
      SLOT(validateFile(QTreeWidgetItem*, int)));
   connect(mpNewButton, SIGNAL(clicked()), this, SLOT(addSearchCriterion()));
   connect(mpAddButton, SIGNAL(clicked()), this, SLOT(addFiles()));
   connect(mpDeleteButton, SIGNAL(clicked()), this, SLOT(deleteSelectedCriteria()));
   connect(this, SIGNAL(modified()), this, SLOT(updateFileset()));
}

FilesetWidget::~FilesetWidget()
{
}

void FilesetWidget::setActiveFileset(BatchFileset* pFileset)
{
   disconnect(mpDirectoryEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(modified()));
   disconnect(mpCriteriaTree, SIGNAL(cellTextChanged(QTreeWidgetItem*, int)), this, SIGNAL(modified()));

   bool bEnable = false;
   QString strDirectory;
   mpCriteriaTree->clear();

   if (pFileset != NULL)
   {
      string directory = pFileset->getDirectory();
      if (directory.empty() == false)
      {
         strDirectory = QString::fromStdString(directory);
      }

      multimap<string, string> criteria = pFileset->getFilesetRequirements();

      multimap<string, string>::iterator iter;
      for (iter = criteria.begin(); iter != criteria.end(); iter++)
      {
         QString strType;
         QString strPattern;

         string type = iter->first;
         string pattern = iter->second;

         if (type.empty() == false)
         {
            strType = QString::fromStdString(type);
         }

         if (pattern.empty() == false)
         {
            strPattern = QString::fromStdString(pattern);
         }

         QTreeWidgetItem* pItem = new QTreeWidgetItem(mpCriteriaTree);
         if (pItem != NULL)
         {
            pItem->setText(0, strPattern);
            pItem->setText(1, strType);

            mpCriteriaTree->setCellWidgetType(pItem, 0, CustomTreeWidget::BROWSE_FILE_EDIT);
            mpCriteriaTree->setFileBrowser(pItem, 0, mpFileBrowser);
            mpCriteriaTree->setCellWidgetType(pItem, 1, CustomTreeWidget::COMBO_BOX);
            mpCriteriaTree->setComboBox(pItem, 1, mpInclusionCombo);
         }
      }

      bEnable = true;
   }

   mpFileset = pFileset;
   mpDirectoryEdit->setText(strDirectory);

   mpDirectoryEdit->setEnabled(bEnable);
   mpBrowseButton->setEnabled(bEnable);
   mpCriteriaTree->setEnabled(bEnable);
   mpNewButton->setEnabled(bEnable);
   mpAddButton->setEnabled(bEnable);
   mpDeleteButton->setEnabled(bEnable);

   connect(mpDirectoryEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(modified()));
   connect(mpCriteriaTree, SIGNAL(cellTextChanged(QTreeWidgetItem*, int)), this, SIGNAL(modified()));
}

void FilesetWidget::acceptEditedValues(bool bAccept)
{
   mpCriteriaTree->closeActiveCellWidget(bAccept);
}

void FilesetWidget::browseForDirectory()
{
   acceptEditedValues(true);

   QString strDefault = mpDirectoryEdit->text();
   if (strDefault.isEmpty() == false)
   {
      if (strDefault.endsWith("/") == true)
      {
         strDefault.truncate(strDefault.length() - 1);
      }
   }

   QString strDirectory = QFileDialog::getExistingDirectory(this, "Select File Set Directory", strDefault);
   if (strDirectory.isEmpty() == false)
   {
      strDirectory.replace(QRegExp("\\\\"), "/");
      mpDirectoryEdit->setText(strDirectory);
   }
}

void FilesetWidget::addSearchCriterion()
{
   acceptEditedValues(true);

   QTreeWidgetItem* pItem = new QTreeWidgetItem(mpCriteriaTree);
   if (pItem != NULL)
   {
      pItem->setText(0, "*.*");
      pItem->setText(1, "include");

      mpCriteriaTree->setCellWidgetType(pItem, 0, CustomTreeWidget::BROWSE_FILE_EDIT);
      mpCriteriaTree->setFileBrowser(pItem, 0, mpFileBrowser);
      mpCriteriaTree->setCellWidgetType(pItem, 1, CustomTreeWidget::COMBO_BOX);
      mpCriteriaTree->setComboBox(pItem, 1, mpInclusionCombo);
   }

   emit modified();
}

void FilesetWidget::addFiles()
{
   acceptEditedValues(true);

   bool bSuccess = false;
   while (bSuccess == false)
   {
      QString strFilesetDirectory = mpDirectoryEdit->text();
      QDir filesetDir (strFilesetDirectory);
      strFilesetDirectory = filesetDir.path();

      QStringList filenames = QFileDialog::getOpenFileNames(this, "Add Files to File Set", strFilesetDirectory);
      if (filenames.isEmpty() == true)
      {
         return;
      }

      int iFiles = 0;
      iFiles = filenames.count();
      for (int i = 0; i < iFiles; i++)
      {
         QString strFilename = filenames[i];
         if (strFilename.isEmpty() == false)
         {
            QFileInfo fileInfo(strFilename);

            QString strDirectory = fileInfo.absolutePath();

            if ((strDirectory == strFilesetDirectory) || (strFilesetDirectory.isEmpty() == true))
            {
               QString strText = fileInfo.fileName();
               if (strText.isEmpty() == false)
               {
                  QTreeWidgetItem* pItem = new QTreeWidgetItem(mpCriteriaTree);
                  if (pItem != NULL)
                  {
                     pItem->setText(0, strText);
                     pItem->setText(1, "include");

                     mpCriteriaTree->setCellWidgetType(pItem, 0, CustomTreeWidget::BROWSE_FILE_EDIT);
                     mpCriteriaTree->setFileBrowser(pItem, 0, mpFileBrowser);
                     mpCriteriaTree->setCellWidgetType(pItem, 1, CustomTreeWidget::COMBO_BOX);
                     mpCriteriaTree->setComboBox(pItem, 1, mpInclusionCombo);
                     bSuccess = true;
                  }
               }
            }
            else
            {
               QMessageBox::critical(this, "Batch Wizard Editor", "The files must exist in the current "
                  "file set directory!");
               break;
            }
         }
      }
   }

   if (bSuccess == true)
   {
      emit modified();
   }
}

void FilesetWidget::deleteSelectedCriteria()
{
   acceptEditedValues(true);

   int iNumSelected = 0;

   QTreeWidgetItemIterator iter(mpCriteriaTree);
   while (*iter != NULL)
   {
      QTreeWidgetItem* pItem = *iter;
      if (pItem != NULL)
      {
         bool bSelected = mpCriteriaTree->isItemSelected(pItem);
         if (bSelected == true)
         {
            delete pItem;
            iNumSelected++;
         }
         else
         {
            ++iter;
         }
      }
   }

   if (iNumSelected > 0)
   {
      emit modified();
   }
}

void FilesetWidget::validateFile(QTreeWidgetItem* pItem, int iColumn)
{
   if ((pItem == NULL) || (iColumn != 0))
   {
      return;
   }

   QString strText = pItem->text(0);
   if (strText.isEmpty() == false)
   {
      QFileInfo fileInfo(strText);
      if (fileInfo.isFile() == true)
      {
         QString strFilename;
         QString strFilesetDirectory = mpDirectoryEdit->text();
         QString strDirectory = fileInfo.absolutePath();

         if ((strDirectory == strFilesetDirectory) || (strFilesetDirectory.isEmpty() == true))
         {
            strFilename = fileInfo.fileName();
         }
         else
         {
            QMessageBox::critical(this, "Batch Wizard Editor", "The filename is invalid because it "
               "is outside of the current file set directory!");
         }

         if (strFilename != strText)
         {
            pItem->setText(0, strFilename);
         }
      }
   }
}

void FilesetWidget::updateFileset()
{
   if (mpFileset == NULL)
   {
      return;
   }

   // Directory
   QString strDirectory = mpDirectoryEdit->text();
   if (strDirectory.isEmpty() == false)
   {
      strDirectory.replace(QRegExp("\\\\"), "/");
      mpFileset->setDirectory(strDirectory.toStdString());
   }

   // Criteria
   mpFileset->clearFilesetRequirements();

   QTreeWidgetItemIterator iter(mpCriteriaTree);
   while (*iter != NULL)
   {
      QTreeWidgetItem* pItem = *iter;
      if (pItem != NULL)
      {
         QString strCriterion = pItem->text(0);
         QString strInclusion = pItem->text(1);

         if (strCriterion.isEmpty() == false)
         {
            BatchFileset::FilesetRequirementType eType = BatchFileset::INCLUDE;
            if (strInclusion == "exclude")
            {
               eType = BatchFileset::EXCLUDE;
            }

            mpFileset->addFilesetRequirement(eType, strCriterion.toStdString());
         }
      }

      ++iter;
   }
}
