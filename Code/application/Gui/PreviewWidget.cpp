/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtGui/QApplication>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

#include "PreviewWidget.h"
#include "ChippingWidget.h"
#include "AppVerify.h"
#include "FileDescriptor.h"
#include "ImportDescriptor.h"
#include "Importer.h"
#include "PlugIn.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "Slot.h"
#include "SpatialDataView.h"

#include <string>
using namespace std;

PreviewWidget::PreviewWidget(QWidget* parent) :
   QWidget(parent),
   mpStack(NULL),
   mpProgressLabel(NULL),
   mpProgressBar(NULL),
   mpPreview(NULL),
   mpDatasetLabel(NULL),
   mpAllDatasetsLabel(NULL),
   mpPreviewDatasetsLabel(NULL),
   mpBackButton(NULL),
   mpNextButton(NULL),
   mpImporterWidget(NULL),
   mpImporter(NULL),
   mpActiveDataset(NULL)
{
   // 'No preview available' widget
   QLabel* pNoPreviewLabel = new QLabel("No preview is available.");
   pNoPreviewLabel->setAlignment(Qt::AlignCenter);
   pNoPreviewLabel->setWordWrap(true);

   // Progress widget
   QFrame* pProgressBox = new QFrame();

   mpProgressLabel = new QLabel(pProgressBox);
   mpProgressLabel->setWordWrap(true);

   mpProgressBar = new QProgressBar(pProgressBox);

   QVBoxLayout* pProgressLayout = new QVBoxLayout(pProgressBox);
   pProgressLayout->setMargin(0);
   pProgressLayout->setSpacing(5);
   pProgressLayout->addWidget(mpProgressLabel, 0, Qt::AlignBottom);
   pProgressLayout->addWidget(mpProgressBar, 0, Qt::AlignTop);

   // Preview widget
   mpPreview = new QWidget();
   mpDatasetLabel = new QLabel(mpPreview);
   mpAllDatasetsLabel = new QLabel(mpPreview);
   mpPreviewDatasetsLabel = new QLabel(mpPreview);

   mpBackButton = new QToolButton(mpPreview);
   mpBackButton->setArrowType(Qt::LeftArrow);
   mpBackButton->setFixedSize(14, 14);
   mpBackButton->setEnabled(false);

   mpNextButton = new QToolButton(mpPreview);
   mpNextButton->setArrowType(Qt::RightArrow);
   mpNextButton->setFixedSize(14, 14);
   mpNextButton->setEnabled(false);

   QGridLayout* pPreviewGrid = new QGridLayout(mpPreview);
   pPreviewGrid->setMargin(0);
   pPreviewGrid->setSpacing(5);
   pPreviewGrid->addWidget(mpDatasetLabel, 0, 0, 1, 3);
   pPreviewGrid->addWidget(mpBackButton, 0, 3);
   pPreviewGrid->addWidget(mpNextButton, 0, 4);
   pPreviewGrid->addWidget(mpAllDatasetsLabel, 1, 0, Qt::AlignLeft);
   pPreviewGrid->addWidget(mpPreviewDatasetsLabel, 1, 2, 1, 3, Qt::AlignRight);
   pPreviewGrid->setRowStretch(2, 10);
   pPreviewGrid->setColumnStretch(2, 10);
   pPreviewGrid->setColumnMinimumWidth(1, 15);

   // Widget stack
   mpStack = new QStackedWidget(this);
   mpStack->addWidget(pNoPreviewLabel);
   mpStack->addWidget(pProgressBox);
   mpStack->addWidget(mpPreview);
   mpStack->setCurrentWidget(pNoPreviewLabel);

   // Layout
   QHBoxLayout* pLayout = new QHBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(0);
   pLayout->addSpacing(5);
   pLayout->addWidget(mpStack, 10);

   // Initialization
   setMinimumWidth(260);

   // Connections
   VERIFYNR(connect(mpBackButton, SIGNAL(clicked()), this, SLOT(displayPreviousPreview())));
   VERIFYNR(connect(mpNextButton, SIGNAL(clicked()), this, SLOT(displayNextPreview())));
}

PreviewWidget::~PreviewWidget()
{
   destroyPreview();
}

void PreviewWidget::progressUpdated(Subject &subject, const string &signal, const boost::any &data)
{
   Progress* pProgress = dynamic_cast<Progress*>(&subject);
   if (NN(pProgress))
   {
      // Get the current Progress values
      string label = "";
      int iPercent = 0;
      ReportingLevel reportLevel = NORMAL;
      pProgress->getProgress(label, iPercent, reportLevel);

      // Set the label text if there is new text
      QString strOldLabel = mpProgressLabel->text();
      QString strLabel;
      if (label.empty() == false)
      {
         strLabel = QString::fromStdString(label);
      }

      if (strLabel != strOldLabel)
      {
         mpProgressLabel->setText(strLabel);
         mpProgressLabel->repaint();
      }

      // Set the progress percentage only if it is a new value
      int iOldPercent = mpProgressBar->value();
      if (iPercent != iOldPercent)
      {
         mpProgressBar->setValue(iPercent);
      }
   }
}

