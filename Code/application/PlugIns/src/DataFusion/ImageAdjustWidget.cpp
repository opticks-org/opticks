/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLCDNumber>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>

#include "AttachmentPtr.h"
#include "AppVerify.h"
#include "ImageAdjustWidget.h"
#include "Layer.h"
#include "LayerList.h"
#include "RasterLayer.h"
#include "RasterElement.h"
#include "Slot.h"
#include "RasterDataDescriptor.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

#include <limits>
using namespace std;

namespace
{
   inline int pctToAlpha(int pct)
   {
      if (pct > 100)
      {
         return 255;
      }
      if (pct < 0)
      {
         return 0;
      }
      return (pct*255)/100;
   }
}

const float INTERVAL = 1000;
const int MAX_FLICKER_RATE = 30;
const QString NO_LAYER = "None";

ImageAdjustWidget::ImageAdjustWidget(WorkspaceWindow* pWindow, QWidget* pParent) :
   QWidget(pParent),
   mpPrimaryLayer(NULL),
   mpSecondaryLayer(NULL),
   mpDesktopServices(Service<DesktopServices>().get(), SIGNAL_NAME(DesktopServices, WindowActivated),
      Slot(this, &ImageAdjustWidget::windowActivated))
{
   setEnabled(false);
   mpTimer = new QTimer(this);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);

   QGridLayout* pCurrentLayerLayout = new QGridLayout();
   pCurrentLayerLayout->setRowStretch(0, 10);
   pLayout->addLayout(pCurrentLayerLayout);
  
   QLabel* pLayerLabel = new QLabel("Current Flicker Layer: ", this);
   pCurrentLayerLayout->addWidget(pLayerLabel, 1, 0);

   mpCurrentLayerLabel = new QLabel(NO_LAYER, this);
   pCurrentLayerLayout->addWidget(mpCurrentLayerLabel, 1, 1, Qt::AlignLeft);

   QLabel* pCubeLayerLabel = new QLabel("Data Cube Layer: ", this);
   pCurrentLayerLayout->addWidget(pCubeLayerLabel, 2, 0);

   mpCurrentCubeLayerLabel = new QLabel(NO_LAYER, this);
   pCurrentLayerLayout->addWidget(mpCurrentCubeLayerLabel, 2, 1, Qt::AlignLeft);

   QGroupBox* pFlickerBox = new QGroupBox("Flicker / Blend Tools", this);
   pLayout->addWidget(pFlickerBox, 10);

   // Bottom Part - Flicker Tools
   QVBoxLayout* pFlickerToolLayout = new QVBoxLayout(pFlickerBox);
   pFlickerToolLayout->setMargin(10);
   pFlickerToolLayout->setSpacing(10);

   // top part - flicker tools
   QHBoxLayout* pFlickerRateLayout = new QHBoxLayout();
   pFlickerRateLayout->setMargin(0);
   pFlickerRateLayout->setSpacing(5);

   pFlickerToolLayout->addLayout(pFlickerRateLayout);

   mpAutomatedFlickerBox = new QGroupBox("Automated Flicker", pFlickerBox);
   pFlickerRateLayout->addWidget(mpAutomatedFlickerBox, 10);
   QVBoxLayout* pAutomatedFlickerLayout = new QVBoxLayout(mpAutomatedFlickerBox);
   pAutomatedFlickerLayout->setMargin(10);
   pAutomatedFlickerLayout->setSpacing(5);

   // Automated flicker section
   // TOP ROW - Requested Flicker Rate
   QHBoxLayout* pRequestedRateLayout = new QHBoxLayout();
   pRequestedRateLayout->setMargin(0);
   pRequestedRateLayout->setSpacing(5);

   pAutomatedFlickerLayout->addLayout(pRequestedRateLayout);

   QLabel* pRequestedFlickerRateLabel = new QLabel("Requested Flicker Rate (fps):", mpAutomatedFlickerBox);

   pRequestedRateLayout->addWidget(pRequestedFlickerRateLabel);

   mpCurrentFlickerSpeed = new QLabel("0", mpAutomatedFlickerBox);
   pRequestedRateLayout->addWidget(mpCurrentFlickerSpeed);

   pRequestedRateLayout->addStretch();

   // MIDDLE ROW - Flicker Rate Slider & Labels
   QHBoxLayout* pSliderLayout = new QHBoxLayout();
   pSliderLayout->setMargin(0);
   pSliderLayout->setSpacing(5);

   pAutomatedFlickerLayout->addLayout(pSliderLayout);

   QLabel* pMinimumFlickerRateLabel = new QLabel("0", mpAutomatedFlickerBox);
   pSliderLayout->addWidget(pMinimumFlickerRateLabel);

   mpFlickerSlider = new QSlider(mpAutomatedFlickerBox);
   mpFlickerSlider->setFocusPolicy(Qt::WheelFocus);
   mpFlickerSlider->setMaximum(100);
   mpFlickerSlider->setPageStep(5);
   mpFlickerSlider->setOrientation(Qt::Horizontal);
   mpFlickerSlider->setTickPosition(QSlider::NoTicks);
   mpFlickerSlider->setMinimumWidth(200);
   pSliderLayout->addWidget(mpFlickerSlider, 10);

   mpFlickerMax = new QLabel(QString::number(MAX_FLICKER_RATE), mpAutomatedFlickerBox);
   pSliderLayout->addWidget(mpFlickerMax);

   pSliderLayout->addStretch();

   // BOTTOM ROW - flicker controls - start/stop
   QHBoxLayout* pControlLayout = new QHBoxLayout();
   pControlLayout->setMargin(0);
   pControlLayout->setSpacing(5);

   pAutomatedFlickerLayout->addLayout(pControlLayout);

   QPushButton* pStartFlickerButton = new QPushButton("Start", mpAutomatedFlickerBox);
   pControlLayout->addWidget(pStartFlickerButton);

   QPushButton* pStopFlickerButton = new QPushButton("Stop", mpAutomatedFlickerBox);
   pControlLayout->addWidget(pStopFlickerButton);

   pControlLayout->addStretch();

   // Manual flicker section
   mpManualFlickerBox = new QGroupBox("Manual Flicker", pFlickerBox);
   mpManualFlicker = new QPushButton("Toggle", mpManualFlickerBox);
   mpManualFlicker->setEnabled(true);
   mpManualFlicker->setCheckable(true);

   QVBoxLayout* pManualLayout = new QVBoxLayout(mpManualFlickerBox);
   pManualLayout->setMargin(10);
   pManualLayout->setSpacing(5);
   pManualLayout->addWidget(mpManualFlicker);
   pManualLayout->addStretch();

   pFlickerRateLayout->addWidget(mpManualFlickerBox, 0, Qt::AlignTop);

   QFrame* pTransparencySeparator = new QFrame(pFlickerBox);
   pTransparencySeparator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   pFlickerToolLayout->addWidget(pTransparencySeparator);

   QHBoxLayout* pAlphaChannelLayout = new QHBoxLayout();
   pAlphaChannelLayout->setMargin(0);
   pAlphaChannelLayout->setSpacing(5);

   pFlickerToolLayout->addLayout(pAlphaChannelLayout);

   QLabel* pTransparencyLabel = new QLabel("Transparent", pFlickerBox);
   pAlphaChannelLayout->addWidget(pTransparencyLabel);

   mpOpacitySlider = new QSlider(pFlickerBox);
   mpOpacitySlider->setMaximum(100);
   mpOpacitySlider->setValue(100);
   mpOpacitySlider->setOrientation(Qt::Horizontal);
   pAlphaChannelLayout->addWidget(mpOpacitySlider, 10);

   QLabel* pOpaqueLabel = new QLabel("Opaque", pFlickerBox);
   pAlphaChannelLayout->addWidget(pOpaqueLabel);

   // Bottom part - Translation sliders
   QFrame* pTranslationSeparator = new QFrame(pFlickerBox);
   pTranslationSeparator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   pFlickerToolLayout->addWidget(pTranslationSeparator);

   QGridLayout* pTranslationLayout = new QGridLayout();
   pTranslationLayout->setMargin(0);
   pTranslationLayout->setSpacing(5);

   pFlickerToolLayout->addLayout(pTranslationLayout);

   QLabel* mpFlickerXLabel = new QLabel("X Shift", pFlickerBox);
   pTranslationLayout->addWidget(mpFlickerXLabel, 0, 0);

   QLabel* mpFlickerYLabel = new QLabel("Y Shift", pFlickerBox);
   pTranslationLayout->addWidget(mpFlickerYLabel, 0, 3);

   mpFlickerXSlider = new QSlider(pFlickerBox);
   mpFlickerXSlider->setMouseTracking(true);
   mpFlickerXSlider->setFocusPolicy(Qt::WheelFocus);
   mpFlickerXSlider->setMinimum(-100);
   mpFlickerXSlider->setMaximum(100);
   mpFlickerXSlider->setPageStep(1);
   mpFlickerXSlider->setOrientation(Qt::Horizontal);
   mpFlickerXSlider->setTickPosition(QSlider::NoTicks);
   mpFlickerXSlider->setMinimumWidth(125);
   pTranslationLayout->addWidget(mpFlickerXSlider, 1, 0, 1, 2);

   QLCDNumber* pFlickerXDisp = new QLCDNumber(pFlickerBox);

   QPalette flickerXPalette = pFlickerXDisp->palette();
   flickerXPalette.setColor(QPalette::Window, QColor(135, 135, 135));
   pFlickerXDisp->setPalette(flickerXPalette);

   pTranslationLayout->addWidget(pFlickerXDisp, 1, 2);

   mpFlickerYSlider = new QSlider(pFlickerBox);
   mpFlickerYSlider->setMouseTracking(true);
   mpFlickerYSlider->setFocusPolicy(Qt::WheelFocus);
   mpFlickerYSlider->setMinimum(-100);
   mpFlickerYSlider->setMaximum(100);
   mpFlickerYSlider->setSingleStep(1);
   mpFlickerYSlider->setPageStep(1);
   mpFlickerYSlider->setOrientation(Qt::Horizontal);
   mpFlickerYSlider->setTickPosition(QSlider::NoTicks);
   mpFlickerYSlider->setMinimumWidth(125);
   pTranslationLayout->addWidget(mpFlickerYSlider, 1, 3, 1, 2);

   QLCDNumber* pFlickerYDisp = new QLCDNumber(pFlickerBox);

   QPalette flickerYPalette = pFlickerYDisp->palette();
   flickerYPalette.setColor(QPalette::Window, QColor(135, 135, 135));
   pFlickerYDisp->setPalette(flickerYPalette);

   pTranslationLayout->addWidget(pFlickerYDisp, 1, 5);

   pTranslationLayout->setColumnStretch(0, 10);
   pTranslationLayout->setColumnStretch(3, 10);

   resize(minimumSizeHint());

   // Internal signal and slot connections
   connect(mpFlickerXSlider, SIGNAL(valueChanged(int)), pFlickerXDisp, SLOT(display(int)));
   connect(mpFlickerYSlider, SIGNAL(valueChanged(int)), pFlickerYDisp, SLOT(display(int)));
   connect(mpFlickerSlider, SIGNAL(valueChanged(int)), this, SLOT(changeFlickerRate(int)));
   connect(mpFlickerXSlider, SIGNAL(valueChanged(int)), this, SLOT(shift(int)));
   connect(mpFlickerYSlider, SIGNAL(valueChanged(int)), this, SLOT(shift(int)));
   connect(mpOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(changeFlickerAlpha(int)));
   connect(mpManualFlicker, SIGNAL(toggled(bool)), this, SLOT(runFlicker()));
   connect(pStartFlickerButton, SIGNAL(clicked()), this, SLOT(startFlicker()));
   connect(pStopFlickerButton, SIGNAL(clicked()), this, SLOT(stopFlicker()));
   connect(mpTimer, SIGNAL(timeout()), this, SLOT(runFlicker()));


   double maxRate = (mpFlickerMax->text().toDouble());
   mpFlickerMax->setNum(static_cast<int>(maxRate));
   maxRate = log(maxRate)*INTERVAL;
   mpFlickerSlider->setMaximum(maxRate);

   // mpLayerList initialization.
   setLayerList(pWindow);
   mpLayerList.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &ImageAdjustWidget::layerListChanged));
   mpLayerList.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &ImageAdjustWidget::layerListChanged));
}

