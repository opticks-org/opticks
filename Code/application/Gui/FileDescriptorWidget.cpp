/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>

#include "AppVerify.h"
#include "CustomTreeWidget.h"
#include "DimensionDescriptor.h"
#include "FileBrowser.h"
#include "FileDescriptorWidget.h"
#include "GeoPoint.h"
#include "ObjectResource.h"
#include "RasterFileDescriptor.h"
#include "SignatureFileDescriptor.h"
#include "StringUtilities.h"
#include "Units.h"

#include <string>
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
   mReadOnly(true),
   mModified(false),
   mpTreeWidget(NULL),
   mpFileBrowser(NULL),
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
}

void FileDescriptorWidget::setFileDescriptor(FileDescriptor* pFileDescriptor)
{
   mpTreeWidget->closeActiveCellWidget(false);

   mpFileDescriptor = pFileDescriptor;
   mReadOnly = false;
   initialize();
   mModified = false;
}

void FileDescriptorWidget::setFileDescriptor(const FileDescriptor* pFileDescriptor)
{
   mpTreeWidget->closeActiveCellWidget(false);

   mpFileDescriptor = const_cast<FileDescriptor*>(pFileDescriptor);
   mReadOnly = true;
   initialize();
   mModified = false;
}

void FileDescriptorWidget::setDescriptorValue(const QString& strValueName, const QString& strValue)
{
   QTreeWidgetItem* pItem = getDescriptorItem(strValueName);
   if (pItem != NULL)
   {
      pItem->setText(1, strValue);
   }
}

QString FileDescriptorWidget::getDescriptorValue(const QString& strValueName) const
{
   QString strValue;
   if (strValueName.isEmpty() == false)
   {
      const QTreeWidgetItem* pItem = getDescriptorItem(strValueName);
      if (pItem != NULL)
      {
         strValue = pItem->text(1);
         if (strValueName == "Interleave Format")
         {
            strValue = strValue.left(3);  // only return the interleave not single/multi file info
         }
      }
   }

   return strValue;
}

