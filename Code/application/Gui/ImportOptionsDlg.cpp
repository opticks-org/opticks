/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtGui/QHeaderView>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QSplitter>

#include "AppVersion.h"
#include "DataDescriptorWidget.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "FileDescriptorWidget.h"
#include "ImportDescriptorImp.h"
#include "Importer.h"
#include "ImportOptionsDlg.h"
#include "MetadataWidget.h"
#include "ModelServices.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"
#include "SubsetWidget.h"

#include <algorithm>
#include <queue>
#include <sstream>
#include <string>
using namespace std;

namespace
{
   struct ElementDepthComparitor : less<vector<string>::size_type >
   {
      bool operator()(ImportDescriptor *pA, ImportDescriptor *pB)
      {
         if (pA == NULL || pB == NULL)
         {
            return false;
         }
         DataDescriptor* pDdA = pA->getDataDescriptor();
         DataDescriptor* pDdB = pB->getDataDescriptor();
         if (pDdA == NULL || pDdB == NULL)
         {
            return false;
         }
         return less<vector<string>::size_type>::operator()(pDdA->getParentDesignator().size(), pDdB->getParentDesignator().size());
      }
   };
};

// The ImportOptionsDlg should be changed to listen to the FileDescriptor and DataDescriptor being edited
// and immediately update their gui's in case the importer mutates either descriptor
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Make ImportOptionsDlg provide the data descriptor " \
   "being modified by the user to the custom options widget (kstreith)")

