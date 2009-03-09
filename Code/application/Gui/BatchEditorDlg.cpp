/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <QtGui/QBitmap>
#include <QtGui/QCloseEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QLayout>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>

#include "AppConfig.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "BatchEditorDlg.h"
#include "BatchFileParser.h"
#include "BatchFileset.h"
#include "BatchWizard.h"
#include "ConfigurationSettings.h"
#include "FilenameImp.h"
#include "FilesetWidget.h"
#include "IconImages.h"
#include "ObjectResource.h"
#include "WizardUtilities.h"
#include "WizardWidget.h"

#if defined(WIN_API)
#include <windows.h>
#endif

#include <string>
using namespace std;

XERCES_CPP_NAMESPACE_USE

BatchEditorDlg::BatchEditorDlg(QWidget* parent) :
   QMainWindow(parent)
{
   mXmlFilename = QString();
   mbModified = false;

   // View actions
   QActionGroup* pViewGroup = new QActionGroup(this);
   pViewGroup->setExclusive(true);
   VERIFYNR(connect(pViewGroup, SIGNAL(triggered(QAction*)), this, SLOT(setView(QAction*))));

   mpFilesetAction = pViewGroup->addAction("&File Sets");
   mpFilesetAction->setCheckable(true);
   mpFilesetAction->setToolTip("File Sets");

   mpWizardAction = pViewGroup->addAction("&Wizards");
   mpWizardAction->setCheckable(true);
   mpWizardAction->setToolTip("Wizards");

   // Menu bar
   QMenuBar* pMenuBar = menuBar();
   if (pMenuBar != NULL)
   {
      // Icons
      QPixmap pixNew(IconImages::NewIcon);
      pixNew.setMask(pixNew.createHeuristicMask());
      QIcon icnNew(pixNew);

      QPixmap pixOpen(IconImages::OpenIcon);
      pixOpen.setMask(pixOpen.createHeuristicMask());
      QIcon icnOpen(pixOpen);

      QPixmap pixSave(IconImages::SaveIcon);
      pixSave.setMask(pixSave.createHeuristicMask());
      QIcon icnSave(pixSave);

      // File menu
      QMenu* pFileMenu = new QMenu("&File", this);
      pFileMenu->addAction(icnNew, "&New...", this, SLOT(newWizard()));
      pFileMenu->addAction(icnOpen, "&Open...", this, SLOT(open()));
      pFileMenu->addAction(icnSave, "&Save", this, SLOT(save()));
      pFileMenu->addAction("Save &As...", this, SLOT(saveAs()));
      pFileMenu->addSeparator();
      pFileMenu->addAction("Execute &Batch", this, SLOT(executeBatch()));
      pFileMenu->addAction("Execute &Interactive", this, SLOT(executeInteractive()));
      pFileMenu->addSeparator();
      pFileMenu->addAction("&Close", this, SLOT(close()));

      // View menu
      QMenu* pViewMenu = new QMenu("&View", this);
      pViewMenu->addAction(mpFilesetAction);
      pViewMenu->addAction(mpWizardAction);

      // Insert the menus on the menu bar
      pMenuBar->addMenu(pFileMenu);
      pMenuBar->addMenu(pViewMenu);
   }

   // Central widget
   QWidget* pCentralWidget = new QWidget(this);

   // View info widgets
   mpViewLabel = new QLabel(pCentralWidget);
   mpViewCombo = new QComboBox(pCentralWidget);
   mpViewCombo->setEditable(false);
   VERIFYNR(connect(mpViewCombo, SIGNAL(activated(int)), this, SLOT(setWidgetValues(int))));

   QFont labelFont = mpViewLabel->font();
   labelFont.setBold(true);
   mpViewLabel->setFont(labelFont);

   QHBoxLayout* pViewLayout = new QHBoxLayout();
   pViewLayout->setMargin(0);
   pViewLayout->setSpacing(5);
   pViewLayout->addWidget(mpViewLabel);
   pViewLayout->addWidget(mpViewCombo, 10);

   // Fileset widget
   mpFilesetWidget = new FilesetWidget();
   VERIFYNR(connect(mpFilesetWidget, SIGNAL(modified()), this, SLOT(setModified())));

   // Wizard widget
   mpWizardWidget = new WizardWidget();
   VERIFYNR(connect(mpWizardWidget, SIGNAL(modified()), this, SLOT(setModified())));

   // Widget stack
   mpStack = new QStackedWidget(pCentralWidget);
   mpStack->addWidget(mpFilesetWidget);
   mpStack->addWidget(mpWizardWidget);

   // Horizontal line
   QFrame* pLine = new QFrame(pCentralWidget);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pAddButton = new QPushButton("&Add...", pCentralWidget);
   QPushButton* pRemoveButton = new QPushButton("&Remove", pCentralWidget);
   QPushButton* pCloseButton = new QPushButton("&Close", pCentralWidget);
   pCloseButton->setDefault(true);
   pCloseButton->setFocus();

   VERIFYNR(connect(pAddButton, SIGNAL(clicked()), this, SLOT(add())));
   VERIFYNR(connect(pRemoveButton, SIGNAL(clicked()), this, SLOT(remove())));
   VERIFYNR(connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close())));

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addWidget(pAddButton);
   pButtonLayout->addWidget(pRemoveButton);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(pCloseButton);

   // Layout
   QGridLayout* pGrid = new QGridLayout(pCentralWidget);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addLayout(pViewLayout, 0, 0);
   pGrid->addWidget(mpStack, 1, 0);
   pGrid->addWidget(pLine, 2, 0);
   pGrid->addLayout(pButtonLayout, 3, 0);

   // Initialization
   setAttribute(Qt::WA_DeleteOnClose);
   setCentralWidget(pCentralWidget);
   mpStack->setCurrentWidget(mpWizardWidget);
   mpWizardAction->setChecked(true);
   setBatchWizard(QString());
   resize(600, 400);
}

