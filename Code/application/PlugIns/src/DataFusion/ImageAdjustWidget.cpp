/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ImageAdjustWidget.h"
#include "Layer.h"
#include "RasterLayer.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "WorkspaceWindow.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <qwt_knob.h>
#include <qwt_slider.h>
#include <qwt_wheel.h>

XERCES_CPP_NAMESPACE_USE

ImageAdjustWidget::ImageAdjustWidget(WorkspaceWindow* pWindow, QWidget* pParent) :
   QWidget(pParent),
   mpDesktop(Service<DesktopServices>().get(), SIGNAL_NAME(DesktopServices, WindowActivated),
      Slot(this, &ImageAdjustWidget::windowActivated))
{
   mpView.addSignal(SIGNAL_NAME(SpatialDataView, LayerShown), Slot(this, &ImageAdjustWidget::layerVisibilityChanged));
   mpView.addSignal(SIGNAL_NAME(SpatialDataView, LayerHidden), Slot(this, &ImageAdjustWidget::layerVisibilityChanged));
   mpLayerList.addSignal(SIGNAL_NAME(LayerList, LayerAdded), Slot(this, &ImageAdjustWidget::layerListChanged));
   mpLayerList.addSignal(SIGNAL_NAME(LayerList, LayerDeleted), Slot(this, &ImageAdjustWidget::layerListChanged));
   // listen to Subject::Modified since some of the signals we need only exist on Layer subclasses (alpha modified)
   // It will call more often then necessary but the slot doesn't do a ton of work so there shouldn't be a noticable
   // slow-down
   mpLayerAttachment.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &ImageAdjustWidget::layerPropertyChanged));

   QHBoxLayout* pTopLevel = new QHBoxLayout(this);
   pTopLevel->setMargin(10);
   pTopLevel->setSpacing(10);
   QVBoxLayout* pLeftSideLayout = new QVBoxLayout();
   pLeftSideLayout->setMargin(0);
   pLeftSideLayout->setSpacing(10);

   // Layer selection
   QHBoxLayout* pLayerSelectLayout = new QHBoxLayout();
   pLayerSelectLayout->setMargin(0);
   pLayerSelectLayout->setSpacing(5);
   QLabel* pLayerLabel = new QLabel("Layer:", this);
   pLayerSelectLayout->addWidget(pLayerLabel);

   mpLayerSelection = new QComboBox(this);
   pLayerSelectLayout->addWidget(mpLayerSelection, 10);

   pLeftSideLayout->addLayout(pLayerSelectLayout);

   // Offset/scale
   QGroupBox* pOffsetScaleGroup = new QGroupBox("Offset/Scale Tools", this);
   QVBoxLayout* pOsgTopLevel = new QVBoxLayout(pOffsetScaleGroup);
   QHBoxLayout* pShiftLayout = new QHBoxLayout();

   QLabel* pXShiftLabel = new QLabel("X Shift:", pOffsetScaleGroup);
   pShiftLayout->addWidget(pXShiftLabel);
   mpXOffsetWheel = new QwtWheel(pOffsetScaleGroup);
   mpXOffsetWheel->setValid(false);
   mpXOffsetWheel->setOrientation(Qt::Horizontal);
   pShiftLayout->addWidget(mpXOffsetWheel);
   mpXOffset = new QDoubleSpinBox(pOffsetScaleGroup);
   mpXOffset->setMinimum(-10000);
   mpXOffset->setMaximum(10000);
   pShiftLayout->addWidget(mpXOffset);
   pShiftLayout->addSpacing(10);

   mpXOffsetWheel->setPeriodic(true);
   mpXOffsetWheel->setRange(0, 10000, 1, 100);
   mpXOffsetWheel->setTotalAngle(36000);
   mXOffsetPrev = mpXOffsetWheel->value();

   QLabel* pYShiftLabel = new QLabel("Y Shift:", pOffsetScaleGroup);
   pShiftLayout->addWidget(pYShiftLabel);
   mpYOffsetWheel = new QwtWheel(pOffsetScaleGroup);
   pShiftLayout->addWidget(mpYOffsetWheel);
   mpYOffset = new QDoubleSpinBox(pOffsetScaleGroup);
   mpYOffset->setMinimum(-10000);
   mpYOffset->setMaximum(10000);
   pShiftLayout->addWidget(mpYOffset);
   pShiftLayout->addStretch(10);

   mpYOffsetWheel->setPeriodic(true);
   mpYOffsetWheel->setRange(0, 10000, 1, 100);
   mpYOffsetWheel->setTotalAngle(36000);
   mYOffsetPrev = mpYOffsetWheel->value();

   pOsgTopLevel->addLayout(pShiftLayout);

   QHBoxLayout* pScaleLayout = new QHBoxLayout();
   QLabel* pXScaleLabel = new QLabel("X Scale:", pOffsetScaleGroup);
   pScaleLayout->addWidget(pXScaleLabel);
   mpXScale = new QDoubleSpinBox(pOffsetScaleGroup);
   mpXScale->setValue(1);
   mpXScale->setSingleStep(0.1);
   pScaleLayout->addWidget(mpXScale);
   pScaleLayout->addSpacing(10);

   QLabel* pYScaleLabel = new QLabel("Y Scale:", pOffsetScaleGroup);
   pScaleLayout->addWidget(pYScaleLabel);
   mpYScale = new QDoubleSpinBox(pOffsetScaleGroup);
   mpYScale->setValue(1);
   mpYScale->setSingleStep(0.1);
   pScaleLayout->addWidget(mpYScale);
   pScaleLayout->addStretch(10);

   pOsgTopLevel->addLayout(pScaleLayout);
   pOsgTopLevel->addStretch(10);
   pLeftSideLayout->addWidget(pOffsetScaleGroup, 10);

   // Flicker/blend
   QGroupBox* pFlickerBlendGroup = new QGroupBox("Flicker/Blend Tools", this);
   QGridLayout* pFbgLayout = new QGridLayout(pFlickerBlendGroup);

   mpFramerateKnob = new QwtKnob(pFlickerBlendGroup);
   // 15fps is usually plenty for the typical use case
   // and a larger range on the knob makes it difficult to
   // adjust since 1fps is a very small movement.
   mpFramerateKnob->setRange(0.0, 15.0, 1);
   mpFramerateKnob->setScale(0, 15, 3);
   pFbgLayout->addWidget(mpFramerateKnob, 0, 0, 4, 1);

   QLabel* pAutoFlickerRateLabel = new QLabel("Automated Flicker Rate (fps):", pFlickerBlendGroup);
   pFbgLayout->addWidget(pAutoFlickerRateLabel, 0, 1, 1, 2);
   mpFlickerRate = new QDoubleSpinBox(pFlickerBlendGroup);
   mpFlickerRate->setRange(0.0, 15.0);
   mpFlickerRate->setSingleStep(0.1);
   pFbgLayout->addWidget(mpFlickerRate, 1, 1, 1, 2);
   QLabel* pAutoFlickerLabel = new QLabel("Automated Flicker:", pFlickerBlendGroup);
   pFbgLayout->addWidget(pAutoFlickerLabel, 2, 1);
   mpAutoFlickerButton = new QPushButton(pFlickerBlendGroup);
   mpAutoFlickerButton->setToolTip(
      "When set to On, the layer will automatically flicker on and off at the predetermined rate.");
   mpAutoFlickerButton->setCheckable(true);
   mpAutoFlickerButton->setChecked(false);
   mpAutoFlickerButton->setFlat(true);
   mpAutoFlickerButton->setFixedSize(75,32);
   pFbgLayout->addWidget(mpAutoFlickerButton, 3, 1);

   QLabel* pManualFlickerLabel = new QLabel("Manual Flicker:", pFlickerBlendGroup);
   pFbgLayout->addWidget(pManualFlickerLabel, 2, 2);
   mpManualFlickerButton = new QPushButton(pFlickerBlendGroup);
   mpManualFlickerButton->setToolTip("Indicates the current visibility of the layer.");
   mpManualFlickerButton->setCheckable(true);
   mpManualFlickerButton->setFlat(true);
   mpManualFlickerButton->setFixedSize(75,32);
   pFbgLayout->addWidget(mpManualFlickerButton, 3, 2);

   QFrame* pLine = new QFrame(pFlickerBlendGroup);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   pFbgLayout->addWidget(pLine, 4, 0, 1, 3);

   mpTransparencyLabel = new QLabel("Transparency (% opacity):", pFlickerBlendGroup);
   pFbgLayout->addWidget(mpTransparencyLabel, 5, 0, 1, 3);
   mpAlpha = new QwtSlider(pFlickerBlendGroup);
   mpAlpha->setAutoFillBackground(false);
   mpAlpha->setScalePosition(QwtSlider::BottomScale);
   mpAlpha->setBgStyle(QwtSlider::BgSlot);
   pFbgLayout->addWidget(mpAlpha, 6, 0, 1, 3);

   pFbgLayout->setRowStretch(7, 10);
   pFbgLayout->setColumnStretch(3, 10);

   pTopLevel->addLayout(pLeftSideLayout);
   pTopLevel->addWidget(pFlickerBlendGroup);

   setStyleSheet("QPushButton {"\
         "background-image: url(:/images/Toggle-switch-off);"\
         "background-repeat: no-repeat;"\
         "background-clip: margin;"\
         "margin: 0;"\
         "border: 0;"\
      "}"\
      "QPushButton:checked {"\
         "background-image: url(:/images/Toggle-switch-on) }"\
      "QPushButton:disabled {"\
         "background-image: url(:/images/Toggle-switch-off-disabled) }"\
      "QPushButton:checked:disabled {"\
         "background-image: url(:/images/Toggle-switch-on-disabled) }");

   mpTimer = new QTimer(this);

   VERIFYNR(connect(mpFramerateKnob, SIGNAL(valueChanged(double)), mpFlickerRate, SLOT(setValue(double))));
   VERIFYNR(connect(mpFlickerRate, SIGNAL(valueChanged(double)), mpFramerateKnob, SLOT(setValue(double))));
   VERIFYNR(connect(mpXOffsetWheel, SIGNAL(valueChanged(double)), this, SLOT(updateOffset(double))));
   VERIFYNR(connect(mpYOffsetWheel, SIGNAL(valueChanged(double)), this, SLOT(updateOffset(double))));
   VERIFYNR(connect(mpLayerSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(updateControls())));
   VERIFYNR(connect(mpAlpha, SIGNAL(valueChanged(double)), this, SLOT(updateAlpha(double))));
   VERIFYNR(connect(mpAutoFlickerButton, SIGNAL(toggled(bool)), this, SLOT(toggleAutoFlicker(bool))));
   VERIFYNR(connect(mpManualFlickerButton, SIGNAL(toggled(bool)), this, SLOT(changeVisibility(bool))));
   VERIFYNR(connect(mpXOffset, SIGNAL(valueChanged(double)), this, SLOT(changeLayerOffset(double))));
   VERIFYNR(connect(mpYOffset, SIGNAL(valueChanged(double)), this, SLOT(changeLayerOffset(double))));
   VERIFYNR(connect(mpXScale, SIGNAL(valueChanged(double)), this, SLOT(changeLayerScale(double))));
   VERIFYNR(connect(mpYScale, SIGNAL(valueChanged(double)), this, SLOT(changeLayerScale(double))));
   VERIFYNR(connect(mpFlickerRate, SIGNAL(valueChanged(double)), this, SLOT(changeFlickerRate(double))));
   VERIFYNR(connect(mpTimer, SIGNAL(timeout()), this, SLOT(changeVisibility())));

   mpFramerateKnob->setValue(2.0);
   changeFlickerRate(2.0);

   SpatialDataView* pView = (pWindow == NULL) ? NULL : dynamic_cast<SpatialDataView*>(pWindow->getView());
   mpView.reset(pView);
   mpLayerList.reset((pView == NULL) ? NULL : pView->getLayerList());
   resetLayers();
}

