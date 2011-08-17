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
#include <QtCore/QString>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHelpEvent>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QToolTip>

#include "AppVerify.h"
#include "ChippingWidget.h"
#include "DataVariant.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "LayerList.h"
#include "LocationType.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "RasterLayerImp.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataViewImp.h"
#include "SpecialMetadata.h"
#include "SubsetWidget.h"
#include "ZoomPanWidget.h"

#include <sstream>

using namespace std;

ChippingWidget::ChippingWidget(SpatialDataView* pView,
                               const RasterDataDescriptor* pDefaultDescriptor, QWidget* pParent) :
   QWidget(pParent),
   mExportMode(false),
   mpView(pView),
   mpDefaultDescriptor(pDefaultDescriptor),
   mpDataDescriptor(NULL),
   mRowSkipFactor(0),
   mColumnSkipFactor(0),
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
   mpSizeCombo->setMinimumWidth(100);

   // Buttons
   QPushButton* pAdvancedButton = new QPushButton("Advanced...", this);
   QPushButton* pZoomButton = new QPushButton(this);
   QPushButton* pCenterButton = new QPushButton("Center View", this);

   QIcon icnZoom(":/icons/ZoomToFit");
   pZoomButton->setIcon(icnZoom);

   // Pixel coordinate
   mpCoordLabel = new QLabel("Pixel:", this);

   // Layout
   QHBoxLayout* pSizeLayout = new QHBoxLayout();
   pSizeLayout->setMargin(0);
   pSizeLayout->setSpacing(5);
   pSizeLayout->addWidget(pSizeLabel);
   pSizeLayout->addWidget(mpSizeCombo, 10);
   pSizeLayout->addWidget(pAdvancedButton);

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
   initializeComboBox();

   // Initialize the DimensionDescriptor vectors
   const RasterDataDescriptor* pDescriptor = getDataDescriptor();
   if (pDescriptor != NULL)
   {
      mChipRows = pDescriptor->getRows();
      mChipColumns = pDescriptor->getColumns();
      mChipBands = pDescriptor->getBands();
   }

   // Initialize the selection box
   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;
   if (mpView != NULL)
   {
      mpView->getExtents(dMinX, dMinY, dMaxX, dMaxY);
   }

   vector<LocationType> selection;
   selection.push_back(LocationType(dMinX, dMinY));
   selection.push_back(LocationType(dMaxX, dMinY));
   selection.push_back(LocationType(dMaxX, dMaxY));
   selection.push_back(LocationType(dMinX, dMaxY));

   mpSelectionWidget->setSelection(selection);

   // Connections
   VERIFYNR(connect(mpSelectionWidget, SIGNAL(selectionChanged(const std::vector<LocationType>&)),
      this, SLOT(updateChip())));
   VERIFYNR(connect(mpSelectionWidget, SIGNAL(locationChanged(const LocationType&)),
      this, SLOT(updateCoordinateText(const LocationType&))));
   VERIFYNR(connect(mpSizeCombo, SIGNAL(activated(int)), this, SLOT(updateChip())));
   VERIFYNR(connect(pAdvancedButton, SIGNAL(clicked()), this, SLOT(showAdvanced())));
   VERIFYNR(connect(pZoomButton, SIGNAL(clicked()), this, SLOT(zoomExtents())));
   VERIFYNR(connect(pCenterButton, SIGNAL(clicked()), this, SLOT(centerView())));
}

ChippingWidget::~ChippingWidget()
{}

void ChippingWidget::setExportMode(bool enableExportMode)
{
   mExportMode = enableExportMode;
}

const vector<DimensionDescriptor>& ChippingWidget::getChipRows() const
{
   return mChipRows;
}

const vector<DimensionDescriptor>& ChippingWidget::getChipColumns() const
{
   return mChipColumns;
}