void PreviewWidget::setImporter(Importer* pImporter)
{
   mpImporter = pImporter;
}

void PreviewWidget::setDatasets(const vector<ImportDescriptor*>& datasets)
{
   if (datasets != mDatasets)
   {
      mDatasets = datasets;

      // Enable the buttons to cycle through the imported data sets
      unsigned int numImportedDatasets = getNumImportedDatasets();
      mpBackButton->setEnabled(numImportedDatasets > 1);
      mpNextButton->setEnabled(numImportedDatasets > 1);

      // Reset the active dataset
      setActiveDataset(NULL);
   }
}

void PreviewWidget::setActiveDataset(ImportDescriptor* pDataset)
{
   ImportDescriptor* pActiveDataset = getActiveDataset();
   if (pDataset == pActiveDataset)
   {
      return;
   }

   mpActiveDataset = NULL;

   // Delete the current preview
   destroyPreview();

   // Show label indicating no preview is available
   mpStack->setCurrentIndex(0);

   // Check for no active data set
   if (pDataset == NULL)
   {
      emit activeDatasetChanged(NULL);
      return;
   }

   // Only show the preview if the widget is visible or if the data set is imported
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pDataset->getDataDescriptor());
   if ((isVisible() == false) || (pDataset->isImported() == false)|| (pDescriptor == NULL))
   {
      if (pActiveDataset != NULL)
      {
         emit activeDatasetChanged(NULL);
      }

      return;
   }

   if (mpImporter != NULL)
   {
      Service<PlugInManagerServices> pManager;

      Progress* pProgress = pManager->getProgress(dynamic_cast<PlugIn*>(mpImporter));
      if (pProgress != NULL)
      {
         pProgress->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &PreviewWidget::progressUpdated));
         pProgress->updateProgress("Getting preview...", 0, NORMAL);
      }

      // Activate the progress bar
      mpStack->setCurrentIndex(1);

      // Proces events to erase the no preview available page
      qApp->processEvents();

      // Get the preview from the importer
      mpImporterWidget = mpImporter->getPreview(pDescriptor, pProgress);
      if (mpImporterWidget != NULL)
      {
         // Set the current data set location in the file
         QString strDatasetLocation = "Data set location unavailable!";

         const FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
         if (pFileDescriptor != NULL)
         {
            const string& datasetLocation = pFileDescriptor->getDatasetLocation();
            if (datasetLocation.empty() == false)
            {
               strDatasetLocation = "Data set: " + QString::fromStdString(datasetLocation);
            }
            else
            {
               const string& filename = pFileDescriptor->getFilename();
               if (filename.empty() == false)
               {
                  QFileInfo fileInfo(QString::fromStdString(filename));
                  if ((fileInfo.isFile() == true) && (fileInfo.exists() == true))
                  {
                     strDatasetLocation = "Data set: " + fileInfo.fileName();
                  }
               }
            }
         }

         mpDatasetLabel->setText(strDatasetLocation);

         // Update the data set reference labels
         unsigned int iIndex = 0;
         unsigned int numDatasets = mDatasets.size();
         for (iIndex = 0; iIndex < numDatasets; ++iIndex)
         {
            ImportDescriptor* pCurrentDataset = mDatasets[iIndex];
            if (pCurrentDataset == pDataset)
            {
               break;
            }
         }

         mpAllDatasetsLabel->setText("Data set in file: " + QString::number(iIndex + 1) + " of " +
            QString::number(numDatasets));

         numDatasets = getNumImportedDatasets();
         iIndex = 0;
         for (vector<ImportDescriptor*>::const_iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
         {
            ImportDescriptor* pCurrentDataset = *iter;
            if (pCurrentDataset != NULL)
            {
               if (pCurrentDataset == pDataset)
               {
                  break;
               }

               if (pCurrentDataset->isImported() == true)
               {
                  iIndex++;
               }
            }
         }

         mpPreviewDatasetsLabel->setText("Data set to import: " + QString::number(iIndex + 1) + " of " +
            QString::number(numDatasets));

         // Display the preview widget
         SpatialDataView* pView = dynamic_cast<SpatialDataView*>(mpImporterWidget);
         if (pView != NULL)
         {
            ChippingWidget* pChippingWidget = new ChippingWidget(pView, mpPreview);
            VERIFYNRV(pChippingWidget != NULL);

            VERIFYNR(connect(pChippingWidget, SIGNAL(chipChanged()), this, SLOT(updateActiveDataset())));
            mpImporterWidget = pChippingWidget;
         }
         else
         {
            mpImporterWidget->setParent(mpPreview);
         }

         QGridLayout* pGrid = dynamic_cast<QGridLayout*>(mpPreview->layout());
         if (pGrid != NULL)
         {
            pGrid->addWidget(mpImporterWidget, 2, 0, 1, 5);
         }

         // Activate the preview page in the widget stack
         mpStack->setCurrentIndex(2);

         mpActiveDataset = pDataset;
         emit activeDatasetChanged(mpActiveDataset);
      }
      else
      {
         mpStack->setCurrentIndex(0);

         if (pActiveDataset != NULL)
         {
            emit activeDatasetChanged(NULL);
         }
      }

      if (pProgress != NULL)
      {
         pProgress->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &PreviewWidget::progressUpdated));
      }
   }
}

