/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QSplitter>
#include <QtGui/QStyleOptionButton>

#include "AppVerify.h"
#include "AppVersion.h"
#include "Classification.h"
#include "ClassificationWidget.h"
#include "DataDescriptorWidget.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "FileDescriptorWidget.h"
#include "GeoreferenceDescriptor.h"
#include "GeoreferenceWidget.h"
#include "ImportDescriptor.h"
#include "Importer.h"
#include "ImportOptionsDlg.h"
#include "MetadataWidget.h"
#include "ModelServices.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "Slot.h"
#include "SpecialMetadata.h"
#include "SubsetWidget.h"
#include "UtilityServices.h"
#include "WavelengthsWidget.h"

#include <algorithm>
#include <queue>
#include <sstream>
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

ImportOptionsDlg::ImportOptionsDlg(Importer* pImporter, const QMap<QString, vector<ImportDescriptor*> >& files,
   QWidget* pParent) :
   QDialog(pParent),
   mpImporter(pImporter),
   mpCurrentDataset(NULL),
   mpEditDescriptor(NULL),
   mEditDataDescriptorModified(false),
   mPromptForChanges(true),
   mAllowDeselectedFiles(true),
   mpDatasetTree(NULL),
   mpClassificationLabel(NULL),
   mpTabWidget(NULL),
   mpDataPage(NULL),
   mpFilePage(NULL),
   mpClassificationPage(NULL),
   mpSubsetPage(NULL),
   mpMetadataPage(NULL),
   mpWavelengthsPage(NULL),
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
   mpDatasetTree->setHeaderHidden(true);

   QPushButton* pImportAllButton = new QPushButton("Import All", pDatasetWidget);
   QPushButton* pImportNoneButton = new QPushButton("Import None", pDatasetWidget);

   // Classification label
   QWidget* pInfoWidget = new QWidget(pSplitter);

   QFont labelFont = font();
   labelFont.setBold(true);

   mpClassificationLabel = new QLabel(pInfoWidget);
   mpClassificationLabel->setFont(labelFont);
   mpClassificationLabel->setAlignment(Qt::AlignCenter);

   // Tab widget
   mpTabWidget = new QTabWidget(pInfoWidget);
   mpDataPage = new DataDescriptorWidget();
   mpFilePage = new FileDescriptorWidget();
   mpClassificationPage = new ClassificationWidget();
   mpSubsetPage = new SubsetWidget();
   mpMetadataPage = new MetadataWidget();
   mpWavelengthsPage = new WavelengthsWidget();
   mpGeoreferencePage = new QWidget();

   mpGeoreferenceCheck = new QCheckBox("Automatically georeference on import", mpGeoreferencePage);
   mpGeoreferenceWidget = new GeoreferenceWidget(this);
   mpGeoreferenceWidget->setEnabled(false);

   // Add the georeference connection here so that the georeference widget is properly enabled
   // when initializing the tab widgets from the edit data descriptor later in the constructor
   VERIFYNR(connect(mpGeoreferenceCheck, SIGNAL(toggled(bool)), this, SLOT(enableGeoreference(bool))));

   QGridLayout* pGeoreferenceLayout = new QGridLayout(mpGeoreferencePage);
   pGeoreferenceLayout->setMargin(10);
   pGeoreferenceLayout->setSpacing(10);
   pGeoreferenceLayout->addWidget(mpGeoreferenceCheck, 0, 0, 1, 2);
   pGeoreferenceLayout->addWidget(mpGeoreferenceWidget, 1, 1);
   pGeoreferenceLayout->setRowStretch(1, 10);
   pGeoreferenceLayout->setColumnStretch(1, 10);

   QStyleOptionButton option;
   option.initFrom(mpGeoreferenceCheck);
   int checkWidth = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option).width();
   pGeoreferenceLayout->setColumnMinimumWidth(0, checkWidth);

   mpTabWidget->addTab(mpDataPage, "Data");
   mpTabWidget->addTab(mpFilePage, "File");
   mpTabWidget->addTab(mpClassificationPage, "Classification");
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

   QLayout* pFileLayout = mpFilePage->layout();
   if (pFileLayout != NULL)
   {
      pFileLayout->setMargin(10);
   }

   QLayout* pClassificationLayout = mpClassificationPage->layout();
   if (pClassificationLayout != NULL)
   {
      pClassificationLayout->setMargin(10);
   }

   QLayout* pSubsetLayout = mpSubsetPage->layout();
   if (pSubsetLayout != NULL)
   {
      pSubsetLayout->setMargin(10);
   }

   QLayout* pMetadataLayout = mpMetadataPage->layout();
   if (pMetadataLayout != NULL)
   {
      pMetadataLayout->setMargin(10);
   }

   QLayout* pWavelengthsLayout = mpWavelengthsPage->layout();
   if (pWavelengthsLayout != NULL)
   {
      pWavelengthsLayout->setMargin(10);
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

   QVBoxLayout* pInfoLayout = new QVBoxLayout(pInfoWidget);
   pInfoLayout->setMargin(0);
   pInfoLayout->setSpacing(10);
   pInfoLayout->addWidget(mpClassificationLabel);
   pInfoLayout->addWidget(mpTabWidget, 10);

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
   resize(810, 500);

   pSplitter->addWidget(pDatasetWidget);
   pSplitter->addWidget(pInfoWidget);

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
   if (pSelectItem == NULL)
   {
      // No data set is set to import by default so select the first data set in the tree widget
      for (int i = 0; i < mpDatasetTree->topLevelItemCount() && pSelectItem == NULL; ++i)
      {
         QTreeWidgetItem* pParentItem = mpDatasetTree->topLevelItem(i);
         if ((pParentItem != NULL) && (pParentItem->childCount() > 0))
         {
            pSelectItem = pParentItem->child(0);
         }
      }
   }

   if (pSelectItem != NULL)
   {
      mpDatasetTree->setItemSelected(pSelectItem, true);
      updateEditDataset();
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
   if ((mpCurrentDataset != NULL) && (mEditDataDescriptorModified == true))
   {
      if (mPromptForChanges == true)
      {
         int iReturn = QMessageBox::question(this, APP_NAME, "Apply changes to data?",
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
      Classification* pClassification = mpEditDescriptor->getClassification();
      if (pClassification != NULL)
      {
         VERIFYNR(pClassification->detach(SIGNAL_NAME(Subject, Modified),
            Slot(this, &ImportOptionsDlg::editClassificationModified)));
      }

      RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(mpEditDescriptor);
      if (pRasterDescriptor != NULL)
      {
         VERIFYNR(pRasterDescriptor->detach(SIGNAL_NAME(RasterDataDescriptor, RowsChanged),
            Slot(this, &ImportOptionsDlg::editDataDescriptorRowsModified)));
         VERIFYNR(pRasterDescriptor->detach(SIGNAL_NAME(RasterDataDescriptor, ColumnsChanged),
            Slot(this, &ImportOptionsDlg::editDataDescriptorColumnsModified)));
         VERIFYNR(pRasterDescriptor->detach(SIGNAL_NAME(RasterDataDescriptor, BandsChanged),
            Slot(this, &ImportOptionsDlg::editDataDescriptorBandsModified)));

         GeoreferenceDescriptor* pGeorefDescriptor = pRasterDescriptor->getGeoreferenceDescriptor();
         if (pGeorefDescriptor != NULL)
         {
            VERIFYNR(pGeorefDescriptor->detach(SIGNAL_NAME(GeoreferenceDescriptor, GeoreferenceOnImportChanged),
               Slot(this, &ImportOptionsDlg::editGeoreferenceOnImportModified)));
         }
      }

      RasterFileDescriptor* pRasterFileDescriptor =
         dynamic_cast<RasterFileDescriptor*>(mpEditDescriptor->getFileDescriptor());
      if (pRasterFileDescriptor != NULL)
      {
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, RowsChanged),
            Slot(this, &ImportOptionsDlg::editFileDescriptorRowsModified)));
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, ColumnsChanged),
            Slot(this, &ImportOptionsDlg::editFileDescriptorColumnsModified)));
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, BandsChanged),
            Slot(this, &ImportOptionsDlg::editFileDescriptorBandsModified)));
      }

      VERIFYNR(mpEditDescriptor->detach(SIGNAL_NAME(Subject, Modified),
         Slot(this, &ImportOptionsDlg::editDataDescriptorModified)));
      pModel->destroyDataDescriptor(mpEditDescriptor);
      mpEditDescriptor = NULL;
      mEditDataDescriptorModified = false;
   }

   // Create a new data descriptor to validate the user inputs
   DataDescriptor* pDescriptor = mpCurrentDataset->getDataDescriptor();
   if (pDescriptor != NULL)
   {
      mpEditDescriptor = pDescriptor->copy();
   }

   VERIFYNRV(mpEditDescriptor != NULL);
   VERIFYNR(mpEditDescriptor->attach(SIGNAL_NAME(Subject, Modified),
      Slot(this, &ImportOptionsDlg::editDataDescriptorModified)));

   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(mpEditDescriptor);
   FileDescriptor* pFileDescriptor = mpEditDescriptor->getFileDescriptor();
   RasterFileDescriptor* pRasterFileDescriptor = dynamic_cast<RasterFileDescriptor*>(pFileDescriptor);

   if (pRasterDescriptor != NULL)
   {
      VERIFYNR(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, RowsChanged),
         Slot(this, &ImportOptionsDlg::editDataDescriptorRowsModified)));
      VERIFYNR(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, ColumnsChanged),
         Slot(this, &ImportOptionsDlg::editDataDescriptorColumnsModified)));
      VERIFYNR(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, BandsChanged),
         Slot(this, &ImportOptionsDlg::editDataDescriptorBandsModified)));

      GeoreferenceDescriptor* pGeorefDescriptor = pRasterDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         VERIFYNR(pGeorefDescriptor->attach(SIGNAL_NAME(GeoreferenceDescriptor, GeoreferenceOnImportChanged),
            Slot(this, &ImportOptionsDlg::editGeoreferenceOnImportModified)));
      }
   }

   if (pRasterFileDescriptor != NULL)
   {
      VERIFYNR(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, RowsChanged),
         Slot(this, &ImportOptionsDlg::editFileDescriptorRowsModified)));
      VERIFYNR(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, ColumnsChanged),
         Slot(this, &ImportOptionsDlg::editFileDescriptorColumnsModified)));
      VERIFYNR(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, BandsChanged),
         Slot(this, &ImportOptionsDlg::editFileDescriptorBandsModified)));
   }

   // Select the tree widget item for the current data set
   selectCurrentDatasetItem();

   // Disconnect pages
   updateConnections(false);

   // Subset page
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
      const vector<DimensionDescriptor>& selectedBands = pRasterDescriptor->getBands();
      setSubsetBands(bands, selectedBands);

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
   bool editFilePage = false;
   if (pRasterFileDescriptor != NULL)
   {
      unsigned int numRows = pRasterFileDescriptor->getRowCount();
      unsigned int numColumns = pRasterFileDescriptor->getColumnCount();
      unsigned int bitsPerElement = pRasterFileDescriptor->getBitsPerElement();
      unsigned int numBands = pRasterFileDescriptor->getBandCount();
      if ((numRows == 0) || (numColumns == 0) || (numBands == 0) || (bitsPerElement == 0))
      {
         editFilePage = true;
      }
   }

   mpFilePage->setFileDescriptor(pFileDescriptor, editFilePage);

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

   // Classification page
   updateClassificationLabel();

   Classification* pClassification = mpEditDescriptor->getClassification();
   if (pClassification != NULL)
   {
      VERIFYNR(pClassification->attach(SIGNAL_NAME(Subject, Modified),
         Slot(this, &ImportOptionsDlg::editClassificationModified)));
      mpClassificationPage->setClassification(pClassification);
   }

   // Metadata page
   mpMetadataPage->setMetadata(mpEditDescriptor->getMetadata());

   // Wavelengths page
   bool bWavelengthsPageActive = false;
   if (mpTabWidget->currentWidget() == mpWavelengthsPage)
   {
      bWavelengthsPageActive = true;
   }

   int index = mpTabWidget->indexOf(mpWavelengthsPage);
   if (index != -1)
   {
      mpTabWidget->removeTab(index);
   }

   if (pRasterFileDescriptor != NULL)
   {
      // Populate the wavelengths with the file descriptor bands since the metadata wavelengths
      // apply to all bands in the file
      mpWavelengthsPage->setWavelengths(pRasterFileDescriptor->getBands(), mpEditDescriptor->getMetadata());

      if (pRasterDescriptor != NULL)
      {
         mpWavelengthsPage->highlightActiveBands(pRasterDescriptor->getBands());
      }

      mpTabWidget->addTab(mpWavelengthsPage, "Wavelengths");

      if (bWavelengthsPageActive == true)
      {
         mpTabWidget->setCurrentWidget(mpWavelengthsPage);
      }
   }

   // Georeference page
   bool georeferencePageActive = false;
   if (mpTabWidget->currentWidget() == mpGeoreferencePage)
   {
      georeferencePageActive = true;
   }

   index = mpTabWidget->indexOf(mpGeoreferencePage);
   if (index != -1)
   {
      mpTabWidget->removeTab(index);
   }

   if (pRasterDescriptor != NULL)
   {
      bool georeference = GeoreferenceDescriptor::getSettingAutoGeoreference();

      const GeoreferenceDescriptor* pGeorefDescriptor = pRasterDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         georeference = pGeorefDescriptor->getGeoreferenceOnImport();
      }

      mpGeoreferenceCheck->setChecked(georeference);
      mpGeoreferenceWidget->setDataDescriptor(pRasterDescriptor);

      mpTabWidget->addTab(mpGeoreferencePage, "Georeference");

      if (georeferencePageActive == true)
      {
         mpTabWidget->setCurrentWidget(mpGeoreferencePage);
      }
   }

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
      mpImporterPage = mpImporter->getImportOptionsWidget(mpEditDescriptor);
      if (mpImporterPage != NULL)
      {
         QLayout* pLayout = mpImporterPage->layout();
         if (pLayout != NULL)
         {
            if (pLayout->margin() <= 0)
            {
               pLayout->setMargin(10);
            }
         }

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

      // Set the valid processing locations on the data page.  This must be done after getting the import options
      // widget from the importer so that the auto importer will correctly query the importer that is used.
      // This can be changed if the importer design (and auto importer) is modified to support valid processing
      // locations for a specific data set.
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

   // Set a georeference plug-in in the georeference descriptor if one has not yet been set
   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(pDescriptor);
   if (pRasterDescriptor != NULL)
   {
      const GeoreferenceDescriptor* pGeorefDescriptor = pRasterDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         const string& plugInName = pGeorefDescriptor->getGeoreferencePlugInName();
         if (plugInName.empty() == true)
         {
            pRasterDescriptor->setDefaultGeoreferencePlugIn();
         }
      }
   }

   // Validate the user inputs from the importer
   bool validDataset = false;
   if (mpImporter != NULL)
   {
      vector<const DataDescriptor*> importedDataDescriptors;
      for (map<ImportDescriptor*, QTreeWidgetItem*>::const_iterator iter = mDatasets.begin();
         iter != mDatasets.end();
         ++iter)
      {
         ImportDescriptor* pImportDescriptor = iter->first;
         QTreeWidgetItem* pItem = iter->second;
         if ((pImportDescriptor != NULL) && (pItem != NULL) && (pItem->checkState(0) == Qt::Checked))
         {
            DataDescriptor* pDataDescriptor = pImportDescriptor->getDataDescriptor();
            if (pImportDescriptor == mpCurrentDataset)
            {
               pDataDescriptor = mpEditDescriptor;
            }

            if (pDataDescriptor != NULL)
            {
               importedDataDescriptors.push_back(pDataDescriptor);
            }
         }
      }

      string errorMessage;
      validDataset = mpImporter->validate(pDescriptor, importedDataDescriptors, errorMessage);
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
         bool disconnected = disconnect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
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

         if (disconnected)
         {
            //only connect, if the signal was originally connected
            VERIFYNR(connect(mpDatasetTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
               SLOT(datasetItemChanged(QTreeWidgetItem*))));
         }
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
         bool disconnected = disconnect(mpDatasetTree, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateEditDataset()));
         mpDatasetTree->setCurrentItem(pItem);     // Also selects the item
         if (disconnected)
         {
            //only connect, if the signal was originally connected
            VERIFYNR(connect(mpDatasetTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateEditDataset())));
         }
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

