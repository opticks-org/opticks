/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtCore/QRegExp>
#include <QtGui/QGridLayout>
#include <QtGui/QListView>
#include <QtGui/QMessageBox>
#include <QtGui/QSplitter>
#include <QtGui/QToolButton>

#include "AppVerify.h"
#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "DataDescriptor.h"
#include "FileDescriptor.h"
#include "ImportAgentImp.h"
#include "ImportDescriptorImp.h"
#include "ImportDlg.h"
#include "Importer.h"
#include "ImportOptionsDlg.h"
#include "PlugIn.h"
#include "PlugInManagerServices.h"
#include "PreviewWidget.h"

#include <algorithm>
using namespace std;

ImportDlg::ImportDlg(const QString& strPlugInSubtype, const QString& strInitialPlugIn, QWidget* pParent) :
   FilePlugInDlg(getPlugInNames(PlugInManagerServices::ImporterType(), strPlugInSubtype.toStdString()),
      ConfigurationSettings::getSettingPluginWorkingDirectoryKey(strPlugInSubtype.toStdString()), pParent),
   mpImporter(NULL),
   mpPreview(new PreviewWidget(this)),
   mPreviewEnabled(false)
{
   // Preview
   if (mpPreview != NULL)
   {
      // Create the preview tool button
      QToolButton* pPreviewButton = new QToolButton(this);
      pPreviewButton->setIcon(style()->standardPixmap(QStyle::SP_FileDialogContentsView));
      pPreviewButton->setToolTip("Preview");
      pPreviewButton->setAutoRaise(true);
      pPreviewButton->setCheckable(true);
      pPreviewButton->setFixedSize(QSize(22, 22));
      connect(pPreviewButton, SIGNAL(toggled(bool)), mpPreview, SLOT(setVisible(bool)));

      // Add the preview tool button to the dialog layout and get the splitter
      QSplitter* pSplitter = NULL;

      QGridLayout* pGrid = dynamic_cast<QGridLayout*>(layout());
      if (pGrid != NULL)
      {
         int numItems = pGrid->count();
         for (int i = 0; i < numItems; ++i)
         {
            QLayoutItem* pItem = pGrid->itemAt(i);
            if (pItem != NULL)
            {
               int row = -1;
               int column = -1;
               int rowSpan = -1;
               int columnSpan = -1;
               pGrid->getItemPosition(i, &row, &column, &rowSpan, &columnSpan);

               if ((row == 0) && (column == 1) && (rowSpan == 1) && (columnSpan == 2))
               {
                  QHBoxLayout* pLayout = dynamic_cast<QHBoxLayout*>(pItem->layout());
                  if (pLayout != NULL)
                  {
                     pLayout->addWidget(pPreviewButton);
                  }
               }
               else if ((row == 1) && (column == 0) && (rowSpan == 1) && (columnSpan == 3))
               {
                  pSplitter = dynamic_cast<QSplitter*>(pItem->widget());
               }
            }
         }
      }

      // Add the preview widget to the splitter
      if (pSplitter != NULL)
      {
         pSplitter->addWidget(mpPreview);
      }

      // Initialize the preview widget
      mpPreview->installEventFilter(this);
      mpPreview->hide();
   }

   // Initializtion
   setWindowTitle("Import");
   setFileMode(QFileDialog::ExistingFiles);
   setAcceptMode(QFileDialog::AcceptOpen);
   setPlugInLabel("Importer");
   enableOptions(true);

   // Set the initial directory
   string directory;

   Service<ConfigurationSettings> pSettings;
   const Filename* pWorkingDir = pSettings->getSetting(ConfigurationSettings::getSettingPluginWorkingDirectoryKey(
      strPlugInSubtype.toStdString())).getPointerToValue<Filename>();
   if (pWorkingDir == NULL)
   {
      pWorkingDir = ConfigurationSettings::getSettingImportPath();
   }

   if (pWorkingDir != NULL)
   {
      directory = pWorkingDir->getFullPathAndName();
   }

   if (!directory.empty())
   {
      setDirectory(QString::fromStdString(directory));
   }

   // If the auto importer is available, select it from the drop down
   if (strInitialPlugIn.isEmpty() == true)
   {
      if (isDefaultPlugIn())
      {
         setSelectedPlugIn("Auto Importer");
      }
   }
   else
   {
      setSelectedPlugIn(strInitialPlugIn);
   }

   updateFromImporter(getSelectedPlugIn());

   // Connections
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Change the selectionChanged() signal to " \
   "filesSelected() and remove the mFilename member when the filesSelected() signal is emitted when the " \
   "selection changes but before the dialog is accepted. (Qt 4.5.2) (dsulgrov)")
   QListView* pListView = findChild<QListView*>("listView");
   VERIFYNRV(pListView != NULL);

   VERIFYNR(connect(pListView->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(updateFromFiles())));
   VERIFYNR(connect(this, SIGNAL(plugInSelected(const QString&)), this, SLOT(updateFromImporter(const QString&))));
   VERIFYNR(connect(this, SIGNAL(optionsClicked()), this, SLOT(invokeOptionsDialog())));
}

