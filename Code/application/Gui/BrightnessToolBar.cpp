/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>

#include "BrightnessToolBar.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "HistogramWindowImp.h"
#include "LayerList.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayerAdapter.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Undo.h"
#include "WorkspaceWindow.h"

#include <vector>
using namespace std;

static const unsigned int GRAY_BANDCOMBO_INDEX = 0;
static const unsigned int RED_BANDCOMBO_INDEX = 1;
static const unsigned int GREEN_BANDCOMBO_INDEX = 2;
static const unsigned int BLUE_BANDCOMBO_INDEX = 3;
static const unsigned int RGB_BANDCOMBO_INDEX = 4;

BrightnessToolBar::BrightnessToolBar(const string& id, QWidget* parent) :
   ToolBarAdapter(id, "Brightness", parent),
   mBrightnessValue(0.0),
   mpBrightnessSlider(NULL),
   mpBrightnessText(NULL),
   mContrastValue(0.0),
   mpContrastSlider(NULL),
   mpContrastText(NULL),
   mpBandCombo(NULL),
   mpResetAction(NULL),
   mRasterChannelType(GRAY),
   mRgb(false),
   mpRasterLayer(NULL)
{
   // Brightness
   QWidget* pBrightnessWidget = new QWidget(this);
   QLabel* pBrightnessLabel = new QLabel("Brightness", pBrightnessWidget);

   mpBrightnessSlider = new QSlider(Qt::Horizontal, pBrightnessWidget);
   mpBrightnessSlider->setFixedWidth(200);
   mpBrightnessSlider->setMinimum(0);
   mpBrightnessSlider->setMaximum(100);
   mpBrightnessSlider->setPageStep(5);
   mpBrightnessSlider->setTracking(false);

   mpBrightnessText = new QLabel(pBrightnessWidget);
   mpBrightnessText->setFixedWidth(25);
   mpBrightnessText->setAlignment(Qt::AlignCenter);

   QHBoxLayout* pBrightnessLayout = new QHBoxLayout(pBrightnessWidget);
   pBrightnessLayout->setMargin(0);
   pBrightnessLayout->setSpacing(5);
   pBrightnessLayout->addWidget(pBrightnessLabel);
   pBrightnessLayout->addWidget(mpBrightnessSlider);
   pBrightnessLayout->addWidget(mpBrightnessText);

   pBrightnessWidget->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));

   addWidget(pBrightnessWidget);
   addSeparator();

   // Contrast
   QWidget* pContrastWidget = new QWidget(this);
   QLabel* pContrastLabel = new QLabel("Contrast", pContrastWidget);

   mpContrastSlider = new QSlider(Qt::Horizontal, pContrastWidget);
   mpContrastSlider->setFixedWidth(200);
   mpContrastSlider->setMinimum(0);
   mpContrastSlider->setMaximum(100);
   mpContrastSlider->setPageStep(5);
   mpContrastSlider->setTracking(false);

   mpContrastText = new QLabel(pContrastWidget);
   mpContrastText->setFixedWidth(25);
   mpContrastText->setAlignment(Qt::AlignCenter);

   QHBoxLayout* pContrastLayout = new QHBoxLayout(pContrastWidget);
   pContrastLayout->setMargin(0);
   pContrastLayout->setSpacing(5);
   pContrastLayout->addWidget(pContrastLabel);
   pContrastLayout->addWidget(mpContrastSlider);
   pContrastLayout->addWidget(mpContrastText);

   pContrastWidget->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));

   addWidget(pContrastWidget);
   addSeparator();

   // Band combo
   mpBandCombo = new QComboBox(this);
   mpBandCombo->setEditable(false);
   mpBandCombo->setFixedWidth(150);
   mpBandCombo->setInsertPolicy(QComboBox::NoInsert);
   mpBandCombo->setToolTip("Band");
   mpBandCombo->setStatusTip("Picks the band affected by the Brightness and Contrast sliders");
   mpBandCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
   addWidget(mpBandCombo);

   mpLayerCombo = new QComboBox(this);
   mpLayerCombo->setEditable(false);
   mpLayerCombo->setInsertPolicy(QComboBox::NoInsert);
   mpLayerCombo->setToolTip("Layer");
   mpLayerCombo->setStatusTip("Picks the layer affected by the Brightness and Contrast sliders");
   mpLayerCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
   addWidget(mpLayerCombo);

   Service<DesktopServices> pDesktop;

   HistogramWindowImp* pHistWnd =
      dynamic_cast<HistogramWindowImp*>(pDesktop->getWindow("Histogram Window", PLOT_WINDOW));
   if (pHistWnd != NULL)
   {
      VERIFYNR(connect(pHistWnd, SIGNAL(plotActivated(Layer*, const RasterChannelType&)), this,
         SLOT(setCurrentLayer(Layer*, const RasterChannelType&))));
      VERIFYNR(connect(this, SIGNAL(layerActivated(Layer*, const RasterChannelType&)), pHistWnd,
         SLOT(setCurrentPlot(Layer*, const RasterChannelType&))));
   }

   // Reset button
   mpResetAction = new QAction(QIcon(":/icons/ResetStretch"), "Reset", this);
   mpResetAction->setAutoRepeat(false);
   mpResetAction->setStatusTip("Updates the current brightness and contrast to their imported values");
   VERIFYNR(connect(mpResetAction, SIGNAL(triggered()), this, SLOT(reset())));
   addButton(mpResetAction, windowTitle().toStdString());

   // Initialization
   updateValues();

   // Connections
   VERIFYNR(connect(mpBrightnessSlider, SIGNAL(sliderMoved(int)), this, SLOT(updateBrightnessValue(int))));
   VERIFYNR(connect(mpBrightnessSlider, SIGNAL(sliderMoved(int)), this, SLOT(updateBrightnessLabel(int))));
   VERIFYNR(connect(mpBrightnessSlider, SIGNAL(sliderReleased()), this, SLOT(adjustLayerStretch())));
   VERIFYNR(connect(mpBrightnessSlider, SIGNAL(actionTriggered(int)), this, SLOT(updateBrightness(int))));
   VERIFYNR(connect(mpContrastSlider, SIGNAL(sliderMoved(int)), this, SLOT(updateContrastValue(int))));
   VERIFYNR(connect(mpContrastSlider, SIGNAL(sliderMoved(int)), this, SLOT(updateContrastLabel(int))));
   VERIFYNR(connect(mpContrastSlider, SIGNAL(sliderReleased()), this, SLOT(adjustLayerStretch())));
   VERIFYNR(connect(mpContrastSlider, SIGNAL(actionTriggered(int)), this, SLOT(updateContrast(int))));
   VERIFYNR(connect(mpBandCombo, SIGNAL(activated(int)), this, SLOT(onBandSelectionChange(int))));
   VERIFYNR(connect(mpLayerCombo, SIGNAL(activated(int)), this, SLOT(onLayerSelectionChange(int))));
}

