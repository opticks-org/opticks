/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>

#include "AppVerify.h"
#include "CustomTreeWidget.h"
#include "FileBrowser.h"
#include "FileDescriptorWidget.h"
#include "GeoPoint.h"
#include "ObjectResource.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SignatureFileDescriptor.h"
#include "Slot.h"
#include "StringUtilities.h"
#include "TypesFile.h"
#include "Units.h"

#include <algorithm>
#include <vector>
using namespace std;

namespace
{
   const string BSQ_SINGLE_SUFFIX = " - single file";
   const string BSQ_MULTI_SUFFIX = " - multiple files";
};

FileDescriptorWidget::FileDescriptorWidget(QWidget* parent) :
   QWidget(parent),
   mpFileDescriptor(NULL),
   mEditable(false),
   mNeedsInitialization(false),
   mpTreeWidget(NULL),
   mpFileBrowser(NULL),
   mpEndianCombo(NULL),
   mpInterleaveCombo(NULL),
   mpUnitTypeCombo(NULL),
   mpGcpGroup(NULL),
   mpGcpTree(NULL)
{
   // Item tree widget
   QStringList columnNames;
   columnNames.append("Item");
   columnNames.append("Value");

   mpTreeWidget = new CustomTreeWidget(this);
   mpTreeWidget->setColumnCount(columnNames.count());
   mpTreeWidget->setHeaderLabels(columnNames);
   mpTreeWidget->setRootIsDecorated(true);
   mpTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
   mpTreeWidget->setGridlinesShown(Qt::Horizontal | Qt::Vertical, true);
   mpTreeWidget->setSortingEnabled(false);

   QHeaderView* pHeader = mpTreeWidget->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(false);
      pHeader->setMovable(false);
      pHeader->setStretchLastSection(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->resizeSection(0, 150);
   }

   // Band file browser
   mpFileBrowser = new FileBrowser(mpTreeWidget);
   mpFileBrowser->setBrowseCaption("Select Band File");
   mpFileBrowser->hide();

   // Combo boxes
   mpEndianCombo = new QComboBox(mpTreeWidget);
   mpEndianCombo->setEditable(false);
   mpEndianCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(LITTLE_ENDIAN_ORDER)));
   mpEndianCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BIG_ENDIAN_ORDER)));
   mpEndianCombo->hide();

   mpInterleaveCombo = new QComboBox(mpTreeWidget);
   mpInterleaveCombo->setEditable(false);
   mpInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BIL)));
   mpInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BIP)));
   mpInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BSQ) + BSQ_SINGLE_SUFFIX));
   mpInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BSQ) + BSQ_MULTI_SUFFIX));
   mpInterleaveCombo->hide();

   mpUnitTypeCombo = new QComboBox(mpTreeWidget);
   mpUnitTypeCombo->setEditable(false);

   vector<string> unitNames = StringUtilities::getAllEnumValuesAsDisplayString<UnitType>();
   sort(unitNames.begin(), unitNames.end());
   for (vector<string>::const_iterator iter = unitNames.begin(); iter != unitNames.end(); ++iter)
   {
      mpUnitTypeCombo->addItem(QString::fromStdString(*iter));
   }

   mpUnitTypeCombo->hide();

   // GCP group box
   mpGcpGroup = new QGroupBox("Ground Control Points (GCP)", this);

   // GCP tree widget
   columnNames.clear();
   columnNames.append("Name");
   columnNames.append("Column");
   columnNames.append("Row");
   columnNames.append("Latitude");
   columnNames.append("Longitude");

   mpGcpTree = new CustomTreeWidget(mpGcpGroup);
   mpGcpTree->setColumnCount(columnNames.count());
   mpGcpTree->setHeaderLabels(columnNames);
   mpGcpTree->setRootIsDecorated(false);
   mpGcpTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpGcpTree->setGridlinesShown(Qt::Horizontal | Qt::Vertical, true);
   mpGcpTree->setSortingEnabled(true);

   pHeader = mpGcpTree->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(true);
      pHeader->setMovable(false);
      pHeader->setStretchLastSection(false);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->resizeSection(0, 75);
      pHeader->resizeSection(1, 75);
      pHeader->resizeSection(2, 75);
   }

   // Layout
   QVBoxLayout* pGcpLayout = new QVBoxLayout(mpGcpGroup);
   pGcpLayout->setMargin(10);
   pGcpLayout->setSpacing(10);
   pGcpLayout->addWidget(mpGcpTree, 10);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(mpTreeWidget);
   pLayout->addWidget(mpGcpGroup);

   // Connections
   VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
}

FileDescriptorWidget::~FileDescriptorWidget()
{
   setFileDescriptor(NULL, false);
}

