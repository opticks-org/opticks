/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtGui/QBitmap>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>

#include "SignatureSelector.h"
#include "ApplicationServices.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "IconImages.h"
#include "Importer.h"
#include "ModelServices.h"
#include "NameValueDlg.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugIn.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "SearchDlg.h"
#include "Signature.h"
#include "SignaturePropertiesDlg.h"
#include "SignatureSet.h"
#include "UtilityServices.h"

#include <string>
using namespace std;

SignatureSelector::SignatureSelector(Progress* pProgress, QWidget* parent,
                                     QAbstractItemView::SelectionMode mode,
                                     bool addApply) :
   QDialog(parent),
   mpProgress(pProgress),
   mpSearchDlg(NULL)
{
   // Display format
   QLabel* pSignaturesLabel = new QLabel("Signatures:", this);
   mpFormatCombo = new QComboBox(this);
   mpFormatCombo->setEditable(false);
   mpFormatCombo->addItem("All Signatures");
   mpFormatCombo->addItem("-----------------------");
   mpFormatCombo->addItem("Metadata...");

   // Signature list
   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Location");

   mpSignatureList = new QTreeWidget(this);
   mpSignatureList->setColumnCount(columnNames.count());
   mpSignatureList->setHeaderLabels(columnNames);
   mpSignatureList->setSelectionMode(mode);
   mpSignatureList->setRootIsDecorated(true);

   QHeaderView* pHeader = mpSignatureList->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->resizeSection(0, 125);
   }

   // Buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);
   mpPropertiesButton = new QPushButton("&Properties", this);
   mpExportButton = new QPushButton("&Export", this);
   mpUnloadButton = new QPushButton("&Unload", this);
   mpImportButton = new QPushButton("&Import >>", this);

   mpApplyButton = NULL;
   if (addApply)
   {
      mpApplyButton = new QPushButton("&Apply", this);
      connect(mpApplyButton, SIGNAL(clicked()), this, SLOT(apply()));
      mpApplyButton->setEnabled(false);
   }

   QVBoxLayout* pButtonLayout = new QVBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addWidget(pOkButton);

   if (mpApplyButton != NULL)
   {
      pButtonLayout->addWidget(mpApplyButton);
   }

   pButtonLayout->addWidget(pCancelButton);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(mpPropertiesButton);
   pButtonLayout->addWidget(mpExportButton);
   pButtonLayout->addWidget(mpUnloadButton);
   pButtonLayout->addWidget(mpImportButton);

   // Empty layout
   mpEmptyLayout = new QGridLayout();

   // Import widget
   mpImportWidget = new QWidget(this);

   QFrame* pLine = new QFrame(mpImportWidget);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QLabel* pFilesLabel = new QLabel("Import Files:", mpImportWidget);
   mpFilesList = new QListWidget(mpImportWidget);
   mpFilesList->setMinimumHeight(110);
   mpFilesList->setSelectionMode(QAbstractItemView::ExtendedSelection);

   QPixmap pixOpen(IconImages::OpenIcon);
   pixOpen.setMask(pixOpen.createHeuristicMask());
   QIcon icnBrowse(pixOpen);

   QPushButton* pBrowseButton = new QPushButton(icnBrowse, "&Browse...", mpImportWidget);
   QPushButton* pSearchButton = new QPushButton("&Search...", mpImportWidget);
   QPushButton* pLoadButton = new QPushButton("&Load", mpImportWidget);

   // Import widget layout
   QVBoxLayout* pImportFileLayout = new QVBoxLayout();
   pImportFileLayout->setMargin(0);
   pImportFileLayout->setSpacing(5);
   pImportFileLayout->addWidget(pFilesLabel);
   pImportFileLayout->addWidget(mpFilesList, 10);

   QVBoxLayout* pImportButtonLayout = new QVBoxLayout();
   pImportButtonLayout->setMargin(0);
   pImportButtonLayout->setSpacing(5);
   pImportButtonLayout->addWidget(pBrowseButton);
   pImportButtonLayout->addWidget(pSearchButton);
   pImportButtonLayout->addStretch(10);
   pImportButtonLayout->addWidget(pLoadButton);

   QGridLayout* pImportGrid = new QGridLayout(mpImportWidget);
   pImportGrid->setMargin(10);
   pImportGrid->setSpacing(10);
   pImportGrid->addWidget(pLine, 0, 0, 1, 2);
   pImportGrid->addLayout(pImportFileLayout, 1, 0);
   pImportGrid->addLayout(pImportButtonLayout, 1, 1);
   pImportGrid->setRowStretch(1, 10);
   pImportGrid->setColumnStretch(0, 10);

   // Search dialog
   mpSearchDlg = new SearchDlg(mpProgress, this);

   // Dialog layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addWidget(pSignaturesLabel, 0, 0);
   pGrid->addWidget(mpFormatCombo, 0, 1);
   pGrid->addWidget(mpSignatureList, 1, 0, 1, 2);
   pGrid->addLayout(mpEmptyLayout, 2, 0, 1, 2);
   pGrid->addLayout(pButtonLayout, 0, 2, 3, 1);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   setWindowTitle("Signature Selector");
   setModal(true);
   setOrientation(Qt::Vertical);
   setExtension(mpImportWidget);
   resize(470, 215);

   vector<PlugInDescriptor*> importerPlugIns = mpManager->getPlugInDescriptors(PlugInManagerServices::ImporterType());
      
   
   for (vector<PlugInDescriptor*>::iterator iter = importerPlugIns.begin();
        iter != importerPlugIns.end(); ++iter)
   {
      PlugInDescriptor* pDescriptor = *iter;
      if (pDescriptor == NULL)
      {
         continue;
      }
      string subtype = pDescriptor->getSubtype();
      if ((subtype == "Signature") || (subtype == "Signature Set"))
      {
         string filters = pDescriptor->getFileExtensions();
         QString strFilter = QString::fromStdString(filters);
         QStringList currentFilters = strFilter.split(";;");

         mImporterFilters += currentFilters;
      }
   }

   mpSearchDlg->setTypes(mImporterFilters);
   updateSignatureList();

   // Connections
   connect(mpFormatCombo, SIGNAL(activated(const QString&)), this, SLOT(setDisplayType(const QString&)));
   connect(mpSignatureList, SIGNAL(itemSelectionChanged()), this, SLOT(enableButtons()));
   connect(mpSignatureList, SIGNAL(itemSelectionChanged()), this, SIGNAL(selectionChanged()));
   connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
   connect(mpPropertiesButton, SIGNAL(clicked()), this, SLOT(displaySignatureProperties()));
   connect(mpExportButton, SIGNAL(clicked()), this, SLOT(exportSignatures()));
   connect(mpUnloadButton, SIGNAL(clicked()), this, SLOT(unloadSignatures()));
   connect(mpImportButton, SIGNAL(clicked()), this, SLOT(importSignatures()));
   connect(pBrowseButton, SIGNAL(clicked()), this, SLOT(browseFiles()));
   connect(pSearchButton, SIGNAL(clicked()), this, SLOT(searchDirectories()));
   connect(pLoadButton, SIGNAL(clicked()), this, SLOT(loadSignatures()));
}