ImageAdjustWidget::~ImageAdjustWidget()
{}

void ImageAdjustWidget::windowActivated(Subject& subject, const std::string& signal, const boost::any& value)
{
   WorkspaceWindow* pWindow = boost::any_cast<WorkspaceWindow*>(value);
   SpatialDataView* pView = (pWindow == NULL) ? NULL : dynamic_cast<SpatialDataView*>(pWindow->getView());
   mpView.reset(pView);
   mpLayerList.reset((pView == NULL) ? NULL : pView->getLayerList());
   resetLayers();
}

void ImageAdjustWidget::layerListChanged(Subject& subject, const std::string& signal, const boost::any& value)
{
   resetLayers();
}

void ImageAdjustWidget::layerVisibilityChanged(Subject& subject, const std::string& signal, const boost::any& value)
{
   Layer* pLayer = boost::any_cast<Layer*>(value);
   if (pLayer != NULL && pLayer == getLayer())
   {
      if (signal == SIGNAL_NAME(SpatialDataView, LayerShown))
      {
         mpManualFlickerButton->setChecked(true);
      }
      else if (signal == SIGNAL_NAME(SpatialDataView, LayerHidden))
      {
         mpManualFlickerButton->setChecked(false);
      }
   }
}

void ImageAdjustWidget::layerPropertyChanged(Subject& subject, const std::string& signal, const boost::any& value)
{
   updateControls();
}

