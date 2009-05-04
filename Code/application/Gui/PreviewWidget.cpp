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

#include "AppVerify.h"
#include "ChippingWidget.h"
#include "FileDescriptor.h"
#include "ImportDescriptor.h"
#include "Importer.h"
#include "PlugIn.h"
#include "PlugInManagerServices.h"
#include "PreviewWidget.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "Slot.h"
#include "SpatialDataView.h"

using namespace std;

PreviewWidget::PreviewWidget(QWidget* pParent) :
   QWidget(pParent),
   mpStack(NULL),
   mpFileLabel(NULL),
   mpPreviousFileButton(NULL),
   mpNextFileButton(NULL),
   mpFileStack(NULL),
   mpDatasetLabel(NULL),
   mpPreviousDatasetButton(NULL),
   mpNextDatasetButton(NULL),
   mpDatasetStack(NULL),
   mpProgressLabel(NULL),
   mpProgressBar(NULL),
   mpPreview(NULL),
   mpImporterWidget(NULL),
   mpImporter(NULL),
   mpCurrentDataset(NULL)
{
   // 'No preview available' widget
   QLabel* pNoPreviewLabel = new QLabel("No preview available.");
   pNoPreviewLabel->setAlignment(Qt::AlignCenter);
   pNoPreviewLabel->setWordWrap(true);

   // File widgets
   QWidget* pFileWidget = new QWidget();
   mpFileLabel = new QLabel("<b>File:</b>", pFileWidget);

   mpPreviousFileButton = new QToolButton(pFileWidget);
   mpPreviousFileButton->setArrowType(Qt::LeftArrow);
   mpPreviousFileButton->setFixedSize(15, 15);
   mpPreviousFileButton->setEnabled(false);

   mpNextFileButton = new QToolButton(pFileWidget);
   mpNextFileButton->setArrowType(Qt::RightArrow);
   mpNextFileButton->setFixedSize(15, 15);
   mpNextFileButton->setEnabled(false);

   QLabel* pNoFilePreviewLabel = new QLabel("No preview available for this file.");
   pNoFilePreviewLabel->setAlignment(Qt::AlignCenter);
   pNoFilePreviewLabel->setWordWrap(true);

   // Data set widgets
   QWidget* pDatasetWidget = new QWidget();
   mpDatasetLabel = new QLabel("<b>Data Set:</b>", pDatasetWidget);

   mpPreviousDatasetButton = new QToolButton(pDatasetWidget);
   mpPreviousDatasetButton->setArrowType(Qt::LeftArrow);
   mpPreviousDatasetButton->setFixedSize(15, 15);
   mpPreviousDatasetButton->setEnabled(false);

   mpNextDatasetButton = new QToolButton(pDatasetWidget);
   mpNextDatasetButton->setArrowType(Qt::RightArrow);
   mpNextDatasetButton->setFixedSize(15, 15);
   mpNextDatasetButton->setEnabled(false);

   QLabel* pNoDatasetPreviewLabel = new QLabel("No preview available for this data set.");
   pNoDatasetPreviewLabel->setAlignment(Qt::AlignCenter);
   pNoDatasetPreviewLabel->setWordWrap(true);

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

   QGridLayout* pPreviewGrid = new QGridLayout(mpPreview);
   pPreviewGrid->setMargin(0);
   pPreviewGrid->setSpacing(5);
   pPreviewGrid->setRowStretch(0, 10);
   pPreviewGrid->setColumnStretch(0, 10);

   // File widget stack
   mpFileStack = new QStackedWidget(pFileWidget);
   mpFileStack->addWidget(pNoFilePreviewLabel);
   mpFileStack->addWidget(pDatasetWidget);
   mpFileStack->setCurrentWidget(pNoFilePreviewLabel);

   // Data set widget stack
   mpDatasetStack = new QStackedWidget(pDatasetWidget);
   mpDatasetStack->addWidget(pNoDatasetPreviewLabel);
   mpDatasetStack->addWidget(pProgressBox);
   mpDatasetStack->addWidget(mpPreview);
   mpDatasetStack->setCurrentWidget(pNoDatasetPreviewLabel);

   // Contents widget stack
   mpStack = new QStackedWidget(this);
   mpStack->addWidget(pNoPreviewLabel);
   mpStack->addWidget(pFileWidget);
   mpStack->setCurrentWidget(pNoPreviewLabel);

   // Layout
   QGridLayout* pFileGrid = new QGridLayout(pFileWidget);
   pFileGrid->setMargin(0);
   pFileGrid->setSpacing(5);
   pFileGrid->addWidget(mpFileLabel, 0, 0);
   pFileGrid->addWidget(mpPreviousFileButton, 0, 1);
   pFileGrid->addWidget(mpNextFileButton, 0, 2);
   pFileGrid->addWidget(mpFileStack, 1, 0, 1, 3);
   pFileGrid->setRowStretch(1, 10);
   pFileGrid->setColumnStretch(0, 10);

   QGridLayout* pDatasetGrid = new QGridLayout(pDatasetWidget);
   pDatasetGrid->setMargin(0);
   pDatasetGrid->setSpacing(5);
   pDatasetGrid->addWidget(mpDatasetLabel, 0, 0);
   pDatasetGrid->addWidget(mpPreviousDatasetButton, 0, 1);
   pDatasetGrid->addWidget(mpNextDatasetButton, 0, 2);
   pDatasetGrid->addWidget(mpDatasetStack, 1, 0, 1, 3);
   pDatasetGrid->setRowStretch(1, 10);
   pDatasetGrid->setColumnStretch(0, 10);

   QHBoxLayout* pLayout = new QHBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(0);
   pLayout->addSpacing(5);
   pLayout->addWidget(mpStack, 10);

   // Initialization
   setMinimumWidth(260);

   // Connections
   VERIFYNR(connect(mpPreviousFileButton, SIGNAL(clicked()), this, SLOT(displayPreviousFile())));
   VERIFYNR(connect(mpNextFileButton, SIGNAL(clicked()), this, SLOT(displayNextFile())));
   VERIFYNR(connect(mpPreviousDatasetButton, SIGNAL(clicked()), this, SLOT(displayPreviousDataset())));
   VERIFYNR(connect(mpNextDatasetButton, SIGNAL(clicked()), this, SLOT(displayNextDataset())));
}