ImportOptionsDlg::ImportOptionsDlg(Importer* pImporter, const QMap<QString, vector<ImportDescriptor*> >& files,
   QWidget* pParent) :
   QDialog(pParent),
   mpImporter(pImporter),
   mpCurrentDataset(NULL),
   mpEditDescriptor(NULL),
   mPromptForChanges(true),
   mAllowDeselectedFiles(true),
   mpDatasetTree(NULL),
   mpTabWidget(NULL),
   mpDataPage(NULL),
   mpFilePage(NULL),
   mpSubsetPage(NULL),
   mpMetadataPage(NULL),
   mpImporterPage(NULL),
   mpValidationLabel(NULL)
{
   QSplitter* pSplitter = new QSplitter(this);

   // Dataset list
   QWidget* pDatasetWidget = new QWidget(pSplitter);
   QLabel* pDatasetLabel = new QLabel("Data Sets:", pDatasetWidget);

   mpDatasetTree = new QTreeWidget(pDatasetWidget);
   mpDatasetTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpDatasetTree->setSelectionBehavior(QAbstractItemView::SelectItems);
   mpDatasetTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   mpDatasetTree->setTextElideMode(Qt::ElideLeft);
   mpDatasetTree->setMinimumWidth(225);
   mpDatasetTree->header()->hide();

   QPushButton* pImportAllButton = new QPushButton("Import All", pDatasetWidget);
   QPushButton* pImportNoneButton = new QPushButton("Import None", pDatasetWidget);

   // Tab widget
   mpTabWidget = new QTabWidget(pSplitter);
   mpDataPage = new DataDescriptorWidget();
   mpSubsetPage = new SubsetWidget();
   mpFilePage = new FileDescriptorWidget();
   mpMetadataPage = new MetadataWidget();

   mpTabWidget->addTab(mpDataPage, "Data");
   mpTabWidget->addTab(mpFilePage, "File");
   mpTabWidget->addTab(mpSubsetPage, "Subset");
   mpTabWidget->addTab(mpMetadataPage, "Metadata");

   // Validation label
   mpValidationLabel = new QLabel(this);
   mpValidationLabel->setWordWrap(true);

   QFont validationFont = mpValidationLabel->font();
   validationFont.setBold(true);
   mpValidationLabel->setFont(validationFont);

   // Dialog buttons
   mpOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Layout
   QLayout* pDataLayout = mpDataPage->layout();
   if (pDataLayout != NULL)
   {
      pDataLayout->setMargin(10);
   }

   QLayout* pSubsetLayout = mpSubsetPage->layout();
   if (pSubsetLayout != NULL)
   {
      pSubsetLayout->setMargin(10);
   }

   QLayout* pFileLayout = mpFilePage->layout();
   if (pFileLayout != NULL)
   {
      pFileLayout->setMargin(10);
   }

   QLayout* pMetadataLayout = mpMetadataPage->layout();
   if (pMetadataLayout != NULL)
   {
      pMetadataLayout->setMargin(10);
   }

   QGridLayout* pDatasetLayout = new QGridLayout(pDatasetWidget);
   pDatasetLayout->setMargin(0);
   pDatasetLayout->setSpacing(5);
   pDatasetLayout->addWidget(pDatasetLabel, 0, 0, 1, 2);
   pDatasetLayout->addWidget(mpDatasetTree, 1, 0, 1, 2);
   pDatasetLayout->addWidget(pImportAllButton, 2, 0, Qt::AlignRight);
   pDatasetLayout->addWidget(pImportNoneButton, 2, 1);
   pDatasetLayout->setRowStretch(1, 10);
   pDatasetLayout->setColumnStretch(0, 10);

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addWidget(mpValidationLabel, 10);
   pButtonLayout->addWidget(mpOkButton, 0, Qt::AlignBottom);
   pButtonLayout->addWidget(pCancelButton, 0, Qt::AlignBottom);

   QVBoxLayout* pVBox = new QVBoxLayout(this);
   pVBox->setMargin(10);
   pVBox->setSpacing(10);
   pVBox->addWidget(pSplitter, 10);
   pVBox->addLayout(pButtonLayout);

   // Initialization
   setWindowTitle("Import Options");
   setModal(true);
   resize(700, 450);

   pSplitter->addWidget(pDatasetWidget);
   pSplitter->addWidget(mpTabWidget);

   if (mpImporter != NULL)
   {
      vector<ProcessingLocation> locations;
      if (mpImporter->isProcessingLocationSupported(IN_MEMORY) == true)
      {
         locations.push_back(IN_MEMORY);
      }

      if (mpImporter->isProcessingLocationSupported(ON_DISK) == true)
      {
         locations.push_back(ON_DISK);
      }

      if (mpImporter->isProcessingLocationSupported(ON_DISK_READ_ONLY) == true)
      {
         locations.push_back(ON_DISK_READ_ONLY);
      }

      mpDataPage->setValidProcessingLocations(locations);
   }

   // Populate the data set tree widget
   QTreeWidgetItem* pSelectItem = NULL;

   QMap<QString, vector<ImportDescriptor*> >::const_iterator fileIter;
   for (fileIter = files.begin(); fileIter != files.end(); ++fileIter)
   {
      // Filename item
      QString filename = fileIter.key();
      if (filename.isEmpty() == true)
      {
         continue;
      }

      QTreeWidgetItem* pFileItem = new QTreeWidgetItem(mpDatasetTree);

      QFileInfo fileInfo(filename);
      pFileItem->setText(0, fileInfo.fileName());
      pFileItem->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
      pFileItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
      pFileItem->setToolTip(0, filename);
      pFileItem->setCheckState(0, Qt::Unchecked);

      // Dataset items
      vector<ImportDescriptor*> fileDatasets = fileIter.value();
      vector<ImportDescriptor*> datasets;
      copy(fileDatasets.begin(), fileDatasets.end(), back_inserter(datasets));
      stable_sort(datasets.begin(), datasets.end(), ElementDepthComparitor());

      map<vector<string>, QTreeWidgetItem*> parentPaths;

      vector<ImportDescriptor*>::iterator datasetIter;
      for (datasetIter = datasets.begin(); datasetIter != datasets.end(); ++datasetIter)
      {
         ImportDescriptor* pImportDescriptor = *datasetIter;
         if (pImportDescriptor == NULL)
         {
            continue;
         }

         DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
         if (pDescriptor == NULL)
         {
            continue;
         }

         const string& name = pDescriptor->getName();
         if (name.empty())
         {
            continue;
         }

         QTreeWidgetItem* pItem = new QTreeWidgetItem(static_cast<QTreeWidget*>(NULL),
            QStringList() << QString::fromStdString(name));
         if (pItem == NULL)
         {
            continue;
         }

         // Tool tip
         pItem->setToolTip(0, QString::fromStdString(name));

         // Check state
         Qt::ItemFlags itemFlags = pItem->flags();
         itemFlags |= Qt::ItemIsUserCheckable;
         pItem->setFlags(itemFlags);

         if (pImportDescriptor->isImported())
         {
            pItem->setCheckState(0, Qt::Checked);

            if (pSelectItem == NULL)
            {
               pSelectItem = pItem;
            }
         }
         else
         {
            pItem->setCheckState(0, Qt::Unchecked);
         }

         // Add to the proper parent
         map<vector<string>, QTreeWidgetItem*>::iterator parent = parentPaths.find(pDescriptor->getParentDesignator());
         if (parent != parentPaths.end())
         {
            parent->second->addChild(pItem);
         }
         else
         {
            pFileItem->addChild(pItem);
         }

         vector<string> myDesignator = pDescriptor->getParentDesignator();
         myDesignator.push_back(pDescriptor->getName());
         parentPaths[myDesignator] = pItem;

         mDatasets[pImportDescriptor] = pItem;
         validateDataset(pDescriptor);
         enforceSelections(pItem);
      }
   }

   mpDatasetTree->expandAll();

   // Update the tab widget for the selected data set
   if (pSelectItem != NULL)
   {
      mpDatasetTree->setItemSelected(pSelectItem, true);
      updateEditDataset();
   }
   else
   {
      map<ImportDescriptor*, QTreeWidgetItem*>::iterator iter = mDatasets.begin();
      if (iter != mDatasets.end())
      {
         setCurrentDataset(iter->first);
      }
   }

   // Connections
   VERIFYNR(connect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(datasetItemChanged(QTreeWidgetItem*))));
   VERIFYNR(connect(mpDatasetTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateEditDataset())));
   VERIFYNR(connect(pImportAllButton, SIGNAL(clicked()), this, SLOT(selectAllDatasets())));
   VERIFYNR(connect(pImportNoneButton, SIGNAL(clicked()), this, SLOT(deselectAllDatasets())));
   VERIFYNR(connect(mpOkButton, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject())));
   updateConnections(true);
}

