/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QStringList>
#include <QtGui/QAction>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QListWidget>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QStackedWidget>

#include "AppConfig.h"
#include "AppVersion.h"
#if defined (CG_SUPPORTED)
#include "CgContext.h"
#endif
#include "ComplexComponentComboBox.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "ImageFilterManager.h"
#include "LabeledSection.h"
#include "ModelServices.h"
#include "PropertiesRasterLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "RasterLayerImp.h"
#include "RasterUtilities.h"
#include "RegionUnitsComboBox.h"
#include "SignalBlocker.h"
#include "StretchTypeComboBox.h"
#include "StringUtilities.h"
#include "Undo.h"
#include "View.h"

#include <limits>
using namespace std;

PropertiesRasterLayer::PropertiesRasterLayer() :
   LabeledSectionGroup(NULL),
   mInitializing(false),
   mpRasterLayer(NULL)
{
   // Display configuration
   QWidget* pDisplayWidget = new QWidget(this);

   QLabel* pDisplayModeLabel = new QLabel("Display Mode:", pDisplayWidget);
   mpDisplayModeCombo = new QComboBox(pDisplayWidget);
   vector<string> modes = StringUtilities::getAllEnumValuesAsDisplayString<DisplayMode>();
   for (vector<string>::iterator it = modes.begin(); it != modes.end(); ++it)
   {
      mpDisplayModeCombo->addItem(QString::fromStdString(*it));
   }
   mpComplexComponentLabel = new QLabel("Complex Component:", pDisplayWidget);
   mpComplexComponentCombo = new ComplexComponentComboBox(pDisplayWidget);
   QLabel* pOpacityLabel = new QLabel("Opacity:", pDisplayWidget);
   mpOpacitySpin = new QSpinBox(pDisplayWidget);
   mpOpacitySpin->setMinimum(0);
   mpOpacitySpin->setMaximum(255);
   mpOpacitySpin->setSingleStep(1);
   mDisplayConfigModifier.setModified(false);

   LabeledSection* pDisplaySection = new LabeledSection(pDisplayWidget, "Display Configuration", this);

   QGridLayout* pDisplayGrid = new QGridLayout(pDisplayWidget);
   pDisplayGrid->setMargin(0);
   pDisplayGrid->setSpacing(5);
   pDisplayGrid->addWidget(pDisplayModeLabel, 0, 0);
   pDisplayGrid->addWidget(mpDisplayModeCombo, 0, 1, Qt::AlignLeft);
   pDisplayGrid->addWidget(mpComplexComponentLabel, 1, 0);
   pDisplayGrid->addWidget(mpComplexComponentCombo, 1, 1, Qt::AlignLeft);
   pDisplayGrid->addWidget(pOpacityLabel, 2, 0);
   pDisplayGrid->addWidget(mpOpacitySpin, 2, 1, Qt::AlignLeft);
   pDisplayGrid->setRowStretch(3, 10);
   pDisplayGrid->setColumnStretch(1, 10);

   // Grayscale
   QWidget* pGrayscaleWidget = new QWidget(this);

   QLabel* pGrayElementLabel = new QLabel("Display Element:", pGrayscaleWidget);
   mpGrayElementCombo = new QComboBox(pGrayscaleWidget);
   mpGrayElementCombo->setMinimumContentsLength(10);

   QLabel* pGrayBandLabel = new QLabel("Display Band:", pGrayscaleWidget);
   mpGrayBandCombo = new QComboBox(pGrayscaleWidget);

   QLabel* pGrayLowerLabel = new QLabel("Lower Stretch Value:", pGrayscaleWidget);
   mpGrayLowerSpin = new QDoubleSpinBox(pGrayscaleWidget);
   mpGrayLowerSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpGrayLowerSpin->setDecimals(6);

   QLabel* pGrayUpperLabel = new QLabel("Upper Stretch Value:", pGrayscaleWidget);
   mpGrayUpperSpin = new QDoubleSpinBox(pGrayscaleWidget);
   mpGrayUpperSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpGrayUpperSpin->setDecimals(6);

   QLabel* pGrayUnitsLabel = new QLabel("Stretch Units:", pGrayscaleWidget);
   mpGrayUnitsCombo = new RegionUnitsComboBox(pGrayscaleWidget);

   QLabel* pGrayStretchTypeLabel = new QLabel("Stretch Type:", pGrayscaleWidget);
   mpGrayStretchTypeCombo = new StretchTypeComboBox(pGrayscaleWidget);

   QPushButton* pGrayStretchFavoritesButton = new QPushButton("Favorites", this);
   mpGrayStretchMenu = new QMenu(pGrayStretchFavoritesButton);
   pGrayStretchFavoritesButton->setMenu(mpGrayStretchMenu);

   mpAddFavoriteGrayAction = new QAction("Add Stretch to Favorites", this);
   mpRemoveFavoriteAction = new QAction("Remove Stretch from Favorites...", this);
   mGrayscaleModifier.setModified(false);

   LabeledSection* pGrayscaleSection = new LabeledSection(pGrayscaleWidget, "Grayscale", this);

   QGridLayout* pGrayscaleGrid = new QGridLayout(pGrayscaleWidget);
   pGrayscaleGrid->setMargin(0);
   pGrayscaleGrid->setSpacing(5);
   pGrayscaleGrid->addWidget(pGrayElementLabel, 0, 0);
   pGrayscaleGrid->addWidget(mpGrayElementCombo, 0, 1, 1, 2);
   pGrayscaleGrid->addWidget(pGrayBandLabel, 1, 0);
   pGrayscaleGrid->addWidget(mpGrayBandCombo, 1, 1, 1, 2);
   pGrayscaleGrid->addWidget(pGrayLowerLabel, 2, 0);
   pGrayscaleGrid->addWidget(mpGrayLowerSpin, 2, 1);
   pGrayscaleGrid->addWidget(pGrayStretchFavoritesButton, 2, 2, Qt::AlignLeft);
   pGrayscaleGrid->addWidget(pGrayUpperLabel, 3, 0);
   pGrayscaleGrid->addWidget(mpGrayUpperSpin, 3, 1);
   pGrayscaleGrid->addWidget(pGrayUnitsLabel, 4, 0);
   pGrayscaleGrid->addWidget(mpGrayUnitsCombo, 4, 1);
   pGrayscaleGrid->addWidget(pGrayStretchTypeLabel, 5, 0);
   pGrayscaleGrid->addWidget(mpGrayStretchTypeCombo, 5, 1);
   pGrayscaleGrid->setRowStretch(6, 10);
   pGrayscaleGrid->setColumnStretch(2, 10);

   // RGB
   QWidget* pRgbWidget = new QWidget(this);

   QLabel* pRedLabel = new QLabel("Red:", pRgbWidget);
   QLabel* pGreenLabel = new QLabel("Green:", pRgbWidget);
   QLabel* pBlueLabel = new QLabel("Blue:", pRgbWidget);

   QLabel* pRgbElementLabel = new QLabel("Display Element:", pRgbWidget);
   mpRedElementCombo = new QComboBox(pRgbWidget);
   mpRedElementCombo->setMinimumContentsLength(10);
   mpGreenElementCombo = new QComboBox(pRgbWidget);
   mpGreenElementCombo->setMinimumContentsLength(10);
   mpBlueElementCombo = new QComboBox(pRgbWidget);
   mpBlueElementCombo->setMinimumContentsLength(10);

   QLabel* pRgbBandLabel = new QLabel("Display Band:", pRgbWidget);
   mpRedBandCombo = new QComboBox(pRgbWidget);
   mpGreenBandCombo = new QComboBox(pRgbWidget);
   mpBlueBandCombo = new QComboBox(pRgbWidget);

   QLabel* pRgbLowerLabel = new QLabel("Lower Stretch Value:", pRgbWidget);
   mpRedLowerSpin = new QDoubleSpinBox(pRgbWidget);
   mpRedLowerSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpRedLowerSpin->setDecimals(6);
   mpGreenLowerSpin = new QDoubleSpinBox(pRgbWidget);
   mpGreenLowerSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpGreenLowerSpin->setDecimals(6);
   mpBlueLowerSpin = new QDoubleSpinBox(pRgbWidget);
   mpBlueLowerSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpBlueLowerSpin->setDecimals(6);

   QLabel* pRgbUpperLabel = new QLabel("Upper Stretch Value:", pRgbWidget);
   mpRedUpperSpin = new QDoubleSpinBox(pRgbWidget);
   mpRedUpperSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpRedUpperSpin->setDecimals(6);
   mpGreenUpperSpin = new QDoubleSpinBox(pRgbWidget);
   mpGreenUpperSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpGreenUpperSpin->setDecimals(6);
   mpBlueUpperSpin = new QDoubleSpinBox(pRgbWidget);
   mpBlueUpperSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpBlueUpperSpin->setDecimals(6);

   QLabel* pRgbUnitsLabel = new QLabel("Stretch Units:", pRgbWidget);
   mpRedUnitsCombo = new RegionUnitsComboBox(pRgbWidget);
   mpGreenUnitsCombo = new RegionUnitsComboBox(pRgbWidget);
   mpBlueUnitsCombo = new RegionUnitsComboBox(pRgbWidget);

   mpDisplayBandButton = new QPushButton("Presets", pRgbWidget);
   mpDisplayBandButton->setEnabled(false);
   QMenu* pMenu = new QMenu(mpDisplayBandButton);
   mpDisplayBandButton->setMenu(pMenu);

   const DynamicObject* pColorComposites = RasterLayer::getSettingColorComposites();
   if (pColorComposites != NULL)
   {
      vector<string> names;
      pColorComposites->getAttributeNames(names);
      if (names.empty() == false)
      {
         for (vector<string>::const_iterator iter = names.begin(); iter != names.end(); ++iter)
         {
            pMenu->addAction(QString::fromStdString(*iter));
         }

         mpDisplayBandButton->setEnabled(true);
      }
   }

   QLabel* pRgbStretchTypeLabel = new QLabel("Stretch Type:", pRgbWidget);
   mpRgbStretchTypeCombo = new StretchTypeComboBox(pRgbWidget);

   QPushButton* pRgbStretchFavoritesButton = new QPushButton("Favorites", pRgbWidget);
   mpRgbStretchMenu = new QMenu(pRgbStretchFavoritesButton);
   pRgbStretchFavoritesButton->setMenu(mpRgbStretchMenu);

   mpAddFavoriteRedAction = new QAction("Add Red Stretch to Favorites", pRgbWidget);
   mpAddFavoriteGreenAction = new QAction("Add Green Stretch to Favorites", pRgbWidget);
   mpAddFavoriteBlueAction = new QAction("Add Blue Stretch to Favorites", pRgbWidget);
   mRgbModifier.setModified(false);

   LabeledSection* pRgbSection = new LabeledSection(pRgbWidget, "RGB", this);

   QGridLayout* pRgbGrid = new QGridLayout(pRgbWidget);
   pRgbGrid->setMargin(0);
   pRgbGrid->setSpacing(5);
   pRgbGrid->addWidget(pRedLabel, 0, 1);
   pRgbGrid->addWidget(pGreenLabel, 0, 2);
   pRgbGrid->addWidget(pBlueLabel, 0, 3);
   pRgbGrid->addWidget(pRgbElementLabel, 1, 0);
   pRgbGrid->addWidget(mpRedElementCombo, 1, 1);
   pRgbGrid->addWidget(mpGreenElementCombo, 1, 2);
   pRgbGrid->addWidget(mpBlueElementCombo, 1, 3);
   pRgbGrid->addWidget(pRgbBandLabel, 2, 0);
   pRgbGrid->addWidget(mpRedBandCombo, 2, 1);
   pRgbGrid->addWidget(mpGreenBandCombo, 2, 2);
   pRgbGrid->addWidget(mpBlueBandCombo, 2, 3);
   pRgbGrid->addWidget(mpDisplayBandButton, 2, 4);
   pRgbGrid->addWidget(pRgbLowerLabel, 3, 0);
   pRgbGrid->addWidget(mpRedLowerSpin, 3, 1, Qt::AlignLeft);
   pRgbGrid->addWidget(mpGreenLowerSpin, 3, 2, Qt::AlignLeft);
   pRgbGrid->addWidget(mpBlueLowerSpin, 3, 3, Qt::AlignLeft);
   pRgbGrid->addWidget(pRgbStretchFavoritesButton, 3, 4);
   pRgbGrid->addWidget(pRgbUpperLabel, 4, 0);
   pRgbGrid->addWidget(mpRedUpperSpin, 4, 1, Qt::AlignLeft);
   pRgbGrid->addWidget(mpGreenUpperSpin, 4, 2, Qt::AlignLeft);
   pRgbGrid->addWidget(mpBlueUpperSpin, 4, 3, Qt::AlignLeft);
   pRgbGrid->addWidget(pRgbUnitsLabel, 5, 0);
   pRgbGrid->addWidget(mpRedUnitsCombo, 5, 1, Qt::AlignLeft);
   pRgbGrid->addWidget(mpGreenUnitsCombo, 5, 2, Qt::AlignLeft);
   pRgbGrid->addWidget(mpBlueUnitsCombo, 5, 3, Qt::AlignLeft);
   pRgbGrid->addWidget(pRgbStretchTypeLabel, 6, 0);
   pRgbGrid->addWidget(mpRgbStretchTypeCombo, 6, 1, Qt::AlignLeft);
   pRgbGrid->setRowStretch(7, 10);
   pRgbGrid->setColumnStretch(1, 10);
   pRgbGrid->setColumnStretch(2, 10);
   pRgbGrid->setColumnStretch(3, 10);

   // Graphics acceleration
   QLabel* pUnsupportedLabel = new QLabel("Dynamic texture generation is not supported on this system.", this);
   pUnsupportedLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

   QWidget* pAccelerationWidget = new QWidget(this);
   mpAccelerationCheck = new QCheckBox("Enable Dynamic Generation", pAccelerationWidget);
   mpFilterCheck = new QCheckBox("Filter:", pAccelerationWidget);
   mpFilterList = new QListWidget(pDisplayWidget);
   mpFilterList->setSelectionMode(QAbstractItemView::ExtendedSelection);

   QStringList filterNames;
   Service<ImageFilterManager> pFilterManager;

   vector<string> filters = pFilterManager->getAvailableFilters();
   for (vector<string>::iterator iter = filters.begin(); iter != filters.end(); ++iter)
   {
      string filterName = *iter;
      if (filterName.empty() == false)
      {
         filterNames.append(QString::fromStdString(filterName));
      }
   }

   filterNames.sort();
   mpFilterList->addItems(filterNames);
   mGraphicsAccModifier.setModified(false);

   QStackedWidget* pAccelerationStack = new QStackedWidget(this);
   LabeledSection* pAccelerationSection = new LabeledSection(pAccelerationStack,
      "Dynamic Texture Generation", this);

   QGridLayout* pAccelerationGrid = new QGridLayout(pAccelerationWidget);
   pAccelerationGrid->setMargin(0);
   pAccelerationGrid->setSpacing(5);
   pAccelerationGrid->addWidget(mpAccelerationCheck, 0, 0, 1, 2);
   pAccelerationGrid->addWidget(mpFilterCheck, 1, 0, Qt::AlignTop);
   pAccelerationGrid->addWidget(mpFilterList, 1, 1, Qt::AlignLeft);
   pAccelerationGrid->setRowStretch(1, 10);
   pAccelerationGrid->setColumnStretch(1, 10);

   bool bUnsupported = true;
#if defined (CG_SUPPORTED)
   if (CgContext::instance() != NULL)
   {
      bUnsupported = false;
   }
#endif

   if (bUnsupported == true)
   {
      pAccelerationStack->addWidget(pUnsupportedLabel);
      pAccelerationWidget->hide();
   }
   else
   {
      pAccelerationStack->addWidget(pAccelerationWidget);
      pUnsupportedLabel->hide();
   }

   // Initialization
   addSection(pDisplaySection);
   addSection(pGrayscaleSection);
   addSection(pRgbSection);
   addSection(pAccelerationSection, 10);
   setSizeHint(600, 610);

   // Connections
   VERIFYNR(mDisplayConfigModifier.attachSignal(mpDisplayModeCombo, SIGNAL(currentIndexChanged(int))));
   VERIFYNR(connect(mpGrayElementCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDisplayedBandCombo(int))));
   VERIFYNR(mGrayscaleModifier.attachSignal(mpGrayElementCombo, SIGNAL(currentIndexChanged(int))));
   VERIFYNR(connect(mpRedElementCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDisplayedBandCombo(int))));
   VERIFYNR(mRgbModifier.attachSignal(mpRedElementCombo, SIGNAL(currentIndexChanged(int))));
   VERIFYNR(connect(mpGreenElementCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDisplayedBandCombo(int))));
   VERIFYNR(mRgbModifier.attachSignal(mpGreenElementCombo, SIGNAL(currentIndexChanged(int))));
   VERIFYNR(connect(mpBlueElementCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDisplayedBandCombo(int))));
   VERIFYNR(mRgbModifier.attachSignal(mpBlueElementCombo, SIGNAL(currentIndexChanged(int))));
   VERIFYNR(connect(mpGrayBandCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateStretchValuesFromBand())));
   VERIFYNR(mGrayscaleModifier.attachSignal(mpGrayBandCombo, SIGNAL(currentIndexChanged(int))));
   VERIFYNR(connect(mpRedBandCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateStretchValuesFromBand())));
   VERIFYNR(mRgbModifier.attachSignal(mpRedBandCombo, SIGNAL(currentIndexChanged(int))));
   VERIFYNR(connect(mpGreenBandCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateStretchValuesFromBand())));
   VERIFYNR(mRgbModifier.attachSignal(mpGreenBandCombo, SIGNAL(currentIndexChanged(int))));
   VERIFYNR(connect(mpBlueBandCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateStretchValuesFromBand())));
   VERIFYNR(mRgbModifier.attachSignal(mpBlueBandCombo, SIGNAL(currentIndexChanged(int))));
   VERIFYNR(connect(mpGrayUnitsCombo, SIGNAL(valueChanged(RegionUnits)), this,
      SLOT(updateStretchValuesFromUnits(RegionUnits))));
   VERIFYNR(mGrayscaleModifier.attachSignal(mpGrayUnitsCombo, SIGNAL(valueChanged(RegionUnits))));
   VERIFYNR(connect(mpRedUnitsCombo, SIGNAL(valueChanged(RegionUnits)), this,
      SLOT(updateStretchValuesFromUnits(RegionUnits))));
   VERIFYNR(mRgbModifier.attachSignal(mpRedUnitsCombo, SIGNAL(valueChanged(RegionUnits))));
   VERIFYNR(connect(mpGreenUnitsCombo, SIGNAL(valueChanged(RegionUnits)), this,
      SLOT(updateStretchValuesFromUnits(RegionUnits))));
   VERIFYNR(mRgbModifier.attachSignal(mpGreenUnitsCombo, SIGNAL(valueChanged(RegionUnits))));
   VERIFYNR(connect(mpBlueUnitsCombo, SIGNAL(valueChanged(RegionUnits)), this,
      SLOT(updateStretchValuesFromUnits(RegionUnits))));
   VERIFYNR(mRgbModifier.attachSignal(mpBlueUnitsCombo, SIGNAL(valueChanged(RegionUnits))));
   VERIFYNR(connect(pMenu, SIGNAL(triggered(QAction*)), this, SLOT(setDisplayBands(QAction*))));
   VERIFYNR(connect(mpGrayStretchMenu, SIGNAL(aboutToShow()), this, SLOT(initializeStretchMenu())));
   VERIFYNR(connect(mpGrayStretchMenu, SIGNAL(triggered(QAction*)), this, SLOT(setGrayStretch(QAction*))));
   VERIFYNR(connect(mpRgbStretchMenu, SIGNAL(aboutToShow()), this, SLOT(initializeStretchMenu())));
   VERIFYNR(connect(mpRgbStretchMenu, SIGNAL(triggered(QAction*)), this, SLOT(setRgbStretch(QAction*))));
   VERIFYNR(connect(mpRemoveFavoriteAction, SIGNAL(triggered()), this, SLOT(removeStretchFavorite())));
   VERIFYNR(connect(mpAccelerationCheck, SIGNAL(toggled(bool)), this, SLOT(enableFilterCheck(bool))));
   VERIFYNR(mGraphicsAccModifier.attachSignal(mpAccelerationCheck, SIGNAL(toggled(bool))));
   VERIFYNR(connect(mpFilterCheck, SIGNAL(toggled(bool)), this, SLOT(enableFilterCombo(bool))));
   VERIFYNR(mGraphicsAccModifier.attachSignal(mpFilterCheck, SIGNAL(toggled(bool))));
   VERIFYNR(mGraphicsAccModifier.attachSignal(mpFilterList, SIGNAL(itemSelectionChanged())));
}

