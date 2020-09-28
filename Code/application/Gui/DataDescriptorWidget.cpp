/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "BadValues.h"
#include "BadValuesDlg.h"
#include "DataDescriptorWidget.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "ObjectResource.h"
#include "PointCloudDataDescriptor.h"
#include "PropertiesRasterLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SignatureDataDescriptor.h"
#include "SignatureFileDescriptor.h"
#include "Slot.h"
#include "StringUtilities.h"
#include "Units.h"

#include <QtCore/QRegExp>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtGui/QFocusEvent>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtGui/QRegExpValidator>

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

using namespace std;

DataDescriptorWidget::DataDescriptorWidget(QWidget* pParent) :
   QWidget(pParent),
   mpDescriptor(NULL),
   mEditAll(false),
   mNeedsInitialization(false),
   mpTreeWidget(NULL),
   mpProcessingLocationCombo(NULL),
   mpDataTypeCombo(NULL),
   mpUnitTypeCombo(NULL),
   mpInterleaveCombo(NULL),
   mpDisplayModeCombo(NULL),
   mpSetDisplayButton(NULL)
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
      pHeader->setSectionsMovable(false);
      pHeader->setStretchLastSection(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->resizeSection(0, 150);
   }

   // Combo boxes
   mpProcessingLocationCombo = new QComboBox(mpTreeWidget);
   mpProcessingLocationCombo->setEditable(false);
   mpProcessingLocationCombo->hide();

   mpDataTypeCombo = new QComboBox(mpTreeWidget);
   mpDataTypeCombo->setEditable(false);
   mpDataTypeCombo->hide();

   mpUnitTypeCombo = new QComboBox(mpTreeWidget);
   mpUnitTypeCombo->setEditable(false);

   vector<string> unitNames = StringUtilities::getAllEnumValuesAsDisplayString<UnitType>();
   sort(unitNames.begin(), unitNames.end());
   for (vector<string>::const_iterator iter = unitNames.begin(); iter != unitNames.end(); ++iter)
   {
      mpUnitTypeCombo->addItem(QString::fromStdString(*iter));
   }

   mpUnitTypeCombo->hide();

   mpInterleaveCombo = new QComboBox(mpTreeWidget);
   mpInterleaveCombo->setEditable(false);
   mpInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BIL)));
   mpInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BIP)));
   mpInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BSQ)));
   mpInterleaveCombo->hide();

   mpDisplayModeCombo = new QComboBox(mpTreeWidget);
   mpDisplayModeCombo->setEditable(false);
   mpDisplayModeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(GRAYSCALE_MODE)));
   mpDisplayModeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(RGB_MODE)));
   mpDisplayModeCombo->hide();

   // Set display bands push button
   mpSetDisplayButton = new QPushButton("Set Display Bands", this);
   QMenu* pMenu = new QMenu(mpSetDisplayButton);
   mpSetDisplayButton->setMenu(pMenu);

   // create custom edit widget for bad Values
   mpBadValuesEdit = new BadValuesEdit(mpTreeWidget);
   mpBadValuesEdit->hide();

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(mpTreeWidget);
   pLayout->addWidget(mpSetDisplayButton, 0, Qt::AlignRight);

   // Connections
   VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
   VERIFYNR(connect(pMenu, SIGNAL(triggered(QAction*)), this, SLOT(setDisplayBands(QAction*))));
}

DataDescriptorWidget::~DataDescriptorWidget()
{
   setDataDescriptor(NULL, false);
}

