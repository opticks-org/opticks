/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "CustomColorButton.h"
#include "LabeledSection.h"
#include "OptionsView.h"
#include "PerspectiveView.h"
#include "ResolutionWidget.h"
#include "TypesFile.h"
#include "View.h"
#include "WorkspaceWindow.h"

#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <limits>
#include <string>

using namespace std;

OptionsView::OptionsView() :
   QWidget(NULL)
{
   // Initial Size
   mpFixedSizeRadio = new QRadioButton("Fixed Size", this);
   mpMaximizedRadio = new QRadioButton("Maximized", this);
   mpPercentageRadio = new QRadioButton("Percentage:", this);
   QButtonGroup* pInitialSizeGroup = new QButtonGroup(this);
   pInitialSizeGroup->addButton(mpFixedSizeRadio);
   pInitialSizeGroup->addButton(mpMaximizedRadio);
   pInitialSizeGroup->addButton(mpPercentageRadio);

   QLabel* pWidthLabel = new QLabel("Width:", this);
   mpWidthSpin = new QSpinBox(this);
   mpWidthSpin->setRange(1, numeric_limits<int>::max());
   mpWidthSpin->setSuffix(" Pixel(s)");

   QLabel* pHeightLabel = new QLabel("Height:", this);
   mpHeightSpin = new QSpinBox(this);
   mpHeightSpin->setRange(1, numeric_limits<int>::max());
   mpHeightSpin->setSuffix(" Pixel(s)");

   mpPercentageSpin = new QSpinBox(this);
   mpPercentageSpin->setRange(0, 100);
   mpPercentageSpin->setSuffix("%");

   QHBoxLayout* pPercentageLayout = new QHBoxLayout();
   pPercentageLayout->addWidget(mpPercentageRadio);
   pPercentageLayout->addWidget(mpPercentageSpin);
   pPercentageLayout->addStretch(10);

   QWidget* pInitialSizeWidget = new QWidget(this);
   QGridLayout* pInitialSizeLayout = new QGridLayout(pInitialSizeWidget);
   pInitialSizeLayout->setMargin(0);
   pInitialSizeLayout->setSpacing(5);
   pInitialSizeLayout->addWidget(mpFixedSizeRadio, 0, 0, 1, 3);
   pInitialSizeLayout->addWidget(pWidthLabel, 1, 1);
   pInitialSizeLayout->addWidget(mpWidthSpin, 1, 2);
   pInitialSizeLayout->addWidget(pHeightLabel, 2, 1);
   pInitialSizeLayout->addWidget(mpHeightSpin, 2, 2);
   pInitialSizeLayout->addWidget(mpMaximizedRadio, 3, 0, 1, 3);
   pInitialSizeLayout->addLayout(pPercentageLayout, 4, 0, 1, 4);
   pInitialSizeLayout->setColumnStretch(3, 10);
   pInitialSizeLayout->setColumnMinimumWidth(0, 10);
   LabeledSection* pInitialSizeSection = new LabeledSection(pInitialSizeWidget, "Default Size", this);

   // Initial Zoom
   mpZoomToFitRadio = new QRadioButton("Zoom To Fit", this);
   mpZoomPercentRadio = new QRadioButton("Percentage:", this);
   mpZoomPercentSpin = new QSpinBox(this);
   mpZoomPercentSpin->setRange(0, numeric_limits<int>::max());
   mpZoomPercentSpin->setSuffix("%");
   QButtonGroup* pInitialZoomGroup = new QButtonGroup(this);
   pInitialZoomGroup->addButton(mpZoomToFitRadio);
   pInitialZoomGroup->addButton(mpZoomPercentRadio);

   QWidget* pInitialZoomWidget = new QWidget(this);
   QGridLayout* pInitialZoomLayout = new QGridLayout(pInitialZoomWidget);
   pInitialZoomLayout->setMargin(0);
   pInitialZoomLayout->setSpacing(5);
   pInitialZoomLayout->addWidget(mpZoomToFitRadio, 0, 0, 1, 3);
   pInitialZoomLayout->addWidget(mpZoomPercentRadio, 1, 0);
   pInitialZoomLayout->addWidget(mpZoomPercentSpin, 1, 1);
   pInitialZoomLayout->setColumnStretch(2, 10);
   LabeledSection* pInitialZoomSection = new LabeledSection(pInitialZoomWidget, "Default Zoom", this);

   // Copy Snapshot
   mpResolutionWidget = new ResolutionWidget();
   LabeledSection* pResolutionSection = new LabeledSection(mpResolutionWidget, "Copy Snapshot", this);
   mpResolutionWidget->setAspectRatioLock(View::getSettingAspectRatioLock());
   mpResolutionWidget->setResolution(View::getSettingOutputWidth(), View::getSettingOutputHeight());
   mpResolutionWidget->setUseViewResolution(View::getSettingUseViewResolution());

   // Other Initial Options
   QLabel* pBackgroundLabel = new QLabel("Background Color:", this);
   mpBackgroundColor = new CustomColorButton(this);
   mpBackgroundColor->usePopupGrid(true);

   QLabel* pOriginLabel = new QLabel("Origin Location:", this);
   mpOriginCombo = new QComboBox(this);
   mpOriginCombo->setEditable(false);
   mpOriginCombo->addItem("Lower Left");
   mpOriginCombo->addItem("Upper Left");

   QLabel* pLinkLabel = new QLabel("Link Type:", this);
   mpLinkCombo = new QComboBox(this);
   mpLinkCombo->setEditable(false);
   mpLinkCombo->addItem("Automatic");
   mpLinkCombo->addItem("Mirror");
   mpLinkCombo->addItem("Latitude/Longitude");

   mpConfirmClose = new QCheckBox("Confirm Close", this);

   QWidget* pOtherOptWidget = new QWidget(this);
   QGridLayout* pOtherOptLayout = new QGridLayout(pOtherOptWidget);
   pOtherOptLayout->setMargin(0);
   pOtherOptLayout->setSpacing(5);
   pOtherOptLayout->addWidget(pBackgroundLabel, 0, 0);
   pOtherOptLayout->addWidget(mpBackgroundColor, 0, 1, Qt::AlignLeft);
   pOtherOptLayout->addWidget(pOriginLabel, 1, 0);
   pOtherOptLayout->addWidget(mpOriginCombo, 1, 1, Qt::AlignLeft);
   pOtherOptLayout->addWidget(pLinkLabel, 2, 0);
   pOtherOptLayout->addWidget(mpLinkCombo, 2, 1, Qt::AlignLeft);
   pOtherOptLayout->addWidget(mpConfirmClose, 3, 0, 1, 2);
   pOtherOptLayout->setColumnStretch(1, 10);
   LabeledSection* pOtherOptSection = new LabeledSection(pOtherOptWidget, "Other Default Properties", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pInitialSizeSection);
   pLayout->addWidget(pInitialZoomSection);
   pLayout->addWidget(pResolutionSection);
   pLayout->addWidget(pOtherOptSection);
   pLayout->addStretch(10);

   // Connections
   VERIFYNR(connect(mpZoomPercentRadio, SIGNAL(toggled(bool)), mpZoomPercentSpin, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpFixedSizeRadio, SIGNAL(toggled(bool)), pHeightLabel, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpFixedSizeRadio, SIGNAL(toggled(bool)), mpHeightSpin, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpFixedSizeRadio, SIGNAL(toggled(bool)), pWidthLabel, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpFixedSizeRadio, SIGNAL(toggled(bool)), mpWidthSpin, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpPercentageRadio, SIGNAL(toggled(bool)), mpPercentageSpin, SLOT(setEnabled(bool))));

   // Initialize From Settings
   WindowSizeType sizeType = WorkspaceWindow::getSettingWindowSize();
   mpHeightSpin->setValue(WorkspaceWindow::getSettingWindowHeight());
   mpWidthSpin->setValue(WorkspaceWindow::getSettingWindowWidth());
   mpPercentageSpin->setValue(WorkspaceWindow::getSettingWindowPercentage());

   mpFixedSizeRadio->setChecked(sizeType == FIXED_SIZE);
   mpMaximizedRadio->setChecked(sizeType == MAXIMIZED);
   mpPercentageRadio->setChecked(sizeType == WORKSPACE_PERCENTAGE);
   pHeightLabel->setEnabled(sizeType == FIXED_SIZE);
   mpHeightSpin->setEnabled(sizeType == FIXED_SIZE);
   pWidthLabel->setEnabled(sizeType == FIXED_SIZE);
   mpWidthSpin->setEnabled(sizeType == FIXED_SIZE);
   mpPercentageSpin->setEnabled(sizeType == WORKSPACE_PERCENTAGE);

   unsigned int zoomPercentage = PerspectiveView::getSettingZoomPercentage();
   if (zoomPercentage == 0)
   {
      mpZoomToFitRadio->setChecked(true);
      mpZoomPercentSpin->setEnabled(false);
      mpZoomPercentSpin->setValue(100);
   }
   else
   {
      mpZoomPercentRadio->setChecked(true);
      mpZoomPercentSpin->setEnabled(true);
      mpZoomPercentSpin->setValue(zoomPercentage);
   }

   DataOrigin origin = View::getSettingDataOrigin();
   if (origin == LOWER_LEFT)
   {
      mpOriginCombo->setCurrentIndex(0);
   }
   else if (origin == UPPER_LEFT)
   {
      mpOriginCombo->setCurrentIndex(1);
   }

   LinkType linkType = View::getSettingLinkType();
   if (linkType == AUTOMATIC_LINK)
   {
      mpLinkCombo->setCurrentIndex(0);
   }
   else if (linkType == MIRRORED_LINK)
   {
      mpLinkCombo->setCurrentIndex(1);
   }
   else if (linkType == GEOCOORD_LINK)
   {
      mpLinkCombo->setCurrentIndex(2);
   }

   mpBackgroundColor->setColor(View::getSettingBackgroundColor());
   mpConfirmClose->setChecked(WorkspaceWindow::getSettingConfirmClose());
}

