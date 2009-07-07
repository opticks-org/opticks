/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtGui/QFileDialog>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>

#include "SearchDlg.h"
#include "Progress.h"

SearchDlg::SearchDlg(Progress* pProgress, QWidget* parent) :
   QDialog(parent),
   mpProgress(pProgress),
   mbAbort(false)
{
   // Tab widget
   QTabWidget* pTabWidget = new QTabWidget(this);

   // Location tab
   QWidget* pLocationWidget = new QWidget(pTabWidget);

   QIcon icnBrowse(":/icons/Open");

   QLabel* pDirectoryLabel = new QLabel("Search Directories:", pLocationWidget);
   mpDirectoryEdit = new QTextEdit(pLocationWidget);
   QPushButton* pBrowseButton = new QPushButton(icnBrowse, QString(), pLocationWidget);
   pBrowseButton->setFixedWidth(27);
   mpSubDirCheck = new QCheckBox("Include Subdirectories", pLocationWidget);
   mpSubDirCheck->setChecked(true);

   QGridLayout* pLocationGrid = new QGridLayout(pLocationWidget);
   pLocationGrid->setMargin(10);
   pLocationGrid->setSpacing(5);
   pLocationGrid->addWidget(pDirectoryLabel, 0, 0, 1, 2);
   pLocationGrid->addWidget(mpDirectoryEdit, 1, 0);
   pLocationGrid->addWidget(pBrowseButton, 1, 1, Qt::AlignTop);
   pLocationGrid->addWidget(mpSubDirCheck, 2, 0, 1, 2);
   pLocationGrid->setRowStretch(1, 10);
   pLocationGrid->setColumnStretch(0, 10);

   // Type tab
   QWidget* pTypeWidget = new QWidget(pTabWidget);

   QLabel* pTypeLabel = new QLabel("File Types:", pTypeWidget);
   mpTypeList = new QListWidget(pTypeWidget);
   mpTypeList->setSelectionMode(QAbstractItemView::ExtendedSelection);

   QVBoxLayout* pTypeLayout = new QVBoxLayout(pTypeWidget);
   pTypeLayout->setMargin(10);
   pTypeLayout->setSpacing(5);
   pTypeLayout->addWidget(pTypeLabel);
   pTypeLayout->addWidget(mpTypeList, 10);

   // Buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Dialog layout
   QVBoxLayout* pButtonLayout = new QVBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addWidget(pOkButton);
   pButtonLayout->addWidget(pCancelButton);
   pButtonLayout->addStretch(10);

   QHBoxLayout* pLayout = new QHBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pTabWidget, 10);
   pLayout->addLayout(pButtonLayout);

   // Initialization
   setWindowTitle("Search");
   setModal(true);
   resize(500, 200);
   pTabWidget->addTab(pLocationWidget, "Location");
   pTabWidget->addTab(pTypeWidget, "File Type");

   // Connections
   connect(pBrowseButton, SIGNAL(clicked()), this, SLOT(browseDirectories()));
   connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

SearchDlg::~SearchDlg()
{
}

QStringList SearchDlg::getFilenames() const
{
   return mFileList;
}

QStringList SearchDlg::getDirectories()
{
   QStringList directoryList;

   // Disable progress
   Progress* pProgress = mpProgress;
   mpProgress = NULL;

   // Get search directories
   createDirectoryList(directoryList);

   // Enable progress
   mpProgress = pProgress;

   return directoryList;
}

bool SearchDlg::usesSubdirectories() const
{
   bool bSubdirectories = false;
   bSubdirectories = mpSubDirCheck->isChecked();

   return bSubdirectories;
}

QStringList SearchDlg::getTypes() const
{
   QStringList typeList;

   for (int i = 0; i < mpTypeList->count(); i++)
   {
      QListWidgetItem* pItem = mpTypeList->item(i);
      if (pItem != NULL)
      {
         QString strType = pItem->text();
         if (strType.isEmpty() == false)
         {
            typeList.append(strType);
         }
      }
   }

   return typeList;
}

void SearchDlg::setBrowseDirectory(const QString& strDirectory)
{
   mBrowseDirectory = strDirectory;
}

void SearchDlg::setSearchDirectories(QStringList directoryList)
{
   mpDirectoryEdit->clear();

   int iCount = 0;
   iCount = directoryList.count();
   for (int i = 0; i < iCount; i++)
   {
      QString strDirectory = directoryList[i];
      if (strDirectory.isEmpty() == false)
      {
         mpDirectoryEdit->append(strDirectory);
      }
   }
}

void SearchDlg::setSubdirectories(bool bSubdirectories)
{
   mpSubDirCheck->setChecked(bSubdirectories);
}

void SearchDlg::setTypes(QStringList typeList)
{
   mpTypeList->clear();

   if (typeList.isEmpty() == false)
   {
      mpTypeList->addItems(typeList);
      mpTypeList->selectAll();
   }
}