ImportOptionsDlg::~ImportOptionsDlg()
{
   removeImporterPage();
}

void ImportOptionsDlg::allowDeselectedFiles(bool allowDeselectedFiles)
{
   mAllowDeselectedFiles = allowDeselectedFiles;
}

bool ImportOptionsDlg::areDeselectedFilesAllowed() const
{
   return mAllowDeselectedFiles;
}

void ImportOptionsDlg::setCurrentDataset(ImportDescriptor* pImportDescriptor)
{
   if (pImportDescriptor == NULL)
   {
      return;
   }

   if (pImportDescriptor == mpCurrentDataset)
   {
      return;
   }

   // Apply changes to the current data set if necessary
   bool bSuccess = true;
   if (mpCurrentDataset != NULL)
   {
      if ((mpDataPage->isModified() == true) || (mpFilePage->isModified() == true) ||
         (mpMetadataPage->isModified() == true))
      {
         if (mPromptForChanges == true)
         {
            int iReturn = QMessageBox::question(this, APP_NAME, "Apply changes?",
               QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::Cancel);
            if ((iReturn == QMessageBox::Yes) || (iReturn == QMessageBox::YesToAll))
            {
               bSuccess = applyChanges();
               if (iReturn == QMessageBox::YesToAll)
               {
                  mPromptForChanges = false;
               }
            }
            else if (iReturn == QMessageBox::No)
            {
               // Update the validation icon for the original data descriptor
               validateDataset(mpCurrentDataset->getDataDescriptor());
            }
            else if (iReturn == QMessageBox::Cancel)
            {
               bSuccess = false;
            }
         }
         else
         {
            bSuccess = applyChanges();
         }
      }
   }

   if (bSuccess == false)
   {
      // Select the tree widget item for the previously selected data set
      selectCurrentDatasetItem();
      return;
   }

   mpCurrentDataset = pImportDescriptor;

   // Destroy the existing edit data descriptor if necessary
   Service<ModelServices> pModel;
   if (mpEditDescriptor != NULL)
   {
      pModel->destroyDataDescriptor(mpEditDescriptor);
      mpEditDescriptor = NULL;
   }

   // Create a new data descriptor to validate the user inputs
   DataDescriptor* pDescriptor = mpCurrentDataset->getDataDescriptor();
   if (pDescriptor != NULL)
   {
      mpEditDescriptor = pDescriptor->copy(pDescriptor->getName(), NULL);
   }

   VERIFYNRV(mpEditDescriptor != NULL);

   // Select the tree widget item for the current data set
   selectCurrentDatasetItem();

   // Disconnect pages
   updateConnections(false);

   // Subset page
   const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpEditDescriptor);
   const FileDescriptor* pFileDescriptor = mpEditDescriptor->getFileDescriptor();
   const RasterFileDescriptor* pRasterFileDescriptor = dynamic_cast<const RasterFileDescriptor*>(pFileDescriptor);

   if (pRasterFileDescriptor != NULL)
   {
      // Show the tab if necessary
      if (mpTabWidget->indexOf(mpSubsetPage) == -1)
      {
         mpTabWidget->insertTab(2, mpSubsetPage, "Subset");
      }

      // Rows
      const vector<DimensionDescriptor>& rows = pRasterFileDescriptor->getRows();
      const vector<DimensionDescriptor>& loadedRows = pRasterDescriptor->getRows();
      mpSubsetPage->setRows(rows, loadedRows);

      // Columns
      const vector<DimensionDescriptor>& columns = pRasterFileDescriptor->getColumns();
      const vector<DimensionDescriptor>& loadedColumns = pRasterDescriptor->getColumns();
      mpSubsetPage->setColumns(columns, loadedColumns);

      // Bands
      const vector<DimensionDescriptor>& bands = pRasterFileDescriptor->getBands();
      vector<string> bandNames;
      const DynamicObject* pMetadata = pRasterDescriptor->getMetadata();
      if (pMetadata != NULL)
      {
         string pNamesPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, NAMES_METADATA_NAME, END_METADATA_NAME };
         const vector<string>* pBandNames = dv_cast<vector<string> >(&pMetadata->getAttributeByPath(pNamesPath));
         if ((pBandNames != NULL) && (pBandNames->size() == bands.size()))
         {
            bandNames = *pBandNames;
         }
         else
         {
            string pPrefixPath[] = { SPECIAL_METADATA_NAME, BAND_NAME_PREFIX_METADATA_NAME, END_METADATA_NAME };
            const string* pBandPrefix = dv_cast<string>(&pMetadata->getAttributeByPath(pPrefixPath));
            if (pBandPrefix != NULL)
            {
               string bandNamePrefix = *pBandPrefix;
               for (unsigned int i = 0; i < bands.size(); ++i)
               {
                  ostringstream formatter;
                  formatter << bandNamePrefix << " " << bands[i].getOriginalNumber() + 1;
                  bandNames.push_back(formatter.str());
               }
            }
         }
      }
      const vector<DimensionDescriptor>& selectedBands = pRasterDescriptor->getBands();
      mpSubsetPage->setBands(bands, bandNames, selectedBands);

      // Initial bad band file directory
      if (pFileDescriptor != NULL)
      {
         QString strDirectory;

         string filename = pFileDescriptor->getFilename();
         if (filename.empty() == false)
         {
            QFileInfo fileInfo(QString::fromStdString(filename));
            strDirectory = fileInfo.absolutePath();
         }

         mpSubsetPage->setBadBandFileDirectory(strDirectory);
      }
   }
   else
   {
      // Remove the subset page, since the file descriptor either isn't
      // present or isn't a RasterFileDescriptor, just a FileDescriptor
      int index = mpTabWidget->indexOf(mpSubsetPage);
      if (index != -1)
      {
         mpTabWidget->removeTab(index);
      }
   }

   // Data descriptor page - enable editing for all fields
   mpDataPage->setDataDescriptor(mpEditDescriptor, true);

   // File descriptor page
   bool bValidFile = true;

   if (pRasterFileDescriptor != NULL)
   {
      unsigned int numRows = pRasterFileDescriptor->getRowCount();
      unsigned int numColumns = pRasterFileDescriptor->getColumnCount();
      unsigned int bitsPerElement = pRasterFileDescriptor->getBitsPerElement();
      unsigned int numBands = pRasterFileDescriptor->getBandCount();
      if ((numRows == 0) || (numColumns == 0) || (numBands == 0) || (bitsPerElement == 0))
      {
         bValidFile = false;
      }
   }

   if (bValidFile == false)
   {
      mpFilePage->setFileDescriptor(const_cast<FileDescriptor*>(pFileDescriptor));
   }
   else
   {
      mpFilePage->setFileDescriptor(pFileDescriptor);
   }

   int iIndex = mpTabWidget->indexOf(mpFilePage);
   if (iIndex != -1)
   {
      if (pFileDescriptor == NULL)
      {
         mpTabWidget->removeTab(iIndex);
      }
   }
   else
   {
      if (pFileDescriptor != NULL)
      {
         mpTabWidget->insertTab(1, mpFilePage, "File");
      }
   }

   // Metadata page
   mpMetadataPage->setMetadata(mpEditDescriptor->getMetadata());

   // Importer page
   bool bImporterPageActive = false;
   if (mpImporterPage != NULL)
   {
      if (mpTabWidget->currentWidget() == mpImporterPage)
      {
         bImporterPageActive = true;
      }
   }

   removeImporterPage();

   if (mpImporter != NULL)
   {
      mpImporterPage = mpImporter->getImportOptionsWidget(pDescriptor);
      if (mpImporterPage != NULL)
      {
         QString strCaption = mpImporterPage->windowTitle();
         if (strCaption.isEmpty() == true)
         {
            strCaption = "Importer";
         }

         mpTabWidget->addTab(mpImporterPage, strCaption);

         if (bImporterPageActive == true)
         {
            mpTabWidget->setCurrentWidget(mpImporterPage);
         }
      }
   }

   // Validate the current data descriptor
   validateEditDataset();

   // Reconnect the pages
   updateConnections(true);

   // Notify connected objects
   emit currentDatasetChanged(mpCurrentDataset);
}

