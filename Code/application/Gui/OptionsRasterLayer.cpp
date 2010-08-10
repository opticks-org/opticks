/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"

#include "OptionsRasterLayer.h"

#if defined(CG_SUPPORTED)
#include "CgContext.h"
#endif 
#include "ComplexComponentComboBox.h"
#include "CustomTreeWidget.h"
#include "DynamicObject.h"
#include "LabeledSection.h"
#include "ObjectResource.h"
#include "RasterLayer.h"
#include "RegionUnitsComboBox.h"
#include "StretchTypeComboBox.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QDoubleValidator>
#include <QtGui/QGridLayout>
#include <QtGui/QIntValidator>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QVBoxLayout>

#include <limits>
#include <memory>
#include <string>
#include <vector>

using namespace std;

OptionsRasterLayer::OptionsRasterLayer() :
   QWidget(NULL)
{
   // Image Properties
   QLabel* pComplexComponentLabel = new QLabel("Complex Component:", this);
   mpComplexComponent = new ComplexComponentComboBox(this);

   mpUseGpuImage = new QCheckBox("Enable Dynamic Texture Generation", this);  // Do not have an option to select an
                                                                              // initial image filter until required
                                                                              // to do so

   mpFastContrast = new QCheckBox("Fast Contrast", this);
   mpBackgroundTileGen = new QCheckBox("Background Tile Generation", this);

   QWidget* pImagePropWidget = new QWidget(this);
   QGridLayout* pImagePropLayout = new QGridLayout(pImagePropWidget);
   pImagePropLayout->setMargin(0);
   pImagePropLayout->setSpacing(5);
   pImagePropLayout->addWidget(pComplexComponentLabel, 0, 0);
   pImagePropLayout->addWidget(mpComplexComponent, 0, 1, Qt::AlignLeft);
   pImagePropLayout->addWidget(mpUseGpuImage, 1, 0, 1, 2);
   pImagePropLayout->addWidget(mpFastContrast, 2, 0, 1, 2);
   pImagePropLayout->addWidget(mpBackgroundTileGen, 3, 0, 1, 2);
   pImagePropLayout->setColumnStretch(1, 10);
   LabeledSection* pImageSection = new LabeledSection(pImagePropWidget, "Default Image Properties", this);

   // RGB Stretch
   QLabel* pRedStretchLabel = new QLabel("Red:", this);
   QLabel* pGreenStretchLabel = new QLabel("Green:", this);
   QLabel* pBlueStretchLabel = new QLabel("Blue:", this);

   QLabel* pLowerStretchLabel = new QLabel("Lower Value:", this);
   mpRedLowerStretchValue = new QDoubleSpinBox(this);
   mpRedLowerStretchValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpGreenLowerStretchValue = new QDoubleSpinBox(this);
   mpGreenLowerStretchValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpBlueLowerStretchValue = new QDoubleSpinBox(this);
   mpBlueLowerStretchValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pUpperStretchLabel = new QLabel("Upper Value:", this);
   mpRedUpperStretchValue = new QDoubleSpinBox(this);
   mpRedUpperStretchValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpGreenUpperStretchValue = new QDoubleSpinBox(this);
   mpGreenUpperStretchValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpBlueUpperStretchValue = new QDoubleSpinBox(this);
   mpBlueUpperStretchValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pStretchUnitsLabel = new QLabel("Units:", this);
   mpRedStretch = new RegionUnitsComboBox(this);
   mpGreenStretch = new RegionUnitsComboBox(this);
   mpBlueStretch = new RegionUnitsComboBox(this);

   QLabel* pRgbStretchLabel = new QLabel("Type:", this);
   mpRgbStretch = new StretchTypeComboBox(this);

   QWidget* pRgbPropWidget = new QWidget(this);
   QGridLayout* pRgbPropLayout = new QGridLayout(pRgbPropWidget);
   pRgbPropLayout->setMargin(0);
   pRgbPropLayout->setSpacing(5);
   pRgbPropLayout->addWidget(pRedStretchLabel, 0, 1);
   pRgbPropLayout->addWidget(pGreenStretchLabel, 0, 2);
   pRgbPropLayout->addWidget(pBlueStretchLabel, 0, 3);
   pRgbPropLayout->addWidget(pLowerStretchLabel, 1, 0);
   pRgbPropLayout->addWidget(mpRedLowerStretchValue, 1, 1);
   pRgbPropLayout->addWidget(mpGreenLowerStretchValue, 1, 2);
   pRgbPropLayout->addWidget(mpBlueLowerStretchValue, 1, 3);
   pRgbPropLayout->addWidget(pUpperStretchLabel, 2, 0);
   pRgbPropLayout->addWidget(mpRedUpperStretchValue, 2, 1);
   pRgbPropLayout->addWidget(mpGreenUpperStretchValue, 2, 2);
   pRgbPropLayout->addWidget(mpBlueUpperStretchValue, 2, 3);
   pRgbPropLayout->addWidget(pStretchUnitsLabel, 3, 0);
   pRgbPropLayout->addWidget(mpRedStretch, 3, 1);
   pRgbPropLayout->addWidget(mpGreenStretch, 3, 2);
   pRgbPropLayout->addWidget(mpBlueStretch, 3, 3);
   pRgbPropLayout->addWidget(pRgbStretchLabel, 4, 0);
   pRgbPropLayout->addWidget(mpRgbStretch, 4, 1);
   pRgbPropLayout->setColumnStretch(4, 10);
   LabeledSection* pRgbSection = new LabeledSection(pRgbPropWidget, "Default RGB Stretch", this);

   // Grayscale Stretch
   QLabel* pGrayLowerStretchLabel = new QLabel("Lower Value:", this);
   mpGrayLowerStretchValue = new QDoubleSpinBox(this);
   mpGrayLowerStretchValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pGrayUpperStretchLabel = new QLabel("Upper Value:", this);
   mpGrayUpperStretchValue = new QDoubleSpinBox(this);
   mpGrayUpperStretchValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pGrayStretchLabel = new QLabel("Units:", this);
   mpGrayStretch = new RegionUnitsComboBox(this);

   QLabel* pGrayscaleStretchLabel = new QLabel("Type:", this);
   mpGrayscaleStretch = new StretchTypeComboBox(this);

   QWidget* pGrayPropWidget = new QWidget(this);
   QGridLayout* pGrayPropLayout = new QGridLayout(pGrayPropWidget);
   pGrayPropLayout->setMargin(0);
   pGrayPropLayout->setSpacing(5);
   pGrayPropLayout->addWidget(pGrayLowerStretchLabel, 0, 0);
   pGrayPropLayout->addWidget(mpGrayLowerStretchValue, 0, 1, Qt::AlignLeft);
   pGrayPropLayout->addWidget(pGrayUpperStretchLabel, 1, 0);
   pGrayPropLayout->addWidget(mpGrayUpperStretchValue, 1, 1, Qt::AlignLeft);
   pGrayPropLayout->addWidget(pGrayStretchLabel, 2, 0);
   pGrayPropLayout->addWidget(mpGrayStretch, 2, 1, Qt::AlignLeft);
   pGrayPropLayout->addWidget(pGrayscaleStretchLabel, 3, 0);
   pGrayPropLayout->addWidget(mpGrayscaleStretch, 3, 1, Qt::AlignLeft);
   pGrayPropLayout->setColumnStretch(1, 10);
   LabeledSection* pGraySection = new LabeledSection(pGrayPropWidget, "Default Grayscale Stretch", this);

   QStringList columnNames;
   columnNames << "Name" << "Red" << "Green" << "Blue";
   mpColorCompositesTree = new CustomTreeWidget(this);
   mpColorCompositesTree->setColumnCount(columnNames.count());
   mpColorCompositesTree->setHeaderLabels(columnNames);

   QPushButton* pColorCompositesAddButton = new QPushButton("Add", this);
   QPushButton* pColorCompositesRemoveButton = new QPushButton("Remove", this);

   QWidget* pColorCompositesWidget = new QWidget(this);
   QGridLayout* pColorCompositesLayout = new QGridLayout(pColorCompositesWidget);
   pColorCompositesLayout->setMargin(0);
   pColorCompositesLayout->setSpacing(5);
   pColorCompositesLayout->addWidget(mpColorCompositesTree, 0, 0, 3, 1);
   pColorCompositesLayout->addWidget(pColorCompositesAddButton, 0, 1);
   pColorCompositesLayout->addWidget(pColorCompositesRemoveButton, 1, 1);
   pColorCompositesLayout->setRowStretch(2, 10);
   pColorCompositesLayout->setColumnStretch(0, 10);
   LabeledSection* pColorCompositesSection = new LabeledSection(pColorCompositesWidget, "Color Composites", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pImageSection);
   pLayout->addWidget(pRgbSection);
   pLayout->addWidget(pGraySection);
   pLayout->addWidget(pColorCompositesSection, 10);

   bool systemSupportsGpuImage = false;
#if defined(CG_SUPPORTED)
   if (CgContext::instance() != NULL)
   {
      systemSupportsGpuImage = true;
   }
#endif

   mpUseGpuImage->setEnabled(systemSupportsGpuImage);

   // Initialize From Settings
   mpUseGpuImage->setChecked(RasterLayer::getSettingGpuImage());
   mpRedUpperStretchValue->setValue(RasterLayer::getSettingRedUpperStretchValue());
   mpRedStretch->setCurrentValue(RasterLayer::getSettingRedStretchUnits());
   mpRedLowerStretchValue->setValue(RasterLayer::getSettingRedLowerStretchValue());
   mpGreenUpperStretchValue->setValue(RasterLayer::getSettingGreenUpperStretchValue());
   mpGreenStretch->setCurrentValue(RasterLayer::getSettingGreenStretchUnits());
   mpGreenLowerStretchValue->setValue(RasterLayer::getSettingGreenLowerStretchValue());
   mpBlueUpperStretchValue->setValue(RasterLayer::getSettingBlueUpperStretchValue());
   mpBlueStretch->setCurrentValue(RasterLayer::getSettingBlueStretchUnits());
   mpBlueLowerStretchValue->setValue(RasterLayer::getSettingBlueLowerStretchValue());
   mpGrayUpperStretchValue->setValue(RasterLayer::getSettingGrayUpperStretchValue());
   mpGrayStretch->setCurrentValue(RasterLayer::getSettingGrayscaleStretchUnits());
   mpGrayLowerStretchValue->setValue(RasterLayer::getSettingGrayLowerStretchValue());
   mpFastContrast->setChecked(RasterLayer::getSettingFastContrastStretch());
   mpComplexComponent->setCurrentValue(RasterLayer::getSettingComplexComponent());
   mpBackgroundTileGen->setChecked(RasterLayer::getSettingBackgroundTileGeneration());
   mpRgbStretch->setCurrentValue(RasterLayer::getSettingRgbStretchType());
   mpGrayscaleStretch->setCurrentValue(RasterLayer::getSettingGrayscaleStretchType());

   const DynamicObject* pColorComposites = RasterLayer::getSettingColorComposites();
   VERIFYNRV(pColorComposites != NULL);

   vector<string> compositeNames;
   pColorComposites->getAttributeNames(compositeNames);
   for (vector<string>::const_iterator iter = compositeNames.begin(); iter != compositeNames.end(); ++iter)
   {
      addColorCompositeToTree(*iter, dv_cast<DynamicObject>(&pColorComposites->getAttribute(*iter)));
   }

   mpColorCompositesTree->expandAll();
   mpColorCompositesTree->resizeColumnToContents(0);
   mpColorCompositesTree->resizeColumnToContents(1);
   mpColorCompositesTree->resizeColumnToContents(2);
   mpColorCompositesTree->resizeColumnToContents(3);
   mpColorCompositesTree->collapseAll();

   // Connections
   VERIFYNR(connect(pColorCompositesAddButton, SIGNAL(clicked()), this, SLOT(addColorComposite())));
   VERIFYNR(connect(pColorCompositesRemoveButton, SIGNAL(clicked()), this, SLOT(removeColorComposite())));
}

