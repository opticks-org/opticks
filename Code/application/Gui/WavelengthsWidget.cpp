/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QDoubleValidator>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QInputDialog>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>

#include "AppVerify.h"
#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "CustomTreeWidget.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "Filename.h"
#include "ObjectFactory.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RasterElement.h"
#include "Slot.h"
#include "WavelengthsImp.h"
#include "WavelengthsWidget.h"
#include "WavelengthUnitsComboBox.h"

#include <limits>
using namespace std;

const QString WavelengthsWidget::mMetadataFilter = "Wavelength Metadata Files (*.wmd)";
const QString WavelengthsWidget::mTextFilter = "Wavelength Files (*.wav *.wave)";

WavelengthsWidget::WavelengthsWidget(QWidget* pParent) :
   QWidget(pParent),
   mpWavelengthData(NULL),
   mpWavelengths(NULL)
{
   // Wavelengths
   QLabel* pWavelengthLabel = new QLabel("Wavelengths:", this);
   mpWavelengthTree = new CustomTreeWidget(this);

   QHeaderView* pHeader = mpWavelengthTree->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultSectionSize(85);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
   }

   QStringList columnNames;
   columnNames << "Band" << "Start" << "Center" << "End";

   mpWavelengthTree->setColumnCount(columnNames.count());
   mpWavelengthTree->setHeaderLabels(columnNames);
   mpWavelengthTree->setRootIsDecorated(false);
   mpWavelengthTree->setGridlinesShown(Qt::Horizontal | Qt::Vertical, true);
   mpWavelengthTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpWavelengthTree->setAllColumnsShowFocus(true);
   mpWavelengthTree->setSortingEnabled(false);
   mpWavelengthTree->setColumnWidth(0, 45);

   // Wavelength buttons
   QPushButton* pElementButton = new QPushButton("From Element...", this);
   QPushButton* pLoadButton = new QPushButton(QIcon(":/icons/Open"), " Load...", this);
   mpSaveButton = new QPushButton(QIcon(":/icons/Save"), " Save...", this);
   mpFwhmButton = new QPushButton("FWHM...", this);
   mpScaleButton = new QPushButton("Scale...", this);

   // Units
   QLabel* pUnitsLabel = new QLabel("Units:", this);
   mpUnitsCombo = new WavelengthUnitsComboBox(this);

   // Layout
   QHBoxLayout* pUnitsLayout = new QHBoxLayout();
   pUnitsLayout->setMargin(0);
   pUnitsLayout->setSpacing(5);
   pUnitsLayout->addWidget(pUnitsLabel);
   pUnitsLayout->addWidget(mpUnitsCombo);
   pUnitsLayout->addStretch();

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pWavelengthLabel, 0, 0);
   pGrid->addWidget(mpWavelengthTree, 1, 0, 5, 2);
   pGrid->addWidget(pElementButton, 1, 2);
   pGrid->addWidget(pLoadButton, 2, 2);
   pGrid->addWidget(mpSaveButton, 3, 2, Qt::AlignTop);
   pGrid->addWidget(mpFwhmButton, 4, 2);
   pGrid->addWidget(mpScaleButton, 5, 2);
   pGrid->setRowMinimumHeight(6, 5);
   pGrid->addLayout(pUnitsLayout, 7, 0, 1, 2);
   pGrid->setRowStretch(3, 10);
   pGrid->setColumnStretch(1, 10);

   // Connections
   VERIFYNR(connect(mpWavelengthTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(wavelengthChanged(QTreeWidgetItem*, int))));
   VERIFYNR(connect(pElementButton, SIGNAL(clicked()), this, SLOT(initializeWavelengthsFromElement())));
   VERIFYNR(connect(pLoadButton, SIGNAL(clicked()), this, SLOT(loadWavelengths())));
   VERIFYNR(connect(mpSaveButton, SIGNAL(clicked()), this, SLOT(saveWavelengths())));
   VERIFYNR(connect(mpFwhmButton, SIGNAL(clicked()), this, SLOT(calculateFwhm())));
   VERIFYNR(connect(mpScaleButton, SIGNAL(clicked()), this, SLOT(applyScaleFactor())));
   VERIFYNR(connect(mpUnitsCombo, SIGNAL(unitsActivated(WavelengthUnitsType)), this,
      SLOT(convertWavelengths(WavelengthUnitsType))));
}

