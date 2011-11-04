/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QStackedWidget>

#include "AppVerify.h"
#include "Exporter.h"
#include "ExportOptionsDlg.h"
#include "AppConfig.h"
#include "PlugIn.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SessionItem.h"
#include "SubsetWidget.h"

using namespace std;

//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Make cancel work for custom exporter options widgets. " \
//   "(kstreith)")

ExportOptionsDlg::ExportOptionsDlg(ExporterResource& pExporter, QWidget* pParent) :
   QDialog(pParent),
   mpExporter(pExporter),
   mpTabWidget(NULL),
   mpSubsetPage(NULL),
   mpExporterPage(NULL)
{
   // Options widget
   QStackedWidget* pStack = new QStackedWidget(this);

   // Subset page
   RasterElement* pRasterElement = dynamic_cast<RasterElement*>(mpExporter->getItem());
   RasterFileDescriptor* pRasterWholeCubeFileDescriptor = NULL;
   RasterDataDescriptor* pRasterOrgDataDescriptor = NULL;
   if (pRasterElement != NULL)
   {
      pRasterOrgDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      if (pRasterOrgDataDescriptor != NULL)
      {
         // we are creating a file descriptor for export from the original cube, because the SubsetWidget
         // uses DimensionDescriptor::operator= compare's to determine selection which dictate that on-disk,
         // original and active numbers need to be identical, this guarantees that DimensionDescriptors will
         // compare correctly.
         pRasterWholeCubeFileDescriptor = dynamic_cast<RasterFileDescriptor*>(
            RasterUtilities::generateFileDescriptorForExport(pRasterOrgDataDescriptor, "foobar"));
      }
   }
   RasterFileDescriptor* pRasterFileDescriptor = dynamic_cast<RasterFileDescriptor*>(mpExporter->getFileDescriptor());
   if ((pRasterFileDescriptor != NULL) && (pRasterWholeCubeFileDescriptor != NULL) &&
      (pRasterOrgDataDescriptor != NULL))
   {
      mpSubsetPage = new SubsetWidget();
      mpSubsetPage->setExportMode(true);

      // Rows
      const vector<DimensionDescriptor>& orgRows = pRasterWholeCubeFileDescriptor->getRows();
      const vector<DimensionDescriptor>& selectedRows = pRasterFileDescriptor->getRows();
      mpSubsetPage->setRows(orgRows, selectedRows);

      // Columns
      const vector<DimensionDescriptor>& orgColumns = pRasterWholeCubeFileDescriptor->getColumns();
      const vector<DimensionDescriptor>& selectedColumns = pRasterFileDescriptor->getColumns();
      mpSubsetPage->setColumns(orgColumns, selectedColumns);

      // Bands
      const vector<DimensionDescriptor>& orgBands = pRasterWholeCubeFileDescriptor->getBands();
      const vector<DimensionDescriptor>& selectedBands = pRasterFileDescriptor->getBands();
      vector<string> bandNames = RasterUtilities::getBandNames(pRasterOrgDataDescriptor);
      mpSubsetPage->setBands(orgBands, bandNames, selectedBands);

      // Initial bad band file directory
      QString strDirectory;

      string filename = pRasterFileDescriptor->getFilename();
      if (filename.empty() == false)
      {
         QFileInfo fileInfo(QString::fromStdString(filename));
         strDirectory = fileInfo.absolutePath();
      }

      mpSubsetPage->setBadBandFileDirectory(strDirectory);
   }

   // Exporter page
   if (mpExporter->getPlugIn() != NULL)
   {
      mpExporterPage = mpExporter->getExportOptionsWidget();
   }

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(this);
   pButtonBox->setOrientation(Qt::Horizontal);
   pButtonBox->setStandardButtons(QDialogButtonBox::Ok);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pStack, 10);
   pLayout->addWidget(pLine);
   pLayout->addWidget(pButtonBox);

   // Initialization
   QString strWindowTitle = "Export Options";

   SessionItem* pSessionItem = mpExporter->getItem();
   if (pSessionItem != NULL)
   {
      string name = pSessionItem->getDisplayName();
      if (name.empty() == true)
      {
         name = pSessionItem->getName();
      }

      if (name.empty() == false)
      {
         strWindowTitle += ": " + QString::fromStdString(name);
      }
   }

   setWindowTitle(strWindowTitle);
   setModal(true);

   if ((mpSubsetPage != NULL) || (mpExporterPage != NULL))
   {
      QWidget* pSubsetWidget = NULL;
      if (mpSubsetPage != NULL)
      {
         pSubsetWidget = new QWidget();
         mpSubsetPage->setParent(pSubsetWidget);

         QVBoxLayout* pSubsetLayout = new QVBoxLayout(pSubsetWidget);
         if (mpExporterPage != NULL)
         {
            pSubsetLayout->setMargin(10);
         }
         else
         {
            pSubsetLayout->setMargin(0);
         }

         pSubsetLayout->setSpacing(10);
         pSubsetLayout->addWidget(mpSubsetPage);
      }

      QWidget* pExporterWidget = NULL;
      if (mpExporterPage != NULL)
      {
         pExporterWidget = new QWidget();
         mpExporterPage->setParent(pExporterWidget);

         QVBoxLayout* pExporterLayout = new QVBoxLayout(pExporterWidget);
         if (mpSubsetPage != NULL)
         {
            pExporterLayout->setMargin(10);
         }
         else
         {
            pExporterLayout->setMargin(0);
         }

         pExporterLayout->setSpacing(10);
         pExporterLayout->addWidget(mpExporterPage);
      }

      if ((pSubsetWidget != NULL) && (pExporterWidget != NULL))
      {
         QString strExporterCaption = mpExporterPage->windowTitle();
         if (strExporterCaption.isEmpty() == true)
         {
            PlugIn* pPlugIn = mpExporter->getPlugIn();
            if (pPlugIn != NULL)
            {
               strExporterCaption = QString::fromStdString(pPlugIn->getName());
            }

            if (strExporterCaption.isEmpty() == true)
            {
               strExporterCaption = "Exporter";
            }
         }

         mpTabWidget = new QTabWidget(this);
         mpTabWidget->setTabPosition(QTabWidget::North);
         mpTabWidget->addTab(pSubsetWidget, "Subset");
         mpTabWidget->addTab(pExporterWidget, strExporterCaption);
         pStack->addWidget(mpTabWidget);
      }
      else if (pSubsetWidget != NULL)
      {
         pStack->addWidget(pSubsetWidget);
         pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
      }
      else if (pExporterWidget != NULL)
      {
         pStack->addWidget(pExporterWidget);
      }
   }

   if (pStack->count() == 0)
   {
      QLabel* pNoOptionsLabel = new QLabel("No options are available", this);
      pNoOptionsLabel->setAlignment(Qt::AlignCenter);
      pNoOptionsLabel->setMinimumSize(250, 100);

      pStack->addWidget(pNoOptionsLabel);
   }

   // Connections
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
}

