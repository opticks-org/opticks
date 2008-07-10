/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsSession.h"

#include "LabeledSection.h"
#include "SessionManager.h"

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

using namespace std;

OptionsSession::OptionsSession() :
   QWidget(NULL)
{
   // Save On Close
   QLabel* pSaveSettingLabel = new QLabel("On Close:", this);
   mpSaveCombo = new QComboBox(this);
   mpSaveCombo->setEditable(false);
   mpSaveCombo->addItem("Save");
   mpSaveCombo->addItem("Don't save");
   mpSaveCombo->addItem("Prompt to save");
   QWidget* pCloseWidget = new QWidget(this);
   QHBoxLayout* pCloseLayout = new QHBoxLayout(pCloseWidget);
   pCloseLayout->setMargin(0);
   pCloseLayout->setSpacing(5);
   pCloseLayout->addWidget(pSaveSettingLabel);
   pCloseLayout->addWidget(mpSaveCombo);
   pCloseLayout->addStretch(10);
   LabeledSection* pCloseSection = new LabeledSection(pCloseWidget, "Save On Close", this);
   
   // Auto-Save
   mpAutoSaveEnabledCheck = new QCheckBox("Enabled", this);
   QLabel* pAutoSaveIntervalLabel = new QLabel("Interval:", this);
   mpAutoSaveIntervalSpin = new QSpinBox(this);
   mpAutoSaveIntervalSpin->setRange(1, 1440);
   mpAutoSaveIntervalSpin->setSuffix(" Minute(s)");
   QWidget* pAutoSaveWidget = new QWidget(this);
   QGridLayout* pAutoSaveLayout = new QGridLayout(pAutoSaveWidget);
   pAutoSaveLayout->setMargin(0);
   pAutoSaveLayout->setSpacing(5);
   pAutoSaveLayout->addWidget(mpAutoSaveEnabledCheck, 0, 0, 1, 2, Qt::AlignLeft);
   pAutoSaveLayout->addWidget(pAutoSaveIntervalLabel, 1, 0);
   pAutoSaveLayout->addWidget(mpAutoSaveIntervalSpin, 1, 1, Qt::AlignLeft);
   pAutoSaveLayout->setColumnStretch(1, 10);
   LabeledSection* pAutoSaveSection = new LabeledSection(pAutoSaveWidget, "Auto-Save", this);
   
   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pCloseSection);
   pLayout->addWidget(pAutoSaveSection);
   pLayout->addStretch(10);

   // Connections
   VERIFYNR(connect(mpAutoSaveEnabledCheck, SIGNAL(toggled(bool)),
      mpAutoSaveIntervalSpin, SLOT(setEnabled(bool))));
   
   // Initialize From Settings
   SessionSaveType saveType = SessionManager::getSettingQueryForSave();
   if (saveType == SESSION_AUTO_SAVE)
   {
      mpSaveCombo->setCurrentIndex(0);
   }
   else if (saveType == SESSION_DONT_AUTO_SAVE)
   {
      mpSaveCombo->setCurrentIndex(1);
   }
   else if (saveType == SESSION_QUERY_SAVE)
   {
      mpSaveCombo->setCurrentIndex(2);
   }
   mpAutoSaveIntervalSpin->setValue(static_cast<int>(SessionManager::getSettingAutoSaveInterval()));
   bool autoSaveEnabled = SessionManager::getSettingAutoSaveEnabled();
   mpAutoSaveEnabledCheck->setChecked(autoSaveEnabled);
   mpAutoSaveIntervalSpin->setEnabled(autoSaveEnabled);
}
   
void OptionsSession::applyChanges()
{  
   SessionSaveType saveType;
   int saveIndex = mpSaveCombo->currentIndex();
   if (saveIndex == 0)
   {
      saveType = SESSION_AUTO_SAVE;
   }
   else if (saveIndex == 1)
   {
      saveType = SESSION_DONT_AUTO_SAVE;
   }
   else if (saveIndex == 2)
   {
      saveType = SESSION_QUERY_SAVE;
   }
   SessionManager::setSettingQueryForSave(saveType);
   SessionManager::setSettingAutoSaveEnabled(mpAutoSaveEnabledCheck->isChecked());
   SessionManager::setSettingAutoSaveInterval(static_cast<unsigned int>(mpAutoSaveIntervalSpin->value()));
}

OptionsSession::~OptionsSession()
{
}