ImageAdjustWidget::~ImageAdjustWidget()
{
   // Nothing needs to be done here.
}

void ImageAdjustWidget::runFlicker()
{
   if (mpSecondaryLayer != NULL)
   {
      if (mpSecondaryLayer->getAlpha() == 0)
      {
         mpSecondaryLayer->setAlpha(pctToAlpha(mpOpacitySlider->value()));
      }
      else
      {
         mpSecondaryLayer->setAlpha(pctToAlpha(0));
      }
   }
}

void ImageAdjustWidget::startFlicker()
{
   // rate is > 0 and timer is NOT active, so activate it
   double fps = computeFlickerRate(mpFlickerSlider->value());
   if (fps > 0)
   {
      if (mpTimer != NULL && !mpTimer->isActive())
      {
         mpTimer->start(1000/fps); // convert from fps -> ms between frames
      }
   }
   else
   {
      stopFlicker();
   }
}

void ImageAdjustWidget::stopFlicker()
{
   if (mpTimer != NULL)
   {
      mpTimer->stop();
   }
}

void ImageAdjustWidget::changeFlickerAlpha(int position)
{
   if (mpSecondaryLayer != NULL)
   {
      mpSecondaryLayer->setAlpha(pctToAlpha(position));
   }
}

double ImageAdjustWidget::computeFlickerRate(int position) const
{
   double maxFrameRate = mpFlickerMax->text().toInt(); // maximum frame rate
   double maxSlider = mpFlickerSlider->maximum(); // number of positions on slider
   double fps = 0;
   if (position > 0)
   {
      fps = exp(static_cast<double>(position)/INTERVAL)-1;
   }

   fps = (fps/(maxFrameRate-1))*maxFrameRate;
   // get 2 decimal places for fps meter
   fps = static_cast<int>(fps*100)/static_cast<double>(100);
   return fps;
}

