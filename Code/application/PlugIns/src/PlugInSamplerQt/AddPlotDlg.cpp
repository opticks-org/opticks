/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "AddPlotDlg.h"
#include "AoiElement.h"
#include "Arrow.h"
#include "ApplicationServices.h"
#include "BitMask.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "HistogramPlot.h"
#include "LayerList.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "PlotManager.h"
#include "PlotView.h"
#include "PlotWidget.h"
#include "Point.h"
#include "PointSet.h"
#include "PolarGridlines.h"
#include "PolarPlot.h"
#include "PolygonPlotObject.h"
#include "ProductView.h"
#include "ProductWindow.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RegionObject.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "Text.h"

#include <string>
using namespace std;

AddPlotDlg::AddPlotDlg(QWidget* pParent) :
   QDialog(pParent)
{
   // Name
   QLabel* pNameLabel = new QLabel("Name:", this);
   mpNameEdit = new QLineEdit(this);

   // Type
   QGroupBox* pTypeGroupBox = new QGroupBox("Type", this);
   mpCartesianRadio = new QRadioButton("Cartesian", pTypeGroupBox);
   mpHistogramRadio = new QRadioButton("Histogram", pTypeGroupBox);
   mpSignatureRadio = new QRadioButton("Signature", pTypeGroupBox);
   mpPolarRadio = new QRadioButton("Polar", pTypeGroupBox);

   mpTypeGroup = new QButtonGroup(this);
   mpTypeGroup->addButton(mpCartesianRadio, 0);
   mpTypeGroup->addButton(mpHistogramRadio, 1);
   mpTypeGroup->addButton(mpSignatureRadio, 2);
   mpTypeGroup->addButton(mpPolarRadio, 3);

   QVBoxLayout* pTypeLayout = new QVBoxLayout(pTypeGroupBox);
   pTypeLayout->setMargin(10);
   pTypeLayout->setSpacing(5);
   pTypeLayout->addWidget(mpCartesianRadio);
   pTypeLayout->addWidget(mpHistogramRadio);
   pTypeLayout->addWidget(mpSignatureRadio);
   pTypeLayout->addWidget(mpPolarRadio);

   // Gridlines
   mpGridlinesCheck = new QCheckBox("Gridlines", this);

   // Legend
   mpLegendCheck = new QCheckBox("Legend", this);

   // Cartesian data
   mpCartesianDataWidget = new QWidget(this);

   // Axes
   QLabel* pXAxisLabel = new QLabel("X-Axis Text:", mpCartesianDataWidget);
   QLabel* pYAxisLabel = new QLabel("Y-Axis Text:", mpCartesianDataWidget);
   mpXAxisEdit = new QLineEdit(mpCartesianDataWidget);
   mpYAxisEdit = new QLineEdit(mpCartesianDataWidget);

   // Data
   QGroupBox* pDataGroup = new QGroupBox("Data", mpCartesianDataWidget);
   mpDataLabel = new QLabel(pDataGroup);
   mpDataCombo = new QComboBox(pDataGroup);
   mpDataCombo->setEditable(false);

   QHBoxLayout* pDataLayout = new QHBoxLayout(pDataGroup);
   pDataLayout->setMargin(10);
   pDataLayout->setSpacing(5);
   pDataLayout->addWidget(mpDataLabel);
   pDataLayout->addWidget(mpDataCombo, 10);

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Layout
   QGridLayout* pCartesianDataLayout = new QGridLayout(mpCartesianDataWidget);
   pCartesianDataLayout->setMargin(0);
   pCartesianDataLayout->setSpacing(5);
   pCartesianDataLayout->addWidget(pXAxisLabel, 0, 0);
   pCartesianDataLayout->addWidget(mpXAxisEdit, 0, 1);
   pCartesianDataLayout->addWidget(pYAxisLabel, 1, 0);
   pCartesianDataLayout->addWidget(mpYAxisEdit, 1, 1);
   pCartesianDataLayout->addWidget(pDataGroup, 2, 0, 1, 2);

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(pOkButton);
   pButtonLayout->addWidget(pCancelButton);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);
   pGrid->addWidget(pNameLabel, 0, 0);
   pGrid->addWidget(mpNameEdit, 0, 1, 1, 2);
   pGrid->addWidget(pTypeGroupBox, 1, 0, 2, 2);
   pGrid->addWidget(mpGridlinesCheck, 1, 2, Qt::AlignLeft);
   pGrid->addWidget(mpLegendCheck, 2, 2, Qt::AlignLeft | Qt::AlignTop);
   pGrid->addWidget(mpCartesianDataWidget, 3, 0, 1, 3, Qt::AlignTop);
   pGrid->addWidget(pHLine, 4, 0, 1, 3);
   pGrid->addLayout(pButtonLayout, 5, 0, 1, 3);
   pGrid->setRowStretch(3, 10);
   pGrid->setColumnStretch(2, 10);

   // Initialization
   RasterElement* pRasterElement = NULL;

   WorkspaceWindow* pWindow = mpDesktop->getCurrentWorkspaceWindow();
   if (pWindow != NULL)
   {
      if (pWindow->isKindOf("SpatialDataWindow") == true)
      {
         SpatialDataView* pView = NULL;
         pView = ((SpatialDataWindow*) pWindow)->getSpatialDataView();
         if (pView != NULL)
         {
            LayerList* pLayerList = pView->getLayerList();
            if (pLayerList != NULL)
            {
               pRasterElement = pLayerList->getPrimaryRasterElement();
            }
         }
      }
   }

   if (pRasterElement != NULL)
   {
      vector<string> aoiNames = mpModel->getElementNames(pRasterElement, "AoiElement");

      vector<string>::iterator iter;
      for (iter = aoiNames.begin(); iter != aoiNames.end(); ++iter)
      {
         string name = *iter;
         if (name.empty() == false)
         {
            QString strAoiName = QString::fromStdString(name);
            mAoiNames.append(strAoiName);
         }
      }
   }
   else
   {
      pDataGroup->setEnabled(false);
   }

   setWindowTitle("Add Plot");
   resize(300, 200);
   mpCartesianRadio->setChecked(true);
   updateDataWidgets(mpCartesianRadio);

   // Connections
   connect(mpTypeGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(updateDataWidgets(QAbstractButton*)));
   connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

