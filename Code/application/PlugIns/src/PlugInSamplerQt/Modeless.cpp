/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

#include "ApplicationServices.h"
#include "ConfigurationSettings.h"
#include "DataVariant.h"
#include "DesktopServices.h"
#include "Executable.h"
#include "Modeless.h"
#include "Slot.h"

//////////////////////////////////////////////////////////////////////////
// ModelessPlugIn
static const char* spConfigKey = "ModelessDialog/Runs";

ModelessPlugIn::ModelessPlugIn() :
   mpDialog(NULL),
   mRuns(1),
   mSessionClosed(true)    // Reset this when execute() runs
{
   setName("Modeless Dialog");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Demonstrates creation and cleanup of a modeless dialog in a viewer plug-in.");
   setDescriptorId("{616CAF5A-300D-4c65-9BB1-455CAF5571A1}");
   setShortDescription("Modeless dialog in a plug-in");
   setMenuLocation("[Demo]/Modeless Dialog");
   setProductionStatus(false);
}

ModelessPlugIn::~ModelessPlugIn()
{
   Service<ApplicationServices> pApp;
   pApp->detach(SIGNAL_NAME(ApplicationServices, ApplicationClosed), Slot(this, &ModelessPlugIn::aboutToClose));
   pApp->detach(SIGNAL_NAME(ApplicationServices, SessionClosed), Slot(this, &ModelessPlugIn::sessionClosed));

   Service<ConfigurationSettings> pConfig;
   pConfig->detach(SIGNAL_NAME(ConfigurationSettings, AboutToSave), Slot(this, &ModelessPlugIn::updateConfigSettings));

   VERIFYNR(mSessionClosed);
}

void ModelessPlugIn::aboutToClose(Subject& subject, const std::string& signal, const boost::any &args)
{
   Service<ConfigurationSettings> pConfig;
   DataVariant var = pConfig->getSetting(spConfigKey);
   mRuns = dv_cast<int>(var, 1);
}

void ModelessPlugIn::updateConfigSettings(Subject& subject, const std::string& signal, const boost::any &args)
{
   Service<ConfigurationSettings> pConfig;
   pConfig->setSetting(spConfigKey, mRuns);
}

void ModelessPlugIn::sessionClosed(Subject& subject, const std::string& signal, const boost::any& value)
{
   mSessionClosed = true;
}

bool ModelessPlugIn::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (mpDialog == NULL)
   {
      Service<DesktopServices> pDesktop;
      mpDialog = new ModelessDlg(this, pDesktop->getMainWidget());

      Service<ApplicationServices> pApp;
      pApp->attach(SIGNAL_NAME(ApplicationServices, ApplicationClosed), Slot(this, &ModelessPlugIn::aboutToClose));
      pApp->attach(SIGNAL_NAME(ApplicationServices, SessionClosed), Slot(this, &ModelessPlugIn::sessionClosed));

      mSessionClosed = false;

      Service<ConfigurationSettings> pConfig;
      pConfig->attach(SIGNAL_NAME(ConfigurationSettings, AboutToSave),
         Slot(this, &ModelessPlugIn::updateConfigSettings));

      DataVariant var = pConfig->getSetting(spConfigKey);
      mRuns = dv_cast<int>(var, 1);
      ++mRuns;
   }

   if (mpDialog != NULL)
   {
      mpDialog->show();
   }

   return mpDialog != NULL;
}

bool ModelessPlugIn::abort()
{
   mSessionClosed = true;
   return ViewerShell::abort();
}

QWidget* ModelessPlugIn::getWidget() const
{
   return mpDialog;
}

//////////////////////////////////////////////////////////////////////////
// ModelessDlg

ModelessDlg::ModelessDlg(PlugIn* pPlugIn, QWidget* pParent) :
   QDialog(pParent),
   mpPlugIn(pPlugIn)
{
   Service<ConfigurationSettings> conf;
   DataVariant var = conf->getSetting(spConfigKey);
   int runs = dv_cast<int>(var, 1);

   QLabel* pLabel = new QLabel(QString("Runs: %1").arg(runs), this);

   QPushButton* pCloseButton = new QPushButton("&Close", this);
   pCloseButton->setFocus();

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   pLayout->addStretch();
   pLayout->addWidget(pLabel, 0, Qt::AlignRight);
   pLayout->addWidget(pCloseButton, 0, Qt::AlignRight);

   // Initialization
   setWindowTitle("Modeless Dialog");
   setModal(false);
   resize(200, 100);

   // Connections
   connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close()));
}

ModelessDlg::~ModelessDlg()
{
}

void ModelessDlg::closeEvent(QCloseEvent* pEvent)
{
   QDialog::closeEvent(pEvent);

   if (mpPlugIn != NULL)
   {
      Executable* pExecutable = dynamic_cast<Executable*>(mpPlugIn);
      if (pExecutable != NULL)
      {
         pExecutable->abort();
      }
   }
}