BrightnessToolBar::~BrightnessToolBar()
{
   if (mpRasterLayer != NULL)
   {
      mpRasterLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &BrightnessToolBar::onRasterLayerDeleted));
   }
}

Layer* BrightnessToolBar::getCurrentLayer() const
{
   return mpRasterLayer;
}

void BrightnessToolBar::updateValues()
{
   double dLower = 0;
   double dUpper = 0;

   if (mpRasterLayer != NULL)
   {
      mpRasterLayer->getStretchValues(mRasterChannelType, dLower, dUpper);
   }

   updateValues(mRasterChannelType, dLower, dUpper);
}

namespace
{
double getDelta1(const RasterLayer *pLayer, RasterChannelType eColor)
{
   double percentile25 = pLayer->convertStretchValue(eColor, PERCENTILE, 25, RAW_VALUE);
   double percentile75 = pLayer->convertStretchValue(eColor, PERCENTILE, 75, RAW_VALUE);
   return (percentile75-percentile25)/50.0;
}
}

void BrightnessToolBar::updateValues(const RasterChannelType& eColor, double dLower, double dUpper)
{
   if (eColor != mRasterChannelType)
   {
      return;
   }

   mContrastValue = 0.0;
   mBrightnessValue = 0.0;

   if (mpRasterLayer != NULL)
   {
      RegionUnits eRegionUnits = mpRasterLayer->getStretchUnits(eColor);
      if (eRegionUnits != RAW_VALUE)
      {
         dLower = mpRasterLayer->convertStretchValue(eColor, eRegionUnits, dLower, RAW_VALUE);
         dUpper = mpRasterLayer->convertStretchValue(eColor, eRegionUnits, dUpper, RAW_VALUE);
      }
      if (dUpper - dLower >= 0)
      {
         mBrightnessValue = 100.0 - mpRasterLayer->convertStretchValue(eColor, RAW_VALUE,
            (dUpper + dLower) / 2.0, PERCENTILE);
         double dWidth = getDelta1(mpRasterLayer, eColor);
         if (fabs(dWidth) > 1e-20) // prevent divide-by-zero
         {
            mContrastValue = 100.0 - (dUpper - dLower) / (5.0 * dWidth);
         }
      }
      else
      {
         mBrightnessValue = mpRasterLayer->convertStretchValue(eColor, RAW_VALUE, (dUpper + dLower) / 2.0, PERCENTILE);
         double dWidth = -getDelta1(mpRasterLayer, eColor);
         if (fabs(dWidth) > 1e-20) // prevent divide-by-zero
         {
            mContrastValue = 100.0 - (dUpper - dLower) / (5.0 * dWidth);
         }
      }
   }

   // Set the slider and label values
   mpBrightnessSlider->setValue(static_cast<int>(mBrightnessValue + 0.5));
   mpContrastSlider->setValue(static_cast<int>(mContrastValue + 0.5));
   updateBrightnessLabel(static_cast<int>(mBrightnessValue + 0.5));
   updateContrastLabel(static_cast<int>(mContrastValue + 0.5));

   // Make sure all three stretch values are updated when displaying RGB
   if (mRgb == true)
   {
      adjustLayerStretch();
   }

   enableSliders();
}