AddPlotDlg::~AddPlotDlg()
{
}

QString AddPlotDlg::getName() const
{
   QString strName = mpNameEdit->text();
   return strName;
}

PlotType AddPlotDlg::getType() const
{
   PlotType eType;
   if (mpCartesianRadio->isChecked() == true)
   {
      eType = CARTESIAN_PLOT;
   }
   else if (mpHistogramRadio->isChecked() == true)
   {
      eType = HISTOGRAM_PLOT;
   }
   else if (mpSignatureRadio->isChecked() == true)
   {
      eType = SIGNATURE_PLOT;
   }
   else if (mpPolarRadio->isChecked() == true)
   {
      eType = POLAR_PLOT;
   }

   return eType;
}

QString AddPlotDlg::getAxisText(const AxisPosition& eAxis) const
{
   QString strAxis;
   if (eAxis == AXIS_BOTTOM)
   {
      strAxis = mpXAxisEdit->text();
   }
   else if (eAxis == AXIS_LEFT)
   {
      strAxis = mpYAxisEdit->text();
   }

   return strAxis;
}

bool AddPlotDlg::useGridlines() const
{
   bool bGridlines = mpGridlinesCheck->isChecked();
   return bGridlines;
}

bool AddPlotDlg::useLegend() const
{
   bool bLegend = mpLegendCheck->isChecked();
   return bLegend;
}