ImportDescriptor* ImportOptionsDlg::getCurrentDataset() const
{
   return mpCurrentDataset;
}

QString ImportOptionsDlg::getCurrentFile() const
{
   QString filename;

   map<ImportDescriptor*, QTreeWidgetItem*>::const_iterator iter = mDatasets.find(mpCurrentDataset);
   if (iter != mDatasets.end())
   {
      QTreeWidgetItem* pDatasetItem = iter->second;
      if (pDatasetItem != NULL)
      {
         QTreeWidgetItem* pFileItem = pDatasetItem->parent();
         if (pFileItem != NULL)
         {
            filename = pFileItem->toolTip(0);
         }
      }
   }

   return filename;
}

void ImportOptionsDlg::accept()
{
   // Apply changes to the data descriptor
   applyChanges();

   // Check if the user has no selected data sets for a file and display a message box if necessary
   if (mAllowDeselectedFiles == true)
   {
      for (int i = 0; i < mpDatasetTree->topLevelItemCount(); ++i)
      {
         QTreeWidgetItem* pFileItem = mpDatasetTree->topLevelItem(i);
         if ((pFileItem != NULL) && (pFileItem->checkState(0) == Qt::Unchecked))
         {
            int returnValue = QMessageBox::question(this, windowTitle(), "At least one file does not have any data "
               "sets that will be imported.  Do you want to continue?", QMessageBox::Yes | QMessageBox::No);
            if (returnValue == QMessageBox::No)
            {
               return;
            }

            break;
         }
      }
   }

   // Apply changes to the imported data sets
   for (map<ImportDescriptor*, QTreeWidgetItem*>::iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      ImportDescriptor* pDataset = iter->first;
      if (pDataset != NULL)
      {
         QTreeWidgetItem* pItem = iter->second;
         if (pItem != NULL)
         {
            bool bImport = (pItem->checkState(0) != Qt::Unchecked);
            pDataset->setImported(bImport);
         }
      }
   }

   // Close the dialog
   QDialog::accept();
}