SignatureSelector::~SignatureSelector()
{
}

void SignatureSelector::enableApplyButton(bool enable)
{
   if (mpApplyButton != NULL)
   {
      mpApplyButton->setEnabled(enable);
   }
}

bool SignatureSelector::isApplyButtonEnabled() const
{
   bool isEnabled = false;
   if (mpApplyButton != NULL)
   {
      isEnabled = mpApplyButton->isEnabled();
   }
   return isEnabled;
}

vector<Signature*> SignatureSelector::getSignatures() const
{
   vector<Signature*> signatures;

   map<QTreeWidgetItem*, Signature*>::const_iterator iter = mLoadedSignatures.begin();
   while (iter != mLoadedSignatures.end())
   {
      QTreeWidgetItem* pItem = iter->first;
      if (pItem != NULL)
      {
         bool bSelected = mpSignatureList->isItemSelected(pItem);
         if (bSelected == true)
         {
            Signature* pSignature = iter->second;
            if (pSignature != NULL)
            {
               signatures.push_back(pSignature);
            }
         }
      }

      iter++;
   }

   return signatures;
}

static void extractFromSigSets(const vector<Signature*>& sourceSigs, vector<Signature*>& destSigs)
{
   vector<Signature*>::const_iterator ppSig;
   for (ppSig = sourceSigs.begin(); ppSig != sourceSigs.end(); ++ppSig)
   {
      if ((*ppSig)->isKindOf("SignatureSet"))
      {
         const vector<Signature*>& subSigs = ((SignatureSet*)(*ppSig))->getSignatures();
         extractFromSigSets(subSigs, destSigs);
      }
      else
      {
         destSigs.push_back(*ppSig);
      }
   }
}

