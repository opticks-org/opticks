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
#include "AppVerify.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "GcpList.h"
#include "LayerList.h"
#include "ModelServices.h"
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
#include <vector>
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
   mpChippingWidget = new ChippingWidget(pChipView, NULL, this);
   mpChippingWidget->setExportMode(true);

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
   setWindowIcon(QIcon(":/icon/ChipImage"));

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
{}

SpatialDataView* ChippingWindow::createChipView() const
{
   if (mpView == NULL)
   {
      return NULL;
   }

   SpatialDataView* pChipView = dynamic_cast<SpatialDataView*>(mpView->copy());
   if (pChipView != NULL)
   {
      vector<std::pair<View*, LinkType> > linkedViews;
      pChipView->getLinkedViews(linkedViews);
      for (unsigned int i = 0; i < linkedViews.size(); ++i)
      {
         if (linkedViews[i].second == NO_LINK)
         {
            continue;
         }

         pChipView->unlinkView(linkedViews[i].first);
      }

      LayerList* pLayerList = pChipView->getLayerList();
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

      pChipView->resetOrientation();
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

   // Create a file descriptor based on the data with the chip rows and columns
   FactoryResource<FileDescriptor> fileDescriptor(RasterUtilities::generateFileDescriptorForExport(pDescriptor,
      string(), startRow, stopRow, 0, startCol, stopCol, 0, mpChippingWidget->getChipBands()));

   Service<DesktopServices>()->exportSessionItem(pRaster, fileDescriptor.get());
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

   // Create the new raster element from the primary element of the source.
   // Note that this does not chip displayed elements if they differ from the primary element.
   // This causes a special case below where the stretch values are being applied to the chipped layer.
   RasterElement* pRasterChip = pRaster->createChip(pRaster->getParent(), "_chip",
      mpChippingWidget->getChipRows(), mpChippingWidget->getChipColumns(), mpChippingWidget->getChipBands());
   if (pRasterChip == NULL)
   {
      QMessageBox::critical(this, windowTitle(), "Unable to create a new cube!");
      return;
   }

   const RasterDataDescriptor* pDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(pRasterChip->getDataDescriptor());
   VERIFYNRV(pDescriptor != NULL);

   // Create a view for the new chip
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(
      Service<DesktopServices>()->createWindow(pRasterChip->getName(), SPATIAL_DATA_WINDOW));
   if (pWindow == NULL)
   {
      return;
   }

   SpatialDataView* pView = pWindow->getSpatialDataView();
   if (pView == NULL)
   {
      Service<DesktopServices>()->deleteWindow(pWindow);
      return;
   }

   UndoLock lock(pView);
   if (pView->setPrimaryRasterElement(pRasterChip) == false)
   {
      Service<DesktopServices>()->deleteWindow(pWindow);
      return;
   }

   // RasterLayerImp is needed for the call to setCurrentStretchAsOriginalStretch().
   RasterLayerImp* pLayer = dynamic_cast<RasterLayerImp*>(pView->createLayer(RASTER, pRasterChip));
   if (pLayer == NULL)
   {
      Service<DesktopServices>()->deleteWindow(pWindow);
      return;
   }

   string origName = pRaster->getName();

   SpatialDataWindow* pOrigWindow = dynamic_cast<SpatialDataWindow*>(
      Service<DesktopServices>()->getWindow(origName, SPATIAL_DATA_WINDOW));
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
               // Set the stretch type first so that stretch values are interpreted correctly.
               pLayer->setStretchType(GRAYSCALE_MODE, pOrigLayer->getStretchType(GRAYSCALE_MODE));
               pLayer->setStretchType(RGB_MODE, pOrigLayer->getStretchType(RGB_MODE));
               pLayer->setDisplayMode(pOrigLayer->getDisplayMode());

               // Set the properties of the cube layer in the new view.
               // For each channel, display the first band if the previously displayed band was chipped.
               vector<RasterChannelType> channels = StringUtilities::getAllEnumValues<RasterChannelType>();
               for (vector<RasterChannelType>::const_iterator iter = channels.begin(); iter != channels.end(); ++iter)
               {
                  bool bandCopied = true;
                  DimensionDescriptor newBand;
                  DimensionDescriptor oldBand = pOrigLayer->getDisplayedBand(*iter);
                  if (oldBand.isOriginalNumberValid() == true)
                  {
                     newBand = pDescriptor->getOriginalBand(oldBand.getOriginalNumber());
                  }

                  if (newBand.isValid() == false)
                  {
                     bandCopied = false;
                     newBand = pDescriptor->getBands().front();
                  }

                  // No need to explicitly set the RasterElement here since the new view only has one RasterElement.
                  pLayer->setDisplayedBand(*iter, newBand);

                  // Use the default stretch properties if the displayed band was removed from the view or
                  // if the non-primary raster element was displayed. Otherwise, copy the stretch properties.
                  if (bandCopied && pRaster == pOrigLayer->getDisplayedRasterElement(*iter))
                  {
                     // Set the stretch units first so that stretch values are interpreted correctly.
                     pLayer->setStretchUnits(*iter, pOrigLayer->getStretchUnits(*iter));

                     double lower;
                     double upper;
                     pOrigLayer->getStretchValues(*iter, lower, upper);
                     pLayer->setStretchValues(*iter, lower, upper);
                  }
               }

               pLayer->setCurrentStretchAsOriginalStretch();
               pView->refresh();
            }
         }
      }
   }

   // Create a GCP layer
   if (pRaster->isGeoreferenced() == true)
   {
      const vector<DimensionDescriptor>& rows = mpChippingWidget->getChipRows();
      const vector<DimensionDescriptor>& columns = mpChippingWidget->getChipColumns();
      if ((rows.empty() == false) && (columns.empty() == false))
      {
         // Get the geocoordinates at the chip corners
         VERIFYNRV(rows.front().isActiveNumberValid() == true);
         VERIFYNRV(rows.back().isActiveNumberValid() == true);
         VERIFYNRV(columns.front().isActiveNumberValid() == true);
         VERIFYNRV(columns.back().isActiveNumberValid() == true);

         unsigned int startRow = rows.front().getActiveNumber();
         unsigned int endRow = rows.back().getActiveNumber();
         unsigned int startCol = columns.front().getActiveNumber();
         unsigned int endCol = columns.back().getActiveNumber();

         GcpPoint ulPoint;
         ulPoint.mPixel = LocationType(startCol, startRow);
         ulPoint.mCoordinate = pRaster->convertPixelToGeocoord(ulPoint.mPixel);

         GcpPoint urPoint;
         urPoint.mPixel = LocationType(endCol, startRow);
         urPoint.mCoordinate = pRaster->convertPixelToGeocoord(urPoint.mPixel);

         GcpPoint llPoint;
         llPoint.mPixel = LocationType(startCol, endRow);
         llPoint.mCoordinate = pRaster->convertPixelToGeocoord(llPoint.mPixel);

         GcpPoint lrPoint;
         lrPoint.mPixel = LocationType(endCol, endRow);
         lrPoint.mCoordinate = pRaster->convertPixelToGeocoord(lrPoint.mPixel);

         GcpPoint centerPoint;
         centerPoint.mPixel = LocationType((startCol + endCol) / 2, (startRow + endRow) / 2);
         centerPoint.mCoordinate = pRaster->convertPixelToGeocoord(centerPoint.mPixel);

         // Reset the coordinates to be in active numbers relative to the chip
         const vector<DimensionDescriptor>& chipRows = pDescriptor->getRows();
         const vector<DimensionDescriptor>& chipColumns = pDescriptor->getColumns();

         VERIFYNRV(chipRows.front().isActiveNumberValid() == true);
         VERIFYNRV(chipRows.back().isActiveNumberValid() == true);
         VERIFYNRV(chipColumns.front().isActiveNumberValid() == true);
         VERIFYNRV(chipColumns.back().isActiveNumberValid() == true);

         unsigned int chipStartRow = chipRows.front().getActiveNumber();
         unsigned int chipEndRow = chipRows.back().getActiveNumber();
         unsigned int chipStartCol = chipColumns.front().getActiveNumber();
         unsigned int chipEndCol = chipColumns.back().getActiveNumber();
         ulPoint.mPixel = LocationType(chipStartCol, chipStartRow);
         urPoint.mPixel = LocationType(chipEndCol, chipStartRow);
         llPoint.mPixel = LocationType(chipStartCol, chipEndRow);
         lrPoint.mPixel = LocationType(chipEndCol, chipEndRow);
         centerPoint.mPixel = LocationType((chipStartCol + chipEndCol) / 2, (chipStartRow + chipEndRow) / 2);

         // Create the GCP list
         Service<ModelServices> pModel;

         GcpList* pGcpList = static_cast<GcpList*>(pModel->createElement("Corner Coordinates",
            TypeConverter::toString<GcpList>(), pRasterChip));
         if (pGcpList != NULL)
         {
            list<GcpPoint> gcps;
            gcps.push_back(ulPoint);
            gcps.push_back(urPoint);
            gcps.push_back(llPoint);
            gcps.push_back(lrPoint);
            gcps.push_back(centerPoint);

            pGcpList->addPoints(gcps);

            // Create the layer
            if (pView->createLayer(GCP_LAYER, pGcpList) == NULL)
            {
               QMessageBox::warning(this, windowTitle(), "Could not create a GCP layer.");
            }
         }
         else
         {
            QMessageBox::warning(this, windowTitle(), "Could not create a GCP list.");
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
      VERIFYNRV(pRaster != NULL);

      // Display a warning if the data set has not been georeferenced
      if (pRaster->isGeoreferenced() == false)
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