void ImageAdjustWidget::updateOffset(double value)
{
   QwtWheel* pSender = dynamic_cast<QwtWheel*>(sender());
   if (pSender == mpXOffsetWheel)
   {
      double delta = value - mXOffsetPrev;
      if (delta > (mpXOffsetWheel->maxValue() / 2))
      {
         delta -= mpXOffsetWheel->maxValue();
      }
      else if (-delta > (mpXOffsetWheel->maxValue() / 2))
      {
         delta += mpXOffsetWheel->maxValue();
      }
      mpXOffset->setValue(mpXOffset->value() + delta);
      mXOffsetPrev = value;
   }
   else if (pSender == mpYOffsetWheel)
   {
      double delta = value - mYOffsetPrev;
      if (delta > (mpYOffsetWheel->maxValue() / 2))
      {
         delta -= mpYOffsetWheel->maxValue();
      }
      else if (-delta > (mpYOffsetWheel->maxValue() / 2))
      {
         delta += mpYOffsetWheel->maxValue();
      }
      mpYOffset->setValue(mpYOffset->value() + delta);
      mYOffsetPrev = value;
   }
}

void ImageAdjustWidget::resetLayers()
{
   if (mpLayerList.get() == NULL)
   {
      mpLayerSelection->clear();
      mpLayerAttachment.reset(NULL);
      setEnabled(false);
      return;
   }
   setEnabled(true);
   int selected = mpLayerSelection->currentIndex();
   std::vector<Layer*> layers = mpLayerList->getLayers();
   mpLayerSelection->clear();
   for (std::vector<Layer*>::const_iterator layer = layers.begin(); layer != layers.end(); ++layer)
   {
      mpLayerSelection->addItem(QString::fromStdString((*layer)->getDisplayName(true)), QVariant::fromValue(*layer));
   }
   if (selected >= 0 && selected < mpLayerSelection->count())
   {
      mpLayerSelection->setCurrentIndex(selected);
   }
   else if (layers.size() > 1)
   {
      // default to the second layer
      // This is consistent with the behavior of the old
      // flicker controls and the data fusion plug-in.
      // Also, it is generally preferable to not change the
      // offset of the primary layer (which is often the first layer)
      // as this can lead to off behavior. Defaulting to layer two
      // ensures that the user must purposefully select the first layer
      // before making adjustments.
      mpLayerSelection->setCurrentIndex(1);
   }
   updateControls();
}