void ImportOptionsDlg::validateAllDatasets()
{
   for (map<ImportDescriptor*, QTreeWidgetItem*>::const_iterator iter = mDatasets.begin();
      iter != mDatasets.end();
      ++iter)
   {
      // Must validate the current data set last using the edit descriptor to properly validate any local
      // changes made by the user and to properly update the OK button and validation label message
      ImportDescriptor* pImportDescriptor = iter->first;
      if ((pImportDescriptor != NULL) && (pImportDescriptor != mpCurrentDataset))
      {
         DataDescriptor* pDataDescriptor = pImportDescriptor->getDataDescriptor();
         if (pDataDescriptor != NULL)
         {
            validateDataset(pDataDescriptor);
         }
      }
   }

   // Validate the current data set, which also updates the dialog OK button and validation label message
   validateEditDataset();
}

namespace
{
   void recursivelySetChildren(QTreeWidgetItem* pItem, int column, Qt::CheckState newState)
   {
      if (pItem == NULL)
      {
         return;
      }
      for (int i = 0; i < pItem->childCount(); ++i)
      {
         QTreeWidgetItem* pChild = pItem->child(i);
         if (pChild == NULL)
         {
            continue;
         }
         pChild->setCheckState(column, newState);
         recursivelySetChildren(pChild, column, newState);
      }
   }
};

