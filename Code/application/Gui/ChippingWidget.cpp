/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QLayout>
#include <QtGui/QPushButton>

#include "ChippingWidget.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DimensionDescriptor.h"
#include "Icons.h"
#include "LayerList.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterLayer.h"
#include "RasterLayerImp.h"
#include "SpatialDataView.h"
#include "SpatialDataViewImp.h"
#include "ZoomPanWidget.h"

using namespace std;

ChippingWidget::ChippingWidget(SpatialDataView* pView, QWidget* parent) :
   QWidget(parent),
   mpView(pView),
   mpSelectionWidget(NULL),
   mpSizeCombo(NULL),
   mpCoordLabel(NULL)
{
   mChipSizes.push_back(64);
   mChipSizes.push_back(128);
   mChipSizes.push_back(256);
   mChipSizes.push_back(512);
   mChipSizes.push_back(1024);
   mChipSizes.push_back(2048);

   // View
   mpSelectionWidget = new ZoomPanWidget(dynamic_cast<SpatialDataViewImp*>(mpView), this);

   // Chip size
   QLabel* pSizeLabel = new QLabel("Chip Size:", this);
   mpSizeCombo = new QComboBox(this);
   mpSizeCombo->setEditable(false);
   mpSizeCombo->setMinimumWidth(150);

   // Buttons
   QPushButton* pZoomButton = new QPushButton(this);
   QPushButton* pCenterButton = new QPushButton("&Center View", this);

   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   QIcon icnZoom(pIcons->mZoomToFit);
   pZoomButton->setIcon(icnZoom);

   // Pixel coordinate
   mpCoordLabel = new QLabel("Pixel:", this);

   // Layout
   QHBoxLayout* pSizeLayout = new QHBoxLayout();
   pSizeLayout->setMargin(0);
   pSizeLayout->setSpacing(5);
   pSizeLayout->addWidget(pSizeLabel);
   pSizeLayout->addWidget(mpSizeCombo, 10);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(mpSelectionWidget, 0, 0, 1, 3);
   pGrid->addLayout(pSizeLayout, 1, 0, 1, 3);
   pGrid->addWidget(mpCoordLabel, 2, 0);
   pGrid->addWidget(pZoomButton, 2, 1);
   pGrid->addWidget(pCenterButton, 2, 2);
   pGrid->setRowStretch(0, 10);
   pGrid->setColumnStretch(0, 10);

   // Initialize the combo box
   mpSizeCombo->addItem("Custom");

   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;
   if (mpView != NULL)
   {
      mpView->getExtents(dMinX, dMinY, dMaxX, dMaxY);

      for (unsigned int i = 0; i < mChipSizes.size(); i++)
      {
         unsigned int chipSize = mChipSizes.at(i);
         if ((chipSize <= (unsigned int) dMaxY) && (chipSize <= (unsigned int) dMaxX))
         {
            QString strSize;
            strSize.sprintf("%u x %u", chipSize, chipSize);

            mpSizeCombo->addItem(strSize);
         }
      }
   }

   mpSizeCombo->setCurrentIndex(0);

   // Initialize the row and column vectors to the active rows and columns
   const RasterDataDescriptor* pDescriptor = getDataDescriptor();
   if (pDescriptor != NULL)
   {
      mChipRows = pDescriptor->getRows();
      mChipColumns = pDescriptor->getColumns();
   }

   // Initialize the selection box
   vector<LocationType> selection;
   selection.push_back(LocationType(dMinX, dMinY));
   selection.push_back(LocationType(dMaxX, dMinY));
   selection.push_back(LocationType(dMaxX, dMaxY));
   selection.push_back(LocationType(dMinX, dMaxY));

   mpSelectionWidget->setSelection(selection);

   // Connections
   connect(mpSelectionWidget, SIGNAL(selectionChanged(const std::vector<LocationType>&)), this, SLOT(updateChip()));
   connect(mpSelectionWidget, SIGNAL(locationChanged(const LocationType&)), this,
      SLOT(updateCoordinateText(const LocationType&)));
   connect(mpSizeCombo, SIGNAL(activated(int)), this, SLOT(updateChip()));
   connect(pZoomButton, SIGNAL(clicked()), this, SLOT(zoomExtents()));
   connect(pCenterButton, SIGNAL(clicked()), this, SLOT(centerView()));
}