vector<Signature*> SignatureSelector::getExtractedSignatures() const
{
   vector<Signature*> sigs = getSignatures();
   vector<Signature*> extractedSigs;
   extractFromSigSets(sigs, extractedSigs);
   return extractedSigs;
}

void SignatureSelector::abortSearch()
{
   if (mpSearchDlg != NULL)
   {
      mpSearchDlg->abortSearch();
   }
}

void SignatureSelector::addCustomType(const QString &type)
{
   if (!type.isEmpty() && mpFormatCombo->findText(type) == -1)
   {
      mpFormatCombo->addItem(type);
   }
}

QString SignatureSelector::getCurrentFormatType() const
{
   return mpFormatCombo->currentText();
}

QTreeWidget *SignatureSelector::getSignatureList() const
{
   return mpSignatureList;
}

QGridLayout* SignatureSelector::getLayout() const
{
   return mpEmptyLayout;
}

void SignatureSelector::setNameText(const QString& strName)
{
   if (strName.isEmpty() == false)
   {
      QTreeWidgetItem* pHeaderItem = mpSignatureList->headerItem();
      if (pHeaderItem != NULL)
      {
         pHeaderItem->setText(0, strName);
      }
   }
}

int SignatureSelector::getNumSelectedSignatures() const
{
   QList<QTreeWidgetItem*> selectedItems = mpSignatureList->selectedItems();
   return selectedItems.count();
}

vector<Signature*> SignatureSelector::loadSignatures(const QStringList& sigFilenames, Progress* pProgress)
{
   vector<Signature*> signatures;

   int iFiles = 0;
   iFiles = sigFilenames.count();
   if (iFiles > 0)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Loading signatures...", 0, NORMAL);
      }

      for (int i = 0; i < iFiles; i++)
      {
         QString strFilename = sigFilenames[i];
         if (strFilename.isEmpty() == true)
         {
            continue;
         }

         ImporterResource importer("Auto Importer", strFilename.toStdString(), pProgress);

         bool bSuccess = importer->execute();
         if (bSuccess == true)
         {
            vector<DataElement*> currentSignatures = importer->getImportedElements();

            vector<DataElement*>::iterator iter;
            for (iter = currentSignatures.begin(); iter != currentSignatures.end(); ++iter)
            {
               Signature* pSignature = dynamic_cast<Signature*>(*iter);
               if (pSignature != NULL)
               {
                  signatures.push_back(pSignature);
               }
            }
         }
         else if (pProgress != NULL)
         {
            string message = "Could not load the signature from the file:\n";
            message += strFilename.toStdString();

            pProgress->updateProgress(message, i * 100 / iFiles, WARNING);
         }

         if (pProgress != NULL)
         {
            pProgress->updateProgress("Loading signatures...", i * 100 / iFiles, NORMAL);
         }
      }

      if (pProgress != NULL)
      {
         pProgress->updateProgress("Loading signatures complete!", 100, NORMAL);
      }
   }

   return signatures;
}