void ImageAdjustWidget::changeFlickerRate(int position)
{
   double fps = computeFlickerRate(position);
   // interval in ms between frames = 1000/fps

   if (mpTimer != NULL)
   {
      if (fps > 0 && mpTimer->isActive())
      {
         // is already active, so just change the rate
         mpTimer->setInterval(1000/fps); // convert from fps -> ms between frames
      }

      // allow manual control iff fps == 0
      mpManualFlicker->setEnabled(fps == 0);
   }
   mpCurrentFlickerSpeed->setNum(fps);
}

void ImageAdjustWidget::shift(int value)
{
   stopFlicker();
   if (mpSecondaryLayer != NULL)
   {
      const QSlider* pSender = static_cast<const QSlider*>(sender());
      if (pSender == mpFlickerXSlider)
      {
         mpSecondaryLayer->setXOffset(value);
      }
      else if (pSender == mpFlickerYSlider)
      {
         mpSecondaryLayer->setYOffset(value);
      }

      View* pView = mpSecondaryLayer->getView();
      if (NN(pView))
      {
         pView->refresh();
      }
   }
   startFlicker();
}

void ImageAdjustWidget::windowActivated(Subject& subject, const string& signal, const boost::any& value)
{
   setLayerList(boost::any_cast<WorkspaceWindow*>(value));
}