ChippingWidget::~ChippingWidget()
{
}

const vector<DimensionDescriptor>& ChippingWidget::getChipRows() const
{
   return mChipRows;
}

const vector<DimensionDescriptor>& ChippingWidget::getChipColumns() const
{
   return mChipColumns;
}

void ChippingWidget::showEvent(QShowEvent* pEvent)
{
   QWidget::showEvent(pEvent);
   zoomExtents();
}

RasterLayer* ChippingWidget::getRasterLayer() const
{
   if (mpView == NULL)
   {
      return NULL;
   }

   LayerList* pLayerList = mpView->getLayerList();
   if (pLayerList != NULL)
   {
      vector<Layer*> layers;
      pLayerList->getLayers(RASTER, layers);
      if (layers.empty() == false)
      {
         RasterLayer* pLayer = static_cast<RasterLayer*>(layers.front());
         return pLayer;
      }
   }

   return NULL;
}

const RasterDataDescriptor* ChippingWidget::getDataDescriptor() const
{
   if (mpView != NULL)
   {
      LayerList* pLayerList = mpView->getLayerList();
      if (pLayerList != NULL)
      {
         RasterElement* pRasterElement = pLayerList->getPrimaryRasterElement();
         if (pRasterElement != NULL)
         {
            const RasterDataDescriptor* pDescriptor =
               dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
            if (pDescriptor != NULL)
            {
               return pDescriptor;
            }
         }
      }
   }

   return NULL;
}

