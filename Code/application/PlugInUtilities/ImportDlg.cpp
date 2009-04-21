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
#include <QtGui/QMessageBox>
#include <QtGui/QSplitter>
#include <QtGui/QToolButton>

#include "AppVerify.h"
#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "ImportAgentImp.h"
#include "ImportDescriptorImp.h"
#include "ImportDlg.h"
#include "Importer.h"
#include "ImportOptionsDlg.h"
#include "PlugIn.h"
#include "PlugInManagerServicesImp.h"
#include "PreviewWidget.h"

#include <string>
using namespace std;

ImportDlg::ImportDlg(const QString& strPlugInSubtype, const QString& strInitialPlugIn, QWidget* pParent) :
   FilePlugInDlg(getPlugInNames(PlugInManagerServices::ImporterType(), strPlugInSubtype.toStdString()),
      ConfigurationSettings::getSettingPluginWorkingDirectoryKey(strPlugInSubtype.toStdString()), pParent),
   mFileChanged(true),
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
   setFileMode(QFileDialog::ExistingFile);
   setAcceptMode(QFileDialog::AcceptOpen);
   setPlugInLabel("Importer");
   enableOptions(true);

   // Set the initial directory
   string directory;
   const Filename* pWorkingDir = NULL;
   Service<ConfigurationSettings> pSettings;
   pWorkingDir = pSettings->getSetting(ConfigurationSettings::getSettingPluginWorkingDirectoryKey(strPlugInSubtype.toStdString())).getPointerToValue<Filename>();
   if (pWorkingDir == NULL)
   {
      pWorkingDir = ConfigurationSettings::getSettingImportPath();
   }
   if (pWorkingDir != NULL)
   {
      directory = pWorkingDir->getFullPathAndName();
   }

   if(!directory.empty())
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
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Change the currentChanged() signal to " \
   "filesSelected() and remove the mFilename member when the filesSelected() signal is emitted when the " \
   "selection changes but before the dialog is accepted. (Qt 4.5.0) (dsulgrov)")
   VERIFYNR(connect(this, SIGNAL(currentChanged(const QString&)), this, SLOT(updateFromFile(const QString&))));
   VERIFYNR(connect(this, SIGNAL(plugInSelected(const QString&)), this, SLOT(updateFromImporter(const QString&))));
   VERIFYNR(connect(this, SIGNAL(optionsClicked()), this, SLOT(invokeOptionsDialog())));
}

ImportDlg::~ImportDlg()
{
}

const vector<ImportDescriptor*>& ImportDlg::getImportDescriptors() const
{
   const_cast<ImportDlg*>(this)->updateDescriptorsIfNeeded();
   return mDescriptors;
}

PlugIn* ImportDlg::getSelectedImporter() const
{
   return mpImporter;
}

void ImportDlg::accept()
{
   // Check for a valid plug-in
   QString strPlugIn = getSelectedPlugIn();
   if (strPlugIn.isEmpty() == true)
   {
      QMessageBox::critical(this, APP_NAME, "The selected importer is invalid!");
      return;
   }

   // Get the selected file
   if (mFilename.empty())
   {
      return;
   }

   QString strFilename = QString::fromStdString(mFilename);
   QFileInfo fileInfo = QFileInfo(strFilename);

   bool bExists = fileInfo.exists();
   if (bExists == true)
   {
      updateDescriptorsIfNeeded();

      if (mDescriptors.empty())
      {
         QString strImporter = getSelectedPlugIn();
         QMessageBox::warning(this, APP_NAME, strImporter + " does not recognize the " + strFilename + " file!");
         return;
      }

      // Invoke the Options dialog if no valid data sets are selected
      string errorMessage;
      if (ImportAgentImp::validateImportDescriptors(mDescriptors, dynamic_cast<Importer*>(mpImporter),
         errorMessage) == 0)
      {
         if (invokeOptionsDialog() == false)
         {
            return;
         }

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Remove this check when the import options dialog " \
   "prevents clicking the OK button when one or more imported data sets is invalid, not just the selected " \
   "data set (dsulgrov)")
         if (ImportAgentImp::validateImportDescriptors(mDescriptors, dynamic_cast<Importer*>(mpImporter),
            errorMessage) == 0)
         {
            return;
         }
      }
   }
   else
   {
      QMessageBox::critical(this, APP_NAME, "The '" + fileInfo.fileName() + "' file does not exist!  Please "
         "select an existing file.");
      return;
   }

   if (mpPreview != NULL)
   {
      // Reset the preview to ensure the importer's preview widget is deleted before the importer is unloaded,
      // which must occur after calling the base class method because data descriptor validation may fail and
      // the user could cancel the import options dialog, thereby not actually closing this dialog
      mpPreview->setActiveDataset(NULL);
   }

   FilePlugInDlg::accept();
}

