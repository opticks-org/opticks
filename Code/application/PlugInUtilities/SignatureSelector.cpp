/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtCore/QMap>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QSplitter>
#include <QtGui/QToolButton>

#include "DesktopServices.h"
#include "DynamicObject.h"
#include "ObjectResource.h"
#include "PlugInDescriptor.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "SearchDlg.h"
#include "Signature.h"
#include "SignatureFilterDlg.h"
#include "SignaturePropertiesDlg.h"
#include "SignatureSelector.h"
#include "SignatureSet.h"

using namespace std;

//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : These special cases for the display format type " \
//   "should be removed when functional compatibility can be broken (dsulgrov)")
const char* SignatureSelector::spMetadataType = "Metadata...";
const char* SignatureSelector::spDashType = "-----------------------";

SignatureSelector::SignatureSelector(Progress* pProgress, QWidget* pParent, QAbstractItemView::SelectionMode mode,
                                     bool addApply, const string& customButtonLabel) :
   QDialog(pParent),
   mpProgress(pProgress),
   mpApplyButton(NULL),
   mpCustomButton(NULL)
{
   // Display format
   mpFormatTree = new QTreeWidget(this);
   mpFormatTree->setContextMenuPolicy(Qt::CustomContextMenu);
   mpFormatTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpFormatTree->setSelectionBehavior(QAbstractItemView::SelectItems);
   mpFormatTree->setEditTriggers(QAbstractItemView::EditKeyPressed);
   mpFormatTree->setRootIsDecorated(true);
   mpFormatTree->setSortingEnabled(false);
   mpFormatTree->setHeaderHidden(true);

   mpSignaturesItem = new QTreeWidgetItem(mpFormatTree);
   mpSignaturesItem->setText(0, "Signatures");
   mpSignaturesItem->setSelected(true);
   mpFormatTree->setCurrentItem(mpSignaturesItem);

   // Signature list
   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Location");

   mpSignatureList = new QTreeWidget(this);
   mpSignatureList->setContextMenuPolicy(Qt::CustomContextMenu);
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

   // Splitter widget
   QSplitter* pSplitter = new QSplitter(Qt::Horizontal, this);
   pSplitter->setOpaqueResize(true);
   pSplitter->insertWidget(0, mpFormatTree);
   pSplitter->insertWidget(1, mpSignatureList);

   QList<int> splitterSizes;
   splitterSizes.append(125);
   splitterSizes.append(325);
   pSplitter->setSizes(splitterSizes);

   // Toolbar
   mpCreateFilterAction = new QAction(QIcon(":/icons/CreateFilter"), "Create Filter", this);
   mpCreateFilterAction->setToolTip("Create Filter");
   mpEditFilterAction = new QAction(QIcon(":/icons/EditFilter"), "Edit Filter", this);
   mpEditFilterAction->setToolTip("Edit Filter");
   mpEditFilterAction->setEnabled(false);
   mpDeleteFilterAction = new QAction(QIcon(":/icons/DeleteFilter"), "Delete Filter", this);
   mpDeleteFilterAction->setToolTip("Delete Filter");
   mpDeleteFilterAction->setEnabled(false);

   mpPropertiesAction = new QAction(QIcon(":/icons/Properties"), "Properties", this);
   mpPropertiesAction->setToolTip("Properties");
   mpExportAction = new QAction(QIcon(":/icons/Save"), "Export Signature(s)", this);
   mpExportAction->setToolTip("Export Signature(s)");
   mpDeleteAction = new QAction(QIcon(":/icons/Delete"), "Delete Signature(s)", this);
   mpDeleteAction->setToolTip("Delete Signature(s)");

   QToolButton* pCreateFilterButton = new QToolButton(this);
   pCreateFilterButton->setAutoRaise(true);
   pCreateFilterButton->setDefaultAction(mpCreateFilterAction);

   QToolButton* pEditFilterButton = new QToolButton(this);
   pEditFilterButton->setAutoRaise(true);
   pEditFilterButton->setDefaultAction(mpEditFilterAction);

   QToolButton* pDeleteFilterButton = new QToolButton(this);
   pDeleteFilterButton->setAutoRaise(true);
   pDeleteFilterButton->setDefaultAction(mpDeleteFilterAction);

   QToolButton* pPropertiesButton = new QToolButton(this);
   pPropertiesButton->setAutoRaise(true);
   pPropertiesButton->setDefaultAction(mpPropertiesAction);

   QToolButton* pExportButton = new QToolButton(this);
   pExportButton->setAutoRaise(true);
   pExportButton->setDefaultAction(mpExportAction);

   QToolButton* pDeleteButton = new QToolButton(this);
   pDeleteButton->setAutoRaise(true);
   pDeleteButton->setDefaultAction(mpDeleteAction);

   QHBoxLayout* pToolBarLayout = new QHBoxLayout();
   pToolBarLayout->setMargin(0);
   pToolBarLayout->setSpacing(0);
   pToolBarLayout->addWidget(pCreateFilterButton);
   pToolBarLayout->addWidget(pEditFilterButton);
   pToolBarLayout->addWidget(pDeleteFilterButton);
   pToolBarLayout->addSpacing(15);
   pToolBarLayout->addWidget(pPropertiesButton);
   pToolBarLayout->addWidget(pExportButton);
   pToolBarLayout->addWidget(pDeleteButton);
   pToolBarLayout->addStretch(10);

   // Empty layout
   mpEmptyLayout = new QGridLayout();

   // Buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);

   if (addApply)
   {
      mpApplyButton = new QPushButton("&Apply", this);
      VERIFYNR(connect(mpApplyButton, SIGNAL(clicked()), this, SLOT(apply())));
      mpApplyButton->setEnabled(false);
   }

   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   if (customButtonLabel.empty() == false)
   {
      mpCustomButton = new QPushButton(QString::fromStdString(customButtonLabel), this);
      VERIFYNR(connect(mpCustomButton, SIGNAL(clicked()), this, SLOT(customButtonClicked())));
   }

   mpImportButton = new QPushButton("&Import >>", this);

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
   pButtonLayout->addWidget(mpImportButton);

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
   pGrid->addWidget(pSplitter, 0, 0);
   pGrid->addLayout(pToolBarLayout, 1, 0);
   pGrid->addLayout(mpEmptyLayout, 2, 0);
   pGrid->addLayout(pButtonLayout, 0, 1, 3, 1);
   pGrid->setRowStretch(0, 10);
   pGrid->setColumnStretch(0, 10);

   // Initialization
   setWindowTitle("Signature Selector");
   setModal(true);
   setOrientation(Qt::Vertical);
   setExtension(mpImportWidget);
   resize(470, 215);

   vector<PlugInDescriptor*> importerPlugIns = mpManager->getPlugInDescriptors(PlugInManagerServices::ImporterType());
   for (vector<PlugInDescriptor*>::const_iterator iter = importerPlugIns.begin(); iter != importerPlugIns.end(); ++iter)
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
   VERIFYNR(connect(mpFormatTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this,
      SLOT(formatChanged())));
   VERIFYNR(connect(mpFormatTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this,
      SLOT(updateSignatureList())));
   VERIFYNR(connect(mpFormatTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(checkFilterName(QTreeWidgetItem*, int))));
   VERIFYNR(connect(mpFormatTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(editFilter())));
   VERIFYNR(connect(mpFormatTree, SIGNAL(customContextMenuRequested(const QPoint&)), this,
      SLOT(displayFormatContextMenu(const QPoint&))));
   VERIFYNR(connect(mpSignatureList, SIGNAL(itemSelectionChanged()), this, SLOT(enableButtons())));
   VERIFYNR(connect(mpSignatureList, SIGNAL(itemSelectionChanged()), this, SIGNAL(selectionChanged())));
   VERIFYNR(connect(mpSignatureList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this,
      SLOT(displaySignatureProperties())));
   VERIFYNR(connect(mpSignatureList, SIGNAL(customContextMenuRequested(const QPoint&)), this,
      SLOT(displaySignatureContextMenu(const QPoint&))));
   VERIFYNR(connect(mpCreateFilterAction, SIGNAL(triggered()), this, SLOT(createFilter())));
   VERIFYNR(connect(mpEditFilterAction, SIGNAL(triggered()), this, SLOT(editFilter())));
   VERIFYNR(connect(mpDeleteFilterAction, SIGNAL(triggered()), this, SLOT(deleteFilter())));
   VERIFYNR(connect(mpPropertiesAction, SIGNAL(triggered()), this, SLOT(displaySignatureProperties())));
   VERIFYNR(connect(mpExportAction, SIGNAL(triggered()), this, SLOT(exportSignatures())));
   VERIFYNR(connect(mpDeleteAction, SIGNAL(triggered()), this, SLOT(unloadSignatures())));
   VERIFYNR(connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject())));
   VERIFYNR(connect(mpImportButton, SIGNAL(clicked()), this, SLOT(importSignatures())));
   VERIFYNR(connect(pBrowseButton, SIGNAL(clicked()), this, SLOT(browseFiles())));
   VERIFYNR(connect(pSearchButton, SIGNAL(clicked()), this, SLOT(searchDirectories())));
   VERIFYNR(connect(pLoadButton, SIGNAL(clicked()), this, SLOT(loadSignatures())));
}

