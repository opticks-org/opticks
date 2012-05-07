/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QTableWidget>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

#include "AboutDlg.h"
#include "Aeb.h"
#include "AppConfig.h"
#include "AppVersion.h"
#include "ConfigurationSettingsImp.h"
#include "DateTime.h"
#include "InstallerServices.h"
#include "LabeledSection.h"
#include "LabeledSectionGroup.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "Service.h"
#include "StringUtilities.h"
#include "UtilityServices.h"

#include <string>
using namespace std;

namespace
{
   struct CaseInsensitiveComparator
   {
      bool operator()(const string& lhs, const string& rhs) const
      {
         return StringUtilities::toLower(lhs) < StringUtilities::toLower(rhs);
      }
   };
}

AboutDlg::AboutDlg(QWidget* parent) :
   QDialog(parent)
{
   // Get the version and date values
   QString strVersion;
   QString strReleaseDate;
   bool bProductionRelease = false;
   QString strReleaseType;

   ConfigurationSettingsImp* pConfigSettings =
      dynamic_cast<ConfigurationSettingsImp*>(Service<ConfigurationSettings>().get());
   strVersion = QString::fromStdString(pConfigSettings->getVersion());
   strVersion += " Build " + QString::fromStdString(pConfigSettings->getBuildRevision());

   const DateTime* pReleaseDate = NULL;
   pReleaseDate = pConfigSettings->getReleaseDate();
   if (pReleaseDate != NULL)
   {
      string format = "%d %b %Y";
      strReleaseDate = QString::fromStdString(pReleaseDate->getFormattedUtc(format));
   }

   bProductionRelease = pConfigSettings->isProductionRelease();
   strReleaseType = QString::fromStdString(
      StringUtilities::toDisplayString(pConfigSettings->getReleaseType()));

   QString strReleaseDescription = QString::fromStdString(pConfigSettings->getReleaseDescription());

   QFont ftApp = QApplication::font();
   ftApp.setBold(true);

   LabeledSectionGroup* pAboutGroup = new LabeledSectionGroup(this);

   QWidget* pAppInfo = new QWidget(this);

   // Version
   QLabel* pVersionLabel = new QLabel("Version:", pAppInfo);
   pVersionLabel->setFont(ftApp);

   QLabel* pVersion = new QLabel(strVersion, pAppInfo);

   // Release date
   QLabel* pReleaseLabel = new QLabel("Release Date:", pAppInfo);
   pReleaseLabel->setFont(ftApp);

   QLabel* pRelease = new QLabel(strReleaseDate, pAppInfo);

   // Homepage
   QLabel* pHomepageLabel = new QLabel("Homepage:", pAppInfo);
   pHomepageLabel->setFont(ftApp);

   QLabel* pHomepage = new QLabel("<a href=\"http://opticks.org/\";>http://opticks.org/</a>", pAppInfo);
   pHomepage->setOpenExternalLinks(true);

   // Release information
   QString infoLabelStr = strReleaseType;
   if (bProductionRelease == false)
   {
      infoLabelStr += " - Not for Production Use";
   }

   QLabel* pInfoLabel = new QLabel(infoLabelStr, pAppInfo);
   pInfoLabel->setFont(ftApp);

   QPalette labelPalette = pInfoLabel->palette();
   labelPalette.setColor(QPalette::WindowText, Qt::red);
   pInfoLabel->setPalette(labelPalette);

   QLabel* pIconLabel = new QLabel(pAppInfo);
   pIconLabel->setPixmap(QPixmap(":/images/application-large"));

   // Release description
   QLabel* pDescriptionLabel = new QLabel(strReleaseDescription, pAppInfo);
   pDescriptionLabel->setFont(ftApp);
   pDescriptionLabel->setWordWrap(true);

   QPalette labelDescriptionPalette = pDescriptionLabel->palette();
   labelDescriptionPalette.setColor(QPalette::WindowText, Qt::blue);
   pDescriptionLabel->setPalette(labelDescriptionPalette);

   // Copyright
   QLabel* pCopyrightLabel = new QLabel(APP_COPYRIGHT, pAppInfo);
   pCopyrightLabel->setAlignment(Qt::AlignLeft);
   pCopyrightLabel->setWordWrap(true);

   QGridLayout* pTextGrid = new QGridLayout(pAppInfo);
   pTextGrid->setMargin(0);
   pTextGrid->setSpacing(5);
   pTextGrid->addWidget(pIconLabel, 0, 0, 7, 1, Qt::AlignVCenter);
   pTextGrid->addWidget(pVersionLabel, 0, 1);
   pTextGrid->addWidget(pVersion, 0, 2);
   pTextGrid->addWidget(pReleaseLabel, 1, 1);
   pTextGrid->addWidget(pRelease, 1, 2);
   pTextGrid->addWidget(pHomepageLabel, 2, 1);
   pTextGrid->addWidget(pHomepage, 2, 2);

   pTextGrid->addWidget(pInfoLabel, 3, 1, 1, 2);
   pTextGrid->addWidget(pDescriptionLabel, 4, 1, 1, 2);

   pTextGrid->setRowMinimumHeight(5, 10);
   pTextGrid->addWidget(pCopyrightLabel, 6, 1, 1, 2);
   pTextGrid->setColumnStretch(2, 10);
   pTextGrid->setColumnMinimumWidth(0, 75);

   LabeledSection* pAppInfoSection = new LabeledSection(pAppInfo, APP_NAME_LONG " Version Information");
   pAboutGroup->addSection(pAppInfoSection);

   //Plug-In Suites listing
   list<const Aeb*> aebs = Service<InstallerServices>()->getAebs();
   QTableWidget* pPlugInBrandingTable = new QTableWidget(this);
   pPlugInBrandingTable->setColumnCount(3);
   pPlugInBrandingTable->setRowCount(aebs.size());
   pPlugInBrandingTable->setSelectionMode(QAbstractItemView::NoSelection);
   pPlugInBrandingTable->verticalHeader()->hide();
   pPlugInBrandingTable->setHorizontalHeaderLabels(QStringList() << "Name" << "Description" << "Version");
   pPlugInBrandingTable->horizontalHeader()->setClickable(false);
   pPlugInBrandingTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
   pPlugInBrandingTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
   int itemCount = 0;
   for(list<const Aeb*>::const_iterator aeb = aebs.begin(); aeb != aebs.end(); ++aeb, ++itemCount)
   {
      const Aeb* pAeb = *aeb;
      if (pAeb == NULL || pAeb->isHidden())
      {
         itemCount--;
         continue;
      }
      QTableWidgetItem* pTitle = new QTableWidgetItem(QString::fromStdString(pAeb->getName()));
      pTitle->setToolTip(pTitle->text());
      pPlugInBrandingTable->setItem(itemCount, 0, pTitle);
      QTableWidgetItem* pDesc = new QTableWidgetItem(QString::fromStdString(pAeb->getDescription()));
      pDesc->setToolTip(pDesc->text());
      pPlugInBrandingTable->setItem(itemCount, 1, pDesc);
      QTableWidgetItem* pVersionItem = new QTableWidgetItem(QString::fromStdString(pAeb->getVersion().toString()));
      pVersionItem->setToolTip(pVersionItem->text());
      pPlugInBrandingTable->setItem(itemCount, 2, pVersionItem);
   }
   pPlugInBrandingTable->setRowCount(itemCount);
   pPlugInBrandingTable->resizeColumnToContents(0);
   pPlugInBrandingTable->resizeColumnToContents(2);
   pPlugInBrandingTable->sortItems(0);

   LabeledSection* pPlugInSuiteSection = new LabeledSection(pPlugInBrandingTable, "Extensions");
   pAboutGroup->addSection(pPlugInSuiteSection, 1000);

   // Create the copyright notice box
   QTextEdit* pNoticeEdit = new QTextEdit(this);
   pNoticeEdit->setLineWrapMode(QTextEdit::WidgetWidth);
   pNoticeEdit->setReadOnly(true);
   pNoticeEdit->setHtml(QString::fromStdString(Service<UtilityServices>()->getTextFromFile(":/licenses/opticks")));

   QPalette pltNotice = pNoticeEdit->palette();
   pltNotice.setColor(QPalette::Base, Qt::lightGray);
   pNoticeEdit->setPalette(pltNotice);

   QTabWidget* pLicenseTabBox = new QTabWidget(this);
   pLicenseTabBox->addTab(pNoticeEdit, APP_NAME);

   for(list<const Aeb*>::const_iterator aeb = aebs.begin(); aeb != aebs.end(); ++aeb)
   {
      const Aeb* pAeb = *aeb;
      if (pAeb == NULL)
      {
         continue;
      }
      QStringList licenses = pAeb->getLicenses();
      std::vector<std::string> licenseURLs = pAeb->getLicenseURLs();
      for (int licnum = 0; licnum < licenses.size(); licnum++)
      {
         QTextEdit *pEdit = new QTextEdit(this);
         pEdit->setLineWrapMode(QTextEdit::WidgetWidth);
         pEdit->setReadOnly(true);

         QString url = QString::fromStdString(licenseURLs[licnum]).toLower();
         if (url.endsWith(".html") || url.endsWith(".htm"))
         {
            pEdit->setHtml(licenses[licnum]);
         }
         else
         {
            pEdit->setPlainText(licenses[licnum]);
         }
         QPalette plt = pEdit->palette();
         plt.setColor(QPalette::Base, Qt::lightGray);
         pEdit->setPalette(plt);
         if (licenses.size() > 1)
         {
            pLicenseTabBox->addTab(pEdit, QString("%1 %2").arg(QString::fromStdString(pAeb->getName())).arg(licnum + 1));
         }
         else
         {
            pLicenseTabBox->addTab(pEdit, QString::fromStdString(pAeb->getName()));
         }
      }
   }

   LabeledSection* pCopyrightSection = new LabeledSection(pLicenseTabBox, APP_NAME " and Plug-In Suite Licenses");
   pAboutGroup->addSection(pCopyrightSection, 1000);

   // Create the Special license box
   QWidget* pSpecialLicensesWidget = new QWidget(this);
   QTabWidget* pSpecialLicensesTabBox = new QTabWidget(pSpecialLicensesWidget);

   map<string, string, CaseInsensitiveComparator> dependencyCopyrights;
   vector<PlugInDescriptor*> pluginDescriptors = Service<PlugInManagerServices>()->getPlugInDescriptors();
   for (vector<PlugInDescriptor*>::iterator pluginDescriptor = pluginDescriptors.begin();
      pluginDescriptor != pluginDescriptors.end(); ++pluginDescriptor)
   {
      if (*pluginDescriptor != NULL)
      {
         map<string, string> tmp = (*pluginDescriptor)->getDependencyCopyright();
         for (map<string, string>::iterator tmpIt = tmp.begin(); tmpIt != tmp.end(); ++tmpIt)
         {
            map<string, string, CaseInsensitiveComparator>::iterator current = dependencyCopyrights.find(tmpIt->first);
            string verifyMessage = "A duplicate dependency copyright message name was found, but the copyright message "
               "did not match the existing message for dependency " + tmpIt->first + ".";
            VERIFYNR_MSG(current == dependencyCopyrights.end() || current->second == tmpIt->second, verifyMessage.c_str());
            dependencyCopyrights[tmpIt->first] = tmpIt->second;
         }
      }
   }

   for (map<string, string, CaseInsensitiveComparator>::const_iterator dependencyCopyright = dependencyCopyrights.begin();
      dependencyCopyright != dependencyCopyrights.end(); ++dependencyCopyright)
   {
      QTextEdit* pEdit = new QTextEdit(this);
      pEdit->setLineWrapMode(QTextEdit::WidgetWidth);
      pEdit->setReadOnly(true);
      pEdit->setHtml(QString::fromStdString(dependencyCopyright->second));
      QPalette plt = pEdit->palette();
      plt.setColor(QPalette::Base, Qt::lightGray);
      pEdit->setPalette(plt);
      pSpecialLicensesTabBox->addTab(pEdit, QString::fromStdString(dependencyCopyright->first));
   }

   QLabel* pSpecialLicensesDisclaimer = new QLabel(
      "Some of these libraries have been modified as "
      "indicated following the applicable license text.<br>"
      "Please direct inquiries to the <a href=\"http://opticks.org/confluence/display/opticks/Mailing+Lists\">"
      "Opticks Mailing Lists</a>.", pSpecialLicensesWidget);
   pSpecialLicensesDisclaimer->setFont(ftApp);
   pSpecialLicensesDisclaimer->setWordWrap(true);
   pSpecialLicensesDisclaimer->setOpenExternalLinks(true);

   QLayout* pSpecialLicensesLayout = new QVBoxLayout(pSpecialLicensesWidget);
   pSpecialLicensesLayout->addWidget(pSpecialLicensesTabBox);
   pSpecialLicensesLayout->addWidget(pSpecialLicensesDisclaimer);

   LabeledSection* pLicenseSection = new LabeledSection(pSpecialLicensesWidget, "Third-Party Licenses");
   pAboutGroup->addSection(pLicenseSection, 1000);

   pAboutGroup->addStretch(1);
   pAboutGroup->collapseSection(pLicenseSection);
   pAboutGroup->collapseSection(pCopyrightSection);

   // Create the buttons
   QPushButton* pOk = new QPushButton("&OK", this);
   pOk->setDefault(true);
   VERIFYNR(connect(pOk, SIGNAL(clicked()), this, SLOT(accept())));

   QHBoxLayout* pButtonLayout = new QHBoxLayout;
   pButtonLayout->setMargin(5);
   pButtonLayout->setSpacing(10);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(pOk);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pAboutGroup, 10);
   pLayout->addLayout(pButtonLayout);

   // Initialization
   setWindowTitle(QString("About %1").arg(APP_NAME));
   setModal(true);
   resize(400, 500);
}

AboutDlg::~AboutDlg()
{}