void DataDescriptorWidget::setDataDescriptor(DataDescriptor* pDescriptor, bool editAll)
{
   mpTreeWidget->closeActiveCellWidget(false);

   if (mpDescriptor.get() != NULL)
   {
      if (isVisible() == false)
      {
         VERIFYNR(mpDescriptor->detach(SIGNAL_NAME(Subject, Modified),
            Slot(this, &DataDescriptorWidget::dataDescriptorModified)));
      }

      SignatureFileDescriptor* pSignatureFileDescriptor =
         dynamic_cast<SignatureFileDescriptor*>(mpDescriptor->getFileDescriptor());
      if (pSignatureFileDescriptor != NULL)
      {
         VERIFYNR(pSignatureFileDescriptor->detach(SIGNAL_NAME(SignatureFileDescriptor, UnitsChanged),
            Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
      }

      RasterFileDescriptor* pRasterFileDescriptor =
         dynamic_cast<RasterFileDescriptor*>(mpDescriptor->getFileDescriptor());
      if (pRasterFileDescriptor != NULL)
      {
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, RowsChanged),
            Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, ColumnsChanged),
            Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, BandsChanged),
            Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, BitsPerElementChanged),
            Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, InterleaveFormatChanged),
            Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
         VERIFYNR(pRasterFileDescriptor->detach(SIGNAL_NAME(RasterFileDescriptor, PixelSizeChanged),
            Slot(this, &DataDescriptorWidget::fileDescriptorModified)));

         Units* pFileUnits = pRasterFileDescriptor->getUnits();
         if (pFileUnits != NULL)
         {
            VERIFYNR(pFileUnits->detach(SIGNAL_NAME(Units, Renamed),
               Slot(this, &DataDescriptorWidget::fileDescriptorUnitsModified)));
            VERIFYNR(pFileUnits->detach(SIGNAL_NAME(Units, TypeChanged),
               Slot(this, &DataDescriptorWidget::fileDescriptorUnitsModified)));
            VERIFYNR(pFileUnits->detach(SIGNAL_NAME(Units, ScaleChanged),
               Slot(this, &DataDescriptorWidget::fileDescriptorUnitsModified)));
            VERIFYNR(pFileUnits->detach(SIGNAL_NAME(Units, RangeChanged),
               Slot(this, &DataDescriptorWidget::fileDescriptorUnitsModified)));
         }
      }
   }

   mpDescriptor.reset(pDescriptor);
   mEditAll = editAll;
   mpTreeWidget->clear();
   mpSetDisplayButton->setEnabled(false);
   mpSetDisplayButton->setVisible(mEditAll);

   if (mpDescriptor.get() == NULL)
   {
      return;
   }

   // Only attach to the data descriptor to update the tree widget if this widget is currently hidden.  This prevents
   // a potentially large amount of time being spent updating the tree widget items when the data descriptor changes
   // externally.  This assumes that the data descriptor will not updated externally when this page is shown, which
   // means that this widget must exist in a modal dialog or other modal widget.
   if (isVisible() == false)
   {
      VERIFYNR(mpDescriptor->attach(SIGNAL_NAME(Subject, Modified),
         Slot(this, &DataDescriptorWidget::dataDescriptorModified)));
   }

   VERIFYNR(disconnect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));

   // Name
   QTreeWidgetItem* pNameItem = new QTreeWidgetItem(mpTreeWidget);
   if (pNameItem != NULL)
   {
      const string& name = mpDescriptor->getName();

      pNameItem->setText(0, "Name");
      pNameItem->setText(1, QString::fromStdString(name));
   }

   // Type
   QTreeWidgetItem* pTypeItem = new QTreeWidgetItem(mpTreeWidget);
   if (pTypeItem != NULL)
   {
      const string& type = mpDescriptor->getType();

      pTypeItem->setText(0, "Type");
      pTypeItem->setText(1, QString::fromStdString(type));
   }

   // Parent
   QTreeWidgetItem* pDatasetItem = new QTreeWidgetItem(mpTreeWidget);
   if (pDatasetItem != NULL)
   {
      QString strParent;

      DataElement* pParent = mpDescriptor->getParent();
      if (pParent != NULL)
      {
         strParent = QString::fromStdString(pParent->getName());
      }
      else
      {
         vector<string> parentDesignator = mpDescriptor->getParentDesignator();
         if (parentDesignator.empty() == false)
         {
            strParent = QString::fromStdString(parentDesignator.back());
         }
      }

      pDatasetItem->setText(0, "Parent Data Set");
      pDatasetItem->setText(1, strParent);
   }

   // Processing location
   QTreeWidgetItem* pProcessingLocationItem = new QTreeWidgetItem(mpTreeWidget);
   if (pProcessingLocationItem != NULL)
   {
      ProcessingLocation processingLocation = mpDescriptor->getProcessingLocation();
      string processingLocationText = StringUtilities::toDisplayString(processingLocation);

      pProcessingLocationItem->setText(0, "Processing Location");
      pProcessingLocationItem->setText(1, QString::fromStdString(processingLocationText));

      // Set a combo box as the edit widget
      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pProcessingLocationItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pProcessingLocationItem, 1, mpProcessingLocationCombo);
      }
   }

   PointCloudDataDescriptor* pPointDescriptor = dynamic_cast<PointCloudDataDescriptor*>(mpDescriptor.get());
   if (pPointDescriptor != NULL)
   {
      //Point cloud descriptor items
      int widgetIndex = 3;
      // Point count
      QTreeWidgetItem* pCountItem = new QTreeWidgetItem();
      pCountItem->setText(0, "Point Count");
      pCountItem->setText(1, QString::number(pPointDescriptor->getPointCount()));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pCountItem);

      // X Data type
      QTreeWidgetItem* pDataTypeItem = new QTreeWidgetItem();
      string dataTypeText = StringUtilities::toDisplayString(pPointDescriptor->getSpatialDataType());
      pDataTypeItem->setText(0, "Spatial Data Type");
      pDataTypeItem->setText(1, QString::fromStdString(dataTypeText));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pDataTypeItem);

      // X Min
      QTreeWidgetItem* pMinimumItem = new QTreeWidgetItem();
      pMinimumItem->setText(0, "X Minimum");
      pMinimumItem->setText(1, QString::number(pPointDescriptor->getXMin() * pPointDescriptor->getXScale() + pPointDescriptor->getXOffset(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMinimumItem);

      // X Max
      QTreeWidgetItem* pMaximumItem = new QTreeWidgetItem();
      pMaximumItem->setText(0, "X Maximum");
      pMaximumItem->setText(1, QString::number(pPointDescriptor->getXMax() * pPointDescriptor->getXScale() + pPointDescriptor->getXOffset(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMaximumItem);

      // X Raw Min
      pMinimumItem = new QTreeWidgetItem();
      pMinimumItem->setText(0, "X Raw Minimum");
      pMinimumItem->setText(1, QString::number(pPointDescriptor->getXMin(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMinimumItem);

      // X Raw Max
      pMaximumItem = new QTreeWidgetItem();
      pMaximumItem->setText(0, "X Raw Maximum");
      pMaximumItem->setText(1, QString::number(pPointDescriptor->getXMax(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMaximumItem);

      // X Scale
      QTreeWidgetItem* pScaleItem = new QTreeWidgetItem();
      pScaleItem->setText(0, "X Scale");
      pScaleItem->setText(1, QString::number(pPointDescriptor->getXScale(), 'f', numeric_limits<double>::digits10));
      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pScaleItem, 1, CustomTreeWidget::LINE_EDIT);
      }
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pScaleItem);

      // X Offset
      QTreeWidgetItem* pOffsetItem = new QTreeWidgetItem();
      pOffsetItem->setText(0, "X Offset");
      pOffsetItem->setText(1, QString::number(pPointDescriptor->getXOffset(), 'f', numeric_limits<double>::digits10));
      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pOffsetItem, 1, CustomTreeWidget::LINE_EDIT);
      }
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pOffsetItem);

      // Y Min
      pMinimumItem = new QTreeWidgetItem();
      pMinimumItem->setText(0, "Y Minimum");
      pMinimumItem->setText(1, QString::number(pPointDescriptor->getYMin() * pPointDescriptor->getYScale() + pPointDescriptor->getYOffset(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMinimumItem);

      // Y Max
      pMaximumItem = new QTreeWidgetItem();
      pMaximumItem->setText(0, "Y Maximum");
      pMaximumItem->setText(1, QString::number(pPointDescriptor->getYMax() * pPointDescriptor->getYScale() + pPointDescriptor->getYOffset(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMaximumItem);

      // Y Min
      pMinimumItem = new QTreeWidgetItem();
      pMinimumItem->setText(0, "Y Raw Minimum");
      pMinimumItem->setText(1, QString::number(pPointDescriptor->getYMin(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMinimumItem);

      // Y Max
      pMaximumItem = new QTreeWidgetItem();
      pMaximumItem->setText(0, "Y Raw Maximum");
      pMaximumItem->setText(1, QString::number(pPointDescriptor->getYMax(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMaximumItem);

      // Y Scale
      pScaleItem = new QTreeWidgetItem();
      pScaleItem->setText(0, "Y Scale");
      pScaleItem->setText(1, QString::number(pPointDescriptor->getYScale(), 'f', numeric_limits<double>::digits10));
      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pScaleItem, 1, CustomTreeWidget::LINE_EDIT);
      }
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pScaleItem);

      // Y Offset
      pOffsetItem = new QTreeWidgetItem();
      pOffsetItem->setText(0, "Y Offset");
      pOffsetItem->setText(1, QString::number(pPointDescriptor->getYOffset(), 'f', numeric_limits<double>::digits10));
      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pOffsetItem, 1, CustomTreeWidget::LINE_EDIT);
      }
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pOffsetItem);

      // Z Min
      pMinimumItem = new QTreeWidgetItem();
      pMinimumItem->setText(0, "Z Minimum");
      pMinimumItem->setText(1, QString::number(pPointDescriptor->getZMin() * pPointDescriptor->getZScale() + pPointDescriptor->getZOffset(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMinimumItem);

      // Z Max
      pMaximumItem = new QTreeWidgetItem();
      pMaximumItem->setText(0, "Z Maximum");
      pMaximumItem->setText(1, QString::number(pPointDescriptor->getZMax() * pPointDescriptor->getZScale() + pPointDescriptor->getZOffset(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMaximumItem);

      // Z Min
      pMinimumItem = new QTreeWidgetItem();
      pMinimumItem->setText(0, "Z Raw Minimum");
      pMinimumItem->setText(1, QString::number(pPointDescriptor->getZMin(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMinimumItem);

      // Z Max
      pMaximumItem = new QTreeWidgetItem();
      pMaximumItem->setText(0, "Z Raw Maximum");
      pMaximumItem->setText(1, QString::number(pPointDescriptor->getZMax(), 'f', numeric_limits<double>::digits10));
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pMaximumItem);

      // Z Scale
      pScaleItem = new QTreeWidgetItem();
      pScaleItem->setText(0, "Z Scale");
      pScaleItem->setText(1, QString::number(pPointDescriptor->getZScale(), 'f', numeric_limits<double>::digits10));
      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pScaleItem, 1, CustomTreeWidget::LINE_EDIT);
      }
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pScaleItem);

      // Z Offset
      pOffsetItem = new QTreeWidgetItem();
      pOffsetItem->setText(0, "Z Offset");
      pOffsetItem->setText(1, QString::number(pPointDescriptor->getZOffset(), 'f', numeric_limits<double>::digits10));
      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pOffsetItem, 1, CustomTreeWidget::LINE_EDIT);
      }
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pOffsetItem);

      // Intensity Data type
      pDataTypeItem = new QTreeWidgetItem();
      pDataTypeItem->setText(0, "Intensity Data Type");
      if (pPointDescriptor->hasIntensityData())
      {
         string dataTypeText2 = StringUtilities::toDisplayString(pPointDescriptor->getIntensityDataType());
         pDataTypeItem->setText(1, QString::fromStdString(dataTypeText2));
      }
      else
      {
         pDataTypeItem->setText(1, "No Intensity Data Available");
      }
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pDataTypeItem);

      // Classification Data type
      pDataTypeItem = new QTreeWidgetItem();
      pDataTypeItem->setText(0, "Classification Data Type");
      if (pPointDescriptor->hasClassificationData())
      {
         string dataTypeText2 = StringUtilities::toDisplayString(pPointDescriptor->getClassificationDataType());
         pDataTypeItem->setText(1, QString::fromStdString(dataTypeText2));
      }
      else
      {
         pDataTypeItem->setText(1, "No Classification Data Available");
      }
      mpTreeWidget->insertTopLevelItem(widgetIndex++, pDataTypeItem);
   }

   // Signature data descriptor items
   SignatureDataDescriptor* pSignatureDescriptor = dynamic_cast<SignatureDataDescriptor*>(mpDescriptor.get());
   if (pSignatureDescriptor != NULL)
   {
      SignatureFileDescriptor* pSignatureFileDescriptor =
         dynamic_cast<SignatureFileDescriptor*>(pSignatureDescriptor->getFileDescriptor());
      if (pSignatureFileDescriptor != NULL)
      {
         VERIFYNR(pSignatureFileDescriptor->attach(SIGNAL_NAME(SignatureFileDescriptor, UnitsChanged),
            Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
      }

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

                  mpTreeWidget->setCellWidgetType(pUnitNameItem, 1, CustomTreeWidget::LINE_EDIT);
               }

               // Type
               QTreeWidgetItem* pUnitTypeItem = new QTreeWidgetItem(pComponentItem);
               if (pUnitTypeItem != NULL)
               {
                  UnitType unitType = pUnits->getUnitType();
                  string unitTypeText = StringUtilities::toDisplayString(unitType);

                  pUnitTypeItem->setText(0, "Unit Type");
                  pUnitTypeItem->setText(1, QString::fromStdString(unitTypeText));

                  mpTreeWidget->setCellWidgetType(pUnitTypeItem, 1, CustomTreeWidget::COMBO_BOX);
                  mpTreeWidget->setComboBox(pUnitTypeItem, 1, mpUnitTypeCombo);
               }

               // Scale
               QTreeWidgetItem* pScaleItem = new QTreeWidgetItem(pComponentItem);
               if (pScaleItem != NULL)
               {
                  double dScale = pUnits->getScaleFromStandard();

                  pScaleItem->setText(0, "Scale");
                  pScaleItem->setText(1, QString::number(dScale));

                  mpTreeWidget->setCellWidgetType(pScaleItem, 1, CustomTreeWidget::LINE_EDIT);
               }

               // Range minimum
               QTreeWidgetItem* pMinimumItem = new QTreeWidgetItem(pComponentItem);
               if (pMinimumItem != NULL)
               {
                  double dMinimum = pUnits->getRangeMin();

                  pMinimumItem->setText(0, "Range Minimum");
                  pMinimumItem->setText(1, QString::number(dMinimum));

                  mpTreeWidget->setCellWidgetType(pMinimumItem, 1, CustomTreeWidget::LINE_EDIT);
               }

               // Range maximum
               QTreeWidgetItem* pMaximumItem = new QTreeWidgetItem(pComponentItem);
               if (pMaximumItem != NULL)
               {
                  double dMaximum = pUnits->getRangeMax();

                  pMaximumItem->setText(0, "Range Maximum");
                  pMaximumItem->setText(1, QString::number(dMaximum));

                  mpTreeWidget->setCellWidgetType(pMaximumItem, 1, CustomTreeWidget::LINE_EDIT);
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

   // Raster data descriptor items
   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(mpDescriptor.get());
   if (pRasterDescriptor == NULL)
   {
      VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
         SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
      return;
   }

   RasterFileDescriptor* pRasterFileDescriptor =
      dynamic_cast<RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
   if (pRasterFileDescriptor != NULL)
   {
      VERIFYNR(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, RowsChanged),
         Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
      VERIFYNR(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, ColumnsChanged),
         Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
      VERIFYNR(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, BandsChanged),
         Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
      VERIFYNR(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, BitsPerElementChanged),
         Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
      VERIFYNR(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, InterleaveFormatChanged),
         Slot(this, &DataDescriptorWidget::fileDescriptorModified)));
      VERIFYNR(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, PixelSizeChanged),
         Slot(this, &DataDescriptorWidget::fileDescriptorModified)));

      Units* pFileUnits = pRasterFileDescriptor->getUnits();
      if (pFileUnits != NULL)
      {
         VERIFYNR(pFileUnits->attach(SIGNAL_NAME(Units, Renamed),
            Slot(this, &DataDescriptorWidget::fileDescriptorUnitsModified)));
         VERIFYNR(pFileUnits->attach(SIGNAL_NAME(Units, TypeChanged),
            Slot(this, &DataDescriptorWidget::fileDescriptorUnitsModified)));
         VERIFYNR(pFileUnits->attach(SIGNAL_NAME(Units, ScaleChanged),
            Slot(this, &DataDescriptorWidget::fileDescriptorUnitsModified)));
         VERIFYNR(pFileUnits->attach(SIGNAL_NAME(Units, RangeChanged),
            Slot(this, &DataDescriptorWidget::fileDescriptorUnitsModified)));
      }
   }

   // Display As button
   const DynamicObject* pColorComposites = RasterLayer::getSettingColorComposites();
   if (pColorComposites != NULL)
   {
      QMenu* pMenu = mpSetDisplayButton->menu();
      VERIFYNR(pMenu != NULL);
      pMenu->clear();

      vector<string> names;
      pColorComposites->getAttributeNames(names);
      if (names.empty() == false)
      {
         for (vector<string>::const_iterator iter = names.begin(); iter != names.end(); ++iter)
         {
            pMenu->addAction(QString::fromStdString(*iter));
         }

         mpSetDisplayButton->setEnabled(true);
      }
   }

   // Rows
   QTreeWidgetItem* pRowsItem = new QTreeWidgetItem();
   if (pRowsItem != NULL)
   {
      pRowsItem->setText(0, "Rows");
      pRowsItem->setText(1, QString::number(pRasterDescriptor->getRowCount()));
      mpTreeWidget->insertTopLevelItem(3, pRowsItem);
   }

   // Columns
   QTreeWidgetItem* pColumnsItem = new QTreeWidgetItem();
   if (pColumnsItem != NULL)
   {
      pColumnsItem->setText(0, "Columns");
      pColumnsItem->setText(1, QString::number(pRasterDescriptor->getColumnCount()));
      mpTreeWidget->insertTopLevelItem(4, pColumnsItem);
   }

   // Data type
   QTreeWidgetItem* pDataTypeItem = new QTreeWidgetItem();
   if (pDataTypeItem != NULL)
   {
      EncodingType dataType = pRasterDescriptor->getDataType();
      string dataTypeText = StringUtilities::toDisplayString(dataType);

      pDataTypeItem->setText(0, "Data Type");
      pDataTypeItem->setText(1, QString::fromStdString(dataTypeText));
      mpTreeWidget->insertTopLevelItem(5, pDataTypeItem);

      if (mEditAll)
      {
         mpDataTypeCombo->clear();

         const vector<EncodingType>& validDataTypes = pRasterDescriptor->getValidDataTypes();
         for (vector<EncodingType>::const_iterator iter = validDataTypes.begin(); iter != validDataTypes.end(); ++iter)
         {
            mpDataTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(*iter)));
         }

         mpTreeWidget->setCellWidgetType(pDataTypeItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pDataTypeItem, 1, mpDataTypeCombo);
      }
   }

   // Bad values
   QTreeWidgetItem* pBadValuesItem = new QTreeWidgetItem(mpTreeWidget);
   if (pBadValuesItem != NULL)
   {
      const BadValues* pBadValues = pRasterDescriptor->getBadValues();
      string badValuesText;
      if (pBadValues != NULL)
      {
         badValuesText = pBadValues->getBadValuesString();
      }
      else
      {
         badValuesText = "Bad values vary by band";
      }

      pBadValuesItem->setText(0, "Bad Values");
      pBadValuesItem->setText(1, QString::fromStdString(badValuesText));

      mpTreeWidget->setCellWidgetType(pBadValuesItem, 1, CustomTreeWidget::CUSTOM_WIDGET);
      mpTreeWidget->setCustomEditWidget(pBadValuesItem, 1, mpBadValuesEdit);
   }

   // Pixel size
   QTreeWidgetItem* pXPixelSizeItem = new QTreeWidgetItem(mpTreeWidget);
   if (pXPixelSizeItem != NULL)
   {
      double pixelSize = pRasterDescriptor->getXPixelSize();

      pXPixelSizeItem->setText(0, "X Pixel Size");
      pXPixelSizeItem->setText(1, QString::number(pixelSize));

      if (mEditAll)
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

      if (mEditAll)
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
         QTreeWidgetItem* pUnitNameItem = new QTreeWidgetItem(pUnitsItem);
         if (pUnitNameItem != NULL)
         {
            string unitsName = pUnits->getUnitName();

            pUnitNameItem->setText(0, "Unit Name");
            pUnitNameItem->setText(1, QString::fromStdString(unitsName));

            mpTreeWidget->setCellWidgetType(pUnitNameItem, 1, CustomTreeWidget::LINE_EDIT);
         }

         // Type
         QTreeWidgetItem* pUnitTypeItem = new QTreeWidgetItem(pUnitsItem);
         if (pUnitTypeItem != NULL)
         {
            UnitType unitType = pUnits->getUnitType();
            string unitTypeText = StringUtilities::toDisplayString(unitType);

            pUnitTypeItem->setText(0, "Unit Type");
            pUnitTypeItem->setText(1, QString::fromStdString(unitTypeText));

            mpTreeWidget->setCellWidgetType(pUnitTypeItem, 1, CustomTreeWidget::COMBO_BOX);
            mpTreeWidget->setComboBox(pUnitTypeItem, 1, mpUnitTypeCombo);
         }

         // Scale
         QTreeWidgetItem* pScaleItem = new QTreeWidgetItem(pUnitsItem);
         if (pScaleItem != NULL)
         {
            double dScale = pUnits->getScaleFromStandard();

            pScaleItem->setText(0, "Scale");
            pScaleItem->setText(1, QString::number(dScale));

            mpTreeWidget->setCellWidgetType(pScaleItem, 1, CustomTreeWidget::LINE_EDIT);
         }

         // Range minimum
         QTreeWidgetItem* pMinimumItem = new QTreeWidgetItem(pUnitsItem);
         if (pMinimumItem != NULL)
         {
            double dMinimum = pUnits->getRangeMin();

            pMinimumItem->setText(0, "Range Minimum");
            pMinimumItem->setText(1, QString::number(dMinimum));

            mpTreeWidget->setCellWidgetType(pMinimumItem, 1, CustomTreeWidget::LINE_EDIT);
         }

         // Range maximum
         QTreeWidgetItem* pMaximumItem = new QTreeWidgetItem(pUnitsItem);
         if (pMaximumItem != NULL)
         {
            double dMaximum = pUnits->getRangeMax();

            pMaximumItem->setText(0, "Range Maximum");
            pMaximumItem->setText(1, QString::number(dMaximum));

            mpTreeWidget->setCellWidgetType(pMaximumItem, 1, CustomTreeWidget::LINE_EDIT);
         }
      }
   }

   // Bands
   QTreeWidgetItem* pBandsItem = new QTreeWidgetItem();
   if (pBandsItem != NULL)
   {
      pBandsItem->setText(0, "Bands");
      pBandsItem->setText(1, QString::number(pRasterDescriptor->getBandCount()));
      mpTreeWidget->insertTopLevelItem(5, pBandsItem);
   }

   // Interleave format
   QTreeWidgetItem* pInterleaveItem = new QTreeWidgetItem();
   if (pInterleaveItem != NULL)
   {
      InterleaveFormatType interleave = pRasterDescriptor->getInterleaveFormat();
      string interleaveText = StringUtilities::toDisplayString(interleave);

      pInterleaveItem->setText(0, "Interleave Format");
      pInterleaveItem->setText(1, QString::fromStdString(interleaveText));

      mpTreeWidget->insertTopLevelItem(8, pInterleaveItem);
      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pInterleaveItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pInterleaveItem, 1, mpInterleaveCombo);
      }
   }

   // Gray band
   QTreeWidgetItem* pGrayBandItem = new QTreeWidgetItem(mpTreeWidget);
   if (pGrayBandItem != NULL)
   {
      QString strBand;

      DimensionDescriptor bandDim = pRasterDescriptor->getDisplayBand(GRAY);
      if (bandDim.isValid())
      {
         strBand = QString::number(bandDim.getOriginalNumber() + 1);
      }

      pGrayBandItem->setText(0, "Gray Band");
      pGrayBandItem->setText(1, strBand);

      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pGrayBandItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Red band
   QTreeWidgetItem* pRedBandItem = new QTreeWidgetItem(mpTreeWidget);
   if (pRedBandItem != NULL)
   {
      QString strBand;

      DimensionDescriptor bandDim = pRasterDescriptor->getDisplayBand(RED);
      if (bandDim.isValid())
      {
         strBand = QString::number(bandDim.getOriginalNumber() + 1);
      }

      pRedBandItem->setText(0, "Red Band");
      pRedBandItem->setText(1, strBand);

      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pRedBandItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Green band
   QTreeWidgetItem* pGreenBandItem = new QTreeWidgetItem(mpTreeWidget);
   if (pGreenBandItem != NULL)
   {
      QString strBand;

      DimensionDescriptor bandDim = pRasterDescriptor->getDisplayBand(GREEN);
      if (bandDim.isValid())
      {
         strBand = QString::number(bandDim.getOriginalNumber() + 1);
      }

      pGreenBandItem->setText(0, "Green Band");
      pGreenBandItem->setText(1, strBand);

      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pGreenBandItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Blue band
   QTreeWidgetItem* pBlueBandItem = new QTreeWidgetItem(mpTreeWidget);
   if (pBlueBandItem != NULL)
   {
      QString strBand;

      DimensionDescriptor bandDim = pRasterDescriptor->getDisplayBand(BLUE);
      if (bandDim.isValid())
      {
         strBand = QString::number(bandDim.getOriginalNumber() + 1);
      }

      pBlueBandItem->setText(0, "Blue Band");
      pBlueBandItem->setText(1, strBand);

      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pBlueBandItem, 1, CustomTreeWidget::LINE_EDIT);
      }
   }

   // Display mode
   QTreeWidgetItem* pDisplayModeItem = new QTreeWidgetItem(mpTreeWidget);
   if (pDisplayModeItem != NULL)
   {
      DisplayMode displayMode = pRasterDescriptor->getDisplayMode();
      string displayModeText = StringUtilities::toDisplayString(displayMode);

      pDisplayModeItem->setText(0, "Display Mode");
      pDisplayModeItem->setText(1, QString::fromStdString(displayModeText));

      if (mEditAll)
      {
         mpTreeWidget->setCellWidgetType(pDisplayModeItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pDisplayModeItem, 1, mpDisplayModeCombo);
      }
   }

   VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
}