const vector<DimensionDescriptor>& ChippingWidget::getChipBands() const
{
   return mChipBands;
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
   // Return mpDataDescriptor if it was already set.
   if (mpDataDescriptor.get() != NULL)
   {
      return mpDataDescriptor.get();
   }

   // Otherwise, return the data descriptor of the primary raster element being displayed in the view.
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
               if (mpDefaultDescriptor != NULL && RasterUtilities::isSubcube(pDescriptor, true))
               {
                  // Create a copy so that the original is unchanged.
                  // This is REQUIRED for PreviewWidget since it listens to the chipChanged() signal.
                  mpDataDescriptor = DataDescriptorResource<RasterDataDescriptor>(
                     dynamic_cast<RasterDataDescriptor*>(mpDefaultDescriptor->copy()));
                  VERIFYRV(mpDataDescriptor.get() != NULL, NULL);

                  // Set the active numbers for the copy. This is not the "correct" thing to do since the data has not
                  // yet been loaded, however it is required for ZoomPanWidget::setSelection() to function properly.
                  // However, since mpDataDescriptor is completely internal to this class, this abuse does not have a
                  // negative impact on other objects. This is only strictly necessary for rows and columns; bands
                  // should not have their Active Numbers set since the ZoomPanWidget does not use them.
                  vector<DimensionDescriptor> rows = mpDataDescriptor->getRows();
                  for (vector<DimensionDescriptor>::size_type i = 0; i < rows.size(); ++i)
                  {
                     rows[i].setActiveNumber(i);
                     rows[i].setActiveNumberValid(true);
                  }
                  mpDataDescriptor->setRows(rows);

                  vector<DimensionDescriptor> columns = mpDataDescriptor->getColumns();
                  for (vector<DimensionDescriptor>::size_type i = 0; i < columns.size(); ++i)
                  {
                     columns[i].setActiveNumber(i);
                     columns[i].setActiveNumberValid(true);
                  }
                  mpDataDescriptor->setColumns(columns);

                  return mpDataDescriptor.get();
               }

               return pDescriptor;
            }
         }
      }
   }

   return NULL;
}

