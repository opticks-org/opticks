/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ConfigurationSettings.h"
#include "LabeledSection.h"
#include "OptionsGeneral.h"
#include "PlotView.h"
#include "ProductView.h"
#include "SpatialDataView.h"
#include "Statistics.h"
#include "StringUtilities.h"
#include "TypesFile.h"
#include "UtilityServices.h"

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <string>
#include <vector>

OptionsGeneral::OptionsGeneral() :
   QWidget(NULL)
{
   // MRU files
   QLabel* pMruFilesLabel = new QLabel("MRU File Entries:", this);

   mpMruFilesSpin = new QSpinBox(this);
   mpMruFilesSpin->setMinimum(0);
   mpMruFilesSpin->setMaximum(16);
   mpMruFilesSpin->setSingleStep(1);

   QWidget* pMruFilesWidget = new QWidget(this);
   QHBoxLayout* pMruFilesLayout = new QHBoxLayout(pMruFilesWidget);
   pMruFilesLayout->setMargin(0);
   pMruFilesLayout->setSpacing(5);
   pMruFilesLayout->addWidget(pMruFilesLabel);
   pMruFilesLayout->addWidget(mpMruFilesSpin);
   pMruFilesLayout->addStretch(10);

   LabeledSection* pMruFilesSection = new LabeledSection(pMruFilesWidget, "Most Recently Used (MRU) Files", this);

   // Undo/redo
   QLabel* pBufferLabel = new QLabel("Buffer Size:", this);
   mpBufferSpin = new QSpinBox(this);
   mpBufferSpin->setRange(1, 3000);
   mpBufferSpin->setSingleStep(1);
   mpBufferSpin->setSuffix(" Actions");
   mpBufferSpin->setToolTip("Maximum number of undo actions per view");

   QWidget* pBufferLayoutWidget = new QWidget();
   QHBoxLayout* pUndoLayout = new QHBoxLayout(pBufferLayoutWidget);
   pUndoLayout->setMargin(0);
   pUndoLayout->setSpacing(5);
   pUndoLayout->addWidget(pBufferLabel);
   pUndoLayout->addWidget(mpBufferSpin);
   pUndoLayout->addStretch(10);
   LabeledSection* pUndoSection = new LabeledSection(pBufferLayoutWidget, "Undo/Redo", this);

   // Threading
   QLabel* pThreadCountLabel = new QLabel("Worker Threads:", this);
   unsigned int maxThreadCount = Service<UtilityServices>()->getNumProcessors();
   mpThreadSpin = new QSpinBox(this);
   mpThreadSpin->setMinimum(1);
   mpThreadSpin->setMaximum(maxThreadCount);
   mpThreadSpin->setSingleStep(1);
   QWidget* pThreadingLayoutWidget = new QWidget(this);
   QHBoxLayout* pThreadingLayout = new QHBoxLayout(pThreadingLayoutWidget);
   pThreadingLayout->setMargin(0);
   pThreadingLayout->setSpacing(5);
   pThreadingLayout->addWidget(pThreadCountLabel);
   pThreadingLayout->addWidget(mpThreadSpin);
   pThreadingLayout->addStretch(10);
   LabeledSection* pThreadingSection = new LabeledSection(pThreadingLayoutWidget, "Multi-threading", this);

   // Statistics Resolution
   QWidget* pStatsWidget = new QWidget(this);
   mpStatsAutoRadio = new QRadioButton("Automatic", pStatsWidget);
   mpStatsFullRadio = new QRadioButton("Full", pStatsWidget);
   mpStatsSkipRadio = new QRadioButton("Skip Factor:", pStatsWidget);
   mpStatsSkipSpin = new QSpinBox(pStatsWidget);
   mpStatsSkipSpin->setRange(1, 100);

   QGridLayout* pStatsLayout = new QGridLayout(pStatsWidget);
   pStatsLayout->setMargin(0);
   pStatsLayout->setSpacing(5);
   pStatsLayout->addWidget(mpStatsAutoRadio, 0, 0, 1, 3);
   pStatsLayout->addWidget(mpStatsFullRadio, 1, 0, 1, 3);
   pStatsLayout->addWidget(mpStatsSkipRadio, 2, 0);
   pStatsLayout->addWidget(mpStatsSkipSpin, 2, 1);
   pStatsLayout->setColumnStretch(2, 10);
   LabeledSection* pStatsSection = new LabeledSection(pStatsWidget, "Statistics Resolution", this);

   // Progress Dialog
   mpProgressClose = new QCheckBox("Automatically close on process completion", this);
   LabeledSection* pProgressSection = new LabeledSection(mpProgressClose, "Progress Dialog", this);

   // Mouse Wheel Zoom Dialog
   mpMouseWheelZoom = new QCheckBox("Alternate mouse wheel zoom direction", this);
   LabeledSection* pMouseWheelZoomSection = new LabeledSection(mpMouseWheelZoom, "Mouse Wheel Zoom Direction", this);

   // Classification markings positions - only added if display markings is enabled
   LabeledSection* pMarkingsSection(NULL);
   if (ConfigurationSettings::getSettingDisplayClassificationMarkings())
   {
      QWidget* pMarkingsWidget = new QWidget(this);
      QLabel* pLabelPlot = new QLabel("Plot Views:", pMarkingsWidget);
      mpClassificationPositionPlotView = new QComboBox(pMarkingsWidget);
      QLabel* pLabelProduct = new QLabel("Product Views:", pMarkingsWidget);
      mpClassificationPositionProductView = new QComboBox(pMarkingsWidget);
      QLabel* pLabelSpatial = new QLabel("Spatial Data Views:", pMarkingsWidget);
      mpClassificationPositionSpatialView = new QComboBox(pMarkingsWidget);
      QGridLayout* pMarkingsLayout = new QGridLayout(pMarkingsWidget);
      pMarkingsLayout->setMargin(0);
      pMarkingsLayout->setSpacing(5);
      pMarkingsLayout->addWidget(pLabelPlot, 0, 0);
      pMarkingsLayout->addWidget(mpClassificationPositionPlotView, 0, 1);
      pMarkingsLayout->addWidget(pLabelProduct, 1, 0);
      pMarkingsLayout->addWidget(mpClassificationPositionProductView, 1, 1);
      pMarkingsLayout->setRowMinimumHeight(1, mpClassificationPositionProductView->height() * 2);
      pMarkingsLayout->addWidget(pLabelSpatial, 2, 0);
      pMarkingsLayout->addWidget(mpClassificationPositionSpatialView, 2, 1);
      pMarkingsLayout->setColumnStretch(2, 10);
      pMarkingsSection = new LabeledSection(pMarkingsWidget,
         "Classification Marking Positions", this);
      std::vector<std::string> positions = StringUtilities::getAllEnumValuesAsDisplayString<PositionType>();
      for (std::vector<std::string>::iterator it = positions.begin(); it != positions.end(); ++it)
      {
         mpClassificationPositionPlotView->addItem(QString::fromStdString(*it));
         mpClassificationPositionProductView->addItem(QString::fromStdString(*it));
         mpClassificationPositionSpatialView->addItem(QString::fromStdString(*it));
      }
   }

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pMruFilesSection);
   pLayout->addWidget(pUndoSection);
   pLayout->addWidget(pThreadingSection);
   pLayout->addWidget(pStatsSection);
   pLayout->addWidget(pProgressSection);
   pLayout->addWidget(pMouseWheelZoomSection);
   if (pMarkingsSection != NULL)
   {
      pLayout->addWidget(pMarkingsSection);
   }
   pLayout->addStretch(10);

   // Connections
   VERIFYNR(connect(mpStatsSkipRadio, SIGNAL(toggled(bool)), mpStatsSkipSpin, SLOT(setEnabled(bool))));

   // Initialization
   mpMruFilesSpin->setValue(static_cast<int>(ConfigurationSettings::getSettingNumberOfMruFiles()));
   mpBufferSpin->setValue(static_cast<int>(ConfigurationSettings::getSettingUndoBufferSize()));
   mpThreadSpin->setValue(static_cast<int>(ConfigurationSettings::getSettingThreadCount()));

   int statsResolution = Statistics::getSettingResolution();
   if (statsResolution <= 0)
   {
      mpStatsAutoRadio->setChecked(true);
      mpStatsSkipSpin->setEnabled(false);
      mpStatsSkipSpin->setValue(1);
   }
   else if (statsResolution == 1)
   {
      mpStatsFullRadio->setChecked(true);
      mpStatsSkipSpin->setEnabled(false);
      mpStatsSkipSpin->setValue(1);
   }
   else
   {
      mpStatsSkipRadio->setChecked(true);
      mpStatsSkipSpin->setEnabled(true);
      mpStatsSkipSpin->setValue(statsResolution - 1);
   }

   mpProgressClose->setChecked(Progress::getSettingAutoClose());
   mpMouseWheelZoom->setChecked(ConfigurationSettings::getSettingAlternateMouseWheelZoom());
   if (pMarkingsSection != NULL)
   {
      std::string positionStr = StringUtilities::toDisplayString<PositionType>
         (PlotView::getSettingClassificationMarkingPositions());
      int index =  mpClassificationPositionPlotView->findText(QString::fromStdString(positionStr));
      mpClassificationPositionPlotView->setCurrentIndex(index);
      positionStr = StringUtilities::toDisplayString<PositionType>
         (ProductView::getSettingClassificationMarkingPositions());
      index = mpClassificationPositionProductView->findText(QString::fromStdString(positionStr));
      mpClassificationPositionProductView->setCurrentIndex(index);
      positionStr = StringUtilities::toDisplayString<PositionType>
         (SpatialDataView::getSettingClassificationMarkingPositions());
      index = mpClassificationPositionSpatialView->findText(QString::fromStdString(positionStr));
      mpClassificationPositionSpatialView->setCurrentIndex(index);
   }
}