void OptionsRasterLayer::applyChanges()
{
   RasterLayer::setSettingGpuImage(mpUseGpuImage->isChecked());
   RasterLayer::setSettingRedUpperStretchValue(mpRedUpperStretchValue->value());
   RasterLayer::setSettingRedStretchUnits(mpRedStretch->getCurrentValue());
   RasterLayer::setSettingRedLowerStretchValue(mpRedLowerStretchValue->value());
   RasterLayer::setSettingGreenUpperStretchValue(mpGreenUpperStretchValue->value());
   RasterLayer::setSettingGreenStretchUnits(mpGreenStretch->getCurrentValue());
   RasterLayer::setSettingGreenLowerStretchValue(mpGreenLowerStretchValue->value());
   RasterLayer::setSettingBlueUpperStretchValue(mpBlueUpperStretchValue->value());
   RasterLayer::setSettingBlueStretchUnits(mpBlueStretch->getCurrentValue());
   RasterLayer::setSettingBlueLowerStretchValue(mpBlueLowerStretchValue->value());
   RasterLayer::setSettingGrayUpperStretchValue(mpGrayUpperStretchValue->value());
   RasterLayer::setSettingGrayscaleStretchUnits(mpGrayStretch->getCurrentValue());
   RasterLayer::setSettingGrayLowerStretchValue(mpGrayLowerStretchValue->value());
   RasterLayer::setSettingFastContrastStretch(mpFastContrast->isChecked());
   RasterLayer::setSettingComplexComponent(mpComplexComponent->getCurrentValue());
   RasterLayer::setSettingBackgroundTileGeneration(mpBackgroundTileGen->isChecked());
   RasterLayer::setSettingRgbStretchType(mpRgbStretch->getCurrentValue());
   RasterLayer::setSettingGrayscaleStretchType(mpGrayscaleStretch->getCurrentValue());

   FactoryResource<DynamicObject> pColorComposites;
   int numComposites = mpColorCompositesTree->topLevelItemCount();
   for (int i = 0; i < numComposites; ++i)
   {
      QTreeWidgetItem* pItem = mpColorCompositesTree->topLevelItem(i);
      VERIFYNRV(pItem != NULL);

      QTreeWidgetItem* pLower = pItem->child(0);
      VERIFYNRV(pLower != NULL);

      QTreeWidgetItem* pUpper = pItem->child(1);
      VERIFYNRV(pUpper != NULL);

      QTreeWidgetItem* pBandNumber = pItem->child(2);
      VERIFYNRV(pBandNumber != NULL);

      FactoryResource<DynamicObject> pComposite;
      pComposite->setAttribute<double>("redLower", pLower->data(1, Qt::DisplayRole).toDouble());
      pComposite->setAttribute<double>("greenLower", pLower->data(2, Qt::DisplayRole).toDouble());
      pComposite->setAttribute<double>("blueLower", pLower->data(3, Qt::DisplayRole).toDouble());
      pComposite->setAttribute<double>("redUpper", pUpper->data(1, Qt::DisplayRole).toDouble());
      pComposite->setAttribute<double>("greenUpper", pUpper->data(2, Qt::DisplayRole).toDouble());
      pComposite->setAttribute<double>("blueUpper", pUpper->data(3, Qt::DisplayRole).toDouble());
      pComposite->setAttribute<unsigned int>("redBand", pBandNumber->data(1, Qt::DisplayRole).toUInt());
      pComposite->setAttribute<unsigned int>("greenBand", pBandNumber->data(2, Qt::DisplayRole).toUInt());
      pComposite->setAttribute<unsigned int>("blueBand", pBandNumber->data(3, Qt::DisplayRole).toUInt());
      pColorComposites->setAttribute(pItem->text(0).toStdString(), *pComposite.get());
   }

   RasterLayer::setSettingColorComposites(pColorComposites.get());
}