QTreeWidgetItem* SignatureSelector::addSignatureItem(Signature* pSignature, QTreeWidgetItem* pParentItem)
{
   if (pSignature == NULL)
   {
      return NULL;
   }

   // Do not add a single signature if it is already contained in at least one sig set
   map<QTreeWidgetItem*, Signature*>::iterator iter = mLoadedSignatures.begin();
   while (iter != mLoadedSignatures.end())
   {
      Signature* pItemSignature = iter->second;
      if ((pItemSignature == pSignature) && (pParentItem == NULL))
      {
         return NULL;
      }

      iter++;
   }

   // Name
   QString strName;

   string elementName = pSignature->getName();
   if (elementName.empty() == false)
   {
      strName = QString::fromStdString(elementName);
   }

   // Location
   QString strLocation;

   string elementFilename = pSignature->getFilename();
   if (elementFilename.empty() == false)
   {
      strLocation = QString::fromStdString(elementFilename);
   }

   QTreeWidgetItem* pItem = NULL;
   if (pParentItem == NULL)
   {
      pItem = new QTreeWidgetItem(mpSignatureList);
   }
   else
   {
      pItem = new QTreeWidgetItem(pParentItem);
   }

   if (pItem != NULL)
   {
      pItem->setText(0, strName);
      pItem->setText(1, strLocation);

      mLoadedSignatures[pItem] = pSignature;

      SignatureSet* pSignatureSet = dynamic_cast<SignatureSet*>(pSignature);
      if (pSignatureSet != NULL)
      {
         vector<Signature*> signatures = pSignatureSet->getSignatures();

         int iCount = signatures.size();
         for (int i = 0; i < iCount; i++)
         {
            Signature* pCurrentSignature = signatures.at(i);
            if (pCurrentSignature != NULL)
            {
               addSignatureItem(pCurrentSignature, pItem);
            }
         }
      }
   }

   return pItem;
}

bool SignatureSelector::searchForMetadata(Signature* pSignature, const QString& strMetadataName,
                                          const QString& strMetadataValue)
{
   if (pSignature == NULL)
   {
      return false;
   }

   const DynamicObject* pMetadata = NULL;
   pMetadata = pSignature->getMetadata();
   if (pMetadata == NULL)
   {
      return false;
   }

   // Return true if no metadata search is specified
   bool bMatch = true;

   if (strMetadataName.isEmpty() == false)
   {
      const DataVariant& attrValue = pMetadata->getAttribute(strMetadataName.toStdString());
      bMatch = attrValue.isValid();
      if (bMatch == true)
      {
         if (strMetadataValue.isEmpty() == false)
         {
            string curValue = attrValue.toDisplayString();
            bMatch = (curValue == strMetadataValue.toStdString());
         }
      }
   }

   return bMatch;
}

void SignatureSelector::apply()
{
   // Do nothing - provided for derived dialogs
}