void ChippingWidget::updateChip()
{
   if ((mpView == NULL) || (mpSelectionWidget == NULL))
   {
      return;
   }

   // Get the selection box
   const vector<LocationType>& selection = mpSelectionWidget->getSelection();
   if (selection.size() != 4)
   {
      return;
   }

   LocationType lowerLeft = selection[0];
   LocationType lowerRight = selection[1];
   LocationType upperRight = selection[2];
   LocationType upperLeft = selection[3];

   // Normalize the values
   if (lowerLeft.mX > upperRight.mX)
   {
      double dTemp = lowerLeft.mX;
      lowerLeft.mX = upperRight.mX;
      upperLeft.mX = upperRight.mX;
      upperRight.mX = dTemp;
      lowerRight.mX = dTemp;
   }

   if (lowerLeft.mY > upperRight.mY)
   {
      double dTemp = lowerLeft.mY;
      lowerLeft.mY = upperRight.mY;
      lowerRight.mY = upperRight.mY;
      upperLeft.mY = dTemp;
      upperRight.mY = dTemp;
   }

   // Get the layer extents
   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;
   RasterLayerImp* pLayer = dynamic_cast<RasterLayerImp*>(getRasterLayer());
   VERIFYNRV(pLayer != NULL);
   pLayer->getExtents(dMinX, dMinY, dMaxX, dMaxY);
   pLayer->translateWorldToData(dMinX, dMinY, dMinX, dMinY);
   pLayer->translateWorldToData(dMaxX, dMaxY, dMaxX, dMaxY);

   // Adjust the selection box size based on the chip size
   int iSizeIndex = 0;
   if (mpSizeCombo != NULL)
   {
      iSizeIndex = mpSizeCombo->currentIndex();
   }

   if (iSizeIndex > 0)
   {
      // Get the selected chip size
      unsigned int chipSize = 0;
      if (iSizeIndex <= (int) mChipSizes.size())
      {
         chipSize = mChipSizes.at(iSizeIndex - 1);
      }

      // Get the center of the current box
      LocationType center;
      center.mX = (lowerLeft.mX + lowerRight.mX + upperRight.mX + upperLeft.mX) / 4.0;
      center.mY = (lowerLeft.mY + lowerRight.mY + upperRight.mY + upperLeft.mY) / 4.0;

      // Calculate the new selection box extents in pixels
      lowerLeft.mX = center.mX - (chipSize / 2);
      lowerLeft.mY = center.mY - (chipSize / 2);
      lowerRight.mX = center.mX + (chipSize / 2);
      lowerRight.mY = center.mY - (chipSize / 2);
      upperRight.mX = center.mX + (chipSize / 2);
      upperRight.mY = center.mY + (chipSize / 2);
      upperLeft.mX = center.mX - (chipSize / 2);
      upperLeft.mY = center.mY + (chipSize / 2);
   }
   else
   {
      // Clip the box to fit within the view extents
      if (lowerLeft.mX < dMinX)
      {
         lowerLeft.mX = dMinX;
      }
      else if (lowerLeft.mX > dMaxX)
      {
         lowerLeft.mX = dMaxX - 1.0;
      }

      if (lowerLeft.mY < dMinY)
      {
         lowerLeft.mY = dMinY;
      }
      else if (lowerLeft.mY > dMaxY)
      {
         lowerLeft.mY = dMaxY - 1.0;
      }

      if (lowerRight.mX > dMaxX)
      {
         lowerRight.mX = dMaxX;
      }
      else if (lowerRight.mX < dMinX)
      {
         lowerRight.mX = dMinX + 1.0;
      }

      if (lowerRight.mY < dMinY)
      {
         lowerRight.mY = dMinY;
      }
      else if (lowerRight.mY > dMaxY)
      {
         lowerRight.mY = dMaxY - 1.0;
      }

      if (upperRight.mX > dMaxX)
      {
         upperRight.mX = dMaxX;
      }
      else if (upperRight.mX < dMinX)
      {
         upperRight.mX = dMinX + 1.0;
      }

      if (upperRight.mY > dMaxY)
      {
         upperRight.mY = dMaxY;
      }
      else if (upperRight.mY < dMinY)
      {
         upperRight.mY = dMinY + 1.0;
      }

      if (upperLeft.mX < dMinX)
      {
         upperLeft.mX = dMinX;
      }
      else if (upperLeft.mX > dMaxX)
      {
         upperLeft.mX = dMaxX - 1.0;
      }

      if (upperLeft.mY > dMaxY)
      {
         upperLeft.mY = dMaxY;
      }
      else if (upperLeft.mY < dMinY)
      {
         upperLeft.mY = dMinY + 1.0;
      }
   }

   // Ensure the selection box covers at least one pixel
   if ((upperRight.mX - lowerLeft.mX) < 1.0)
   {
      lowerRight.mX = lowerLeft.mX + 1.0;
      upperRight.mX = lowerLeft.mX + 1.0;
   }

   if ((upperRight.mY - lowerLeft.mY) < 1.0)
   {
      upperRight.mY = lowerLeft.mY + 1.0;
      upperLeft.mY = lowerLeft.mY + 1.0;
   }

   // Move the selection to be contained within the view extents
   LocationType delta;
   if (lowerLeft.mX < dMinX)
   {
      delta.mX = dMinX - lowerLeft.mX;
   }
   else if (upperRight.mX > dMaxX)
   {
      delta.mX = dMaxX - upperRight.mX;
   }

   if (lowerLeft.mY < dMinY)
   {
      delta.mY = dMinY - lowerLeft.mY;
   }
   else if (upperRight.mY > dMaxY)
   {
      delta.mY = dMaxY - upperRight.mY;
   }

   lowerLeft += delta;
   lowerRight += delta;
   upperRight += delta;
   upperLeft += delta;

   // Zoom in on the selection box
   centerView();

   // Get the new rows and columns for the chip
   unsigned int startColumn = (unsigned int) dMinX;
   unsigned int startRow = (unsigned int) dMinY;
   unsigned int endColumn = (unsigned int) (dMaxX - 1.0);
   unsigned int endRow = (unsigned int) (dMaxY - 1.0);

   unsigned int numRows = 0;
   unsigned int numColumns = 0;

   const RasterDataDescriptor* pDescriptor = getDataDescriptor();
   if (pDescriptor != NULL)
   {
      numRows = pDescriptor->getRowCount();
      numColumns = pDescriptor->getColumnCount();

      if ((lowerLeft.mX >= 0.0) && (lowerLeft.mX < numColumns))
      {
         startColumn = static_cast<unsigned int>(lowerLeft.mX);
      }

      if ((lowerLeft.mY >= 0.0) && (lowerLeft.mY < numRows))
      {
         startRow = static_cast<unsigned int>(lowerLeft.mY);
      }

      if ((upperRight.mX >= 0.0) && (upperRight.mX <= numColumns))
      {
         // Check if the selection box is drawn at the upper edge of the columns
         if (upperRight.mX == numColumns)
         {
            endColumn = static_cast<unsigned int>(upperRight.mX - 1.0);
         }
         else
         {
            endColumn = static_cast<unsigned int>(upperRight.mX);
         }
      }

      if ((upperRight.mY >= 0.0) && (upperRight.mY <= numRows))
      {
         // Check if the selection box is drawn at the upper edge of the rows
         if (upperRight.mY == numRows)
         {
            endRow = static_cast<unsigned int> (upperRight.mY - 1.0);
         }
         else
         {
            endRow = static_cast<unsigned int> (upperRight.mY);
         }
      }
   }

   if (startRow > endRow)
   {
      unsigned int temp = startRow;
      startRow = endRow;
      endRow = temp;
   }

   if (startColumn > endColumn)
   {
      unsigned int temp = startColumn;
      startColumn = endColumn;
      endColumn = temp;
   }

   // Update the chip rows and columns
   mChipRows.clear();
   mChipColumns.clear();

   if (pDescriptor != NULL)
   {
      // Rows
      for (unsigned int i = startRow; i <= endRow; ++i)
      {
         DimensionDescriptor row = pDescriptor->getActiveRow(i);
         if (row.isValid())
         {
            mChipRows.push_back(row);
         }
      }

      // Columns
      for (unsigned int i = startColumn; i <= endColumn; ++i)
      {
         DimensionDescriptor column = pDescriptor->getActiveColumn(i);
         if (column.isValid())
         {
            mChipColumns.push_back(column);
         }
      }
   }

   // Notify connected objects
   emit chipChanged();

   // Update the selection box in the selection widget
   vector<LocationType> newSelection;
   newSelection.push_back(lowerLeft);
   newSelection.push_back(lowerRight);
   newSelection.push_back(upperRight);
   newSelection.push_back(upperLeft);

   if (newSelection != selection)
   {
      mpSelectionWidget->setSelection(newSelection);
   }
}