void FileDescriptorWidget::initialize()
{
   mpTreeWidget->clear();
   mpGcpTree->clear();
   mpGcpGroup->hide();

   if (mpFileDescriptor == NULL)
   {
      return;
   }

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

      if (mReadOnly == false)
      {
         QComboBox* pEndianCombo = new QComboBox(mpTreeWidget);
         pEndianCombo->setEditable(false);
         pEndianCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(LITTLE_ENDIAN_ORDER)));
         pEndianCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BIG_ENDIAN_ORDER)));
         pEndianCombo->hide();

         mpTreeWidget->setCellWidgetType(pEndianItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pEndianItem, 1, pEndianCombo);
      }
   }

   // Signature file descriptor item
   SignatureFileDescriptor* pSignatureDescriptor = dynamic_cast<SignatureFileDescriptor*>(mpFileDescriptor);
   if (pSignatureDescriptor != NULL)
   {
      QTreeWidgetItem* pUnitsItem = new QTreeWidgetItem(mpTreeWidget);
      if (pUnitsItem != NULL)
      {
         pUnitsItem->setText(0, "Units");
         set<string> unitNames = pSignatureDescriptor->getUnitNames();
         if (unitNames.empty() == false)
         {
            pUnitsItem->setBackgroundColor(1, Qt::lightGray);
            for (set<string>::const_iterator it = unitNames.begin(); it != unitNames.end(); ++it)
            {
               string unitName = *it;
               const Units* pUnits = pSignatureDescriptor->getUnits(unitName);
               if (pUnits == NULL)
               {
                  continue;
               }

               // unit item
               QTreeWidgetItem* pNamedItem = new QTreeWidgetItem(pUnitsItem);
               if (pNamedItem == NULL)
               {
                  continue;
               }
               pNamedItem->setText(0, QString::fromStdString(unitName));
               pNamedItem->setBackgroundColor(1, Qt::lightGray);

               // Name
               QTreeWidgetItem* pUnitNameItem = new QTreeWidgetItem(pNamedItem);
               if (pUnitNameItem != NULL)
               {
                  string unitsName = pUnits->getUnitName();

                  pUnitNameItem->setText(0, "Unit Name");
                  pUnitNameItem->setText(1, QString::fromStdString(unitsName));

                  if (mReadOnly == false)
                  {
                     mpTreeWidget->setCellWidgetType(pUnitNameItem, 1, CustomTreeWidget::LINE_EDIT);
                  }
               }

               // Type
               QTreeWidgetItem* pUnitTypeItem = new QTreeWidgetItem(pNamedItem);
               if (pUnitTypeItem != NULL)
               {
                  UnitType unitType = pUnits->getUnitType();
                  string unitTypeText = StringUtilities::toDisplayString(unitType);

                  pUnitTypeItem->setText(0, "Unit Type");
                  pUnitTypeItem->setText(1, QString::fromStdString(unitTypeText));

                  if (mReadOnly == false)
                  {
                     QComboBox* pUnitTypeCombo = new QComboBox(mpTreeWidget);
                     pUnitTypeCombo->setEditable(false);
                     pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(ABSORBANCE)));
                     pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(ABSORPTANCE)));
                     pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DIGITAL_NO)));
                     pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DISTANCE)));
                     pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(EMISSIVITY)));
                     pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(RADIANCE)));
                     pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(REFLECTANCE)));
                     pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(REFLECTANCE_FACTOR)));
                     pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(TRANSMITTANCE)));
                     pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(CUSTOM_UNIT)));
                     pUnitTypeCombo->hide();

                     mpTreeWidget->setCellWidgetType(pUnitTypeItem, 1, CustomTreeWidget::COMBO_BOX);
                     mpTreeWidget->setComboBox(pUnitTypeItem, 1, pUnitTypeCombo);
                  }
               }

               // Scale
               QTreeWidgetItem* pScaleItem = new QTreeWidgetItem(pNamedItem);
               if (pScaleItem != NULL)
               {
                  double dScale = pUnits->getScaleFromStandard();

                  pScaleItem->setText(0, "Scale");
                  pScaleItem->setText(1, QString::number(dScale));

                  if (mReadOnly == false)
                  {
                     mpTreeWidget->setCellWidgetType(pScaleItem, 1, CustomTreeWidget::LINE_EDIT);
                  }
               }

               // Range minimum
               QTreeWidgetItem* pMinimumItem = new QTreeWidgetItem(pNamedItem);
               if (pMinimumItem != NULL)
               {
                  double dMinimum = pUnits->getRangeMin();

                  pMinimumItem->setText(0, "Range Minimum");
                  pMinimumItem->setText(1, QString::number(dMinimum));

                  if (mReadOnly == false)
                  {
                     mpTreeWidget->setCellWidgetType(pMinimumItem, 1, CustomTreeWidget::LINE_EDIT);
                  }
               }

               // Range maximum
               QTreeWidgetItem* pMaximumItem = new QTreeWidgetItem(pNamedItem);
               if (pMaximumItem != NULL)
               {
                  double dMaximum = pUnits->getRangeMax();

                  pMaximumItem->setText(0, "Range Maximum");
                  pMaximumItem->setText(1, QString::number(dMaximum));

                  if (mReadOnly == false)
                  {
                     mpTreeWidget->setCellWidgetType(pMaximumItem, 1, CustomTreeWidget::LINE_EDIT);
                  }
               }
            }
         }
         else  // no Units associated with components in this signature
         {
            pUnitsItem->setText(1, "No Units defined");
         }
      }

      return;
   }

   // Raster element file descriptor items
   RasterFileDescriptor* pRasterDescriptor = dynamic_cast<RasterFileDescriptor*>(mpFileDescriptor);
   if (pRasterDescriptor == NULL)
   {
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
      if (mReadOnly == false)
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
      if (mReadOnly == false)
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
      if (mReadOnly == false)
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

      if (mReadOnly == false)
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

      if (mReadOnly == false)
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

      if (mReadOnly == false)
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

      if (mReadOnly == false)
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

      if (mReadOnly == false)
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

      if (mReadOnly == false)
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

            if (mReadOnly == false)
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

            if (mReadOnly == false)
            {
               QComboBox* pUnitTypeCombo = new QComboBox(mpTreeWidget);
               pUnitTypeCombo->setEditable(false);
               pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(ABSORBANCE)));
               pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(ABSORPTANCE)));
               pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DIGITAL_NO)));
               pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DISTANCE)));
               pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(EMISSIVITY)));
               pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(RADIANCE)));
               pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(REFLECTANCE)));
               pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(REFLECTANCE_FACTOR)));
               pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(TRANSMITTANCE)));
               pUnitTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(CUSTOM_UNIT)));
               pUnitTypeCombo->hide();

               mpTreeWidget->setCellWidgetType(pTypeItem, 1, CustomTreeWidget::COMBO_BOX);
               mpTreeWidget->setComboBox(pTypeItem, 1, pUnitTypeCombo);
            }
         }

         // Scale
         QTreeWidgetItem* pScaleItem = new QTreeWidgetItem(pUnitsItem);
         if (pScaleItem != NULL)
         {
            double dScale = pUnits->getScaleFromStandard();

            pScaleItem->setText(0, "Scale");
            pScaleItem->setText(1, QString::number(dScale));

            if (mReadOnly == false)
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

            if (mReadOnly == false)
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

            if (mReadOnly == false)
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
      if (mReadOnly == false)
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
      if (mReadOnly == false)
      {
         QComboBox* pInterleaveCombo = new QComboBox(mpTreeWidget);
         pInterleaveCombo->setEditable(false);
         pInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BIL)));
         pInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BIP)));
         pInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BSQ) + BSQ_SINGLE_SUFFIX));
         pInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BSQ) + BSQ_MULTI_SUFFIX));
         pInterleaveCombo->hide();

         mpTreeWidget->setCellWidgetType(pInterleaveItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pInterleaveItem, 1, pInterleaveCombo);
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

            if (mReadOnly == false)
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
      if ((mReadOnly == false) && (interleave == BSQ))
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
      if ((mReadOnly == false) && (interleave == BSQ))
      {
         mpTreeWidget->setCellWidgetType(pPostbandBytesItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }
}