void ImageAdjustWidget::layerListChanged(Subject& subject, const string& signal, const boost::any& value)
{
   resetWidgets();
}


string ImageAdjustWidget::getLayerName(RasterLayer* pRasterLayer)
{
   if (pRasterLayer == NULL)
   {
      return NO_LAYER.toStdString();
   }
   else
   {
      return pRasterLayer->getName();
   }
}

DataElement* ImageAdjustWidget::getDataElement(RasterLayer* pRasterLayer)
{
   if (pRasterLayer == NULL)
   {
      return NULL;
   }
   else
   {
      return pRasterLayer->getDataElement();
   }
}

void ImageAdjustWidget::resetLabels(string layerName, string cubeLayerName)
{
   VERIFYNR(layerName.empty() == false);
   mpCurrentLayerLabel->setText(layerName.c_str());

   VERIFYNR(cubeLayerName.empty() == false);
   mpCurrentCubeLayerLabel->setText(cubeLayerName.c_str());

}

void ImageAdjustWidget::resetWidgets()
{
   resetLayers();
   resetSliders();
   resetLabels(getLayerName(mpPrimaryLayer), getLayerName(mpSecondaryLayer));
   setEnabled(mpPrimaryLayer != NULL && mpSecondaryLayer != NULL);
}

void ImageAdjustWidget::setLayerList(WorkspaceWindow* pWindow)
{
   LayerList* pLayerList = NULL;

   SpatialDataWindow* pSpatialDataWindow = dynamic_cast<SpatialDataWindow*>(pWindow);
   if (pSpatialDataWindow != NULL)
   {
      SpatialDataView* pSpatialDataView = pSpatialDataWindow->getSpatialDataView();
      if (pSpatialDataView != NULL)
      {
         pLayerList = pSpatialDataView->getLayerList();
      }
   }

   if (pLayerList != mpLayerList.get())
   {
      mpLayerList.reset(pLayerList);
      mpPrimaryLayer = NULL;
      mpSecondaryLayer = NULL;
      resetWidgets();
   }
}