WavelengthsWidget::~WavelengthsWidget()
{
   setWavelengths(vector<DimensionDescriptor>(), NULL, NULL);
}

void WavelengthsWidget::setWavelengths(const vector<DimensionDescriptor>& bands, DynamicObject* pWavelengthData)
{
   setWavelengths(bands, pWavelengthData, NULL);
}

void WavelengthsWidget::setWavelengths(const vector<DimensionDescriptor>& bands, Wavelengths* pWavelengths)
{
   setWavelengths(bands, NULL, pWavelengths);
}

void WavelengthsWidget::setWavelengths(const vector<DimensionDescriptor>& bands, DynamicObject* pWavelengthData,
                                       Wavelengths* pWavelengths)
{
   if (mpWavelengthData != NULL)
   {
      mpWavelengthData->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WavelengthsWidget::wavelengthDataDeleted));
      if (mpWavelengths != NULL)
      {
         delete dynamic_cast<WavelengthsImp*>(mpWavelengths);
         mpWavelengths = NULL;
      }
   }

   mpWavelengthData = pWavelengthData;
   mpWavelengths = pWavelengths;

   if (mpWavelengthData != NULL)
   {
      mpWavelengthData->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WavelengthsWidget::wavelengthDataDeleted));
      mpWavelengths = new WavelengthsImp();
      mpWavelengths->initializeFromDynamicObject(mpWavelengthData, true);
   }

   // Update the number of tree widget items
   VERIFYNR(disconnect(mpWavelengthTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(wavelengthChanged(QTreeWidgetItem*, int))));

   mpWavelengthTree->clear();

   for (unsigned int i = 0; i < bands.size(); ++i)
   {
      QTreeWidgetItem* pItem = new QTreeWidgetItem(mpWavelengthTree);

      // Band number
      unsigned int bandNumber = bands[i].getOriginalNumber();
      pItem->setText(0, QString::number(bandNumber + 1));

      // Wavelength values
      QLineEdit* pStartEdit = new QLineEdit(mpWavelengthTree);
      pStartEdit->setValidator(new QDoubleValidator(0.0, numeric_limits<double>::max(), 8, pStartEdit));
      pStartEdit->hide();

      mpWavelengthTree->setCellWidgetType(pItem, 1, CustomTreeWidget::CUSTOM_LINE_EDIT);
      mpWavelengthTree->setCustomLineEdit(pItem, 1, pStartEdit);

      QLineEdit* pCenterEdit = new QLineEdit(mpWavelengthTree);
      pCenterEdit->setValidator(new QDoubleValidator(0.0, numeric_limits<double>::max(), 8, pCenterEdit));
      pCenterEdit->hide();

      mpWavelengthTree->setCellWidgetType(pItem, 2, CustomTreeWidget::CUSTOM_LINE_EDIT);
      mpWavelengthTree->setCustomLineEdit(pItem, 2, pCenterEdit);

      QLineEdit* pEndEdit = new QLineEdit(mpWavelengthTree);
      pEndEdit->setValidator(new QDoubleValidator(0.0, numeric_limits<double>::max(), 8, pEndEdit));
      pEndEdit->hide();

      mpWavelengthTree->setCellWidgetType(pItem, 3, CustomTreeWidget::CUSTOM_LINE_EDIT);
      mpWavelengthTree->setCustomLineEdit(pItem, 3, pEndEdit);
   }

   VERIFYNR(connect(mpWavelengthTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(wavelengthChanged(QTreeWidgetItem*, int))));

   // Populate the tree widget items with the wavelength values and the units combo
   updateWidgetsFromWavelengths();
}

QSize WavelengthsWidget::sizeHint() const
{
   return QSize(500, 300);
}