PropertiesRasterLayer::~PropertiesRasterLayer()
{}

bool PropertiesRasterLayer::initialize(SessionItem* pSessionItem)
{
   mpRasterLayer = dynamic_cast<RasterLayer*>(pSessionItem);
   if (mpRasterLayer == NULL)
   {
      return false;
   }

   mInitializing = true;

   // Populate the raster element combos with raster elements of the same size as the layer element
   mRasterElements.clear();
   QStringList elementNames;

   RasterElement* pRasterElement = dynamic_cast<RasterElement*>(mpRasterLayer->getDataElement());
   if (pRasterElement != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         unsigned int rows = pDescriptor->getRowCount();
         unsigned int columns = pDescriptor->getColumnCount();

         Service<ModelServices> pModel;
         vector<DataElement*> rasterElements = pModel->getElements("RasterElement");

         for (unsigned int i = 0; i < rasterElements.size(); ++i)
         {
            RasterElement* pCurrentRasterElement = dynamic_cast<RasterElement*>(rasterElements[i]);
            if (pCurrentRasterElement != NULL)
            {
               unsigned int currentRows = 0;
               unsigned int currentColumns = 0;

               const RasterDataDescriptor* pCurrentDescriptor =
                  dynamic_cast<const RasterDataDescriptor*>(pCurrentRasterElement->getDataDescriptor());
               if (pCurrentDescriptor != NULL)
               {
                  currentRows = pCurrentDescriptor->getRowCount();
                  currentColumns = pCurrentDescriptor->getColumnCount();
               }

               if ((currentRows == rows) && (currentColumns == columns))
               {
                  mRasterElements.push_back(pCurrentRasterElement);

                  string displayName = pCurrentRasterElement->getDisplayName();
                  if (displayName.empty() == true)
                  {
                     displayName = pCurrentRasterElement->getName();
                  }

                  if (displayName.empty() == false)
                  {
                     elementNames.append(QString::fromStdString(displayName));
                  }
               }
            }
         }
      }
   }

   mpGrayElementCombo->clear();
   mpGrayElementCombo->addItems(elementNames);

   mpRedElementCombo->clear();
   mpRedElementCombo->addItems(elementNames);

   mpGreenElementCombo->clear();
   mpGreenElementCombo->addItems(elementNames);

   mpBlueElementCombo->clear();
   mpBlueElementCombo->addItems(elementNames);

   // Display configuration
   DisplayMode displayMode = mpRasterLayer->getDisplayMode();
   if (displayMode == GRAYSCALE_MODE)
   {
      mpDisplayModeCombo->setCurrentIndex(0);
   }
   else if (displayMode == RGB_MODE)
   {
      mpDisplayModeCombo->setCurrentIndex(1);
   }

   mpComplexComponentCombo->setCurrentValue(mpRasterLayer->getComplexComponent());
   mpOpacitySpin->setValue(static_cast<int>(mpRasterLayer->getAlpha()));

   // Grayscale
   int grayIndex = getElementIndex(mpRasterLayer->getDisplayedRasterElement(GRAY));
   mpGrayElementCombo->setCurrentIndex(grayIndex);

   double dGrayLower = 0.0;
   double dGrayUpper = 0.0;
   mpRasterLayer->getStretchValues(GRAY, dGrayLower, dGrayUpper);

   mpGrayLowerSpin->setValue(dGrayLower);
   mpGrayUpperSpin->setValue(dGrayUpper);
   mGrayUnits = mpRasterLayer->getStretchUnits(GRAY);
   mpGrayUnitsCombo->setCurrentValue(mGrayUnits);
   mpGrayStretchTypeCombo->setCurrentValue(mpRasterLayer->getStretchType(GRAYSCALE_MODE));

   // RGB
   int redIndex = getElementIndex(mpRasterLayer->getDisplayedRasterElement(RED));
   mpRedElementCombo->setCurrentIndex(redIndex);

   int greenIndex = getElementIndex(mpRasterLayer->getDisplayedRasterElement(GREEN));
   mpGreenElementCombo->setCurrentIndex(greenIndex);

   int blueIndex = getElementIndex(mpRasterLayer->getDisplayedRasterElement(BLUE));
   mpBlueElementCombo->setCurrentIndex(blueIndex);

   double dRedLower = 0.0;
   double dRedUpper = 0.0;
   double dGreenLower = 0.0;
   double dGreenUpper = 0.0;
   double dBlueLower = 0.0;
   double dBlueUpper = 0.0;
   mpRasterLayer->getStretchValues(RED, dRedLower, dRedUpper);
   mpRasterLayer->getStretchValues(GREEN, dGreenLower, dGreenUpper);
   mpRasterLayer->getStretchValues(BLUE, dBlueLower, dBlueUpper);

   mpRedLowerSpin->setValue(dRedLower);
   mpRedUpperSpin->setValue(dRedUpper);
   mpGreenLowerSpin->setValue(dGreenLower);
   mpGreenUpperSpin->setValue(dGreenUpper);
   mpBlueLowerSpin->setValue(dBlueLower);
   mpBlueUpperSpin->setValue(dBlueUpper);

   mRedUnits = mpRasterLayer->getStretchUnits(RED);
   mGreenUnits = mpRasterLayer->getStretchUnits(GREEN);
   mBlueUnits = mpRasterLayer->getStretchUnits(BLUE);
   mpRedUnitsCombo->setCurrentValue(mRedUnits);
   mpGreenUnitsCombo->setCurrentValue(mGreenUnits);
   mpBlueUnitsCombo->setCurrentValue(mBlueUnits);
   mpRgbStretchTypeCombo->setCurrentValue(mpRasterLayer->getStretchType(RGB_MODE));

   // Graphics acceleration
   mpAccelerationCheck->setChecked(mpRasterLayer->isGpuImageEnabled());
   mpFilterCheck->setEnabled(mpRasterLayer->isGpuImageEnabled());
   mpFilterList->setEnabled(false);
   mpFilterList->clearSelection();

   if (mpFilterCheck->isEnabled())
   {
      vector<string> filters = mpRasterLayer->getEnabledFilterNames();
      mpFilterCheck->setChecked(!(filters.empty()));
      mpFilterList->setEnabled(!(filters.empty()));

      for (vector<string>::iterator iter = filters.begin(); iter != filters.end(); ++iter)
      {
         QString filterName = QString::fromStdString(*iter);
         if (filterName.isEmpty() == false)
         {
            QList<QListWidgetItem*> filterItems = mpFilterList->findItems(filterName, Qt::MatchExactly);
            for (int i = 0; i < filterItems.count(); ++i)
            {
               QListWidgetItem* pItem = filterItems[i];
               if (pItem != NULL)
               {
                  pItem->setSelected(true);
               }
            }
         }
      }
   }

   mInitializing = false;
   return true;
}