bool ChippingWidget::event(QEvent* pEvent)
{
   QHelpEvent* pHelpEvent = dynamic_cast<QHelpEvent*>(pEvent);
   if (pHelpEvent != NULL && pHelpEvent->type() == QEvent::ToolTip &&
      mChipColumns.empty() == false && mChipRows.empty() == false)
   {
      QString toolTip = QString(
         "Upper Left: [%1, %2]\n"
         "Lower Right: [%3, %4]\n"
         "Row Skip Factor: %5\n"
         "Column Skip Factor: %6")
         .arg(mChipColumns.front().getOriginalNumber() + 1)
         .arg(mChipRows.front().getOriginalNumber() + 1)
         .arg(mChipColumns.back().getOriginalNumber() + 1)
         .arg(mChipRows.back().getOriginalNumber() + 1)
         .arg(mRowSkipFactor)
         .arg(mColumnSkipFactor);

      QToolTip::showText(pHelpEvent->globalPos(), toolTip, this);
      pEvent->accept();
      return true;
   }

   return QWidget::event(pEvent);
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
      unsigned int rowChipSize = 0;
      unsigned int columnChipSize = 0;
      if (iSizeIndex <= static_cast<int>(mChipSizes.size()))
      {
         unsigned int baseChipSize = mChipSizes.at(iSizeIndex - 1);
         rowChipSize = baseChipSize * (mRowSkipFactor + 1);
         columnChipSize = baseChipSize * (mColumnSkipFactor + 1);
      }

      // Get the center of the current box
      LocationType center;
      center.mX = (lowerLeft.mX + lowerRight.mX + upperRight.mX + upperLeft.mX) / 4.0;
      center.mY = (lowerLeft.mY + lowerRight.mY + upperRight.mY + upperLeft.mY) / 4.0;

      // Calculate the new selection box extents in pixels
      lowerLeft.mX = center.mX - (columnChipSize / 2);
      lowerLeft.mY = center.mY - (rowChipSize / 2);
      lowerRight.mX = center.mX + (columnChipSize / 2);
      lowerRight.mY = center.mY - (rowChipSize / 2);
      upperRight.mX = center.mX + (columnChipSize / 2);
      upperRight.mY = center.mY + (rowChipSize / 2);
      upperLeft.mX = center.mX - (columnChipSize / 2);
      upperLeft.mY = center.mY + (rowChipSize / 2);
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
   unsigned int startColumn = static_cast<unsigned int>(dMinX);
   unsigned int startRow = static_cast<unsigned int>(dMinY);
   unsigned int endColumn = static_cast<unsigned int>(dMaxX - 1.0);
   unsigned int endRow = static_cast<unsigned int>(dMaxY - 1.0);

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
      for (unsigned int i = startRow; i <= endRow; i += mRowSkipFactor + 1)
      {
         DimensionDescriptor row = pDescriptor->getActiveRow(i);
         if (row.isValid())
         {
            mChipRows.push_back(row);
         }
      }

      // Columns
      for (unsigned int i = startColumn; i <= endColumn; i += mColumnSkipFactor + 1)
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

void ChippingWidget::showAdvanced()
{
   const RasterDataDescriptor* pDescriptor = getDataDescriptor();
   if (pDescriptor == NULL)
   {
      return;
   }

   // Get the RasterFileDescriptor to determine the unchipped rows, columns, and bands.
   // Note: When a copy of mpDefaultDescriptor has been created its RasterFileDescriptor is set from within copy().
   //       This behavior is imperative to ensure that the proper bands are set into the SubsetWidget if
   //       the importer sets its own bad bands during getImportDescriptors().
   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());

   // The File Descriptor may be NULL (e.g.: when chipping algorithm results) so use the RasterDataDescriptor
   // in that case. In export mode, the data must already be loaded so use the RasterDataDescriptor in that case too.
   vector<DimensionDescriptor> rows;
   vector<DimensionDescriptor> columns;
   vector<DimensionDescriptor> bands;
   vector<string> bandNames;
   if (pFileDescriptor == NULL || mExportMode == true)
   {
      rows = pDescriptor->getRows();
      columns = pDescriptor->getColumns();
      bands = pDescriptor->getBands();
      bandNames = RasterUtilities::getBandNames(pDescriptor);
   }
   else
   {
      rows = pFileDescriptor->getRows();
      columns = pFileDescriptor->getColumns();
      bands = pFileDescriptor->getBands();

      // Note: The band names here cannot use the RasterUtilities::getBandNames() method because that method uses a
      // RasterDataDescriptor (cf. a RasterFileDescriptor) to determine band names, meaning that bad bands will not
      // have their names included in the list of band names. If bad bands are not included in the list of band names,
      // then the SubsetWidget will have a mismatch between the number of band names and the number of bands specified
      // if bad bands are set by the importer. If SubsetWidget has a mismatch between these two numbers, then no band
      // names are displayed to the user.
      // In summary, if we change this to always use RasterUtilities::getBandNames(), then the SubsetWidget will ignore
      // band names and replace them with "Band 1", ..., "Band n" for any image with bad bands set by its importer.
      const DynamicObject* pMetadata = pDescriptor->getMetadata();
      if (pMetadata != NULL)
      {
         string pNamesPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, NAMES_METADATA_NAME, END_METADATA_NAME };
         const vector<string>* pBandNames = dv_cast<vector<string> >(&pMetadata->getAttributeByPath(pNamesPath));
         if ((pBandNames != NULL) && (pBandNames->size() == bands.size()))
         {
            bandNames = *pBandNames;
         }
         else
         {
            string pPrefixPath[] = { SPECIAL_METADATA_NAME, BAND_NAME_PREFIX_METADATA_NAME, END_METADATA_NAME };
            const string* pBandPrefix = dv_cast<string>(&pMetadata->getAttributeByPath(pPrefixPath));
            if (pBandPrefix != NULL)
            {
               string bandNamePrefix = *pBandPrefix;
               for (unsigned int i = 0; i < bands.size(); ++i)
               {
                  ostringstream formatter;
                  formatter << bandNamePrefix << " " << bands[i].getOriginalNumber() + 1;
                  bandNames.push_back(formatter.str());
               }
            }
         }
      }
   }

   // Set the Active Numbers for the rows and columns. This is not the "correct" thing to do since the data may not have
   // been loaded yet, however it is required for ZoomPanWidget::setSelection() to function properly. This is only
   // needed for rows and columns; bands should not have Active Numbers set since the ZoomPanWidget does not use them.
   for (vector<DimensionDescriptor>::size_type i = 0; i < rows.size(); ++i)
   {
      rows[i].setActiveNumber(i);
      rows[i].setActiveNumberValid(true);
   }

   for (vector<DimensionDescriptor>::size_type i = 0; i < columns.size(); ++i)
   {
      columns[i].setActiveNumber(i);
      columns[i].setActiveNumberValid(true);
   }

   // Display the SubsetWidget to the user.
   QDialog dlg(this);
   dlg.setWindowTitle("Select Image Subset");

   SubsetWidget* pSubsetWidget = new SubsetWidget(&dlg);
   pSubsetWidget->setExportMode(mExportMode);
   pSubsetWidget->setRows(rows, mChipRows);
   pSubsetWidget->setColumns(columns, mChipColumns);
   pSubsetWidget->setBands(bands, bandNames, mChipBands);
   if (pFileDescriptor != NULL)
   {
      QString strDirectory;

      string filename = pFileDescriptor->getFilename();
      if (filename.empty() == false)
      {
         QFileInfo fileInfo(QString::fromStdString(filename));
         strDirectory = fileInfo.absolutePath();
      }

      pSubsetWidget->setBadBandFileDirectory(strDirectory);
   }

   QFrame* pLine = new QFrame(&dlg);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   QDialogButtonBox* pBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);

   QVBoxLayout* pLayout = new QVBoxLayout(&dlg);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pSubsetWidget, 10);
   pLayout->addWidget(pLine);
   pLayout->addWidget(pBox);
   VERIFYNR(connect(pBox, SIGNAL(accepted()), &dlg, SLOT(accept())));
   VERIFYNR(connect(pBox, SIGNAL(rejected()), &dlg, SLOT(reject())));
   if (dlg.exec() == QDialog::Accepted)
   {
      // The row and column skip factors must be set BEFORE calling updateChip().
      // The chipped rows and columns are overwritten in updateChip() so there is no need to set them here.
      mChipBands = pSubsetWidget->getSubsetBands();
      mRowSkipFactor = pSubsetWidget->getSubsetRowSkipFactor();
      mColumnSkipFactor = pSubsetWidget->getSubsetColumnSkipFactor();

      // Reinitialize mpSizeCombo to take row and column skip factors into account.
      // The code in updateChip() assumes that the entries in the combo box coupled with
      // the skip factor do not exceed the size of the image. If this assumption is violated,
      // updateChip() can recurse infinitely when the selected Chip Size is too large.
      initializeComboBox();

      // Update the selection widget. Recall that Active Numbers were set above, and VERIFY that they are still valid.
      const vector<DimensionDescriptor> chipRows = pSubsetWidget->getSubsetRows();
      const vector<DimensionDescriptor> chipColumns = pSubsetWidget->getSubsetColumns();
      VERIFYNRV(chipRows.empty() == false && chipColumns.empty() == false &&
         chipRows.front().isActiveNumberValid() && chipRows.back().isActiveNumberValid() &&
         chipColumns.front().isActiveNumberValid() && chipColumns.back().isActiveNumberValid());
      vector<LocationType> newSelection;
      double x1 = static_cast<double>(chipColumns.front().getActiveNumber());
      double x2 = static_cast<double>(chipColumns.back().getActiveNumber());
      double y1 = static_cast<double>(chipRows.front().getActiveNumber());
      double y2 = static_cast<double>(chipRows.back().getActiveNumber());
      newSelection.push_back(LocationType(x1, y1));
      newSelection.push_back(LocationType(x2, y1));
      newSelection.push_back(LocationType(x2, y2));
      newSelection.push_back(LocationType(x1, y2));
      mpSelectionWidget->setSelection(newSelection);

      // Call updateChip() manually because mpSelectionWidget does not call it if there is no change to the selection.
      updateChip();
   }
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

   RasterLayer* pLayer = getRasterLayer();
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

void ChippingWidget::initializeComboBox()
{
   VERIFYNRV(mpSizeCombo != NULL);
   mpSizeCombo->clear();
   mpSizeCombo->addItem("Custom");

   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;
   if (mpView != NULL)
   {
      mpView->getExtents(dMinX, dMinY, dMaxX, dMaxY);

      // Apply the current row and/or column skip factors so that entries too large for them are not displayed.
      dMinX /= (mColumnSkipFactor + 1);
      dMaxX /= (mColumnSkipFactor + 1);
      dMinY /= (mRowSkipFactor + 1);
      dMinY /= (mRowSkipFactor + 1);

      const double deltaX = dMaxX - dMinX;
      const double deltaY = dMaxY - dMinY;
      VERIFYNRV(deltaX >= 0.0 && deltaY >= 0.0);
      const unsigned int maxChipSize = static_cast<unsigned int>(std::min(deltaX, deltaY));

      for (unsigned int i = 0; i < mChipSizes.size(); i++)
      {
         unsigned int chipSize = mChipSizes.at(i);
         if (chipSize <= maxChipSize)
         {
            mpSizeCombo->addItem(QString("%1 x %1").arg(chipSize));
         }
      }
   }

   mpSizeCombo->setCurrentIndex(0);
}