void WavelengthsWidget::highlightActiveBands(const vector<DimensionDescriptor>& bands)
{
   QFont itemFont = mpWavelengthTree->font();

   QFont activeFont = itemFont;
   activeFont.setBold(true);

   VERIFYNR(disconnect(mpWavelengthTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(wavelengthChanged(QTreeWidgetItem*, int))));

   for (unsigned int i = 0; i < static_cast<unsigned int>(mpWavelengthTree->topLevelItemCount()); ++i)
   {
      QTreeWidgetItem* pItem = mpWavelengthTree->topLevelItem(i);
      if (pItem != NULL)
      {
         unsigned int bandNumber = pItem->text(0).toUInt() - 1;
         bool activeBand = false;

         for (vector<DimensionDescriptor>::const_iterator iter = bands.begin(); iter != bands.end(); ++iter)
         {
            unsigned int currentBandNumber = iter->getOriginalNumber();
            if (currentBandNumber == bandNumber)
            {
               activeBand = true;
               break;
            }
         }

         if (activeBand == true)
         {
            pItem->setFont(0, activeFont);
            pItem->setFont(1, activeFont);
            pItem->setFont(2, activeFont);
            pItem->setFont(3, activeFont);
         }
         else
         {
            pItem->setFont(0, itemFont);
            pItem->setFont(1, itemFont);
            pItem->setFont(2, itemFont);
            pItem->setFont(3, itemFont);
         }
      }
   }

   VERIFYNR(connect(mpWavelengthTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(wavelengthChanged(QTreeWidgetItem*, int))));
}

void WavelengthsWidget::wavelengthDataDeleted(Subject& subject, const string& signal, const boost::any& value)\
{
   setWavelengths(vector<DimensionDescriptor>(), NULL, NULL);
}

void WavelengthsWidget::showEvent(QShowEvent* pEvent)
{
   // Update the widgets in case the wavelengths changed externally to this widget
   if (mpWavelengths != NULL)
   {
      if (mpWavelengthData != NULL)
      {
         mpWavelengths->initializeFromDynamicObject(mpWavelengthData, true);
      }

      updateWidgetsFromWavelengths();
   }

   QWidget::showEvent(pEvent);
}

void WavelengthsWidget::initializeWavelengthsFromElement()
{
   if (mpWavelengths == NULL)
   {
      return;
   }

   int numBands = mpWavelengthTree->topLevelItemCount();

   // Get the raster elements with valid wavelengths and at least the same number of bands
   Service<ModelServices> pModel;
   QMap<QString, const DynamicObject*> elementMetadata;

   vector<DataElement*> elements = pModel->getElements(TypeConverter::toString<RasterElement>());
   for (vector<DataElement*>::const_iterator iter = elements.begin(); iter != elements.end(); ++iter)
   {
      DataElement* pElement = *iter;
      if (pElement != NULL)
      {
         const DynamicObject* pMetadata = pElement->getMetadata();
         if (pMetadata != NULL)
         {
            if (Wavelengths::getNumWavelengths(pMetadata) == static_cast<unsigned int>(numBands))
            {
               elementMetadata[QString::fromStdString(pElement->getName())] = pMetadata;
            }
         }
      }
   }

   // Have the user choose a raster element
   QStringList elementNames(elementMetadata.keys());
   if (elementNames.empty() == true)
   {
      QMessageBox::warning(this, APP_NAME, "No raster elements are available with valid wavelengths for all bands.");
      return;
   }

   QString elementName = QInputDialog::getItem(this, APP_NAME, "Select Raster Element:", elementNames, 0, false);
   if (elementName.isEmpty() == true)
   {
      return;
   }

   // Set the wavelengths to the raster element wavelengths
   const DynamicObject* pMetadata = elementMetadata.value(elementName);
   if (pMetadata != NULL)
   {
      mpWavelengths->initializeFromDynamicObject(pMetadata, true);
      mpWavelengths->applyToDynamicObject(mpWavelengthData);
      updateWidgetsFromWavelengths();
   }
}

