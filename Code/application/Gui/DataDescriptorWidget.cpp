/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QHeaderView>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "AppVersion.h"
#include "Classification.h"
#include "CustomTreeWidget.h"
#include "DataDescriptor.h"
#include "DataDescriptorWidget.h"
#include "DataElement.h"
#include "DimensionDescriptor.h"
#include "RasterDataDescriptor.h"
#include "RasterUtilities.h"
#include "Service.h"
#include "StringUtilities.h"
#include "Units.h"
#include "UtilityServicesImp.h"

#include <string>
#include <vector>
using namespace std;

DataDescriptorWidget::DataDescriptorWidget(QWidget* parent) :
   QWidget(parent),
   mReadOnly(true),
   mModified(false),
   mpDescriptor(NULL),
   mpClassificationLabel(NULL),
   mpTreeWidget(NULL),
   mpProcessingLocationCombo(NULL)
{
   // Classification label
   QFont labelFont = font();
   labelFont.setBold(true);

   mpClassificationLabel = new QLabel(this);
   mpClassificationLabel->setFont(labelFont);
   mpClassificationLabel->setAlignment(Qt::AlignCenter);

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

   // Processing locations combo box
   mpProcessingLocationCombo = new QComboBox(mpTreeWidget);
   mpProcessingLocationCombo->setEditable(false);
   mpProcessingLocationCombo->hide();

   // Set True Color pushbutton
   mpTrueColorButton = new QPushButton("Set True Color Display", this);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(mpClassificationLabel);
   pLayout->addWidget(mpTreeWidget);
   pLayout->addWidget(mpTrueColorButton, 0, Qt::AlignRight);

   // Connections
   VERIFYNR(connect(mpTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(descriptorItemChanged(QTreeWidgetItem*, int))));
   VERIFYNR(connect(mpTrueColorButton, SIGNAL(clicked()), this, SLOT(setDisplayBandsToTrueColor())));
}

DataDescriptorWidget::~DataDescriptorWidget()
{
}

void DataDescriptorWidget::setDataDescriptor(DataDescriptor* pDescriptor)
{
   mpTreeWidget->closeActiveCellWidget(false);

   mReadOnly = false;
   mpDescriptor = pDescriptor;
   initialize();
   mModified = false;
}

void DataDescriptorWidget::setDataDescriptor(const DataDescriptor* pDescriptor)
{
   mpTreeWidget->closeActiveCellWidget(false);

   mReadOnly = true;
   mpDescriptor = const_cast<DataDescriptor*>(pDescriptor);
   initialize();
   mModified = false;
}

void DataDescriptorWidget::setValidProcessingLocations(const vector<ProcessingLocation>& locations)
{
   mProcessingLocations = locations;

   mpProcessingLocationCombo->clear();
   for (unsigned int i = 0; i < mProcessingLocations.size(); ++i)
   {
      ProcessingLocation location = mProcessingLocations[i];
      string locationText = StringUtilities::toDisplayString(location);
      if (locationText.empty() == false)
      {
         mpProcessingLocationCombo->addItem(QString::fromStdString(locationText));
      }
   }
}

void DataDescriptorWidget::setDescriptorValue(const QString& strValueName, const QString& strValue)
{
   QTreeWidgetItem* pItem = getDescriptorItem(strValueName);
   if (pItem != NULL)
   {
      pItem->setText(1, strValue);
   }
}

QString DataDescriptorWidget::getDescriptorValue(const QString& strValueName) const
{
   QString strValue;
   if (strValueName.isEmpty() == false)
   {
      QTreeWidgetItem* pItem = getDescriptorItem(strValueName);
      if (pItem != NULL)
      {
         strValue = pItem->text(1);
      }
   }

   return strValue;
}