void FileDescriptorWidget::setFileDescriptor(FileDescriptor* pFileDescriptor, bool editable)
{
   mpTreeWidget->closeActiveCellWidget(false);

   if (mpFileDescriptor.get() != NULL)
   {
      // Only attach to the file descriptor to update the tree widget if this widget is currently hidden.  This
      // prevents a potentially large amount of time being spent updating the tree widget items when the file
      // descriptor changes externally.  This assumes that the file descriptor will not updated externally when
      // this page is shown, which means that this widget must exist in a modal dialog or other modal widget.
      if (isVisible() == false)
      {
         VERIFYNR(mpFileDescriptor->detach(SIGNAL_NAME(Subject, Modified),
            Slot(this, &FileDescriptorWidget::fileDescriptorModified)));
      }
   }

   mpFileDescriptor.reset(pFileDescriptor);
   mEditable = editable;

   mpTreeWidget->clear();
   mpGcpTree->clear();
   mpGcpGroup->hide();

   if (mpFileDescriptor.get() == NULL)
   {
      return;
   }

   if (isVisible() == false)
   {
      VERIFYNR(mpFileDescriptor->attach(SIGNAL_NAME(Subject, Modified),
         Slot(this, &FileDescriptorWidget::fileDescriptorModified)));
   }

   VERIFYNR(disconnect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));

   // Filename
   QTreeWidgetItem* pFilenameItem = new QTreeWidgetItem(mpTreeWidget);
   if (pFilenameItem != NULL)
   {
      string filename = mpFileDescriptor->getFilename();

      pFilenameItem->setText(0, "Filename");
      pFilenameItem->setText(1, QString::fromStdString(filename));
   }

   // Data set location
   QTreeWidgetItem* pDatasetLocationItem = new QTreeWidgetItem(mpTreeWidget);
   if (pDatasetLocationItem != NULL)
   {
      string datasetLocation = mpFileDescriptor->getDatasetLocation();

      pDatasetLocationItem->setText(0, "Data Set Location");
      pDatasetLocationItem->setText(1, QString::fromStdString(datasetLocation));
   }

   // Endian
   QTreeWidgetItem* pEndianItem = new QTreeWidgetItem(mpTreeWidget);
   if (pEndianItem != NULL)
   {
      EndianType endian = mpFileDescriptor->getEndian();
      string endianText = StringUtilities::toDisplayString(endian);

      pEndianItem->setText(0, "Endian");
      pEndianItem->setText(1, QString::fromStdString(endianText));

      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pEndianItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pEndianItem, 1, mpEndianCombo);
      }
   }

   // Signature file descriptor items
   SignatureFileDescriptor* pSignatureDescriptor = dynamic_cast<SignatureFileDescriptor*>(mpFileDescriptor.get());
   if (pSignatureDescriptor != NULL)
   {
      // Units
      QTreeWidgetItem* pUnitsItem = new QTreeWidgetItem(mpTreeWidget);
      if (pUnitsItem != NULL)
      {
         pUnitsItem->setText(0, "Units");

         set<string> unitNames = pSignatureDescriptor->getUnitNames();
         if (unitNames.empty() == false)
         {
            pUnitsItem->setBackgroundColor(1, Qt::lightGray);

            for (set<string>::const_iterator iter = unitNames.begin(); iter != unitNames.end(); ++iter)
            {
               string unitName = *iter;

               const Units* pUnits = pSignatureDescriptor->getUnits(unitName);
               if (pUnits == NULL)
               {
                  continue;
               }

               // Component item
               QTreeWidgetItem* pComponentItem = new QTreeWidgetItem(pUnitsItem);
               if (pComponentItem == NULL)
               {
                  continue;
               }

               pComponentItem->setText(0, QString::fromStdString(unitName));
               pComponentItem->setBackgroundColor(1, Qt::lightGray);

               // Name
               QTreeWidgetItem* pUnitNameItem = new QTreeWidgetItem(pComponentItem);
               if (pUnitNameItem != NULL)
               {
                  string unitsName = pUnits->getUnitName();

                  pUnitNameItem->setText(0, "Unit Name");
                  pUnitNameItem->setText(1, QString::fromStdString(unitsName));

                  if (mEditable == true)
                  {
                     mpTreeWidget->setCellWidgetType(pUnitNameItem, 1, CustomTreeWidget::LINE_EDIT);
                  }
               }

               // Type
               QTreeWidgetItem* pUnitTypeItem = new QTreeWidgetItem(pComponentItem);
               if (pUnitTypeItem != NULL)
               {
                  UnitType unitType = pUnits->getUnitType();
                  string unitTypeText = StringUtilities::toDisplayString(unitType);

                  pUnitTypeItem->setText(0, "Unit Type");
                  pUnitTypeItem->setText(1, QString::fromStdString(unitTypeText));

                  if (mEditable == true)
                  {
                     mpTreeWidget->setCellWidgetType(pUnitTypeItem, 1, CustomTreeWidget::COMBO_BOX);
                     mpTreeWidget->setComboBox(pUnitTypeItem, 1, mpUnitTypeCombo);
                  }
               }

               // Scale
               QTreeWidgetItem* pScaleItem = new QTreeWidgetItem(pComponentItem);
               if (pScaleItem != NULL)
               {
                  double dScale = pUnits->getScaleFromStandard();

                  pScaleItem->setText(0, "Scale");
                  pScaleItem->setText(1, QString::number(dScale));

                  if (mEditable == true)
                  {
                     mpTreeWidget->setCellWidgetType(pScaleItem, 1, CustomTreeWidget::LINE_EDIT);
                  }
               }

               // Range minimum
               QTreeWidgetItem* pMinimumItem = new QTreeWidgetItem(pComponentItem);
               if (pMinimumItem != NULL)
               {
                  double dMinimum = pUnits->getRangeMin();

                  pMinimumItem->setText(0, "Range Minimum");
                  pMinimumItem->setText(1, QString::number(dMinimum));

                  if (mEditable == true)
                  {
                     mpTreeWidget->setCellWidgetType(pMinimumItem, 1, CustomTreeWidget::LINE_EDIT);
                  }
               }

               // Range maximum
               QTreeWidgetItem* pMaximumItem = new QTreeWidgetItem(pComponentItem);
               if (pMaximumItem != NULL)
               {
                  double dMaximum = pUnits->getRangeMax();

                  pMaximumItem->setText(0, "Range Maximum");
                  pMaximumItem->setText(1, QString::number(dMaximum));

                  if (mEditable == true)
                  {
                     mpTreeWidget->setCellWidgetType(pMaximumItem, 1, CustomTreeWidget::LINE_EDIT);
                  }
               }
            }
         }
         else  // No units associated with components in this signature
         {
            pUnitsItem->setText(1, "No Units defined");
         }
      }

      VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
         SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
      return;
   }

   // Raster file descriptor items
   RasterFileDescriptor* pRasterDescriptor = dynamic_cast<RasterFileDescriptor*>(mpFileDescriptor.get());
   if (pRasterDescriptor == NULL)
   {
      VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
         SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
      return;
   }

   mpGcpGroup->show();

   // Rows
   QTreeWidgetItem* pRowsItem = new QTreeWidgetItem();
   if (pRowsItem != NULL)
   {
      unsigned int rows = pRasterDescriptor->getRowCount();

      pRowsItem->setText(0, "Rows");
      pRowsItem->setText(1, QString::number(rows));

      mpTreeWidget->insertTopLevelItem(2, pRowsItem);
      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pRowsItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Columns
   QTreeWidgetItem* pColumnsItem = new QTreeWidgetItem();
   if (pColumnsItem != NULL)
   {
      unsigned int columns = pRasterDescriptor->getColumnCount();

      pColumnsItem->setText(0, "Columns");
      pColumnsItem->setText(1, QString::number(columns));

      mpTreeWidget->insertTopLevelItem(3, pColumnsItem);
      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pColumnsItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Bits per element
   QTreeWidgetItem* pBitsPerElementItem = new QTreeWidgetItem();
   if (pBitsPerElementItem != NULL)
   {
      unsigned int bitsPerElement = pRasterDescriptor->getBitsPerElement();

      pBitsPerElementItem->setText(0, "Bits Per Element");
      pBitsPerElementItem->setText(1, QString::number(bitsPerElement));

      mpTreeWidget->insertTopLevelItem(4, pBitsPerElementItem);
      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pBitsPerElementItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Header bytes
   QTreeWidgetItem* pHeaderBytesItem = new QTreeWidgetItem(mpTreeWidget);
   if (pHeaderBytesItem != NULL)
   {
      unsigned int headerBytes = pRasterDescriptor->getHeaderBytes();

      pHeaderBytesItem->setText(0, "Header Bytes");
      pHeaderBytesItem->setText(1, QString::number(headerBytes));

      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pHeaderBytesItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Trailer bytes
   QTreeWidgetItem* pTrailerBytesItem = new QTreeWidgetItem(mpTreeWidget);
   if (pTrailerBytesItem != NULL)
   {
      unsigned int trailerBytes = pRasterDescriptor->getTrailerBytes();

      pTrailerBytesItem->setText(0, "Trailer Bytes");
      pTrailerBytesItem->setText(1, QString::number(trailerBytes));

      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pTrailerBytesItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Preline bytes
   QTreeWidgetItem* pPrelineBytesItem = new QTreeWidgetItem(mpTreeWidget);
   if (pPrelineBytesItem != NULL)
   {
      unsigned int prelineBytes = pRasterDescriptor->getPrelineBytes();

      pPrelineBytesItem->setText(0, "Preline Bytes");
      pPrelineBytesItem->setText(1, QString::number(prelineBytes));

      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pPrelineBytesItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Postline bytes
   QTreeWidgetItem* pPostlineBytesItem = new QTreeWidgetItem(mpTreeWidget);
   if (pPostlineBytesItem != NULL)
   {
      unsigned int postlineBytes = pRasterDescriptor->getPostlineBytes();

      pPostlineBytesItem->setText(0, "Postline Bytes");
      pPostlineBytesItem->setText(1, QString::number(postlineBytes));

      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pPostlineBytesItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Pixel size
   QTreeWidgetItem* pXPixelSizeItem = new QTreeWidgetItem(mpTreeWidget);
   if (pXPixelSizeItem != NULL)
   {
      double pixelSize = pRasterDescriptor->getXPixelSize();

      pXPixelSizeItem->setText(0, "X Pixel Size");
      pXPixelSizeItem->setText(1, QString::number(pixelSize));

      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pXPixelSizeItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   QTreeWidgetItem* pYPixelSizeItem = new QTreeWidgetItem(mpTreeWidget);
   if (pYPixelSizeItem != NULL)
   {
      double pixelSize = pRasterDescriptor->getYPixelSize();

      pYPixelSizeItem->setText(0, "Y Pixel Size");
      pYPixelSizeItem->setText(1, QString::number(pixelSize));

      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pYPixelSizeItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Units
   const Units* pUnits = pRasterDescriptor->getUnits();
   if (pUnits != NULL)
   {
      QTreeWidgetItem* pUnitsItem = new QTreeWidgetItem(mpTreeWidget);
      if (pUnitsItem != NULL)
      {
         pUnitsItem->setText(0, "Units");
         pUnitsItem->setBackgroundColor(1, Qt::lightGray);

         // Name
         QTreeWidgetItem* pNameItem = new QTreeWidgetItem(pUnitsItem);
         if (pNameItem != NULL)
         {
            string unitsName = pUnits->getUnitName();

            pNameItem->setText(0, "Name");
            pNameItem->setText(1, QString::fromStdString(unitsName));

            if (mEditable == true)
            {
               mpTreeWidget->setCellWidgetType(pNameItem, 1, CustomTreeWidget::LINE_EDIT);
            }
         }

         // Type
         QTreeWidgetItem* pTypeItem = new QTreeWidgetItem(pUnitsItem);
         if (pTypeItem != NULL)
         {
            UnitType unitType = pUnits->getUnitType();
            string unitTypeText = StringUtilities::toDisplayString(unitType);

            pTypeItem->setText(0, "Type");
            pTypeItem->setText(1, QString::fromStdString(unitTypeText));

            if (mEditable == true)
            {
               mpTreeWidget->setCellWidgetType(pTypeItem, 1, CustomTreeWidget::COMBO_BOX);
               mpTreeWidget->setComboBox(pTypeItem, 1, mpUnitTypeCombo);
            }
         }

         // Scale
         QTreeWidgetItem* pScaleItem = new QTreeWidgetItem(pUnitsItem);
         if (pScaleItem != NULL)
         {
            double dScale = pUnits->getScaleFromStandard();

            pScaleItem->setText(0, "Scale");
            pScaleItem->setText(1, QString::number(dScale));

            if (mEditable == true)
            {
               mpTreeWidget->setCellWidgetType(pScaleItem, 1, CustomTreeWidget::LINE_EDIT);
            }
         }

         // Range minimum
         QTreeWidgetItem* pMinimumItem = new QTreeWidgetItem(pUnitsItem);
         if (pMinimumItem != NULL)
         {
            double dMinimum = pUnits->getRangeMin();

            pMinimumItem->setText(0, "Range Minimum");
            pMinimumItem->setText(1, QString::number(dMinimum));

            if (mEditable == true)
            {
               mpTreeWidget->setCellWidgetType(pMinimumItem, 1, CustomTreeWidget::LINE_EDIT);
            }
         }

         // Range maximum
         QTreeWidgetItem* pMaximumItem = new QTreeWidgetItem(pUnitsItem);
         if (pMaximumItem != NULL)
         {
            double dMaximum = pUnits->getRangeMax();

            pMaximumItem->setText(0, "Range Maximum");
            pMaximumItem->setText(1, QString::number(dMaximum));

            if (mEditable == true)
            {
               mpTreeWidget->setCellWidgetType(pMaximumItem, 1, CustomTreeWidget::LINE_EDIT);
            }
         }
      }
   }

   // GCPs
   if (mpGcpTree != NULL)
   {
      const list<GcpPoint>& gcps = pRasterDescriptor->getGcps();
      if (gcps.empty() == false)
      {
         list<GcpPoint>::const_iterator iter;
         unsigned int i = 0;

         for (iter = gcps.begin(), i = 0; iter != gcps.end(); ++iter, ++i)
         {
            GcpPoint gcp = *iter;

            QTreeWidgetItem* pGcpItem = new QTreeWidgetItem(mpGcpTree);
            if (pGcpItem != NULL)
            {
               QString strLatitude;
               QString strLongitude;
               LatLonPoint latLonPoint(gcp.mCoordinate);

               string latText = latLonPoint.getLatitudeText();
               if (latText.empty() == false)
               {
                  strLatitude = QString::fromStdString(latText);
               }

               string longText = latLonPoint.getLongitudeText();
               if (longText.empty() == false)
               {
                  strLongitude = QString::fromStdString(longText);
               }

               pGcpItem->setText(0, QString("GCP ") + QString::number(i + 1));
               pGcpItem->setText(1, QString::number(gcp.mPixel.mX + 1.0));
               pGcpItem->setText(2, QString::number(gcp.mPixel.mY + 1.0));
               pGcpItem->setText(3, strLatitude);
               pGcpItem->setText(4, strLongitude);
            }
         }

         mpGcpTree->setEnabled(true);
      }
      else
      {
         mpGcpTree->setEnabled(false);
      }
   }

   // Bands
   QTreeWidgetItem* pBandsItem = new QTreeWidgetItem();
   if (pBandsItem != NULL)
   {
      unsigned int bands = pRasterDescriptor->getBandCount();

      pBandsItem->setText(0, "Bands");
      pBandsItem->setText(1, QString::number(bands));

      mpTreeWidget->insertTopLevelItem(4, pBandsItem);
      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pBandsItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Interleave format
   InterleaveFormatType interleave = pRasterDescriptor->getInterleaveFormat();

   QTreeWidgetItem* pInterleaveItem = new QTreeWidgetItem();
   if (pInterleaveItem != NULL)
   {
      string interleaveText = StringUtilities::toDisplayString(interleave);
      if (interleave == BSQ)
      {
         const vector<const Filename*>& bandFiles = pRasterDescriptor->getBandFiles();
         if (bandFiles.size() > 0)
         {
            interleaveText += BSQ_MULTI_SUFFIX;
         }
         else
         {
            interleaveText += BSQ_SINGLE_SUFFIX;
         }
      }
      pInterleaveItem->setText(0, "Interleave Format");
      pInterleaveItem->setText(1, QString::fromStdString(interleaveText));

      mpTreeWidget->insertTopLevelItem(7, pInterleaveItem);
      if (mEditable == true)
      {
         mpTreeWidget->setCellWidgetType(pInterleaveItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pInterleaveItem, 1, mpInterleaveCombo);
      }
   }

   // Band files
   const vector<const Filename*>& bandFiles = pRasterDescriptor->getBandFiles();
   for (unsigned int i = 0; i < bandFiles.size(); ++i)
   {
      string bandFilename = bandFiles[i]->getFullPathAndName();
      if (bandFilename.empty() == false)
      {
         QTreeWidgetItem* pBandItem = new QTreeWidgetItem(pFilenameItem);
         if (pBandItem != NULL)
         {
            pBandItem->setText(0, QString::number(i + 1));
            pBandItem->setText(1, QString::fromStdString(bandFilename));

            if (mEditable == true)
            {
               mpTreeWidget->setCellWidgetType(pBandItem, 1, CustomTreeWidget::BROWSE_FILE_EDIT);
               mpTreeWidget->setFileBrowser(pBandItem, 1, mpFileBrowser);
            }
         }
      }
   }

   // Preband bytes
   QTreeWidgetItem* pPrebandBytesItem = new QTreeWidgetItem();
   if (pPrebandBytesItem != NULL)
   {
      QString strPrebandBytes;
      QColor cellColor = Qt::lightGray;

      if (interleave == BSQ)
      {
         strPrebandBytes = QString::number(pRasterDescriptor->getPrebandBytes());
         cellColor = Qt::white;
      }

      pPrebandBytesItem->setText(0, "Preband Bytes");
      pPrebandBytesItem->setText(1, strPrebandBytes);
      pPrebandBytesItem->setBackgroundColor(1, cellColor);

      mpTreeWidget->insertTopLevelItem(12, pPrebandBytesItem);
      if ((mEditable == true) && (interleave == BSQ))
      {
         mpTreeWidget->setCellWidgetType(pPrebandBytesItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Postband bytes
   QTreeWidgetItem* pPostbandBytesItem = new QTreeWidgetItem();
   if (pPostbandBytesItem != NULL)
   {
      QString strPostbandBytes;
      QColor cellColor = Qt::lightGray;

      if (interleave == BSQ)
      {
         strPostbandBytes = QString::number(pRasterDescriptor->getPostbandBytes());
         cellColor = Qt::white;
      }

      pPostbandBytesItem->setText(0, "Postband Bytes");
      pPostbandBytesItem->setText(1, strPostbandBytes);
      pPostbandBytesItem->setBackgroundColor(1, cellColor);

      mpTreeWidget->insertTopLevelItem(13, pPostbandBytesItem);
      if ((mEditable == true) && (interleave == BSQ))
      {
         mpTreeWidget->setCellWidgetType(pPostbandBytesItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
}

void FileDescriptorWidget::showEvent(QShowEvent* pEvent)
{
   QWidget::showEvent(pEvent);

   if (mpFileDescriptor.get() != NULL)
   {
      VERIFYNR(mpFileDescriptor->detach(SIGNAL_NAME(Subject, Modified),
         Slot(this, &FileDescriptorWidget::fileDescriptorModified)));
      initialize();
   }
}

void FileDescriptorWidget::hideEvent(QHideEvent* pEvent)
{
   QWidget::hideEvent(pEvent);

   if (mpFileDescriptor.get() != NULL)
   {
      VERIFYNR(mpFileDescriptor->attach(SIGNAL_NAME(Subject, Modified),
         Slot(this, &FileDescriptorWidget::fileDescriptorModified)));
   }
}

void FileDescriptorWidget::fileDescriptorModified(Subject& subject, const string& signal, const boost::any& value)
{
   mNeedsInitialization = true;
}

void FileDescriptorWidget::initialize()
{
   if ((mpFileDescriptor.get() == NULL) || (mNeedsInitialization == false) || (isVisible() == false))
   {
      return;
   }

   mNeedsInitialization = false;

   VERIFYNR(disconnect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));

   // File descriptor items
   QTreeWidgetItem* pItem = getDescriptorItem("Filename");
   if (pItem != NULL)
   {
      const string& filename = mpFileDescriptor->getFilename();
      pItem->setText(1, QString::fromStdString(filename));
   }

   pItem = getDescriptorItem("Data Set Location");
   if (pItem != NULL)
   {
      const string& datasetLocation = mpFileDescriptor->getDatasetLocation();
      pItem->setText(1, QString::fromStdString(datasetLocation));
   }

   pItem = getDescriptorItem("Endian");
   if (pItem != NULL)
   {
      EndianType endian = mpFileDescriptor->getEndian();
      string endianText = StringUtilities::toDisplayString(endian);
      pItem->setText(1, QString::fromStdString(endianText));

      if (mpEndianCombo != NULL)
      {
         int index = mpEndianCombo->findText(QString::fromStdString(endianText));
         mpEndianCombo->setCurrentIndex(index);
      }
   }

   // Signature file descriptor items
   const SignatureFileDescriptor* pSignatureFileDescriptor =
      dynamic_cast<const SignatureFileDescriptor*>(mpFileDescriptor.get());
   if (pSignatureFileDescriptor != NULL)
   {
      set<string> componentNames = pSignatureFileDescriptor->getUnitNames();
      for (set<string>::const_iterator iter = componentNames.begin(); iter != componentNames.end(); ++iter)
      {
         const Units* pUnits = pSignatureFileDescriptor->getUnits(*iter);
         if (pUnits != NULL)
         {
            QTreeWidgetItem* pComponentItem = getDescriptorItem(QString::fromStdString(*iter));

            pItem = getDescriptorItem("Unit Name", pComponentItem);
            if (pItem != NULL)
            {
               pItem->setText(1, QString::fromStdString(pUnits->getUnitName()));
            }

            pItem = getDescriptorItem("Unit Type", pComponentItem);
            if (pItem != NULL)
            {
               UnitType unitType = pUnits->getUnitType();
               string unitTypeText = StringUtilities::toDisplayString(unitType);
               pItem->setText(1, QString::fromStdString(unitTypeText));

               if (mpUnitTypeCombo != NULL)
               {
                  int index = mpUnitTypeCombo->findText(QString::fromStdString(unitTypeText));
                  mpUnitTypeCombo->setCurrentIndex(index);
               }
            }

            pItem = getDescriptorItem("Scale", pComponentItem);
            if (pItem != NULL)
            {
               pItem->setText(1, QString::number(pUnits->getScaleFromStandard()));
            }

            pItem = getDescriptorItem("Range Minimum", pComponentItem);
            if (pItem != NULL)
            {
               pItem->setText(1, QString::number(pUnits->getRangeMin()));
            }

            pItem = getDescriptorItem("Range Maximum", pComponentItem);
            if (pItem != NULL)
            {
               pItem->setText(1, QString::number(pUnits->getRangeMax()));
            }
         }
      }

      VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
         SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
      return;
   }

   // Raster file descriptor items
   const RasterFileDescriptor* pRasterFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(mpFileDescriptor.get());
   if (pRasterFileDescriptor == NULL)
   {
      VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
         SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
      return;
   }

   pItem = getDescriptorItem("Rows");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterFileDescriptor->getRowCount()));
   }

   pItem = getDescriptorItem("Columns");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterFileDescriptor->getColumnCount()));
   }

   pItem = getDescriptorItem("Bands");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterFileDescriptor->getBandCount()));
   }

   pItem = getDescriptorItem("Bits Per Element");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterFileDescriptor->getBitsPerElement()));
   }

   pItem = getDescriptorItem("Header Bytes");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterFileDescriptor->getHeaderBytes()));
   }

   pItem = getDescriptorItem("Trailer Bytes");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterFileDescriptor->getTrailerBytes()));
   }

   pItem = getDescriptorItem("Preline Bytes");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterFileDescriptor->getPrelineBytes()));
   }

   pItem = getDescriptorItem("Postline Bytes");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterFileDescriptor->getPostlineBytes()));
   }

   pItem = getDescriptorItem("Preband Bytes");
   if (pItem != NULL)
   {
      QString prebandBytes;
      if (pRasterFileDescriptor->getInterleaveFormat() == BSQ)
      {
         prebandBytes = QString::number(pRasterFileDescriptor->getPrebandBytes());
      }

      pItem->setText(1, prebandBytes);
   }

   pItem = getDescriptorItem("Postband Bytes");
   if (pItem != NULL)
   {
      QString postbandBytes;
      if (pRasterFileDescriptor->getInterleaveFormat() == BSQ)
      {
         postbandBytes = QString::number(pRasterFileDescriptor->getPostbandBytes());
      }

      pItem->setText(1, postbandBytes);
   }

   pItem = getDescriptorItem("Interleave Format");
   if (pItem != NULL)
   {
      InterleaveFormatType interleave = pRasterFileDescriptor->getInterleaveFormat();
      string interleaveText = StringUtilities::toDisplayString(interleave);

      if (interleave == BSQ)
      {
         const vector<const Filename*>& bandFiles = pRasterFileDescriptor->getBandFiles();
         if (bandFiles.empty() == false)
         {
            interleaveText += BSQ_MULTI_SUFFIX;
         }
         else
         {
            interleaveText += BSQ_SINGLE_SUFFIX;
         }
      }

      pItem->setText(1, QString::fromStdString(interleaveText));

      if (mpInterleaveCombo != NULL)
      {
         int index = mpInterleaveCombo->findText(QString::fromStdString(interleaveText));
         mpInterleaveCombo->setCurrentIndex(index);
      }
   }

   pItem = getDescriptorItem("X Pixel Size");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterFileDescriptor->getXPixelSize()));
   }

   pItem = getDescriptorItem("Y Pixel Size");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterFileDescriptor->getYPixelSize()));
   }

   pItem = getDescriptorItem("Filename");    // This must occur after setting the bands and interleave items
   if (pItem != NULL)
   {
      // Clear the existing band file items
      QList<QTreeWidgetItem*> bandFileItems = pItem->takeChildren();
      for (int i = 0; i < bandFileItems.count(); ++i)
      {
         QTreeWidgetItem* pBandFileItem = bandFileItems[i];
         if (pBandFileItem != NULL)
         {
            delete pBandFileItem;
         }
      }

      // Ensure that the interleave format is BSQ - multiple files
      QTreeWidgetItem* pInterleaveItem = getDescriptorItem("Interleave Format");
      if (pInterleaveItem != NULL)
      {
         if (pInterleaveItem->text(1).toStdString() == (StringUtilities::toDisplayString(BSQ) + BSQ_MULTI_SUFFIX))
         {
            // Get the number of specified bands
            unsigned int numBands = 0;

            QTreeWidgetItem* pBandsItem = getDescriptorItem("Bands");
            if (pBandsItem != NULL)
            {
               QString numBandsText = pBandsItem->text(1);
               numBands = numBandsText.toUInt();
            }

            // Create the band file items based on the band files in the
            // file descriptor to match the specified number of bands
            const vector<const Filename*>& bandFiles = pRasterFileDescriptor->getBandFiles();
            for (unsigned int i = 0; i < numBands; ++i)
            {
               QString bandFilename;
               if (i < bandFiles.size())
               {
                  const Filename* pFilename = bandFiles[i];
                  if (pFilename != NULL)
                  {
                     bandFilename = QString::fromStdString(pFilename->getFullPathAndName());
                  }
               }

               QStringList itemValues;
               itemValues << QString::number(i + 1) << bandFilename;

               QTreeWidgetItem* pBandItem = new QTreeWidgetItem(pItem, itemValues);
               if (mEditable == true)
               {
                  mpTreeWidget->setCellWidgetType(pBandItem, 1, CustomTreeWidget::BROWSE_FILE_EDIT);
                  mpTreeWidget->setFileBrowser(pBandItem, 1, mpFileBrowser);
               }
            }
         }
      }
   }

   if (mpGcpTree != NULL)
   {
      mpGcpTree->clear();

      const list<GcpPoint>& gcps = pRasterFileDescriptor->getGcps();
      if (gcps.empty() == false)
      {
         list<GcpPoint>::const_iterator iter;
         unsigned int i = 0;

         for (iter = gcps.begin(), i = 0; iter != gcps.end(); ++iter, ++i)
         {
            GcpPoint gcp = *iter;

            QTreeWidgetItem* pGcpItem = new QTreeWidgetItem(mpGcpTree);
            if (pGcpItem != NULL)
            {
               QString strLatitude;
               QString strLongitude;
               LatLonPoint latLonPoint(gcp.mCoordinate);

               string latText = latLonPoint.getLatitudeText();
               if (latText.empty() == false)
               {
                  strLatitude = QString::fromStdString(latText);
               }

               string longText = latLonPoint.getLongitudeText();
               if (longText.empty() == false)
               {
                  strLongitude = QString::fromStdString(longText);
               }

               pGcpItem->setText(0, QString("GCP ") + QString::number(i + 1));
               pGcpItem->setText(1, QString::number(gcp.mPixel.mX + 1.0));
               pGcpItem->setText(2, QString::number(gcp.mPixel.mY + 1.0));
               pGcpItem->setText(3, strLatitude);
               pGcpItem->setText(4, strLongitude);
            }
         }

         mpGcpTree->setEnabled(true);
      }
      else
      {
         mpGcpTree->setEnabled(false);
      }
   }

   // Units items
   const Units* pUnits = pRasterFileDescriptor->getUnits();
   if (pUnits != NULL)
   {
      pItem = getDescriptorItem("Name");
      if (pItem != NULL)
      {
         pItem->setText(1, QString::fromStdString(pUnits->getUnitName()));
      }

      pItem = getDescriptorItem("Type");
      if (pItem != NULL)
      {
         UnitType unitType = pUnits->getUnitType();
         string unitTypeText = StringUtilities::toDisplayString(unitType);
         pItem->setText(1, QString::fromStdString(unitTypeText));

         if (mpUnitTypeCombo != NULL)
         {
            int index = mpUnitTypeCombo->findText(QString::fromStdString(unitTypeText));
            mpUnitTypeCombo->setCurrentIndex(index);
         }
      }

      pItem = getDescriptorItem("Scale");
      if (pItem != NULL)
      {
         pItem->setText(1, QString::number(pUnits->getScaleFromStandard()));
      }

      pItem = getDescriptorItem("Range Minimum");
      if (pItem != NULL)
      {
         pItem->setText(1, QString::number(pUnits->getRangeMin()));
      }

      pItem = getDescriptorItem("Range Maximum");
      if (pItem != NULL)
      {
         pItem->setText(1, QString::number(pUnits->getRangeMax()));
      }
   }

   VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
}