void ImageAdjustWidget::updateControls()
{
   Layer* pLayer = getLayer();
   if (pLayer != mpLayerAttachment.get())
   {
      mpLayerAttachment.reset(pLayer);
   }
   if (pLayer == NULL)
   {
      return;
   }
   mpXOffset->setValue(pLayer->getXOffset());
   mpYOffset->setValue(pLayer->getYOffset());
   mpXScale->setValue(pLayer->getXScaleFactor());
   mpYScale->setValue(pLayer->getYScaleFactor());
   mpManualFlickerButton->setChecked(mpView->isLayerDisplayed(pLayer));
   mpAutoFlickerButton->setChecked(false);
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
   if (pRasterLayer != NULL)
   {
      mpTransparencyLabel->setEnabled(true);
      mpAlpha->setEnabled(true);
      mpAlpha->setValue(pRasterLayer->getAlpha() / 2.55); // scale is 0-99
   }
   else
   {
      mpTransparencyLabel->setEnabled(false);
      mpAlpha->setEnabled(false);
      mpAlpha->setValue(100);
   }
}

void ImageAdjustWidget::updateAlpha(double value)
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(getLayer());
   if (pRasterLayer != NULL)
   {
      // deal with rounding issues and ensure it's bound to 0-255
      unsigned int alpha = (2.55 * value) + 0.5;
      pRasterLayer->setAlpha(std::min(255u, alpha));
   }
}