void ImportOptionsDlg::enforceSelections(QTreeWidgetItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   if (pItem->parent() == NULL && pItem->checkState(0) == Qt::Checked)
   {
      //for top-level items (files), if it becomes checked, then check
      //all items under the file.
      recursivelySetChildren(pItem, 0, Qt::Checked);
      return;
   }

   // uncheck all children below this one that was unchecked recursively
   if (pItem->checkState(0) == Qt::Unchecked)
   {
      recursivelySetChildren(pItem, 0, Qt::Unchecked);

      //look for a dataset directly under a file that just became unchecked
      QTreeWidgetItem* pParent = pItem->parent();
      if (pParent != NULL)
      {
         QTreeWidgetItem* pGrandParent = pParent->parent();
         if (pGrandParent == NULL)
         {
            //let's determine if the siblings are unchecked, if so, we should
            //uncheck the file.
            bool uncheckParent = true;
            for (int i = 0; i < pParent->childCount(); ++i)
            {
               QTreeWidgetItem* pSibling = pParent->child(i);
               if (pSibling == NULL)
               {
                  continue;
               }
               if (pSibling->checkState(0) != Qt::Unchecked)
               {
                  uncheckParent = false;
                  break;
               }
            }
            if (uncheckParent)
            {
               pParent->setCheckState(0, Qt::Unchecked);
            }
         }
      }
   }
   else
   {
      // if becoming checked or partially checked, then force all ancestors to be checked.
      QTreeWidgetItem* pWorkingItem = pItem;
      for (; pWorkingItem != NULL; pWorkingItem = pWorkingItem->parent())
      {
         pWorkingItem->setCheckState(0, Qt::Checked);
      }
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

void ImportOptionsDlg::editDataDescriptorModified(Subject& subject, const string& signal, const boost::any& value)
{
   mEditDataDescriptorModified = true;
   validateAllDatasets();
}

void ImportOptionsDlg::editDataDescriptorRowsModified(Subject& subject, const string& signal, const boost::any& value)
{
   if ((mpEditDescriptor == NULL) || (dynamic_cast<DataDescriptor*>(&subject) != mpEditDescriptor))
   {
      return;
   }

   // Update the rows marked to import
   const vector<DimensionDescriptor>& importedRows = boost::any_cast<vector<DimensionDescriptor> >(value);

   // Subset page
   RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(mpEditDescriptor->getFileDescriptor());
   if (pFileDescriptor != NULL)
   {
      const vector<DimensionDescriptor>& rows = pFileDescriptor->getRows();
      mpSubsetPage->setRows(rows, importedRows);
   }
}

void ImportOptionsDlg::editDataDescriptorColumnsModified(Subject& subject, const string& signal,
                                                         const boost::any& value)
{
   if ((mpEditDescriptor == NULL) || (dynamic_cast<DataDescriptor*>(&subject) != mpEditDescriptor))
   {
      return;
   }

   // Update the columns marked to import
   const vector<DimensionDescriptor>& importedColumns = boost::any_cast<vector<DimensionDescriptor> >(value);

   // Subset page
   RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(mpEditDescriptor->getFileDescriptor());
   if (pFileDescriptor != NULL)
   {
      const vector<DimensionDescriptor>& columns = pFileDescriptor->getColumns();
      mpSubsetPage->setColumns(columns, importedColumns);
   }
}

void ImportOptionsDlg::editDataDescriptorBandsModified(Subject& subject, const string& signal, const boost::any& value)
{
   if ((mpEditDescriptor == NULL) || (dynamic_cast<DataDescriptor*>(&subject) != mpEditDescriptor))
   {
      return;
   }

   // Update the bands marked to import
   const vector<DimensionDescriptor>& importedBands = boost::any_cast<vector<DimensionDescriptor> >(value);

   // Subset page
   RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(mpEditDescriptor->getFileDescriptor());
   if (pFileDescriptor != NULL)
   {
      const vector<DimensionDescriptor>& bands = pFileDescriptor->getBands();
      setSubsetBands(bands, importedBands);
   }

   // Wavelengths page
   mpWavelengthsPage->highlightActiveBands(importedBands);
}

void ImportOptionsDlg::editFileDescriptorRowsModified(Subject& subject, const string& signal, const boost::any& value)
{
   // Update the total number of rows
   vector<DimensionDescriptor> rows = boost::any_cast<vector<DimensionDescriptor> >(value);

   // Subset page
   mpSubsetPage->setRows(rows, vector<DimensionDescriptor>());
}

void ImportOptionsDlg::editFileDescriptorColumnsModified(Subject& subject, const string& signal,
                                                         const boost::any& value)
{
   // Update the total number of columns
   vector<DimensionDescriptor> columns = boost::any_cast<vector<DimensionDescriptor> >(value);

   // Subset page
   mpSubsetPage->setColumns(columns, vector<DimensionDescriptor>());
}

void ImportOptionsDlg::editFileDescriptorBandsModified(Subject& subject, const string& signal, const boost::any& value)
{
   // Update the total number of bands, resetting the imported bands to all bands
   vector<DimensionDescriptor> bands = boost::any_cast<vector<DimensionDescriptor> >(value);

   // Subset page
   mpSubsetPage->setBands(bands);

   // Wavelengths page
   if (mpEditDescriptor != NULL)
   {
      mpWavelengthsPage->setWavelengths(bands, mpEditDescriptor->getMetadata());
      mpWavelengthsPage->highlightActiveBands(bands);
   }
}

void ImportOptionsDlg::editClassificationModified(Subject& subject, const string& signal, const boost::any& value)
{
   updateClassificationLabel();
}

void ImportOptionsDlg::editGeoreferenceOnImportModified(Subject& subject, const string& signal, const boost::any& value)
{
   mpGeoreferenceCheck->setChecked(boost::any_cast<bool>(value));
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

   // This method is called when any data set import state changes, so validate
   // all data sets again with an updated vector of imported data sets
   validateAllDatasets();

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
      bool autoScroll = mpDatasetTree->hasAutoScroll();
      mpDatasetTree->setAutoScroll(false);
      selectCurrentDatasetItem();
      mpDatasetTree->setAutoScroll(autoScroll);
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

void ImportOptionsDlg::updateEditDataDescriptorRows(const vector<DimensionDescriptor>& rows)
{
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpEditDescriptor);
   if (pDescriptor != NULL)
   {
      pDescriptor->setRows(rows);
   }
}

void ImportOptionsDlg::updateEditDataDescriptorColumns(const vector<DimensionDescriptor>& columns)
{
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpEditDescriptor);
   if (pDescriptor != NULL)
   {
      pDescriptor->setColumns(columns);
   }
}

void ImportOptionsDlg::updateEditDataDescriptorBands(const vector<DimensionDescriptor>& bands)
{
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpEditDescriptor);
   if (pDescriptor != NULL)
   {
      pDescriptor->setBands(bands);
   }
}