void SignatureSelector::setDisplayType(const QString& strFormat)
{
   if (strFormat == "Metadata...")
   {
      // Get the metadata filters from the user
      QString strName;
      QString strValue;

      NameValueDlg metadataDlg(this);
      metadataDlg.setWindowTitle("Metadata Search");

      while (strName.isEmpty() == true)
      {
         int iReturn = metadataDlg.exec();
         if (iReturn == QDialog::Rejected)
         {
            strName.clear();
            break;
         }

         strName = metadataDlg.getName();
         strValue = metadataDlg.getValue();

         if (strName.isEmpty() == true)
         {
            QMessageBox::critical(this, metadataDlg.windowTitle(), "The metadata search must specify a name!");
         }
      }

      QString strText = "All Signatures";
      if (strName.isEmpty() == false)
      {
         strText = "Metadata: Name '" + strName + "'";
      }

      if (strValue.isEmpty() == false)
      {
         if (strName.isEmpty() == true)
         {
            strText = "Metadata: ";
         }
         else
         {
            strText += ",  ";
         }

         strText += "Value '" + strValue + "'";
      }

      int iIndex = 0;
      if (strText != "All Signatures")
      {
         // Do not insert the item if it already exists
         bool bInsert = true;

         int iCount = 0;
         iCount = mpFormatCombo->count();
         for (int i = 0; i < iCount; i++)
         {
            if (mpFormatCombo->itemText(i) == strText)
            {
               bInsert = false;
               break;
            }
         }

         if (bInsert == true)
         {
            // Limit the combo box to five metadata entries
            if (iCount == 8)
            {
               mpFormatCombo->removeItem(5);
               iCount--;
            }

            // Insert the new metadata entry
            iIndex = 1;
            mpFormatCombo->insertItem(iIndex, strText);
         }
      }

      mpFormatCombo->setCurrentIndex(iIndex);
   }
   else if (strFormat == "-----------------------")
   {
      mpFormatCombo->setCurrentIndex(0);
   }

   QString strCurrentFormat = getCurrentFormatType();
   if (strCurrentFormat == "All Signatures")
   {
      mpImportButton->setEnabled(true);
      mpImportWidget->setEnabled(true);
   }
   else
   {
      mpImportButton->setEnabled(false);
      mpImportWidget->setEnabled(false);
   }

   updateSignatureList();
}

void SignatureSelector::enableButtons()
{
   int iSelected = getNumSelectedSignatures();

   mpPropertiesButton->setEnabled(iSelected == 1);
   mpExportButton->setEnabled(iSelected > 0);
   mpUnloadButton->setEnabled(iSelected > 0);
}

void SignatureSelector::displaySignatureProperties()
{
   Signature* pSignature = NULL;

   QList<QTreeWidgetItem*> selectedItems = mpSignatureList->selectedItems();
   for (int i = 0; i < selectedItems.count(); ++i)
   {
      QTreeWidgetItem* pItem = selectedItems[i];
      if (pItem != NULL)
      {
         pSignature = mLoadedSignatures[pItem];
         break;
      }
   }

   if (pSignature != NULL)
   {
      SignaturePropertiesDlg propertiesDlg(pSignature, this);
      propertiesDlg.exec();
   }
   else
   {
      QMessageBox::critical(this, windowTitle(), "Could not get the properties for "
         "the selected signature!");
   }
}

void SignatureSelector::exportSignatures()
{
   ModelResource<SignatureSet> pExportSet("SignatureSelectorExportSet");
   if (pExportSet.get() == NULL)
   {
      return;
   }

   Signature* pSignature = NULL;

   QList<QTreeWidgetItem*> selectedSignatures = mpSignatureList->selectedItems();
   if (selectedSignatures.count() == 1)
   {
      QTreeWidgetItem* pItem = selectedSignatures.front();
      if (pItem != NULL)
      {
         map<QTreeWidgetItem*, Signature*>::iterator iter = mLoadedSignatures.find(pItem);
         if (iter != mLoadedSignatures.end())
         {
            pSignature = iter->second;
         }
      }
   }
   else if (selectedSignatures.count() > 1)
   {
      map<QTreeWidgetItem*, Signature*>::iterator iter;
      for (iter = mLoadedSignatures.begin(); iter != mLoadedSignatures.end(); ++iter)
      {
         QTreeWidgetItem* pItem = iter->first;
         if (pItem != NULL)
         {
            if (mpSignatureList->isItemSelected(pItem))
            {
               SignatureSet* pSet = dynamic_cast<SignatureSet*>(iter->second);
               if (pSet != NULL)
               {
                  pExportSet->insertSignatures(pSet->getSignatures());
               }
               else
               {
                  pExportSet->insertSignature(iter->second);
               }
            }
         }
      }
   }

   Service<DesktopServices> pDesktop;
   if (pExportSet->getNumSignatures() > 0)   // Export a signature set
   {
      pDesktop->exportSessionItem(pExportSet.get(), NULL, mpProgress);
      pExportSet->clear(false);
   }
   else if (pSignature != NULL)              // Export a single signature or signature set
   {
      pDesktop->exportSessionItem(pSignature, NULL, mpProgress);
   }
   else
   {
      QMessageBox::critical(this, windowTitle(), "Please select at least one signature or signature set!");
   }
}