void AddPlotDlg::setPlot(PlotWidget* pPlot) const
{
   if (pPlot == NULL)
   {
      return;
   }

   PlotView* pPlotView = pPlot->getPlot();
   if (pPlotView == NULL)
   {
      return;
   }

   QRadioButton* pRadio = dynamic_cast<QRadioButton*>(mpTypeGroup->checkedButton());
   if (pRadio == mpPolarRadio)
   {
      PolarPlot* pPolarPlot = dynamic_cast<PolarPlot*>(pPlotView);
      if (pPolarPlot != NULL)
      {
         PolarGridlines* pGridlines = pPolarPlot->getGridlines();
         if (pGridlines != NULL)
         {
            pGridlines->setRadialInterval(22.5);
         }
      }

      RegionObject* pRegion = static_cast<RegionObject*>(pPlotView->addObject(REGION, true));
      if (pRegion != NULL)
      {
         pRegion->setObjectName("Region");
         pRegion->setRegion(5.0, 225.0 * PI / 180.0, 5.0, 45.0 * PI / 180.0);
         pRegion->setColor(ColorType(0, 0, 255));
         pRegion->setTransparency(15);
      }

      PolygonPlotObject* pPolygon = static_cast<PolygonPlotObject*>(pPlotView->addObject(POLYGON_OBJECT_TYPE, true));
      if (pPolygon != NULL)
      {
         pPolygon->setObjectName("Polygon");
         pPolygon->setFillStyle(SOLID_FILL);
         pPolygon->setFillColor(ColorType(0, 255, 0));

         for (unsigned int i = 0; i < 9; ++i)
         {
            pPolygon->addPoint(1.0, ((i * 45.0) + 22.5) * PI / 180.0);
         }
      }

      PointSet* pPointSet = static_cast<PointSet*>(pPlotView->addObject(POINT_SET, true));
      if (pPointSet != NULL)
      {
         pPointSet->setObjectName("Point Set");
         pPointSet->addPoint(0.0, 0.0);
         pPointSet->addPoint(0.5, 45.0 * PI / 180.0);
         pPointSet->addPoint(1.0, 90.0 * PI / 180.0);
         pPointSet->addPoint(1.5, 135.0 * PI / 180.0);
         pPointSet->addPoint(2.0, 180.0 * PI / 180.0);
         pPointSet->addPoint(2.5, 225.0 * PI / 180.0);
         pPointSet->addPoint(3.0, 270.0 * PI / 180.0);
         pPointSet->addPoint(3.5, 315.0 * PI / 180.0);
         pPointSet->addPoint(4.0, 0.0);
         pPointSet->addPoint(4.5, 45.0 * PI / 180.0);
         pPointSet->addPoint(5.0, 90.0 * PI / 180.0);
      }

      Arrow* pArrow = static_cast<Arrow*>(pPlotView->addObject(ARROW, true));
      if (pArrow != NULL)
      {
         pArrow->setObjectName("Arrow");
         pArrow->setBaseLocation(LocationType(5.0, 135.0 * PI / 180.0));
         pArrow->setTipLocation(LocationType(1.0, 180.0 * PI / 180.0));
         pArrow->setArrowStyle(ARROW_TRIANGLE_SMALL_FILL);
         pArrow->setColor(ColorType(255, 0, 0));
      }

      Text* pText = static_cast<Text*>(pPlotView->addObject(TEXT_OBJECT_TYPE, true));
      if (pText != NULL)
      {
         pText->setObjectName("Text");
         pText->setText("Sample Point");
         pText->setLocation(5.0, 135.0 * PI / 180.0);
         pText->setColor(ColorType(255, 0, 0));
      }

      Point* pPoint = static_cast<Point*>(pPlotView->addObject(POINT_OBJECT, true));
      if (pPoint != NULL)
      {
         pPoint->setObjectName("Point");
         pPoint->setLocation(1.0, 180.0 * PI / 180.0);
         pPoint->setSymbol(Point::ASTERISK);
         pPoint->setSymbolSize(5);
      }

      pPlotView->zoomExtents();
   }
   else
   {
      RasterElement* pRasterElement = NULL;

      WorkspaceWindow* pWindow = mpDesktop->getCurrentWorkspaceWindow();
      if (pWindow != NULL)
      {
         SpatialDataView* pView = NULL;

         SpatialDataWindow* pSpatialDataWindow = dynamic_cast<SpatialDataWindow*> (pWindow);
         if (pSpatialDataWindow != NULL)
         {
            pView = pSpatialDataWindow->getSpatialDataView();
         }
         else
         {
            ProductWindow* pProductWindow = dynamic_cast<ProductWindow*> (pWindow);
            if (pProductWindow != NULL)
            {
               ProductView* pProductView = pProductWindow->getProductView();
               if (pProductView != NULL)
               {
                  pView = dynamic_cast<SpatialDataView*> (pProductView->getActiveEditView());
               }
            }
         }

         if (pView != NULL)
         {
            LayerList* pLayerList = pView->getLayerList();
            if (pLayerList != NULL)
            {
               pRasterElement = pLayerList->getPrimaryRasterElement();
            }
         }
      }

      if (pRasterElement == NULL)
      {
         return;
      }

      if ((pRadio == mpCartesianRadio) || (pRadio == mpSignatureRadio))
      {
         AoiElement* pAoi = NULL;

         QString strAoiName = mpDataCombo->currentText();
         if (strAoiName.isEmpty() == false)
         {
            pAoi = static_cast<AoiElement*>(mpModel->getElement(strAoiName.toStdString(),
               "AoiElement", pRasterElement));
         }

         if (pAoi != NULL)
         {
            BitMask* pMask = const_cast<BitMask*>(pAoi->getSelectedPoints());
            if (pMask != NULL)
            {
               int x1, x2, y1, y2;
               pMask->getBoundingBox(x1, y1, x2, y2);

               const bool** pRegion = pMask->getRegion(x1, y1, x2, y2);
               if (pRegion != NULL)
               {
                  vector<Point*> points;
                  for (int i = 0; i <= x2 - x1; i++)
                  {
                     for (int j = 0; j <= y2 - y1; j++)
                     {
                        if (pRegion[j][i] == true)
                        {
                           Point* pPoint = static_cast<Point*>(pPlotView->createObject(POINT_OBJECT, true));
                           if (pPoint != NULL)
                           {
                              pPoint->setLocation(i + x1, j + y1);
                              points.push_back(pPoint);
                           }
                        }
                     }
                  }

                  if (points.empty() == false)
                  {
                     PointSet* pPointSet = static_cast<PointSet*>(pPlotView->addObject(POINT_SET, true));
                     if (pPointSet != NULL)
                     {
                        pPointSet->setObjectName(strAoiName.toStdString());
                        pPointSet->setPoints(points);
                        pPlotView->zoomExtents();
                     }
                  }
               }
            }
         }
      }
      else if (pRadio == mpHistogramRadio)
      {
         HistogramPlot* pHistogramPlot = dynamic_cast<HistogramPlot*>(pPlotView);
         if (pHistogramPlot != NULL)
         {
            QString strOriginalBand = mpDataCombo->currentText();
            unsigned int originalBand = strOriginalBand.toUInt() - 1;

            const RasterDataDescriptor* pDescriptor =
               dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
            if (pDescriptor != NULL)
            {
               DimensionDescriptor bandDim = pDescriptor->getOriginalBand(originalBand);
               if (bandDim.isValid())
               {
                  Statistics* pStatistics = pRasterElement->getStatistics(bandDim);
                  if (pStatistics != NULL)
                  {
                     const double* pHistogramLocations = NULL;
                     const unsigned int* pHistogramCounts = NULL;
                     pStatistics->getHistogram(pHistogramLocations, pHistogramCounts);

                     unsigned int uiCount = 256;
                     unsigned int i = 0;

                     double* pLocations = new double[uiCount];
                     if (pLocations != NULL)
                     {
                        for (i = 0; i < uiCount; i++)
                        {
                           pLocations[i] = (double) pHistogramLocations[i];
                        }
                     }

                     double* pCounts = new double[uiCount];
                     if (pCounts != NULL)
                     {
                        for (i = 0; i < uiCount; i++)
                        {
                           pCounts[i] = (double) pHistogramCounts[i];
                        }
                     }

                     pHistogramPlot->setHistogram(uiCount, pLocations, pCounts);
                     delete pLocations;
                     delete pCounts;
                  }
               }
            }
         }
      }
   }
}

