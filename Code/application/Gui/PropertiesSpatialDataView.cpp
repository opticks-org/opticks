/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "CustomColorButton.h"
#include "DesktopServices.h"
#include "LabeledSection.h"
#include "LineWidthComboBox.h"
#include "PanLimitTypeComboBox.h"
#include "PropertiesSpatialDataView.h"
#include "SpatialDataViewImp.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "TypesFile.h"
#include "Undo.h"
#include "Window.h"

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QSpinBox>
#include <QtGui/QStyleOptionButton>

#include <limits>
#include <vector>
using namespace std;

PropertiesSpatialDataView::PropertiesSpatialDataView() :
   LabeledSectionGroup(NULL),
   mpView(NULL),
   mpClassificationPosition(NULL)
{
   // Pan and zoom
   QWidget* pPanZoomWidget = new QWidget(this);

   QLabel* pPanLimitLabel = new QLabel("Pan Limit:", pPanZoomWidget);
   mpPanLimitCombo = new PanLimitTypeComboBox(pPanZoomWidget);

   QLabel* pMinZoomLabel = new QLabel("When Zooming In:", pPanZoomWidget);
   pMinZoomLabel->setToolTip("Show at least this many data pixels");
   mpMinZoomSpin = new QDoubleSpinBox(pPanZoomWidget);
   mpMinZoomSpin->setSuffix(" Pixel(s)");
   mpMinZoomSpin->setRange(0.0, numeric_limits<double>::max());
   mpMinZoomSpin->setToolTip("Show at least this many data pixels");

   QLabel* pMaxZoomLabel = new QLabel("When Zooming Out:", pPanZoomWidget);
   pMaxZoomLabel->setToolTip("The data should occupy at least this much percentage of the window");
   mpMaxZoomSpin = new QSpinBox(pPanZoomWidget);
   mpMaxZoomSpin->setSuffix("%");
   mpMaxZoomSpin->setRange(0, 100);
   mpMaxZoomSpin->setToolTip("The data should occupy at least this much percentage of the window");

   LabeledSection* pPanZoomSection = new LabeledSection(pPanZoomWidget, "Pan and Zoom", this);

   QGridLayout* pPanZoomGrid = new QGridLayout(pPanZoomWidget);
   pPanZoomGrid->setMargin(0);
   pPanZoomGrid->setSpacing(5);
   pPanZoomGrid->addWidget(pPanLimitLabel, 0, 0);
   pPanZoomGrid->addWidget(mpPanLimitCombo, 0, 1, Qt::AlignLeft);
   pPanZoomGrid->addWidget(pMinZoomLabel, 1, 0);
   pPanZoomGrid->addWidget(mpMinZoomSpin, 1, 1, Qt::AlignLeft);
   pPanZoomGrid->addWidget(pMaxZoomLabel, 2, 0);
   pPanZoomGrid->addWidget(mpMaxZoomSpin, 2, 1, Qt::AlignLeft);
   pPanZoomGrid->setRowStretch(3, 10);
   pPanZoomGrid->setColumnStretch(1, 10);

   // Image
   QWidget* pImageWidget = new QWidget(this);

   mpSmoothCheck = new QCheckBox("Smooth pixel edges", pImageWidget);
   mpOriginCheck = new QCheckBox("Display origin location", pImageWidget);
   mpAxisCheck = new QCheckBox("Display orientation axis", pImageWidget);
   mpCrosshairCheck = new QCheckBox("Crosshair", pImageWidget);
   mpCrosshairColorLabel = new QLabel("Color:", this);
   mpCrosshairColorButton = new CustomColorButton(this);
   mpCrosshairColorButton->usePopupGrid(true);
   mpCrosshairBlendCheck = new QCheckBox("Blend", this);
   mpCrosshairBlendCheck->setToolTip("If enabled, the crosshair color is combined with the color in "
      "the view\njust behind the crosshair to provide enough contrast for the crosshair\nto always "
      "be visible.  If disabled, the crosshair is drawn as a solid color.");
   mpCrosshairSizeLabel = new QLabel("Size:", this);
   mpCrosshairSizeSpin = new QSpinBox(this);
   mpCrosshairSizeSpin->setSuffix(" Pixel(s)");
   mpCrosshairSizeSpin->setRange(1, numeric_limits<int>::max());
   mpCrosshairWidthLabel = new QLabel("Width:", this);
   mpCrosshairWidthCombo = new LineWidthComboBox(this);

   LabeledSection* pImageSection = new LabeledSection(pImageWidget, "Image", this);

   QGridLayout* pImageLayout = new QGridLayout(pImageWidget);
   pImageLayout->setMargin(0);
   pImageLayout->setSpacing(5);
   pImageLayout->addWidget(mpSmoothCheck, 0, 0, 1, 4);
   pImageLayout->addWidget(mpOriginCheck, 1, 0, 1, 4);
   pImageLayout->addWidget(mpAxisCheck, 2, 0, 1, 4);
   pImageLayout->addWidget(mpCrosshairCheck, 3, 0, 1, 4);
   pImageLayout->addWidget(mpCrosshairColorLabel, 4, 1);
   pImageLayout->addWidget(mpCrosshairColorButton, 4, 2);
   pImageLayout->addWidget(mpCrosshairBlendCheck, 4, 3, Qt::AlignLeft);
   pImageLayout->addWidget(mpCrosshairSizeLabel, 5, 1);
   pImageLayout->addWidget(mpCrosshairSizeSpin, 5, 2, 1, 2, Qt::AlignLeft);
   pImageLayout->addWidget(mpCrosshairWidthLabel, 6, 1);
   pImageLayout->addWidget(mpCrosshairWidthCombo, 6, 2, 1, 2, Qt::AlignLeft);
   QStyleOptionButton option;
   option.initFrom(mpCrosshairCheck);
   int checkBoxWidth = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option).width();
   pImageLayout->setColumnMinimumWidth(0, checkBoxWidth);
   pImageLayout->setColumnStretch(3, 10);

   // Classification markings
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
   addSection(pPanZoomSection);
   addSection(pImageSection);
   if (pMarkingsSection != NULL)
   {
      addSection(pMarkingsSection);
   }
   addStretch(10);

   // Connections
   VERIFYNR(connect(mpCrosshairCheck, SIGNAL(toggled(bool)), mpCrosshairColorLabel, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpCrosshairCheck, SIGNAL(toggled(bool)), mpCrosshairColorButton, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpCrosshairCheck, SIGNAL(toggled(bool)), mpCrosshairBlendCheck, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpCrosshairCheck, SIGNAL(toggled(bool)), mpCrosshairSizeLabel, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpCrosshairCheck, SIGNAL(toggled(bool)), mpCrosshairSizeSpin, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpCrosshairCheck, SIGNAL(toggled(bool)), mpCrosshairWidthLabel, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpCrosshairCheck, SIGNAL(toggled(bool)), mpCrosshairWidthCombo, SLOT(setEnabled(bool))));
}