void WavelengthsWidget::loadWavelengths()
{
   if (mpWavelengths == NULL)
   {
      return;
   }

   // Get the default import directory
   Service<ConfigurationSettings> pSettings;
   string key = ConfigurationSettings::getSettingPluginWorkingDirectoryKey(Wavelengths::WavelengthType());

   const Filename* pDirectory = pSettings->getSetting(key).getPointerToValue<Filename>();
   if (pDirectory == NULL)
   {
      pDirectory = ConfigurationSettings::getSettingImportPath();
   }

   QString initialDirectory;
   if (pDirectory != NULL)
   {
      initialDirectory = QString::fromStdString(pDirectory->getFullPathAndName());
   }

   // Get the filename from the user
   QString strCaption = "Load Wavelengths";
   QString strSelectedFilter;

   QString strFilename = QFileDialog::getOpenFileName(this, strCaption, initialDirectory,
      mMetadataFilter + ";;" + mTextFilter + ";;All Files (*)", &strSelectedFilter);
   if (strFilename.isEmpty() == true)
   {
      return;
   }

   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(strFilename.toStdString());

   // Update the import directory
   QFileInfo fileInfo(strFilename);
   QString importDirectory = fileInfo.absolutePath();
   if (importDirectory.isEmpty() == false)
   {
      FactoryResource<Filename> pImportDirectory;
      pImportDirectory->setFullPathAndName(importDirectory.toStdString());
      pSettings->setSessionSetting(key, *pImportDirectory.get());
   }

   // Load as metadata in XML format
   ExecutableResource pImporter;
   bool bSuccess = false;

   if (strSelectedFilter != mTextFilter)
   {
      pImporter = ExecutableResource("Wavelength Metadata Importer", string(), NULL, false);
      pImporter->getInArgList().setPlugInArgValue(Wavelengths::WavelengthFileArg(), pFilename.get());

      bSuccess = pImporter->execute();
   }

   // Load as an ASCII text file
   if ((bSuccess == false) && (strSelectedFilter != mMetadataFilter))
   {
      pImporter = ExecutableResource("Wavelength Text Importer", string(), NULL, false);
      pImporter->getInArgList().setPlugInArgValue(Wavelengths::WavelengthFileArg(), pFilename.get());

      bSuccess = pImporter->execute();
   }

   if (bSuccess == true)
   {
      // Update the wavelength values
      Wavelengths* pWavelengths =
         pImporter->getOutArgList().getPlugInArgValue<Wavelengths>(Wavelengths::WavelengthsArg());
      if (pWavelengths != NULL)
      {
         // Ensure the number of wavelengths equals the number of bands
         bool applyWavelengths = true;

         unsigned int numWavelengths = pWavelengths->getNumWavelengths();
         unsigned int numBands = static_cast<unsigned int>(mpWavelengthTree->topLevelItemCount());

         if (numWavelengths > numBands)
         {
            int value = QMessageBox::question(this, APP_NAME, "The number of wavelengths in the file is greater "
               "than the total number of bands.  If the wavelengths are applied, values beyond the number "
               "of bands will be ignored.  Do you want to continue?", QMessageBox::Yes | QMessageBox::No);
            if (value == QMessageBox::Yes)
            {
               vector<double> startValues = pWavelengths->getStartValues();
               vector<double> centerValues = pWavelengths->getCenterValues();
               vector<double> endValues = pWavelengths->getEndValues();
               WavelengthUnitsType units = pWavelengths->getUnits();

               // Clear the Wavelengths object since it enforces vectors of the same size
               pWavelengths->clear();

               // Set the units before setting the wavelength values to avoid value conversions
               pWavelengths->setUnits(units);

               // Set the resized values
               if (startValues.empty() == false)
               {
                  startValues.resize(numBands);
                  pWavelengths->setStartValues(startValues, units);
               }

               if (centerValues.empty() == false)
               {
                  centerValues.resize(numBands);
                  pWavelengths->setCenterValues(centerValues, units);
               }

               if (endValues.empty() == false)
               {
                  endValues.resize(numBands);
                  pWavelengths->setEndValues(endValues, units);
               }
            }
            else if (value == QMessageBox::No)
            {
               applyWavelengths = false;
            }
         }
         else if (numWavelengths < numBands)
         {
            int value = QMessageBox::question(this, APP_NAME, "The number of wavelengths in the file is less "
               "than the total number of bands.  If the wavelengths are applied, values in bands beyond the number "
               "of wavelengths in the file will be set to zero.  Do you want to continue?",
               QMessageBox::Yes | QMessageBox::No);
            if (value == QMessageBox::Yes)
            {
               vector<double> startValues = pWavelengths->getStartValues();
               vector<double> centerValues = pWavelengths->getCenterValues();
               vector<double> endValues = pWavelengths->getEndValues();
               WavelengthUnitsType units = pWavelengths->getUnits();

               // Clear the Wavelengths object since it enforces vectors of the same size
               pWavelengths->clear();

               // Set the units before setting the wavelength values to avoid value conversions
               pWavelengths->setUnits(units);

               // Set the resized values
               if (startValues.empty() == false)
               {
                  startValues.resize(numBands, 0.0);
                  pWavelengths->setStartValues(startValues, units);
               }

               if (centerValues.empty() == false)
               {
                  centerValues.resize(numBands, 0.0);
                  pWavelengths->setCenterValues(centerValues, units);
               }

               if (endValues.empty() == false)
               {
                  endValues.resize(numBands, 0.0);
                  pWavelengths->setEndValues(endValues, units);
               }
            }
            else if (value == QMessageBox::No)
            {
               applyWavelengths = false;
            }
         }

         if (applyWavelengths == true)
         {
            mpWavelengths->initializeFromWavelengths(pWavelengths);
         }

         // Destroy the arg value since Wavelengths are not supported by DataVariant
         delete dynamic_cast<WavelengthsImp*>(pWavelengths);

         if (applyWavelengths == false)
         {
            return;
         }
      }
      else
      {
         mpWavelengths->clear();
      }

      mpWavelengths->applyToDynamicObject(mpWavelengthData);
      updateWidgetsFromWavelengths();
   }
   else
   {
      // Report the error to the user
      Progress* pProgress = pImporter->getProgress();
      if (pProgress != NULL)
      {
         string message;
         int percent = 0;
         ReportingLevel level;
         pProgress->getProgress(message, percent, level);

         if ((message.empty() == false) && (level == ERRORS))
         {
            QMessageBox::critical(this, APP_NAME, QString::fromStdString(message));
         }
      }
   }
}

