/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>

#include "ApplicationServices.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "Importer.h"
#include "MetadataFilterDlg.h"
#include "ModelServices.h"
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
#include "SignatureSelector.h"
#include "SignatureSet.h"
#include "UtilityServices.h"

#include <string>
using namespace std;

SignatureSelector::SignatureSelector(Progress* pProgress, QWidget* parent,
                                     QAbstractItemView::SelectionMode mode,
                                     bool addApply,
                                     const string& customButtonLabel) :
   QDialog(parent),
   mpProgress(pProgress),
   mpApplyButton(NULL),
   mpCustomButton(NULL),
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

   if (addApply)
   {
      mpApplyButton = new QPushButton("&Apply", this);
      connect(mpApplyButton, SIGNAL(clicked()), this, SLOT(apply()));
      mpApplyButton->setEnabled(false);
   }

   if (customButtonLabel.empty() == false)
   {
      mpCustomButton = new QPushButton(QString::fromStdString(customButtonLabel), this);
      VERIFYNRV(connect(mpCustomButton, SIGNAL(clicked()), this, SLOT(customButtonClicked())));
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

   if (mpCustomButton != NULL)
   {
      pButtonLayout->addWidget(mpCustomButton);
   }

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

   QIcon icnBrowse(":/icons/Open");

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

void SignatureSelector::enableCustomButton(bool enable)
{
   if (mpCustomButton != NULL)
   {
      mpCustomButton->setEnabled(enable);
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

bool SignatureSelector::searchForMetadata(const DynamicObject* pMetadata, const QRegExp& nameFilter,
                                          const QRegExp& valueFilter)
{
   if (pMetadata == NULL)
   {
      return false;
   }

   // Return true if no metadata search is specified
   if ((nameFilter.isEmpty() == true) && (valueFilter.isEmpty() == true))
   {
      return true;
   }

   // Search this object
   const DynamicObjectExt1* pMetadataExt = dynamic_cast<const DynamicObjectExt1*>(pMetadata);
   if (pMetadataExt == NULL)
   {
      return false;
   }

   const DataVariant& attribute = pMetadataExt->findFirstOf(nameFilter, valueFilter);
   if (attribute.isValid() == true)
   {
      return true;
   }

   // Search this object's children
   vector<string> attributeNames;
   pMetadata->getAttributeNames(attributeNames);

   for (vector<string>::iterator iter = attributeNames.begin(); iter != attributeNames.end(); ++iter)
   {
      string attributeName = *iter;
      if (attributeName.empty() == false)
      {
         const DataVariant& attributeValue = pMetadata->getAttribute(attributeName);
         if ((attributeValue.isValid() == true) &&
            (attributeValue.getTypeName() == TypeConverter::toString<DynamicObject>()))
         {
            const DynamicObject* pChild = attributeValue.getPointerToValue<DynamicObject>();
            if (searchForMetadata(pChild, nameFilter, valueFilter) == true)
            {
               return true;
            }
         }
      }
   }

   return false;
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
      QString filterText = "All Signatures";

      MetadataFilterDlg filterDlg(this);
      if (filterDlg.exec() == QDialog::Accepted)
      {
         // Name
         QRegExp nameFilter = filterDlg.getNameFilter();

         QString name = nameFilter.pattern();
         if (name.isEmpty() == false)
         {
            filterText = "Metadata: Name '" + name + "'";

            // Add wildcard and case sensitivity to filter text
            if (nameFilter.patternSyntax() == QRegExp::Wildcard)
            {
               filterText += " (wildcard, ";
            }
            else
            {
               filterText += " (exact match, ";
            }

            if (nameFilter.caseSensitivity() == Qt::CaseSensitive)
            {
               filterText += "case sensitive)";
            }
            else
            {
               filterText += "case insensitive)";
            }
         }

         // Value
         QRegExp valueFilter = filterDlg.getValueFilter();

         QString value = valueFilter.pattern();
         if (value.isEmpty() == false)
         {
            if (name.isEmpty() == true)
            {
               filterText = "Metadata: ";
            }
            else
            {
               filterText += ",  ";
            }

            filterText += "Value '" + value + "'";

            // Add wildcard and case sensitivity to filter text
            if (valueFilter.patternSyntax() == QRegExp::Wildcard)
            {
               filterText += " (wildcard, ";
            }
            else
            {
               filterText += " (exact match, ";
            }

            if (valueFilter.caseSensitivity() == Qt::CaseSensitive)
            {
               filterText += "case sensitive)";
            }
            else
            {
               filterText += "case insensitive)";
            }
         }
      }

      int iIndex = 0;
      if (filterText != "All Signatures")
      {
         // Do not insert the item if it already exists
         bool bInsert = true;

         int iCount = mpFormatCombo->count();
         for (int i = 0; i < iCount; ++i)
         {
            if (mpFormatCombo->itemText(i) == filterText)
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
            mpFormatCombo->insertItem(iIndex, filterText);
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

   // Get any metadata filter values
   QRegExp nameFilter;
   QRegExp valueFilter;
   QString strFilter = getCurrentFormatType();

   int namePos = strFilter.indexOf("Name");
   int valuePos = strFilter.indexOf("Value");

   if (namePos != -1)
   {
      int startPos = strFilter.indexOf("'", namePos);
      int endPos = strFilter.indexOf("'", startPos + 1);
      if ((startPos != -1) && (endPos != -1))
      {
         ++startPos;
         nameFilter.setPattern(strFilter.mid(startPos, endPos - startPos));
      }

      int wildcardPos = strFilter.indexOf("wildcard", endPos + 1, Qt::CaseSensitive);
      if ((wildcardPos != -1) && ((valuePos == -1) || (wildcardPos < valuePos)))
      {
         nameFilter.setPatternSyntax(QRegExp::Wildcard);
      }
      else
      {
         nameFilter.setPatternSyntax(QRegExp::FixedString);
      }

      int casePos = strFilter.indexOf("case sensitive", endPos + 1, Qt::CaseSensitive);
      if ((casePos != -1) && ((valuePos == -1) || (casePos < valuePos)))
      {
         nameFilter.setCaseSensitivity(Qt::CaseSensitive);
      }
      else
      {
         nameFilter.setCaseSensitivity(Qt::CaseInsensitive);
      }
   }

   if (valuePos != -1)
   {
      int startPos = strFilter.indexOf("'", valuePos);
      int endPos = strFilter.indexOf("'", startPos + 1);
      if ((startPos != -1) && (endPos != -1))
      {
         ++startPos;
         valueFilter.setPattern(strFilter.mid(startPos, endPos - startPos));
      }

      int wildcardPos = strFilter.indexOf("wildcard", endPos + 1, Qt::CaseSensitive);
      if (wildcardPos != -1)
      {
         valueFilter.setPatternSyntax(QRegExp::Wildcard);
      }
      else
      {
         valueFilter.setPatternSyntax(QRegExp::FixedString);
      }

      int casePos = strFilter.indexOf("case sensitive", endPos + 1, Qt::CaseSensitive);
      if (casePos != -1)
      {
         valueFilter.setCaseSensitivity(Qt::CaseSensitive);
      }
      else
      {
         valueFilter.setCaseSensitivity(Qt::CaseInsensitive);
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
         const DynamicObject* pMetadata = pSignature->getMetadata();
         if (pMetadata != NULL)
         {
            if (searchForMetadata(pMetadata, nameFilter, valueFilter) == true)
            {
               addSignatureItem(pSignature);
            }
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
         const DynamicObject* pMetadata = pSignature->getMetadata();
         if (pMetadata != NULL)
         {
            if (searchForMetadata(pMetadata, nameFilter, valueFilter) == true)
            {
               addSignatureItem(pSignature);
            }
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

   // invoke the dialog
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
   vector<string> filesToLoad;
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
               filesToLoad.push_back(currentFilename);
            }
         }
      }
   }

   if (filesToLoad.empty() == true)
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

   ImporterResource importer("Auto Importer", filesToLoad, mpProgress);
   if (importer->execute() == true)
   {
      updateSignatureList();
   }
}

void SignatureSelector::customButtonClicked()
{
   // Overload this method in derived classes
   QMessageBox::warning(this, "Custom Button Clicked", "No actions have been setup for this button");
}