void SignatureSelector::unloadSignatures()
{
   int iDeleted = 0;

   map<QTreeWidgetItem*, Signature*>::iterator iter;
   iter = mLoadedSignatures.begin();
   while (iter != mLoadedSignatures.end())
   {
      QTreeWidgetItem* pItem = iter->first;
      if (pItem != NULL)
      {
         bool bSelected = mpSignatureList->isItemSelected(pItem);
         if (bSelected == true)
         {
            Signature* pSignature = iter->second;
            if (pSignature != NULL)
            {
               bool bDeleted = mpModel->destroyElement(pSignature);
               if (bDeleted == true)
               {
                  iDeleted++;
               }
            }
         }
      }

      ++iter;
   }

   if (iDeleted > 0)
   {
      updateSignatureList();
   }
}

void SignatureSelector::importSignatures()
{
   bool bVisible = false;
   bVisible = mpImportWidget->isVisible();
   if (bVisible == false)
   {
      mpImportButton->setText("&Import <<");
   }
   else if (bVisible == true)
   {
      mpImportButton->setText("&Import >>");
   }

   showExtension(!bVisible);
}

void SignatureSelector::updateSignatureList()
{
   mpSignatureList->clear();
   mLoadedSignatures.clear();
   enableButtons();

   QString strMetadataName;
   QString strMetadataValue;

   // Get any metadata filter values
   QString strFilter = getCurrentFormatType();

   int namePos = strFilter.indexOf("Name");
   if (namePos != -1)
   {
      int startPos = strFilter.indexOf("'", namePos);
      int endPos = strFilter.indexOf("'", startPos + 1);
      if ((startPos != -1) && (endPos != -1))
      {
         ++startPos;
         strMetadataName = strFilter.mid(startPos, endPos - startPos);
      }
   }

   int valuePos = strFilter.indexOf("Value");
   if (valuePos != -1)
   {
      int startPos = strFilter.indexOf("'", valuePos);
      int endPos = strFilter.indexOf("'", startPos + 1);
      if ((startPos != -1) && (endPos != -1))
      {
         ++startPos;
         strMetadataValue = strFilter.mid(startPos, endPos - startPos);
      }
   }

   // Add the signature sets before the signatures so that signatures will appear in the set
   vector<DataElement*> signatureSets = mpModel->getElements("SignatureSet");

   vector<DataElement*>::iterator iter = signatureSets.begin();
   while (iter != signatureSets.end())
   {
      Signature* pSignature = static_cast<Signature*>(*iter);
      if (pSignature != NULL)
      {
         bool bAdd = searchForMetadata(pSignature, strMetadataName, strMetadataValue);
         if (bAdd == true)
         {
            addSignatureItem(pSignature);
         }
      }

      ++iter;
   }

   // Add the signatures
   vector<DataElement*> signatures = mpModel->getElements("Signature");

   iter = signatures.begin();
   while (iter != signatures.end())
   {
      Signature* pSignature = static_cast<Signature*>(*iter);
      if (pSignature != NULL)
      {
         bool bAdd = searchForMetadata(pSignature, strMetadataName, strMetadataValue);
         if (bAdd == true)
         {
            addSignatureItem(pSignature);
         }
      }

      ++iter;
   }

   emit selectionChanged();
}