PreviewWidget::~PreviewWidget()
{
   destroyPreview();
}

void PreviewWidget::setImporter(Importer* pImporter)
{
   mpImporter = pImporter;
}

void PreviewWidget::setDatasets(const QMap<QString, vector<ImportDescriptor*> >& datasets)
{
   if (datasets == mDatasets)
   {
      return;
   }

   mDatasets = datasets;

   // Enable the file buttons
   unsigned int numFiles = datasets.size();
   mpPreviousFileButton->setEnabled(numFiles > 1);
   mpNextFileButton->setEnabled(numFiles > 1);

   // Reset the preview if the current file is not in the map of new files
   if ((mCurrentFile.isEmpty() == true) || (mDatasets.contains(mCurrentFile) == false))
   {
      QString filename;
      if (mDatasets.empty() == false)
      {
         QMap<QString, vector<ImportDescriptor*> >::iterator iter = mDatasets.begin();
         filename = iter.key();
      }

      setCurrentFile(filename);
   }
   else
   {
      // Set the preview to the first imported data set if the current data set is no longer in the map
      QMap<QString, vector<ImportDescriptor*> >::iterator iter = mDatasets.find(mCurrentFile);
      VERIFYNRV(iter != mDatasets.end());

      vector<ImportDescriptor*> fileDatasets = iter.value();

      vector<ImportDescriptor*>::iterator datasetIter =
         std::find(fileDatasets.begin(), fileDatasets.end(), mpCurrentDataset);
      if (datasetIter == fileDatasets.end())
      {
         QString filename = mCurrentFile;
         mCurrentFile.clear();
         setCurrentFile(filename);
      }
      else
      {
         // Update the file number
         updateFileNumber();
      }
   }
}