PropertiesSpatialDataView::~PropertiesSpatialDataView()
{}

bool PropertiesSpatialDataView::initialize(SessionItem* pSessionItem)
{
   mpView = dynamic_cast<SpatialDataView*>(pSessionItem);
   if (mpView == NULL)
   {
      return false;
   }

   // Pan and zoom
   mpPanLimitCombo->setCurrentValue(mpView->getPanLimit());
   mpMinZoomSpin->setValue(mpView->getMinimumZoom());
   mpMaxZoomSpin->setValue(mpView->getMaximumZoom() * 100);

   // Image
   mpSmoothCheck->setChecked(mpView->getTextureMode() == TEXTURE_LINEAR);
   mpOriginCheck->setChecked(mpView->isOriginDisplayed());
   mpAxisCheck->setChecked(mpView->isAxisDisplayed());
   mpCrosshairCheck->setChecked(mpView->isCrossHairEnabled());
   mpCrosshairColorLabel->setEnabled(mpView->isCrossHairEnabled());
   mpCrosshairColorButton->setColor(mpView->getCrossHairColor());
   mpCrosshairColorButton->setEnabled(mpView->isCrossHairEnabled());
   mpCrosshairBlendCheck->setChecked(mpView->isCrossHairBlended());
   mpCrosshairBlendCheck->setEnabled(mpView->isCrossHairEnabled());
   mpCrosshairSizeLabel->setEnabled(mpView->isCrossHairEnabled());
   mpCrosshairSizeSpin->setValue(mpView->getCrossHairSize());
   mpCrosshairSizeSpin->setEnabled(mpView->isCrossHairEnabled());
   mpCrosshairWidthLabel->setEnabled(mpView->isCrossHairEnabled());
   mpCrosshairWidthCombo->setCurrentValue(mpView->getCrossHairWidth());
   mpCrosshairWidthCombo->setEnabled(mpView->isCrossHairEnabled());

   SpatialDataViewImp* pViewImp = dynamic_cast<SpatialDataViewImp*>(mpView);
   if (pViewImp != NULL)
   {
      mpSmoothCheck->setEnabled(pViewImp->isSmoothingAvailable());
   }

   // Classification markings
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

bool PropertiesSpatialDataView::applyChanges()
{
   if (mpView == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpView, actionText);

   // Pan and zoom
   std::vector<Window*> pWindows;
   Service<DesktopServices> pDesktop;
   pDesktop->getWindows(pWindows); 
   bool bLinked = false;

   for (unsigned int i = 0; i < pWindows.size(); ++i)
   {
      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pWindows[i]);
      if (pWindow != NULL)
      {
         View* pView = pWindow->getView();
         if (pView != NULL)
         {
            if (pView->getViewLinkType(mpView) != NO_LINK &&
               (mpView->getPanLimit() != mpPanLimitCombo->getCurrentValue() || 
               mpView->getMinimumZoom() != mpMinZoomSpin->value() ||
               mpView->getMaximumZoom() != (mpMaxZoomSpin->value() / 100.0)))
            {
               bLinked = true;
               mpPanLimitCombo->setCurrentValue(NO_LIMIT);
               mpMinZoomSpin->setValue(0);
               mpMaxZoomSpin->setValue(0);
               QMessageBox::information(pDesktop->getMainWidget(), QString::fromStdString(mpView->getDisplayName()), 
                     "The pan/zoom limits can't be changed if this view is linked to other views.");
               break;
            }
         }
      }
   }

   if (!bLinked)
   {
      mpView->setPanLimit(mpPanLimitCombo->getCurrentValue());
      mpView->setMinimumZoom(mpMinZoomSpin->value());
      mpView->setMaximumZoom(mpMaxZoomSpin->value() / 100.0);
   }

   // Image
   TextureMode textureMode;
   if (mpSmoothCheck->isChecked() == true)
   {
      textureMode = TEXTURE_LINEAR;
   }
   else
   {
      textureMode = TEXTURE_NEAREST_NEIGHBOR;
   }

   mpView->setTextureMode(textureMode);
   mpView->displayOrigin(mpOriginCheck->isChecked());
   mpView->displayAxis(mpAxisCheck->isChecked());
   mpView->enableCrossHair(mpCrosshairCheck->isChecked());

   if (mpCrosshairCheck->isChecked() == true)
   {
      mpView->setCrossHairColor(QCOLOR_TO_COLORTYPE(mpCrosshairColorButton->getColor()));
      mpView->setCrossHairBlended(mpCrosshairBlendCheck->isChecked());
      mpView->setCrossHairSize(mpCrosshairSizeSpin->value());
      mpView->setCrossHairWidth(mpCrosshairWidthCombo->getCurrentValue());
   }

   // Classification markings
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

const string& PropertiesSpatialDataView::getName()
{
   static string name = "Spatial Data View Properties";
   return name;
}

const string& PropertiesSpatialDataView::getPropertiesName()
{
   static string propertiesName = "Spatial Data View";
   return propertiesName;
}

const string& PropertiesSpatialDataView::getDescription()
{
   static string description = "General setting properties of a spatial data view";
   return description;
}

const string& PropertiesSpatialDataView::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesSpatialDataView::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesSpatialDataView::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesSpatialDataView::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesSpatialDataView::getDescriptorId()
{
   static string id = "{23F4A65B-C380-41D0-8F08-0CC968F26042}";
   return id;
}

bool PropertiesSpatialDataView::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}