void SignatureSelector::browseFiles()
{
   // Get the signature filters
   QString strFilters;
   if (mImporterFilters.isEmpty() == false)
   {
      strFilters = mImporterFilters.join(";;");
   }

   // Get the current signature directory
   QString strDirectory;
   const Filename* pWorkingDir = NULL;
   if (ConfigurationSettings::hasSettingPluginWorkingDirectory("Signature"))
   {
      pWorkingDir = ConfigurationSettings::getSettingPluginWorkingDirectory("Signature");
   }
   else
   {
      pWorkingDir = ConfigurationSettings::getSettingImportPath();
   }
   if (pWorkingDir != NULL)
   {
      strDirectory = QString::fromStdString(pWorkingDir->getFullPathAndName());
   }
   QStringList fileList = QFileDialog::getOpenFileNames(this, "Select Signature Files", strDirectory, strFilters);
   if (fileList.isEmpty() == false)
   {
      // Set the current signature directory
      if (fileList.isEmpty() == false)
      {
         QString strFile = fileList.first();
         if (strFile.isEmpty() == false)
         {
            QFileInfo fileInfo(strFile);
            strDirectory = fileInfo.absolutePath();
            if (strDirectory.isEmpty() == false)
            {
               FactoryResource<Filename> pNewWorkingDir;
               pNewWorkingDir->setFullPathAndName(strDirectory.toStdString());
               Service<ConfigurationSettings> pSettings;
               pSettings->setSessionSetting(ConfigurationSettings::getSettingPluginWorkingDirectoryKey("Signature"),
                  *pNewWorkingDir.get());
            }
         }
      }

      // Add the filenames to the list
      mpFilesList->addItems(fileList);
   }
}

void SignatureSelector::searchDirectories()
{
   if (mpSearchDlg == NULL)
   {
      return;
   }

   // Set the initial browse directory in the search dialog to the current signature directory
   QString strDirectory;
   const Filename* pWorkingDir = NULL;
   if (ConfigurationSettings::hasSettingPluginWorkingDirectory("Signature"))
   {
      pWorkingDir = ConfigurationSettings::getSettingPluginWorkingDirectory("Signature");
   }
   else
   {
      pWorkingDir = ConfigurationSettings::getSettingImportPath();
   }
   if (pWorkingDir != NULL)
   {
      strDirectory = QString::fromStdString(pWorkingDir->getFullPathAndName());
   }
   mpSearchDlg->setBrowseDirectory(strDirectory);

   // Invoe the dialog
   int iReturn = -1;
   iReturn = mpSearchDlg->exec();
   if (iReturn == QDialog::Accepted)
   {
      // Add the filenames to the list
      mpFilesList->clear();

      QStringList fileList = mpSearchDlg->getFilenames();
      if (fileList.isEmpty() == false)
      {
         mpFilesList->addItems(fileList);
      }
   }
}

void SignatureSelector::loadSignatures()
{
   QStringList loadList;
   int iNumSelected = 0;

   QList<QListWidgetItem*> selectedItems = mpFilesList->selectedItems();
   for (int i = 0; i < selectedItems.count(); ++i)
   {
      QListWidgetItem* pItem = selectedItems[i];
      if (pItem != NULL)
      {
         iNumSelected++;

         QString strFilename = pItem->text();
         if (strFilename.isEmpty() == false)
         {
            string currentFilename = strFilename.toStdString();

            // Do not load signature if it is already loaded
            bool bLoad = true;
            vector<DataElement*> signatures = mpModel->getElements(currentFilename, "Signature");
            if (signatures.empty() == false)
            {
               bLoad = false;
            }

            vector<DataElement*> signatureSets = mpModel->getElements(currentFilename, "SignatureSet");
            if (signatureSets.empty() == false)
            {
               bLoad = false;
            }

            if (bLoad == true)
            {
               loadList.append(strFilename);
            }
         }
      }
   }

   if (loadList.empty() == true)
   {
      if (iNumSelected == 0)
      {
         QMessageBox::critical(this, windowTitle(), "Please select at least one signature file to load!");
      }
      else
      {
         QMessageBox::critical(this, windowTitle(), "The selected signature files are already loaded!");
      }

      return;
   }

   loadSignatures(loadList, mpProgress);
   updateSignatureList();
}