void ImageAdjustWidget::resetLayers()
{
   // Initialize mpPrimaryLayer and mpSecondaryLayer to known values.
   mpPrimaryLayer = NULL;
   mpSecondaryLayer = NULL;

   // Get a pointer to the current LayerList object.
   LayerList* pLayerList = mpLayerList.get();
   if (pLayerList != NULL)
   {
      // Retrieve all raster layers from pLayerList.
      vector<Layer*> rasterLayers;
      pLayerList->getLayers(LayerType(RASTER), rasterLayers);

      // Set the first layer to mpPrimaryLayer.
      if (rasterLayers.empty() == false)
      {
         mpPrimaryLayer = dynamic_cast<RasterLayer*>(rasterLayers.front());

         // Set the second layer to mpSecondaryLayer.
         if (rasterLayers.size() > 1)
         {
            mpSecondaryLayer = dynamic_cast<RasterLayer*>(rasterLayers[1]);
         }
      }
   }
}

void ImageAdjustWidget::resetSliders()
{
   RasterElement* pPrimaryRaster = dynamic_cast<RasterElement*>(getDataElement(mpPrimaryLayer));
   RasterElement* pSecondaryRaster = dynamic_cast<RasterElement*>(getDataElement(mpSecondaryLayer));
   if (pSecondaryRaster != NULL) // use the RasterElement bounds by default
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pSecondaryRaster->getDataDescriptor());
      if (NN(pDescriptor))
      {
         // update translation bars based on the RasterLayer's RasterElement first
         // also need to update extrema based on the scale factors since resample() is not being called
         int numRows = static_cast<int>(pDescriptor->getRowCount());
         numRows *= mpSecondaryLayer->getYScaleFactor();
         int numColumns = static_cast<int>(pDescriptor->getColumnCount());
         numColumns *= mpSecondaryLayer->getXScaleFactor();
         if (pPrimaryRaster != NULL) // if the primary exists below the secondary, use its bounds instead
         {
            pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pPrimaryRaster->getDataDescriptor());
            if (NN(pDescriptor))
            {
               numRows = static_cast<int>(pDescriptor->getRowCount());
               numColumns = static_cast<int>(pDescriptor->getColumnCount());
               if (mpPrimaryLayer != NULL)
               {
                  numRows *= mpPrimaryLayer->getYScaleFactor();
                  numColumns *= mpPrimaryLayer->getXScaleFactor();
               }
            }
         }

         VERIFYNR(disconnect(mpFlickerXSlider, SIGNAL(valueChanged(int)), this, SLOT(shift(int))));
         VERIFYNR(disconnect(mpFlickerYSlider, SIGNAL(valueChanged(int)), this, SLOT(shift(int))));

         // widen the slider ranges to accomodate a new offset
         mpFlickerYSlider->setMinimum(min(-numRows, static_cast<int>(mpSecondaryLayer->getYOffset())));
         mpFlickerYSlider->setMaximum(max(numRows, static_cast<int>(mpSecondaryLayer->getYOffset())));
         mpFlickerXSlider->setMinimum(min(-numColumns, static_cast<int>(mpSecondaryLayer->getXOffset())));
         mpFlickerXSlider->setMaximum(max(numColumns, static_cast<int>(mpSecondaryLayer->getXOffset())));

         // fetch & set the offsets
         mpFlickerYSlider->setValue(mpSecondaryLayer->getYOffset());
         mpFlickerXSlider->setValue(mpSecondaryLayer->getXOffset());

         VERIFYNR(connect(mpFlickerXSlider, SIGNAL(valueChanged(int)), this, SLOT(shift(int))));
         VERIFYNR(connect(mpFlickerYSlider, SIGNAL(valueChanged(int)), this, SLOT(shift(int))));
      }
   }
}
