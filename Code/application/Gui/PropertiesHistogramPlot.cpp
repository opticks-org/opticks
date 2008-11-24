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
#include "CustomColorButton.h"
#include "LabeledSection.h"
#include "HistogramPlot.h"
#include "PlotWidget.h"
#include "PropertiesHistogramPlot.h"
#include "Undo.h"

using namespace std;

PropertiesHistogramPlot::PropertiesHistogramPlot() :
   LabeledSectionGroup(NULL),
   mpHistogramPlot(NULL)
{
   // Histogram
   QWidget* pHistogramWidget = new QWidget(this);

   QLabel* pColorLabel = new QLabel("Color:", pHistogramWidget);
   mpColorButton = new CustomColorButton(pHistogramWidget);
   mpColorButton->usePopupGrid(true);

   LabeledSection* pHistogramSection = new LabeledSection(pHistogramWidget, "Histogram", this);

   QGridLayout* pHistogramGrid = new QGridLayout(pHistogramWidget);
   pHistogramGrid->setMargin(0);
   pHistogramGrid->setSpacing(5);
   pHistogramGrid->addWidget(pColorLabel, 0, 0);
   pHistogramGrid->addWidget(mpColorButton, 0, 1, Qt::AlignLeft);
   pHistogramGrid->setRowStretch(1, 10);
   pHistogramGrid->setColumnStretch(1, 10);

   // Initialization
   addSection(pHistogramSection);
   addStretch(10);
   setSizeHint(325, 125);
}

PropertiesHistogramPlot::~PropertiesHistogramPlot()
{
}

bool PropertiesHistogramPlot::initialize(SessionItem* pSessionItem)
{
   mpHistogramPlot = dynamic_cast<HistogramPlot*>(pSessionItem);
   if (mpHistogramPlot == NULL)
   {
      PlotWidget* pPlotWidget = dynamic_cast<PlotWidget*>(pSessionItem);
      if (pPlotWidget != NULL)
      {
         mpHistogramPlot = dynamic_cast<HistogramPlot*>(pPlotWidget->getPlot());
      }

      if (mpHistogramPlot == NULL)
      {
         return false;
      }
   }

   // Histogram
   mpColorButton->setColor(mpHistogramPlot->getHistogramColor());

   return true;
}

bool PropertiesHistogramPlot::applyChanges()
{
   if (mpHistogramPlot == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpHistogramPlot, actionText);

   // Histogram
   mpHistogramPlot->setHistogramColor(QCOLOR_TO_COLORTYPE(mpColorButton->getColor()));

   // Refresh the plot
   mpHistogramPlot->refresh();

   return true;
}

const string& PropertiesHistogramPlot::getName()
{
   static string name = "Histogram Plot Properties";
   return name;
}

const string& PropertiesHistogramPlot::getPropertiesName()
{
   static string propertiesName = "Histogram Plot";
   return propertiesName;
}

const string& PropertiesHistogramPlot::getDescription()
{
   static string description = "General setting properties of a histogram plot";
   return description;
}

const string& PropertiesHistogramPlot::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesHistogramPlot::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesHistogramPlot::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesHistogramPlot::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesHistogramPlot::getDescriptorId()
{
   static string id = "{8740D843-22EF-4FD3-8080-E43F3B26B769}";
   return id;
}

bool PropertiesHistogramPlot::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}