ImportDlg::~ImportDlg()
{
}

QMap<QString, vector<ImportDescriptor*> > ImportDlg::getImportDescriptors()
{
   updateDescriptorsIfNeeded();
   return mFiles;
}

Importer* ImportDlg::getSelectedImporter() const
{
   return mpImporter;
}

void ImportDlg::accept()
{
   // Check if the user typed in a directory
   QStringList files = selectedFiles();
   for (int i = 0; i < files.count(); ++i)
   {
      QFileInfo info(files.at(i));
      if (info.isDir() == true)
      {
         // Switch to the directory
         QFileDialog::accept();  // Do not call FilePlugInDlg::accept() to prevent the config settings from updating
         return;
      }
   }

   // Check for a valid plug-in
   QString strPlugIn = getSelectedPlugIn();
   if (strPlugIn.isEmpty() == true)
   {
      QMessageBox::critical(this, APP_NAME, "The selected importer is invalid.");
      return;
   }

   // Update the descriptors for all files if necessary
   updateDescriptorsIfNeeded();

   // Get the selected files
   if (mFiles.empty() == true)
   {
      QMessageBox::critical(this, APP_NAME, "Please select one or more files to import.");
      return;
   }

   // Ensure that each file exists and that it is recognized by the selected importer
   for (QMap<QString, vector<ImportDescriptor*> >::iterator iter = mFiles.begin(); iter != mFiles.end(); ++iter)
   {
      QString filename = iter.key();
      filename.replace(QRegExp("\\\\"), "/");

      QFileInfo fileInfo = QFileInfo(filename);
      if (fileInfo.exists() == true)
      {
         if (iter.value().empty() == true)
         {
            QString strImporter = getSelectedPlugIn();
            QMessageBox::warning(this, APP_NAME, strImporter + " does not recognize the " + filename + " file.");
            return;
         }
      }
      else
      {
         QMessageBox::critical(this, APP_NAME, "The '" + fileInfo.fileName() + "' file does not exist.  "
            "Please select only existing files.");
         return;
      }
   }

   // Validate the imported data sets and invoke the options dialog if any imported data set is invalid
   vector<ImportDescriptor*> descriptors;
   unsigned int numImportedDatasets = 0;

   for (QMap<QString, vector<ImportDescriptor*> >::const_iterator iter = mFiles.begin(); iter != mFiles.end(); ++iter)
   {
      vector<ImportDescriptor*> fileDescriptors = iter.value();
      for (vector<ImportDescriptor*>::iterator descriptorIter = fileDescriptors.begin();
         descriptorIter != fileDescriptors.end();
         ++descriptorIter)
      {
         ImportDescriptor* pImportDescriptor = *descriptorIter;
         if (pImportDescriptor != NULL)
         {
            if (pImportDescriptor->isImported() == true)
            {
               ++numImportedDatasets;
            }

            descriptors.push_back(pImportDescriptor);
         }
      }
   }

   string errorMessage;
   if (ImportAgentImp::validateImportDescriptors(descriptors, mpImporter, errorMessage) != numImportedDatasets)
   {
      if (invokeOptionsDialog() == false)
      {
         return;
      }
   }

   // Reset the preview to ensure the importer's preview widget is deleted before the importer is unloaded
   if (mpPreview != NULL)
   {
      mpPreview->setCurrentFile(QString());
   }

   FilePlugInDlg::accept();
}