bool PropertiesRasterLayer::applyChanges()
{
   if (mpRasterLayer == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpRasterLayer->getView(), actionText);

   // Display configuration
   if (mDisplayConfigModifier.isModified())
   {
      DisplayMode displayMode =
         StringUtilities::fromDisplayString<DisplayMode>(mpDisplayModeCombo->currentText().toStdString());

      mpRasterLayer->setDisplayMode(displayMode);
      mpRasterLayer->setComplexComponent(mpComplexComponentCombo->getCurrentValue());
      mpRasterLayer->setAlpha(static_cast<unsigned int>(mpOpacitySpin->value()));
   }

   // Grayscale
   if (mGrayscaleModifier.isModified())
   {
      RasterElement* pGrayElement = NULL;
      DimensionDescriptor grayBand = getSelectedBand(GRAY, pGrayElement);
      mpRasterLayer->setStretchType(GRAYSCALE_MODE, mpGrayStretchTypeCombo->getCurrentValue());
      mpRasterLayer->setDisplayedBand(GRAY, grayBand, pGrayElement);
      mpRasterLayer->setStretchUnits(GRAY, mpGrayUnitsCombo->getCurrentValue());
      mpRasterLayer->setStretchValues(GRAY, mpGrayLowerSpin->value(), mpGrayUpperSpin->value());
   }

   // RGB
   if (mRgbModifier.isModified())
   {
      mpRasterLayer->setStretchType(RGB_MODE, mpRgbStretchTypeCombo->getCurrentValue());

      RasterElement* pRedElement = NULL;
      DimensionDescriptor redBand = getSelectedBand(RED, pRedElement);
      mpRasterLayer->setDisplayedBand(RED, redBand, pRedElement);
      mpRasterLayer->setStretchUnits(RED, mpRedUnitsCombo->getCurrentValue());
      mpRasterLayer->setStretchValues(RED, mpRedLowerSpin->value(), mpRedUpperSpin->value());

      RasterElement* pGreenElement = NULL;
      DimensionDescriptor greenBand = getSelectedBand(GREEN, pGreenElement);
      mpRasterLayer->setDisplayedBand(GREEN, greenBand, pGreenElement);
      mpRasterLayer->setStretchUnits(GREEN, mpGreenUnitsCombo->getCurrentValue());
      mpRasterLayer->setStretchValues(GREEN, mpGreenLowerSpin->value(), mpGreenUpperSpin->value());

      RasterElement* pBlueElement = NULL;
      DimensionDescriptor blueBand = getSelectedBand(BLUE, pBlueElement);
      mpRasterLayer->setDisplayedBand(BLUE, blueBand, pBlueElement);
      mpRasterLayer->setStretchUnits(BLUE, mpBlueUnitsCombo->getCurrentValue());
      mpRasterLayer->setStretchValues(BLUE, mpBlueLowerSpin->value(), mpBlueUpperSpin->value());
   }

   // Graphics acceleration
   if (mGraphicsAccModifier.isModified())
   {
      mpRasterLayer->enableGpuImage(mpAccelerationCheck->isChecked());

      vector<string> filterNames;
      if (mpFilterCheck->isChecked() == true)
      {
         QList<QListWidgetItem*> selectedFilters = mpFilterList->selectedItems();
         for (int i = 0; i < selectedFilters.count(); ++i)
         {
            QListWidgetItem* pItem = selectedFilters[i];
            if (pItem != NULL)
            {
               QString filterName = pItem->text();
               if (filterName.isEmpty() == false)
               {
                  filterNames.push_back(filterName.toStdString());
               }
            }
         }
      }

      mpRasterLayer->enableFilters(filterNames);
   }

   // Refresh the view
   View* pView = mpRasterLayer->getView();
   if (pView != NULL)
   {
      pView->refresh();
   }

   return true;
}