void DataDescriptorWidget::initialize()
{
   mpTreeWidget->clear();
   mpTrueColorButton->setEnabled(false);
   mpTrueColorButton->setVisible(!mReadOnly);

   if (mpDescriptor == NULL)
   {
      return;
   }

   // Classification
   QString strClassification;
   string classificationLevel;
   const Classification* pClassification = mpDescriptor->getClassification();
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
      labelPalette.setColor(QPalette::Window, Qt::darkYellow); //default to background color used for TS

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
         string name = pParent->getName();
         if (name.empty() == false)
         {
            strParent = QString::fromStdString(name);
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
      if (mReadOnly == false)
      {
         mpTreeWidget->setCellWidgetType(pProcessingLocationItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pProcessingLocationItem, 1, mpProcessingLocationCombo);
      }
   }

   // Raster data descriptor items
   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(mpDescriptor);
   if (pRasterDescriptor == NULL)
   {
      return;
   }

   // True color button
   if (RasterUtilities::canBeDisplayedInTrueColor(pRasterDescriptor) == true)
   {
      mpTrueColorButton->setEnabled(true);
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

      if (mReadOnly == false)
      {
         QComboBox* pDataTypeCombo = new QComboBox(mpTreeWidget);
         pDataTypeCombo->setEditable(false);
         pDataTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT1SBYTE)));
         pDataTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT1UBYTE)));
         pDataTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT2SBYTES)));
         pDataTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT2UBYTES)));
         pDataTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT4SCOMPLEX)));
         pDataTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT4SBYTES)));
         pDataTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT4UBYTES)));
         pDataTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(FLT4BYTES)));
         pDataTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(FLT8COMPLEX)));
         pDataTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(FLT8BYTES)));
         pDataTypeCombo->hide();

         mpTreeWidget->setCellWidgetType(pDataTypeItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pDataTypeItem, 1, pDataTypeCombo);
      }
   }

   // Bad values
   QTreeWidgetItem* pBadValues = new QTreeWidgetItem(mpTreeWidget);
   if (pBadValues != NULL)
   {
      vector<int> badValues = pRasterDescriptor->getBadValues();
      string badValuesText = StringUtilities::toDisplayString(badValues);

      pBadValues->setText(0, "Bad Values");
      pBadValues->setText(1, QString::fromStdString(badValuesText));

      if (mReadOnly == false)
      {
         mpTreeWidget->setCellWidgetType(pBadValues, 1, CustomTreeWidget::LINE_EDIT);
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
         QTreeWidgetItem* pUnitNameItem = new QTreeWidgetItem(pUnitsItem);
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
         QTreeWidgetItem* pUnitTypeItem = new QTreeWidgetItem(pUnitsItem);
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
      if (mReadOnly == false)
      {
         QComboBox* pInterleaveCombo = new QComboBox(mpTreeWidget);
         pInterleaveCombo->setEditable(false);
         pInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BIL)));
         pInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BIP)));
         pInterleaveCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(BSQ)));
         pInterleaveCombo->hide();

         mpTreeWidget->setCellWidgetType(pInterleaveItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pInterleaveItem, 1, pInterleaveCombo);
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

      if (mReadOnly == false)
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

      if (mReadOnly == false)
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

      if (mReadOnly == false)
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

      if (mReadOnly == false)
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

      if (mReadOnly == false)
      {
         QComboBox* pDisplayModeCombo = new QComboBox(mpTreeWidget);
         pDisplayModeCombo->setEditable(false);
         pDisplayModeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(GRAYSCALE_MODE)));
         pDisplayModeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(RGB_MODE)));
         pDisplayModeCombo->hide();

         mpTreeWidget->setCellWidgetType(pDisplayModeItem, 1, CustomTreeWidget::COMBO_BOX);
         mpTreeWidget->setComboBox(pDisplayModeItem, 1, pDisplayModeCombo);
      }
   }
}

bool DataDescriptorWidget::isModified() const
{
   return mModified;
}

bool DataDescriptorWidget::applyChanges()
{
   return applyToDataDescriptor(mpDescriptor);
}

