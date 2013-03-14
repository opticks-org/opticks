/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>

#include "AppVersion.h"
#include "CustomColorButton.h"
#include "LabeledSection.h"
#include "PassAreaComboBox.h"
#include "PropertiesThresholdLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "RegionUnitsComboBox.h"
#include "StretchTypeComboBox.h"
#include "SymbolTypeGrid.h"
#include "ThresholdLayer.h"
#include "Undo.h"
#include "View.h"

#include <limits>
using namespace std;

PropertiesThresholdLayer::PropertiesThresholdLayer() :
   LabeledSectionGroup(NULL),
   mpThresholdLayer(NULL)
{
   // Display
   QWidget* pDisplayWidget = new QWidget(this);
   QLabel* pBandLabel = new QLabel("Display band:", pDisplayWidget);
   mpDisplayBand = new QComboBox(pDisplayWidget);
   LabeledSection* pDisplaySection = new LabeledSection(pDisplayWidget, "Display", this);
   QGridLayout* pDisplayGrid = new QGridLayout(pDisplayWidget);
   pDisplayGrid->setMargin(0);
   pDisplayGrid->setSpacing(5);
   pDisplayGrid->addWidget(pBandLabel, 0, 0);
   pDisplayGrid->addWidget(mpDisplayBand, 0, 1);
   pDisplayGrid->setRowStretch(1, 10);
   pDisplayGrid->setColumnStretch(2, 10);

   // Pixel marker
   QWidget* pMarkerWidget = new QWidget(this);

   QLabel* pSymbolLabel = new QLabel("Symbol:", pMarkerWidget);
   mpSymbolButton = new SymbolTypeButton(pMarkerWidget);
   mpSymbolButton->setBorderedSymbols(true);

   QLabel* pColorLabel = new QLabel("Color:", pMarkerWidget);
   mpColorButton = new CustomColorButton(pMarkerWidget);
   mpColorButton->usePopupGrid(true);

   LabeledSection* pMarkerSection = new LabeledSection(pMarkerWidget, "Pixel Marker", this);

   QGridLayout* pMarkerGrid = new QGridLayout(pMarkerWidget);
   pMarkerGrid->setMargin(0);
   pMarkerGrid->setSpacing(5);
   pMarkerGrid->addWidget(pSymbolLabel, 0, 0);
   pMarkerGrid->addWidget(mpSymbolButton, 0, 1);
   pMarkerGrid->addWidget(pColorLabel, 1, 0);
   pMarkerGrid->addWidget(mpColorButton, 1, 1);
   pMarkerGrid->setRowStretch(2, 10);
   pMarkerGrid->setColumnStretch(2, 10);

   // Pass area
   QWidget* pPassAreaWidget = new QWidget(this);

   QLabel* pPassAreaLabel = new QLabel("Pass Area:", pPassAreaWidget);
   mpPassAreaCombo = new PassAreaComboBox(pPassAreaWidget);
   QLabel* pUnitsLabel = new QLabel("Pass Area Units:", pPassAreaWidget);
   mpUnitsCombo = new RegionUnitsComboBox(pPassAreaWidget);
   QLabel* pFirstValueLabel = new QLabel("First Value:", pPassAreaWidget);
   mpFirstValueSpin = new QDoubleSpinBox(pPassAreaWidget);
   mpFirstValueSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpFirstValueSpin->setDecimals(6);
   mpSecondValueLabel = new QLabel("Second Value:", pPassAreaWidget);
   mpSecondValueSpin = new QDoubleSpinBox(pPassAreaWidget);
   mpSecondValueSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpSecondValueSpin->setDecimals(6);

   LabeledSection* pPassAreaSection = new LabeledSection(pPassAreaWidget, "Pass Area", this);

   QGridLayout* pPassAreaGrid = new QGridLayout(pPassAreaWidget);
   pPassAreaGrid->setMargin(0);
   pPassAreaGrid->setSpacing(5);
   pPassAreaGrid->addWidget(pPassAreaLabel, 0, 0);
   pPassAreaGrid->addWidget(mpPassAreaCombo, 0, 1, Qt::AlignLeft);
   pPassAreaGrid->addWidget(pUnitsLabel, 1, 0);
   pPassAreaGrid->addWidget(mpUnitsCombo, 1, 1, Qt::AlignLeft);
   pPassAreaGrid->addWidget(pFirstValueLabel, 2, 0);
   pPassAreaGrid->addWidget(mpFirstValueSpin, 2, 1, Qt::AlignLeft);
   pPassAreaGrid->addWidget(mpSecondValueLabel, 3, 0);
   pPassAreaGrid->addWidget(mpSecondValueSpin, 3, 1, Qt::AlignLeft);
   pPassAreaGrid->setRowStretch(4, 10);
   pPassAreaGrid->setColumnStretch(1, 10);

   // Initialization
   addSection(pDisplaySection);
   addSection(pMarkerSection);
   addSection(pPassAreaSection);
   addStretch(10);
   setSizeHint(425, 300);

   // Connections
   VERIFYNR(connect(mpPassAreaCombo, SIGNAL(valueChanged(PassArea)), this, SLOT(setPassArea(PassArea))));
   VERIFYNR(connect(mpUnitsCombo, SIGNAL(valueChanged(RegionUnits)), this, SLOT(setRegionUnits(RegionUnits))));
   VERIFYNR(mDisplayModifier.attachSignal(mpDisplayBand, SIGNAL(currentIndexChanged(int))));
   VERIFYNR(mMarkerModifier.attachSignal(mpSymbolButton, SIGNAL(valueChanged(SymbolType))));
   VERIFYNR(mMarkerModifier.attachSignal(mpColorButton, SIGNAL(colorChanged(const QColor&))));
   VERIFYNR(mPassModifier.attachSignal(mpPassAreaCombo, SIGNAL(valueChanged(PassArea))));
   VERIFYNR(mPassModifier.attachSignal(mpUnitsCombo, SIGNAL(valueChanged(RegionUnits))));
   VERIFYNR(mPassModifier.attachSignal(mpFirstValueSpin, SIGNAL(valueChanged(double))));
   VERIFYNR(mPassModifier.attachSignal(mpSecondValueSpin, SIGNAL(valueChanged(double))));
}