void ImportDlg::reject()
{
   if (mpImporter != NULL)
   {
      PlugInManagerServicesImp* pManager = PlugInManagerServicesImp::instance();
      if (pManager != NULL)
      {
         pManager->destroyPlugIn(mpImporter);
      }
   }

   // Reset the preview to ensure the importer's preview widget is deleted before the importer is unloaded,
   // which must occur before calling the base class method to prevent a crash in Qt when the dialog is hidden
   if (mpPreview != NULL)
   {
      mpPreview->setActiveDataset(NULL);
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

bool ImportDlg::updateFromFile(const QString& strFilename)
{
   // Update the selected filename
   QString strFile = strFilename;
   strFile.replace(QRegExp("\\\\"), "/");

   string filename = strFile.toStdString();

   QFileInfo fileInfo(strFile);
   if (filename != mFilename)
   {
      if (fileInfo.isFile())
      {
         mFilename = filename;
      }
      else
      {
         mFilename.clear();
      }
   }

   // Clear the import descriptors
   clearDescriptors();

   // Update the preview data sets
   updatePreviewDatasets();

   // Set the active data set to the first imported data set in the file
   if ((mpPreview != NULL) && (mPreviewEnabled == true))
   {
      ImportDescriptor* pDataset = getFirstImportedDescriptor();
      mpPreview->setActiveDataset(pDataset);
   }

   return true;
}

void ImportDlg::updateDescriptorsIfNeeded()
{
   if (!mFileChanged)
   {
      return;
   }

   if (mFilename.empty() == false)
   {
      Importer* pImporter = dynamic_cast<Importer*>(mpImporter);
      if (pImporter != NULL)
      {
         mDescriptors = pImporter->getImportDescriptors(mFilename);
      }
   }

   mFileChanged = false;
}

void ImportDlg::clearDescriptors()
{
   vector<ImportDescriptor*>::iterator iter;
   for (iter = mDescriptors.begin(); iter != mDescriptors.end(); ++iter)
   {
      ImportDescriptorImp* pDescriptor = dynamic_cast<ImportDescriptorImp*>(*iter);
      if (pDescriptor != NULL)
      {
         delete pDescriptor;
      }
   }

   mDescriptors.clear();
   mFileChanged = true;
}

ImportDescriptor* ImportDlg::getFirstImportedDescriptor() const
{
   const_cast<ImportDlg*>(this)->updateDescriptorsIfNeeded();
   for (vector<ImportDescriptor*>::const_iterator iter = mDescriptors.begin(); iter != mDescriptors.end(); ++iter)
   {
      ImportDescriptor* pImportDescriptor = *iter;
      if (pImportDescriptor != NULL)
      {
         if (pImportDescriptor->isImported() == true)
         {
            return pImportDescriptor;
         }
      }
   }

   return NULL;
}

void ImportDlg::updateFromImporter(const QString& strImporter)
{
   // Update the importer
   PlugInManagerServicesImp* pManager = PlugInManagerServicesImp::instance();
   if (pManager != NULL)
   {
      if (mpImporter != NULL)
      {
         pManager->destroyPlugIn(mpImporter);
         mpImporter = NULL;
      }

      if (strImporter.isEmpty() == false)
      {
         string importer = strImporter.toStdString();
         mpImporter = pManager->createPlugIn(importer);
      }
   }

   clearDescriptors();
}

bool ImportDlg::invokeOptionsDialog()
{
   updateDescriptorsIfNeeded();

   // Display the dialog
   if (mDescriptors.empty() == false)
   {
      // Use the default dialog
      ImportOptionsDlg optionsDlg(dynamic_cast<Importer*>(mpImporter), mDescriptors, this);
      if (optionsDlg.exec() == QDialog::Accepted)
      {
         // Update the preview data sets
         updatePreviewDatasets();

         // Set the active data set based on the selected data set in the options dialog
         if ((mpPreview != NULL) && (mPreviewEnabled == true))
         {
            ImportDescriptor* pDataset = optionsDlg.getCurrentDataset();
            if (pDataset != NULL)
            {
               if (pDataset->isImported() == false)
               {
                  pDataset = NULL;
               }
            }

            if (pDataset == NULL)
            {
               // The current data set in the dialog is not imported, so get the first imported data set
               pDataset = getFirstImportedDescriptor();
            }

            mpPreview->setActiveDataset(pDataset);
         }

         return true;
      }
   }
   else
   {
      if (mFilename.empty() == false)
      {
         QString filename = QString::fromStdString(mFilename);

         QFileInfo fileInfo(filename);
         if (fileInfo.exists() == false)
         {
            QMessageBox::warning(this, APP_NAME, "The '" + filename + "' file does not exist.  "
               "Please select an existing file.");
         }
         else if (fileInfo.isFile() == true)
         {
            QString strImporter = getSelectedPlugIn();
            QMessageBox::warning(this, APP_NAME, strImporter + " does not recognize the '" + filename + "' file.");
         }
      }
      else
      {
         QMessageBox::warning(this, APP_NAME, "Please select a file to display its import options.");
      }
   }

   return false;
}

bool ImportDlg::eventFilter(QObject* o, QEvent* e)
{
   if ((o != NULL) && (e != NULL))
   {
      if ((o == mpPreview) && (e->type() == QEvent::Show))
      {
         // Enable the preview
         mPreviewEnabled = true;

         // Update the preview data sets
         updatePreviewDatasets();

         // Set the active data set to the first imported data set in the selected file
         if (mpPreview != NULL)
         {
            ImportDescriptor* pDataset = getFirstImportedDescriptor();
            mpPreview->setActiveDataset(pDataset);
         }
      }
      else if ((o == mpPreview) && (e->type() == QEvent::Hide))
      {
         mPreviewEnabled = false;
      }
      else if ((e->type() == QEvent::DragEnter) || (e->type() == QEvent::DragLeave) ||
         (e->type() == QEvent::DragMove) || (e->type() == QEvent::DragResponse))
      {
         return true;
      }
   }

   return FilePlugInDlg::eventFilter(o, e);
}

void ImportDlg::updatePreviewDatasets()
{
   if ((mpPreview != NULL) && (mPreviewEnabled == true))
   {
      // Update the importer
      Importer* pImporter = dynamic_cast<Importer*>(getSelectedImporter());
      mpPreview->setImporter(pImporter);

      // Update the data sets, which will reset the active data set
      const vector<ImportDescriptor*>& descriptors = getImportDescriptors();
      mpPreview->setDatasets(descriptors);
   }
}