const string& PropertiesRasterLayer::getName()
{
   static string name = "Raster Layer Properties";
   return name;
}

const string& PropertiesRasterLayer::getPropertiesName()
{
   static string propertiesName = "Raster Layer";
   return propertiesName;
}

const string& PropertiesRasterLayer::getDescription()
{
   static string description = "General setting properties of a raster layer";
   return description;
}

const string& PropertiesRasterLayer::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesRasterLayer::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesRasterLayer::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesRasterLayer::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesRasterLayer::getDescriptorId()
{
   static string id = "{CB48D179-498C-482F-8FF6-47931DCA861A}";
   return id;
}

const string& PropertiesRasterLayer::getFilterWarningDialogId()
{
   static string filterWarningDialog = "{98E390BE-2D58-4e89-9DFE-506412BD59C1}";
   return filterWarningDialog;
}

const string& PropertiesRasterLayer::getDisplayAsWarningDialogId()
{
   static string displayAsWarningDialog = "{528FD510-D05B-46ef-9F48-3BEBD20A5F7D}";
   return displayAsWarningDialog;
}

bool PropertiesRasterLayer::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}

int PropertiesRasterLayer::getElementIndex(RasterElement* pRasterElement) const
{
   for (unsigned int i = 0; i < mRasterElements.size(); ++i)
   {
      RasterElement* pCurrentRasterElement = mRasterElements[i];
      if (pCurrentRasterElement == pRasterElement)
      {
         return static_cast<int>(i);
      }
   }

   return -1;
}