void PreviewWidget::setCurrentFile(const QString& filename)
{
   // Always clear the current file even if the filename is the same as the current filename
   // to ensure that the previous and next dataset buttons are enabled properly
   mCurrentFile.clear();

   // Activate the label indicating that no preview is available
   mpStack->setCurrentIndex(0);

   // Get a data set to activate
   ImportDescriptor* pDataset = NULL;
   if (filename.isEmpty() == false)
   {
      // Do not update the preview if the file does not exist in the map
      QMap<QString, vector<ImportDescriptor*> >::iterator iter = mDatasets.find(filename);
      if (iter != mDatasets.end())
      {
         mCurrentFile = filename;

         // Activate the file widget
         mpStack->setCurrentIndex(1);

         // Update the file label
         updateFileNumber();

         // Activate the label indicating that no file preview is available
         mpFileStack->setCurrentIndex(0);

         // Update the preview with the first imported data set in the file
         unsigned int numImportedDatasets = 0;

         vector<ImportDescriptor*> fileDatasets = iter.value();
         for (vector<ImportDescriptor*>::iterator datasetIter = fileDatasets.begin();
            datasetIter != fileDatasets.end();
            ++datasetIter)
         {
            ImportDescriptor* pCurrentDataset = *datasetIter;
            if ((pCurrentDataset != NULL) && (pCurrentDataset->isImported() == true))
            {
               if (pDataset == NULL)
               {
                  pDataset = pCurrentDataset;
               }

               ++numImportedDatasets;

               // Activate the data set widget
               mpFileStack->setCurrentIndex(1);
            }
         }

         // Enable the data set buttons
         mpPreviousDatasetButton->setEnabled(numImportedDatasets > 1);
         mpNextDatasetButton->setEnabled(numImportedDatasets > 1);
      }
   }

   setCurrentDataset(pDataset);
}

QString PreviewWidget::getCurrentFile() const
{
   return mCurrentFile;
}

void PreviewWidget::setCurrentDataset(ImportDescriptor* pDataset)
{
   ImportDescriptor* pActiveDataset = getCurrentDataset();
   if (pDataset == pActiveDataset)
   {
      return;
   }

   // Do nothing if the new current data set is not a data set in the current file
   if (pDataset != NULL)
   {
      QMap<QString, vector<ImportDescriptor*> >::const_iterator iter = mDatasets.find(mCurrentFile);
      if (iter != mDatasets.end())
      {
         vector<ImportDescriptor*> fileDatasets = iter.value();
         if (std::find(fileDatasets.begin(), fileDatasets.end(), pDataset) == fileDatasets.end())
         {
            return;
         }
      }
   }

   mpCurrentDataset = pDataset;

   // Delete the current preview
   destroyPreview();

   // Activate the label indicating that no data set preview is available
   mpDatasetStack->setCurrentIndex(0);

   // Check for no active data set
   if (mpCurrentDataset == NULL)
   {
      emit currentDatasetChanged(mpCurrentDataset);
      return;
   }

   // Only show the preview if the widget is visible or if the data set is imported
   if ((isVisible() == false) || (mpCurrentDataset->isImported() == false))
   {
      if (pActiveDataset != NULL)
      {
         emit currentDatasetChanged(NULL);
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
      mpDatasetStack->setCurrentIndex(1);

      // Update the data set label
      QMap<QString, vector<ImportDescriptor*> >::const_iterator iter = mDatasets.find(mCurrentFile);
      VERIFYNRV(iter != mDatasets.end());

      vector<ImportDescriptor*> fileDatasets = iter.value();

      unsigned int iIndex = 0;
      unsigned int numDatasets = fileDatasets.size();
      for (iIndex = 0; iIndex < numDatasets; ++iIndex)
      {
         ImportDescriptor* pCurrentDataset = fileDatasets[iIndex];
         if (pCurrentDataset == pDataset)
         {
            break;
         }
      }

      VERIFYNRV(iIndex < numDatasets);

      const DataDescriptor* pDescriptor = mpCurrentDataset->getDataDescriptor();
      VERIFYNRV(pDescriptor != NULL);

      mpDatasetLabel->setText("<b>Data Set (" + QString::number(iIndex + 1) + " of " +
         QString::number(numDatasets) + "):</b>  " + QString::fromStdString(pDescriptor->getName()));

      // Process events to erase the no preview available page
      qApp->processEvents();

      // Get the preview from the importer
      mpImporterWidget = mpImporter->getPreview(pDescriptor, pProgress);
      if (mpImporterWidget != NULL)
      {
         // Display the preview widget
         SpatialDataView* pView = dynamic_cast<SpatialDataView*>(mpImporterWidget);
         if (pView != NULL)
         {
            ChippingWidget* pChippingWidget = new ChippingWidget(pView, mpPreview);
            VERIFYNRV(pChippingWidget != NULL);

            VERIFYNR(connect(pChippingWidget, SIGNAL(chipChanged()), this, SLOT(updateCurrentDataset())));
            mpImporterWidget = pChippingWidget;
         }
         else
         {
            mpImporterWidget->setParent(mpPreview);
         }

         QGridLayout* pGrid = dynamic_cast<QGridLayout*>(mpPreview->layout());
         if (pGrid != NULL)
         {
            pGrid->addWidget(mpImporterWidget, 0, 0);
         }

         // Activate the preview widget
         mpDatasetStack->setCurrentIndex(2);

         // Notify of changes
         emit currentDatasetChanged(mpCurrentDataset);
      }
      else
      {
         mpDatasetStack->setCurrentIndex(0);

         if (pActiveDataset != NULL)
         {
            emit currentDatasetChanged(NULL);
         }
      }

      if (pProgress != NULL)
      {
         pProgress->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &PreviewWidget::progressUpdated));
      }
   }
}