void DataDescriptorWidget::setValidProcessingLocations(const vector<ProcessingLocation>& locations)
{
   mpProcessingLocationCombo->clear();
   for (vector<ProcessingLocation>::const_iterator iter = locations.begin(); iter != locations.end(); ++iter)
   {
      string locationText = StringUtilities::toDisplayString(*iter);
      if (locationText.empty() == false)
      {
         mpProcessingLocationCombo->addItem(QString::fromStdString(locationText));
      }
   }
}

void DataDescriptorWidget::showEvent(QShowEvent* pEvent)
{
   QWidget::showEvent(pEvent);

   if (mpDescriptor.get() != NULL)
   {
      VERIFYNR(mpDescriptor->detach(SIGNAL_NAME(Subject, Modified),
         Slot(this, &DataDescriptorWidget::dataDescriptorModified)));
      initialize();
   }
}

void DataDescriptorWidget::hideEvent(QHideEvent* pEvent)
{
   QWidget::hideEvent(pEvent);

   if (mpDescriptor.get() != NULL)
   {
      VERIFYNR(mpDescriptor->attach(SIGNAL_NAME(Subject, Modified),
         Slot(this, &DataDescriptorWidget::dataDescriptorModified)));
   }
}

void DataDescriptorWidget::dataDescriptorModified(Subject& subject, const string& signal, const boost::any& value)
{
   mNeedsInitialization = true;
}