void AddPlotDlg::updateDataWidgets(QAbstractButton* pButton)
{
   mpCartesianDataWidget->show();
   mpDataCombo->clear();

   if (pButton == mpCartesianRadio)
   {
      mpDataLabel->setText("AOI Pixel Coordinates:");
      mpDataCombo->addItems(mAoiNames);
   }
   else if (pButton == mpHistogramRadio)
   {
      mpDataLabel->setText("Spectral Band:");

      RasterElement *pRasterElement = NULL;

      WorkspaceWindow* pWindow = NULL;
      pWindow = mpDesktop->getCurrentWorkspaceWindow();
      if (pWindow != NULL)
      {
         SpatialDataView* pView = NULL;

         SpatialDataWindow* pSpatialDataWindow = dynamic_cast<SpatialDataWindow*> (pWindow);
         if (pSpatialDataWindow != NULL)
         {
            pView = pSpatialDataWindow->getSpatialDataView();
         }
         else
         {
            ProductWindow* pProductWindow = dynamic_cast<ProductWindow*> (pWindow);
            if (pProductWindow != NULL)
            {
               ProductView* pProductView = pProductWindow->getProductView();
               if (pProductView != NULL)
               {
                  pView = dynamic_cast<SpatialDataView*> (pProductView->getActiveEditView());
               }
            }
         }

         if (pView != NULL)
         {
            LayerList* pLayerList = pView->getLayerList();
            if (pLayerList != NULL)
            {
               pRasterElement = pLayerList->getPrimaryRasterElement();
            }
         }
      }

      if (pRasterElement != NULL)
      {
         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            const vector<DimensionDescriptor> &bands = pDescriptor->getBands();

            vector<DimensionDescriptor>::const_iterator iter;
            for (iter = bands.begin(); iter != bands.end(); ++iter)
            {
               const DimensionDescriptor &bandDim = *iter;
               if (bandDim.isValid())
               {
                  unsigned int originalNumber = bandDim.getOriginalNumber() + 1;
                  mpDataCombo->addItem(QString::number(originalNumber));
               }
            }
         }
      }
   }
   else if (pButton == mpSignatureRadio)
   {
      mpDataLabel->setText("AOI Spectrum:");
      mpDataCombo->addItems(mAoiNames);
   }
   else if (pButton == mpPolarRadio)
   {
      mpCartesianDataWidget->hide();
   }
}

void AddPlotDlg::accept()
{
   QString strName = getName();
   if (strName.isEmpty() == true)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME, "Please enter a name for the plot!");
      return;
   }

   QDialog::accept();
}