void ImportOptionsDlg::updateClassificationLabel()
{
   if (mpEditDescriptor == NULL)
   {
      return;
   }

   QString strClassification;
   string classificationLevel;
   const Classification* pClassification = mpEditDescriptor->getClassification();
   if (pClassification != NULL)
   {
      string classificationText = "";
      classificationLevel = pClassification->getLevel();
      pClassification->getClassificationText(classificationText);
      strClassification = QString::fromStdString(classificationText);
   }

   if (strClassification.isEmpty() == true)
   {
      Service<UtilityServices> pUtilities;
      strClassification = QString::fromStdString(pUtilities->getDefaultClassification());
   }

   QPalette labelPalette = palette();
   if (strClassification.isEmpty() == false)
   {
      // Text color
      labelPalette.setColor(QPalette::WindowText, Qt::white);

      // Background color
      labelPalette.setColor(QPalette::Window, Qt::darkYellow);    // Default to background color used for TS

      if ((classificationLevel == "C") || (classificationLevel == "R"))
      {
         labelPalette.setColor(QPalette::Window, Qt::darkBlue);
      }
      else if (classificationLevel == "S")
      {
         labelPalette.setColor(QPalette::Window, Qt::darkRed);
      }
      else if (classificationLevel == "U")
      {
         labelPalette.setColor(QPalette::Window, Qt::darkGreen);
      }
   }
   else
   {
      labelPalette.setColor(QPalette::WindowText, Qt::black);
   }

   mpClassificationLabel->setPalette(labelPalette);
   mpClassificationLabel->setAutoFillBackground(!strClassification.isEmpty());
   mpClassificationLabel->setText(strClassification);
}