BatchEditorDlg::~BatchEditorDlg()
{
   destroyFilesets();
   destroyWizards();
}

bool BatchEditorDlg::setBatchWizard(const QString& strXmlFilename)
{
   mXmlFilename.clear();
   QString strCaption = APP_NAME " Batch Wizard Editor - ";

   destroyFilesets();
   destroyWizards();

   if (strXmlFilename.isEmpty() == false)
   {
      QFileInfo fileInfo(strXmlFilename);

      bool bFile = false;
      bFile = fileInfo.isFile();
      if (bFile == true)
      {
         BatchFileParser fileParser;

         // Test for a valid XML file
         bool bSuccess = false;
         bSuccess = fileParser.setFile(strXmlFilename.toStdString());
         if (bSuccess == false)
         {
            QString strError = "Could not read the " + strXmlFilename + " file!";

            string errorMessage = fileParser.getError();
            if (errorMessage.empty() == false)
            {
               strError = QString::fromStdString(errorMessage);
            }

            QMessageBox::critical(this, "Batch Wizard Editor", strError);
            return false;
         }

         // Set the member filename
         mXmlFilename = strXmlFilename;

         // Get the file sets
         fileParser.getFileSets(mFilesets);

         // Get the wizards
         BatchWizard* pBatchWizard = NULL;
         pBatchWizard = fileParser.read();
         while (pBatchWizard != NULL)
         {
            mWizards.push_back(pBatchWizard);

            pBatchWizard = NULL;
            pBatchWizard = fileParser.read();
         }

         // Update the caption
         QString strName = fileInfo.fileName();
         strCaption += strName;
      }
   }
   else
   {
      strCaption += "Untitled";
   }

   // Set view widget values
   updateViewWidgets();

   // Set caption
   setWindowTitle(strCaption);

   mbModified = false;
   return true;
}

void BatchEditorDlg::closeEvent(QCloseEvent* e)
{
   bool bSaved = false;
   bSaved = promptForSave();
   if (bSaved == false)
   {
      e->ignore();
      return;
   }

   QMainWindow::closeEvent(e);
}

