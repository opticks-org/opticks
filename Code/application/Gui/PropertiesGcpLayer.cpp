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
#include "GcpLayer.h"
#include "GcpSymbolGrid.h"
#include "PropertiesGcpLayer.h"
#include "Undo.h"
#include "View.h"

using namespace std;

PropertiesGcpLayer::PropertiesGcpLayer() :
   LabeledSectionGroup(NULL),
   mpGcpLayer(NULL)
{
   // Pixel marker
   QWidget* pMarkerWidget = new QWidget(this);

   QLabel* pSymbolLabel = new QLabel("Symbol:", pMarkerWidget);
   mpSymbolButton = new GcpSymbolButton(pMarkerWidget);

   QLabel* pSizeLabel = new QLabel("Symbol Size:", pMarkerWidget);
   mpSizeSpin = new QSpinBox(pMarkerWidget);
   mpSizeSpin->setMinimum(1);

   QLabel* pColorLabel = new QLabel("Color:", pMarkerWidget);
   mpColorButton = new CustomColorButton(pMarkerWidget);
   mpColorButton->usePopupGrid(true);

   LabeledSection* pMarkerSection = new LabeledSection(pMarkerWidget, "Pixel Marker", this);

   QGridLayout* pMarkerGrid = new QGridLayout(pMarkerWidget);
   pMarkerGrid->setMargin(0);
   pMarkerGrid->setSpacing(5);
   pMarkerGrid->addWidget(pSymbolLabel, 0, 0);
   pMarkerGrid->addWidget(mpSymbolButton, 0, 1);
   pMarkerGrid->addWidget(pSizeLabel, 1, 0);
   pMarkerGrid->addWidget(mpSizeSpin, 1, 1);
   pMarkerGrid->addWidget(pColorLabel, 2, 0);
   pMarkerGrid->addWidget(mpColorButton, 2, 1);
   pMarkerGrid->setRowStretch(3, 10);
   pMarkerGrid->setColumnStretch(2, 10);

   // Initialization
   addSection(pMarkerSection);
   addStretch(10);
   setSizeHint(325, 125);
}

PropertiesGcpLayer::~PropertiesGcpLayer()
{
}

bool PropertiesGcpLayer::initialize(SessionItem* pSessionItem)
{
   mpGcpLayer = dynamic_cast<GcpLayer*>(pSessionItem);
   if (mpGcpLayer == NULL)
   {
      return false;
   }

   // Pixel marker
   mpSymbolButton->setCurrentValue(mpGcpLayer->getSymbol());
   mpSizeSpin->setValue(mpGcpLayer->getSymbolSize());
   mpColorButton->setColor(mpGcpLayer->getColor());

   return true;
}

bool PropertiesGcpLayer::applyChanges()
{
   if (mpGcpLayer == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpGcpLayer->getView(), actionText);

   // Pixel marker
   mpGcpLayer->setSymbol(mpSymbolButton->getCurrentValue());
   mpGcpLayer->setSymbolSize(mpSizeSpin->value());
   mpGcpLayer->setColor(QCOLOR_TO_COLORTYPE(mpColorButton->getColor()));

   // Refresh the view
   View* pView = mpGcpLayer->getView();
   if (pView != NULL)
   {
      pView->refresh();
   }

   return true;
}

const string& PropertiesGcpLayer::getName()
{
   static string name = "GCP Layer Properties";
   return name;
}

const string& PropertiesGcpLayer::getPropertiesName()
{
   static string propertiesName = "GCP Layer";
   return propertiesName;
}

const string& PropertiesGcpLayer::getDescription()
{
   static string description = "General setting properties of a GCP layer";
   return description;
}

const string& PropertiesGcpLayer::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesGcpLayer::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesGcpLayer::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesGcpLayer::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesGcpLayer::getDescriptorId()
{
   static string id = "{790210CB-FA8E-4661-BD76-9A50DE0AAF02}";
   return id;
}

bool PropertiesGcpLayer::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}