SignatureSelector::~SignatureSelector()
{}

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

   QList<QTreeWidgetItem*> signatureItems = mpSignatureList->selectedItems();
   for (int i = 0; i < signatureItems.count(); ++i)
   {
      QTreeWidgetItem* pItem = signatureItems[i];
      if (pItem != NULL)
      {
         QTreeWidgetItem* pParentItem = pItem->parent();
         if (pParentItem != NULL)
         {
            if (pParentItem->isSelected())
            {
               continue;
            }
         }

         Signature* pSignature = reinterpret_cast<Signature*>(pItem->data(0, Qt::UserRole).value<void*>());
         if (pSignature != NULL)
         {
            signatures.push_back(pSignature);
         }
      }
   }

   return signatures;
}

void SignatureSelector::extractFromSigSets(const vector<Signature*>& sourceSigs, vector<Signature*>& destSigs) const
{
   vector<Signature*>::const_iterator ppSig;
   for (ppSig = sourceSigs.begin(); ppSig != sourceSigs.end(); ++ppSig)
   {
      if ((*ppSig)->isKindOf("SignatureSet"))
      {
         vector<Signature*> subSigs;

         QTreeWidgetItemIterator itemIter(mpSignatureList);
         while (*itemIter != NULL)
         {
            Signature* pItemSignature = reinterpret_cast<Signature*>((*itemIter)->data(0, Qt::UserRole).value<void*>());
            if (pItemSignature == (*ppSig))
            {
               for (int i = 0; i < (*itemIter)->childCount(); ++i)
               {
                  QTreeWidgetItem* pChildItem = (*itemIter)->child(i);
                  if (pChildItem != NULL)
                  {
                     Signature* pChildSignature =
                        reinterpret_cast<Signature*>(pChildItem->data(0, Qt::UserRole).value<void*>());
                     if (pChildSignature != NULL)
                     {
                        subSigs.push_back(pChildSignature);
                     }
                  }
               }

               break;
            }

            ++itemIter;
         }

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

void SignatureSelector::addCustomType(const QString& type)
{
   if (type.isEmpty() == true)
   {
      return;
   }

   for (int i = 0; i < mpFormatTree->topLevelItemCount(); ++i)
   {
      QTreeWidgetItem* pItem = mpFormatTree->topLevelItem(i);
      if (pItem != NULL)
      {
         if (pItem->text(0) == type)
         {
            return;
         }
      }
   }

   QTreeWidgetItem* pItem = new QTreeWidgetItem(mpFormatTree);
   pItem->setText(0, type);
}

QString SignatureSelector::getCurrentFormatType() const
{
   QTreeWidgetItem* pItem = mpFormatTree->currentItem();
   if (pItem != NULL)
   {
      return pItem->text(0);
   }

   return QString();
}

QTreeWidget* SignatureSelector::getSignatureList() const
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

void SignatureSelector::addSignature(Signature* pSignature)
{
   if (pSignature == NULL)
   {
      return;
   }

   bool librarySignatures = false;
   QRegExp signatureNameFilter;
   QRegExp metadataNameFilter;
   QRegExp metadataValueFilter;

   QTreeWidgetItem* pItem = mpFormatTree->currentItem();
   if (pItem != NULL)
   {
      QVariant filter = pItem->data(0, Qt::UserRole);
      QMap<QString, QVariant> filterMap = filter.toMap();
      if (filterMap.isEmpty() == false)
      {
         librarySignatures = filterMap["Library Signatures"].toBool();
         signatureNameFilter = filterMap["Signature Name"].toRegExp();
         metadataNameFilter = filterMap["Metadata Name"].toRegExp();
         metadataValueFilter = filterMap["Metadata Value"].toRegExp();
      }
   }

   if (matchSignature(pSignature, signatureNameFilter, metadataNameFilter, metadataValueFilter) == true)
   {
      createSignatureItem(pSignature, NULL, true);
   }
   else if (librarySignatures == true)
   {
      SignatureSet* pSignatureSet = dynamic_cast<SignatureSet*>(pSignature);
      if (pSignatureSet != NULL)
      {
         createLibraryItems(pSignatureSet, NULL, signatureNameFilter, metadataNameFilter, metadataValueFilter);
      }
   }
}

QTreeWidgetItem* SignatureSelector::createSignatureItem(Signature* pSignature, QTreeWidgetItem* pParentItem,
                                                        bool createLibraryItems)
{
   if (pSignature == NULL)
   {
      return NULL;
   }

   // Name
   QString toolTipName;

   string elementName = pSignature->getDisplayName();
   if (elementName.empty() == true)
   {
      elementName = pSignature->getName();
   }
   else
   {
      toolTipName = QString::fromStdString(pSignature->getName());
   }

   QString strName = QString::fromStdString(elementName);

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

   pItem->setText(0, strName);
   pItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<void*>(pSignature)));
   pItem->setText(1, strLocation);

   if (toolTipName.isEmpty() == false)
   {
      pItem->setToolTip(0, toolTipName);
   }

   if (createLibraryItems == true)
   {
      SignatureSet* pSignatureSet = dynamic_cast<SignatureSet*>(pSignature);
      if (pSignatureSet != NULL)
      {
         vector<Signature*> signatures = pSignatureSet->getSignatures();
         for (vector<Signature*>::const_iterator iter = signatures.begin(); iter != signatures.end(); ++iter)
         {
            Signature* pCurrentSignature = *iter;
            VERIFYNR(pCurrentSignature != NULL);
            createSignatureItem(pCurrentSignature, pItem, createLibraryItems);
         }
      }
   }

   return pItem;
}

void SignatureSelector::createLibraryItems(SignatureSet* pSignatureSet, QTreeWidgetItem* pParentItem,
                                           const QRegExp& signatureNameFilter, const QRegExp& metadataNameFilter,
                                           const QRegExp& metadataValueFilter)
{
   if (pSignatureSet == NULL)
   {
      return;
   }

   QTreeWidgetItem* pSignatureSetItem = NULL;

   vector<Signature*> signatures = pSignatureSet->getSignatures();
   for (vector<Signature*>::const_iterator iter = signatures.begin(); iter != signatures.end(); ++iter)
   {
      Signature* pSignature = *iter;
      VERIFYNR(pSignature != NULL);

      if (matchSignature(pSignature, signatureNameFilter, metadataNameFilter, metadataValueFilter) == true)
      {
         // Create this library's item if necessary
         if (pSignatureSetItem == NULL)
         {
            pSignatureSetItem = createSignatureItem(pSignatureSet, pParentItem);
         }

         // Create the signature item
         createSignatureItem(pSignature, pSignatureSetItem, true);   // If the signature to add is a library,
                                                                     // add all of its child items as well
      }
      else
      {
         // If the signature is a library, check its child signatures
         SignatureSet* pCurrentSignatureSet = dynamic_cast<SignatureSet*>(pSignature);
         if (pCurrentSignatureSet != NULL)
         {
            if (matchLibrarySignatures(pCurrentSignatureSet, signatureNameFilter, metadataNameFilter,
               metadataValueFilter) == true)
            {
               // Create this library's item if necessary
               if (pSignatureSetItem == NULL)
               {
                  pSignatureSetItem = createSignatureItem(pSignatureSet, pParentItem);
               }

               // Create the items in the child library
               createLibraryItems(pCurrentSignatureSet, pSignatureSetItem, signatureNameFilter, metadataNameFilter,
                  metadataValueFilter);
            }
         }
      }
   }
}

bool SignatureSelector::matchLibrarySignatures(const SignatureSet* pSignatureSet, const QRegExp& signatureNameFilter,
                                               const QRegExp& metadataNameFilter, const QRegExp& metadataValueFilter)
{
   if (pSignatureSet == NULL)
   {
      return false;
   }

   // Check if at least one signature in this library or contained libraries matches the filter
   vector<Signature*> signatures = pSignatureSet->getSignatures();
   for (vector<Signature*>::const_iterator iter = signatures.begin(); iter != signatures.end(); ++iter)
   {
      Signature* pSignature = *iter;
      if (pSignature != NULL)
      {
         SignatureSet* pCurrentSignatureSet = dynamic_cast<SignatureSet*>(pSignature);
         if (pCurrentSignatureSet != NULL)
         {
            if (matchLibrarySignatures(pCurrentSignatureSet, signatureNameFilter, metadataNameFilter,
               metadataValueFilter) == true)
            {
               return true;
            }
         }
         else if (matchSignature(pSignature, signatureNameFilter, metadataNameFilter, metadataValueFilter) == true)
         {
            return true;
         }
      }
   }

   return false;
}

bool SignatureSelector::matchSignature(const Signature* pSignature, const QRegExp& signatureNameFilter,
                                       const QRegExp& metadataNameFilter, const QRegExp& metadataValueFilter)
{
   if (pSignature == NULL)
   {
      return false;
   }

   if (signatureNameFilter.isEmpty() == false)
   {
      const string& sigName = pSignature->getName();
      VERIFY(sigName.empty() == false);
      if (signatureNameFilter.exactMatch(QString::fromStdString(sigName)) == false)
      {
         return false;
      }
   }

   if ((metadataNameFilter.isEmpty() == false) || (metadataValueFilter.isEmpty() == false))
   {
      const DynamicObject* pMetadata = pSignature->getMetadata();
      VERIFY(pMetadata != NULL);
      if (matchMetadata(pMetadata, metadataNameFilter, metadataValueFilter) == false)
      {
         return false;
      }
   }

   return true;
}

bool SignatureSelector::matchMetadata(const DynamicObject* pMetadata, const QRegExp& nameFilter,
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
   const DataVariant& attribute = pMetadata->findFirstOf(nameFilter, valueFilter);
   if (attribute.isValid() == true)
   {
      return true;
   }

   // Search this object's children
   vector<string> attributeNames;
   pMetadata->getAttributeNames(attributeNames);

   for (vector<string>::const_iterator iter = attributeNames.begin(); iter != attributeNames.end(); ++iter)
   {
      const string& attributeName = *iter;
      if (attributeName.empty() == false)
      {
         const DataVariant& attributeValue = pMetadata->getAttribute(attributeName);
         if ((attributeValue.isValid() == true) &&
            (attributeValue.getTypeName() == TypeConverter::toString<DynamicObject>()))
         {
            const DynamicObject* pChild = attributeValue.getPointerToValue<DynamicObject>();
            if (matchMetadata(pChild, nameFilter, valueFilter) == true)
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
   if (strFormat == spMetadataType)
   {
      createFilter();
   }
   else if (strFormat == spDashType)
   {
      mpFormatTree->setCurrentItem(mpSignaturesItem);
   }
   else
   {
      QTreeWidgetItemIterator iter(mpFormatTree);
      while (*iter != NULL)
      {
         QTreeWidgetItem* pItem = *iter;
         if (pItem != NULL)
         {
            QString filterName = pItem->text(0);
            if (filterName == strFormat)
            {
               mpFormatTree->setCurrentItem(pItem);
               break;
            }
         }

         ++iter;
      }
   }
}

void SignatureSelector::enableButtons()
{
   QTreeWidgetItem* pItem = mpFormatTree->currentItem();
   if ((pItem != NULL) && (pItem->parent() != NULL))
   {
      pItem = pItem->parent();
   }

   int numSignatures = getNumSelectedSignatures();
   mpPropertiesAction->setEnabled(numSignatures == 1 && pItem == mpSignaturesItem);
   mpExportAction->setEnabled(numSignatures > 0 && pItem == mpSignaturesItem);
   mpDeleteAction->setEnabled(numSignatures > 0 && pItem == mpSignaturesItem);
}

void SignatureSelector::displaySignatureProperties()
{
   QList<QTreeWidgetItem*> selectedItems = mpSignatureList->selectedItems();
   if (selectedItems.count() != 1)
   {
      return;
   }

   Signature* pSignature = NULL;

   QTreeWidgetItem* pItem = selectedItems.front();
   if (pItem != NULL)
   {
      pSignature = reinterpret_cast<Signature*>(pItem->data(0, Qt::UserRole).value<void*>());
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
         pSignature = reinterpret_cast<Signature*>(pItem->data(0, Qt::UserRole).value<void*>());
      }
   }
   else if (selectedSignatures.count() > 1)
   {
      for (int i = 0; i < selectedSignatures.count(); ++i)
      {
         QTreeWidgetItem* pItem = selectedSignatures[i];
         if (pItem != NULL)
         {
            SignatureSet* pSet = dynamic_cast<SignatureSet*>(reinterpret_cast<Signature*>(pItem->data(0,
               Qt::UserRole).value<void*>()));
            if (pSet != NULL)
            {
               pExportSet->insertSignatures(pSet->getSignatures());
            }
            else
            {
               pExportSet->insertSignature(reinterpret_cast<Signature*>(pItem->data(0,
                  Qt::UserRole).value<void*>()));
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
   bool updateList = false;

   QList<QTreeWidgetItem*> signatureItems = mpSignatureList->selectedItems();
   for (int i = 0; i < signatureItems.count(); ++i)
   {
      QTreeWidgetItem* pItem = signatureItems[i];
      if (pItem != NULL)
      {
         Signature* pSignature = reinterpret_cast<Signature*>(pItem->data(0, Qt::UserRole).value<void*>());
         if (pSignature != NULL)
         {
            if (mpModel->destroyElement(pSignature) == true)
            {
               updateList = true;
            }
         }
      }
   }

   if (updateList == true)
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
   enableButtons();

   vector<DataElement*> signatures = mpModel->getElements("Signature");
   vector<DataElement*> signatureSets = mpModel->getElements("SignatureSet");

   for (vector<DataElement*>::const_iterator iter = signatures.begin(); iter != signatures.end(); ++iter)
   {
      Signature* pSignature = static_cast<Signature*>(*iter);
      if (pSignature != NULL)
      {
         // Only add the signature or signature set if it is not contained in at least one signature set
         bool setSignature = false;
         for (vector<DataElement*>::const_iterator setIter = signatureSets.begin();
            setIter != signatureSets.end();
            ++setIter)
         {
            SignatureSet* pSignatureSet = static_cast<SignatureSet*>(*setIter);
            if ((pSignatureSet != NULL) && (static_cast<Signature*>(pSignatureSet) != pSignature))
            {
               if (pSignatureSet->hasSignature(pSignature) == true)
               {
                  setSignature = true;
                  break;
               }
            }
         }

         if (setSignature == false)
         {
            addSignature(pSignature);
         }
      }
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
               pSettings->setTemporarySetting(ConfigurationSettings::getSettingPluginWorkingDirectoryKey("Signature"),
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

bool SignatureSelector::isFilterNameUnique(const QString& filterName, QTreeWidgetItem* pIgnoreItem) const
{
   if (filterName.isEmpty() == true)
   {
      return false;
   }

   QTreeWidgetItemIterator iter(mpSignaturesItem);
   while (*iter != NULL)
   {
      QTreeWidgetItem* pFilterItem = *iter;
      if ((pFilterItem != NULL) && (pFilterItem != pIgnoreItem))
      {
         if (pFilterItem->text(0) == filterName)
         {
            return false;
         }
      }

      ++iter;
   }

   return true;
}

void SignatureSelector::createFilter()
{
   QTreeWidgetItem* pCurrentItem = mpFormatTree->currentItem();
   VERIFYNRV(pCurrentItem != NULL);

   QTreeWidgetItem* pParentItem = pCurrentItem->parent();
   if (pParentItem == NULL)
   {
      pParentItem = pCurrentItem;
   }

   if (pParentItem != mpSignaturesItem)
   {
      return;
   }

   VERIFYNRV(pParentItem != NULL);

   SignatureFilterDlg filterDlg(this);
   filterDlg.setWindowTitle("Create Signature Filter");
   filterDlg.setFilterName("Filter " + QString::number(pParentItem->childCount() + 1));

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
         QMessageBox::warning(this, windowTitle(), "Another filter exists with the same name.  "
            "Please select a unique name for the filter.");
      }
      else if ((filterName == spMetadataType) || (filterName == spDashType))
      {
         QMessageBox::warning(this, windowTitle(), "The \"" + filterName + "\" filter name is reserved "
            "for internal use.  Please select a different name for the filter.");
         uniqueName = false;
      }
   }

   QMap<QString, QVariant> filter;
   filter.insert("Library Signatures", QVariant(filterDlg.getLibrarySignatures()));
   filter.insert("Signature Name", QVariant(filterDlg.getSignatureNameFilter()));
   filter.insert("Metadata Name", QVariant(filterDlg.getMetadataNameFilter()));
   filter.insert("Metadata Value", QVariant(filterDlg.getMetadataValueFilter()));

   QTreeWidgetItem* pFilterItem = new QTreeWidgetItem(pParentItem);
   pFilterItem->setText(0, filterName);      // Must set the text first to prevent checkFilterName()
                                             // from prompting the user when the data and flags change
   pFilterItem->setData(0, Qt::UserRole, QVariant(filter));
   pFilterItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

   // Setting the current item automatically causes the updateSignatureList() slot to be called
   mpFormatTree->setCurrentItem(pFilterItem);
}

void SignatureSelector::editFilter()
{
   QTreeWidgetItem* pItem = mpFormatTree->currentItem();
   if ((pItem == NULL) || (pItem->parent() == NULL))
   {
      return;
   }

   QString filterName = pItem->text(0);
   QMap<QString, QVariant> filter = pItem->data(0, Qt::UserRole).toMap();
   VERIFYNRV(filter.isEmpty() == false);

   SignatureFilterDlg filterDlg(this);
   filterDlg.setWindowTitle("Edit Signature Filter");
   filterDlg.setFilterName(filterName);
   filterDlg.setLibrarySignatures(filter["Library Signatures"].toBool());
   filterDlg.setSignatureNameFilter(filter["Signature Name"].toRegExp());
   filterDlg.setMetadataNameFilter(filter["Metadata Name"].toRegExp());
   filterDlg.setMetadataValueFilter(filter["Metadata Value"].toRegExp());

   bool uniqueName = false;
   while (uniqueName == false)
   {
      if (filterDlg.exec() == QDialog::Rejected)
      {
         return;
      }

      filterName = filterDlg.getFilterName();   // The dialog ensures that the filter name is not empty
      uniqueName = isFilterNameUnique(filterName, pItem);
      if (uniqueName == false)
      {
         QMessageBox::warning(this, windowTitle(), "Another filter exists with the same name.  "
            "Please select a unique name for the filter.");
      }
      else if ((filterName == spMetadataType) || (filterName == spDashType))
      {
         QMessageBox::warning(this, windowTitle(), "The \"" + filterName + "\" filter name is reserved "
            "for internal use.  Please select a different name for the filter.");
         uniqueName = false;
      }
   }

   filter["Library Signatures"] = QVariant(filterDlg.getLibrarySignatures());
   filter["Signature Name"] = QVariant(filterDlg.getSignatureNameFilter());
   filter["Metadata Name"] = QVariant(filterDlg.getMetadataNameFilter());
   filter["Metadata Value"] = QVariant(filterDlg.getMetadataValueFilter());

   pItem->setText(0, filterName);
   pItem->setData(0, Qt::UserRole, QVariant(filter));

   updateSignatureList();
}

void SignatureSelector::checkFilterName(QTreeWidgetItem* pItem, int column)
{
   if ((pItem == NULL) || (column != 0))
   {
      return;
   }

   if (pItem->parent() != mpSignaturesItem)
   {
      return;
   }

   QString filterName = pItem->text(0);
   if (filterName.isEmpty() == true)
   {
      QMessageBox::warning(this, windowTitle(), "Please enter a valid name for the filter.");
   }
   else if (isFilterNameUnique(filterName, pItem) == false)
   {
      QMessageBox::warning(this, windowTitle(), "Another filter exists with the same name.  "
         "Please select a unique name for the filter.");
   }
   else if ((filterName == spMetadataType) || (filterName == spDashType))
   {
      QMessageBox::warning(this, windowTitle(), "The \"" + filterName + "\" filter name is reserved "
         "for internal use.  Please select a different name for the filter.");
   }
   else
   {
      return;
   }

   mpFormatTree->closePersistentEditor(pItem, 0);
   mpFormatTree->editItem(pItem, 0);
}

void SignatureSelector::deleteFilter()
{
   QTreeWidgetItem* pItem = mpFormatTree->currentItem();
   if (pItem == NULL)
   {
      return;
   }

   // Do not allow the top-level items to be deleted
   QTreeWidgetItem* pParentItem = pItem->parent();
   if (pParentItem == NULL)
   {
      return;
   }

   // Delete the item
   delete pItem;

   // Make the parent item the current item, which updates the signature list
   mpFormatTree->setCurrentItem(pParentItem);
}

void SignatureSelector::formatChanged()
{
   QTreeWidgetItem* pCurrentItem = mpFormatTree->currentItem();
   QTreeWidgetItem* pParentItem = NULL;
   if (pCurrentItem != NULL)
   {
      pParentItem = pCurrentItem->parent();
   }

   mpCreateFilterAction->setEnabled(pParentItem == mpSignaturesItem || pCurrentItem == mpSignaturesItem);
   mpEditFilterAction->setEnabled(pParentItem == mpSignaturesItem);
   mpDeleteFilterAction->setEnabled(pParentItem == mpSignaturesItem);
   mpImportButton->setEnabled(pCurrentItem == mpSignaturesItem);
   mpImportWidget->setEnabled(pCurrentItem == mpSignaturesItem);
}

void SignatureSelector::displayFormatContextMenu(const QPoint& menuPoint)
{
   QTreeWidgetItem* pCurrentItem = mpFormatTree->currentItem();
   QTreeWidgetItem* pParentItem = NULL;
   if (pCurrentItem != NULL)
   {
      pParentItem = pCurrentItem->parent();
   }

   QMenu contextMenu(mpFormatTree);
   if (pCurrentItem == mpSignaturesItem || pParentItem == mpSignaturesItem)
   {
      contextMenu.addAction(mpCreateFilterAction);
   }

   QAction* pRenameFilterAction = NULL;
   if (pParentItem == mpSignaturesItem)
   {
      contextMenu.addSeparator();
      pRenameFilterAction = contextMenu.addAction("Rename");
      contextMenu.addAction(mpEditFilterAction);
      contextMenu.addAction(mpDeleteFilterAction);
   }

   if (contextMenu.actions().empty() == true)
   {
      return;
   }

   QAction* pInvokedAction = contextMenu.exec(mpFormatTree->viewport()->mapToGlobal(menuPoint));
   if (pInvokedAction == pRenameFilterAction && pRenameFilterAction != NULL)
   {
      mpFormatTree->editItem(pCurrentItem);
   }
}

void SignatureSelector::displaySignatureContextMenu(const QPoint& menuPoint)
{
   QTreeWidgetItem* pItem = mpFormatTree->currentItem();
   if ((pItem != NULL) && (pItem->parent() != NULL))
   {
      pItem = pItem->parent();
   }

   if (pItem != mpSignaturesItem)
   {
      return;
   }

   int numSignatures = getNumSelectedSignatures();
   if (numSignatures == 0)
   {
      return;
   }

   QMenu contextMenu(mpFormatTree);
   if (numSignatures == 1)
   {
      contextMenu.addAction(mpPropertiesAction);
   }

   contextMenu.addAction(mpExportAction);
   contextMenu.addAction(mpDeleteAction);
   contextMenu.exec(mpSignatureList->viewport()->mapToGlobal(menuPoint));
}