bool FileDescriptorWidget::isModified() const
{
   return mModified;
}

bool FileDescriptorWidget::applyChanges()
{
   return applyToFileDescriptor(mpFileDescriptor);
}

bool FileDescriptorWidget::applyToFileDescriptor(FileDescriptor* pFileDescriptor)
{
   if (mReadOnly == true)
   {
      return true;
   }

   if (mModified == false)
   {
      return true;
   }

   if (pFileDescriptor == NULL)
   {
      return true;
   }

   // Endian
   QString strEndian = getDescriptorValue("Endian");
   if (strEndian.isEmpty() == false)
   {
      bool bError = true;
      EndianType endian = StringUtilities::fromDisplayString<EndianType>(strEndian.toStdString(), &bError);
      if (bError == false)
      {
         pFileDescriptor->setEndian(endian);
      }
   }

   // SignatureFileDescriptor items
   SignatureFileDescriptor* pSignatureDescriptor = dynamic_cast<SignatureFileDescriptor*>(mpFileDescriptor);
   if (pSignatureDescriptor != NULL)
   {
      set<string> dataNames = pSignatureDescriptor->getUnitNames();
      if (dataNames.empty())
      {
         return true;
      }
      for (set<string>::const_iterator it = dataNames.begin(); it != dataNames.end(); ++it)
      {
         // data name
         QTreeWidgetItem* pItem = getDescriptorItem(QString::fromStdString(*it));

         // Units
         FactoryResource<Units> pUnits;
         if (pUnits.get() != NULL)
         {
            // Name
            QTreeWidgetItem* pSubItem = getDescriptorItem("Unit Name", pItem);
            if (pSubItem != NULL)
            {
               QString strUnitsName = pSubItem->text(1);
               if (strUnitsName.isEmpty() == false)
               {
                  pUnits->setUnitName(strUnitsName.toStdString());
               }
            }

            // Type
            pSubItem = getDescriptorItem("Unit Type", pItem);
            if (pSubItem != NULL)
            {
               QString strUnitsType = pSubItem->text(1);
               if (strUnitsType.isEmpty() == false)
               {
                  bool bError = true;
                  UnitType unitType =
                     StringUtilities::fromDisplayString<UnitType>(strUnitsType.toStdString(), &bError);
                  if (bError == false)
                  {
                     pUnits->setUnitType(unitType);
                  }
               }
            }

            // Scale
            pSubItem = getDescriptorItem("Scale", pItem);
            if (pSubItem != NULL)
            {
               QString strUnitsScale = pSubItem->text(1);
               if (strUnitsScale.isEmpty() == false)
               {
                  double dScale = strUnitsScale.toDouble();
                  pUnits->setScaleFromStandard(dScale);
               }

               // Range minimum
               pSubItem = getDescriptorItem("Range Minimum", pItem);
               if (pSubItem != NULL)
               {
                  QString strMinimum = pSubItem->text(1);
                  if (strMinimum.isEmpty() == false)
                  {
                     double dMinimum = strMinimum.toDouble();
                     pUnits->setRangeMin(dMinimum);
                  }
               }
            }

            // Range maximum
            pSubItem = getDescriptorItem("Range Maximum", pItem);
            if (pSubItem != NULL)
            {
               QString strMaximum = pSubItem->text(1);
               if (strMaximum.isEmpty() == false)
               {
                  double dMaximum = strMaximum.toDouble();
                  pUnits->setRangeMax(dMaximum);
               }
            }

            pSignatureDescriptor->setUnits(*it, pUnits.get());
         }
      }

      return true;
   }

   // Raster element file descriptor items
   RasterFileDescriptor* pRasterDescriptor = dynamic_cast<RasterFileDescriptor*>(pFileDescriptor);
   if (pRasterDescriptor == NULL)
   {
      return true;
   }

   // Bits per element
   QString strBitsPerElement = getDescriptorValue("Bits Per Element");
   if (strBitsPerElement.isEmpty() == false)
   {
      unsigned int bitsPerElement = strBitsPerElement.toUInt();
      pRasterDescriptor->setBitsPerElement(bitsPerElement);
   }

   // Header bytes
   QString strHeaderBytes = getDescriptorValue("Header Bytes");
   if (strHeaderBytes.isEmpty() == false)
   {
      unsigned int headerBytes = strHeaderBytes.toUInt();
      pRasterDescriptor->setHeaderBytes(headerBytes);
   }

   // Trailer bytes
   QString strTrailerBytes = getDescriptorValue("Trailer Bytes");
   if (strTrailerBytes.isEmpty() == false)
   {
      unsigned int trailerBytes = strTrailerBytes.toUInt();
      pRasterDescriptor->setTrailerBytes(trailerBytes);
   }

   // Preline bytes
   QString strPrelineBytes = getDescriptorValue("Preline Bytes");
   if (strPrelineBytes.isEmpty() == false)
   {
      unsigned int prelineBytes = strPrelineBytes.toUInt();
      pRasterDescriptor->setPrelineBytes(prelineBytes);
   }

   // Postline bytes
   QString strPostlineBytes = getDescriptorValue("Postline Bytes");
   if (strPostlineBytes.isEmpty() == false)
   {
      unsigned int postlineBytes = strPostlineBytes.toUInt();
      pRasterDescriptor->setPostlineBytes(postlineBytes);
   }

   // Pixel size
   QString strPixelSize = getDescriptorValue("X Pixel Size");
   if (strPixelSize.isEmpty() == false)
   {
      pRasterDescriptor->setXPixelSize(strPixelSize.toDouble());
   }

   strPixelSize = getDescriptorValue("Y Pixel Size");
   if (strPixelSize.isEmpty() == false)
   {
      pRasterDescriptor->setYPixelSize(strPixelSize.toDouble());
   }

   // Units
   Units* pUnits = pRasterDescriptor->getUnits();
   if (pUnits != NULL)
   {
      // Name
      QString strUnitsName = getDescriptorValue("Name");
      if (strUnitsName.isEmpty() == false)
      {
         pUnits->setUnitName(strUnitsName.toStdString());
      }

      // Type
      QString strUnitsType = getDescriptorValue("Type");
      if (strUnitsType.isEmpty() == false)
      {
         UnitType unitType;

         bool bError = true;
         unitType = StringUtilities::fromDisplayString<UnitType>(strUnitsType.toStdString(), &bError);
         if (bError == false)
         {
            pUnits->setUnitType(unitType);
         }
      }

      // Scale
      QString strUnitsScale = getDescriptorValue("Scale");
      if (strUnitsScale.isEmpty() == false)
      {
         double dScale = strUnitsScale.toDouble();
         pUnits->setScaleFromStandard(dScale);
      }

      // Range minimum
      QString strMinimum = getDescriptorValue("Range Minimum");
      if (strMinimum.isEmpty() == false)
      {
         double dMinimum = strMinimum.toDouble();
         pUnits->setRangeMin(dMinimum);
      }

      // Range maximum
      QString strMaximum = getDescriptorValue("Range Maximum");
      if (strMaximum.isEmpty() == false)
      {
         double dMaximum = strMaximum.toDouble();
         pUnits->setRangeMax(dMaximum);
      }
   }

   // Interleave format
   QString strInterleave = getDescriptorValue("Interleave Format");
   if (strInterleave.isEmpty() == false)
   {
      bool bError = true;
      string interleaveVal = strInterleave.toStdString();
      InterleaveFormatType interleave;
      if ((interleaveVal == StringUtilities::toDisplayString(BSQ) + BSQ_SINGLE_SUFFIX) ||
          (interleaveVal == StringUtilities::toDisplayString(BSQ) + BSQ_MULTI_SUFFIX))
      {
         interleave = BSQ;
         bError = false;
      }
      else
      {
         interleave = StringUtilities::fromDisplayString<InterleaveFormatType>(interleaveVal, &bError);
      }
      if (bError == false)
      {
         pRasterDescriptor->setInterleaveFormat(interleave);
      }
   }

   // Band files
   vector<string> bandFiles;

   QTreeWidgetItem* pFilenameItem = getDescriptorItem("Filename");
   if (pFilenameItem != NULL)
   {
      bool bMultipleFiles = false;
      for (int i = 0; i < pFilenameItem->childCount(); ++i)
      {
         QTreeWidgetItem* pBandFileItem = pFilenameItem->child(i);
         if (pBandFileItem != NULL)
         {
            string bandFile;

            QString strBandFile = pBandFileItem->text(1);
            if (strBandFile.isEmpty() == false)
            {
               bandFile = strBandFile.toStdString();
               bMultipleFiles = true;
            }

            bandFiles.push_back(bandFile);
         }
      }

      if (bMultipleFiles == false)
      {
         bandFiles.clear();
      }
   }

   pRasterDescriptor->setBandFiles(bandFiles);

   // Preband bytes
   QString strPrebandBytes = getDescriptorValue("Preband Bytes");
   if (strPrebandBytes.isEmpty() == false)
   {
      unsigned int prebandBytes = strPrebandBytes.toUInt();
      pRasterDescriptor->setPrebandBytes(prebandBytes);
   }

   // Postband bytes
   QString strPostbandBytes = getDescriptorValue("Postband Bytes");
   if (strPostbandBytes.isEmpty() == false)
   {
      unsigned int postbandBytes = strPostbandBytes.toUInt();
      pRasterDescriptor->setPostbandBytes(postbandBytes);
   }

   return true;
}