bool ImportOptionsDlg::validateDataset(DataDescriptor* pDescriptor)
{
   QString message;
   return validateDataset(pDescriptor, message);
}

bool ImportOptionsDlg::validateDataset(DataDescriptor* pDescriptor, QString& validationMessage)
{
   validationMessage.clear();
   if (pDescriptor == NULL)
   {
      return false;
   }

   // Validate the user inputs from the importer
   bool validDataset = false;
   if (mpImporter != NULL)
   {
      string errorMessage;
      validDataset = mpImporter->validate(pDescriptor, errorMessage);
      if (!errorMessage.empty())
      {
         validationMessage = QString::fromStdString(errorMessage);
      }
   }

   // Update the validation icon for the dataset
   map<ImportDescriptor*, QTreeWidgetItem*>::iterator iter;
   if (pDescriptor == mpEditDescriptor)
   {
      iter = mDatasets.find(mpCurrentDataset);
   }
   else
   {
      for (iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
      {
         ImportDescriptor* pImportDescriptor = iter->first;
         if (pImportDescriptor != NULL)
         {
            if (pImportDescriptor->getDataDescriptor() == pDescriptor)
            {
               break;
            }
         }
      }
   }

   if (iter != mDatasets.end())
   {
      QTreeWidgetItem* pItem = iter->second;
      if (pItem != NULL)
      {
         disconnect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
            SLOT(datasetItemChanged(QTreeWidgetItem*)));

         if (validDataset == true)
         {
            pItem->setIcon(0, QIcon(":/icons/Ok"));
         }
         else
         {
            pItem->setIcon(0, QIcon(":/icons/Critical"));
         }

         pItem->setData(0, Qt::UserRole, QVariant(validDataset));

         VERIFYNR(connect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
            SLOT(datasetItemChanged(QTreeWidgetItem*))));
      }
   }

   return validDataset;
}

void ImportOptionsDlg::selectCurrentDatasetItem()
{
   if (mpCurrentDataset == NULL)
   {
      return;
   }

   map<ImportDescriptor*, QTreeWidgetItem*>::iterator iter = mDatasets.find(mpCurrentDataset);
   if (iter != mDatasets.end())
   {
      QTreeWidgetItem* pItem = iter->second;
      if (pItem != NULL)
      {
         disconnect(mpDatasetTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateEditDataset()));
         mpDatasetTree->setCurrentItem(pItem);     // Also selects the item
         VERIFYNR(connect(mpDatasetTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateEditDataset())));
      }
   }
}

