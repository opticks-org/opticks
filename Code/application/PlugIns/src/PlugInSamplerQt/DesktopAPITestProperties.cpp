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
#include "PlotView.h"
#include "PlotWidget.h"
#include "PointSet.h"
#include "DesktopAPITestProperties.h"

#include <list>
using namespace std;

DesktopAPITestProperties::DesktopAPITestProperties() :
   LabeledSectionGroup(NULL),
   mpPlotView(NULL)
{
   // Signatures
   QWidget* pSignatureWidget = new QWidget(this);

   QLabel* pSignatureColorLabel = new QLabel("Color:", pSignatureWidget);
   mpSignatureColorButton = new CustomColorButton(pSignatureWidget);
   mpSignatureColorButton->usePopupGrid(true);

   LabeledSection* pSignatureSection = new LabeledSection(pSignatureWidget, "Display", this);

   // Layout
   QGridLayout* pSignatureGrid = new QGridLayout(pSignatureWidget);
   pSignatureGrid->setMargin(0);
   pSignatureGrid->setSpacing(5);
   pSignatureGrid->addWidget(pSignatureColorLabel, 0, 0);
   pSignatureGrid->addWidget(mpSignatureColorButton, 0, 1, Qt::AlignLeft);
   pSignatureGrid->setRowStretch(1, 10);
   pSignatureGrid->setColumnStretch(1, 10);

   // Initialization
   addSection(pSignatureSection);
   addStretch(10);
}

DesktopAPITestProperties::~DesktopAPITestProperties()
{
}

bool DesktopAPITestProperties::initialize(SessionItem* pSessionItem)
{
   mpPlotView = dynamic_cast<PlotView*>(pSessionItem);
   if (mpPlotView == NULL)
   {
      PlotWidget* pPlotWidget = dynamic_cast<PlotWidget*>(pSessionItem);
      if (pPlotWidget != NULL)
      {
         mpPlotView = pPlotWidget->getPlot();
      }
   }

   if (mpPlotView == NULL)
   {
      return false;
   }

   // Signatures
   ColorType signatureColor(0, 0, 0);

   list<PlotObject*> objects;
   mpPlotView->getObjects(POINT_SET, objects);
   if (objects.empty() == false)
   {
      PointSet* pPointSet = static_cast<PointSet*>(objects.front());
      if (pPointSet != NULL)
      {
         signatureColor = pPointSet->getLineColor();
      }
   }

   mpSignatureColorButton->setColor(signatureColor);
   return true;
}

bool DesktopAPITestProperties::applyChanges()
{
   if (mpPlotView == NULL)
   {
      return false;
   }

   // Signatures
   QColor signatureColor = mpSignatureColorButton->getColor();

   list<PlotObject*> objects;
   mpPlotView->getObjects(POINT_SET, objects);
   for (list<PlotObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      PointSet* pPointSet = static_cast<PointSet*>(*iter);
      if (pPointSet != NULL)
      {
         pPointSet->setLineColor(QCOLOR_TO_COLORTYPE(signatureColor));
      }
   }

   // Refresh the plot
   mpPlotView->refresh();

   return true;
}

const string& DesktopAPITestProperties::getName()
{
   static string name = "Desktop API Test Properties";
   return name;
}

const string& DesktopAPITestProperties::getPropertiesName()
{
   static string propertiesName = "Signatures";
   return propertiesName;
}

const string& DesktopAPITestProperties::getDescription()
{
   static string description = "General properties for the signatures in the Desktop API Test plot window";
   return description;
}

const string& DesktopAPITestProperties::getShortDescription()
{
   static string description;
   return description;
}

const string& DesktopAPITestProperties::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& DesktopAPITestProperties::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& DesktopAPITestProperties::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& DesktopAPITestProperties::getDescriptorId()
{
   static string id = "{A2EE36A7-1721-43EB-B4E4-74668F00ED54}";
   return id;
}

bool DesktopAPITestProperties::isProduction()
{
   return false;
}
