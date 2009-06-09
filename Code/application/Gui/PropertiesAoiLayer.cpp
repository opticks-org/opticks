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

#include "AoiElement.h"
#include "AoiLayer.h"
#include "AppVersion.h"
#include "CustomColorButton.h"
#include "GraphicGroup.h"
#include "LabeledSection.h"
#include "PropertiesAoiLayer.h"
#include "SymbolTypeGrid.h"
#include "Undo.h"
#include "View.h"

using namespace std;

PropertiesAoiLayer::PropertiesAoiLayer() :
   LabeledSectionGroup(NULL),
   mpAoiLayer(NULL)
{
   // Pixel marker
   QWidget* pMarkerWidget = new QWidget(this);

   QLabel* pSymbolLabel = new QLabel("Symbol:", pMarkerWidget);
   mpSymbolButton = new SymbolTypeButton(pMarkerWidget);
   mpSymbolButton->setBorderedSymbols(true);

   QLabel* pColorLabel = new QLabel("Color:", pMarkerWidget);
   mpColorButton = new CustomColorButton(pMarkerWidget);
   mpColorButton->usePopupGrid(true);

   LabeledSection* pMarkerSection = new LabeledSection(pMarkerWidget, "Pixel Marker", this);

   QGridLayout* pMarkerGrid = new QGridLayout(pMarkerWidget);
   pMarkerGrid->setMargin(0);
   pMarkerGrid->setSpacing(5);
   pMarkerGrid->addWidget(pSymbolLabel, 0, 0);
   pMarkerGrid->addWidget(mpSymbolButton, 0, 1);
   pMarkerGrid->addWidget(pColorLabel, 1, 0);
   pMarkerGrid->addWidget(mpColorButton, 1, 1);
   pMarkerGrid->setRowStretch(2, 10);
   pMarkerGrid->setColumnStretch(2, 10);

   // Labels
   QWidget* pLabelWidget = new QWidget(this);
   mpShapeCheck = new QCheckBox("Shape", pLabelWidget);
   LabeledSection* pLabelSection = new LabeledSection(pLabelWidget, "Labels", this);

   QVBoxLayout* pLabelLayout = new QVBoxLayout(pLabelWidget);
   pLabelLayout->setMargin(0);
   pLabelLayout->setSpacing(5);
   pLabelLayout->addWidget(mpShapeCheck, 0, Qt::AlignLeft);

   // Statistics
   QWidget* pStatisticsWidget = new QWidget(this);
   mpPixelCount = new QLabel(pStatisticsWidget);
   mpObjectCount = new QLabel(pStatisticsWidget);
   LabeledSection* pStatisticsSection = new LabeledSection(pStatisticsWidget, "Statistics", this);
   QVBoxLayout* pStatisticsLayout = new QVBoxLayout(pStatisticsWidget);
   pStatisticsLayout->setMargin(0);
   pStatisticsLayout->setSpacing(5);
   pStatisticsLayout->addWidget(mpPixelCount);
   pStatisticsLayout->addWidget(mpObjectCount);

   // Initialization
   addSection(pMarkerSection);
   addSection(pLabelSection);
   addSection(pStatisticsSection);
   addStretch(10);
   setSizeHint(350, 200);
}

PropertiesAoiLayer::~PropertiesAoiLayer()
{
}

bool PropertiesAoiLayer::initialize(SessionItem* pSessionItem)
{
   mpAoiLayer = dynamic_cast<AoiLayer*>(pSessionItem);
   if (mpAoiLayer == NULL)
   {
      return false;
   }

   // Pixel marker
   mpSymbolButton->setCurrentValue(mpAoiLayer->getSymbol());
   mpColorButton->setColor(mpAoiLayer->getColor());

   // Labels
   mpShapeCheck->setChecked(mpAoiLayer->getShowLabels());

   // Statistics
   size_t pixelCount = 0;
   size_t objectCount = 0;
   AoiElement* pElement = dynamic_cast<AoiElement*>(mpAoiLayer->getDataElement());
   GraphicGroup* pGroup = NULL;
   if (pElement != NULL)
   {
      pGroup = pElement->getGroup();
      pixelCount = pElement->getPixelCount();
   }
   if (pGroup != NULL)
   {
      objectCount = pGroup->getObjects().size();
   }
   mpPixelCount->setText(QString("%1 pixel%2 selected").arg(pixelCount).arg((pixelCount == 1) ? "" : "s"));
   mpObjectCount->setText(QString("%1 object%2").arg(objectCount).arg((objectCount == 1) ? "" : "s"));

   return true;
}

bool PropertiesAoiLayer::applyChanges()
{
   if (mpAoiLayer == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpAoiLayer->getView(), actionText);

   // Pixel marker
   mpAoiLayer->setSymbol(mpSymbolButton->getCurrentValue());
   mpAoiLayer->setColor(QCOLOR_TO_COLORTYPE(mpColorButton->getColor()));

   // Labels
   mpAoiLayer->setShowLabels(mpShapeCheck->isChecked());

   // Refresh the view
   View* pView = mpAoiLayer->getView();
   if (pView != NULL)
   {
      pView->refresh();
   }

   return true;
}

const string& PropertiesAoiLayer::getName()
{
   static string name = "AOI Layer Properties";
   return name;
}

const string& PropertiesAoiLayer::getPropertiesName()
{
   static string propertiesName = "AOI Layer";
   return propertiesName;
}

const string& PropertiesAoiLayer::getDescription()
{
   static string description = "General setting properties of an AOI layer";
   return description;
}

const string& PropertiesAoiLayer::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesAoiLayer::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesAoiLayer::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesAoiLayer::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesAoiLayer::getDescriptorId()
{
   static string id = "{B0CF208F-19AE-469E-B85A-02FB190DCC12}";
   return id;
}

bool PropertiesAoiLayer::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}