void PropertiesRasterLayer::setStretchUnits(RasterChannelType channel, RegionUnits newUnits)
{
   // Get current values based on the channel
   RegionUnits currentUnits = getStretchUnits(channel);
   QDoubleSpinBox* pLowerSpin = NULL;
   QDoubleSpinBox* pUpperSpin = NULL;

   switch (channel)
   {
      case GRAY:
         mGrayUnits = newUnits;
         pLowerSpin = mpGrayLowerSpin;
         pUpperSpin = mpGrayUpperSpin;
         break;

      case RED:
         mRedUnits = newUnits;
         pLowerSpin = mpRedLowerSpin;
         pUpperSpin = mpRedUpperSpin;
         break;

      case GREEN:
         mGreenUnits = newUnits;
         pLowerSpin = mpGreenLowerSpin;
         pUpperSpin = mpGreenUpperSpin;
         break;

      case BLUE:
         mBlueUnits = newUnits;
         pLowerSpin = mpBlueLowerSpin;
         pUpperSpin = mpBlueUpperSpin;
         break;

      default:
         break;
   }

   if ((pLowerSpin == NULL) || (pUpperSpin == NULL) || (currentUnits == newUnits))
   {
      return;
   }

   double dLower = pLowerSpin->value();
   double dUpper = pUpperSpin->value();

   // Convert the stretch values to the new method
   double dNewLower = dLower;
   double dNewUpper = dUpper;

   if (mpRasterLayer != NULL)
   {
      // Update the cube layer to display the currently selected bands
      // so that the stretch values will be converted properly.
      // Block signals so observers ignore this temporary set and reset.
      UndoLock lock(mpRasterLayer->getView());
      SignalBlocker sigBlock(*dynamic_cast<Subject*>(mpRasterLayer));
      RasterLayerImp* pLayerImp = dynamic_cast<RasterLayerImp*>(mpRasterLayer);
      if (pLayerImp != NULL)
      {
         pLayerImp->blockSignals(true);
      }

      RasterElement* pRasterElement = mpRasterLayer->getDisplayedRasterElement(channel);
      DimensionDescriptor band = mpRasterLayer->getDisplayedBand(channel);

      RasterElement* pTempRasterElement = NULL;
      DimensionDescriptor tempBand = getSelectedBand(channel, pTempRasterElement);
      mpRasterLayer->setDisplayedBand(channel, tempBand, pTempRasterElement);

      // Convert the stretch values
      dNewLower = mpRasterLayer->convertStretchValue(channel, currentUnits, dLower, newUnits);
      dNewUpper = mpRasterLayer->convertStretchValue(channel, currentUnits, dUpper, newUnits);

      // Reset the cube layer to the actual displayed bands and remove block on Qt signals
      mpRasterLayer->setDisplayedBand(channel, band, pRasterElement);
      if (pLayerImp != NULL)
      {
         pLayerImp->blockSignals(false);
      }
   }

   // Display the converted values
   pLowerSpin->setValue(dNewLower);
   pUpperSpin->setValue(dNewUpper);
}

