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
         if(pA == NULL || pB == NULL)
         {
            return false;
         }
         DataDescriptor *pDdA = pA->getDataDescriptor();
         DataDescriptor *pDdB = pB->getDataDescriptor();
         if(pDdA == NULL || pDdB == NULL)
         {
            return false;
         }
         return less::operator()(pDdA->getParentDesignator().size(), pDdB->getParentDesignator().size());
      }
   };
};

// The ImportOptionsDlg should be changed to listen to the FileDescriptor and DataDescriptor being edited
// and immediately update their gui's in case the importer mutates either descriptor
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Make ImportOptionsDlg provide the data descriptor being modified by the user to the custom options widget (kstreith)")

ImportOptionsDlg::ImportOptionsDlg(Importer* pImporter, const vector<ImportDescriptor*>& descriptors,
   QWidget* pParent) :
   QDialog(pParent),
   mpImporter(pImporter),
   mpCurrentDataset(NULL),
   mpEditDescriptor(NULL),
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
   mpDatasetTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   mpDatasetTree->setTextElideMode(Qt::ElideLeft);
   mpDatasetTree->header()->hide();

   QPushButton* pImportAllButton = new QPushButton("Import All", pDatasetWidget);
   QPushButton* pImportNoneButton = new QPushButton("Import None", pDatasetWidget);

   // Tab widget
   mpTabWidget = new QTabWidget(pSplitter);

   // Create the pages
   mpDataPage = new DataDescriptorWidget();
   mpSubsetPage = new SubsetWidget();
   mpFilePage = new FileDescriptorWidget();
   mpMetadataPage = new MetadataWidget();

   // Add the pages to the tab widget
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

   // Set the dialog buttons
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

   vector<ImportDescriptor*> datasets;
   copy(descriptors.begin(), descriptors.end(), back_inserter(datasets));
   stable_sort(datasets.begin(), datasets.end(), ElementDepthComparitor());

   map<vector<string>, QTreeWidgetItem*> parentPaths;
   bool bInitialized = false;
   for (vector<ImportDescriptor*>::iterator iter = datasets.begin(); iter != datasets.end(); ++iter)
   {
      ImportDescriptor *pImportDescriptor = *iter;
      if(pImportDescriptor == NULL)
      {
         continue;
      }
      DataDescriptor *pDescriptor = pImportDescriptor->getDataDescriptor();
      if(pDescriptor == NULL)
      {
         continue;
      }
      const string &name = pDescriptor->getName();
      if(name.empty())
      {
         continue;
      }
      QTreeWidgetItem *pItem = new QTreeWidgetItem(static_cast<QTreeWidget*>(NULL), QStringList() << QString::fromStdString(name));
      if(pItem == NULL)
      {
         continue;
      }
      // Tool tip
      pItem->setToolTip(0, QString::fromStdString(name));

      // Check state
      Qt::ItemFlags itemFlags = pItem->flags();
      itemFlags |= Qt::ItemIsUserCheckable;
      pItem->setFlags(itemFlags);

      if(pImportDescriptor->isImported())
      {
         pItem->setCheckState(0, Qt::Checked);

         if(!bInitialized)
         {
            mpDatasetTree->setItemSelected(pItem, true);
            bInitialized = true;
         }
      }
      else
      {
         pItem->setCheckState(0, Qt::Unchecked);
      }

      // Add to the proper parent
      map<vector<string>,QTreeWidgetItem*>::iterator parent = parentPaths.find(pDescriptor->getParentDesignator());
      if(parent != parentPaths.end())
      {
         parent->second->addChild(pItem);
      }
      else
      {
         mpDatasetTree->addTopLevelItem(pItem);
      }
      vector<string> myDesignator = pDescriptor->getParentDesignator();
      myDesignator.push_back(pDescriptor->getName());
      parentPaths[myDesignator] = pItem;

      mDatasets[pImportDescriptor] = pItem;
   }

   QList<QTreeWidgetItem*> selectedItems = mpDatasetTree->selectedItems();
   if (selectedItems.empty() == true)
   {
      map<ImportDescriptor*, QTreeWidgetItem*>::iterator iter = mDatasets.begin();
      if (iter != mDatasets.end())
      {
         setCurrentDataset(iter->first);
      }
   }
   else
   {
      updateCurrentDataset();
   }

   // Connections
   VERIFYNR(connect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(validate())));
   VERIFYNR(connect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(enforceSelections(QTreeWidgetItem*))));
   VERIFYNR(connect(mpDatasetTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateCurrentDataset())));
   VERIFYNR(connect(pImportAllButton, SIGNAL(clicked()), this, SLOT(selectAllDatasets())));
   VERIFYNR(connect(pImportNoneButton, SIGNAL(clicked()), this, SLOT(deselectAllDatasets())));
   VERIFYNR(connect(mpOkButton, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject())));
   updateConnections(true);
   mpDatasetTree->expandAll();
}