bool DataDescriptorWidget::applyToDataDescriptor(DataDescriptor* pDescriptor)
{
   if (mReadOnly == true)
   {
      return true;
   }

   if (mModified == false)
   {
      return true;
   }

   if (pDescriptor == NULL)
   {
      return false;
   }

   // Processing location
   QString strProcessingLocation = getDescriptorValue("Processing Location");
   if (strProcessingLocation.isEmpty() == false)
   {
      bool bError = true;
      ProcessingLocation location =
         StringUtilities::fromDisplayString<ProcessingLocation>(strProcessingLocation.toStdString(), &bError);
      if (bError == false)
      {
         pDescriptor->setProcessingLocation(location);
      }
   }

   // Raster element descriptor items
   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(pDescriptor);
   if (pRasterDescriptor == NULL)
   {
      return true;
   }

   // Data type
   QString strDataType = getDescriptorValue("Data Type");
   if (strDataType.isEmpty() == false)
   {
      bool bError = true;
      EncodingType dataType = StringUtilities::fromDisplayString<EncodingType>(strDataType.toStdString(), &bError);
      if (bError == false)
      {
         pRasterDescriptor->setDataType(dataType);
      }
   }

   // Bad values
   QString strBadValues = getDescriptorValue("Bad Values");
   if (strBadValues.isEmpty() == false)
   {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Use a QListView to allow editing of bad values, since the fromDisplayString parser is brittle? (kstreith)")
      bool bError = true;
      vector<int> badValues = StringUtilities::fromDisplayString<vector<int> >(strBadValues.toStdString(), &bError);
      if (bError == false)
      {
         pRasterDescriptor->setBadValues(badValues);
      }
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
      QString strUnitsName = getDescriptorValue("Unit Name");
      if (strUnitsName.isEmpty() == false)
      {
         pUnits->setUnitName(strUnitsName.toStdString());
      }

      // Type
      QString strUnitsType = getDescriptorValue("Unit Type");
      if (strUnitsType.isEmpty() == false)
      {
         bool bError = true;
         UnitType unitType = StringUtilities::fromDisplayString<UnitType>(strUnitsType.toStdString(), &bError);
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
      InterleaveFormatType interleave =
         StringUtilities::fromDisplayString<InterleaveFormatType>(strInterleave.toStdString(), &bError);
      if (bError == false)
      {
         pRasterDescriptor->setInterleaveFormat(interleave);
      }
   }

   // Gray band
   DimensionDescriptor grayBand;

   QString strBand = getDescriptorValue("Gray Band");
   if (strBand.isEmpty() == false)
   {
      grayBand = pRasterDescriptor->getOriginalBand(strBand.toUInt() - 1);
      if (grayBand.isValid() == false)
      {
         int iReturn = QMessageBox::warning(this, APP_NAME, "The gray display band is not available!  "
            "Do you want to continue?", QMessageBox::Yes, QMessageBox::No);
         if (iReturn == QMessageBox::No)
         {
            return false;
         }
      }
   }

   pRasterDescriptor->setDisplayBand(GRAY, grayBand);

   // Red band
   DimensionDescriptor redBand;

   strBand = getDescriptorValue("Red Band");
   if (strBand.isEmpty() == false)
   {
      redBand = pRasterDescriptor->getOriginalBand(strBand.toUInt() - 1);
      if (redBand.isValid() == false)
      {
         int iReturn = QMessageBox::warning(this, APP_NAME, "The red display band is not available!  "
            "Do you want to continue?", QMessageBox::Yes, QMessageBox::No);
         if (iReturn == QMessageBox::No)
         {
            return false;
         }
      }
   }

   pRasterDescriptor->setDisplayBand(RED, redBand);

   // Green band
   DimensionDescriptor greenBand;

   strBand = getDescriptorValue("Green Band");
   if (strBand.isEmpty() == false)
   {
      greenBand = pRasterDescriptor->getOriginalBand(strBand.toUInt() - 1);
      if (greenBand.isValid() == false)
      {
         int iReturn = QMessageBox::warning(this, APP_NAME, "The green display band is not available!  "
            "Do you want to continue?", QMessageBox::Yes, QMessageBox::No);
         if (iReturn == QMessageBox::No)
         {
            return false;
         }
      }
   }

   pRasterDescriptor->setDisplayBand(GREEN, greenBand);

   // Blue band
   DimensionDescriptor blueBand;

   strBand = getDescriptorValue("Blue Band");
   if (strBand.isEmpty() == false)
   {
      blueBand = pRasterDescriptor->getOriginalBand(strBand.toUInt() - 1);
      if (blueBand.isValid() == false)
      {
         int iReturn = QMessageBox::warning(this, APP_NAME, "The blue display band is not available!  "
            "Do you want to continue?", QMessageBox::Yes, QMessageBox::No);
         if (iReturn == QMessageBox::No)
         {
            return false;
         }
      }
   }

   pRasterDescriptor->setDisplayBand(BLUE, blueBand);

   // Display mode
   QString strDisplayMode = getDescriptorValue("Display Mode");
   if (strDisplayMode.isEmpty() == false)
   {
      bool bError = true;
      DisplayMode displayMode = StringUtilities::fromDisplayString<DisplayMode>(strDisplayMode.toStdString(), &bError);
      if (bError == false)
      {
         pRasterDescriptor->setDisplayMode(displayMode);
      }
   }

   return true;
}

QSize DataDescriptorWidget::sizeHint() const
{
   return QSize(575, 325);
}

QTreeWidgetItem* DataDescriptorWidget::getDescriptorItem(const QString& strName) const
{
   if (strName.isEmpty() == true)
   {
      return NULL;
   }

   QTreeWidgetItemIterator iter(mpTreeWidget);
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
   if ((pItem == NULL) || (iColumn != 1))
   {
      return;
   }

   QString itemName = pItem->text(0);
   if ((itemName == "X Pixel Size") || (itemName == "Y Pixel Size"))
   {
      QString itemValue = pItem->text(iColumn);
      if (itemValue.isEmpty() == false)
      {
         double pixelSize = itemValue.toDouble();
         if (pixelSize <= 0.0)
         {
            QMessageBox::warning(this, APP_NAME, "The pixel size cannot be negative or zero.  "
               "Please enter a positive number.");

            RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(mpDescriptor);
            if (pRasterDescriptor != NULL)
            {
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
            }

            return;
         }
      }
   }

   mModified = true;
   emit valueChanged(itemName);
   emit modified();
}

void DataDescriptorWidget::setDisplayBandsToTrueColor()
{
   VERIFYNRV(mpDescriptor != NULL);

   RasterDataDescriptor* pRaster = dynamic_cast<RasterDataDescriptor*>(mpDescriptor);
   bool success = (pRaster != NULL);

   if (success)
   {
      success = RasterUtilities::setDisplayBandsToTrueColor(pRaster);
   }

   if (success)
   {
      QString strRedBand, strGreenBand, strBlueBand;
      DimensionDescriptor bandDim = pRaster->getDisplayBand(RED);
      if (bandDim.isValid())
      {
         strRedBand = QString::number(bandDim.getOriginalNumber() + 1);
      }
      else
      {
         success = false;
      }
      bandDim = pRaster->getDisplayBand(GREEN);
      if (bandDim.isValid())
      {
         strGreenBand = QString::number(bandDim.getOriginalNumber() + 1);
      }
      else
      {
         success = false;
      }
      bandDim = pRaster->getDisplayBand(BLUE);
      if (bandDim.isValid())
      {
         strBlueBand = QString::number(bandDim.getOriginalNumber() + 1);
      }
      else
      {
         success = false;
      }

      if (success)
      {
         // block mpTreeWidget signals till till last change made
         mpTreeWidget->blockSignals(true);

         QTreeWidgetItem* pItem = getDescriptorItem("Red Band");
         pItem->setText(1, strRedBand);
         pItem = getDescriptorItem("Green Band");
         pItem->setText(1, strGreenBand);
         pItem = getDescriptorItem("Blue Band");
         pItem->setText(1, strBlueBand);

         // unblock
         mpTreeWidget->blockSignals(false);
         DisplayMode displayMode = pRaster->getDisplayMode();
         string displayModeText = StringUtilities::toDisplayString(displayMode);
         pItem = getDescriptorItem("Display Mode");
         pItem->setText(1, displayModeText.c_str());
      }
   }

   if (success == false)
   {
      QMessageBox::warning(this, "True Color Display", "Unable to set display bands for true color.");
   }
}