void ImageAdjustWidget::changeFlickerRate(double rate)
{
   mpTimer->setInterval(1000 / rate);
}

void ImageAdjustWidget::changeVisibility(bool visible)
{
   Layer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      if (visible)
      {
         mpView->showLayer(pLayer);
      }
      else
      {
         mpView->hideLayer(pLayer);
      }
   }
}

void ImageAdjustWidget::changeVisibility()
{
   Layer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      if (mpView->isLayerDisplayed(pLayer))
      {
         mpView->hideLayer(pLayer);
      }
      else
      {
         mpView->showLayer(pLayer);
      }
   }
}

void ImageAdjustWidget::toggleAutoFlicker(bool state)
{
   if (state)
   {
      mpManualFlickerButton->setEnabled(false);
      mpTimer->start();
   }
   else
   {
      mpManualFlickerButton->setEnabled(true);
      Layer* pLayer = getLayer();
      if (pLayer != NULL)
      {
         mpManualFlickerButton->setChecked(mpView->isLayerDisplayed(pLayer));
      }
      mpTimer->stop();
   }
}

void ImageAdjustWidget::changeLayerOffset(double value)
{
   Layer* pLayer = getLayer();
   QDoubleSpinBox* pSender = dynamic_cast<QDoubleSpinBox*>(sender());
   if (pLayer != NULL && pSender == mpXOffset)
   {
      pLayer->setXOffset(value);
   }
   else if (pLayer != NULL && pSender == mpYOffset)
   {
      pLayer->setYOffset(value);
   }
}

void ImageAdjustWidget::changeLayerScale(double value)
{
   Layer* pLayer = getLayer();
   QDoubleSpinBox* pSender = dynamic_cast<QDoubleSpinBox*>(sender());
   if (pLayer != NULL && pSender == mpXScale)
   {
      pLayer->setXScaleFactor(value);
   }
   else if (pLayer != NULL && pSender == mpYScale)
   {
      pLayer->setYScaleFactor(value);
   }
}

Layer* ImageAdjustWidget::getLayer() const
{
   return qvariant_cast<Layer*>(mpLayerSelection->itemData(mpLayerSelection->currentIndex()));
}

bool ImageAdjustWidget::serialize(SessionItemSerializer& serializer) const
{
   XMLWriter writer("ImageAdjustWidget");
   if (mpView.get() != NULL)
   {
      writer.pushAddPoint(writer.addElement("ViewId"));
      writer.addText(mpView->getId());
      writer.popAddPoint();
   }
   VERIFY(mpFlickerRate != NULL);
   writer.addAttr("flickerRate", StringUtilities::toXmlString(mpFlickerRate->value()));

   return serializer.serialize(writer);
}

bool ImageAdjustWidget::deserialize(SessionItemDeserializer& deserializer)
{
   XmlReader reader(NULL, false);

   DOMElement* pRootElement = deserializer.deserialize(reader, "ImageAdjustWidget");
   if (pRootElement == NULL)
   {
      return false;
   }

   for (DOMNode* pChild = pRootElement->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      DOMElement* pElement = static_cast<DOMElement*>(pChild);
      if (XMLString::equals(pChild->getNodeName(), X("ViewId")))
      {
         std::string viewid = A(pChild->getTextContent());
         SpatialDataView* pView = dynamic_cast<SpatialDataView*>(Service<SessionManager>()->getSessionItem(viewid));
         if (pView != NULL)
         {
            mpView.reset(pView);
            mpLayerList.reset(pView->getLayerList());
            resetLayers();
         }
      }
   }
   double flickerRate = StringUtilities::fromXmlString<double>(A(pRootElement->getAttribute(X("flickerRate"))));
   mpFlickerRate->setValue(flickerRate);

   return true;
}