QSize FileDescriptorWidget::sizeHint() const
{
   return QSize(575, 325);
}

QTreeWidgetItem* FileDescriptorWidget::getDescriptorItem(const QString& strName, QTreeWidgetItem* pStartAt) const
{
   if (strName.isEmpty())
   {
      return NULL;
   }

   QTreeWidgetItemIterator iter(mpTreeWidget);
   if (pStartAt != NULL)
   {
      iter = QTreeWidgetItemIterator(pStartAt);
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
   if ((pItem == NULL) || (iColumn != 1))
   {
      return;
   }

   QString strItem = pItem->text(0);
   QString strValue = pItem->text(1);

   if (strItem == "Interleave Format")
   {
      // Enable/disable the preband and postband bytes
      QString strPrebandBytes;
      QString strPostbandBytes;
      QColor cellColor = Qt::lightGray;
      CustomTreeWidget::WidgetType cellWidget = CustomTreeWidget::NO_WIDGET;

      string value = strValue.toStdString();
      if ((value == StringUtilities::toDisplayString(BSQ) + BSQ_SINGLE_SUFFIX) ||
          (value == StringUtilities::toDisplayString(BSQ) + BSQ_MULTI_SUFFIX))
      {
         const RasterFileDescriptor* pRasterFileDescriptor =
            dynamic_cast<const RasterFileDescriptor*>(mpFileDescriptor);
         if (pRasterFileDescriptor != NULL)
         {
            strPrebandBytes = QString::number(pRasterFileDescriptor->getPrebandBytes());
            strPostbandBytes = QString::number(pRasterFileDescriptor->getPostbandBytes());
            cellColor = Qt::white;
            cellWidget = CustomTreeWidget::LINE_EDIT;
         }
      }

      QTreeWidgetItem* pPrebandItem = getDescriptorItem("Preband Bytes");
      if (pPrebandItem != NULL)
      {
         pPrebandItem->setText(1, strPrebandBytes);
         pPrebandItem->setBackgroundColor(1, cellColor);
         mpTreeWidget->setCellWidgetType(pPrebandItem, 1, cellWidget);
      }

      QTreeWidgetItem* pPostbandItem = getDescriptorItem("Postband Bytes");
      if (pPostbandItem != NULL)
      {
         pPostbandItem->setText(1, strPostbandBytes);
         pPostbandItem->setBackgroundColor(1, cellColor);
         mpTreeWidget->setCellWidgetType(pPostbandItem, 1, cellWidget);
      }

      // Update the number of band file items
      updateBandFiles();
   }
   else if (strItem == "Bands")
   {
      // Update the number of band file items
      updateBandFiles();
   }

   mModified = true;
   emit valueChanged(strItem);
   emit modified();
}

void FileDescriptorWidget::updateBandFiles()
{
   QTreeWidgetItem* pFilenameItem = getDescriptorItem("Filename");
   if (pFilenameItem == NULL)
   {
      return;
   }

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

   // Ensure that the interleave format is BSQ - multiple files
   bool bBsqMulti(false);

   QTreeWidgetItem* pInterleaveItem = getDescriptorItem("Interleave Format");
   if (pInterleaveItem != NULL)
   {
      if (pInterleaveItem->text(1).toStdString() == (StringUtilities::toDisplayString(BSQ) + BSQ_MULTI_SUFFIX))
      {
         bBsqMulti = true;
      }
   }

   if (bBsqMulti == false)
   {
      return;
   }

   // Get the number of bands
   int numBands = 0;

   QTreeWidgetItem* pBandsItem = getDescriptorItem("Bands");
   if (pBandsItem != NULL)
   {
      QString strNumBands = pBandsItem->text(1);
      numBands = strNumBands.toInt();
   }

   // Create the new band file items
   for (int i = 0; i < numBands; ++i)
   {
      QTreeWidgetItem* pBandItem = new QTreeWidgetItem(pFilenameItem);
      if (pBandItem != NULL)
      {
         // Set the initial filename to the previous value
         QString strBandFilename;
         if (i < bandFilenames.count())
         {
            strBandFilename = bandFilenames[i];
         }

         pBandItem->setText(0, QString::number(i + 1));
         pBandItem->setText(1, strBandFilename);

         if (mReadOnly == false)
         {
            mpTreeWidget->setCellWidgetType(pBandItem, 1, CustomTreeWidget::BROWSE_FILE_EDIT);
            mpTreeWidget->setFileBrowser(pBandItem, 1, mpFileBrowser);
         }
      }
   }
}