RegionUnits PropertiesRasterLayer::getStretchUnits(RasterChannelType channel) const
{
   RegionUnits units;
   switch (channel)
   {
      case GRAY:
         units = mGrayUnits;
         break;

      case RED:
         units = mRedUnits;
         break;

      case GREEN:
         units = mGreenUnits;
         break;

      case BLUE:
         units = mBlueUnits;
         break;

      default:
         break;
   }

   return units;
}

DimensionDescriptor PropertiesRasterLayer::getSelectedBand(RasterChannelType channel,
                                                           RasterElement*& pRasterElementOut) const
{
   pRasterElementOut = NULL;

   DimensionDescriptor band;
   int elementIndex = -1;
   int bandIndex = -1;

   switch (channel)
   {
      case GRAY:
         elementIndex = mpGrayElementCombo->currentIndex();
         bandIndex = mpGrayBandCombo->currentIndex();
         break;

      case RED:
         elementIndex = mpRedElementCombo->currentIndex();
         bandIndex = mpRedBandCombo->currentIndex();
         break;

      case GREEN:
         elementIndex = mpGreenElementCombo->currentIndex();
         bandIndex = mpGreenBandCombo->currentIndex();
         break;

      case BLUE:
         elementIndex = mpBlueElementCombo->currentIndex();
         bandIndex = mpBlueBandCombo->currentIndex();
         break;

      default:
         break;
   }

   if ((elementIndex > -1) && (static_cast<int>(mRasterElements.size()) > elementIndex))
   {
      pRasterElementOut = mRasterElements[elementIndex];
      if (pRasterElementOut != NULL)
      {
         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pRasterElementOut->getDataDescriptor());
         if ((pDescriptor != NULL) && (bandIndex >= 0))
         {
            band = pDescriptor->getActiveBand(bandIndex);
         }
      }
   }

   return band;
}

