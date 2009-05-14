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
#include "PlugInRegistration.h"
#include "Slot.h"

static const char* sHelpXpm[] = 
{
   "48 48 34 1",
   " 	c None",
   ".	c #CFDAF0",
   "+	c #808080",
   "@	c #DAE4F5",
   "#	c #D5DFF2",
   "$	c #D2DDF1",
   "%	c #D3DEF1",
   "&	c #D6E0F3",
   "*	c #B5C0D4",
   "=	c #D9C4F9",
   "-	c #D5C2F9",
   ";	c #D7CEF9",
   ">	c #DBC8F9",
   ",	c #C0C0C0",
   "'	c #FFFFFF",
   ")	c #D7E0F7",
   "!	c #D2CEF6",
   "~	c #D5B7F5",
   "{	c #D0C3F3",
   "]	c #D0D4F2",
   "^	c #000080",
   "/	c #CAC6ED",
   "(	c #0000FF",
   "_	c #BC9FDB",
   ":	c #BDA9DD",
   "<	c #000000",
   "[	c #510051",
   "}	c #070007",
   "|	c #1A001A",
   "1	c #030003",
   "2	c #C7C9EB",
   "3	c #0D000D",
   "4	c #370037",
   "5	c #750075",
   "    .++++++@#$$$%&*=-;;;==>>                    ",
   "   ++,''+++++++++++++++++++)!~                  ",
   "  +,''++,+++,,++++,,,,,,,,,++++{                ",
   " +,'++,++,''++,,,'''''''''''',,++{              ",
   ".+'+,'+,''++,''''''''''''''''''',++~            ",
   "+,+,'+'''+,''''''''''''''''''''''',+]           ",
   "+'+'+'''+'''''',,,^^^^^^^^^,,'''''''+/          ",
   "++,+'''+'''''',^^^(((((((((^^^,,'''',+/         ",
   "+++,''+''''',,^(((((((((((((((^^,'''',+_        ",
   "+,+''+'''',,^^(((((++++++++(((((^,'''',+:       ",
   "++,'+'''',,^(((((+'''''''''^^((((^,,''',<[      ",
   "++'+'''',,^((((++''''''''''''^((((^,,''',<[     ",
   "++'+''',,^((((+'''''''''''''''^((((^,,'''<:     ",
   "+,+''',,^((((++''''''''''''''''^((((^,''',<[    ",
   "+'+''',,^((((+''''''+++++''''''+((((^,,'''<}    ",
   "++,'',,^(((((+''''',(((((+'''''+(((((^,''',<[   ",
   "++,'',^((((((+'''''+(((((+'''''+((((((^,'',<|   ",
   "++''',^((((((++++++((((((+'''''+((((((^,'''<1   ",
   "++'',,^((((((((((((((((((+'''''^((((((^,'''<2[  ",
   "+,'',^(((((((((((((((((((+'''''^(((((((^,'',<[  ",
   "+,'',^((((((((((((((((((+'''''^((((((((^,'',<|  ",
   "+''',^(((((((((((((((((+''''''^((((((((^,'''<3  ",
   "+''',^((((((((((((((((+''''''^(((((((((^,'''<}  ",
   "+''',^(((((((((((((((+''''''^((((((((((^,'''<1  ",
   "+''',^((((^(((((((((+''''''^(((((((^(^(^,'''<<  ",
   "+''',^(^((((^(^(((((+'''''^((((((((((((^,'''<1  ",
   "+,'',^^((^((((((((^+'''''^((^((^((^(^(^^,'',<}  ",
   "+,'',^(^((((^(((^((+'''''^((((((^((((((^''',<|  ",
   " +'',,^(((^((((^(^(+''''^(^(((^((^(^(^^,'''<_4  ",
   " +''',^(^(((^(^((((+''''^((^(^((^(^(^(^''''<<5  ",
   " +,'',^^(^((((((^((+^^^^^((((((^(^(^(^^''',<1<  ",
   " +,''',^^(^(^(^(((^(^(^(^(^(^(^(^(^(^^'''',<3   ",
   "  +''',,^^(^(^(^(^(^++++(^(^(^(^(^(^^,''''<<[   ",
   "  +,''',^(^(^(^(^(^+''''+(^(^(^(^(^(^'''',<1<   ",
   "   +'''',^(^(^(^(^+''''''+(^(^(^(^(^'''''<<[    ",
   "   +,'''',^(^(^(^(+''''''+^(^(^(^(^,'''',<1<    ",
   "    +,'''',^(^(^(^+''''''^(^(^(^(^,'''',<<5     ",
   "     +,'''',^^^(^(^^''''^(^(^(^^^''''',<<[<     ",
   "      +,''''',^^(^(^^^^^(^(^(^^,''''',<<[<      ",
   "       +,''''',^^^^(^(^(^(^^^^,''''',<<[<       ",
   "        +,''''''',^^^^^^^^^,''''''',<<[<        ",
   "         +,,''''''''''''''''''''',,<<[<         ",
   "          <<,,''''''''''''''''',,<<<[<          ",
   "           5<<,,,'''''''''''',,<<<1[<           ",
   "             [<<<<,,,,,,,,,<<<<<1[<             ",
   "               [[5<<<<<<<<<_<13[<               ",
   "                  5[|3111}|45<                  ",
   "                                                "
};

//////////////////////////////////////////////////////////////////////////
// ModelessPlugIn
static const char* spConfigKey = "ModelessDialog/Runs";

REGISTER_PLUGIN_BASIC(OpticksPlugInSamplerQt, ModelessPlugIn);

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
   setWizardSupported(false);
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

const QIcon& ModelessPlugIn::getIcon() const
{
   if (mMenuIcon.get() == NULL)
   {
      mMenuIcon.reset(new QIcon(QPixmap(sHelpXpm)));
   }
   return *mMenuIcon.get();
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