ExportOptionsDlg::~ExportOptionsDlg()
{
   // Remove the exporter page
   if (mpExporterPage != NULL)
   {
      // Remove the page from the tab widget
      if (mpTabWidget != NULL)
      {
         int iIndex = mpTabWidget->indexOf(mpExporterPage->parentWidget());
         if (iIndex != -1)
         {
            mpTabWidget->removeTab(iIndex);
         }
      }

      // The exporter is responsible for deleting the widget, so just reset the pointer
      mpExporterPage->setParent(NULL);
      mpExporterPage = NULL;
   }
}

class SetOnDiskNumber
{
public:
   SetOnDiskNumber() : mCurrent(0) {}
   DimensionDescriptor operator()(const DimensionDescriptor& dim)
   {
      DimensionDescriptor temp = dim;
      temp.setOnDiskNumber(mCurrent++);
      return temp;
   }
private:
   unsigned int mCurrent;
};

void ExportOptionsDlg::accept()
{
   // Update the user's subset changes in the file descriptor
   RasterFileDescriptor* pRasterFileDescriptor = dynamic_cast<RasterFileDescriptor*>(mpExporter->getFileDescriptor());
   if ((pRasterFileDescriptor != NULL) && (mpSubsetPage != NULL))
   {
      // Rows
      vector<DimensionDescriptor> rows = mpSubsetPage->getSubsetRows();
      std::transform(rows.begin(), rows.end(), rows.begin(), SetOnDiskNumber());
      pRasterFileDescriptor->setRows(rows);

      // Columns
      vector<DimensionDescriptor> columns = mpSubsetPage->getSubsetColumns();
      std::transform(columns.begin(), columns.end(), columns.begin(), SetOnDiskNumber());
      pRasterFileDescriptor->setColumns(columns);

      // Bands
      vector<DimensionDescriptor> bands = mpSubsetPage->getSubsetBands();
      std::transform(bands.begin(), bands.end(), bands.begin(), SetOnDiskNumber());
      pRasterFileDescriptor->setBands(bands);
   }

   QDialog::accept();
}