void PropertiesRasterLayer::updateDisplayedBandCombo(int index)
{
   // Get the selected raster element
   RasterElement* pElement = NULL;
   if ((index > -1) && (static_cast<int>(mRasterElements.size()) > index))
   {
      pElement = mRasterElements[index];
   }

   // Get the band names from the element
   QStringList strBandNames;
   if (pElement != NULL)
   {
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         vector<string> bandNames = RasterUtilities::getBandNames(pDescriptor);
         for (vector<string>::iterator iter = bandNames.begin(); iter != bandNames.end(); ++iter)
         {
            strBandNames.append(QString::fromStdString(*iter));
         }
      }
   }

   // Update the display band combo
   QComboBox* pCombo = dynamic_cast<QComboBox*>(sender());
   if (pCombo == mpGrayElementCombo)
   {
      mpGrayBandCombo->clear();
      mpGrayBandCombo->addItems(strBandNames);

      if (strBandNames.isEmpty() == false)
      {
         mpGrayBandCombo->setCurrentIndex(0);
         if ((mpRasterLayer != NULL) && (mpRasterLayer->getDataElement() == pElement))
         {
            DimensionDescriptor displayedBand = mpRasterLayer->getDisplayedBand(GRAY);
            if (displayedBand.isActiveNumberValid() == true)
            {
               mpGrayBandCombo->setCurrentIndex(displayedBand.getActiveNumber());
            }
         }
      }
   }
   else if (pCombo == mpRedElementCombo)
   {
      mpRedBandCombo->clear();
      mpRedBandCombo->addItems(strBandNames);

      if (strBandNames.isEmpty() == false)
      {
         mpRedBandCombo->setCurrentIndex(0);
         if ((mpRasterLayer != NULL) && (mpRasterLayer->getDataElement() == pElement))
         {
            DimensionDescriptor displayedBand = mpRasterLayer->getDisplayedBand(RED);
            if (displayedBand.isActiveNumberValid() == true)
            {
               mpRedBandCombo->setCurrentIndex(displayedBand.getActiveNumber());
            }
         }
      }
   }
   else if (pCombo == mpGreenElementCombo)
   {
      mpGreenBandCombo->clear();
      mpGreenBandCombo->addItems(strBandNames);

      if (strBandNames.isEmpty() == false)
      {
         mpGreenBandCombo->setCurrentIndex(0);
         if ((mpRasterLayer != NULL) && (mpRasterLayer->getDataElement() == pElement))
         {
            DimensionDescriptor displayedBand = mpRasterLayer->getDisplayedBand(GREEN);
            if (displayedBand.isActiveNumberValid() == true)
            {
               mpGreenBandCombo->setCurrentIndex(displayedBand.getActiveNumber());
            }
         }
      }
   }
   else if (pCombo == mpBlueElementCombo)
   {
      mpBlueBandCombo->clear();
      mpBlueBandCombo->addItems(strBandNames);

      if (strBandNames.isEmpty() == false)
      {
         mpBlueBandCombo->setCurrentIndex(0);
         if ((mpRasterLayer != NULL) && (mpRasterLayer->getDataElement() == pElement))
         {
            DimensionDescriptor displayedBand = mpRasterLayer->getDisplayedBand(BLUE);
            if (displayedBand.isActiveNumberValid() == true)
            {
               mpBlueBandCombo->setCurrentIndex(displayedBand.getActiveNumber());
            }
         }
      }
   }
}

void PropertiesRasterLayer::updateStretchValuesFromBand()
{
   RasterChannelType channel;

   // Get the current units and the default stretch values
   RegionUnits currentUnits;
   RegionUnits defaultUnits;
   double dDefaultLower = 0.0;
   double dDefaultUpper = 0.0;
   QDoubleSpinBox* pLowerSpin = NULL;
   QDoubleSpinBox* pUpperSpin = NULL;

   QComboBox* pCombo = dynamic_cast<QComboBox*>(sender());
   if (pCombo == mpGrayBandCombo)
   {
      channel = GRAY;
      defaultUnits = RasterLayer::getSettingGrayscaleStretchUnits();
      pLowerSpin = mpGrayLowerSpin;
      pUpperSpin = mpGrayUpperSpin;
   }
   else if (pCombo == mpRedBandCombo)
   {
      channel = RED;
      defaultUnits = RasterLayer::getSettingRedStretchUnits();
      pLowerSpin = mpRedLowerSpin;
      pUpperSpin = mpRedUpperSpin;
   }
   else if (pCombo == mpGreenBandCombo)
   {
      channel = GREEN;
      defaultUnits = RasterLayer::getSettingGreenStretchUnits();
      pLowerSpin = mpGreenLowerSpin;
      pUpperSpin = mpGreenUpperSpin;
   }
   else if (pCombo == mpBlueBandCombo)
   {
      channel = BLUE;
      defaultUnits = RasterLayer::getSettingBlueStretchUnits();
      pLowerSpin = mpBlueLowerSpin;
      pUpperSpin = mpBlueUpperSpin;
   }

   if ((pLowerSpin == NULL) || (pUpperSpin == NULL))
   {
      return;
   }

   currentUnits = getStretchUnits(channel);
   RasterLayerImp::getDefaultStretchValues(channel, dDefaultLower, dDefaultUpper);

   // Set the current stretch units to the default units
   setStretchUnits(channel, defaultUnits);

   // Set the current stretch values to the default values
   pLowerSpin->setValue(dDefaultLower);
   pUpperSpin->setValue(dDefaultUpper);

   // Set the current stretch units back to the previous units to update the stretch values
   setStretchUnits(channel, currentUnits);
}

void PropertiesRasterLayer::updateStretchValuesFromUnits(RegionUnits units)
{
   RasterChannelType channel;

   QComboBox* pCombo = dynamic_cast<QComboBox*>(sender());
   if (pCombo == mpGrayUnitsCombo)
   {
      channel = GRAY;
   }
   else if (pCombo == mpRedUnitsCombo)
   {
      channel = RED;
   }
   else if (pCombo == mpGreenUnitsCombo)
   {
      channel = GREEN;
   }
   else if (pCombo == mpBlueUnitsCombo)
   {
      channel = BLUE;
   }

   if (channel.isValid() == true)
   {
      setStretchUnits(channel, units);
   }
}

void PropertiesRasterLayer::enableFilterCheck(bool bEnable)
{
   if (bEnable == false)
   {
      mpFilterCheck->setChecked(false);
   }

   mpFilterCheck->setEnabled(bEnable);
}

void PropertiesRasterLayer::enableFilterCombo(bool bEnable)
{
   if ((mInitializing == false) && (bEnable == true))
   {
      Service<DesktopServices>()->showSuppressibleMsgDlg(APP_NAME,  "<b>This is an EXPERIMENTAL feature!</b><br/>"
         "Enabling image filtering uses additional system resources when generating the image for display.<br/>"
         "For large data sets it is possible for the system to run out of resources, which could have adverse "
         "effects including application shutdown or system reboot.", MESSAGE_WARNING, 
         getFilterWarningDialogId(), this);
   }
   else if (bEnable == false)
   {
      mpFilterList->clearSelection();
   }

   mpFilterList->setEnabled(bEnable);
}