PropertiesThresholdLayer::~PropertiesThresholdLayer()
{}

bool PropertiesThresholdLayer::initialize(SessionItem* pSessionItem)
{
   mpThresholdLayer = dynamic_cast<ThresholdLayer*>(pSessionItem);
   if (mpThresholdLayer == NULL)
   {
      return false;
   }

   // Display
   RasterElement* pElement = dynamic_cast<RasterElement*>(mpThresholdLayer->getDataElement());
   VERIFY(pElement != NULL);
   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
   VERIFY(pDescriptor != NULL);
   // Get the band names from the element
   QStringList strBandNames;
   vector<string> bandNames = RasterUtilities::getBandNames(pDescriptor);
   for (vector<string>::iterator iter = bandNames.begin(); iter != bandNames.end(); ++iter)
   {
      strBandNames.append(QString::fromStdString(*iter));
   }
   mpDisplayBand->addItems(strBandNames);
   DimensionDescriptor displayBand = mpThresholdLayer->getDisplayedBand();
   int index = displayBand.isActiveNumberValid() ? displayBand.getActiveNumber() : 0;
   mpDisplayBand->setCurrentIndex(index);

   // Pixel marker
   mpSymbolButton->setCurrentValue(mpThresholdLayer->getSymbol());
   mpColorButton->setColor(mpThresholdLayer->getColor());

   // Pass area
   mUnits = mpThresholdLayer->getRegionUnits();
   mpPassAreaCombo->setCurrentValue(mpThresholdLayer->getPassArea());
   mpUnitsCombo->setCurrentValue(mUnits);
   mpFirstValueSpin->setValue(mpThresholdLayer->convertThreshold(RAW_VALUE,
      mpThresholdLayer->getFirstThreshold(), mUnits));
   mpSecondValueSpin->setValue(mpThresholdLayer->convertThreshold(RAW_VALUE,
      mpThresholdLayer->getSecondThreshold(), mUnits));
   setPassArea(mpThresholdLayer->getPassArea());

   return true;
}