void WavelengthsWidget::saveWavelengths()
{
   if (mpWavelengths == NULL)
   {
      return;
   }

   if (mpWavelengths->isEmpty() == true)
   {
      QMessageBox::critical(this, APP_NAME, "Wavelength values must be present before saving.");
      return;
   }

   // Get the default export directory
   Service<ConfigurationSettings> pSettings;
   string key = ConfigurationSettings::getSettingPluginWorkingDirectoryKey(Wavelengths::WavelengthType());

   const Filename* pDirectory = pSettings->getSetting(key).getPointerToValue<Filename>();
   if (pDirectory == NULL)
   {
      pDirectory = ConfigurationSettings::getSettingExportPath();
   }

   QString initialDirectory;
   if (pDirectory != NULL)
   {
      initialDirectory = QString::fromStdString(pDirectory->getFullPathAndName());
   }

   // Get the filename from the user
   QString strCaption = "Save Wavelengths";
   QString strSelectedFilter;

   QString strFilename = QFileDialog::getSaveFileName(this, strCaption, initialDirectory,
      mMetadataFilter + ";;" + mTextFilter, &strSelectedFilter);
   if (strFilename.isEmpty() == true)
   {
      return;
   }

   // Update the export directory
   QFileInfo fileInfo(strFilename);
   QString exportDirectory = fileInfo.absolutePath();
   if (exportDirectory.isEmpty() == false)
   {
      FactoryResource<Filename> pExportDirectory;
      pExportDirectory->setFullPathAndName(exportDirectory.toStdString());
      pSettings->setSessionSetting(key, *pExportDirectory.get());
   }

   // Save the wavelengths
   ExecutableResource pExporter;
   if (strSelectedFilter == mMetadataFilter)
   {
      pExporter = ExecutableResource("Wavelength Metadata Exporter", string(), NULL, false);

      if (strFilename.endsWith(".wmd") == false)
      {
         strFilename.append(".wmd");
      }
   }
   else if (strSelectedFilter == mTextFilter)
   {
      if (QMessageBox::question(this, APP_NAME, "The wavelength text file format does not contain "
         "units information.  To save the wavelength units in addition to the values, save the file "
         "in the wavelengths metadata format instead.\n\nDo you want to continue?",
         QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      {
         return;
      }

      pExporter = ExecutableResource("Wavelength Text Exporter", string(), NULL, false);

      if ((strFilename.endsWith(".wav") == false) && (strFilename.endsWith(".wave") == false))
      {
         strFilename.append(".wav");
      }
   }

   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(strFilename.toStdString());

   pExporter->getInArgList().setPlugInArgValue(Wavelengths::WavelengthsArg(), mpWavelengths);
   pExporter->getInArgList().setPlugInArgValue(Wavelengths::WavelengthFileArg(), pFilename.get());
   pExporter->execute();

   // Report to the user that the exporter is finished
   Progress* pProgress = pExporter->getProgress();
   if (pProgress != NULL)
   {
      string message;
      int percent = 0;
      ReportingLevel level;
      pProgress->getProgress(message, percent, level);

      if (message.empty() == false)
      {
         if (level == NORMAL)
         {
            QMessageBox::information(this, APP_NAME, QString::fromStdString(message));
         }
         else if (level == ERRORS)
         {
            QMessageBox::critical(this, APP_NAME, QString::fromStdString(message));
         }
      }
   }
}

void WavelengthsWidget::wavelengthChanged(QTreeWidgetItem* pItem, int column)
{
   if ((pItem == NULL) || (mpWavelengths == NULL))
   {
      return;
   }

   // Get the wavelength vector containing the value that was edited and the value units
   vector<double> wavelengthValues;
   if (column == 1)
   {
      wavelengthValues = mpWavelengths->getStartValues();
   }
   else if (column == 2)
   {
      wavelengthValues = mpWavelengths->getCenterValues();
   }
   else if (column == 3)
   {
      wavelengthValues = mpWavelengths->getEndValues();
   }

   WavelengthUnitsType units = mpWavelengths->getUnits();

   // Get the edited wavelength value
   double wavelength = 0.0;
   bool validWavelengths = false;

   QString valueText = pItem->text(column);
   if (valueText.isEmpty() == true)
   {
      for (int i = 0; i < mpWavelengthTree->topLevelItemCount(); ++i)
      {
         QTreeWidgetItem* pCurrentItem = mpWavelengthTree->topLevelItem(i);
         if (pCurrentItem != NULL)
         {
            QString currentValueText = pCurrentItem->text(column);
            if (currentValueText.isEmpty() == false)
            {
               validWavelengths = true;
               break;
            }
         }
      }

      if (validWavelengths == false)
      {
         wavelengthValues.clear();
      }
   }
   else
   {
      wavelength = valueText.toDouble();
      validWavelengths = true;
   }

   // Update the wavelength value in the vector
   int valueIndex = mpWavelengthTree->indexOfTopLevelItem(pItem);
   if (wavelengthValues.empty() == false)
   {
      if ((valueIndex != -1) && (valueIndex < static_cast<int>(wavelengthValues.size())))
      {
         wavelengthValues[valueIndex] = wavelength;
      }
   }
   else if (validWavelengths == true)
   {
      for (int i = 0; i < mpWavelengthTree->topLevelItemCount(); ++i)
      {
         if (i == valueIndex)
         {
            wavelengthValues.push_back(wavelength);
         }
         else
         {
            wavelengthValues.push_back(0.0);
         }
      }
   }

   // Update the vector in the member wavelengths
   if (column == 1)
   {
      mpWavelengths->setStartValues(wavelengthValues, units);
   }
   else if (column == 2)
   {
      mpWavelengths->setCenterValues(wavelengthValues, units);
   }
   else if (column == 3)
   {
      mpWavelengths->setEndValues(wavelengthValues, units);
   }

   // Apply the changes back to the dynamic object
   mpWavelengths->applyToDynamicObject(mpWavelengthData);
}

void WavelengthsWidget::calculateFwhm()
{
   if (mpWavelengths == NULL)
   {
      return;
   }

   const vector<double>& centerValues = mpWavelengths->getCenterValues();
   if (centerValues.size() < 2)
   {
      QMessageBox::critical(this, APP_NAME, "At least two center wavelength values are required "
         "to calculate the FWHM values.");
      return;
   }

   bool bAccepted = false;

   double dConstant = QInputDialog::getDouble(this, "Calculate FWHM", "FWHM Constant:", 1.0,
      -numeric_limits<double>::max(), numeric_limits<double>::max(), 2, &bAccepted);
   if (bAccepted == true)
   {
      mpWavelengths->calculateFwhm(dConstant);
      mpWavelengths->applyToDynamicObject(mpWavelengthData);
      updateWidgetsFromWavelengths();
   }
}

void WavelengthsWidget::applyScaleFactor()
{
   if (mpWavelengths == NULL)
   {
      return;
   }

   if (mpWavelengths->isEmpty() == true)
   {
      QMessageBox::critical(this, APP_NAME, "Wavelength values must be present before applying a scale factor.");
      return;
   }

   QDialog scaleDialog(this);
   scaleDialog.setWindowTitle("Apply Scale");

   QLabel* pScaleLabel = new QLabel("Wavelength Scale Factor:", &scaleDialog);
   QDoubleSpinBox* pScaleSpin = new QDoubleSpinBox(&scaleDialog);
   pScaleSpin->setDecimals(6);
   pScaleSpin->setRange(0.000001, numeric_limits<double>::max());
   pScaleSpin->setValue(1.0);

   QLabel* pUnitsLabel = new QLabel("Scaled Units:", &scaleDialog);
   pUnitsLabel->setToolTip("These units define the wavelength values after the scale factor is applied.  "
      "No conversion of the wavelength values or scale factor is performed.");
   WavelengthUnitsComboBox* pUnitsCombo = new WavelengthUnitsComboBox(&scaleDialog);
   pUnitsCombo->setUnits(mpWavelengths->getUnits());

   QFrame* pLine = new QFrame(&scaleDialog);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, &scaleDialog);

   QGridLayout* pLayout = new QGridLayout(&scaleDialog);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pScaleLabel, 0, 0);
   pLayout->addWidget(pScaleSpin, 0, 1);
   pLayout->addWidget(pUnitsLabel, 1, 0);
   pLayout->addWidget(pUnitsCombo, 1, 1);
   pLayout->setRowStretch(2, 10);
   pLayout->addWidget(pLine, 3, 0, 1, 2);
   pLayout->addWidget(pButtonBox, 4, 0, 1, 2);

   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), &scaleDialog, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), &scaleDialog, SLOT(reject())));

   if (scaleDialog.exec() == QDialog::Accepted)
   {
      mpWavelengths->scaleValues(pScaleSpin->value());
      mpWavelengths->setUnits(pUnitsCombo->getUnits(), false);
      mpWavelengths->applyToDynamicObject(mpWavelengthData);
      updateWidgetsFromWavelengths();
   }
}