void OptionsView::applyChanges()
{
   WorkspaceWindow::setSettingWindowHeight(mpHeightSpin->value());
   WorkspaceWindow::setSettingWindowWidth(mpWidthSpin->value());
   WorkspaceWindow::setSettingWindowPercentage(mpPercentageSpin->value());
   WindowSizeType sizeType;
   if (mpFixedSizeRadio->isChecked())
   {
      sizeType = FIXED_SIZE;
   }
   else if (mpMaximizedRadio->isChecked())
   {
      sizeType = MAXIMIZED;
   }
   else if (mpPercentageRadio->isChecked())
   {
      sizeType = WORKSPACE_PERCENTAGE;
   }
   WorkspaceWindow::setSettingWindowSize(sizeType);

   unsigned int zoomPercentage = 0;
   if (mpZoomPercentRadio->isChecked() == true)
   {
      zoomPercentage = mpZoomPercentSpin->value();
   }
   PerspectiveView::setSettingZoomPercentage(zoomPercentage);

   DataOrigin origin;
   if (mpOriginCombo->currentIndex() == 0)
   {
      origin = LOWER_LEFT;
   }
   else if (mpOriginCombo->currentIndex() == 1)
   {
      origin = UPPER_LEFT;
   }
   View::setSettingDataOrigin(origin);

   LinkType linkType = NO_LINK;
   if (mpLinkCombo->currentIndex() == 0)
   {
      linkType = AUTOMATIC_LINK;
   }
   else if (mpLinkCombo->currentIndex() == 1)
   {
      linkType = MIRRORED_LINK;
   }
   else if (mpLinkCombo->currentIndex() == 2)
   {
      linkType = GEOCOORD_LINK;
   }
   View::setSettingLinkType(linkType);

   View::setSettingBackgroundColor(mpBackgroundColor->getColorType());
   WorkspaceWindow::setSettingConfirmClose(mpConfirmClose->isChecked());

   unsigned int width = 0;
   unsigned int height = 0;
   View::setSettingUseViewResolution(mpResolutionWidget->getUseViewResolution());
   View::setSettingAspectRatioLock(mpResolutionWidget->getAspectRatioLock());
   mpResolutionWidget->getResolution(width, height);
   View::setSettingOutputWidth(width);
   View::setSettingOutputHeight(height);
}

OptionsView::~OptionsView()
{}