ImportDescriptor* PreviewWidget::getActiveDataset() const
{
   return mpActiveDataset;
}

unsigned int PreviewWidget::getNumImportedDatasets() const
{
   unsigned int numImportedDatasets = 0;
   for (vector<ImportDescriptor*>::const_iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      ImportDescriptor* pDataset = *iter;
      if (pDataset != NULL)
      {
         if (pDataset->isImported() == true)
         {
            numImportedDatasets++;
         }
      }
   }

   return numImportedDatasets;
}

void PreviewWidget::displayPreviousPreview()
{
   // Get the descriptor of the previous imported data set in the vector
   ImportDescriptor* pActiveDataset = getActiveDataset();
   ImportDescriptor* pPreviousDataset = NULL;
   ImportDescriptor* pLastDataset = NULL;
   bool bNextDataset = (pActiveDataset == NULL);

   vector<ImportDescriptor*>::reverse_iterator iter;
   for (iter = mDatasets.rbegin(); iter != mDatasets.rend(); ++iter)
   {
      ImportDescriptor* pDataset = *iter;
      if ((pDataset != NULL) && (pDataset->isImported() == true))
      {
         if (pLastDataset == NULL)
         {
            pLastDataset = pDataset;
         }

         if (bNextDataset == true)
         {
            pPreviousDataset = pDataset;
            break;
         }

         if (pDataset == pActiveDataset)
         {
            bNextDataset = true;
         }
      }
   }

   // The previous imported data set was not found so cycle around to the last imported data set
   if (pPreviousDataset == NULL)
   {
      pPreviousDataset = pLastDataset;
   }

   // Update the active data set
   setActiveDataset(pPreviousDataset);
}

void PreviewWidget::displayNextPreview()
{
   // Get the descriptor of the next imported data set in the vector
   ImportDescriptor* pActiveDataset = getActiveDataset();
   ImportDescriptor* pNextDataset = NULL;
   ImportDescriptor* pFirstDataset = NULL;
   bool bNextDataset = (pActiveDataset == NULL);

   for (vector<ImportDescriptor*>::iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      ImportDescriptor* pDataset = *iter;
      if ((pDataset != NULL) && (pDataset->isImported() == true))
      {
         if (pFirstDataset == NULL)
         {
            pFirstDataset = pDataset;
         }

         if (bNextDataset == true)
         {
            pNextDataset = pDataset;
            break;
         }

         if (pDataset == pActiveDataset)
         {
            bNextDataset = true;
         }
      }
   }

   // The next imported data set was not found so cycle around to the first imported data set
   if (pNextDataset == NULL)
   {
      pNextDataset = pFirstDataset;
   }

   // Update the active data set
   setActiveDataset(pNextDataset);
}

void PreviewWidget::destroyPreview()
{
   // Delete the preview widget
   if (mpImporterWidget != NULL)
   {
      delete mpImporterWidget;
      mpImporterWidget = NULL;
   }
}

class UnsetActiveNumber
{
public:
   DimensionDescriptor operator()(const DimensionDescriptor& val)
   {
      DimensionDescriptor temp = val;
      temp.setActiveNumber(0);
      temp.setActiveNumberValid(false);
      return temp;
   }
};

void PreviewWidget::updateActiveDataset()
{
   ChippingWidget* pChippingWidget = dynamic_cast<ChippingWidget*>(mpImporterWidget);
   if (pChippingWidget == NULL)
   {
      return;
   }

   ImportDescriptor* pActiveDataset = getActiveDataset();
   if (pActiveDataset != NULL)
   {
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pActiveDataset->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         // Rows
         vector<DimensionDescriptor> rows = pChippingWidget->getChipRows();
         transform(rows.begin(), rows.end(), rows.begin(), UnsetActiveNumber());
         pDescriptor->setRows(rows);

         // Columns
         vector<DimensionDescriptor> columns = pChippingWidget->getChipColumns();
         transform(columns.begin(), columns.end(), columns.begin(), UnsetActiveNumber());
         pDescriptor->setColumns(columns);
      }
   }
}