bool BatchEditorDlg::promptForSave()
{
   mpFilesetWidget->acceptEditedValues(true);

   if (mbModified == true)
   {
      QString strFilename = "Untitled";
      if (mXmlFilename.isEmpty() == false)
      {
         strFilename = mXmlFilename;
      }

      int iReturn = -1;
      iReturn = QMessageBox::warning(this, "Batch Wizard Editor", "Do you want to save changes to " +
         strFilename + "?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
      if (iReturn == QMessageBox::Yes)
      {
         bool bSuccess = false;
         bSuccess = save();
         if (bSuccess == false)
         {
            return false;
         }
      }
      else if (iReturn == QMessageBox::Cancel)
      {
         return false;
      }
   }

   return true;
}

BatchWizard* BatchEditorDlg::readWizardFile(const QString& strWizardFilename)
{
   BatchWizard* pBatchWizard = WizardUtilities::createBatchWizardFromWizard(strWizardFilename.toStdString());
   if (pBatchWizard == NULL)
   {
      QMessageBox::critical(this, "Batch Wizard Editor", "Could not read the wizard in the file!");
   }

   return pBatchWizard;
}

void BatchEditorDlg::updateViewWidgets()
{
   QString strView;
   QStringList viewObjects;

   mpViewCombo->clear();

   if (mpFilesetAction->isChecked() == true)
   {
      strView = "Fileset:";
      viewObjects = getFilesetNames();
   }
   else if (mpWizardAction->isChecked() == true)
   {
      strView = "Wizard:";

      vector<BatchWizard*>::iterator iter;
      for (iter = mWizards.begin(); iter != mWizards.end(); iter++)
      {
         BatchWizard* pWizard = NULL;
         pWizard = *iter;
         if (pWizard != NULL)
         {
            string wizardFilename = pWizard->getWizardFilename();
            if (wizardFilename.empty() == false)
            {
               viewObjects.append(QString::fromStdString(wizardFilename));
            }
         }
      }
   }

   mpViewLabel->setText(strView);
   mpViewCombo->addItems(viewObjects);
   mpViewCombo->setCurrentIndex(0);
   setWidgetValues(0);
}

void BatchEditorDlg::newWizard()
{
   // Save the current file if necessary
   bool bSaved = false;
   bSaved = promptForSave();
   if (bSaved == true)
   {
      setBatchWizard(QString());
   }
}

void BatchEditorDlg::open()
{
   // Save the current file if necessary
   bool bSaved = false;
   bSaved = promptForSave();
   if (bSaved == false)
   {
      return;
   }

   // Get the XML filename from the user
   QString strDirectory;
   const Filename* pWizardPath = ConfigurationSettings::getSettingWizardPath();
   if (pWizardPath != NULL)
   {
      strDirectory = QString::fromStdString(pWizardPath->getFullPathAndName());
   }
   QString strFilename = QFileDialog::getOpenFileName(this, QString(), strDirectory,
      "Batch Wizard Files (*.batchwiz);;All Files (*)");
   if (strFilename.isEmpty() == false)
   {
      // Populate the widgets from the file
      setBatchWizard(strFilename);
   }
}

bool BatchEditorDlg::save()
{
   mpFilesetWidget->acceptEditedValues(true);

   bool bSuccess = false;
   if (mXmlFilename.isEmpty() == false)
   {
      if (mbModified == false)
      {
         return true;
      }

      if (mWizards.empty() == true)
      {
         QMessageBox::critical(this, "Batch Wizard Editor", "No wizards have been specified to execute.  "
            "The batch file will not be saved!");
         return false;
      }

      bSuccess = WizardUtilities::writeBatchWizard(mWizards, mXmlFilename.toStdString());
      if (bSuccess == true)
      {
         mbModified = false;
      }
      else
      {
         QMessageBox::critical(this, "Batch Wizard Editor", "Can not save the batch wizard file: '" +
            mXmlFilename + "'.");
      }
   }
   else
   {
      bSuccess = saveAs();
   }

   return bSuccess;
}

bool BatchEditorDlg::saveAs()
{
   mpFilesetWidget->acceptEditedValues(true);

   // Get the default wizard directory
   QString strDefaultDir = QDir::currentPath();
   if (mXmlFilename.isEmpty() == true)
   {
      const Filename* pWizardPath = ConfigurationSettings::getSettingWizardPath();
      if (pWizardPath != NULL)
      {
         strDefaultDir = QString::fromStdString(pWizardPath->getFullPathAndName());
      }
   }
   else
   {
      QFileInfo fileInfo = QFileInfo(mXmlFilename);
      strDefaultDir = fileInfo.absolutePath();
   }

   // Create a default filename
   QString strDefaultFile;
   if (mXmlFilename.isEmpty() == false)
   {
      QFileInfo fileInfo(mXmlFilename);
      strDefaultFile = fileInfo.completeBaseName();
   }

   if (strDefaultFile.isEmpty() == false)
   {
      strDefaultFile += ".batchwiz";
   }

   // Invoke a file selection dialog and get the save file name
   QFileDialog dlg(this, "Save Batch Wizard", strDefaultDir, "Batch Wizard Files (*.batchwiz);;All Files (*)");
   dlg.setAcceptMode(QFileDialog::AcceptSave);
   dlg.setFileMode(QFileDialog::AnyFile);
   dlg.setConfirmOverwrite(true);
   dlg.setDefaultSuffix("batchwiz");
   dlg.selectFile(strDefaultFile);

   QString strFilename;
   if (dlg.exec() == QDialog::Accepted)
   {
      strFilename = dlg.selectedFiles().front();
   }
   if (strFilename.isEmpty() == true)
   {
      return false;
   }

   QString strCurrentFilename = mXmlFilename;
   mXmlFilename = strFilename;
   mbModified = true;

   // Save the file
   bool bSuccess = false;
   bSuccess = save();
   if (bSuccess == true)
   {
      setWindowTitle(APP_NAME " Batch Wizard Editor - " + strFilename);
   }
   else
   {
      mXmlFilename = strCurrentFilename;
   }

   return bSuccess;
}

bool BatchEditorDlg::executeBatch()
{
   bool bSuccess = false;
   bSuccess = execute(true);

   return bSuccess;
}

bool BatchEditorDlg::executeInteractive()
{
   bool bSuccess = false;
   bSuccess = execute(false);

   return bSuccess;
}

void BatchEditorDlg::add()
{
   if (mpFilesetAction->isChecked() == true)
   {
      bool bSuccess = false;

      QString strFileset = QInputDialog::getText(this, "Add File Set", "File set name:",
         QLineEdit::Normal, QString(), &bSuccess);
      if ((bSuccess == true) && (strFileset.isEmpty() == false))
      {
         string filesetName = strFileset.toStdString();

         // Do not add the file set if it already exists in the batch file
         for (unsigned int i = 0; i < mFilesets.size(); i++)
         {
            BatchFileset* pCurrentFileset = mFilesets.at(i);
            if (pCurrentFileset != NULL)
            {
               string currentName = pCurrentFileset->getName();
               if (currentName == filesetName)
               {
                  QMessageBox::critical(this, "Batch Wizard Editor", "The new file set '" +
                     strFileset + "' is already included in the batch file!");
                  return;
               }
            }
         }

         BatchFileset* pFileset = new BatchFileset();
         if (pFileset != NULL)
         {
            pFileset->setName(filesetName);
            mFilesets.push_back(pFileset);

            // Set the file set in the wizards
            vector<BatchWizard*>::iterator iter;
            for (iter = mWizards.begin(); iter != mWizards.end(); iter++)
            {
               BatchWizard* pWizard = NULL;
               pWizard = *iter;
               if (pWizard != NULL)
               {
                  pWizard->addFileset(pFileset);
               }
            }

            // Activate the new file set
            int iCount = mpViewCombo->count();
            mpViewCombo->addItem(strFileset);
            mpViewCombo->setCurrentIndex(iCount);
            setWidgetValues(iCount);

            mbModified = true;
         }
      }
   }
   else if (mpWizardAction->isChecked() == true)
   {
      QString strDirectory;
      const Filename* pWizardPath = ConfigurationSettings::getSettingWizardPath();
      if (pWizardPath != NULL)
      {
         strDirectory = QString::fromStdString(pWizardPath->getFullPathAndName());
      }
      QString strWizardFile = QFileDialog::getOpenFileName(this, "Select Wizard File", strDirectory,
         "Wizard Files (*.wiz)");
      if (strWizardFile.isEmpty() == false)
      {
         // Do not add the wizard file if it already exists in the batch file
         for (int i = 0; i < mpViewCombo->count(); i++)
         {
            QString strCurrentFile = mpViewCombo->itemText(i);
            if (strCurrentFile == strWizardFile)
            {
               QMessageBox::critical(this, "Batch Wizard Editor", "The selected wizard file '" +
                  strWizardFile + "' is already included in the batch file!");
               return;
            }
         }

         BatchWizard* pBatchWizard = NULL;
         pBatchWizard = readWizardFile(strWizardFile);
         if (pBatchWizard != NULL)
         {
            mWizards.push_back(pBatchWizard);

            // Add the file sets to the wizard
            vector<BatchFileset*>::iterator iter;
            for (iter = mFilesets.begin(); iter != mFilesets.end(); iter++)
            {
               BatchFileset* pFileset = NULL;
               pFileset = *iter;
               if (pFileset != NULL)
               {
                  pBatchWizard->addFileset(pFileset);
               }
            }

            int iCount = mpViewCombo->count();
            mpViewCombo->addItem(strWizardFile);
            mpViewCombo->setCurrentIndex(iCount);
            setWidgetValues(iCount);

            mbModified = true;
         }
      }
   }
}

void BatchEditorDlg::remove()
{
   QString strName = mpViewCombo->currentText();
   if (strName.isEmpty() == true)
   {
      return;
   }

   bool bRemoved = false;
   QString strView = "item";

   if (mpFilesetAction->isChecked() == true)
   {
      strView = "file set";
      string filesetName = strName.toStdString();

      vector<BatchFileset*>::iterator iter;
      for (iter = mFilesets.begin(); iter != mFilesets.end(); iter++)
      {
         BatchFileset* pFileset = NULL;
         pFileset = *iter;
         if (pFileset != NULL)
         {
            string name = pFileset->getName();
            if (name == filesetName)
            {
               // Remove the file set in the wizards
               vector<BatchWizard*>::iterator wizardIter;
               for (wizardIter = mWizards.begin(); wizardIter != mWizards.end(); wizardIter++)
               {
                  BatchWizard* pWizard = NULL;
                  pWizard = *wizardIter;
                  if (pWizard != NULL)
                  {
                     pWizard->removeFileset(pFileset);
                  }
               }

               delete pFileset;
               mFilesets.erase(iter);
               bRemoved = true;
               break;
            }
         }
      }
   }
   else if (mpWizardAction->isChecked() == true)
   {
      strView = "wizard";
      string wizardFilename = strName.toStdString();

      vector<BatchWizard*>::iterator iter;
      for (iter = mWizards.begin(); iter != mWizards.end(); iter++)
      {
         BatchWizard* pWizard = NULL;
         pWizard = *iter;
         if (pWizard != NULL)
         {
            string filename = pWizard->getWizardFilename();
            if (filename == wizardFilename)
            {
               delete pWizard;
               mWizards.erase(iter);
               bRemoved = true;
               break;
            }
         }
      }
   }

   if (bRemoved == true)
   {
      // Update the combo box
      mpViewCombo->removeItem(mpViewCombo->currentIndex());

      int iIndex = mpViewCombo->currentIndex();
      setWidgetValues(iIndex);

      // Set the modified flag
      mbModified = true;
   }
   else
   {
      QMessageBox::critical(this, "Batch Wizard Editor", "Can not remove the current " + strView + "!");
   }
}

void BatchEditorDlg::setView(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   mpFilesetWidget->acceptEditedValues(true);

   if (pAction == mpFilesetAction)
   {
      mpStack->setCurrentWidget(mpFilesetWidget);
   }
   else if (pAction == mpWizardAction)
   {
      mpStack->setCurrentWidget(mpWizardWidget);
   }

   updateViewWidgets();
}

void BatchEditorDlg::setWidgetValues(int iIndex)
{
   if (mpFilesetAction->isChecked() == true)
   {
      mpFilesetWidget->acceptEditedValues(true);

      int iFilesetCount = 0;
      iFilesetCount = mFilesets.size();

      BatchFileset* pFileset = NULL;
      if ((iIndex < iFilesetCount) && (iIndex >= 0) && (iFilesetCount > 0))
      {
         pFileset = mFilesets.at(iIndex);
      }

      mpFilesetWidget->setActiveFileset(pFileset);
   }
   else if (mpWizardAction->isChecked() == true)
   {
      int iWizardCount = 0;
      iWizardCount = mWizards.size();

      BatchWizard* pWizard = NULL;
      if ((iIndex < iWizardCount) && (iIndex >= 0) && (iWizardCount > 0))
      {
         pWizard = mWizards.at(iIndex);
      }

      QStringList filesetNames = getFilesetNames();
      mpWizardWidget->setActiveWizard(pWizard, filesetNames);
   }
}

void BatchEditorDlg::setModified()
{
   mbModified = true;
}

QStringList BatchEditorDlg::getFilesetNames() const
{
   QStringList filesetNames;

   vector<BatchFileset*>::const_iterator iter;
   for (iter = mFilesets.begin(); iter != mFilesets.end(); iter++)
   {
      BatchFileset* pFileset = NULL;
      pFileset = *iter;
      if (pFileset != NULL)
      {
         string filesetName = pFileset->getName();
         if (filesetName.empty() == false)
         {
            filesetNames.append(QString::fromStdString(filesetName));
         }
      }
   }

   return filesetNames;
}

bool BatchEditorDlg::execute(bool bBatch)
{
   // Save the batch wizard to ensure proper execution
   bool bSaved = false;
   bSaved = save();
   if (bSaved == false)
   {
      return false;
   }

   if (mXmlFilename.isEmpty() == true)
   {
      QMessageBox::critical(this, "Batch Wizard Editor", "The batch wizard has not yet been saved.  "
         "The wizard will not be executed.");
      return false;
   }

   QString strExecutable;
#if defined(WIN_API)
   QString strInput = "/input:\"" + mXmlFilename + "\"";
#else
   QString strInput = "'-input:" + mXmlFilename + "'";
#endif

   string appHome = Service<ConfigurationSettings>()->getHome();
   if (!appHome.empty())
   {
      strExecutable = QString::fromStdString(appHome);

#if defined(WIN_API)
      if (bBatch == false)
      {
         strExecutable += "/Bin/Opticks.exe";
      }
      else if (bBatch == true)
      {
         strExecutable += "/Bin/OpticksBatch.exe";
      }
#else
      if (bBatch == false)
      {
         strExecutable += "/Bin/Opticks";
      }
      else if (bBatch == true)
      {
         strExecutable += "/Bin/OpticksBatch";
      }
#endif

      strExecutable.replace(QRegExp("\\\\"), "/");
   }

   if (strExecutable.isEmpty() == true)
   {
      return false;
   }

   bool bSuccess = false;

#if defined(WIN_API)
   HINSTANCE hinst = ShellExecute(NULL, NULL, strExecutable.toStdString().c_str(), strInput.toStdString().c_str(),
      NULL, SW_SHOWNORMAL);

   // Handle less than 32 indicates failure
   bSuccess = reinterpret_cast<int>(hinst) > 32;
#else
   QString command = strExecutable + " " + strInput + "&";
   system(command.toStdString().c_str());
   bSuccess = true;
#endif

   if (bSuccess == false)
   {
      QMessageBox::critical(this, "Batch Wizard Editor", "The batch wizard failed to execute!");
   }

   return bSuccess;
}

void BatchEditorDlg::destroyFilesets()
{
   vector<BatchFileset*>::iterator iter;
   for (iter = mFilesets.begin(); iter != mFilesets.end(); iter++)
   {
      BatchFileset* pFileset = NULL;
      pFileset = *iter;
      if (pFileset != NULL)
      {
         delete pFileset;
      }
   }

   mFilesets.clear();
}

void BatchEditorDlg::destroyWizards()
{
   vector<BatchWizard*>::iterator iter;
   for (iter = mWizards.begin(); iter != mWizards.end(); iter++)
   {
      BatchWizard* pWizard = NULL;
      pWizard = *iter;
      if (pWizard != NULL)
      {
         delete pWizard;
      }
   }

   mWizards.clear();
}
