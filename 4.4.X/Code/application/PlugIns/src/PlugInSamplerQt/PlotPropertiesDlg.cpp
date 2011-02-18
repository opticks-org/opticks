/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>

#include "AppAssert.h"
#include "AppVerify.h"
#include "CartesianGridlines.h"
#include "CartesianPlot.h"
#include "ColorMap.h"
#include "ColorType.h"
#include "CustomColorButton.h"
#include "Font.h"
#include "GraphicTextWidget.h"
#include "HistogramPlot.h"
#include "ObjectResource.h"
#include "PlotObject.h"
#include "PlotPropertiesDlg.h"
#include "PlotView.h"
#include "PlotWidget.h"
#include "Point.h"
#include "PointSet.h"
#include "PolarGridlines.h"
#include "PolarPlot.h"
#include "StringUtilities.h"
#include "TypesFile.h"

#include <string>
#include <vector>
using namespace std;

PlotPropertiesDlg::PlotPropertiesDlg(PlotWidget* pPlot, QWidget* pParent) :
   QDialog(pParent),
   mpPlot(pPlot),
   mpClassPositionCombo(NULL),
   mpClassText(NULL),
   mpOrgPositionCombo(NULL),
   mpOrgText(NULL),
   mpGridlineStyleCombo(NULL),
   mpGridlineWidthCombo(NULL),
   mpGridlineColorButton(NULL),
   mpLineStyleCombo(NULL),
   mpLineWidthCombo(NULL),
   mpObjectColorButton(NULL),
   mpXScaleCombo(NULL),
   mpYScaleCombo(NULL),
   mpSymbolSizeCombo(NULL),
   mpDisplayMethodCombo(NULL),
   mpSelectionModeCombo(NULL),
   mpSelectionDisplayModeCombo(NULL),
   mpSymbolChooserCombo(NULL),
   mpEnableShading(NULL)
{
   // Classification
   QLabel* pClassLabel = new QLabel("Classification", this);
   QFrame* pClassLine = new QFrame(this);
   pClassLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QFont classFont(pClassLabel->font());
   classFont.setBold(true);
   pClassLabel->setFont(classFont);

   QLabel* pClassPositionLabel = new QLabel("Position:", this);

   mpClassPositionCombo = new QComboBox(this);
   mpClassPositionCombo->setEditable(false);
   mpClassPositionCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(TOP_LEFT_BOTTOM_LEFT)));
   mpClassPositionCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(TOP_LEFT_BOTTOM_RIGHT)));
   mpClassPositionCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(CENTER)));
   mpClassPositionCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(TOP_RIGHT_BOTTOM_LEFT)));
   mpClassPositionCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(TOP_RIGHT_BOTTOM_RIGHT)));

   mpClassText = new GraphicTextWidget(this);

   // Organization
   QLabel* pOrgLabel = new QLabel("Organization", this);
   QFrame* pOrgLine = new QFrame(this);
   pOrgLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QFont orgFont(pOrgLabel->font());
   orgFont.setBold(true);
   pOrgLabel->setFont(orgFont);

   QLabel* pOrgPositionLabel = new QLabel("Position:", this);

   mpOrgPositionCombo = new QComboBox(this);
   mpOrgPositionCombo->setEditable(false);
   mpOrgPositionCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(TOP_LEFT_BOTTOM_LEFT)));
   mpOrgPositionCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(TOP_LEFT_BOTTOM_RIGHT)));
   mpOrgPositionCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(CENTER)));
   mpOrgPositionCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(TOP_RIGHT_BOTTOM_LEFT)));
   mpOrgPositionCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(TOP_RIGHT_BOTTOM_RIGHT)));

   mpOrgText = new GraphicTextWidget(this);

   // Gridlines
   QLabel* pGridlineLabel = new QLabel("Gridlines", this);
   QFrame* pGridlineLine = new QFrame(this);
   pGridlineLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QFont gridlineFont(pGridlineLabel->font());
   gridlineFont.setBold(true);
   pGridlineLabel->setFont(gridlineFont);

   // Gridline style
   QLabel* pGridlineStyleLabel = new QLabel("Style:", this);

   mpGridlineStyleCombo = new QComboBox(this);
   mpGridlineStyleCombo->setEditable(false);
   mpGridlineStyleCombo->setFixedWidth(175);
   mpGridlineStyleCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(SOLID_LINE)));
   mpGridlineStyleCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DASHED)));
   mpGridlineStyleCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DOT)));
   mpGridlineStyleCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DASH_DOT)));
   mpGridlineStyleCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DASH_DOT_DOT)));

   // Gridline width
   QSize lineWidthSize(100, 16);

   QLabel* pGridlineWidthLabel = new QLabel("Width:", this);
   mpGridlineWidthCombo = new QComboBox(this);
   mpGridlineWidthCombo->setEditable(false);
   mpGridlineWidthCombo->setIconSize(lineWidthSize);
   mpGridlineWidthCombo->setFixedWidth(175);

   for (int i = 0; i < 6; i++)
   {
      QPainter p;
      QPixmap pix = QPixmap(lineWidthSize);
      p.begin(&pix);
      p.fillRect(pix.rect(), Qt::white);
      p.setPen(QPen(Qt::black, i + 1));
      p.drawLine(pix.rect().left(), pix.height() / 2, pix.rect().right(), pix.height() / 2);
      p.end();

      mpGridlineWidthCombo->addItem(QIcon(pix), QString());
   }

   // Gridline color
   mpGridlineColorButton = new CustomColorButton("Color...", this);
   mpGridlineColorButton->usePopupGrid(true);

   // Axes

   PlotView* pPlotView = mpPlot->getPlot();
   REQUIRE(pPlotView != NULL);

   PlotType ePlotType = pPlotView->getPlotType();

   QHBoxLayout* pAxesLayout = NULL;
   QLabel* pXScaleLabel = NULL;
   QLabel* pYScaleLabel = NULL;

   if (ePlotType != POLAR_PLOT)
   {
      QLabel* pAxesLabel = new QLabel("Axes", this);
      QFrame* pAxesLine = new QFrame(this);
      pAxesLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

      QFont axesFont(pAxesLabel->font());
      axesFont.setBold(true);
      pAxesLabel->setFont(axesFont);

      pAxesLayout = new QHBoxLayout();
      pAxesLayout->setMargin(0);
      pAxesLayout->setSpacing(5);
      pAxesLayout->addWidget(pAxesLabel);
      pAxesLayout->addWidget(pAxesLine, 10);

      pXScaleLabel = new QLabel("X Scale:", this);
      mpXScaleCombo = new QComboBox(this);
      mpXScaleCombo->setEditable(false);
      mpXScaleCombo->addItem("Linear");
      mpXScaleCombo->addItem("Log");

      pYScaleLabel = new QLabel("Y Scale:", this);
      mpYScaleCombo = new QComboBox(this);
      mpYScaleCombo->setEditable(false);
      mpYScaleCombo->addItem("Linear");
      mpYScaleCombo->addItem("Log");
   }

   // Plot
   QWidget* pPlotWidget = NULL;

   if (mpPlot != NULL)
   {
      pPlotWidget = new QWidget(this);

      QLabel* pPlotLabel = new QLabel(pPlotWidget);
      QFrame* pPlotLine = new QFrame(pPlotWidget);
      pPlotLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

      QFont plotFont(pPlotLabel->font());
      plotFont.setBold(true);
      pPlotLabel->setFont(plotFont);

      if ((ePlotType == CARTESIAN_PLOT) || (ePlotType == POLAR_PLOT) || (ePlotType == SIGNATURE_PLOT))
      {
         pPlotLabel->setText("Point Sets");
      }
      else if (ePlotType == HISTOGRAM_PLOT)
      {
         pPlotLabel->setText("Histogram");
      }

      QHBoxLayout* pPlotLayout = new QHBoxLayout();
      pPlotLayout->setMargin(0);
      pPlotLayout->setSpacing(5);
      pPlotLayout->addWidget(pPlotLabel);
      pPlotLayout->addWidget(pPlotLine, 10);

      mpObjectColorButton = new CustomColorButton("Color...", pPlotWidget);
      mpObjectColorButton->usePopupGrid(true);

      QGridLayout* pPlotGrid = new QGridLayout(pPlotWidget);
      pPlotGrid->setMargin(0);
      pPlotGrid->setSpacing(5);
      pPlotGrid->addLayout(pPlotLayout, 0, 0, 1, 2);

      if ((ePlotType == CARTESIAN_PLOT) || (ePlotType == POLAR_PLOT) || (ePlotType == SIGNATURE_PLOT))
      {
         QLabel* pLineStyleLabel = new QLabel("Style:", pPlotWidget);
         mpLineStyleCombo = new QComboBox(pPlotWidget);
         mpLineStyleCombo->setEditable(false);
         mpLineStyleCombo->setFixedWidth(175);
         mpLineStyleCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(SOLID_LINE)));
         mpLineStyleCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DASHED)));
         mpLineStyleCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DOT)));
         mpLineStyleCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DASH_DOT)));
         mpLineStyleCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(DASH_DOT_DOT)));

         QLabel* pLineWidthLabel = new QLabel("Width:", pPlotWidget);
         mpLineWidthCombo = new QComboBox(pPlotWidget);
         mpLineWidthCombo->setEditable(false);
         mpLineWidthCombo->setIconSize(lineWidthSize);
         mpLineWidthCombo->setFixedWidth(175);

         for (int i = 0; i < 6; i++)
         {
            QPainter p;
            QPixmap pix = QPixmap(lineWidthSize);
            p.begin(&pix);
            p.fillRect(pix.rect(), Qt::white);
            p.setPen(QPen(Qt::black, i + 1));
            p.drawLine(pix.rect().left(), pix.height() / 2, pix.rect().right(), pix.height() / 2);
            p.end();

            mpLineWidthCombo->addItem(QIcon(pix), QString());
         }

         // Create the symbol size combo
         QLabel* pSymbolSizeLabel = new QLabel("Symbol Size:", pPlotWidget);
         mpSymbolSizeCombo = new QComboBox(pPlotWidget);
         mpSymbolSizeCombo->setEditable(false);
         mpSymbolSizeCombo->setFixedWidth(175);
         for (int i = 1; i < 21; i++)
         {
            mpSymbolSizeCombo->addItem(QString::number(i));
         }

         // Create the display method combo
         QLabel* pDisplayMethodLabel = new QLabel("Display Method:", pPlotWidget);
         mpDisplayMethodCombo = new QComboBox(pPlotWidget);
         mpDisplayMethodCombo->setEditable(false);
         mpDisplayMethodCombo->setFixedWidth(175);
         mpDisplayMethodCombo->addItem("Symbols Only");
         mpDisplayMethodCombo->addItem("Lines Only");
         mpDisplayMethodCombo->addItem("Both");

         // Create the selection mode combo
         QLabel* pSelectionModeLabel = new QLabel("Selection Mode:", pPlotWidget);
         mpSelectionModeCombo = new QComboBox(pPlotWidget);
         mpSelectionModeCombo->setEditable(false);
         mpSelectionModeCombo->setFixedWidth(175);
         mpSelectionModeCombo->addItem("Normal Selection");
         mpSelectionModeCombo->addItem("Deep Selection");
         
         // Create the selection display mode combo
         QLabel* pSelectionDisplayModeLabel = new QLabel("Selection Display:", pPlotWidget);
         mpSelectionDisplayModeCombo = new QComboBox(pPlotWidget);
         mpSelectionDisplayModeCombo->setEditable(false);
         mpSelectionDisplayModeCombo->setFixedWidth(175);
         mpSelectionDisplayModeCombo->addItem("Box Selection");
         mpSelectionDisplayModeCombo->addItem("Invert Selection");
         mpSelectionDisplayModeCombo->addItem("Symbol Selection");
         
         // Create the symbol chooser combo
         QLabel* pSymbolChooserLabel = new QLabel("Symbol:", pPlotWidget);
         mpSymbolChooserCombo = new QComboBox(pPlotWidget);
         mpSymbolChooserCombo->setEditable(false);
         mpSymbolChooserCombo->setFixedWidth(175);
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::SOLID)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::X)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::CROSS_HAIR)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::ASTERISK)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::HORIZONTAL_LINE)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::VERTICAL_LINE)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::FORWARD_SLASH)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::BACK_SLASH)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::BOX)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::BOXED_X)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::BOXED_CROSS_HAIR)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::BOXED_ASTERISK)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::BOXED_HORIZONTAL_LINE)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::BOXED_VERTICAL_LINE)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::BOXED_FORWARD_SLASH)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::BOXED_BACK_SLASH)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::DIAMOND)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::DIAMOND_FILLED)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::DIAMOND_CROSS_HAIR)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::TRIANGLE)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::TRIANGLE_FILLED)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::RIGHT_TRIANGLE)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::RIGHT_TRIANGLE_FILLED)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::LEFT_TRIANGLE)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::LEFT_TRIANGLE_FILLED)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::DOWN_TRIANGLE)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::DOWN_TRIANGLE_FILLED)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::CIRCLE)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::CIRCLE_FILLED)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::OCTAGON)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(Point::OCTAGON_FILLED)));
         mpSymbolChooserCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(
            Point::OCTAGON_CROSS_HAIR)));

         // Create the shading check box
         mpEnableShading = new QCheckBox("Enable Shading");
         mpEnableShading->setChecked(false);

         pPlotGrid->addWidget(pLineStyleLabel, 1, 0);
         pPlotGrid->addWidget(mpLineStyleCombo, 1, 1, Qt::AlignLeft);
         pPlotGrid->addWidget(pLineWidthLabel, 2, 0);
         pPlotGrid->addWidget(mpLineWidthCombo, 2, 1, Qt::AlignLeft);
         pPlotGrid->addWidget(mpObjectColorButton, 3, 0, 1, 2, Qt::AlignLeft);
         pPlotGrid->addWidget(pSymbolSizeLabel, 4, 0);
         pPlotGrid->addWidget(mpSymbolSizeCombo, 4, 1, Qt::AlignLeft);
         pPlotGrid->addWidget(pDisplayMethodLabel, 5, 0);
         pPlotGrid->addWidget(mpDisplayMethodCombo, 5, 1, Qt::AlignLeft);
         pPlotGrid->addWidget(pSelectionModeLabel, 6, 0);
         pPlotGrid->addWidget(mpSelectionModeCombo, 6, 1, Qt::AlignLeft);
         pPlotGrid->addWidget(pSelectionDisplayModeLabel, 7, 0);
         pPlotGrid->addWidget(mpSelectionDisplayModeCombo, 7, 1, Qt::AlignLeft);
         pPlotGrid->addWidget(pSymbolChooserLabel, 8, 0);
         pPlotGrid->addWidget(mpSymbolChooserCombo, 8, 1, Qt::AlignLeft);
         pPlotGrid->addWidget(mpEnableShading, 9, 0);
      }
      else if (ePlotType == HISTOGRAM_PLOT)
      {
         pPlotGrid->addWidget(mpObjectColorButton, 1, 0, 1, 2, Qt::AlignLeft);
      }

      pPlotGrid->setColumnMinimumWidth(0, 40);
      pPlotGrid->setRowStretch(0, 10);
      pPlotGrid->setColumnStretch(1, 10);
   }

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pApplyButton = new QPushButton("&Apply", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Layout
   QHBoxLayout* pClassLayout = new QHBoxLayout();
   pClassLayout->setMargin(0);
   pClassLayout->setSpacing(5);
   pClassLayout->addWidget(pClassLabel);
   pClassLayout->addWidget(pClassLine, 10);

   QHBoxLayout* pOrgLayout = new QHBoxLayout();
   pOrgLayout->setMargin(0);
   pOrgLayout->setSpacing(5);
   pOrgLayout->addWidget(pOrgLabel);
   pOrgLayout->addWidget(pOrgLine, 10);

   QHBoxLayout* pGridlineLayout = new QHBoxLayout();
   pGridlineLayout->setMargin(0);
   pGridlineLayout->setSpacing(5);
   pGridlineLayout->addWidget(pGridlineLabel);
   pGridlineLayout->addWidget(pGridlineLine, 10);

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(pOkButton);
   pButtonLayout->addWidget(pApplyButton);
   pButtonLayout->addWidget(pCancelButton);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);
   pGrid->addLayout(pClassLayout, 0, 0, 1, 2);
   pGrid->addWidget(pClassPositionLabel, 1, 0);
   pGrid->addWidget(mpClassPositionCombo, 1, 1, Qt::AlignLeft);
   pGrid->addWidget(mpClassText, 2, 0, 1, 2);
   pGrid->setColumnMinimumWidth(2, 15);
   pGrid->addLayout(pOrgLayout, 0, 3, 1, 2);
   pGrid->addWidget(pOrgPositionLabel, 1, 3);
   pGrid->addWidget(mpOrgPositionCombo, 1, 4, Qt::AlignLeft);
   pGrid->addWidget(mpOrgText, 2, 3, 1, 2);
   pGrid->setRowMinimumHeight(3, 15);
   pGrid->addLayout(pGridlineLayout, 4, 0, 1, 2);
   pGrid->addWidget(pGridlineStyleLabel, 5, 0);
   pGrid->addWidget(mpGridlineStyleCombo, 5, 1, Qt::AlignLeft);
   pGrid->addWidget(pGridlineWidthLabel, 6, 0);
   pGrid->addWidget(mpGridlineWidthCombo, 6, 1, Qt::AlignLeft);
   pGrid->addWidget(mpGridlineColorButton, 7, 0, 1, 2, Qt::AlignLeft);

   if (ePlotType != POLAR_PLOT)
   {
      pGrid->setRowMinimumHeight(8, 15);
      pGrid->addLayout(pAxesLayout, 9, 0, 1, 2);
      pGrid->addWidget(pXScaleLabel, 10, 0);
      pGrid->addWidget(mpXScaleCombo, 10, 1, Qt::AlignLeft);
      pGrid->addWidget(pYScaleLabel, 11, 0);
      pGrid->addWidget(mpYScaleCombo, 11, 1, Qt::AlignLeft);
   }

   if (pPlotWidget != NULL)
   {
      pGrid->addWidget(pPlotWidget, 4, 3, 8, 2, Qt::AlignTop);
   }

   pGrid->addWidget(pHLine, 12, 0, 1, 5, Qt::AlignBottom);
   pGrid->setRowMinimumHeight(12, 12);
   pGrid->addLayout(pButtonLayout, 13, 0, 1, 5);
   pGrid->setRowStretch(2, 10);
   pGrid->setColumnStretch(1, 10);
   pGrid->setColumnStretch(4, 10);

   // Initialization
   setWindowTitle("Plot Properties");
   pOkButton->setFocus();

   if (mpPlot != NULL)
   {
      // Classification
      PositionType eClassPosition = mpPlot->getClassificationPosition();

      string classPosition = StringUtilities::toDisplayString(eClassPosition);
      if (classPosition.empty() == false)
      {
         int iIndex = mpClassPositionCombo->findText(QString::fromStdString(classPosition));
         if (iIndex != -1)
         {
            mpClassPositionCombo->setCurrentIndex(iIndex);
         }
      }

      mpClassText->setText(QString::fromStdString(mpPlot->getClassificationText()));
      mpClassText->setTextFont(mpPlot->getClassificationFont().getQFont());
      mpClassText->setColor(COLORTYPE_TO_QCOLOR(mpPlot->getClassificationColor()));

      // Organization
      PositionType eOrgPosition = mpPlot->getOrganizationPosition();
      string orgPosition = StringUtilities::toDisplayString(eOrgPosition);
      if (orgPosition.empty() == false)
      {
         int iIndex = mpOrgPositionCombo->findText(QString::fromStdString(orgPosition));
         if (iIndex != -1)
         {
            mpOrgPositionCombo->setCurrentIndex(iIndex);
         }
      }

      mpOrgText->setText(QString::fromStdString(mpPlot->getOrganizationText()));
      mpOrgText->setTextFont(mpPlot->getOrganizationFont().getQFont());
      mpOrgText->setColor(COLORTYPE_TO_QCOLOR(mpPlot->getOrganizationColor()));

      // Selection mode
      if (mpSelectionModeCombo != NULL)
      {
         PlotSelectionModeType selectionMode = pPlotView->getSelectionMode();
         if (selectionMode == NORMAL_SELECTION)
         {
            mpSelectionModeCombo->setCurrentIndex(0);
         }
         else if (selectionMode == DEEP_SELECTION)
         {
            mpSelectionModeCombo->setCurrentIndex(1);
         }
      }

      // Selection display mode
      if (mpSelectionDisplayModeCombo != NULL)
      {
         PointSelectionDisplayType displayType = pPlotView->getSelectionDisplayMode();
         if (displayType == BOX_SELECTION)
         {
            mpSelectionDisplayModeCombo->setCurrentIndex(0);
         }
         else if (displayType == INVERT_SELECTION)
         {
            mpSelectionDisplayModeCombo->setCurrentIndex(1);
         }
         else if (displayType == SYMBOL_SELECTION)
         {
            mpSelectionDisplayModeCombo->setCurrentIndex(2);
         }
      }

      // Shading mode
      if (mpEnableShading != NULL)
      {
         mpEnableShading->setChecked(pPlotView->isShadingEnabled());
      }

      // Gridlines
      CartesianPlot* pCartesianPlot = dynamic_cast<CartesianPlot*>(pPlotView);
      if (pCartesianPlot != NULL)
      {
         Gridlines* pHorizGridlines = pCartesianPlot->getGridlines(HORIZONTAL);
         Gridlines* pVertGridlines = pCartesianPlot->getGridlines(VERTICAL);

         if ((pHorizGridlines != NULL) && (pVertGridlines != NULL))
         {
            ColorType gridlineColor = pHorizGridlines->getColor();
            int lineWidth = pHorizGridlines->getLineWidth();
            LineStyle lineStyle = pHorizGridlines->getLineStyle();

            // Color
            if (pVertGridlines->getColor() == gridlineColor)
            {
               if (gridlineColor.isValid() == true)
               {
                  mpGridlineColorButton->setColor(gridlineColor);
               }
            }

            // Line width
            if (pVertGridlines->getLineWidth() == lineWidth)
            {
               int iCount = mpGridlineWidthCombo->count();
               if (lineWidth > iCount)
               {
                  lineWidth = iCount;
               }

               if (lineWidth > 0)
               {
                  mpGridlineWidthCombo->setCurrentIndex(lineWidth - 1);
               }
            }

            // Line style
            if (pVertGridlines->getLineStyle() == lineStyle)
            {
               string styleText = StringUtilities::toDisplayString(lineStyle);
               if (styleText.empty() == false)
               {
                  int index = mpGridlineStyleCombo->findText(QString::fromStdString(styleText));
                  if (index != -1)
                  {
                     mpGridlineStyleCombo->setCurrentIndex(index);
                  }
               }
            }
         }
      }

      PolarPlot* pPolarPlot = dynamic_cast<PolarPlot*>(pPlotView);
      if (pPolarPlot != NULL)
      {
         Gridlines* pGridlines = pPolarPlot->getGridlines();
         if (pGridlines != NULL)
         {
            ColorType gridlineColor = pGridlines->getColor();
            int lineWidth = pGridlines->getLineWidth();
            LineStyle lineStyle = pGridlines->getLineStyle();

            // Color
            if (gridlineColor.isValid() == true)
            {
               mpGridlineColorButton->setColor(gridlineColor);
            }

            // Line width
            int iCount = mpGridlineWidthCombo->count();
            if (lineWidth > iCount)
            {
               lineWidth = iCount;
            }

            if (lineWidth > 0)
            {
               mpGridlineWidthCombo->setCurrentIndex(lineWidth - 1);
            }

            // Line style
            string styleText = StringUtilities::toDisplayString(lineStyle);
            if (styleText.empty() == false)
            {
               int index = mpGridlineStyleCombo->findText(QString::fromStdString(styleText));
               if (index != -1)
               {
                  mpGridlineStyleCombo->setCurrentIndex(index);
               }
            }
         }
      }

      // Axis scale
      if (pCartesianPlot != NULL)
      {
         ScaleType xScale = pCartesianPlot->getXScaleType();
         if (xScale == SCALE_LINEAR)
         {
            mpXScaleCombo->setCurrentIndex(0);
         }
         else if (xScale == SCALE_LOG)
         {
            mpXScaleCombo->setCurrentIndex(1);
         }

         ScaleType yScale = pCartesianPlot->getYScaleType();
         if (yScale == SCALE_LINEAR)
         {
            mpYScaleCombo->setCurrentIndex(0);
         }
         else if (yScale == SCALE_LOG)
         {
            mpYScaleCombo->setCurrentIndex(1);
         }
      }

      // Plot objects
      if ((ePlotType == CARTESIAN_PLOT) || (ePlotType == SIGNATURE_PLOT))
      {
         list<PlotObject*> objects;
         pPlotView->getObjects(POINT_SET, objects);

         // Line color
         ColorType objectColor;

         list<PlotObject*>::iterator iter = objects.begin();
         while (iter != objects.end())
         {
            PlotObject* pObject = *iter++;
            if (pObject != NULL)
            {
               PointSet* pPointSet = dynamic_cast<PointSet*> (pObject);
               if (pPointSet != NULL)
               {
                  ColorType currentColor = pPointSet->getLineColor();
                  if (currentColor == objectColor)
                  {
                     continue;
                  }

                  if (objectColor.isValid() == true)
                  {
                     objectColor = ColorType();
                     break;
                  }

                  objectColor = currentColor;

                  // Get the list of points to get the current symbol and symbol size
                  std::vector<Point*> points = pPointSet->getPoints();
                  if (points.empty() == false)
                  {
                     // Only check the first point
                     Point* pPoint = points.front();
                     if (pPoint != NULL)
                     {
                        mpSymbolSizeCombo->setCurrentIndex(pPoint->getSymbolSize()-1);
                        int index = mpSymbolChooserCombo->findText(QString::fromStdString(
                           StringUtilities::toDisplayString(pPoint->getSymbol())));
                        if (index != -1)
                        {
                           mpSymbolChooserCombo->setCurrentIndex(index);
                        }
                     }
                  }

                  // Get the display method
                  bool symbols = pPointSet->areSymbolsDisplayed();
                  bool lines = pPointSet->isLineDisplayed();
                  if (symbols == true && lines == false)
                  {
                     mpDisplayMethodCombo->setCurrentIndex(0);
                  }
                  else if (lines == true && symbols == false)
                  {
                     mpDisplayMethodCombo->setCurrentIndex(1);
                  }
                  else if (lines == true && symbols == true)
                  {
                     mpDisplayMethodCombo->setCurrentIndex(2);
                  }
               }
            }
         }

         if (objectColor.isValid() == true)
         {
            mpObjectColorButton->setColor(objectColor);
         }

         // Line width
         int iLineWidth = -1;

         iter = objects.begin();
         while (iter != objects.end())
         {
            PlotObject* pObject = *iter++;
            if (pObject != NULL)
            {
               PointSet* pPointSet = dynamic_cast<PointSet*> (pObject);
               if (pPointSet != NULL)
               {
                  int iCurrentWidth = pPointSet->getLineWidth();
                  if (iCurrentWidth == iLineWidth)
                  {
                     continue;
                  }

                  if (iLineWidth > 0)
                  {
                     iLineWidth = -1;
                     break;
                  }

                  iLineWidth = iCurrentWidth;
               }
            }
         }

         int iNumWidths = mpLineWidthCombo->count();
         if (iLineWidth > iNumWidths)
         {
            iLineWidth = iNumWidths;
         }

         if (iLineWidth > 0)
         {
            mpLineWidthCombo->setCurrentIndex(iLineWidth - 1);
         }

         // Line style
         LineStyle* pLineStyle = NULL;

         iter = objects.begin();
         while (iter != objects.end())
         {
            PlotObject* pObject = *iter++;
            if (pObject != NULL)
            {
               PointSet* pPointSet = dynamic_cast<PointSet*> (pObject);
               if (pPointSet != NULL)
               {
                  LineStyle eCurrentStyle = pPointSet->getLineStyle();
                  if (pLineStyle == NULL)
                  {
                     pLineStyle = new LineStyle;
                     if (pLineStyle != NULL)
                     {
                        *pLineStyle = eCurrentStyle;
                     }
                  }

                  if (pLineStyle != NULL)
                  {
                     if (eCurrentStyle == *pLineStyle)
                     {
                        continue;
                     }

                     delete pLineStyle;
                     pLineStyle = NULL;
                     break;
                  }
               }
            }
         }

         QString strLineStyle;
         if (pLineStyle != NULL)
         {
            string lineStyle = StringUtilities::toDisplayString(*pLineStyle);
            if (lineStyle.empty() == false)
            {
               strLineStyle = QString::fromStdString(lineStyle);
            }

            delete pLineStyle;
         }

         int iIndex = mpLineStyleCombo->findText(strLineStyle);
         if (iIndex != -1)
         {
            mpLineStyleCombo->setCurrentIndex(iIndex);
         }
      }
      else if (ePlotType == HISTOGRAM_PLOT)
      {
         HistogramPlot* pHistogramPlot = static_cast<HistogramPlot*>(pPlotView);

         // Color
         ColorType histogramColor = pHistogramPlot->getHistogramColor();
         if (histogramColor.isValid() == true)
         {
            mpObjectColorButton->setColor(histogramColor);
         }
      }
   }

   // Connections
   connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pApplyButton, SIGNAL(clicked()), this, SLOT(applyChanges()));
   connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

