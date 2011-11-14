/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CustomColorButton.h"
#include "LabeledSection.h"
#include "HistogramWindow.h"
#include "OptionsSpatialDataView.h"
#include "PanLimitTypeComboBox.h"
#include "PerspectiveView.h"
#include "SpatialDataView.h"
#include "View.h"
#include "WorkspaceWindow.h"

#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDesktopWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <limits>
#include <string>

using namespace std;

OptionsSpatialDataView::OptionsSpatialDataView() :
   QWidget(NULL)
{
   // Dynamic Inset
   QLabel* pInsetSizeLabel = new QLabel("Size:", this);
   mpInsetSizeSpin = new QSpinBox(this);
   mpInsetSizeSpin->setSuffix(" Pixel(s)");
   mpInsetSizeSpin->setRange(1, numeric_limits<int>::max());

   QLabel* pInsetZoomFactorLabel = new QLabel("Zoom Factor:", this);
   mpInsetZoomSpin = new QSpinBox(this);

   QLabel* pInsetZoomLabel = new QLabel("Zoom Factor Is:", this);
   mpInsetZoom = new QComboBox(this);
   mpInsetZoom->addItem("Relative");
   mpInsetZoom->addItem("Absolute");

   QWidget* pInsetWidget = new QWidget(this);
   QGridLayout* pInsetLayout = new QGridLayout(pInsetWidget);
   pInsetLayout->setMargin(0);
   pInsetLayout->setSpacing(5);
   pInsetLayout->addWidget(pInsetSizeLabel, 0, 0);
   pInsetLayout->addWidget(mpInsetSizeSpin, 0, 1, Qt::AlignLeft);
   pInsetLayout->addWidget(pInsetZoomFactorLabel, 1, 0);
   pInsetLayout->addWidget(mpInsetZoomSpin, 1, 1, Qt::AlignLeft);
   pInsetLayout->addWidget(pInsetZoomLabel, 2, 0);
   pInsetLayout->addWidget(mpInsetZoom, 2, 1, Qt::AlignLeft);
   pInsetLayout->setColumnStretch(1, 10);
   LabeledSection* pInsetSection = new LabeledSection(pInsetWidget, "Dynamic Inset", this);

   // Pan Speed
   QLabel* pFastPanSpeedLabel = new QLabel("Fast:", this);
   QLabel* pSlowPanSpeedLabel = new QLabel("Slow:", this);
   QLabel* pMousePanSensitivityLabel = new QLabel("Mouse Pan Sensitivity:", this);
   mpFastPanSpeedSpin = new QSpinBox(this);
   mpFastPanSpeedSpin->setSuffix(" Pixel(s)");
   mpFastPanSpeedSpin->setRange(1, numeric_limits<int>::max());
   mpSlowPanSpeedSpin = new QSpinBox(this);
   mpSlowPanSpeedSpin->setSuffix(" Pixel(s)");
   mpSlowPanSpeedSpin->setRange(1, numeric_limits<int>::max());
   mpMousePanSensitivitySpin = new QSpinBox(this);
   mpMousePanSensitivitySpin->setRange(1, 10000);

   QWidget* pPanSpeedWidget = new QWidget(this);
   QGridLayout* pPanSpeedLayout = new QGridLayout(pPanSpeedWidget);
   pPanSpeedLayout->setMargin(0);
   pPanSpeedLayout->setSpacing(5);
   pPanSpeedLayout->addWidget(pFastPanSpeedLabel, 0, 0);
   pPanSpeedLayout->addWidget(mpFastPanSpeedSpin, 0, 1);
   pPanSpeedLayout->addWidget(pSlowPanSpeedLabel, 1, 0);
   pPanSpeedLayout->addWidget(mpSlowPanSpeedSpin, 1, 1);
   pPanSpeedLayout->addWidget(pMousePanSensitivityLabel, 2, 0);
   pPanSpeedLayout->addWidget(mpMousePanSensitivitySpin, 2, 1);
   pPanSpeedLayout->setColumnStretch(2, 10);
   LabeledSection* pPanSpeedSection = new LabeledSection(pPanSpeedWidget, "Pan Speed", this);

   // Other Options
   mpGeoCoordTooltip = new QCheckBox("Geo-coordinate tool tip", this);
   mpConfirmLayerDelete = new QCheckBox("Confirm non-undoable layer delete", this);
   mpActiveLayer = new QCheckBox("Link layer and histogram activation", this);
   mpActiveLayer->setToolTip("Selecting a threshold or raster layer histogram will activate the layer and "
      "make it visible.\nShowing a threshold layer in the session explorer will make the histogram plot "
      "active.\nThe primary raster layer will never be activated and pulled to the top of the layer stack.");
   mpShowCoordinates = new QCheckBox("Show pixel coordinates when zoomed in", this);
   mpShowCoordinates->setToolTip("Show pixel coordinates instead of raw values when zoomed in.");
   mpDisplayOrigin = new QCheckBox("Display origin location", this);
   mpDisplayAxis = new QCheckBox("Display orientation axis", this);
   mpDisplayCrosshair = new QCheckBox("Display crosshair", this);

   QWidget* pOtherOptionsWidget = new QWidget(this);
   QVBoxLayout* pOtherOptionsLayout = new QVBoxLayout(pOtherOptionsWidget);
   pOtherOptionsLayout->setMargin(0);
   pOtherOptionsLayout->setSpacing(5);
   pOtherOptionsLayout->addWidget(mpGeoCoordTooltip);
   pOtherOptionsLayout->addWidget(mpConfirmLayerDelete);
   pOtherOptionsLayout->addWidget(mpActiveLayer);
   pOtherOptionsLayout->addWidget(mpShowCoordinates);
   pOtherOptionsLayout->addWidget(mpDisplayOrigin);
   pOtherOptionsLayout->addWidget(mpDisplayAxis);
   pOtherOptionsLayout->addWidget(mpDisplayCrosshair);
   LabeledSection* pOtherOptionsSection = new LabeledSection(pOtherOptionsWidget, "Other Options", this);

   // Panning and Zooming Limit
   QLabel* pPanLimitLabel = new QLabel("Pan Limit:", this);
   mpPanLimit = new PanLimitTypeComboBox(this);

   QLabel* pMinZoomLabel = new QLabel("When Zooming In:", this);
   QLabel* pMaxZoomLabel = new QLabel("When Zooming Out:", this);
   mpMinZoom = new QDoubleSpinBox(this);
   mpMinZoom->setSuffix(" Pixel(s)");
   mpMinZoom->setRange(0.0, numeric_limits<double>::max());
   pMinZoomLabel->setToolTip("Show at least this many data pixels");
   mpMinZoom->setToolTip("Show at least this many data pixels");

   mpMaxZoom = new QSpinBox(this);
   mpMaxZoom->setSuffix("%");
   mpMaxZoom->setRange(0, 100);
   pMaxZoomLabel->setToolTip("The data should occupy at least this much percentage of the window");
   mpMaxZoom->setToolTip("The data should occupy at least this much percentage of the window");

   QWidget* pPanZoomWidget = new QWidget(this);
   QGridLayout* pPanZoomLayout = new QGridLayout(pPanZoomWidget);
   pPanZoomLayout->setMargin(0);
   pPanZoomLayout->setSpacing(5);
   pPanZoomLayout->addWidget(pPanLimitLabel, 0, 0);
   pPanZoomLayout->addWidget(mpPanLimit, 0, 1, Qt::AlignLeft);
   pPanZoomLayout->addWidget(pMinZoomLabel, 1, 0);
   pPanZoomLayout->addWidget(mpMinZoom, 1, 1, Qt::AlignLeft);
   pPanZoomLayout->addWidget(pMaxZoomLabel, 2, 0);
   pPanZoomLayout->addWidget(mpMaxZoom, 2, 1, Qt::AlignLeft);
   pPanZoomLayout->setColumnStretch(1, 10);
   LabeledSection* pPanZoomLimitSection = new LabeledSection(pPanZoomWidget,
      "Default Panning And Zooming Limits", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pInsetSection);
   pLayout->addWidget(pPanSpeedSection);
   pLayout->addWidget(pOtherOptionsSection);
   pLayout->addWidget(pPanZoomLimitSection);
   pLayout->addStretch(10);

   // Initialize From Settings
   mpInsetSizeSpin->setValue(View::getSettingInsetSize());
   mpInsetZoomSpin->setValue(View::getSettingInsetZoom());
   InsetZoomMode zoomMode = View::getSettingInsetZoomMode();
   if (zoomMode == ABSOLUTE_MODE)
   {
      mpInsetZoom->setCurrentIndex(1);
   }
   else if (zoomMode == RELATIVE_MODE)
   {
      mpInsetZoom->setCurrentIndex(0);
   }
   mpShowCoordinates->setChecked(View::getSettingInsetShowCoordinates());
   mpFastPanSpeedSpin->setValue(SpatialDataView::getSettingFastPanSpeed());
   mpSlowPanSpeedSpin->setValue(SpatialDataView::getSettingSlowPanSpeed());
   mpMousePanSensitivitySpin->setValue(View::getSettingMousePanSensitivity());
   mpPanLimit->setCurrentValue(SpatialDataView::getSettingPanLimit());
   mpMinZoom->setValue(SpatialDataView::getSettingMinimumZoomPixels());
   double zoomRatio = SpatialDataView::getSettingMaximumZoomRatio();
   int zoomPercent = zoomRatio * 100;
   mpMaxZoom->setValue(zoomPercent);
   mpGeoCoordTooltip->setChecked(SpatialDataView::getSettingGeoCoordTooltip());
   mpConfirmLayerDelete->setChecked(SpatialDataView::getSettingConfirmLayerDelete());
   mpActiveLayer->setChecked(HistogramWindow::getSettingLayerActivation());
   mpDisplayOrigin->setChecked(SpatialDataView::getSettingDisplayOrigin());
   mpDisplayAxis->setChecked(SpatialDataView::getSettingDisplayAxis());
   mpDisplayCrosshair->setChecked(View::getSettingDisplayCrosshair());
}