void BrightnessToolBar::updateForNewView()
{
   updateLayerCombo(true);  
   updateBandCombo(true); //this populates band combo based on mpRasterLayer being set to something
   updateValues();
}

void BrightnessToolBar::setCurrentLayer(Layer* pLayer, const RasterChannelType& eColor, bool bRgb)
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);

   bool bNewLayer = false;
   if ((pRasterLayer != mpRasterLayer) || (eColor != mRasterChannelType))
   {
      bNewLayer = true;
   }

   updateLayerCombo(false);

   // Make sure the layer in the member list or is NULL
   bool bValidLayer = false;
   if (pRasterLayer != NULL)
   {      
      vector<RasterLayer*>::iterator foundLayer;
      foundLayer = std::find(mLayers.begin(), mLayers.end(), pRasterLayer);
      if (foundLayer != mLayers.end())
      {
         bValidLayer = true;
      }
   }
   else
   {
      bValidLayer = true;
   }

   if (bValidLayer == false)
   {
      return;
   }

   // Update the layer
   if (pRasterLayer != mpRasterLayer)
   {
      LayerImp* pLayerImp = dynamic_cast<LayerImp*>(mpRasterLayer);
      if (pLayerImp != NULL)
      {
         disconnect(pLayerImp,
            SIGNAL(stretchTypeChanged(const DisplayMode&, const StretchType&)), this, SLOT(enableSliders()));
         disconnect(pLayerImp,
            SIGNAL(stretchValuesChanged(const RasterChannelType&, double, double)), this,
            SLOT(updateValues(const RasterChannelType&, double, double)));
         pLayerImp->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &BrightnessToolBar::onRasterLayerDeleted));
      }

      mpRasterLayer = pRasterLayer;
      pLayerImp = dynamic_cast<LayerImp*>(pRasterLayer);
      if (pLayerImp != NULL)
      {
         VERIFYNR(connect(pLayerImp,
            SIGNAL(stretchTypeChanged(const DisplayMode&, const StretchType&)), this, SLOT(enableSliders())));
         VERIFYNR(connect(pLayerImp,
            SIGNAL(stretchValuesChanged(const RasterChannelType&, double, double)), this,
            SLOT(updateValues(const RasterChannelType&, double, double))));
         pLayerImp->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &BrightnessToolBar::onRasterLayerDeleted));
      }
   }

   // Update the raster element
   if (eColor != mRasterChannelType)
   {
      mRasterChannelType = eColor;
   }

   //now that mpRasterLayer has changed, update the band combo box
   updateBandCombo(false);

   // Update the RGB flag
   if (bRgb != mRgb)
   {
      mRgb = bRgb;
   }

   // Set the display mode
   if (mpRasterLayer != NULL)
   {
      if (mRasterChannelType == GRAY)
      {
         mpRasterLayer->setDisplayMode(GRAYSCALE_MODE);
      }
      else 
      {
         mpRasterLayer->setDisplayMode(RGB_MODE);
      }
   }

   // Update the brightness and contrast values
   updateValues();

   // Emit the signal
   if (bNewLayer == true)
   {
      emit layerActivated(mpRasterLayer, mRasterChannelType);
   }

   // Update the RGB flag - both here and above because required in both places
   if (bRgb != mRgb)
   {
      mRgb = bRgb;
   }

   // Select the right value in the band combo
   if (mpRasterLayer != NULL)
   {
      if (mRgb)
      {
         mpBandCombo->setCurrentIndex(RGB_BANDCOMBO_INDEX);
      }
      else if (mRasterChannelType == GRAY)
      {
         mpBandCombo->setCurrentIndex(GRAY_BANDCOMBO_INDEX);
      }
      else if (mRasterChannelType == RED)
      {
         mpBandCombo->setCurrentIndex(RED_BANDCOMBO_INDEX);
      }
      else if (mRasterChannelType == GREEN)
      {
         mpBandCombo->setCurrentIndex(GREEN_BANDCOMBO_INDEX);
      }
      else if (mRasterChannelType == BLUE)
      {
         mpBandCombo->setCurrentIndex(BLUE_BANDCOMBO_INDEX);
      }
   }

   if (mpRasterLayer != NULL)
   {
      //Pick the right layer out of the combo box
      int newIndex = -1;
      for (int i = 0; i < static_cast<int>(mLayers.size()); i++)
      {
         if (mLayers[i] == mpRasterLayer)
         {
            newIndex = i;
         }
      }
      if (newIndex != -1)
      {
         mpLayerCombo->setCurrentIndex(newIndex);
      }
   }


   // Enable sliders, labels, and the reset button
   enableSliders();
}