void WavelengthsWidget::convertWavelengths(WavelengthUnitsType newUnits)
{
   if (mpWavelengths == NULL)
   {
      return;
   }

   mpWavelengths->setUnits(newUnits);
   mpWavelengths->applyToDynamicObject(mpWavelengthData);
   updateWidgetsFromWavelengths();
}

void WavelengthsWidget::updateWidgetsFromWavelengths()
{
   if (mpWavelengths == NULL)
   {
      return;
   }

   VERIFYNR(disconnect(mpWavelengthTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(wavelengthChanged(QTreeWidgetItem*, int))));

   // Values
   const vector<double>& startValues = mpWavelengths->getStartValues();
   const vector<double>& centerValues = mpWavelengths->getCenterValues();
   const vector<double>& endValues = mpWavelengths->getEndValues();

   for (unsigned int i = 0; i < static_cast<unsigned int>(mpWavelengthTree->topLevelItemCount()); ++i)
   {
      QTreeWidgetItem* pItem = mpWavelengthTree->topLevelItem(i);
      if (pItem != NULL)
      {
         // Start value
         QString startValue;
         if (i < startValues.size())
         {
            if (startValues[i] > 0.0)
            {
               startValue = QString::number(startValues[i]);
            }
         }

         pItem->setText(1, startValue);

         // Center value
         QString centerValue;
         if (i < centerValues.size())
         {
            if (centerValues[i] > 0.0)
            {
               centerValue = QString::number(centerValues[i]);
            }
         }

         pItem->setText(2, centerValue);

         // End value
         QString endValue;
         if (i < endValues.size())
         {
            if (endValues[i] > 0.0)
            {
               endValue = QString::number(endValues[i]);
            }
         }

         pItem->setText(3, endValue);
      }
   }

   // Units
   WavelengthUnitsType units = mpWavelengths->getUnits();
   mpUnitsCombo->setUnits(units);

   VERIFYNR(connect(mpWavelengthTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
      SLOT(wavelengthChanged(QTreeWidgetItem*, int))));
}