QTreeWidgetItem* FileDescriptorWidget::getDescriptorItem(const QString& strName, QTreeWidgetItem* pParentItem) const
{
   if (strName.isEmpty() == true)
   {
      return NULL;
   }

   QTreeWidgetItemIterator iter(mpTreeWidget);
   if (pParentItem != NULL)
   {
      iter = QTreeWidgetItemIterator(pParentItem);
   }

   while (*iter != NULL)
   {
      QTreeWidgetItem* pItem = *iter;
      if (pItem != NULL)
      {
         QString strCurrentName = pItem->text(0);
         if (strCurrentName == strName)
         {
            return pItem;
         }
      }

      ++iter;
   }

   return NULL;
}

void FileDescriptorWidget::descriptorItemChanged(QTreeWidgetItem* pItem, int iColumn)
{
   if ((pItem == NULL) || (iColumn != 1) || (mpFileDescriptor.get() == NULL) || (mEditable == false))
   {
      return;
   }

   QTreeWidgetItem* pParentItem = pItem->parent();
   QString itemName = pItem->text(0);
   QString itemValue = pItem->text(iColumn);

   // File descriptor items
   if (itemName == "Endian")
   {
      if (itemValue.isEmpty() == false)
      {
         bool error = true;
         EndianType endian = StringUtilities::fromDisplayString<EndianType>(itemValue.toStdString(), &error);
         if (error == false)
         {
            mpFileDescriptor->setEndian(endian);
         }
      }
   }

   // Signature file descriptor items
   SignatureFileDescriptor* pSignatureFileDescriptor = dynamic_cast<SignatureFileDescriptor*>(mpFileDescriptor.get());
   if (pSignatureFileDescriptor != NULL)
   {
      set<string> componentNames = pSignatureFileDescriptor->getUnitNames();
      for (set<string>::const_iterator iter = componentNames.begin(); iter != componentNames.end(); ++iter)
      {
         FactoryResource<Units> pUnits;

         const Units* pCurrentUnits = pSignatureFileDescriptor->getUnits(*iter);
         if (pCurrentUnits != NULL)
         {
            pUnits->setUnitName(pCurrentUnits->getUnitName());
            pUnits->setUnitType(pCurrentUnits->getUnitType());
            pUnits->setScaleFromStandard(pCurrentUnits->getScaleFromStandard());
            pUnits->setRangeMin(pCurrentUnits->getRangeMin());
            pUnits->setRangeMax(pCurrentUnits->getRangeMax());
         }

         if (itemName == "Unit Name")
         {
            pUnits->setUnitName(itemValue.toStdString());
         }
         else if (itemName == "Unit Type")
         {
            bool error = true;
            UnitType unitType = StringUtilities::fromDisplayString<UnitType>(itemValue.toStdString(), &error);
            if (error == false)
            {
               pUnits->setUnitType(unitType);
            }
         }
         else if (itemName == "Scale")
         {
            double scale = itemValue.toDouble();
            pUnits->setScaleFromStandard(scale);
         }
         else if (itemName == "Range Minimum")
         {
            double rangeMin = itemValue.toDouble();
            pUnits->setRangeMin(rangeMin);
         }
         else if (itemName == "Range Maximum")
         {
            double rangeMax = itemValue.toDouble();
            pUnits->setRangeMax(rangeMax);
         }

         pSignatureFileDescriptor->setUnits(*iter, pUnits.get());
      }

      return;
   }

   // Raster file descriptor items
   RasterFileDescriptor* pRasterFileDescriptor = dynamic_cast<RasterFileDescriptor*>(mpFileDescriptor.get());
   if (pRasterFileDescriptor == NULL)
   {
      return;
   }

   if (itemName == "Rows")
   {
      vector<DimensionDescriptor> rows;
      if (itemValue.isEmpty() == false)
      {
         rows = RasterUtilities::generateDimensionVector(itemValue.toUInt(), true, false, true);
      }

      pRasterFileDescriptor->setRows(rows);
   }
   else if (itemName == "Columns")
   {
      vector<DimensionDescriptor> columns;
      if (itemValue.isEmpty() == false)
      {
         columns = RasterUtilities::generateDimensionVector(itemValue.toUInt(), true, false, true);
      }

      pRasterFileDescriptor->setColumns(columns);
   }
   else if (itemName == "Bands")
   {
      vector<DimensionDescriptor> bands;
      if (itemValue.isEmpty() == false)
      {
         bands = RasterUtilities::generateDimensionVector(itemValue.toUInt(), true, false, true);
      }

      pRasterFileDescriptor->setBands(bands);

      // Update the band files
      updateBandFiles();
   }
   else if (itemName == "Bits Per Element")
   {
      if (itemValue.isEmpty() == false)
      {
         unsigned int bitsPerElement = itemValue.toUInt();
         pRasterFileDescriptor->setBitsPerElement(bitsPerElement);
      }
   }
   else if (itemName == "Header Bytes")
   {
      unsigned int headerBytes = 0;
      if (itemValue.isEmpty() == false)
      {
         headerBytes = itemValue.toUInt();
      }

      pRasterFileDescriptor->setHeaderBytes(headerBytes);
   }
   else if (itemName == "Trailer Bytes")
   {
      unsigned int trailerBytes = 0;
      if (itemValue.isEmpty() == false)
      {
         trailerBytes = itemValue.toUInt();
      }

      pRasterFileDescriptor->setTrailerBytes(trailerBytes);
   }
   else if (itemName == "Preline Bytes")
   {
      unsigned int prelineBytes = 0;
      if (itemValue.isEmpty() == false)
      {
         prelineBytes = itemValue.toUInt();
      }

      pRasterFileDescriptor->setPrelineBytes(prelineBytes);
   }
   else if (itemName == "Postline Bytes")
   {
      unsigned int postlineBytes = 0;
      if (itemValue.isEmpty() == false)
      {
         postlineBytes = itemValue.toUInt();
      }

      pRasterFileDescriptor->setPostlineBytes(postlineBytes);
   }
   else if (itemName == "X Pixel Size")
   {
      double pixelSize = 1.0;
      if (itemValue.isEmpty() == false)
      {
         pixelSize = itemValue.toDouble();
      }

      pRasterFileDescriptor->setXPixelSize(pixelSize);
   }
   else if (itemName == "Y Pixel Size")
   {
      double pixelSize = 1.0;
      if (itemValue.isEmpty() == false)
      {
         pixelSize = itemValue.toDouble();
      }

      pRasterFileDescriptor->setYPixelSize(pixelSize);
   }
   else if (itemName == "Interleave Format")
   {
      InterleaveFormatType interleave;
      if (itemValue.isEmpty() == false)
      {
         bool error = true;
         if ((itemValue == QString::fromStdString(StringUtilities::toDisplayString(BSQ) + BSQ_SINGLE_SUFFIX)) ||
            (itemValue == QString::fromStdString(StringUtilities::toDisplayString(BSQ) + BSQ_MULTI_SUFFIX)))
         {
            interleave = BSQ;
            error = false;
         }
         else
         {
            interleave = StringUtilities::fromDisplayString<InterleaveFormatType>(itemValue.toStdString(), &error);
         }

         if (error == false)
         {
            pRasterFileDescriptor->setInterleaveFormat(interleave);
         }
      }

      // Enable/disable the preband and postband bytes
      QString prebandBytes;
      QString postbandBytes;
      QColor cellColor = Qt::lightGray;
      CustomTreeWidget::WidgetType cellWidget = CustomTreeWidget::NO_WIDGET;

      if (interleave == BSQ)
      {
         prebandBytes = QString::number(pRasterFileDescriptor->getPrebandBytes());
         postbandBytes = QString::number(pRasterFileDescriptor->getPostbandBytes());
         cellColor = Qt::white;
         cellWidget = CustomTreeWidget::LINE_EDIT;
      }

      QTreeWidgetItem* pPrebandItem = getDescriptorItem("Preband Bytes");
      if (pPrebandItem != NULL)
      {
         pPrebandItem->setText(1, prebandBytes);
         pPrebandItem->setBackgroundColor(1, cellColor);
         mpTreeWidget->setCellWidgetType(pPrebandItem, 1, cellWidget);
      }

      QTreeWidgetItem* pPostbandItem = getDescriptorItem("Postband Bytes");
      if (pPostbandItem != NULL)
      {
         pPostbandItem->setText(1, postbandBytes);
         pPostbandItem->setBackgroundColor(1, cellColor);
         mpTreeWidget->setCellWidgetType(pPostbandItem, 1, cellWidget);
      }

      // Update the band files
      updateBandFiles();
   }
   else if (itemName == "Preband Bytes")
   {
      unsigned int prebandBytes = 0;
      if (itemValue.isEmpty() == false)
      {
         prebandBytes = itemValue.toUInt();
      }

      pRasterFileDescriptor->setPrebandBytes(prebandBytes);
   }
   else if (itemName == "Postband Bytes")
   {
      unsigned int postbandBytes = 0;
      if (itemValue.isEmpty() == false)
      {
         postbandBytes = itemValue.toUInt();
      }

      pRasterFileDescriptor->setPostbandBytes(postbandBytes);
   }
   else if ((pParentItem != NULL) && (pParentItem == getDescriptorItem("Filename")))
   {
      vector<string> bandFiles;
      for (int i = 0; i < pParentItem->childCount(); ++i)
      {
         QTreeWidgetItem* pBandFileItem = pParentItem->child(i);
         if (pBandFileItem != NULL)
         {
            QString bandFile = pBandFileItem->text(1);
            bandFiles.push_back(bandFile.toStdString());
         }
      }

      pRasterFileDescriptor->setBandFiles(bandFiles);
   }
   else
   {
      Units* pUnits = pRasterFileDescriptor->getUnits();
      if ((pUnits != NULL) && (itemValue.isEmpty() == false))
      {
         if (itemName == "Name")
         {
            pUnits->setUnitName(itemValue.toStdString());
         }
         else if (itemName == "Type")
         {
            bool error = true;
            UnitType unitType = StringUtilities::fromDisplayString<UnitType>(itemValue.toStdString(), &error);
            if (error == false)
            {
               pUnits->setUnitType(unitType);
            }
         }
         else if (itemName == "Scale")
         {
            double scale = itemValue.toDouble();
            pUnits->setScaleFromStandard(scale);
         }
         else if (itemName == "Range Minimum")
         {
            double rangeMin = itemValue.toDouble();
            pUnits->setRangeMin(rangeMin);
         }
         else if (itemName == "Range Maximum")
         {
            double rangeMax = itemValue.toDouble();
            pUnits->setRangeMax(rangeMax);
         }
      }
   }
}