PlotPropertiesDlg::~PlotPropertiesDlg()
{
}

void PlotPropertiesDlg::accept()
{
   applyChanges();
   QDialog::accept();
}

void PlotPropertiesDlg::applyChanges()
{
   if (mpPlot == NULL)
   {
      return;
   }

   PlotView* pPlotView = mpPlot->getPlot();
   VERIFYNRV(pPlotView != NULL);

   // Selection mode
   if (mpSelectionModeCombo != NULL)
   {
      int selectionMode = mpSelectionModeCombo->currentIndex();
      if (selectionMode == 0)
      {
         pPlotView->setSelectionMode(NORMAL_SELECTION);
      }
      else if (selectionMode == 1)
      {
         pPlotView->setSelectionMode(DEEP_SELECTION);
      }
   }

   // Selection display mode
   if (mpSelectionDisplayModeCombo != NULL)
   {
      PointSelectionDisplayType eSelectionDisplay;
      int displayType = mpSelectionDisplayModeCombo->currentIndex();
      if (displayType == 0)
      {
         eSelectionDisplay = BOX_SELECTION;
      }
      else if (displayType == 1)
      {
         eSelectionDisplay = INVERT_SELECTION;
      }
      else if (displayType == 2)
      {
         eSelectionDisplay = SYMBOL_SELECTION;
      }
      pPlotView->setSelectionDisplayMode(eSelectionDisplay);
   }

   // Shading mode
   if (mpEnableShading != NULL)
   {
      pPlotView->setEnableShading(mpEnableShading->isChecked());
   }

   // Classification position
   PositionType eClassPosition = CENTER;

   QString strClassPosition = mpClassPositionCombo->currentText();
   if (strClassPosition.isEmpty() == false)
   {
      eClassPosition = StringUtilities::fromDisplayString<PositionType>(strClassPosition.toStdString());
   }

   mpPlot->setClassificationPosition(eClassPosition);

   // Classification text
   FactoryResource<Font> pClassFont;
   pClassFont->setQFont(mpClassText->getTextFont());
   mpPlot->setClassificationFont(*pClassFont.get());
   mpPlot->setClassificationText(mpClassText->getText().toStdString());
   mpPlot->setClassificationColor(QCOLOR_TO_COLORTYPE(mpClassText->getColor()));

   // Organization position
   PositionType eOrgPosition = TOP_RIGHT_BOTTOM_LEFT;

   QString strOrgPosition = mpOrgPositionCombo->currentText();
   if (strOrgPosition.isEmpty() == false)
   {
      eOrgPosition = StringUtilities::fromDisplayString<PositionType>(strOrgPosition.toStdString());
   }

   mpPlot->setOrganizationPosition(eOrgPosition);

   // Organization text
   FactoryResource<Font> pOrgFont;
   pOrgFont->setQFont(mpOrgText->getTextFont());
   mpPlot->setOrganizationFont(*pOrgFont.get());
   mpPlot->setOrganizationText(mpOrgText->getText().toStdString());
   mpPlot->setOrganizationColor(QCOLOR_TO_COLORTYPE(mpOrgText->getColor()));

   // Gridlines
   LineStyle lineStyle = DOT;

   QString strGridlineStyle = mpGridlineStyleCombo->currentText();
   if (strGridlineStyle.isEmpty() == false)
   {
      lineStyle = StringUtilities::fromDisplayString<LineStyle>(strGridlineStyle.toStdString());
   }

   int lineWidth = mpGridlineWidthCombo->currentIndex() + 1;
   ColorType gridlineColor = mpGridlineColorButton->getColorType();

   CartesianPlot* pCartesianPlot = dynamic_cast<CartesianPlot*>(pPlotView);
   if (pCartesianPlot != NULL)
   {
      Gridlines* pHorizGridlines = pCartesianPlot->getGridlines(HORIZONTAL);
      if (pHorizGridlines != NULL)
      {
         pHorizGridlines->setColor(gridlineColor);
         pHorizGridlines->setLineWidth(lineWidth);
         pHorizGridlines->setLineStyle(lineStyle);
      }

      Gridlines* pVertGridlines = pCartesianPlot->getGridlines(VERTICAL);
      if (pVertGridlines != NULL)
      {
         pVertGridlines->setColor(gridlineColor);
         pVertGridlines->setLineWidth(lineWidth);
         pVertGridlines->setLineStyle(lineStyle);
      }
   }

   PolarPlot* pPolarPlot = dynamic_cast<PolarPlot*>(pPlotView);
   if (pPolarPlot != NULL)
   {
      Gridlines* pGridlines = pPolarPlot->getGridlines();
      if (pGridlines != NULL)
      {
         pGridlines->setColor(gridlineColor);
         pGridlines->setLineWidth(lineWidth);
         pGridlines->setLineStyle(lineStyle);
      }
   }

   // Scale
   if (pCartesianPlot != NULL)
   {
      int scaleIndex = mpXScaleCombo->currentIndex();
      if (scaleIndex == 0)
      {
         pCartesianPlot->setXScaleType(SCALE_LINEAR);
      }
      else if (scaleIndex == 1)
      {
         pCartesianPlot->setXScaleType(SCALE_LOG);
      }

      scaleIndex = mpYScaleCombo->currentIndex();
      if (scaleIndex == 0)
      {
         pCartesianPlot->setYScaleType(SCALE_LINEAR);
      }
      else if (scaleIndex == 1)
      {
         pCartesianPlot->setYScaleType(SCALE_LOG);
      }
   }

   // Create a default colormap to test shading
   Service<ConfigurationSettings> pConfigSettings;
   std::string colorFile = pConfigSettings->getHome();
   colorFile.append( SLASH + "SupportFiles" + SLASH + "ColorTables" + SLASH + "RedTemp.cgr");
   ColorMap colorMap(colorFile);

   // Setup temporary min and max values
   double min = 0;
   double max = 200;

   // Plot objects
   PlotType ePlotType = pPlotView->getPlotType();
   if ((ePlotType == CARTESIAN_PLOT) || (ePlotType == POLAR_PLOT) || (ePlotType == SIGNATURE_PLOT))
   {
      list<PlotObject*> objects;
      pPlotView->getObjects(POINT_SET, objects);

      list<PlotObject*>::iterator iter = objects.begin();
      while (iter != objects.end())
      {
         PlotObject* pObject = *iter;
         if (pObject != NULL)
         {
            PointSet* pPointSet = dynamic_cast<PointSet*> (pObject);
            if (pPointSet != NULL)
            {
               // Line color
               pPointSet->setLineColor(mpObjectColorButton->getColorType());

               // Line style
               LineStyle eLineStyle;

               QString strLineStyle = mpLineStyleCombo->currentText();
               if (strLineStyle.isEmpty() == false)
               {
                  eLineStyle = StringUtilities::fromDisplayString<LineStyle>(strLineStyle.toStdString());
               }

               pPointSet->setLineStyle(eLineStyle);

               // Line width
               int iLineWidth = mpLineWidthCombo->currentIndex() + 1;
               pPointSet->setLineWidth(iLineWidth);

               // Set the Display Method
               QString strDisplayMethod = mpDisplayMethodCombo->currentText();
               bool lines = false;
               bool symbols = false;
               if (strDisplayMethod == "Both")
               {
                  lines = true;
                  symbols = true;
               }
               else if (strDisplayMethod == "Lines Only")
               {
                  lines = true;
                  symbols = false;
               }
               else if (strDisplayMethod == "Symbols Only")
               {
                  lines = false;
                  symbols = true;
               }
               pPointSet->displayLine(lines);
               pPointSet->displaySymbols(symbols);

               // Set Symbol and Symbol Size
               Point::PointSymbolType eSymbol = StringUtilities::fromDisplayString<Point::PointSymbolType>(
                  mpSymbolChooserCombo->currentText().toStdString());
               int symbolSize = mpSymbolSizeCombo->currentText().toInt();

               // Get the list of points to set the current symbol and symbol size
               std::vector<Point*> points = pPointSet->getPoints();
               std::vector<Point*>::iterator pntIter;
               for (pntIter = points.begin(); pntIter != points.end(); pntIter++)
               {
                  Point* pPoint = *pntIter;
                  pPoint->setSymbol(eSymbol);
                  pPoint->setSymbolSize(symbolSize);

                  ColorType symbolColor = mpObjectColorButton->getColorType();
                  if (pPlotView->isShadingEnabled() == true)
                  {
                     int colorSize = colorMap.getTable().size() - 1;
                     double value = pPoint->getXLocation();
                     if (value >= min && value <= max)
                     {
                        symbolColor = colorMap[((value - min)/(max-min))*colorSize];
                     }
                     else if (value > max)
                     {
                        symbolColor = colorMap[colorSize];
                     }
                     else
                     {
                        symbolColor = colorMap[0];
                     }
                  }
                  pPoint->setColor(symbolColor);
               }
            }
         }

         ++iter;
      }
   }
   else if (ePlotType == HISTOGRAM_PLOT)
   {
      HistogramPlot* pHistogramPlot = static_cast<HistogramPlot*>(pPlotView);

      // Color
      pHistogramPlot->setHistogramColor(mpObjectColorButton->getColorType());
   }

   pPlotView->refresh();
}