void BrightnessToolBar::onRasterLayerDeleted(Subject &subject, const std::string &signal, const boost::any &v)
{
   RasterLayer* pSubject = dynamic_cast<RasterLayer*>(&subject);
   if (pSubject == mpRasterLayer)
   {
      mpRasterLayer = NULL; //nullify member variable
      updateLayerCombo(false); //clear the layer combo box
      updateBandCombo(false); //clear the band combo box
   }
}

void BrightnessToolBar::updateLayerCombo(bool updateCurrentRasterLayer)
{
   mpLayerCombo->clear();
   mLayers.clear();

   Service<DesktopServices> pDesktop;
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getCurrentWorkspaceWindow());
   vector<Layer*> layers;
   if (pWindow != NULL)
   {      
      SpatialDataView* pView = pWindow->getSpatialDataView();
      if (pView != NULL)
      {
         LayerList* pLayerList = pView->getLayerList();
         if (pLayerList != NULL)
         {
            pLayerList->getLayers(layers);
         }
      }
   }

   if (updateCurrentRasterLayer)
   {
      if (mpRasterLayer != NULL)
      {
         mpRasterLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &BrightnessToolBar::onRasterLayerDeleted));
      }
      mpRasterLayer = NULL;
   }

   bool needToSetRasterLayer = updateCurrentRasterLayer;
   for (unsigned int i = 0; i < layers.size(); i++)
   {
      RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(layers[i]);
      if (pRasterLayer == NULL)
      {
         continue;
      }

      if (needToSetRasterLayer)
      {
         mpRasterLayer = pRasterLayer;
         mpRasterLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &BrightnessToolBar::onRasterLayerDeleted));
         needToSetRasterLayer = false;
      }

      string layerName = pRasterLayer->getName();
      QString strLayerName = QString::fromStdString(layerName);
      mpLayerCombo->addItem(strLayerName);
      mLayers.push_back(pRasterLayer);
   }

   if (updateCurrentRasterLayer)
   {
      if (!layers.empty())
      {
         mpLayerCombo->setCurrentIndex(0);
      }
   }

   adjustSize();
}