void ImportOptionsDlg::enableGeoreference(bool enable)
{
   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(mpEditDescriptor);
   if (pRasterDescriptor != NULL)
   {
      GeoreferenceDescriptor* pGeorefDescriptor = pRasterDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         pGeorefDescriptor->setGeoreferenceOnImport(enable);
      }
   }

   mpGeoreferenceWidget->setEnabled(enable);
}

bool ImportOptionsDlg::applyChanges()
{
   if (mpEditDescriptor == NULL)
   {
      return false;
   }

   ImportDescriptor* pDataset = getCurrentDataset();
   if (pDataset != NULL)
   {
      Classification* pClassification = mpEditDescriptor->getClassification();
      if (pClassification != NULL)
      {
         VERIFYNR(pClassification->detach(SIGNAL_NAME(Subject, Modified),
            Slot(this, &ImportOptionsDlg::editClassificationModified)));
      }

      RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(mpEditDescriptor);
      if (pRasterDescriptor != NULL)
      {
         VERIFYNR(pRasterDescriptor->detach(SIGNAL_NAME(RasterDataDescriptor, RowsChanged),
            Slot(this, &ImportOptionsDlg::editDataDescriptorRowsModified)));
         VERIFYNR(pRasterDescriptor->detach(SIGNAL_NAME(RasterDataDescriptor, ColumnsChanged),
            Slot(this, &ImportOptionsDlg::editDataDescriptorColumnsModified)));
         VERIFYNR(pRasterDescriptor->detach(SIGNAL_NAME(RasterDataDescriptor, BandsChanged),
            Slot(this, &ImportOptionsDlg::editDataDescriptorBandsModified)));

         GeoreferenceDescriptor* pGeorefDescriptor = pRasterDescriptor->getGeoreferenceDescriptor();
         if (pGeorefDescriptor != NULL)
         {
            VERIFYNR(pGeorefDescriptor->detach(SIGNAL_NAME(GeoreferenceDescriptor, GeoreferenceOnImportChanged),
               Slot(this, &ImportOptionsDlg::editGeoreferenceOnImportModified)));
         }
      }

      RasterFileDescriptor* pRasterFileDescriptor =
         dynamic_cast<RasterFileDescriptor*>(mpEditDescriptor->getFileDescriptor());
      if (pRasterFileDescriptor != NULL)
      {
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, RowsChanged),
            Slot(this, &ImportOptionsDlg::editFileDescriptorRowsModified)));
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, ColumnsChanged),
            Slot(this, &ImportOptionsDlg::editFileDescriptorColumnsModified)));
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, BandsChanged),
            Slot(this, &ImportOptionsDlg::editFileDescriptorBandsModified)));
      }

      VERIFYNR(mpEditDescriptor->detach(SIGNAL_NAME(Subject, Modified),
         Slot(this, &ImportOptionsDlg::editDataDescriptorModified)));
      pDataset->setDataDescriptor(mpEditDescriptor);
      mpEditDescriptor = NULL;
      mEditDataDescriptorModified = false;
      return true;
   }

   return false;
}

