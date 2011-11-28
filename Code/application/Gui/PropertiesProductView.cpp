/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "CustomColorButton.h"
#include "LabeledSection.h"
#include "ProductView.h"
#include "PropertiesProductView.h"
#include "StringUtilities.h"
#include "TypesFile.h"
#include "Undo.h"

#include <vector>
using namespace std;

PropertiesProductView::PropertiesProductView() :
   LabeledSectionGroup(NULL),
   mpView(NULL),
   mpClassificationPosition(NULL)
{
   // Paper
   QWidget* pPaperWidget = new QWidget(this);

   QLabel* pColorLabel = new QLabel("Color:", pPaperWidget);
   mpColorButton = new CustomColorButton(pPaperWidget);
   mpColorButton->usePopupGrid(true);

   LabeledSection* pPaperSection = new LabeledSection(pPaperWidget, "Paper", this);

   QGridLayout* pPaperGrid = new QGridLayout(pPaperWidget);
   pPaperGrid->setMargin(0);
   pPaperGrid->setSpacing(5);
   pPaperGrid->addWidget(pColorLabel, 0, 0);
   pPaperGrid->addWidget(mpColorButton, 0, 1, Qt::AlignLeft);
   pPaperGrid->setRowStretch(1, 10);
   pPaperGrid->setColumnStretch(1, 10);

   // classification markings
   LabeledSection* pMarkingsSection(NULL);
   if (ConfigurationSettings::getSettingDisplayClassificationMarkings())
   {
      QWidget* pMarkingsWidget = new QWidget(this);
      pMarkingsSection = new LabeledSection(pMarkingsWidget, "Classification Markings", this);
      mpClassificationPosition = new QComboBox(pMarkingsWidget);
      QLabel* pMarkingsLabel = new QLabel("Position:", pMarkingsWidget);
      QHBoxLayout* pMarkingsLayout = new QHBoxLayout(pMarkingsWidget);
      pMarkingsLayout->setMargin(0);
      pMarkingsLayout->setSpacing(5);
      pMarkingsLayout->addWidget(pMarkingsLabel);
      pMarkingsLayout->addWidget(mpClassificationPosition);
      pMarkingsLayout->addStretch();
      vector<string> positions = StringUtilities::getAllEnumValuesAsDisplayString<PositionType>();
      for (vector<string>::iterator it = positions.begin(); it != positions.end(); ++it)
      {
         mpClassificationPosition->addItem(QString::fromStdString(*it));
      }
   }

   // Initialization
   addSection(pPaperSection);
   if (pMarkingsSection != NULL)
   {
      addSection(pMarkingsSection);
   }
   addStretch(10);
}

PropertiesProductView::~PropertiesProductView()
{
}

bool PropertiesProductView::initialize(SessionItem* pSessionItem)
{
   mpView = dynamic_cast<ProductView*>(pSessionItem);
   if (mpView == NULL)
   {
      return false;
   }

   // Paper
   mpColorButton->setColor(mpView->getPaperColor());

   // classification markings
   if (mpClassificationPosition != NULL)
   {
      string positionStr = StringUtilities::toDisplayString<PositionType>(mpView->getClassificationPosition());
      int index = mpClassificationPosition->findText(QString::fromStdString(positionStr));
      if (index != -1)
      {
         mpClassificationPosition->setCurrentIndex(index);
      }
   }

   return true;
}

bool PropertiesProductView::applyChanges()
{
   if (mpView == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpView, actionText);

   // Paper
   mpView->setPaperColor(QCOLOR_TO_COLORTYPE(mpColorButton->getColor()));

   // classification markings
   if (mpClassificationPosition != NULL)
   {
      PositionType ePosition = StringUtilities::fromDisplayString<PositionType>(
         mpClassificationPosition->currentText().toStdString());
      mpView->setClassificationPosition(ePosition);
   }

   // Refresh the view
   mpView->refresh();

   return true;
}

const string& PropertiesProductView::getName()
{
   static string name = "Product View Properties";
   return name;
}

const string& PropertiesProductView::getPropertiesName()
{
   static string propertiesName = "Product View";
   return propertiesName;
}

const string& PropertiesProductView::getDescription()
{
   static string description = "General setting properties of a product view";
   return description;
}

const string& PropertiesProductView::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesProductView::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesProductView::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesProductView::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesProductView::getDescriptorId()
{
   static string id = "{361EF583-2F2F-477E-AD94-26900E693217}";
   return id;
}

bool PropertiesProductView::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}