void PropertiesRasterLayer::setDisplayBands(QAction* pAction)
{
   if (mpRasterLayer == NULL)
   {
      return;
   }

   RasterElement* pRasterElement = dynamic_cast<RasterElement*>(mpRasterLayer->getDataElement());
   if (pRasterElement == NULL)
   {
      return;
   }

   const RasterDataDescriptor* pDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   const std::string name = pAction->text().toStdString();
   DimensionDescriptor redBand;
   DimensionDescriptor greenBand;
   DimensionDescriptor blueBand;
   if (RasterUtilities::findColorCompositeDimensionDescriptors(
      pDescriptor, name, redBand, greenBand, blueBand) == false)
   {
      Service<DesktopServices>()->showSuppressibleMsgDlg("Error",
         "Unable to display " + name + ": required wavelengths do not exist for all bands. "
         "Broaden the wavelength region or specify band numbers in the Raster Layers section of the Options dialog.",
         MESSAGE_ERROR, PropertiesRasterLayer::getDisplayAsWarningDialogId());
   }

   // If at least one of red, green, or blue is valid set display mode to RGB and update the combo boxes appropriately
   if (redBand.isActiveNumberValid() || greenBand.isActiveNumberValid() || blueBand.isActiveNumberValid())
   {
      mpDisplayModeCombo->setCurrentIndex(1);
      mpRedBandCombo->setCurrentIndex(redBand.isActiveNumberValid() ? redBand.getActiveNumber() : -1);
      mpGreenBandCombo->setCurrentIndex(greenBand.isActiveNumberValid() ? greenBand.getActiveNumber() : -1);
      mpBlueBandCombo->setCurrentIndex(blueBand.isActiveNumberValid() ? blueBand.getActiveNumber() : -1);
   }
}

void PropertiesRasterLayer::initializeStretchMenu()
{
   QMenu* pMenu = dynamic_cast<QMenu*>(sender());
   VERIFYNRV(pMenu != NULL);

   pMenu->clear();

   const DynamicObject* pStretchFavorites = RasterLayer::getSettingStretchFavorites();
   if (pStretchFavorites != NULL)
   {
      vector<string> names;
      pStretchFavorites->getAttributeNames(names);
      if (names.empty() == false)
      {
         for (vector<string>::const_iterator iter = names.begin(); iter != names.end(); ++iter)
         {
            pMenu->addAction(QString::fromStdString(*iter));
         }

         pMenu->addSeparator();
      }
   }

   if (pMenu == mpGrayStretchMenu)
   {
      pMenu->addAction(mpAddFavoriteGrayAction);
   }
   else if (pMenu == mpRgbStretchMenu)
   {
      pMenu->addAction(mpAddFavoriteRedAction);
      pMenu->addAction(mpAddFavoriteGreenAction);
      pMenu->addAction(mpAddFavoriteBlueAction);
   }

   pMenu->addAction(mpRemoveFavoriteAction);
}

void PropertiesRasterLayer::setGrayStretch(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   if (pAction == mpAddFavoriteGrayAction)
   {
      double lower = mpGrayLowerSpin->value();
      double upper = mpGrayUpperSpin->value();
      RegionUnits units = mpGrayUnitsCombo->getCurrentValue();
      StretchType type = mpGrayStretchTypeCombo->getCurrentValue();

      RasterLayerImp::addStretchFavorite(lower, upper, units, type);
      return;
   }
   else if (pAction == mpRemoveFavoriteAction)
   {
      return;
   }

   QString name = pAction->text();
   if (name.isEmpty() == true)
   {
      return;
   }

   double lower = 0.0;
   double upper = 0.0;
   RegionUnits units;
   StretchType type;
   if (RasterLayerImp::getStretchFavorite(name, lower, upper, units, type) == false)
   {
      return;
   }

   mpGrayLowerSpin->setValue(lower);
   mpGrayUpperSpin->setValue(upper);
   mGrayUnits = units;
   mpGrayUnitsCombo->setCurrentValue(units);
   mpGrayStretchTypeCombo->setCurrentValue(type);
}

void PropertiesRasterLayer::setRgbStretch(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   if (pAction == mpAddFavoriteRedAction)
   {
      double lower = mpRedLowerSpin->value();
      double upper = mpRedUpperSpin->value();
      RegionUnits units = mpRedUnitsCombo->getCurrentValue();
      StretchType type = mpRgbStretchTypeCombo->getCurrentValue();

      RasterLayerImp::addStretchFavorite(lower, upper, units, type);
      return;
   }
   else if (pAction == mpAddFavoriteGreenAction)
   {
      double lower = mpGreenLowerSpin->value();
      double upper = mpGreenUpperSpin->value();
      RegionUnits units = mpGreenUnitsCombo->getCurrentValue();
      StretchType type = mpRgbStretchTypeCombo->getCurrentValue();

      RasterLayerImp::addStretchFavorite(lower, upper, units, type);
      return;
   }
   else if (pAction == mpAddFavoriteBlueAction)
   {
      double lower = mpBlueLowerSpin->value();
      double upper = mpBlueUpperSpin->value();
      RegionUnits units = mpBlueUnitsCombo->getCurrentValue();
      StretchType type = mpRgbStretchTypeCombo->getCurrentValue();

      RasterLayerImp::addStretchFavorite(lower, upper, units, type);
      return;
   }
   else if (pAction == mpRemoveFavoriteAction)
   {
      return;
   }

   QString name = pAction->text();
   if (name.isEmpty() == true)
   {
      return;
   }

   double lower = 0.0;
   double upper = 0.0;
   RegionUnits units;
   StretchType type;
   if (RasterLayerImp::getStretchFavorite(name, lower, upper, units, type) == false)
   {
      return;
   }

   mpRedLowerSpin->setValue(lower);
   mpRedUpperSpin->setValue(upper);
   mRedUnits = units;
   mpRedUnitsCombo->setCurrentValue(units);

   mpGreenLowerSpin->setValue(lower);
   mpGreenUpperSpin->setValue(upper);
   mGreenUnits = units;
   mpGreenUnitsCombo->setCurrentValue(units);

   mpBlueLowerSpin->setValue(lower);
   mpBlueUpperSpin->setValue(upper);
   mBlueUnits = units;
   mpBlueUnitsCombo->setCurrentValue(units);

   mpRgbStretchTypeCombo->setCurrentValue(type);
}

void PropertiesRasterLayer::removeStretchFavorite()
{
   RasterLayerImp::removeStretchFavorite();
}