void FileDescriptorWidget::updateBandFiles()
{
   // The number of bands or the interleave format changed, so update both the number
   // of tree widget items and the band files in the raster data descriptor
   vector<string> bandFiles;

   QTreeWidgetItem* pFilenameItem = getDescriptorItem("Filename");
   if (pFilenameItem != NULL)
   {
      // Store the current band filenames
      QStringList bandFilenames;
      for (int i = 0; i < pFilenameItem->childCount(); ++i)
      {
         QTreeWidgetItem* pBandFileItem = pFilenameItem->child(i);
         if (pBandFileItem != NULL)
         {
            bandFilenames.append(pBandFileItem->text(1));
         }
      }

      // Clear the existing band file items
      QList<QTreeWidgetItem*> bandFileItems = pFilenameItem->takeChildren();
      for (int i = 0; i < bandFileItems.count(); ++i)
      {
         QTreeWidgetItem* pBandFileItem = bandFileItems[i];
         if (pBandFileItem != NULL)
         {
            delete pBandFileItem;
         }
      }

      // Only create new band file items for BSQ - multiple files
      QTreeWidgetItem* pInterleaveItem = getDescriptorItem("Interleave Format");
      if (pInterleaveItem != NULL)
      {
         if (pInterleaveItem->text(1).toStdString() == (StringUtilities::toDisplayString(BSQ) + BSQ_MULTI_SUFFIX))
         {
            // Get the number of bands
            int numBands = 0;

            QTreeWidgetItem* pBandsItem = getDescriptorItem("Bands");
            if (pBandsItem != NULL)
            {
               QString strNumBands = pBandsItem->text(1);
               numBands = strNumBands.toInt();
            }

            // Create the band file items based on the number of specified bands
            for (int i = 0; i < numBands; ++i)
            {
               // Set the initial filename to the previous value if possible
               QString strBandFilename;
               if (i < bandFilenames.count())
               {
                  strBandFilename = bandFilenames[i];
               }

               QStringList text;
               text << QString::number(i + 1) << strBandFilename;

               QTreeWidgetItem* pBandItem = new QTreeWidgetItem(pFilenameItem, text);
               if (mEditable == true)
               {
                  mpTreeWidget->setCellWidgetType(pBandItem, 1, CustomTreeWidget::BROWSE_FILE_EDIT);
                  mpTreeWidget->setFileBrowser(pBandItem, 1, mpFileBrowser);
               }

               bandFiles.push_back(strBandFilename.toStdString());
            }
         }
      }
   }

   // Update the bands in the raster file descriptor
   RasterFileDescriptor* pRasterFileDescriptor = dynamic_cast<RasterFileDescriptor*>(mpFileDescriptor.get());
   if (pRasterFileDescriptor != NULL)
   {
      pRasterFileDescriptor->setBandFiles(bandFiles);
   }
}