bool SearchDlg::createDirectoryList(QStringList& directoryList)
{
   QString strAbortMessage = "Search aborted!";

   QString strDirectories = mpDirectoryEdit->toPlainText();
   directoryList = strDirectories.split("\n", QString::SkipEmptyParts);

   int i = 0;
   for (i = 0; i < directoryList.count(); ++i)
   {
      // Check for cancel from user
      if (mbAbort == true)
      {
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(strAbortMessage.toStdString(), 0, ABORT);
         }

         directoryList.clear();
         mbAbort = false;
         return false;
      }

      QString strItemText = directoryList[i];
      if (strItemText.isEmpty() == false)
      {
         strItemText = strItemText.trimmed();
         strItemText.replace(QRegExp("\\\\"), "/");
         if (strItemText.endsWith("/") == true)
         {
            strItemText.truncate(strItemText.length() - 1);
         }

         if (strItemText.isEmpty() == false)
         {
            // Add the listed directory
            QDir directory(strItemText);

            QString strDirectory = directory.absolutePath();
            if (strDirectory.isEmpty() == false)
            {
               directoryList[i] = strDirectory;
            }
         }
      }
   }

   // Add the subdirectories to the list
   int iDirectories = directoryList.count();
   if ((mpSubDirCheck->isChecked() == true) && (iDirectories > 0))
   {
      int iProgress = 0;
      for (i = 0; i < iDirectories; i++)
      {
         // Check for cancel from user
         if (mbAbort == true)
         {
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(strAbortMessage.toStdString(), 0, ABORT);
            }

            directoryList.clear();
            mbAbort = false;
            return false;
         }

         QDir directory(directoryList[i]);
         QStringList subdirectoryList = directory.entryList(QDir::Dirs);

         int iSubdirectories = 0;
         iSubdirectories = subdirectoryList.count();
         for (int j = 0; j < iSubdirectories; j++)
         {
            QString strSubdirectory = subdirectoryList[j];
            if ((strSubdirectory != ".") && (strSubdirectory != ".."))
            {
               directoryList.append(directoryList[i] + "/" + strSubdirectory);
            }
         }

         iDirectories = directoryList.count();

         if (mpProgress != NULL)
         {
            int iCurrentProgress = (i + 1) * 100 / iDirectories;
            if (iCurrentProgress > iProgress)
            {
               iProgress = iCurrentProgress;
            }

            mpProgress->updateProgress("Preparing to search directories...", iProgress, NORMAL);
         }
      }
   }

   return true;
}

void SearchDlg::abortSearch()
{
   mbAbort = true;
}

void SearchDlg::accept()
{
   mFileList.clear();

   // Only search entered directories
   QString strText = mpDirectoryEdit->toPlainText();
   if (strText.isEmpty() == true)
   {
      QMessageBox::critical(this, windowTitle(), "Please select at least one directory to search!");
      return;
   }

   // Update progress
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Preparing to search directories...", 0, NORMAL);
   }

   // Create list of directories to search
   QStringList directoryList;

   bool bSuccess = false;
   bSuccess = createDirectoryList(directoryList);
   if (directoryList.isEmpty() == true)
   {
      return;
   }

   // Get the file types to search
   QStringList fileTypes;

   QList<QListWidgetItem*> selectedItems = mpTypeList->selectedItems();

   int i;
   for (i = 0; i < selectedItems.count(); ++i)
   {
      QListWidgetItem* pItem = selectedItems[i];
      if (pItem != NULL)
      {
         QString strType = pItem->text();
         if (strType.isEmpty() == false)
         {
            int iStartPos = strType.indexOf("(") + 1;
            int iEndPos = strType.lastIndexOf(")");

            QString strFilter = strType.mid(iStartPos, iEndPos - iStartPos);
            if (strFilter.isEmpty() == false)
            {
               QStringList currentTypes = strFilter.split(" ", QString::SkipEmptyParts);
               for (int j = 0; j < currentTypes.count(); ++j)
               {
                  QString strCurrentType = currentTypes[j];
                  if (strCurrentType.isEmpty() == false)
                  {
                     fileTypes.append(strCurrentType.trimmed());
                  }
               }
            }
         }
      }
   }

   if (fileTypes.empty() == true)
   {
      fileTypes.append("*");
   }

   // Search each directory
   int iDirectories = 0;
   iDirectories = directoryList.count();
   for (i = 0; i < iDirectories; i++)
   {
      // Check for cancel from user
      if (mbAbort == true)
      {
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress("Search aborted!", 0, ABORT);
         }

         mFileList.clear();
         mbAbort = false;
         return;
      }

      // Get the current directory
      QDir currentDirectory(directoryList[i]);

      // Update the progress bar
      if (mpProgress != NULL)
      {
         QString strMessage = "Searching directory for files:\n" + currentDirectory.absolutePath();
         int iProgress = i * 100 / iDirectories;

         mpProgress->updateProgress(strMessage.toStdString(), iProgress, NORMAL);
      }

      // Search
      QStringList files = currentDirectory.entryList(fileTypes, QDir::Files);
      for (int k = 0; k < files.count(); k++)
      {
         mFileList.append(currentDirectory.absolutePath() + "/" + files[k]);
      }
   }

   // Close the progress dialog
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("File search complete!", 100, NORMAL);
   }

   QDialog::accept();
}

void SearchDlg::browseDirectories()
{
   if (mBrowseDirectory.isEmpty() == true)
   {
      mBrowseDirectory = QDir::currentPath();
   }

   QString strDirectory = QFileDialog::getExistingDirectory(this, "Select Search Directory", mBrowseDirectory);
   if (strDirectory.isEmpty() == false)
   {
      mpDirectoryEdit->append(strDirectory);
   }
}