void ImportOptionsDlg::validateEditDataset()
{
   QString validationMessage;
   bool validDataset = validateDataset(mpEditDescriptor, validationMessage);

   // Enable/disable the OK button
   bool enableButton = true;
   bool noDatasets = true;
   for (map<ImportDescriptor*, QTreeWidgetItem*>::iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      // Check that there is at least one imported data set and that all imported data sets are valid
      QTreeWidgetItem* pItem = iter->second;
      if (pItem != NULL && pItem->checkState(0) == Qt::Checked)
      {
         QVariant validValue = pItem->data(0, Qt::UserRole);
         if (validValue.toBool() == false)
         {
            if (validationMessage.isEmpty() == true)
            {
               validationMessage = "At least one data set selected for import is invalid.";
               validDataset = false;
            }

            enableButton = false;
            break;
         }

         noDatasets = false;
      }
   }

   if ((enableButton == true) && (noDatasets == true))
   {
      validationMessage = "No data sets are selected for import.";
      validDataset = false;
      enableButton = false;
   }

   // Check if the user has no imported data sets for a file and display a message box if necessary
   if ((enableButton == true) && (mAllowDeselectedFiles == false))
   {
      for (int i = 0; i < mpDatasetTree->topLevelItemCount(); ++i)
      {
         QTreeWidgetItem* pFileItem = mpDatasetTree->topLevelItem(i);
         if ((pFileItem != NULL) && (pFileItem->checkState(0) == Qt::Unchecked))
         {
            if (validationMessage.isEmpty() == false)
            {
               validationMessage += "\n\n";
            }

            validationMessage += "At least one file does not have any data sets that will be imported.";
            validDataset = false;
            enableButton = false;
            break;
         }
      }
   }

   mpOkButton->setEnabled(enableButton);

   // Update the validation text label
   QPalette validationPalette = mpValidationLabel->palette();
   QColor validationColor = (validDataset ? Qt::blue : Qt::red);
   validationPalette.setColor(QPalette::WindowText, validationColor);
   mpValidationLabel->setPalette(validationPalette);

   mpValidationLabel->setText(validationMessage);
}

void ImportOptionsDlg::enforceSelections(QTreeWidgetItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   // Update the children
   Qt::CheckState itemCheck = pItem->checkState(0);
   if (itemCheck != Qt::PartiallyChecked)
   {
      for (int i = 0; i < pItem->childCount(); ++i)
      {
         QTreeWidgetItem* pChild = pItem->child(i);
         if (pChild != NULL)
         {
            pChild->setCheckState(0, itemCheck);
         }
      }
   }

   // Update the parent
   QTreeWidgetItem* pParent = pItem->parent();
   if (pParent != NULL)
   {
      for (int i = 0; i < pParent->childCount(); ++i)
      {
         QTreeWidgetItem* pSibling = pParent->child(i);
         if (pSibling != NULL)
         {
            Qt::CheckState currentCheck = pSibling->checkState(0);
            if (currentCheck != itemCheck)
            {
               itemCheck = Qt::PartiallyChecked;
               break;
            }
         }
      }

      pParent->setCheckState(0, itemCheck);
   }
}

void ImportOptionsDlg::removeImporterPage()
{
   if (mpImporterPage == NULL)
   {
      return;
   }

   // Remove the page from the tab widget
   int iIndex = mpTabWidget->indexOf(mpImporterPage);
   if (iIndex != -1)
   {
      mpTabWidget->removeTab(iIndex);
   }

   // The importer is responsible for deleting the widget, so just reset the pointer
   mpImporterPage->setParent(NULL);
   mpImporterPage = NULL;
}

void ImportOptionsDlg::datasetItemChanged(QTreeWidgetItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   VERIFYNR(disconnect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(datasetItemChanged(QTreeWidgetItem*))));

   enforceSelections(pItem);
   validateEditDataset();

   VERIFYNR(connect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(datasetItemChanged(QTreeWidgetItem*))));
}

void ImportOptionsDlg::updateEditDataset()
{
   // Get the current dataset in the list widget
   QList<QTreeWidgetItem*> selectedItems = mpDatasetTree->selectedItems();
   if (selectedItems.empty() == true)
   {
      // User selected a file item, which clears the selection, so reset the current item to the dataset
      selectCurrentDatasetItem();
      return;
   }

   VERIFYNRV(selectedItems.count() == 1);

   QTreeWidgetItem* pItem = selectedItems.front();
   VERIFYNRV(pItem != NULL);

   ImportDescriptor* pDataset = NULL;
   for (map<ImportDescriptor*, QTreeWidgetItem*>::const_iterator iter = mDatasets.begin();
      iter != mDatasets.end();
      ++iter)
   {
      QTreeWidgetItem* pCurrentItem = iter->second;
      if (pCurrentItem == pItem)
      {
         pDataset = iter->first;
         break;
      }
   }

   // Activate the dataset
   setCurrentDataset(pDataset);
}