void BrightnessToolBar::updateBandCombo(bool updateCurrentBand)
{
   mpBandCombo->clear();

   //Assumes that mpRasterLayer is set
   if (mpRasterLayer == NULL)
   {
      return;
   }

   vector<RasterChannelType> channels = RasterUtilities::getVisibleRasterChannels();
   for (vector<RasterChannelType>::iterator curChannel = channels.begin();
        curChannel != channels.end(); ++curChannel)
   {
      // Find the current band so we can display it with the name
      QString strBandName;
      RasterElement* pElement = mpRasterLayer->getDisplayedRasterElement(*curChannel);
      DimensionDescriptor band = mpRasterLayer->getDisplayedBand(*curChannel);
      QString channelName;
      if (*curChannel == GRAY)
      {
         channelName = "Gray";
      }
      else if (*curChannel == RED)
      {
         channelName = "Red";
      }
      else if (*curChannel == GREEN)
      {
         channelName = "Green";
      }
      else if (*curChannel == BLUE)
      {
         channelName = "Blue";
      }
      if ((pElement != NULL) && (band.isValid()))
      {
         strBandName = QString::fromStdString(RasterUtilities::getBandName(
            dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor()), band));
         strBandName.append(" - ");
         strBandName.append(channelName);
      }
      else
      {
         strBandName.sprintf("No Band Displayed - %s", channelName);
      }
      mpBandCombo->addItem(strBandName);
   }
   // Add the RGB item to update all three color values simultaneously
   QString strBandName = "RGB";
   mpBandCombo->addItem(strBandName);

   if (updateCurrentBand)
   {
      DisplayMode mode = mpRasterLayer->getDisplayMode();
      if (mode == GRAYSCALE_MODE)
      {
         mpBandCombo->setCurrentIndex(GRAY_BANDCOMBO_INDEX);
         mRasterChannelType = GRAY;
      }
      if (mode == RGB_MODE)
      {
         mpBandCombo->setCurrentIndex(RED_BANDCOMBO_INDEX);
         mRasterChannelType = RED;
      }
   }
}

void BrightnessToolBar::onBandSelectionChange(int newIndex)
{
   RasterChannelType channel = GRAY;
   bool bRgb = false;
   if (newIndex == GRAY_BANDCOMBO_INDEX)
   {
      channel = GRAY;
   }
   else if (newIndex == RED_BANDCOMBO_INDEX)
   {
      channel = RED;
   }
   else if (newIndex == GREEN_BANDCOMBO_INDEX)
   {
      channel = GREEN;
   }
   else if (newIndex == BLUE_BANDCOMBO_INDEX)
   {
      channel = BLUE;
   }
   else if (newIndex == RGB_BANDCOMBO_INDEX)
   {
      channel = RED;
      bRgb = true;
   }
   setCurrentLayer(mpRasterLayer, channel, bRgb);
}

