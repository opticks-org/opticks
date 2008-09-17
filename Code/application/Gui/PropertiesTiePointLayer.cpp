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
#include "PropertiesTiePointLayer.h"
#include "TiePointLayer.h"
#include "Undo.h"
#include "View.h"

using namespace std;

PropertiesTiePointLayer::PropertiesTiePointLayer() :
   LabeledSectionGroup(NULL),
   mpTiePointLayer(NULL)
{
   // Pixel marker
   QWidget* pMarkerWidget = new QWidget(this);

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
   pMarkerGrid->addWidget(pSizeLabel, 0, 0);
   pMarkerGrid->addWidget(mpSizeSpin, 0, 1);
   pMarkerGrid->addWidget(pColorLabel, 1, 0);
   pMarkerGrid->addWidget(mpColorButton, 1, 1);
   pMarkerGrid->setRowStretch(2, 10);
   pMarkerGrid->setColumnStretch(2, 10);

   // Labels
   QWidget* pLabelsWidget = new QWidget(this);
   mpLabelsCheck = new QCheckBox("Display Labels", pLabelsWidget);
   LabeledSection* pLabelsSection = new LabeledSection(pLabelsWidget, "Labels", this);

   QVBoxLayout* pLabelsLayout = new QVBoxLayout(pLabelsWidget);
   pLabelsLayout->setMargin(0);
   pLabelsLayout->setSpacing(5);
   pLabelsLayout->addWidget(mpLabelsCheck, 0, Qt::AlignLeft);

   // Initialization
   addSection(pMarkerSection);
   addSection(pLabelsSection);
   addStretch(10);
   setSizeHint(350, 175);
}

PropertiesTiePointLayer::~PropertiesTiePointLayer()
{
}

bool PropertiesTiePointLayer::initialize(SessionItem* pSessionItem)
{
   mpTiePointLayer = dynamic_cast<TiePointLayer*>(pSessionItem);
   if (mpTiePointLayer == NULL)
   {
      return false;
   }

   // Pixel marker
   mpSizeSpin->setValue(mpTiePointLayer->getSymbolSize());
   mpColorButton->setColor(mpTiePointLayer->getColor());

   // Labels
   mpLabelsCheck->setChecked(mpTiePointLayer->areLabelsEnabled());

   return true;
}

bool PropertiesTiePointLayer::applyChanges()
{
   if (mpTiePointLayer == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpTiePointLayer->getView(), actionText);

   // Pixel marker
   mpTiePointLayer->setSymbolSize(mpSizeSpin->value());
   mpTiePointLayer->setColor(QCOLOR_TO_COLORTYPE(mpColorButton->getColor()));

   // Labels
   mpTiePointLayer->enableLabels(mpLabelsCheck->isChecked());

   // Refresh the view
   View* pView = mpTiePointLayer->getView();
   if (pView != NULL)
   {
      pView->refresh();
   }

   return true;
}

const string& PropertiesTiePointLayer::getName()
{
   static string name = "Tie Point Layer Properties";
   return name;
}

const string& PropertiesTiePointLayer::getPropertiesName()
{
   static string propertiesName = "Tie Point Layer";
   return propertiesName;
}

const string& PropertiesTiePointLayer::getDescription()
{
   static string description = "General setting properties of a tie point layer";
   return description;
}

const string& PropertiesTiePointLayer::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesTiePointLayer::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesTiePointLayer::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesTiePointLayer::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesTiePointLayer::getDescriptorId()
{
   static string id = "{F69C2D6D-A843-4C93-B144-754842C51AC7}";
   return id;
}

bool PropertiesTiePointLayer::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}