ImportOptionsDlg::~ImportOptionsDlg()
{
   removeImporterPage();
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
         int iReturn = QMessageBox::question(this, APP_NAME, "Apply changes?", QMessageBox::Yes, QMessageBox::No,
            QMessageBox::Cancel);
         if (iReturn == QMessageBox::Yes)
         {
            bSuccess = applyChanges();
         }
         else if (iReturn == QMessageBox::Cancel)
         {
            bSuccess = false;
         }
      }
   }

   if (bSuccess == false)
   {
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

   // Set the current list widget item
   map<ImportDescriptor*, QTreeWidgetItem*>::iterator iter(mDatasets.find(mpCurrentDataset));
   if (iter != mDatasets.end())
   {
      QTreeWidgetItem* pItem = iter->second;
      if (pItem != NULL)
      {
         disconnect(mpDatasetTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateCurrentDataset()));
         mpDatasetTree->setCurrentItem(pItem);     // Also selects the item
         VERIFYNR(connect(mpDatasetTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateCurrentDataset())));
      }
   }

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

   // Data descriptor page
   mpDataPage->setDataDescriptor(mpEditDescriptor);

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

   // Update the dialog title
   QString strTitle = "Import Options";
   if (pFileDescriptor != NULL)
   {
      string filename = pFileDescriptor->getFilename();
      if (filename.empty() == false)
      {
         QFileInfo fileInfo(QString::fromStdString(filename));

         QString strName = fileInfo.fileName();
         if (strName.isEmpty() == false)
         {
            strTitle += ": " + strName;
         }
      }
   }

   setWindowTitle(strTitle);

   // Validate the current data descriptor
   validate();

   // Reconnect the pages
   updateConnections(true);

   // Notify connected objects
   emit currentDatasetChanged(mpCurrentDataset);
}

ImportDescriptor* ImportOptionsDlg::getCurrentDataset() const
{
   return mpCurrentDataset;
}

void ImportOptionsDlg::accept()
{
   // Apply changes to the data descriptor
   applyChanges();

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

void ImportOptionsDlg::updateCurrentDataset()
{
   // Get the current dataset in the list widget
   QList<QTreeWidgetItem*> selectedItems = mpDatasetTree->selectedItems();
   VERIFYNRV(selectedItems.count() == 1);

   QTreeWidgetItem* pItem = selectedItems.front();
   VERIFYNRV(pItem != NULL);

   ImportDescriptor* pDataset = NULL;

   map<ImportDescriptor*, QTreeWidgetItem*>::const_iterator iter;
   for (iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
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
   for (map<ImportDescriptor*, QTreeWidgetItem*>::iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      QTreeWidgetItem* pItem = iter->second;
      if (pItem != NULL)
      {
         pItem->setCheckState(0, Qt::Checked);
      }
   }
}

void ImportOptionsDlg::deselectAllDatasets()
{
   for (map<ImportDescriptor*, QTreeWidgetItem*>::iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      QTreeWidgetItem* pItem = iter->second;
      if (pItem != NULL)
      {
         pItem->setCheckState(0, Qt::Unchecked);
      }
   }
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
      validate();
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
      validate();
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
      validate();
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
   validate();
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

   validate();
}

void ImportOptionsDlg::validate()
{
   QString strValidation;
   bool bError = true;

   // Validate the user inputs from the importer
   if ((mpEditDescriptor != NULL) && (mpImporter != NULL))
   {
      string errorMessage;

      bool bSuccess = mpImporter->validate(mpEditDescriptor, errorMessage);
      bError = !bSuccess;
      if (!errorMessage.empty())
      {
         strValidation = QString::fromStdString(errorMessage);
      }
   }

   // Validate that there is at least one imported data set
   if (!bError)
   {
      bool bNoDatasets = true;
      for (map<ImportDescriptor*, QTreeWidgetItem*>::iterator item = mDatasets.begin(); item != mDatasets.end(); ++item)
      {
         QTreeWidgetItem *pItem = item->second;
         if (pItem != NULL && pItem->checkState(0) != Qt::Unchecked)
         {
            bNoDatasets = false;
            break;
         }
      }

      if (bNoDatasets == true)
      {
         strValidation = "There are no data sets selected for import.";
         bError = true;
      }
   }

   QPalette validationPalette = mpValidationLabel->palette();
   QColor validationColor = (bError ? Qt::red : Qt::blue);
   validationPalette.setColor(QPalette::WindowText, validationColor);
   mpValidationLabel->setPalette(validationPalette);

   // Update the validation text
   mpValidationLabel->setText(strValidation);

   // Enable/disable the OK button
   mpOkButton->setEnabled(!bError);
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

void ImportOptionsDlg::enforceSelections(QTreeWidgetItem *pItem)
{
   if(pItem == NULL)
   {
      return;
   }
   if(pItem->checkState(0) == Qt::Unchecked)
   {
      queue<QTreeWidgetItem*> toVisit;
      toVisit.push(pItem);
      for(; !toVisit.empty(); toVisit.pop())
      {
         pItem = toVisit.front();
         pItem->setCheckState(0, Qt::Unchecked);
         for(int i = 0; i < pItem->childCount(); i++)
         {
            if(pItem->child(i) != NULL)
            {
               toVisit.push(pItem->child(i));
            }
         }
      }
   }
   else
   {
      for(; pItem != NULL; pItem = pItem->parent())
      {
         if(pItem->checkState(0) == Qt::Unchecked)
         {
            pItem->setCheckState(0, Qt::PartiallyChecked);
         }
      }
   }
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