void ImportOptionsDlg::selectAllDatasets()
{
   VERIFYNR(disconnect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(datasetItemChanged(QTreeWidgetItem*))));

   for (map<ImportDescriptor*, QTreeWidgetItem*>::iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      QTreeWidgetItem* pItem = iter->second;
      if (pItem != NULL)
      {
         pItem->setCheckState(0, Qt::Checked);
         enforceSelections(pItem);
      }
   }

   validateEditDataset();

   VERIFYNR(connect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(datasetItemChanged(QTreeWidgetItem*))));
}

void ImportOptionsDlg::deselectAllDatasets()
{
   VERIFYNR(disconnect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(datasetItemChanged(QTreeWidgetItem*))));

   for (map<ImportDescriptor*, QTreeWidgetItem*>::iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      QTreeWidgetItem* pItem = iter->second;
      if (pItem != NULL)
      {
         pItem->setCheckState(0, Qt::Unchecked);
         enforceSelections(pItem);
      }
   }

   validateEditDataset();

   VERIFYNR(connect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(datasetItemChanged(QTreeWidgetItem*))));
}

void ImportOptionsDlg::generateDimensionVector(const QString& strValueName)
{
   if (mpEditDescriptor == NULL)
   {
      return;
   }

   updateConnections(false);

   // The user entered rows, columns, or bands on the file page
   // so update the data page and the subset page

   QString strValue = mpFilePage->getDescriptorValue(strValueName);
   mpDataPage->setDescriptorValue(strValueName, strValue);

   if (strValue.isEmpty() == false)
   {
      vector<DimensionDescriptor> values =
         RasterUtilities::generateDimensionVector(strValue.toUInt(), true, false, true);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpEditDescriptor);
      RasterFileDescriptor* pFileDescriptor = NULL;
      if (mpEditDescriptor != NULL)
      {
         pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(mpEditDescriptor->getFileDescriptor());
      }

      if (strValueName == "Rows")
      {
         // Create new rows and set into the subset page
         mpSubsetPage->setRows(values, vector<DimensionDescriptor>());
         if (pDescriptor != NULL)
         {
            pDescriptor->setRows(values);
         }
         if (pFileDescriptor != NULL)
         {
            pFileDescriptor->setRows(values);
         }
      }
      else if (strValueName == "Columns")
      {
         // Create new columns and set into the subset page
         mpSubsetPage->setColumns(values, vector<DimensionDescriptor>());
         if (pDescriptor != NULL)
         {
            pDescriptor->setColumns(values);
         }
         if (pFileDescriptor != NULL)
         {
            pFileDescriptor->setColumns(values);
         }
      }
      else if (strValueName == "Bands")
      {
         // Create new bands and set into the subset page
         mpSubsetPage->setBands(values);
         if (pDescriptor != NULL)
         {
            pDescriptor->setBands(values);
         }
         if (pFileDescriptor != NULL)
         {
            pFileDescriptor->setBands(values);
         }
      }
   }

   updateConnections(true);
}

void ImportOptionsDlg::updateDataRows(const vector<DimensionDescriptor>& rows)
{
   disconnect(mpDataPage, SIGNAL(modified()), this, SLOT(pagesModified()));

   mpDataPage->setDescriptorValue("Rows", QString::number(rows.size()));

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpEditDescriptor);
   if (pDescriptor != NULL)
   {
      pDescriptor->setRows(rows);
      validateEditDataset();
   }

   VERIFYNR(connect(mpDataPage, SIGNAL(modified()), this, SLOT(pagesModified())));
}

void ImportOptionsDlg::updateDataColumns(const vector<DimensionDescriptor>& columns)
{
   disconnect(mpDataPage, SIGNAL(modified()), this, SLOT(pagesModified()));

   mpDataPage->setDescriptorValue("Columns", QString::number(columns.size()));

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpEditDescriptor);
   if (pDescriptor != NULL)
   {
      pDescriptor->setColumns(columns);
      validateEditDataset();
   }

   VERIFYNR(connect(mpDataPage, SIGNAL(modified()), this, SLOT(pagesModified())));
}

