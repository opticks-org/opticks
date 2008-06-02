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
#include "PropertiesView.h"
#include "Undo.h"
#include "View.h"

using namespace std;

PropertiesView::PropertiesView() :
   LabeledSectionGroup(NULL),
   mpView(NULL)
{
   // General
   QWidget* pGeneralWidget = new QWidget(this);

   QLabel* pColorLabel = new QLabel("Background Color:", pGeneralWidget);
   mpColorButton = new CustomColorButton(pGeneralWidget);
   mpColorButton->usePopupGrid(true);
   QLabel* pOriginLabel = new QLabel("Origin Location:", pGeneralWidget);
   mpOriginCombo = new QComboBox(pGeneralWidget);
   mpOriginCombo->setEditable(false);
   mpOriginCombo->addItem("Lower Left");
   mpOriginCombo->addItem("Upper Left");
   mpCrosshairCheck = new QCheckBox("Crosshair", pGeneralWidget);

   LabeledSection* pGeneralSection = new LabeledSection(pGeneralWidget, "General", this);

   QGridLayout* pGeneralGrid = new QGridLayout(pGeneralWidget);
   pGeneralGrid->setMargin(0);
   pGeneralGrid->setSpacing(5);
   pGeneralGrid->addWidget(pColorLabel, 0, 0);
   pGeneralGrid->addWidget(mpColorButton, 0, 1);
   pGeneralGrid->addWidget(pOriginLabel, 1, 0);
   pGeneralGrid->addWidget(mpOriginCombo, 1, 1);
   pGeneralGrid->addWidget(mpCrosshairCheck, 2, 0, 1, 2);
   pGeneralGrid->setRowStretch(3, 10);
   pGeneralGrid->setColumnStretch(2, 10);

   // Initialization
   addSection(pGeneralSection);
   addStretch(10);
   setSizeHint(350, 150);
}

PropertiesView::~PropertiesView()
{
}

bool PropertiesView::initialize(SessionItem* pSessionItem)
{
   mpView = dynamic_cast<View*>(pSessionItem);
   if (mpView == NULL)
   {
      return false;
   }

   // General
   mpColorButton->setColor(mpView->getBackgroundColor());
   mpCrosshairCheck->setChecked(mpView->isCrossHairEnabled());

   DataOrigin origin = mpView->getDataOrigin();
   if (origin == LOWER_LEFT)
   {
      mpOriginCombo->setCurrentIndex(0);
   }
   else if (origin == UPPER_LEFT)
   {
      mpOriginCombo->setCurrentIndex(1);
   }

   return true;
}

bool PropertiesView::applyChanges()
{
   if (mpView == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpView, actionText);

   // General
   DataOrigin origin;
   if (mpOriginCombo->currentIndex() == 0)
   {
      origin = LOWER_LEFT;
   }
   else if (mpOriginCombo->currentIndex() == 1)
   {
      origin = UPPER_LEFT;
   }

   mpView->setBackgroundColor(QCOLOR_TO_COLORTYPE(mpColorButton->getColor()));
   mpView->setDataOrigin(origin);
   mpView->enableCrossHair(mpCrosshairCheck->isChecked());

   // Refresh the view
   mpView->refresh();

   return true;
}

const string& PropertiesView::getName()
{
   static string name = "View Properties";
   return name;
}

const string& PropertiesView::getPropertiesName()
{
   static string propertiesName = "View";
   return propertiesName;
}

const string& PropertiesView::getDescription()
{
   static string description = "General setting properties of a view";
   return description;
}

const string& PropertiesView::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesView::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesView::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesView::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesView::getDescriptorId()
{
   static string id = "{A86EC4AA-9E56-441C-A6B7-819A8F9237D9}";
   return id;
}

bool PropertiesView::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}
