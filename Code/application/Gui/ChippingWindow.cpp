/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtCore/QFileInfo>
#include <QtGui/QGroupBox>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "ChippingWindow.h"
#include "ChippingWidget.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DesktopServicesImp.h"
#include "DimensionDescriptor.h"
#include "Icons.h"
#include "LayerList.h"
#include "ObjectResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "RasterLayerImp.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Undo.h"

#include <string>
using namespace std;

ChippingWindow::ChippingWindow(SpatialDataView* pView, QWidget* parent) :
   QDialog(parent),
   mpView(pView),
   mpChippingWidget(NULL),
   mpWindowRadio(NULL),
   mpFileRadio(NULL)
{
   // View widget
   SpatialDataView* pChipView = createChipView();
   pChipView->getWidget()->installEventFilter(this);
   mpChippingWidget = new ChippingWidget(pChipView, this);

   // Chip mode
   QGroupBox* pModeGroup = new QGroupBox("Chipping Mode", this);
   mpWindowRadio = new QRadioButton("Create new window", pModeGroup);
   mpFileRadio = new QRadioButton("Export to file", pModeGroup);

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Layout
   QVBoxLayout* pModeLayout = new QVBoxLayout(pModeGroup);
   pModeLayout->setMargin(10);
   pModeLayout->setSpacing(5);
   pModeLayout->addWidget(mpWindowRadio);
   pModeLayout->addWidget(mpFileRadio);

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(pOkButton);
   pButtonLayout->addWidget(pCancelButton);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(mpChippingWidget, 10);
   pLayout->addWidget(pModeGroup, 0, Qt::AlignLeft);
   pLayout->addWidget(pHLine);
   pLayout->addLayout(pButtonLayout);

   // Initialization
   setModal(true);
   resize(400, 200);
   mpWindowRadio->setChecked(true);

   // Set the window icon
   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   setWindowIcon(QIcon(pIcons->mChipImage));

   // Set the caption of the dialog
   QString strCaption = "Create Image Chip";
   if (mpView != NULL)
   {
      string viewName = mpView->getName();
      if (viewName.empty() == false)
      {
         QFileInfo fileInfo(QString::fromStdString(viewName));
         QString strFilename = fileInfo.fileName();
         if (strFilename.isEmpty() == false)
         {
            strCaption += ": " + strFilename;
         }
      }
   }

   setWindowTitle(strCaption);

   // Connections
   connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

ChippingWindow::~ChippingWindow()
{
}

SpatialDataView* ChippingWindow::createChipView() const
{
   if (mpView == NULL)
   {
      return NULL;
   }

   SpatialDataView* pChipView = NULL;
   pChipView = (SpatialDataView*) (mpView->copy());
   if (pChipView != NULL)
   {
      vector<std::pair<View*,LinkType> > linkedViews;
      pChipView->getLinkedViews(linkedViews);
      for (unsigned int i = 0; i < linkedViews.size(); ++i)
      {
         if (linkedViews[i].second == NO_LINK)
         {
            continue;
         }

         pChipView->unlinkView(linkedViews[i].first);
      }

      LayerList* pLayerList = NULL;
      pLayerList = pChipView->getLayerList();
      if (pLayerList != NULL)
      {
         vector<Layer*> layers;
         pLayerList->getLayers(layers);
         for (unsigned int i = 0; i < layers.size(); i++)
         {
            Layer* pLayer = layers.at(i);
            if (dynamic_cast<RasterLayer*>(pLayer) == NULL)
            {
               pChipView->deleteLayer(pLayer);
            }
         }
      }
   }

   return pChipView;
}

void ChippingWindow::createFile()
{
   if (mpChippingWidget == NULL)
   {
      return;
   }

   RasterElement* pRaster = getRasterElement();
   if (pRaster == NULL)
   {
      return;
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   // Rows
   const vector<DimensionDescriptor>& rows = mpChippingWidget->getChipRows();
   const DimensionDescriptor startRow = rows.front();
   const DimensionDescriptor stopRow = rows.back();

   // Columns
   const vector<DimensionDescriptor>& columns = mpChippingWidget->getChipColumns();
   const DimensionDescriptor startCol = columns.front();
   const DimensionDescriptor stopCol = columns.back();

   // Create a file descriptor based on the sensor data with the chip rows and columns
   FactoryResource<FileDescriptor> fileDescriptor( 
                   RasterUtilities::generateFileDescriptorForExport(pDescriptor, "", 
                   startRow, stopRow, 0, startCol, stopCol, 0));

   Service<DesktopServices> pDesktop;
   pDesktop->exportSessionItem(pRaster, fileDescriptor.get());
}

void ChippingWindow::createView()
{
   if (mpChippingWidget == NULL)
   {
      return;
   }

   RasterElement* pRaster = getRasterElement();
   if (pRaster == NULL)
   {
      return;
   }
   std::vector<DimensionDescriptor> bands;

   // Create the new sensor data
   RasterElement* pRasterChip = pRaster->createChip(pRaster->getParent(), 
      "_chip", mpChippingWidget->getChipRows(), mpChippingWidget->getChipColumns(), bands);
   if (pRasterChip == NULL)
   {
      QMessageBox::critical(this, windowTitle(), "Unable to create a new cube!");
      return;
   }

   // Create a view for the new sensor data
   DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
   if (pDesktop == NULL)
   {
      return;
   }

   SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow(pRasterChip->getName(), SPATIAL_DATA_WINDOW));
   if (pWindow == NULL)
   {
      return;
   }

   SpatialDataView* pView = pWindow->getSpatialDataView();
   if (pView == NULL)
   {
      pDesktop->deleteWindow(pWindow);
      return;
   }

   UndoLock lock(pView);
   bool bSuccess = pView->setPrimaryRasterElement(pRasterChip);
   if (bSuccess == false)
   {
      pDesktop->deleteWindow(pWindow);
      return;
   }

   RasterLayer* pLayer = static_cast<RasterLayer*>(pView->createLayer(RASTER, pRasterChip));
   if (pLayer == NULL)
   {
      pDesktop->deleteWindow(pWindow);
      return;
   }

   string origName = pRaster->getName();

   SpatialDataWindow* pOrigWindow = static_cast<SpatialDataWindow*>(pDesktop->getWindow(origName,
      SPATIAL_DATA_WINDOW));
   if (pOrigWindow != NULL)
   {
      SpatialDataView* pOrigView = pOrigWindow->getSpatialDataView();
      if (pOrigView != NULL)
      {
         LayerList* pLayerList = pOrigView->getLayerList();
         if (pLayerList != NULL)
         {
            RasterLayer* pOrigLayer = static_cast<RasterLayer*>(pLayerList->getLayer(RASTER, pRaster));
            if (pOrigLayer != NULL)
            {
               DimensionDescriptor grayBand = pOrigLayer->getDisplayedBand(GRAY);
               DimensionDescriptor redBand = pOrigLayer->getDisplayedBand(RED);
               DimensionDescriptor greenBand = pOrigLayer->getDisplayedBand(GREEN);
               DimensionDescriptor blueBand = pOrigLayer->getDisplayedBand(BLUE);

               RasterElement* pGrayRaster = pOrigLayer->getDisplayedRasterElement(GRAY);
               RasterElement* pRedRaster = pOrigLayer->getDisplayedRasterElement(RED);
               RasterElement* pGreenRaster = pOrigLayer->getDisplayedRasterElement(GREEN);
               RasterElement* pBlueRaster= pOrigLayer->getDisplayedRasterElement(BLUE);

               const RasterDataDescriptor *pDescriptor = 
                  dynamic_cast<const RasterDataDescriptor*>(pRasterChip->getDataDescriptor());
               VERIFYNRV(pDescriptor != NULL);
               // Set the properties of the cube layer in the new view
               pLayer->setDisplayedBand(GRAY, grayBand);
               pLayer->setDisplayedBand(RED, redBand);
               pLayer->setDisplayedBand(GREEN, greenBand);
               pLayer->setDisplayedBand(BLUE, blueBand);
               pLayer->setDisplayMode(pOrigLayer->getDisplayMode());

               double dGrayLower = 0.0;
               double dGrayUpper = 0.0;
               double dRedLower = 0.0;
               double dRedUpper = 0.0;
               double dGreenLower = 0.0;
               double dGreenUpper = 0.0;
               double dBlueLower = 0.0;
               double dBlueUpper = 0.0;

               if (pRedRaster != pRaster)
               {
                  RegionUnits redUnits = RasterLayer::getSettingRedStretchUnits();
                  pLayer->setStretchUnits(RED, redUnits);

                  RasterLayerImp::getDefaultStretchValues(RED, dRedLower, dRedUpper);
               }
               else
               {
                  pOrigLayer->getStretchValues(RED, dRedLower, dRedUpper);
                  pLayer->setStretchUnits(RED, pOrigLayer->getStretchUnits(RED));
               }

               if (pGreenRaster != pRaster)
               {
                  RegionUnits greenUnits = RasterLayer::getSettingGreenStretchUnits();
                  pLayer->setStretchUnits(GREEN, greenUnits);

                  RasterLayerImp::getDefaultStretchValues(GREEN, dGreenLower, dGreenUpper);
               }
               else
               {
                  pOrigLayer->getStretchValues(GREEN, dGreenLower, dGreenUpper);
                  pLayer->setStretchUnits(GREEN, pOrigLayer->getStretchUnits(GREEN));
               }

               if (pBlueRaster != pRaster)
               {
                  RegionUnits blueUnits = RasterLayer::getSettingBlueStretchUnits();
                  pLayer->setStretchUnits(BLUE, blueUnits);

                  RasterLayerImp::getDefaultStretchValues(BLUE, dBlueLower, dBlueUpper);
               }
               else
               {
                  pOrigLayer->getStretchValues(BLUE, dBlueLower, dBlueUpper);
                  pLayer->setStretchUnits(BLUE, pOrigLayer->getStretchUnits(BLUE));
               }

               if (pGrayRaster != pRaster)
               {
                  RegionUnits grayUnits = RasterLayer::getSettingGrayscaleStretchUnits();
                  pLayer->setStretchUnits(GRAY, grayUnits);

                  RasterLayerImp::getDefaultStretchValues(GRAY, dGrayLower, dGrayUpper);
               }
               else
               {
                  pOrigLayer->getStretchValues(GRAY, dGrayLower, dGrayUpper);
                  pLayer->setStretchUnits(GRAY, pOrigLayer->getStretchUnits(GRAY));
               }

               pLayer->setStretchType(GRAYSCALE_MODE, pOrigLayer->getStretchType(GRAYSCALE_MODE));
               pLayer->setStretchType(RGB_MODE, pOrigLayer->getStretchType(RGB_MODE));

               pLayer->setStretchValues(GRAY, dGrayLower, dGrayUpper);
               pLayer->setStretchValues(RED, dRedLower, dRedUpper);
               pLayer->setStretchValues(GREEN, dGreenLower, dGreenUpper);
               pLayer->setStretchValues(BLUE, dBlueLower, dBlueUpper);

               pView->refresh();
            }
         }
      }
   }
}

RasterElement* ChippingWindow::getRasterElement() const
{
   LayerList* pLayerList = mpView->getLayerList();
   if (pLayerList == NULL)
   {
      return NULL;
   }

   return pLayerList->getPrimaryRasterElement();
}

void ChippingWindow::accept()
{
   // Check if the original data set has been georeferenced
   if (mpView != NULL)
   {
      RasterElement* pRaster = getRasterElement();
      VERIFYNRV(pRaster != NULL)
      bool bGeoreferenced = pRaster->isGeoreferenced();

      // Check for GCPs if chipping to a new view since the file descriptor
      // of the chip will match that of the original data set
      if (bGeoreferenced == false)
      {
         if ((mpWindowRadio->isChecked() == true) && (pRaster != NULL))
         {
            const DataDescriptor* pDescriptor = pRaster->getDataDescriptor();
            if (pDescriptor != NULL)
            {
               const RasterFileDescriptor* pFileDescriptor =
                  dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
               if (pFileDescriptor != NULL)
               {
                  const list<GcpPoint>& gcps = pFileDescriptor->getGcps();
                  bGeoreferenced = !gcps.empty();
               }
            }
         }
      }

      // Display a warning if the data set has not been georeferenced
      if (bGeoreferenced == false)
      {
         QString strMessage = "The parent data set is not georeferenced so the created chip will not "
            "contain a GCP List.\nIf you want to preserve geographic info, press Cancel, cancel the "
            "chipping dialog,\ngeoreference the parent data set, and then create a new chip.";
         int iReturn = QMessageBox::warning(this, windowTitle(), strMessage, "Continue", "Cancel");
         if (iReturn == 1)
         {
            return;
         }
      }
   }

   // Create the chip
   if (mpFileRadio->isChecked() == true)
   {
      createFile();
   }
   else if (mpWindowRadio->isChecked() == true)
   {
      createView();
   }

   // Close the dialog
   QDialog::accept();
}

bool ChippingWindow::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (pEvent->type() == QEvent::ContextMenu)
      {
         return true;
      }
   }

   return QDialog::eventFilter(pObject, pEvent);
}