void ImportOptionsDlg::updateDataBands(const vector<DimensionDescriptor>& bands)
{
   disconnect(mpDataPage, SIGNAL(modified()), this, SLOT(pagesModified()));

   mpDataPage->setDescriptorValue("Bands", QString::number(bands.size()));

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpEditDescriptor);
   if (pDescriptor != NULL)
   {
      pDescriptor->setBands(bands);
      validateEditDataset();
   }

   VERIFYNR(connect(mpDataPage, SIGNAL(modified()), this, SLOT(pagesModified())));
}

void ImportOptionsDlg::updateMetadata()
{
   if (mpEditDescriptor == NULL)
   {
      return;
   }

   disconnect(mpMetadataPage, SIGNAL(modified()), this, SLOT(updateMetadata()));

   bool bSuccess = mpMetadataPage->applyChanges();
   if (bSuccess == false)
   {
      mpTabWidget->setCurrentWidget(mpMetadataPage);
   }

   VERIFYNR(connect(mpMetadataPage, SIGNAL(modified()), this, SLOT(updateMetadata())));
   validateEditDataset();
}

void ImportOptionsDlg::pagesModified()
{
   if (mpEditDescriptor == NULL)
   {
      return;
   }

   disconnect(mpDataPage, SIGNAL(modified()), this, SLOT(pagesModified()));
   disconnect(mpFilePage, SIGNAL(modified()), this, SLOT(pagesModified()));

   // Data descriptor
   bool bSuccess = mpDataPage->applyChanges();
   if (!bSuccess)
   {
      mpTabWidget->setCurrentWidget(mpDataPage);
   }

   // File descriptor
   FileDescriptor* pFileDescriptor = mpEditDescriptor->getFileDescriptor();
   if (pFileDescriptor != NULL)
   {
      bSuccess = bSuccess && mpFilePage->applyChanges();
      if (!bSuccess)
      {
         mpTabWidget->setCurrentWidget(mpFilePage);
      }
   }

   VERIFYNR(connect(mpDataPage, SIGNAL(modified()), this, SLOT(pagesModified())));
   VERIFYNR(connect(mpFilePage, SIGNAL(modified()), this, SLOT(pagesModified())));

   validateEditDataset();
}

bool ImportOptionsDlg::applyChanges()
{
   if (mpEditDescriptor == NULL)
   {
      return false;
   }

   ImportDescriptorImp* pDataset = dynamic_cast<ImportDescriptorImp*>(getCurrentDataset());
   if (pDataset != NULL)
   {
      pDataset->setDataDescriptor(mpEditDescriptor);
      mpEditDescriptor = NULL;
      return true;
   }

   return false;
}

void ImportOptionsDlg::updateConnections(bool bConnect)
{
   if (bConnect == true)
   {
      VERIFYNR(connect(mpDataPage, SIGNAL(modified()), this, SLOT(pagesModified())));
      VERIFYNR(connect(mpFilePage, SIGNAL(modified()), this, SLOT(pagesModified())));
      VERIFYNR(connect(mpFilePage, SIGNAL(valueChanged(const QString&)), this,
         SLOT(generateDimensionVector(const QString&))));
      VERIFYNR(connect(mpSubsetPage, SIGNAL(subsetRowsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateDataRows(const std::vector<DimensionDescriptor>&))));
      VERIFYNR(connect(mpSubsetPage, SIGNAL(subsetColumnsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateDataColumns(const std::vector<DimensionDescriptor>&))));
      VERIFYNR(connect(mpSubsetPage, SIGNAL(subsetBandsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateDataBands(const std::vector<DimensionDescriptor>&))));
      VERIFYNR(connect(mpMetadataPage, SIGNAL(modified()), this, SLOT(updateMetadata())));
   }
   else
   {
      disconnect(mpDataPage, SIGNAL(modified()), this, SLOT(pagesModified()));
      disconnect(mpFilePage, SIGNAL(modified()), this, SLOT(pagesModified()));
      disconnect(mpFilePage, SIGNAL(valueChanged(const QString&)), this,
         SLOT(generateDimensionVector(const QString&)));
      disconnect(mpSubsetPage, SIGNAL(subsetRowsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateDataRows(const std::vector<DimensionDescriptor>&)));
      disconnect(mpSubsetPage, SIGNAL(subsetColumnsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateDataColumns(const std::vector<DimensionDescriptor>&)));
      disconnect(mpSubsetPage, SIGNAL(subsetBandsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateDataBands(const std::vector<DimensionDescriptor>&)));
      disconnect(mpMetadataPage, SIGNAL(modified()), this, SLOT(updateMetadata()));
   }
}