void OptionsSpatialDataView::applyChanges()
{
   View::setSettingInsetSize(mpInsetSizeSpin->value());
   View::setSettingInsetZoom(mpInsetZoomSpin->value());
   InsetZoomMode zoomMode;
   if (mpInsetZoom->currentIndex() == 1)
   {
      zoomMode = ABSOLUTE_MODE;
   }
   else if (mpInsetZoom->currentIndex() == 0)
   {
      zoomMode = RELATIVE_MODE;
   }
   View::setSettingInsetZoomMode(zoomMode);
   View::setSettingInsetShowCoordinates(mpShowCoordinates->isChecked());
   SpatialDataView::setSettingFastPanSpeed(mpFastPanSpeedSpin->value());
   SpatialDataView::setSettingSlowPanSpeed(mpSlowPanSpeedSpin->value());
   View::setSettingMousePanSensitivity(mpMousePanSensitivitySpin->value());
   SpatialDataView::setSettingPanLimit(mpPanLimit->getCurrentValue());
   SpatialDataView::setSettingMinimumZoomPixels(mpMinZoom->value());
   int zoomPercent = mpMaxZoom->value();
   double zoomRatio = zoomPercent / 100.0;
   SpatialDataView::setSettingMaximumZoomRatio(zoomRatio);
   SpatialDataView::setSettingGeoCoordTooltip(mpGeoCoordTooltip->isChecked());
   SpatialDataView::setSettingConfirmLayerDelete(mpConfirmLayerDelete->isChecked());
   HistogramWindow::setSettingLayerActivation(mpActiveLayer->isChecked());
   SpatialDataView::setSettingDisplayOrigin(mpDisplayOrigin->isChecked());
   SpatialDataView::setSettingDisplayAxis(mpDisplayAxis->isChecked());
   View::setSettingDisplayCrosshair(mpDisplayCrosshair->isChecked());
}

OptionsSpatialDataView::~OptionsSpatialDataView()
{}