void OptionsGeneral::applyChanges()
{
   // Settings
   ConfigurationSettings::setSettingNumberOfMruFiles(static_cast<unsigned int>(mpMruFilesSpin->value()));
   ConfigurationSettings::setSettingUndoBufferSize(static_cast<unsigned int>(mpBufferSpin->value()));
   ConfigurationSettings::setSettingThreadCount(static_cast<unsigned int>(mpThreadSpin->value()));

   int statsResolution = 0;
   if (mpStatsFullRadio->isChecked() == true)
   {
      statsResolution = 1;
   }
   else if (mpStatsSkipRadio->isChecked() == true)
   {
      statsResolution = mpStatsSkipSpin->value() + 1;
   }
   Statistics::setSettingResolution(statsResolution);

   Progress::setSettingAutoClose(mpProgressClose->isChecked());
   ConfigurationSettings::setSettingAlternateMouseWheelZoom(mpMouseWheelZoom->isChecked());

   if (ConfigurationSettings::getSettingDisplayClassificationMarkings())
   {
      PlotView::setSettingClassificationMarkingPositions(StringUtilities::fromDisplayString<PositionType>(
         mpClassificationPositionPlotView->currentText().toStdString()));
      ProductView::setSettingClassificationMarkingPositions(StringUtilities::fromDisplayString<PositionType>(
         mpClassificationPositionProductView->currentText().toStdString()));
      SpatialDataView::setSettingClassificationMarkingPositions(StringUtilities::fromDisplayString<PositionType>(
         mpClassificationPositionSpatialView->currentText().toStdString()));
   }
}

OptionsGeneral::~OptionsGeneral()
{}