void ImportDlg::reject()
{
   // Reset the preview to ensure the importer's preview widget is deleted before the importer is unloaded
   if (mpPreview != NULL)
   {
      mpPreview->setCurrentFile(QString());
   }

   // Destroy the importer
   if (mpImporter != NULL)
   {
      Service<PlugInManagerServices> pManager;
      pManager->destroyPlugIn(dynamic_cast<PlugIn*>(mpImporter));
   }

   FilePlugInDlg::reject();
}

void ImportDlg::hideEvent(QHideEvent* pEvent)
{
   // Set the selected directory to the current directory
   QDir dir = directory();
   QDir::setCurrent(dir.absolutePath());

   FilePlugInDlg::hideEvent(pEvent);
}

bool ImportDlg::updateFromFiles()
{
   QStringList filenames = selectedFiles();
   filenames.replaceInStrings(QRegExp("\\\\"), "/");

   // Remove files that are no longer selected
   for (QMap<QString, vector<ImportDescriptor*> >::iterator iter = mFiles.begin(); iter != mFiles.end(); )
   {
      QString filename = iter.key();
      if (filenames.contains(filename) == false)
      {
         clearDescriptors(filename);
         iter = mFiles.erase(iter);
      }
      else
      {
         ++iter;
      }
   }

   // Add newly selected files
   for (int i = 0; i < filenames.count(); ++i)
   {
      QString filename = filenames[i];

      QFileInfo fileInfo(filename);
      if ((fileInfo.isFile() == true) && (fileInfo.exists() == true))
      {
         QMap<QString, vector<ImportDescriptor*> >::iterator iter = mFiles.find(filename);
         if (iter == mFiles.end())
         {
            mFiles[filename] = vector<ImportDescriptor*>();
         }
      }
   }

   // Update the preview data sets
   updatePreviewDatasets();

   return true;
}

void ImportDlg::updateDescriptorsIfNeeded()
{
   if (mpImporter == NULL)
   {
      return;
   }

   for (QMap<QString, vector<ImportDescriptor*> >::iterator iter = mFiles.begin(); iter != mFiles.end(); ++iter)
   {
      QString filename = iter.key();

      vector<ImportDescriptor*> descriptors = iter.value();
      if (descriptors.empty() == true)
      {
         mFiles[filename] = mpImporter->getImportDescriptors(filename.toStdString());
      }
   }
}

void ImportDlg::clearDescriptors(const QString& filename)
{
   QMap<QString, vector<ImportDescriptor*> >::iterator iter = mFiles.find(filename);
   if (iter != mFiles.end())
   {
      vector<ImportDescriptor*> descriptors = iter.value();

      for (vector<ImportDescriptor*>::iterator descriptorIter = descriptors.begin();
         descriptorIter != descriptors.end();
         ++descriptorIter)
      {
         ImportDescriptorImp* pDescriptor = dynamic_cast<ImportDescriptorImp*>(*descriptorIter);
         if (pDescriptor != NULL)
         {
            delete pDescriptor;
         }
      }

      iter.value().clear();
   }
}

void ImportDlg::clearDescriptors()
{
   for (QMap<QString, vector<ImportDescriptor*> >::iterator iter = mFiles.begin(); iter != mFiles.end(); ++iter)
   {
      clearDescriptors(iter.key());
   }
}