OptionsRasterLayer::~OptionsRasterLayer()
{}

void OptionsRasterLayer::addColorComposite()
{
   FactoryResource<DynamicObject> pComposite;
   pComposite->setAttribute<double>("redLower", 0.0);
   pComposite->setAttribute<double>("greenLower", 0.0);
   pComposite->setAttribute<double>("blueLower", 0.0);
   pComposite->setAttribute<double>("redUpper", 0.0);
   pComposite->setAttribute<double>("greenUpper", 0.0);
   pComposite->setAttribute<double>("blueUpper", 0.0);
   pComposite->setAttribute<unsigned int>("redBand", 0);
   pComposite->setAttribute<unsigned int>("greenBand", 0);
   pComposite->setAttribute<unsigned int>("blueBand", 0);
   addColorCompositeToTree("New Color Composite", pComposite.get());
}

void OptionsRasterLayer::removeColorComposite()
{
   QModelIndex index = mpColorCompositesTree->currentIndex();
   if (index.isValid() == true)
   {
      if (index.parent().isValid() == true)
      {
         index = index.parent();
      }

      delete mpColorCompositesTree->takeTopLevelItem(index.row());
   }
}

void OptionsRasterLayer::addColorCompositeToTree(const std::string& name, const DynamicObject* pObject)
{
   if (pObject == NULL)
   {
      return;
   }

   QStringList columnValues(QString::fromStdString(name));
   std::auto_ptr<QTreeWidgetItem> pRoot(new QTreeWidgetItem(columnValues));

   try
   {
      // Min wavelength
      columnValues.clear();
      columnValues << "Min Wavelength" <<
         QString::number(dv_cast<double>(pObject->getAttribute("redLower"))) <<
         QString::number(dv_cast<double>(pObject->getAttribute("greenLower"))) <<
         QString::number(dv_cast<double>(pObject->getAttribute("blueLower")));
      QTreeWidgetItem* pLower = new QTreeWidgetItem(pRoot.get(), columnValues);
      for (int column = 1; column < columnValues.size(); ++column)
      {
         QLineEdit* pLineEdit = new QLineEdit(mpColorCompositesTree);
         QDoubleValidator* pValidator = new QDoubleValidator(pLineEdit);
         pValidator->setBottom(0.0);
         pLineEdit->setValidator(pValidator);
         mpColorCompositesTree->setCellWidgetType(pLower, column, CustomTreeWidget::CUSTOM_LINE_EDIT);
         mpColorCompositesTree->setCustomLineEdit(pLower, column, pLineEdit);
      }

      // Max wavelength
      columnValues.clear();
      columnValues << "Max Wavelength" <<
         QString::number(dv_cast<double>(pObject->getAttribute("redUpper"))) <<
         QString::number(dv_cast<double>(pObject->getAttribute("greenUpper"))) <<
         QString::number(dv_cast<double>(pObject->getAttribute("blueUpper")));
      QTreeWidgetItem* pUpper = new QTreeWidgetItem(pRoot.get(), columnValues);
      for (int column = 1; column < columnValues.size(); ++column)
      {
         QLineEdit* pLineEdit = new QLineEdit(mpColorCompositesTree);
         QDoubleValidator* pValidator = new QDoubleValidator(pLineEdit);
         pValidator->setBottom(0.0);
         pLineEdit->setValidator(pValidator);
         mpColorCompositesTree->setCellWidgetType(pUpper, column, CustomTreeWidget::CUSTOM_LINE_EDIT);
         mpColorCompositesTree->setCustomLineEdit(pUpper, column, pLineEdit);
      }

      // Band Number
      columnValues.clear();
      columnValues << "Band Number" <<
         QString::number(dv_cast<unsigned int>(pObject->getAttribute("redBand"))) <<
         QString::number(dv_cast<unsigned int>(pObject->getAttribute("greenBand"))) <<
         QString::number(dv_cast<unsigned int>(pObject->getAttribute("blueBand")));
      QTreeWidgetItem* pBandNumber = new QTreeWidgetItem(pRoot.get(), columnValues);
      for (int column = 1; column < columnValues.size(); ++column)
      {
         QLineEdit* pLineEdit = new QLineEdit(mpColorCompositesTree);
         QIntValidator* pValidator = new QIntValidator(pLineEdit);
         pValidator->setBottom(0);
         pLineEdit->setValidator(pValidator);
         mpColorCompositesTree->setCellWidgetType(pBandNumber, column, CustomTreeWidget::CUSTOM_LINE_EDIT);
         mpColorCompositesTree->setCustomLineEdit(pBandNumber, column, pLineEdit);
      }

      mpColorCompositesTree->setCellWidgetType(pRoot.get(), 0, CustomTreeWidget::LINE_EDIT);
      mpColorCompositesTree->addTopLevelItem(pRoot.release());
   }
   catch (const std::bad_cast&)
   {}
}