ImportDescriptor* PreviewWidget::getCurrentDataset() const
{
   return mpCurrentDataset;
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

void PreviewWidget::displayPreviousFile()
{
   QMap<QString, vector<ImportDescriptor*> >::iterator iter = mDatasets.find(mCurrentFile);
   if (iter == mDatasets.end())
   {
      return;
   }

   if (iter == mDatasets.begin())
   {
      iter = --mDatasets.end();
   }
   else
   {
      --iter;
   }

   setCurrentFile(iter.key());
}

void PreviewWidget::displayNextFile()
{
   QMap<QString, vector<ImportDescriptor*> >::iterator iter = mDatasets.find(mCurrentFile);
   if (iter == mDatasets.end())
   {
      return;
   }

   if (iter == --mDatasets.end())
   {
      iter = mDatasets.begin();
   }
   else
   {
      ++iter;
   }

   setCurrentFile(iter.key());
}

void PreviewWidget::displayPreviousDataset()
{
   // Get the descriptor of the previous imported data set in the vector
   ImportDescriptor* pCurrentDataset = getCurrentDataset();
   ImportDescriptor* pPreviousDataset = NULL;
   ImportDescriptor* pLastDataset = NULL;
   bool bNextDataset = (pCurrentDataset == NULL);

   QMap<QString, vector<ImportDescriptor*> >::iterator fileIter = mDatasets.find(mCurrentFile);
   VERIFYNRV(fileIter != mDatasets.end());

   vector<ImportDescriptor*> fileDatasets = fileIter.value();

   vector<ImportDescriptor*>::reverse_iterator iter;
   for (iter = fileDatasets.rbegin(); iter != fileDatasets.rend(); ++iter)
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

         if (pDataset == pCurrentDataset)
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
   setCurrentDataset(pPreviousDataset);
}

void PreviewWidget::displayNextDataset()
{
   // Get the descriptor of the next imported data set in the vector
   ImportDescriptor* pCurrentDataset = getCurrentDataset();
   ImportDescriptor* pNextDataset = NULL;
   ImportDescriptor* pFirstDataset = NULL;
   bool bNextDataset = (pCurrentDataset == NULL);

   QMap<QString, vector<ImportDescriptor*> >::iterator fileIter = mDatasets.find(mCurrentFile);
   VERIFYNRV(fileIter != mDatasets.end());

   vector<ImportDescriptor*> fileDatasets = fileIter.value();
   for (vector<ImportDescriptor*>::iterator iter = fileDatasets.begin(); iter != fileDatasets.end(); ++iter)
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

         if (pDataset == pCurrentDataset)
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
   setCurrentDataset(pNextDataset);
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

void PreviewWidget::updateFileNumber()
{
   if (mCurrentFile.isEmpty() == true)
   {
      return;
   }

   // Update the file label
   QMap<QString, vector<ImportDescriptor*> >::iterator iter;
   unsigned int i = 1;

   for (iter = mDatasets.begin(); iter != mDatasets.end(); ++iter, ++i)
   {
      if (iter.key() == mCurrentFile)
      {
         break;
      }
   }

   if (iter != mDatasets.end())
   {
      QString baseFilename = mCurrentFile;

      QFileInfo fileInfo(baseFilename);
      if ((fileInfo.isFile() == true) && (fileInfo.exists() == true))
      {
         baseFilename = fileInfo.fileName();
      }

      mpFileLabel->setText("<b>File (" + QString::number(i) + " of " + QString::number(mDatasets.size()) +
         "):</b>  " + baseFilename);
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

void PreviewWidget::updateCurrentDataset()
{
   ChippingWidget* pChippingWidget = dynamic_cast<ChippingWidget*>(mpImporterWidget);
   if (pChippingWidget == NULL)
   {
      return;
   }

   ImportDescriptor* pActiveDataset = getCurrentDataset();
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