void DataDescriptorWidget::initialize()
{
   if ((mpDescriptor.get() == NULL) || (mNeedsInitialization == false) || (isVisible() == false))
   {
      return;
   }

   mNeedsInitialization = false;

   VERIFYNR(disconnect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));

   // Data descriptor items
   QTreeWidgetItem* pItem = getDescriptorItem("Name");
   if (pItem != NULL)
   {
      const string& name = mpDescriptor->getName();
      pItem->setText(1, QString::fromStdString(name));
   }

   pItem = getDescriptorItem("Parent Data Set");
   if (pItem != NULL)
   {
      QString parentName;

      DataElement* pParent = mpDescriptor->getParent();
      if (pParent != NULL)
      {
         parentName = QString::fromStdString(pParent->getName());
      }

      pItem->setText(1, parentName);
   }

   pItem = getDescriptorItem("Processing Location");
   if (pItem != NULL)
   {
      ProcessingLocation processingLocation = mpDescriptor->getProcessingLocation();
      string processingLocationText = StringUtilities::toDisplayString(processingLocation);
      pItem->setText(1, QString::fromStdString(processingLocationText));

      if (mpProcessingLocationCombo != NULL)
      {
         int index = mpProcessingLocationCombo->findText(QString::fromStdString(processingLocationText));
         mpProcessingLocationCombo->setCurrentIndex(index);
      }
   }

   // Signature data descriptor items
   SignatureDataDescriptor* pSignatureDescriptor = dynamic_cast<SignatureDataDescriptor*>(mpDescriptor.get());
   if (pSignatureDescriptor != NULL)
   {
      set<string> componentNames = pSignatureDescriptor->getUnitNames();
      for (set<string>::const_iterator iter = componentNames.begin(); iter != componentNames.end(); ++iter)
      {
         const Units* pUnits = pSignatureDescriptor->getUnits(*iter);
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

   // Raster data descriptor items
   const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpDescriptor.get());
   if (pRasterDescriptor == NULL)
   {
      VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
         SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
      return;
   }

   pItem = getDescriptorItem("Rows");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterDescriptor->getRowCount()));
   }

   pItem = getDescriptorItem("Columns");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterDescriptor->getColumnCount()));
   }

   pItem = getDescriptorItem("Bands");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterDescriptor->getBandCount()));
   }

   pItem = getDescriptorItem("Data Type");
   if (pItem != NULL)
   {
      EncodingType dataType = pRasterDescriptor->getDataType();
      string dataTypeText = StringUtilities::toDisplayString(dataType);
      pItem->setText(1, QString::fromStdString(dataTypeText));

      if (mpDataTypeCombo != NULL)
      {
         mpDataTypeCombo->clear();

         const vector<EncodingType>& validDataTypes = pRasterDescriptor->getValidDataTypes();
         for (vector<EncodingType>::const_iterator iter = validDataTypes.begin(); iter != validDataTypes.end(); ++iter)
         {
            EncodingType currentDataType = *iter;

            QString currentDataTypeText = QString::fromStdString(StringUtilities::toDisplayString(currentDataType));
            if (currentDataTypeText.isEmpty() == false)
            {
               mpDataTypeCombo->addItem(currentDataTypeText);
            }
         }

         int index = mpDataTypeCombo->findText(QString::fromStdString(dataTypeText));
         mpDataTypeCombo->setCurrentIndex(index);
      }
   }

   // Bad values
   pItem = getDescriptorItem("Bad Values");
   if (pItem != NULL)
   {
      const BadValues* pBadValues = pRasterDescriptor->getBadValues();
      if (pBadValues != NULL)
      {
         pItem->setText(1, QString::fromStdString(pBadValues->getBadValuesString()));
      }
      else
      {
         pItem->setText(1, "Bad values vary by band");
      }
   }

   pItem = getDescriptorItem("X Pixel Size");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterDescriptor->getXPixelSize()));
   }

   pItem = getDescriptorItem("Y Pixel Size");
   if (pItem != NULL)
   {
      pItem->setText(1, QString::number(pRasterDescriptor->getYPixelSize()));
   }

   pItem = getDescriptorItem("Interleave Format");
   if (pItem != NULL)
   {
      InterleaveFormatType interleave = pRasterDescriptor->getInterleaveFormat();
      string interleaveText = StringUtilities::toDisplayString(interleave);
      pItem->setText(1, QString::fromStdString(interleaveText));

      if (mpInterleaveCombo != NULL)
      {
         int index = mpInterleaveCombo->findText(QString::fromStdString(interleaveText));
         mpInterleaveCombo->setCurrentIndex(index);
      }
   }

   pItem = getDescriptorItem("Gray Band");
   if (pItem != NULL)
   {
      QString bandText;

      DimensionDescriptor band = pRasterDescriptor->getDisplayBand(GRAY);
      if (band.isValid() == true)
      {
         bandText = QString::number(band.getOriginalNumber() + 1);
      }

      pItem->setText(1, bandText);
   }

   pItem = getDescriptorItem("Red Band");
   if (pItem != NULL)
   {
      QString bandText;

      DimensionDescriptor band = pRasterDescriptor->getDisplayBand(RED);
      if (band.isValid() == true)
      {
         bandText = QString::number(band.getOriginalNumber() + 1);
      }

      pItem->setText(1, bandText);
   }

   pItem = getDescriptorItem("Green Band");
   if (pItem != NULL)
   {
      QString bandText;

      DimensionDescriptor band = pRasterDescriptor->getDisplayBand(GREEN);
      if (band.isValid() == true)
      {
         bandText = QString::number(band.getOriginalNumber() + 1);
      }

      pItem->setText(1, bandText);
   }

   pItem = getDescriptorItem("Blue Band");
   if (pItem != NULL)
   {
      QString bandText;

      DimensionDescriptor band = pRasterDescriptor->getDisplayBand(BLUE);
      if (band.isValid() == true)
      {
         bandText = QString::number(band.getOriginalNumber() + 1);
      }

      pItem->setText(1, bandText);
   }

   pItem = getDescriptorItem("Display Mode");
   if (pItem != NULL)
   {
      DisplayMode displayMode = pRasterDescriptor->getDisplayMode();
      string displayModeText = StringUtilities::toDisplayString(displayMode);
      pItem->setText(1, QString::fromStdString(displayModeText));

      if (mpDisplayModeCombo != NULL)
      {
         int index = mpDisplayModeCombo->findText(QString::fromStdString(displayModeText));
         mpDisplayModeCombo->setCurrentIndex(index);
      }
   }

   // Units items
   const Units* pUnits = pRasterDescriptor->getUnits();
   if (pUnits != NULL)
   {
      pItem = getDescriptorItem("Unit Name");
      if (pItem != NULL)
      {
         pItem->setText(1, QString::fromStdString(pUnits->getUnitName()));
      }

      pItem = getDescriptorItem("Unit Type");
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

void DataDescriptorWidget::fileDescriptorModified(Subject& subject, const string& signal, const boost::any& value)
{
   SignatureDataDescriptor* pSignatureDescriptor = dynamic_cast<SignatureDataDescriptor*>(mpDescriptor.get());
   if (pSignatureDescriptor != NULL)
   {
      SignatureFileDescriptor* pSignatureFileDescriptor =
         dynamic_cast<SignatureFileDescriptor*>(pSignatureDescriptor->getFileDescriptor());
      if ((pSignatureFileDescriptor != NULL) &&
         (dynamic_cast<SignatureFileDescriptor*>(&subject) == pSignatureFileDescriptor))
      {
         if (signal == SIGNAL_NAME(SignatureFileDescriptor, UnitsChanged))
         {
            pair<string, const Units*> units = boost::any_cast<pair<string, const Units*> >(value);
            pSignatureDescriptor->setUnits(units.first, units.second);
         }
      }

      return;
   }

   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(mpDescriptor.get());
   if (pRasterDescriptor == NULL)
   {
      return;
   }

   RasterFileDescriptor* pRasterFileDescriptor =
      dynamic_cast<RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
   if ((pRasterFileDescriptor == NULL) || (dynamic_cast<RasterFileDescriptor*>(&subject) != pRasterFileDescriptor))
   {
      return;
   }

   if (signal == SIGNAL_NAME(RasterFileDescriptor, RowsChanged))
   {
      pRasterDescriptor->setRows(pRasterFileDescriptor->getRows());
   }
   else if (signal == SIGNAL_NAME(RasterFileDescriptor, ColumnsChanged))
   {
      pRasterDescriptor->setColumns(pRasterFileDescriptor->getColumns());
   }
   else if (signal == SIGNAL_NAME(RasterFileDescriptor, BandsChanged))
   {
      pRasterDescriptor->setBands(pRasterFileDescriptor->getBands());
   }
   else if (signal == SIGNAL_NAME(RasterFileDescriptor, BitsPerElementChanged))
   {
      // There is no way to determine which data type to use (e.g. signed/unsigned),
      // so pick a reasonable default based on the number of bytes
      EncodingType dataType;

      double bytesPerElement = static_cast<double>(pRasterFileDescriptor->getBitsPerElement()) / 8.0;
      if (bytesPerElement <= 1.0)
      {
         dataType = INT1UBYTE;
      }
      else if (bytesPerElement <= 2.0)
      {
         dataType = INT2UBYTES;
      }
      else if (bytesPerElement <= 4.0)
      {
         dataType = INT4UBYTES;
      }
      else
      {
         dataType = FLT8BYTES;
      }

      pRasterDescriptor->setDataType(dataType);
   }
   else if (signal == SIGNAL_NAME(RasterFileDescriptor, InterleaveFormatChanged))
   {
      pRasterDescriptor->setInterleaveFormat(pRasterFileDescriptor->getInterleaveFormat());
   }
   else if (signal == SIGNAL_NAME(RasterFileDescriptor, PixelSizeChanged))
   {
      pRasterDescriptor->setXPixelSize(pRasterFileDescriptor->getXPixelSize());
      pRasterDescriptor->setYPixelSize(pRasterFileDescriptor->getYPixelSize());
   }
}

void DataDescriptorWidget::fileDescriptorUnitsModified(Subject& subject, const string& signal, const boost::any& value)
{
   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(mpDescriptor.get());
   if (pRasterDescriptor == NULL)
   {
      return;
   }

   Units* pUnits = pRasterDescriptor->getUnits();
   if (pUnits == NULL)
   {
      return;
   }

   RasterFileDescriptor* pRasterFileDescriptor =
      dynamic_cast<RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
   if (pRasterFileDescriptor == NULL)
   {
      return;
   }

   Units* pFileUnits = pRasterFileDescriptor->getUnits();
   if ((pFileUnits == NULL) || (dynamic_cast<Units*>(&subject) != pFileUnits))
   {
      return;
   }

   if (signal == SIGNAL_NAME(Units, Renamed))
   {
      pUnits->setUnitName(pFileUnits->getUnitName());
   }
   else if (signal == SIGNAL_NAME(Units, TypeChanged))
   {
      pUnits->setUnitType(pFileUnits->getUnitType());
   }
   else if (signal == SIGNAL_NAME(Units, ScaleChanged))
   {
      pUnits->setScaleFromStandard(pFileUnits->getScaleFromStandard());
   }
   else if (signal == SIGNAL_NAME(Units, RangeChanged))
   {
      pUnits->setRangeMin(pFileUnits->getRangeMin());
      pUnits->setRangeMax(pFileUnits->getRangeMax());
   }
}

QTreeWidgetItem* DataDescriptorWidget::getDescriptorItem(const QString& strName, QTreeWidgetItem* pParentItem) const
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

void DataDescriptorWidget::descriptorItemChanged(QTreeWidgetItem* pItem, int iColumn)
{
   if ((pItem == NULL) || (iColumn != 1) || (mpDescriptor.get() == NULL))
   {
      return;
   }

   QString itemName = pItem->text(0);
   QString itemValue = pItem->text(iColumn);

   // Data descriptor items
   if (itemName == "Processing Location")
   {
      if (itemValue.isEmpty() == false)
      {
         bool error = true;
         ProcessingLocation location = StringUtilities::fromDisplayString<ProcessingLocation>(itemValue.toStdString(),
            &error);
         if (error == false)
         {
            mpDescriptor->setProcessingLocation(location);
         }
      }
   }

   // Signature data descriptor items
   SignatureDataDescriptor* pSignatureDescriptor = dynamic_cast<SignatureDataDescriptor*>(mpDescriptor.get());
   if (pSignatureDescriptor != NULL)
   {
      set<string> componentNames = pSignatureDescriptor->getUnitNames();
      for (set<string>::const_iterator iter = componentNames.begin(); iter != componentNames.end(); ++iter)
      {
         FactoryResource<Units> pUnits;

         const Units* pCurrentUnits = pSignatureDescriptor->getUnits(*iter);
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

         pSignatureDescriptor->setUnits(*iter, pUnits.get());
      }

      return;
   }

   // Raster data descriptor items
   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(mpDescriptor.get());
   if (pRasterDescriptor == NULL)
   {
      return;
   }

   Units* pUnits = pRasterDescriptor->getUnits();
   if ((pUnits != NULL) && (itemValue.isEmpty() == false))
   {
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
   }

   if (itemName == "Bad Values")
   {
      FactoryResource<BadValues> pBadValues;
      if (pBadValues->setBadValues(itemValue.toStdString()) == false)
      {
         QString errorMsg = "Unable to set bad values: " + QString::fromStdString(pBadValues->getLastErrorMsg());
         QMessageBox::warning(this, APP_NAME, errorMsg);
         QString badValuesText = "Bad values vary by band";
         const BadValues* pCurrentBadValues = pRasterDescriptor->getBadValues();
         if (pCurrentBadValues != NULL)
         {
            badValuesText = QString::fromStdString(pCurrentBadValues->getBadValuesString());
         }
         VERIFYNR(disconnect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
            SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
         pItem->setText(iColumn, badValuesText);
         VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
            SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
         return;
      }

      pRasterDescriptor->setBadValues(pBadValues.get());
   }

   if (mEditAll == false)  // Nothing else could have been changed, so return
   {
      return;
   }

   RasterFileDescriptor* pRasterFileDescriptor =
      dynamic_cast<RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());

   if ((itemName == "X Pixel Size") || (itemName == "Y Pixel Size"))
   {
      if (itemValue.isEmpty() == false)
      {
         double pixelSize = itemValue.toDouble();
         if (pixelSize <= 0.0)
         {
            QMessageBox::warning(this, APP_NAME, "The pixel size cannot be negative or zero.  "
               "Please enter a positive number.");

            if (itemName == "X Pixel Size")
            {
               pixelSize = pRasterDescriptor->getXPixelSize();
            }
            else if (itemName == "Y Pixel Size")
            {
               pixelSize = pRasterDescriptor->getYPixelSize();
            }

            VERIFYNR(disconnect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
               SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));

            pItem->setText(iColumn, QString::number(pixelSize));

            VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
               SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
            return;
         }

         if (itemName == "X Pixel Size")
         {
            pRasterDescriptor->setXPixelSize(pixelSize);
         }
         else if (itemName == "Y Pixel Size")
         {
            pRasterDescriptor->setYPixelSize(pixelSize);
         }
      }
   }
   else if (itemName == "Data Type")
   {
      if (itemValue.isEmpty() == false)
      {
         bool error = true;
         EncodingType dataType = StringUtilities::fromDisplayString<EncodingType>(itemValue.toStdString(), &error);
         if (error == false)
         {
            pRasterDescriptor->setDataType(dataType);
         }
      }
   }
   else if (itemName == "Interleave Format")
   {
      if (itemValue.isEmpty() == false)
      {
         bool error = true;
         InterleaveFormatType interleave =
            StringUtilities::fromDisplayString<InterleaveFormatType>(itemValue.toStdString(), &error);
         if (error == false)
         {
            pRasterDescriptor->setInterleaveFormat(interleave);
         }
      }
   }
   else if (itemName == "Gray Band")
   {
      DimensionDescriptor grayBand;
      if (itemValue.isEmpty() == false)
      {
         grayBand = pRasterDescriptor->getOriginalBand(itemValue.toUInt() - 1);
         if (grayBand.isValid() == false)
         {
            if (pRasterFileDescriptor != NULL)
            {
               grayBand = pRasterFileDescriptor->getOriginalBand(itemValue.toUInt() - 1);
            }

            if (grayBand.isValid() == false)
            {
               if (QMessageBox::warning(this, APP_NAME, "The gray display band does not exist!  "
                  "Do you want to continue, where the band will be reset to the first available band?",
                  QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
               {
                  DimensionDescriptor band = pRasterDescriptor->getDisplayBand(GRAY);
                  pItem->setText(iColumn, QString::number(band.getOriginalNumber() + 1));
               }
               else
               {
                  pItem->setText(iColumn, QString());
               }

               return;
            }
         }
      }

      pRasterDescriptor->setDisplayBand(GRAY, grayBand);
   }
   else if (itemName == "Red Band")
   {
      DimensionDescriptor redBand;
      if (itemValue.isEmpty() == false)
      {
         redBand = pRasterDescriptor->getOriginalBand(itemValue.toUInt() - 1);
         if (redBand.isValid() == false)
         {
            if (pRasterFileDescriptor != NULL)
            {
               redBand = pRasterFileDescriptor->getOriginalBand(itemValue.toUInt() - 1);
            }

            if (redBand.isValid() == false)
            {
               if (QMessageBox::warning(this, APP_NAME, "The red display band does not exist!  "
                  "Do you want to continue, where the band will be reset to the first available band?",
                  QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
               {
                  DimensionDescriptor band = pRasterDescriptor->getDisplayBand(RED);
                  pItem->setText(iColumn, QString::number(band.getOriginalNumber() + 1));
               }
               else
               {
                  pItem->setText(iColumn, QString());
               }

               return;
            }
         }
      }

      pRasterDescriptor->setDisplayBand(RED, redBand);
   }
   else if (itemName == "Green Band")
   {
      DimensionDescriptor greenBand;
      if (itemValue.isEmpty() == false)
      {
         greenBand = pRasterDescriptor->getOriginalBand(itemValue.toUInt() - 1);
         if (greenBand.isValid() == false)
         {
            if (pRasterFileDescriptor != NULL)
            {
               greenBand = pRasterFileDescriptor->getOriginalBand(itemValue.toUInt() - 1);
            }

            if (greenBand.isValid() == false)
            {
               if (QMessageBox::warning(this, APP_NAME, "The green display band does not exist!  "
                  "Do you want to continue, where the band will be reset to the first available band?",
                  QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
               {
                  DimensionDescriptor band = pRasterDescriptor->getDisplayBand(GREEN);
                  pItem->setText(iColumn, QString::number(band.getOriginalNumber() + 1));
               }
               else
               {
                  pItem->setText(iColumn, QString());
               }

               return;
            }
         }
      }

      pRasterDescriptor->setDisplayBand(GREEN, greenBand);
   }
   else if (itemName == "Blue Band")
   {
      DimensionDescriptor blueBand;
      if (itemValue.isEmpty() == false)
      {
         blueBand = pRasterDescriptor->getOriginalBand(itemValue.toUInt() - 1);
         if (blueBand.isValid() == false)
         {
            if (pRasterFileDescriptor != NULL)
            {
               blueBand = pRasterFileDescriptor->getOriginalBand(itemValue.toUInt() - 1);
            }

            if (blueBand.isValid() == false)
            {
               if (QMessageBox::warning(this, APP_NAME, "The blue display band does not exist!  "
                  "Do you want to continue, where the band will be reset to the first available band?",
                  QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
               {
                  DimensionDescriptor band = pRasterDescriptor->getDisplayBand(BLUE);
                  pItem->setText(iColumn, QString::number(band.getOriginalNumber() + 1));
               }
               else
               {
                  pItem->setText(iColumn, QString());
               }

               return;
            }
         }
      }

      pRasterDescriptor->setDisplayBand(BLUE, blueBand);
   }
   else if (itemName == "Display Mode")
   {
      if (itemValue.isEmpty() == false)
      {
         bool error = true;
         DisplayMode displayMode = StringUtilities::fromDisplayString<DisplayMode>(itemValue.toStdString(), &error);
         if (error == false)
         {
            pRasterDescriptor->setDisplayMode(displayMode);
         }
      }
   }
}

void DataDescriptorWidget::setDisplayBands(QAction* pAction)
{
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpDescriptor.get());
   VERIFYNRV(pDescriptor != NULL && pAction != NULL);

   const string name = pAction->text().toStdString();
   DimensionDescriptor redBand;
   DimensionDescriptor greenBand;
   DimensionDescriptor blueBand;

   // Ignore the return value of RasterUtilities::findColorCompositeIndices; each band will be individually checked
   RasterUtilities::findColorCompositeDimensionDescriptors(pDescriptor, name, redBand, greenBand, blueBand);
   if (redBand.isOriginalNumberValid() || greenBand.isOriginalNumberValid() || blueBand.isOriginalNumberValid())
   {
      QTreeWidgetItem* pItem = getDescriptorItem("Red Band");
      if (pItem != NULL)
      {
         pItem->setText(1, redBand.isOriginalNumberValid() ?
            QString::number(redBand.getOriginalNumber() + 1) : QString());
      }

      pItem = getDescriptorItem("Green Band");
      if (pItem != NULL)
      {
         pItem->setText(1, greenBand.isOriginalNumberValid() ?
            QString::number(greenBand.getOriginalNumber() + 1) : QString());
      }

      pItem = getDescriptorItem("Blue Band");
      if (pItem != NULL)
      {
         pItem->setText(1, blueBand.isOriginalNumberValid() ?
            QString::number(blueBand.getOriginalNumber() + 1) : QString());
      }

      pItem = getDescriptorItem("Display Mode");
      if (pItem != NULL)
      {
         pItem->setText(1, QString::fromStdString(StringUtilities::toDisplayString(RGB_MODE)));
      }
   }
   else
   {
      Service<DesktopServices>()->showSuppressibleMsgDlg("Error",
         "Unable to display " + name + ": required wavelengths do not exist for all bands. "
         "Broaden the wavelength region or specify band numbers in the Raster Layers section of the Options dialog.",
         MESSAGE_ERROR, PropertiesRasterLayer::getDisplayAsWarningDialogId());
   }
}

BadValuesEdit::BadValuesEdit(QWidget* pParent) :
   CustomEditWidget(pParent)
{
   // create line edit validator expression
   QRegExp badValuesExp;
   badValuesExp.setPattern("^([<>]?[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?[,]?[<>]?[\\s]?)+$");

   QHBoxLayout* pLayout = new QHBoxLayout(this);
   mpBadValuesStr = new QLineEdit(this);
   mpBadValuesStr->setValidator(new QRegExpValidator(badValuesExp, this));
   mpBadValuesStr->installEventFilter(this);
   mpBadValuesStr->setFrame(false);
   mpEditButton = new QPushButton("Edit...", this);
   mpEditButton->installEventFilter(this);
   pLayout->addWidget(mpBadValuesStr, 10);
   pLayout->addWidget(mpEditButton);
   pLayout->setMargin(0);
   pLayout->setSpacing(0);

   setFocusPolicy(Qt::StrongFocus);
   setFocusProxy(mpBadValuesStr);

   VERIFYNR(connect(mpEditButton, SIGNAL(clicked()), this, SLOT(displayBadValuesDialog())));
}

BadValuesEdit::~BadValuesEdit()
{}

QString BadValuesEdit::text() const
{
   VERIFYRV(mpBadValuesStr != NULL, QString());
   return mpBadValuesStr->text();
}

void BadValuesEdit::setText(const QString& text)
{
   VERIFYNRV(mpBadValuesStr != NULL);
   mpBadValuesStr->setText(text);
}

void BadValuesEdit::selectAll()
{
   VERIFYNRV(mpBadValuesStr != NULL);
   mpBadValuesStr->selectAll();
}

void BadValuesEdit::displayBadValuesDialog()
{
   // Remove the event filter on the edit button to prevent the focus
   // out event from being sent when the edit dialog is invoked
   mpEditButton->removeEventFilter(this);

   // window() used as parent so the expand/collapse indicators for labeled sections are drawn properly
   BadValuesDlg dlg(window());
   FactoryResource<BadValues> pBadValues;
   pBadValues->setBadValues(text().toStdString());
   dlg.setBadValues(pBadValues.get());
   if (dlg.exec() == QDialog::Accepted)
   {
      setText(QString::fromStdString(dlg.getBadValuesString()));
   }
   setFocus();

   // Reinstall the event filter
   mpEditButton->installEventFilter(this);
}

bool BadValuesEdit::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (pEvent->type() == QEvent::FocusOut)
      {
         QWidget* pFocusWidget = QApplication::focusWidget();
         if ((pFocusWidget != mpBadValuesStr) && (pFocusWidget != mpEditButton))
         {
            QFocusEvent* pFocusEvent = static_cast<QFocusEvent*>(pEvent);
            QApplication::sendEvent(this, pFocusEvent);
         }
      }
   }

   return QWidget::eventFilter(pObject, pEvent);
}