bool PropertiesThresholdLayer::applyChanges()
{
   if (mpThresholdLayer == NULL)
   {
      return false;
   }

   PassArea passArea = mpPassAreaCombo->getCurrentValue();
   double dFirstValue = mpFirstValueSpin->value();
   double dSecondValue = mpSecondValueSpin->value();

   if (((passArea == MIDDLE) || (passArea == OUTSIDE)) && (dFirstValue > dSecondValue))
   {
      QMessageBox::warning(this, APP_NAME, "The first threshold value is greater than "
         "the second value.\nPlease readjust the values.");
      return false;
   }

   int index = mpDisplayBand->currentIndex();
   if (index < 0)
   {
      QMessageBox::warning(this, APP_NAME, "Invalid band selection.\nPlease select the band to display.");
      return false;
   }

   bool refreshNeeded = (mDisplayModifier.isModified() || mMarkerModifier.isModified() || mPassModifier.isModified());
   string actionText = "Set " + getName();
   UndoGroup group(mpThresholdLayer->getView(), actionText);

   // Display
   if (mDisplayModifier.isModified())
   {
      RasterElement* pElement = dynamic_cast<RasterElement*>(mpThresholdLayer->getDataElement());
      VERIFY(pElement != NULL);
      const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
      VERIFY(pDescriptor != NULL);
      mpThresholdLayer->setDisplayedBand(pDescriptor->getActiveBand(index));
      mDisplayModifier.setModified(false);
   }

   // Pixel marker
   if (mMarkerModifier.isModified())
   {
      mpThresholdLayer->setSymbol(mpSymbolButton->getCurrentValue());
      mpThresholdLayer->setColor(QCOLOR_TO_COLORTYPE(mpColorButton->getColor()));
      mMarkerModifier.setModified(false);
   }

   // Pass area
   if (mPassModifier.isModified())
   {
      mpThresholdLayer->setPassArea(passArea);
      mpThresholdLayer->setRegionUnits(mpUnitsCombo->getCurrentValue());
      mpThresholdLayer->setFirstThreshold(mpThresholdLayer->convertThreshold(mUnits, dFirstValue, RAW_VALUE));
      mpThresholdLayer->setSecondThreshold(mpThresholdLayer->convertThreshold(mUnits, dSecondValue, RAW_VALUE));
      mPassModifier.setModified(false);
   }

   // Refresh the view
   if (refreshNeeded)
   {
      View* pView = mpThresholdLayer->getView();
      if (pView != NULL)
      {
         pView->refresh();
      }
   }

   return true;
}

const string& PropertiesThresholdLayer::getName()
{
   static string name = "Threshold Layer Properties";
   return name;
}

const string& PropertiesThresholdLayer::getPropertiesName()
{
   static string propertiesName = "Threshold Layer";
   return propertiesName;
}

const string& PropertiesThresholdLayer::getDescription()
{
   static string description = "General setting properties of a threshold layer";
   return description;
}

const string& PropertiesThresholdLayer::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesThresholdLayer::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesThresholdLayer::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesThresholdLayer::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesThresholdLayer::getDescriptorId()
{
   static string id = "{6201BA78-12C5-480F-AF16-00685A2EF5BD}";
   return id;
}

bool PropertiesThresholdLayer::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}

void PropertiesThresholdLayer::setPassArea(PassArea passArea)
{
   // Show and hide the second threshold value as necessary
   switch (passArea)
   {
      case LOWER:    // Fall through
      case UPPER:
         mpSecondValueLabel->setEnabled(false);
         mpSecondValueSpin->setEnabled(false);
         break;

      case MIDDLE:   // Fall through
      case OUTSIDE:
         mpSecondValueLabel->setEnabled(true);
         mpSecondValueSpin->setEnabled(true);
         break;

      default:
         break;
   }
}

void PropertiesThresholdLayer::setRegionUnits(RegionUnits newUnits)
{
   if (mpThresholdLayer == NULL)
   {
      return;
   }

   // Get the values in the current units
   double dFirst = mpFirstValueSpin->value();
   double dSecond = mpSecondValueSpin->value();

   // Convert the values to the new units
   dFirst = mpThresholdLayer->convertThreshold(mUnits, dFirst, newUnits);
   dSecond = mpThresholdLayer->convertThreshold(mUnits, dSecond, newUnits);

   // Update the value spin boxes
   mpFirstValueSpin->setValue(dFirst);
   mpSecondValueSpin->setValue(dSecond);

   // Update the member units
   mUnits = newUnits;
}