void BrightnessToolBar::onLayerSelectionChange(int newIndex)
{
   RasterLayer* pRasterLayer = NULL;
   if (newIndex < static_cast<int>(mLayers.size()))
   {
      pRasterLayer = mLayers[newIndex];
   }
   RasterChannelType channel = GRAY;
   if (pRasterLayer != NULL)
   {
      DisplayMode mode = pRasterLayer->getDisplayMode();
      channel = (mode == GRAYSCALE_MODE ? GRAY : RED);
   }
   setCurrentLayer(pRasterLayer, channel, false);
}

void BrightnessToolBar::updateBrightnessValue(int iValue)
{
   mBrightnessValue = iValue;
}

void BrightnessToolBar::updateBrightnessLabel(int iValue)
{
   mpBrightnessText->setText(QString::number(iValue));
}

void BrightnessToolBar::updateBrightness(int sliderAction)
{
   if ((sliderAction == QAbstractSlider::SliderNoAction) || (sliderAction == QAbstractSlider::SliderMove))
   {
      return;
   }

   int value = mpBrightnessSlider->sliderPosition();
   updateBrightnessValue(value);
   updateBrightnessLabel(value);
   adjustLayerStretch();
}

void BrightnessToolBar::updateContrastValue(int iValue)
{
   mContrastValue = iValue;
}

void BrightnessToolBar::updateContrastLabel(int iValue)
{
   mpContrastText->setText(QString::number(iValue));
}

void BrightnessToolBar::updateContrast(int sliderAction)
{
   if ((sliderAction == QAbstractSlider::SliderNoAction) || (sliderAction == QAbstractSlider::SliderMove))
   {
      return;
   }

   int value = mpContrastSlider->sliderPosition();
   updateContrastValue(value);
   updateContrastLabel(value);
   adjustLayerStretch();
}