void ImportOptionsDlg::setSubsetBands(const vector<DimensionDescriptor>& bands,
                                      const vector<DimensionDescriptor>& selectedBands)
{
   if ((mpEditDescriptor == NULL) || (mpSubsetPage == NULL))
   {
      return;
   }

   // Note: The band names here cannot use the RasterUtilities::getBandNames() method because that method uses a
   // RasterDataDescriptor (cf. a RasterFileDescriptor) to determine band names, meaning that bad bands will not
   // have their names included in the list of band names. If bad bands are not included in the list of band names,
   // then the SubsetWidget will have a mismatch between the number of band names and the number of bands specified
   // if bad bands are set by the importer. If SubsetWidget has a mismatch between these two numbers, then no band
   // names are displayed to the user.
   // In summary, if we change this to use RasterUtilities::getBandNames(), then the SubsetWidget will ignore band
   // names and replace them with "Band 1", ..., "Band n" for any image with bad bands set by its importer.
   vector<string> bandNames;

   const DynamicObject* pMetadata = mpEditDescriptor->getMetadata();
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

   mpSubsetPage->setBands(bands, bandNames, selectedBands);
}

void ImportOptionsDlg::updateConnections(bool bConnect)
{
   if (bConnect == true)
   {
      VERIFYNR(connect(mpSubsetPage, SIGNAL(subsetRowsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateEditDataDescriptorRows(const std::vector<DimensionDescriptor>&))));
      VERIFYNR(connect(mpSubsetPage, SIGNAL(subsetColumnsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateEditDataDescriptorColumns(const std::vector<DimensionDescriptor>&))));
      VERIFYNR(connect(mpSubsetPage, SIGNAL(subsetBandsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateEditDataDescriptorBands(const std::vector<DimensionDescriptor>&))));
   }
   else
   {
      disconnect(mpSubsetPage, SIGNAL(subsetRowsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateEditDataDescriptorRows(const std::vector<DimensionDescriptor>&)));
      disconnect(mpSubsetPage, SIGNAL(subsetColumnsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateEditDataDescriptorColumns(const std::vector<DimensionDescriptor>&)));
      disconnect(mpSubsetPage, SIGNAL(subsetBandsChanged(const std::vector<DimensionDescriptor>&)), this,
         SLOT(updateEditDataDescriptorBands(const std::vector<DimensionDescriptor>&)));
   }
}
