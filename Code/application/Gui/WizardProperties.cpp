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
#include <QtGui/QPushButton>

#include "AppVerify.h"
#include "WizardProperties.h"
#include "WizardUtilities.h"

WizardProperties::WizardProperties(QWidget* pParent) :
   QWidget(pParent),
   mpNameEdit(NULL),
   mpMenuEdit(NULL),
   mpModeCombo(NULL)
{
   // Name
   QLabel* pNameLabel = new QLabel("Name:", this);
   mpNameEdit = new QLineEdit(this);

   // Menu location
   QLabel* pMenuLabel = new QLabel("Menu Location:", this);
   mpMenuEdit = new QLineEdit(this);

   // Execution mode
   QLabel* pModeLabel = new QLabel("Execution Mode:", this);
   mpModeCombo = new QComboBox(this);
   mpModeCombo->addItem("Batch");
   mpModeCombo->addItem("Interactive");

   // Execute button
   QPushButton* pExecuteButton = new QPushButton("Execute", this);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(5);
   pGrid->setSpacing(5);
   pGrid->addWidget(pNameLabel, 0, 0);
   pGrid->addWidget(mpNameEdit, 0, 1, 1, 2);
   pGrid->addWidget(pMenuLabel, 1, 0);
   pGrid->addWidget(mpMenuEdit, 1, 1, 1, 2);
   pGrid->addWidget(pModeLabel, 2, 0);
   pGrid->addWidget(mpModeCombo, 2, 1);
   pGrid->addWidget(pExecuteButton, 2, 2);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

   // Connections
   mpWizard.addSignal(SIGNAL_NAME(WizardObjectImp, Renamed), Slot(this, &WizardProperties::wizardRenamed));
   mpWizard.addSignal(SIGNAL_NAME(WizardObjectImp, MenuLocationChanged),
      Slot(this, &WizardProperties::menuLocationChanged));
   mpWizard.addSignal(SIGNAL_NAME(WizardObjectImp, BatchModeChanged),
      Slot(this, &WizardProperties::executionModeChanged));

   VERIFYNR(connect(mpNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(setWizardName(const QString&))));
   VERIFYNR(connect(mpMenuEdit, SIGNAL(textChanged(const QString&)), this,
      SLOT(setWizardMenuLocation(const QString&))));
   VERIFYNR(connect(mpModeCombo, SIGNAL(activated(int)), this, SLOT(setWizardExecutionMode(int))));
   VERIFYNR(connect(pExecuteButton, SIGNAL(clicked()), this, SLOT(executeWizard())));
}

WizardProperties::~WizardProperties()
{
   setWizard(NULL);
}

void WizardProperties::setWizard(WizardObject* pWizard)
{
   WizardObjectAdapter* pWizardAdapter = dynamic_cast<WizardObjectAdapter*>(pWizard);
   if (pWizardAdapter == mpWizard.get())
   {
      return;
   }

   mpWizard.reset(pWizardAdapter);
   if (mpWizard.get() == NULL)
   {
      mpNameEdit->clear();
      mpNameEdit->setEnabled(false);
      mpMenuEdit->clear();
      mpMenuEdit->setEnabled(false);
      mpModeCombo->setCurrentIndex(0);
      mpModeCombo->setEnabled(false);
      return;
   }

   // Name
   QString wizardName = QString::fromStdString(mpWizard->getName());
   mpNameEdit->setText(wizardName);
   mpNameEdit->setEnabled(true);

   // Menu location
   QString menuLocation = QString::fromStdString(mpWizard->getMenuLocation());
   mpMenuEdit->setText(menuLocation);
   mpMenuEdit->setEnabled(true);

   // Execution mode
   if (mpWizard->isBatch() == true)
   {
      mpModeCombo->setCurrentIndex(0);
   }
   else
   {
      mpModeCombo->setCurrentIndex(1);
   }

   mpModeCombo->setEnabled(true);
}

WizardObject* WizardProperties::getWizard()
{
   return mpWizard.get();
}

const WizardObject* WizardProperties::getWizard() const
{
   return mpWizard.get();
}

void WizardProperties::wizardRenamed(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardObjectAdapter* pWizard = dynamic_cast<WizardObjectAdapter*>(&subject);
   VERIFYNRV(pWizard != NULL);
   VERIFYNRV(pWizard == mpWizard.get());

   QString& wizardName = QString::fromStdString(mpWizard->getName());
   if (wizardName != mpNameEdit->text())
   {
      mpNameEdit->setText(wizardName);
   }
}

void WizardProperties::menuLocationChanged(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardObjectAdapter* pWizard = dynamic_cast<WizardObjectAdapter*>(&subject);
   VERIFYNRV(pWizard != NULL);
   VERIFYNRV(pWizard == mpWizard.get());

   QString& menuLocation = QString::fromStdString(mpWizard->getMenuLocation());
   if (menuLocation != mpMenuEdit->text())
   {
      mpMenuEdit->setText(menuLocation);
   }
}

void WizardProperties::executionModeChanged(Subject& subject, const std::string& signal, const boost::any& data)
{
   WizardObjectAdapter* pWizard = dynamic_cast<WizardObjectAdapter*>(&subject);
   VERIFYNRV(pWizard != NULL);
   VERIFYNRV(pWizard == mpWizard.get());

   if (mpWizard->isBatch() == true)
   {
      mpModeCombo->setCurrentIndex(0);
   }
   else
   {
      mpModeCombo->setCurrentIndex(1);
   }
}

void WizardProperties::setWizardName(const QString& wizardName)
{
   if (mpWizard.get() != NULL)
   {
      mpWizard->setName(wizardName.toStdString());
   }
}

void WizardProperties::setWizardMenuLocation(const QString& menuLocation)
{
   if (mpWizard.get() != NULL)
   {
      mpWizard->setMenuLocation(menuLocation.toStdString());
   }
}

void WizardProperties::setWizardExecutionMode(int modeIndex)
{
   if (mpWizard.get() != NULL)
   {
      if (modeIndex == 0)
      {
         mpWizard->setBatch(true);
      }
      else if (modeIndex == 1)
      {
         mpWizard->setBatch(false);
      }
   }
}

void WizardProperties::executeWizard()
{
   if (mpWizard.get() != NULL)
   {
      WizardUtilities::runWizard(mpWizard.get());
   }
}
