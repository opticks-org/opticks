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

#include "AppVersion.h"
#include "CartesianGridlines.h"
#include "CartesianPlot.h"
#include "CustomColorButton.h"
#include "LabeledSection.h"
#include "LineStyleComboBox.h"
#include "LineWidthComboBox.h"
#include "PlotView.h"
#include "PlotWidget.h"
#include "PolarGridlines.h"
#include "PolarPlot.h"
#include "PropertiesPlotView.h"
#include "Undo.h"

using namespace std;

PropertiesPlotView::PropertiesPlotView() :
   LabeledSectionGroup(NULL),
   mpPlotView(NULL)
{
   // Gridlines
   QWidget* pGridlinesWidget = new QWidget(this);

   QLabel* pStyleLabel = new QLabel("Style:", pGridlinesWidget);
   mpStyleCombo = new LineStyleComboBox(pGridlinesWidget);

   QLabel* pWidthLabel = new QLabel("Width:", pGridlinesWidget);
   mpWidthCombo = new LineWidthComboBox(pGridlinesWidget);

   QLabel* pColorLabel = new QLabel("Color:", pGridlinesWidget);
   mpColorButton = new CustomColorButton(pGridlinesWidget);
   mpColorButton->usePopupGrid(true);

   LabeledSection* pGridlinesSection = new LabeledSection(pGridlinesWidget, "Gridlines", this);

   QGridLayout* pGridlinesGrid = new QGridLayout(pGridlinesWidget);
   pGridlinesGrid->setMargin(0);
   pGridlinesGrid->setSpacing(5);
   pGridlinesGrid->addWidget(pStyleLabel, 0, 0);
   pGridlinesGrid->addWidget(mpStyleCombo, 0, 1);
   pGridlinesGrid->addWidget(pWidthLabel, 1, 0);
   pGridlinesGrid->addWidget(mpWidthCombo, 1, 1);
   pGridlinesGrid->addWidget(pColorLabel, 2, 0);
   pGridlinesGrid->addWidget(mpColorButton, 2, 1);
   pGridlinesGrid->setRowStretch(3, 10);
   pGridlinesGrid->setColumnStretch(2, 10);

   // Initialization
   addSection(pGridlinesSection);
   addStretch(10);
   setSizeHint(350, 150);
}

PropertiesPlotView::~PropertiesPlotView()
{
}

bool PropertiesPlotView::initialize(SessionItem* pSessionItem)
{
   mpPlotView = dynamic_cast<PlotView*>(pSessionItem);
   if (mpPlotView == NULL)
   {
      PlotWidget* pPlotWidget = dynamic_cast<PlotWidget*>(pSessionItem);
      if (pPlotWidget != NULL)
      {
         mpPlotView = pPlotWidget->getPlot();
      }

      if (mpPlotView == NULL)
      {
         return false;
      }
   }

   // Gridlines
   CartesianPlot* pCartesianPlot = dynamic_cast<CartesianPlot*>(mpPlotView);
   if (pCartesianPlot != NULL)
   {
      Gridlines* pHorizGridlines = pCartesianPlot->getGridlines(HORIZONTAL);
      Gridlines* pVertGridlines = pCartesianPlot->getGridlines(VERTICAL);

      if ((pHorizGridlines != NULL) && (pVertGridlines != NULL))
      {
         LineStyle lineStyle = pHorizGridlines->getLineStyle();
         if (pVertGridlines->getLineStyle() == lineStyle)
         {
            mpStyleCombo->setCurrentValue(lineStyle);
         }

         int lineWidth = pHorizGridlines->getLineWidth();
         if (pVertGridlines->getLineWidth() == lineWidth)
         {
            mpWidthCombo->setCurrentValue(lineWidth);
         }

         ColorType gridlineColor = pHorizGridlines->getColor();
         if (pVertGridlines->getColor() == gridlineColor)
         {
            mpColorButton->setColor(gridlineColor);
         }

         return true;
      }
   }

   PolarPlot* pPolarPlot = dynamic_cast<PolarPlot*>(mpPlotView);
   if (pPolarPlot != NULL)
   {
      Gridlines* pGridlines = pPolarPlot->getGridlines();
      if (pGridlines != NULL)
      {
         mpStyleCombo->setCurrentValue(pGridlines->getLineStyle());
         mpWidthCombo->setCurrentValue(pGridlines->getLineWidth());
         mpColorButton->setColor(pGridlines->getColor());
         return true;
      }
   }

   return false;
}

bool PropertiesPlotView::applyChanges()
{
   if (mpPlotView == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpPlotView, actionText);

   // Gridlines
   LineStyle lineStyle = mpStyleCombo->getCurrentValue();
   unsigned int lineWidth = mpWidthCombo->getCurrentValue();
   ColorType gridlineColor = mpColorButton->getColorType();

   CartesianPlot* pCartesianPlot = dynamic_cast<CartesianPlot*>(mpPlotView);
   if (pCartesianPlot != NULL)
   {
      Gridlines* pHorizGridlines = pCartesianPlot->getGridlines(HORIZONTAL);
      if (pHorizGridlines != NULL)
      {
         pHorizGridlines->setLineStyle(lineStyle);
         pHorizGridlines->setLineWidth(lineWidth);
         pHorizGridlines->setColor(gridlineColor);
      }

      Gridlines* pVertGridlines = pCartesianPlot->getGridlines(VERTICAL);
      if (pVertGridlines != NULL)
      {
         pVertGridlines->setLineStyle(lineStyle);
         pVertGridlines->setLineWidth(lineWidth);
         pVertGridlines->setColor(gridlineColor);
      }
   }

   PolarPlot* pPolarPlot = dynamic_cast<PolarPlot*>(mpPlotView);
   if (pPolarPlot != NULL)
   {
      Gridlines* pGridlines = pPolarPlot->getGridlines();
      if (pGridlines != NULL)
      {
         pGridlines->setLineStyle(lineStyle);
         pGridlines->setLineWidth(lineWidth);
         pGridlines->setColor(gridlineColor);
      }
   }

   // Refresh the view
   mpPlotView->refresh();

   return true;
}

const string& PropertiesPlotView::getName()
{
   static string name = "Plot View Properties";
   return name;
}

const string& PropertiesPlotView::getPropertiesName()
{
   static string propertiesName = "Plot View";
   return propertiesName;
}

const string& PropertiesPlotView::getDescription()
{
   static string description = "General setting properties of a plot view";
   return description;
}

const string& PropertiesPlotView::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesPlotView::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesPlotView::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesPlotView::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesPlotView::getDescriptorId()
{
   static string id = "{7767D8E5-652A-412E-A24D-A91331D33026}";
   return id;
}

bool PropertiesPlotView::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}