void ChippingWidget::updateCoordinateText(const LocationType& pixelCoord)
{
   QString strCoord;

   const RasterDataDescriptor* pDescriptor = getDataDescriptor();
   if (pDescriptor != NULL)
   {
      unsigned int rowCount = pDescriptor->getRowCount();
      unsigned int columnCount = pDescriptor->getColumnCount();

      if ((pixelCoord.mX >= 0.0) && (pixelCoord.mX <= static_cast<double>(columnCount)) &&
         (pixelCoord.mY >= 0.0) && (pixelCoord.mY <= static_cast<double>(rowCount)))
      {
         // Get the original row number
         unsigned int originalRow = 0;

         DimensionDescriptor row = pDescriptor->getActiveRow(static_cast<unsigned int>(pixelCoord.mY));
         if (row.isValid())
         {
            originalRow = row.getOriginalNumber();
         }

         // Get the original column number
         unsigned int originalColumn = 0;

         DimensionDescriptor column = pDescriptor->getActiveColumn(static_cast<unsigned int>(pixelCoord.mX));
         if (column.isValid())
         {
            originalColumn = column.getOriginalNumber();
         }

         strCoord.sprintf("Pixel: (%u, %u)", originalColumn + 1, originalRow + 1);
      }
   }

   mpCoordLabel->setText(strCoord);
}

void ChippingWidget::zoomExtents()
{
   if (mpSelectionWidget != NULL)
   {
      mpSelectionWidget->zoomExtents();
   }
}

void ChippingWidget::centerView()
{
   if ((mpSelectionWidget == NULL) || (mpView == NULL))
   {
      return;
   }

   const vector<LocationType>& selection = mpSelectionWidget->getSelection();
   if (selection.size() != 4)
   {
      return;
   }

   RasterLayer *pLayer = getRasterLayer();
   VERIFYNRV(pLayer != NULL);

   LocationType lowerLeft;
   LocationType upperRight;
   pLayer->translateDataToWorld(selection[0].mX, selection[0].mY, lowerLeft.mX, lowerLeft.mY);
   pLayer->translateDataToWorld(selection[2].mX, selection[2].mY, upperRight.mX, upperRight.mY);

   LocationType pixelMargin(10, 10);
   LocationType zoomLowerLeft = lowerLeft - pixelMargin;
   LocationType zoomUpperRight = upperRight + pixelMargin;
   mpView->zoomToBox(zoomLowerLeft, zoomUpperRight);
   mpView->refresh();
}