void ImportDlg::updateFromImporter(const QString& strImporter)
{
   // Update the importer
   Service<PlugInManagerServices> pManager;
   if (mpImporter != NULL)
   {
      pManager->destroyPlugIn(dynamic_cast<PlugIn*>(mpImporter));
      mpImporter = NULL;
   }

   if (strImporter.isEmpty() == false)
   {
      string importer = strImporter.toStdString();
      mpImporter = dynamic_cast<Importer*>(pManager->createPlugIn(importer));
   }

   // Clear the descriptors for all selected files
   clearDescriptors();

   // Update the preview with the new importer
   updatePreviewDatasets();
}

bool ImportDlg::invokeOptionsDialog()
{
   // Update the descriptors for all files if necessary
   updateDescriptorsIfNeeded();

   // Get the selected files
   if (mFiles.empty() == true)
   {
      QMessageBox::critical(this, APP_NAME, "Please select one or more files to display the import options.");
      return false;
   }

   // Ensure that each file exists and that it is recognized by the selected importer
   for (QMap<QString, vector<ImportDescriptor*> >::iterator iter = mFiles.begin(); iter != mFiles.end(); ++iter)
   {
      QString filename = iter.key();
      filename.replace(QRegExp("\\\\"), "/");

      QFileInfo fileInfo = QFileInfo(filename);
      if (fileInfo.exists() == true)
      {
         if (iter.value().empty() == true)
         {
            QString strImporter = getSelectedPlugIn();
            QMessageBox::warning(this, APP_NAME, strImporter + " does not recognize the " + filename + " file.");
            return false;
         }
      }
      else
      {
         QMessageBox::critical(this, APP_NAME, "The '" + fileInfo.fileName() + "' file does not exist.  "
            "Please select only existing files.");
         return false;
      }
   }

   // Create the import options dialog
   ImportOptionsDlg optionsDlg(mpImporter, mFiles, this);
   optionsDlg.allowDeselectedFiles(false);

   // Initialize the file and data set based on the current preview data set
   if ((mpPreview != NULL) && (mPreviewEnabled == true))
   {
      ImportDescriptor* pDataset = mpPreview->getCurrentDataset();
      if ((pDataset != NULL) && (pDataset->isImported() == true))
      {
         optionsDlg.setCurrentDataset(pDataset);
      }
   }

   // Invoke the dialog
   if (optionsDlg.exec() == QDialog::Rejected)
   {
      return false;
   }

   // Update the preview data set based on the selected data set in the options dialog
   if ((mpPreview != NULL) && (mPreviewEnabled == true))
   {
      // File
      QString filename = optionsDlg.getCurrentFile();
      mpPreview->setCurrentFile(filename);

      // Dataset
      ImportDescriptor* pDataset = optionsDlg.getCurrentDataset();
      if ((pDataset != NULL) && (pDataset->isImported() == true))
      {
         mpPreview->setCurrentDataset(pDataset);
      }
   }

   return true;
}

bool ImportDlg::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if ((pObject != NULL) && (pEvent != NULL))
   {
      if ((pObject == mpPreview) && (pEvent->type() == QEvent::Show))
      {
         // Enable the preview
         mPreviewEnabled = true;
         updatePreviewDatasets();
      }
      else if ((pObject == mpPreview) && (pEvent->type() == QEvent::Hide))
      {
         mPreviewEnabled = false;
      }
      else if ((pEvent->type() == QEvent::DragEnter) || (pEvent->type() == QEvent::DragLeave) ||
         (pEvent->type() == QEvent::DragMove) || (pEvent->type() == QEvent::DragResponse))
      {
         return true;
      }
   }

   return FilePlugInDlg::eventFilter(pObject, pEvent);
}

void ImportDlg::updatePreviewDatasets()
{
   if ((mpPreview != NULL) && (mPreviewEnabled == true))
   {
      updateDescriptorsIfNeeded();

      // Update the importer
      mpPreview->setImporter(mpImporter);

      // Update the data sets, which will reset the active data set
      mpPreview->setDatasets(mFiles);
   }
}