void BrightnessToolBar::adjustLayerStretch()
{
   if (mpRasterLayer == NULL)
   {
      return;
   }

   DisplayMode eDisplayMode = mpRasterLayer->getDisplayMode();
   RegionUnits eRegionUnits = mpRasterLayer->getStretchUnits(mRasterChannelType);

   double dLower = 0;
   double dUpper = 0;
   mpRasterLayer->getStretchValues(mRasterChannelType, dLower, dUpper);

   // Calculate the new stretch values as percentages
   if (eRegionUnits != PERCENTAGE)
   {
      dLower = mpRasterLayer->convertStretchValue(mRasterChannelType, eRegionUnits, dLower, PERCENTAGE);
      dUpper = mpRasterLayer->convertStretchValue(mRasterChannelType, eRegionUnits, dUpper, PERCENTAGE);
   }

   double dWidth = 5.0 * (100 - mContrastValue) * getDelta1(mpRasterLayer, mRasterChannelType);
   double brightness = 100.0 - mBrightnessValue;
   if ((dUpper - dLower) < 0)
   {
      dWidth *= -1;
      brightness = 100-brightness;
   }

   dUpper = mpRasterLayer->convertStretchValue(mRasterChannelType, PERCENTILE, brightness, RAW_VALUE)
         + dWidth/2.0;
   dLower = dUpper - dWidth;

   // Set the new stretch values as percentiles
   dLower = mpRasterLayer->convertStretchValue(mRasterChannelType, RAW_VALUE, dLower, PERCENTILE);
   dUpper = mpRasterLayer->convertStretchValue(mRasterChannelType, RAW_VALUE, dUpper, PERCENTILE);

   // Disconnect the layer to prevent modifying the sliders from the current values
   disconnect(dynamic_cast<RasterLayerImp*>(mpRasterLayer),
      SIGNAL(stretchValuesChanged(const RasterChannelType&, double, double)),
      this, SLOT(updateValues(const RasterChannelType&, double, double)));

   // If RGB is selected in the combo box, set the red, green and blue values all at once
   int bandIndex = mpBandCombo->currentIndex();
   if (bandIndex == RGB_BANDCOMBO_INDEX)
   {
      UndoGroup undoGroup(mpRasterLayer->getView(), "Update Layer Contrast Stretch");

      // Red
      double dRedLower = dLower;
      double dRedUpper = dUpper;

      if (eRegionUnits != PERCENTILE)
      {
         dRedLower = mpRasterLayer->convertStretchValue(RED, PERCENTILE, dLower, eRegionUnits);
         dRedUpper = mpRasterLayer->convertStretchValue(RED, PERCENTILE, dUpper, eRegionUnits);
      }

      mpRasterLayer->setStretchValues(RED, dRedLower, dRedUpper);

      // Green
      double dGreenLower = dLower;
      double dGreenUpper = dUpper;

      if (eRegionUnits != PERCENTILE)
      {
         dGreenLower = mpRasterLayer->convertStretchValue(GREEN, PERCENTILE, dLower, eRegionUnits);
         dGreenUpper = mpRasterLayer->convertStretchValue(GREEN, PERCENTILE, dUpper, eRegionUnits);
      }

      mpRasterLayer->setStretchValues(GREEN, dGreenLower, dGreenUpper);

      // Blue
      double dBlueLower = dLower;
      double dBlueUpper = dUpper;

      if (eRegionUnits != PERCENTILE)
      {
         dBlueLower = mpRasterLayer->convertStretchValue(BLUE, PERCENTILE, dLower, eRegionUnits);
         dBlueUpper = mpRasterLayer->convertStretchValue(BLUE, PERCENTILE, dUpper, eRegionUnits);
      }

      mpRasterLayer->setStretchValues(BLUE, dBlueLower, dBlueUpper);
   }
   else
   {
      if (eRegionUnits != PERCENTILE)
      {
         dLower = mpRasterLayer->convertStretchValue(mRasterChannelType, PERCENTILE, dLower, eRegionUnits);
         dUpper = mpRasterLayer->convertStretchValue(mRasterChannelType, PERCENTILE, dUpper, eRegionUnits);
      }

      mpRasterLayer->setStretchValues(mRasterChannelType, dLower, dUpper);
   }

   // Reconnect the layer to modify the slider values
   VERIFYNR(connect(dynamic_cast<RasterLayerImp*>(mpRasterLayer),
      SIGNAL(stretchValuesChanged(const RasterChannelType&, double, double)),
      this, SLOT(updateValues(const RasterChannelType&, double, double))));

   enableSliders();
}

void BrightnessToolBar::reset()
{
   if (mpRasterLayer != NULL)
   {
      mpRasterLayer->resetStretch();
   }
}

void BrightnessToolBar::enableSliders()
{
   bool enable = true;

   if (mpRasterLayer == NULL)
   {
      enable = false;
   }
   else
   {
      QString strLayer = mpBandCombo->currentText();
      if (strLayer.startsWith("No Band Displayed ") == true)
      {
         enable = false;
      }

      if (mpRasterLayer->getDisplayMode() == GRAYSCALE_MODE &&
         mpRasterLayer->getStretchType(GRAYSCALE_MODE) == EQUALIZATION)
      {
         enable = false;
      }
      if (mpRasterLayer->getDisplayMode() == RGB_MODE &&
         mpRasterLayer->getStretchType(RGB_MODE) == EQUALIZATION)
      {
         enable = false;
      }

      if (mpRasterLayer->canApplyFastContrastStretch())
      {
         mpBrightnessSlider->setTracking(true);
         mpContrastSlider->setTracking(true);
      }
      else
      {
         mpBrightnessSlider->setTracking(false);
         mpContrastSlider->setTracking(false);
      }
   }

   mpContrastSlider->setEnabled(enable);
   mpBrightnessSlider->setEnabled(enable);
   mpContrastText->setEnabled(enable);
   mpBrightnessText->setEnabled(enable);
   mpResetAction->setEnabled(enable);
}
