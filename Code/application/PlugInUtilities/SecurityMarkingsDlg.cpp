/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDir>

#include <QtGui/QInputDialog>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "AppConfig.h"
#include "SecurityMarkingsDlg.h"
#include "ApplicationServices.h"
#include "Classification.h"
#include "ConfigurationSettings.h"
#include "DateTime.h"
#include "DynamicObject.h"
#include "InfoBar.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInManagerServices.h"
#include "StringUtilities.h"
#include "UtilityServices.h"

#include <string>

using namespace std;

namespace
{
   string trimString(string text)
   {
      const char* pText = text.c_str();

      if (text.size() == 0)
      {
         return text;
      }

      unsigned int i;
      for (i = 0; i < text.size(); i++)
      {
         if (!isspace(pText[i]))
         {
            break;
         }
      }

      unsigned int j;
      for (j = text.size() - 1; j > 0; j--)
      {
         if (!isspace(pText[j]))
         {
            break;
         }
      }

      return text.substr(i, j + 1);
   }

};

template <class T>
void populateListFromFile(T* box, const QString& strFilename, bool addBlank)
{
   if ((box == NULL) || (strFilename.isEmpty() == true))
   {
      return;
   }

   string filename = strFilename.toStdString();

   FILE* stream = fopen(filename.c_str(), "r");
   if (stream != NULL)
   {
      box->clear();

      char buffer[1024];
      while (!feof(stream))
      {
         buffer[1023] = 0;
         if (fgets (buffer, 1023, stream) != NULL)
         {
            for (unsigned int i = 0; i < strlen(buffer); i++)
            {
               if (buffer[i] == '\n' || buffer[i] == '\r' || buffer[i] == '/')
               {
                  buffer[i] = 0;
                  break;
               }
            }

            box->addItem(QString::fromStdString(trimString(buffer)));
         }
      }

      if (addBlank)
      {
         box->addItem(QString());
      }

      fclose(stream);
   }
}

SecurityMarkingsDlg::SecurityMarkingsDlg(QWidget* parent, const QString& strInitialMarkings,
                                         Classification* pClassification) :
   QDialog(parent),
   mDeclassDateIsValid(false),
   mDowngradeDateIsValid(false)
{
   mpClass = pClassification;

   // Classification level
   QLabel* pClassificationLabel = new QLabel("Classification Level:", this);
   mpClassLevelCombo = new QComboBox(this);
   mpClassLevelCombo->setEditable(false);

   // Favorites
   QLabel* pFavoritesLabel = new QLabel("Favorites:", this);
   mpFavoritesCombo = new QComboBox(this);
   mpFavoritesCombo->setEditable(false);

   // Top and bottom tab
   mpTabWidget = new QTabWidget(this);

   QWidget* pTopTab = new QWidget();
   QListWidget* pFieldList = new QListWidget(pTopTab);
   pFieldList->addItem("Codewords");
   pFieldList->addItem("System");
   pFieldList->addItem("Releasability");
   pFieldList->addItem("Country Code");
   pFieldList->addItem("Declassification");
   pFieldList->setFixedWidth(100);

   mpValueStack = new QStackedWidget(pTopTab);

   mpCodewordList = new QListWidget(mpValueStack);
   mpCodewordList->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpValueStack->addWidget(mpCodewordList);

   mpSystemList = new QListWidget(mpValueStack);
   mpSystemList->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpValueStack->addWidget(mpSystemList);

   mpFileReleasingList = new QListWidget(mpValueStack);
   mpFileReleasingList->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpValueStack->addWidget(mpFileReleasingList);

   mpCountryCodeList = new QListWidget(mpValueStack);
   mpCountryCodeList->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpValueStack->addWidget(mpCountryCodeList);

   mpExemptionList = new QListWidget(mpValueStack);
   mpExemptionList->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpValueStack->addWidget(mpExemptionList);

   InfoBar* pInfoBar = new InfoBar(pTopTab);
   pInfoBar->setBackgroundColor(Qt::gray);
   pInfoBar->setTitleColor(Qt::black);
   pInfoBar->setTitleFont(QFont("Tahoma", 8, QFont::Bold, true));
   connect(pFieldList, SIGNAL(currentTextChanged(const QString&)), pInfoBar, SLOT(setTitle(const QString&)));
   connect(pFieldList, SIGNAL(currentRowChanged(int)), mpValueStack, SLOT(setCurrentIndex(int)));

   mpAddButton = new QPushButton("&Add...", pTopTab);
   QPushButton* pResetButton = new QPushButton("&Reset", pTopTab);

   QHBoxLayout* pTopButtonLayout = new QHBoxLayout();
   pTopButtonLayout->setMargin(0);
   pTopButtonLayout->setSpacing(5);
   pTopButtonLayout->addStretch(10);
   pTopButtonLayout->addWidget(mpAddButton);
   pTopButtonLayout->addWidget(pResetButton);

   QLabel* pPreviewLabel = new QLabel("Preview:", pTopTab);
   mpMarkingsEdit = new QTextEdit(pTopTab);
   mpMarkingsEdit->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   mpMarkingsEdit->setReadOnly(true);
   mpMarkingsEdit->setLineWrapMode(QTextEdit::WidgetWidth);
   mpMarkingsEdit->setFixedHeight(48);

   QFont markingsFont(mpMarkingsEdit->font());
   markingsFont.setBold(true);
   mpMarkingsEdit->setFont(markingsFont);

   QGridLayout* pTopGrid = new QGridLayout(pTopTab);
   pTopGrid->setMargin(10);
   pTopGrid->setSpacing(5);
   pTopGrid->addWidget(pFieldList, 0, 0, 3, 1);
   pTopGrid->addWidget(pInfoBar, 0, 1);
   pTopGrid->addWidget(mpValueStack, 1, 1);
   pTopGrid->addLayout(pTopButtonLayout, 2, 1);
   pTopGrid->addWidget(pPreviewLabel, 3, 0, 1, 2);
   pTopGrid->addWidget(mpMarkingsEdit, 4, 0, 1, 2);
   pTopGrid->setRowStretch(1, 10);
   pTopGrid->setColumnStretch(1, 10);

   mpTabWidget->addTab(pTopTab, "Top + Bottom");

   // Descriptor tab
   QWidget* pDescriptorTab = new QWidget();

   QLabel* pClassReasonLabel = new QLabel("Classification Reason:", pDescriptorTab);
   mpClassReasonCombo = new QComboBox(pDescriptorTab);
   mpClassReasonCombo->setEditable(false);

   QLabel* pDeclassTypeLabel = new QLabel("Declassification Type:", pDescriptorTab);
   mpDeclassTypeCombo = new QComboBox(pDescriptorTab);
   mpDeclassTypeCombo->setEditable(false);

   QLabel* pDeclassDateLabel = new QLabel("Declassification Date:", pDescriptorTab);
   mpDeclassDateEdit = new QDateEdit(pDescriptorTab);
   mpDeclassDateEdit->clear();

   mpResetDeclassDateButton = new QPushButton("Clear", pDescriptorTab);

   QLabel* pFileDowngradeLabel = new QLabel("File Downgrade:", pDescriptorTab);
   mpFileDowngradeCombo = new QComboBox(pDescriptorTab);
   mpFileDowngradeCombo->setEditable(false);

   QLabel* pDowngradeDateLabel = new QLabel("Downgrade Date:", pDescriptorTab);
   mpDowngradeDateEdit = new QDateEdit(pDescriptorTab);

   mpResetDowngradeDateButton = new QPushButton("Clear", pDescriptorTab);

   QLabel* pFileControlLabel = new QLabel("File Control:", pDescriptorTab);
   mpFileControlCombo = new QComboBox(pDescriptorTab);
   mpFileControlCombo->setEditable(false);

   QLabel* pDescriptionLabel = new QLabel("Description:", pDescriptorTab);
   mpDescriptionEdit = new QTextEdit(pDescriptorTab);

   QLabel* pCopyNumberLabel = new QLabel("Copy Number:", pDescriptorTab);
   mpCopyNumberSpinBox = new QSpinBox(pDescriptorTab);
   mpCopyNumberSpinBox->setMinimum(0);

   QLabel* pNumberOfCopiesLabel = new QLabel("Number of Copies:", pDescriptorTab);
   mpNumberOfCopiesSpinBox = new QSpinBox(pDescriptorTab);
   mpNumberOfCopiesSpinBox->setMinimum(0);

   QGridLayout* pDescriptorGrid = new QGridLayout(pDescriptorTab);
   pDescriptorGrid->setMargin(10);
   pDescriptorGrid->setSpacing(5);
   pDescriptorGrid->addWidget(pClassReasonLabel, 0, 0);
   pDescriptorGrid->addWidget(mpClassReasonCombo, 0, 1);
   pDescriptorGrid->addWidget(pDeclassTypeLabel, 1, 0);
   pDescriptorGrid->addWidget(mpDeclassTypeCombo, 1, 1);
   pDescriptorGrid->addWidget(pDeclassDateLabel, 2, 0);
   pDescriptorGrid->addWidget(mpDeclassDateEdit, 2, 1);
   pDescriptorGrid->addWidget(mpResetDeclassDateButton, 2, 2);
   pDescriptorGrid->addWidget(pFileDowngradeLabel, 3, 0);
   pDescriptorGrid->addWidget(mpFileDowngradeCombo, 3, 1);
   pDescriptorGrid->addWidget(pDowngradeDateLabel, 4, 0);
   pDescriptorGrid->addWidget(mpDowngradeDateEdit, 4, 1);
   pDescriptorGrid->addWidget(mpResetDowngradeDateButton, 4, 2);
   pDescriptorGrid->addWidget(pFileControlLabel, 5, 0);
   pDescriptorGrid->addWidget(mpFileControlCombo, 5, 1);
   pDescriptorGrid->addWidget(pDescriptionLabel, 6, 0, Qt::AlignTop);
   pDescriptorGrid->addWidget(mpDescriptionEdit, 6, 1);
   pDescriptorGrid->addWidget(pCopyNumberLabel, 7, 0);
   pDescriptorGrid->addWidget(mpCopyNumberSpinBox, 7, 1, Qt::AlignLeft);
   pDescriptorGrid->addWidget(pNumberOfCopiesLabel, 8, 0);
   pDescriptorGrid->addWidget(mpNumberOfCopiesSpinBox, 8, 1, Qt::AlignLeft);
   pDescriptorGrid->setRowStretch(4, 10);
   pDescriptorGrid->setColumnStretch(1, 10);

   mpTabWidget->addTab(pDescriptorTab, "Descriptor");

   // Buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);
   QPushButton* pAddToFavoritesButton = new QPushButton("Add to favorites", this);
   QPushButton* pRemoveFromFavoritesButton = new QPushButton("Remove from favorites", this);

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addWidget(pAddToFavoritesButton);
   pButtonLayout->addWidget(pRemoveFromFavoritesButton);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(pOkButton);
   pButtonLayout->addWidget(pCancelButton);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addWidget(pClassificationLabel, 0, 0);
   pGrid->addWidget(mpClassLevelCombo, 0, 1);
   pGrid->addWidget(mpTabWidget, 1, 0, 1, 2);
   pGrid->addWidget(pFavoritesLabel, 2, 0);
   pGrid->addWidget(mpFavoritesCombo, 2, 1);
   pGrid->addLayout(pButtonLayout, 3, 0, 1, 2);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   setWindowTitle("Security Markings");
   setModal(true);
   initialize();
   connect(mpDeclassDateEdit, SIGNAL(dateChanged(const QDate &)), this, SLOT(dateChanged(const QDate &)));
   connect(mpDowngradeDateEdit, SIGNAL(dateChanged(const QDate &)), this, SLOT(dateChanged(const QDate &)));
   importFromClassification(mpClass);
   pFieldList->setCurrentRow(0);

   QString strText;
   if (strInitialMarkings.isEmpty() == false)
   {
      strText = strInitialMarkings;
   }
   else if (mpClass != NULL)
   {
      string classificationText = "";
      mpClass->getClassificationText(classificationText);
      if (classificationText.empty() == false)
      {
         strText = QString::fromStdString(classificationText);
      }
   }

   mpMarkingsEdit->setPlainText(strText);

   checkAddButton(mpValueStack->currentIndex());

   // Connections
   connect(mpClassLevelCombo, SIGNAL(activated(const QString&)), this, SLOT(updateMarkings()));
   connect(mpClassReasonCombo, SIGNAL(activated(const QString&)), this, SLOT(updateMarkings()));
   connect(mpDeclassTypeCombo, SIGNAL(activated(const QString&)), this, SLOT(updateMarkings()));
   connect(mpValueStack, SIGNAL(currentChanged(int)), this, SLOT(updateSize()));
   connect(mpValueStack, SIGNAL(currentChanged(int)), this, SLOT(checkAddButton(int)));
   connect(mpCodewordList, SIGNAL(itemSelectionChanged()), this, SLOT(updateMarkings()));
   connect(mpSystemList, SIGNAL(itemSelectionChanged()), this, SLOT(updateMarkings()));
   connect(mpCountryCodeList, SIGNAL(itemSelectionChanged()), this, SLOT(updateMarkings()));
   connect(mpFileReleasingList, SIGNAL(itemSelectionChanged()), this, SLOT(updateMarkings()));
   connect(mpExemptionList, SIGNAL(itemSelectionChanged()), this, SLOT(updateMarkings()));
   connect(mpDeclassDateEdit, SIGNAL(dateChanged(const QDate &)), this, SLOT(updateMarkings()));
   connect(mpDowngradeDateEdit, SIGNAL(dateChanged(const QDate &)), this, SLOT(updateMarkings()));
   connect(mpAddButton, SIGNAL(clicked()), this, SLOT(addListItem()));
   connect(pResetButton, SIGNAL(clicked()), this, SLOT(resetList()));
   connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
   connect(pAddToFavoritesButton, SIGNAL(clicked()), this, SLOT(addFavoriteItem()));
   connect(pRemoveFromFavoritesButton, SIGNAL(clicked()), this, SLOT(removeFavoriteItem()));
   connect(mpFavoritesCombo, SIGNAL(activated(const QString&)), this, SLOT(favoriteSelected()));
   connect(mpResetDeclassDateButton, SIGNAL(clicked()), this, SLOT(resetDateTimeEdit()));
   connect(mpResetDowngradeDateButton, SIGNAL(clicked()), this, SLOT(resetDateTimeEdit()));
   connect(mpResetDeclassDateButton, SIGNAL(clicked()), this, SLOT(updateMarkings()));
   connect(mpResetDowngradeDateButton, SIGNAL(clicked()), this, SLOT(updateMarkings()));
}

SecurityMarkingsDlg::~SecurityMarkingsDlg()
{
   clearFavorites();
}

QString SecurityMarkingsDlg::getSecurityMarkings() const
{
   QString strMarkings;
   if (mpMarkingsEdit != NULL)
   {
      strMarkings = mpMarkingsEdit->toPlainText();
   }

   return strMarkings;
}

void SecurityMarkingsDlg::initialize(QWidget* pWidget)
{
   // Get the default directory from the options
   QString strDefaultDir = QDir::currentPath();

   const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
   if (pSupportFilesPath != NULL)
   {
      strDefaultDir = QString::fromStdString(pSupportFilesPath->getFullPathAndName() + SLASH + "SecurityMarkings");
   }

   if (strDefaultDir.isEmpty() == false)
   {
      strDefaultDir += "/";
   }
   
   if ((pWidget == NULL) || (pWidget == mpClassLevelCombo))
   {
      populateListFromFile(mpClassLevelCombo, strDefaultDir + "ClassificationLevels.txt", false);
   }

   UtilityServicesExt1* pUtilExt1 = dynamic_cast<UtilityServicesExt1*>(Service<UtilityServices>().get());

   VERIFYNRV(pUtilExt1 != NULL);

   if ((pWidget == NULL) || (pWidget == mpCodewordList))
   {
      vector<string> codewords = pUtilExt1->getCodewords();
      mpCodewordList->clear();
      for (vector<string>::iterator iter = codewords.begin(); iter != codewords.end(); ++iter)
      {
         mpCodewordList->addItem(QString::fromStdString((*iter)));
      }
   }
   if ((pWidget == NULL) || (pWidget == mpSystemList))
   {
      vector<string> systems = pUtilExt1->getSystems();
      mpSystemList->clear();
      for (vector<string>::iterator iter = systems.begin(); iter != systems.end(); ++iter)
      {
         mpSystemList->addItem(QString::fromStdString((*iter)));
      }
   }
   if ((pWidget == NULL) || (pWidget == mpCountryCodeList))
   {
      vector<string> countryCodes = pUtilExt1->getCountryCodes();
      mpCountryCodeList->clear();
      for (vector<string>::iterator iter = countryCodes.begin(); iter != countryCodes.end(); ++iter)
      {
         mpCountryCodeList->addItem(QString::fromStdString((*iter)));
      }
   }
   if ((pWidget == NULL) || (pWidget == mpFileReleasingList))
   {
      vector<string> fileReleasing = pUtilExt1->getFileReleasing();
      mpFileReleasingList->clear();
      for (vector<string>::iterator iter = fileReleasing.begin(); iter != fileReleasing.end(); ++iter)
      {
         mpFileReleasingList->addItem(QString::fromStdString((*iter)));
      }
   }
   if ((pWidget == NULL) || (pWidget == mpExemptionList))
   {
      vector<string> exemption = pUtilExt1->getDeclassificationExemptions();
      mpExemptionList->clear();
      for (vector<string>::iterator iter = exemption.begin(); iter != exemption.end(); ++iter)
      {
         mpExemptionList->addItem(QString::fromStdString((*iter)));
      }
   }
   if ((pWidget == NULL) || (pWidget == mpClassReasonCombo))
   {
      vector<string> classReason = pUtilExt1->getClassificationReasons();
      mpClassReasonCombo->clear();
      for (vector<string>::iterator iter = classReason.begin(); iter != classReason.end(); ++iter)
      {
         mpClassReasonCombo->addItem(QString::fromStdString((*iter)));
      }
   }
   if ((pWidget == NULL) || (pWidget == mpDeclassTypeCombo))
   {
      vector<string> declassType = pUtilExt1->getDeclassificationTypes();
      mpDeclassTypeCombo->clear();
      for (vector<string>::iterator iter = declassType.begin(); iter != declassType.end(); ++iter)
      {
         mpDeclassTypeCombo->addItem(QString::fromStdString((*iter)));
      }
   }
   if ((pWidget == NULL) || (pWidget == mpFileDowngradeCombo))
   {
      vector<string> fileDowngrade = pUtilExt1->getFileDowngrades();
      mpFileDowngradeCombo->clear();
      for (vector<string>::iterator iter = fileDowngrade.begin(); iter != fileDowngrade.end(); ++iter)
      {
         mpFileDowngradeCombo->addItem(QString::fromStdString((*iter)));
      }
   }
   if ((pWidget == NULL) || (pWidget == mpFileControlCombo))
   {
      vector<string> fileControl = pUtilExt1->getFileControls();
      mpFileControlCombo->clear();
      for (vector<string>::iterator iter = fileControl.begin(); iter != fileControl.end(); ++iter)
      {
         mpFileControlCombo->addItem(QString::fromStdString((*iter)));
      }
   }

   deserializeFavorites();
   updateFavoritesCombo();
}

void SecurityMarkingsDlg::importFromClassification(Classification *pClass)
{
   if (pClass == NULL)
   {
      return;
   }

   string text = pClass->getLevel();

   int i = 0;
   if (text.empty() == false)
   {
      QString strText = QString::fromStdString(text);

      for (i = 0; i < mpClassLevelCombo->count(); i++)
      {
         QString strItemText = mpClassLevelCombo->itemText(i);

         if ((strItemText == strText) || (strItemText[0] == strText[0]))
         {
            mpClassLevelCombo->setCurrentIndex(i);
            break;
         }
      }
   }

   mpTabWidget->setEnabled(pClass->getLevel() != "U");

   text = pClass->getCodewords();
   selectListFromString(mpCodewordList, QString::fromStdString(text));

   text = pClass->getSystem();
   selectListFromString(mpSystemList, QString::fromStdString(text));

   text = pClass->getFileControl();
   selectComboFromString(mpFileControlCombo, QString::fromStdString(text));

   text = pClass->getFileReleasing();
   selectListFromString(mpFileReleasingList, QString::fromStdString(text));

   text = pClass->getCountryCode();
   selectListFromString(mpCountryCodeList, QString::fromStdString(text));

   text = pClass->getDeclassificationExemption();
   selectListFromString(mpExemptionList, QString::fromStdString(text));

   const DateTime* pDateTime = pClass->getDeclassificationDate();
   // if a NULL DateTime is passed in, an empty, invalid DateTime will be used
   setDateEdit(mpDeclassDateEdit, pDateTime);

   text = pClass->getClassificationReason();
   if (text.empty() == false)
   {
      QString strText = QString::fromStdString(text);

      for (i = 0; i < mpClassReasonCombo->count(); i++)
      {
         QString strItemText = mpClassReasonCombo->itemText(i);

         if ((strItemText == strText) || (strItemText[0] == strText[0]))
         {
            mpClassReasonCombo->setCurrentIndex(i);
            break;
         }
      }
   }

   text = pClass->getDeclassificationType();
   if (text.empty() == false)
   {
      QString strText = QString::fromStdString(text);

      for (i = 0; i < mpDeclassTypeCombo->count(); i++)
      {
         QString strItemText = mpDeclassTypeCombo->itemText(i);
         if (strItemText == strText)
         {
            mpDeclassTypeCombo->setCurrentIndex(i);
            break;
         }
      }
   }

   text = pClass->getFileDowngrade();
   if (text.empty() == false)
   {
      QString strText = QString::fromStdString(text);

      for (i = 0; i < mpFileDowngradeCombo->count() && i < mpClassLevelCombo->count(); i++)
      {
         QString strItemText = mpClassLevelCombo->itemText(i);
         if (strItemText == strText)
         {
            mpFileDowngradeCombo->setCurrentIndex(i);
            break;
         }
      }
   }

   pDateTime = pClass->getDowngradeDate();
   setDateEdit(mpDowngradeDateEdit, pDateTime);

   mpDescriptionEdit->setPlainText(QString::fromStdString(pClass->getDescription()));
   mpCopyNumberSpinBox->setValue((QString::fromStdString(pClass->getFileCopyNumber())).toInt());
   mpNumberOfCopiesSpinBox->setValue((QString::fromStdString(pClass->getFileNumberOfCopies())).toInt());
}

void SecurityMarkingsDlg::exportToClassification(Classification *pClass)
{
   if (pClass == NULL)
   {
      return;
   }

   QString strText;
   QString strDelimiter = " ";

   strText = mpClassLevelCombo->currentText();
   if (strText.isEmpty() == false)
   {
      string classLevel = strText.toStdString();
      classLevel = classLevel.substr(0, 1);
      pClass->setLevel(classLevel);
      if (strText[0] == 'U')
      {
         return;
      }
   }
   else if (pClass->getLevel().empty() == false)
   {
      pClass->setLevel("");
   }

   strText = getListString(mpSystemList, strDelimiter);
   if (strText.isEmpty() == false)
   {
      pClass->setSystem(strText.toStdString());
   }
   else if (pClass->getSystem().empty() == false)
   {
      pClass->setSystem("");
   }

   strText = getListString(mpCodewordList, strDelimiter);
   if (strText.isEmpty() == false)
   {
      pClass->setCodewords(strText.toStdString());
   }
   else if (pClass->getCodewords().empty() == false)
   {
      pClass->setCodewords("");
   }

   strText = mpFileControlCombo->currentText();
   if (strText.isEmpty() == false)
   {
      pClass->setFileControl(strText.toStdString());
   }
   else if (pClass->getFileControl().empty() == false)
   {
      pClass->setFileControl("");
   }

   strText = getListString(mpCountryCodeList, strDelimiter);
   if (strText.isEmpty() == false)
   {
      pClass->setCountryCode(strText.toStdString());
   }
   else if (pClass->getCountryCode().empty() == false)
   {
      pClass->setCountryCode("");
   }

   strText = getListString(mpFileReleasingList, strDelimiter);
   if (strText.isEmpty() == false)
   {
      pClass->setFileReleasing(strText.toStdString());
   }
   else if (pClass->getFileReleasing().empty() == false)
   {
      pClass->setFileReleasing("");
   }

   QDate newDate = mpDeclassDateEdit->date();
   if (mDeclassDateIsValid == true)
   {
      FactoryResource<DateTime> pDateTime;
      if (pDateTime.get() != NULL)
      {
         pDateTime->set(newDate.year(), newDate.month(), newDate.day());
         pClass->setDeclassificationDate(pDateTime.get());
      }
   }
   else
   {
      FactoryResource<DateTime> pDateTime;
      if (pDateTime.get() != NULL)
      {
         pClass->setDeclassificationDate(pDateTime.get());
      }
   }

   strText = getListString(mpExemptionList, strDelimiter);
   if (strText.isEmpty() == false)
   {
      pClass->setDeclassificationExemption(strText.toStdString());
   }
   else if (pClass->getDeclassificationExemption().empty() == false)
   {
      pClass->setDeclassificationExemption("");
   }

   strText = mpClassReasonCombo->currentText();
   if (strText.isEmpty() == false)
   {
      string classReason = strText.toStdString();
      classReason = classReason.substr(0, 1);
      pClass->setClassificationReason(classReason);
   }
   else if (pClass->getClassificationReason().empty() == false)
   {
      pClass->setClassificationReason("");
   }

   strText = mpDeclassTypeCombo->currentText();
   if (strText.isEmpty() == false)
   {
      string declassType = strText.toStdString();
      declassType = declassType.substr(0, 2);
      pClass->setDeclassificationType(declassType);
   }
   else if (pClass->getDeclassificationType().empty() == false)
   {
      pClass->setDeclassificationType("");
   }

   strText = mpFileDowngradeCombo->currentText();
   if (strText.isEmpty() == false)
   {
      string fileDowngrade = strText.toStdString();
      fileDowngrade = fileDowngrade.substr(0, 1);
      pClass->setFileDowngrade(fileDowngrade);
   }
   else if (pClass->getFileDowngrade().empty() == false)
   {
      pClass->setFileDowngrade("");
   }

   newDate = mpDowngradeDateEdit->date();
   if (mDowngradeDateIsValid == true)
   {
      FactoryResource<DateTime> pDateTime;
      if (pDateTime.get() != NULL)
      {
         pDateTime->set(newDate.year(), newDate.month(), newDate.day());
         pClass->setDowngradeDate(pDateTime.get());
      }
   }
   else
   {
      FactoryResource<DateTime> pDateTime;
      if (pDateTime.get() != NULL)
      {
         pClass->setDowngradeDate(pDateTime.get());
      }
   }

   strText = mpDescriptionEdit->toPlainText();
   if (strText.isEmpty() == false)
   {
      pClass->setDescription(strText.toStdString());
   }
   else if (pClass->getDescription().empty() == false)
   {
      pClass->setDescription("");
   }

   strText = QString::number(mpCopyNumberSpinBox->value());
   if (strText.isEmpty() == false)
   {
      pClass->setFileCopyNumber(strText.toStdString());
   }
   else if (pClass->getFileCopyNumber().empty() == false)
   {
      pClass->setFileCopyNumber("");
   }

   strText = QString::number(mpNumberOfCopiesSpinBox->value());
   if (strText.isEmpty() == false)
   {
      pClass->setFileNumberOfCopies(strText.toStdString());
   }
   else if (pClass->getFileNumberOfCopies().empty() == false)
   {
      pClass->setFileNumberOfCopies("");
   }
}

QString SecurityMarkingsDlg::getListString(QListWidget* pListWidget, const QString& strDelimiter)
{
   if (pListWidget == NULL)
   {
      return QString();
   }

   QString strText;

   QList<QListWidgetItem*> selectedItems = pListWidget->selectedItems();
   for (int j = 0; j < pListWidget->count(); ++j)
   {
      QListWidgetItem* pItem = pListWidget->item(j);
      if (pItem->isSelected())
      {
         if (pItem != NULL)
         {
            if (strText.isEmpty() == false)
            {
               strText += strDelimiter;
            }

            QString strItemText = pItem->text();
            if (strItemText.isEmpty() == false)
            {
               strItemText.replace('\\', "\\\\");
               strItemText.replace(' ', "\\ ");
               strText += strItemText;
            }
         }
      }
   }

   return strText;
}

void SecurityMarkingsDlg::selectListFromString(QListWidget* pListWidget, const QString& strText)
{
   if (pListWidget == NULL)
   {
      return;
   }

   // There are signals emitted whenever there is a change to the listboxes.
   // We really only want the signals when changed through the GUI.  Simulate
   // by blocking them when changing the listbox programmatically.
   pListWidget->blockSignals(true);
 
   pListWidget->clearSelection();
   if (strText.isEmpty() == true)
   {
      pListWidget->blockSignals(false);
      return;
   }

   const char* ptr = StringUtilities::escapedToken(strText.toStdString());

   // setSelected() below will signal to SecurityMarkingsDlg::updateMarkings(),
   // which indirectly calls selectListFromString.  Block the signal to prevent this.
   while (ptr != NULL)
   {
      QList<QListWidgetItem*> items = pListWidget->findItems(QString::fromLatin1(ptr), Qt::MatchExactly);
      for (int i = 0; i < items.count(); ++i)
      {
         QListWidgetItem* pItem = items[i];
         if (pItem != NULL)
         {
            pListWidget->setItemSelected(pItem, true);
         }
         else
         {
            pListWidget->addItem(QString::fromLatin1(ptr));

            pItem = pListWidget->item(pListWidget->count() - 1);
            pListWidget->setItemSelected(pItem, true);
         }
      }

      ptr = StringUtilities::escapedToken();
   }
   pListWidget->blockSignals(false);
}

void SecurityMarkingsDlg::selectComboFromString(QComboBox* pComboBox, const QString& strText)
{
   if ((pComboBox == NULL) || (strText.isEmpty() == true))
   {
      return;
   }

   for (int i = 0; i < pComboBox->count(); i++)
   {
      if (pComboBox->itemText(i) == strText)
      {
         pComboBox->setCurrentIndex(i);
         break;
      }
   }
}

void SecurityMarkingsDlg::setDateEdit(QDateEdit* pDateEdit, const DateTime* pDateTime)
{
   if (pDateEdit == NULL)
   {
      return;
   }

   if (pDateTime == NULL || pDateTime->isValid() == false)
   {
      dateChanged(pDateEdit, QDate());
   }
   else
   {
      string dateString;
#if defined(WIN_API)
      string format = "%#m %#d %#Y %#H %#M %#S";
#else
      string format = "%m %d %Y %H %M %S";
#endif
      dateString = pDateTime->getFormattedUtc(format);

      int iMonth = 0;
      int iDay = 0;
      int iYear = 0;
      int iHour = 0;
      int iMinute = 0;
      int iSecond = 0;

      sscanf(dateString.c_str(), "%d %d %d %d %d %d", &iMonth, &iDay, &iYear, &iHour, &iMinute, &iSecond);

      QDate dateValue(iYear, iMonth, iDay);
      pDateEdit->setDate(dateValue);
   }
}

void SecurityMarkingsDlg::addListItem()
{
   QListWidget* pList = static_cast<QListWidget*>(mpValueStack->currentWidget());
   if (pList != NULL)
   {
      QString strItem = QInputDialog::getText(this, "Security Markings", "New Item:");
      if (strItem.isEmpty() == false)
      {
         pList->addItem(strItem);
      }
   }
}

void SecurityMarkingsDlg::resetList()
{
   QListWidget* pList = static_cast<QListWidget*>(mpValueStack->currentWidget());
   if (pList != NULL)
   {
      initialize(pList);
      updateMarkings();
   }
}

void SecurityMarkingsDlg::updateMarkings()
{
   QString strMarkings;

   FactoryResource<Classification> pClassification;
   if (pClassification.get() != NULL)
   {
      exportToClassification(pClassification.get());
      importFromClassification(pClassification.get());

      string classificationText;
      pClassification->getClassificationText(classificationText);
      if (classificationText.empty() == false)
      {
         strMarkings = QString::fromStdString(classificationText);
      }
   }

   mpMarkingsEdit->setPlainText(strMarkings);
}

void SecurityMarkingsDlg::updateSize()
{
   mpValueStack->updateGeometry();
   mpTabWidget->updateGeometry();
}

void SecurityMarkingsDlg::accept()
{
   QString strMarkings = getSecurityMarkings();
   if (strMarkings.isEmpty() == true)
   {
      QMessageBox::critical(this, "Security Markings", "No markings are selected!  Please "
         "select valid security markings before accepting the changes.");
      return;
   }

   FactoryResource<Classification> pClass;
   if (pClass.get() == NULL)
   {
      return;
   }

   exportToClassification(pClass.get());

   string errorMessage;
   if (pClass->isValid(errorMessage) == false)
   {
      if (errorMessage.empty() == true)
      {
         errorMessage = "An unknown error has occurred while validating the marking.";
      }

      QMessageBox::critical(this, windowTitle(), QString::fromStdString(errorMessage));
      return;
   }

   if (mpClass != NULL)
   {
      mpClass->setClassification(pClass.get());
   }

   serializeFavorites();
   QDialog::accept();
}

void SecurityMarkingsDlg::addFavoriteItem()
{
   FactoryResource<Classification> pFactoryClass;
   Classification* pClass = pFactoryClass.release();

   exportToClassification(pClass);
   mlstFavorites.push_back(pClass);

   updateFavoritesCombo();
}

void SecurityMarkingsDlg::removeFavoriteItem()
{
   int iRemoveClass = mpFavoritesCombo->currentIndex();
   if (iRemoveClass >= 0 && (unsigned int)iRemoveClass < mlstFavorites.size())
   {
      vector<Classification*>::iterator itr = mlstFavorites.begin()+iRemoveClass;

      // track the removed favorites for later sync with ConfigurationSettings
      mlstRemovedFavorites.push_back(mpFavoritesCombo->currentText().toStdString());

      // vector owns the classification, needs to delete it
      Classification* pClassification = (*itr);
      if (pClassification != NULL)
      {
         Service<ApplicationServices> pApp;
         if (pApp.get() != NULL)
         {
            ObjectFactory* pObjFact = pApp->getObjectFactory();
            if (pObjFact != NULL)
            {
               pObjFact->destroyObject(pClassification, "Classification");
            }
         }
      }

      mlstFavorites.erase(itr, itr+1);
      updateFavoritesCombo();
   }
}

void SecurityMarkingsDlg::favoriteSelected()
{
   Classification* pClass = mlstFavorites[mpFavoritesCombo->currentIndex()];
   importFromClassification(pClass);
   updateMarkings();
}

bool SecurityMarkingsDlg::serializeFavorites()
{
   const DynamicObject* pFavorites = SecurityMarkingsDlg::getSettingFavorites();
   FactoryResource<DynamicObject> pNewFavorites;
   VERIFY(pNewFavorites.get() != NULL);
   pNewFavorites->merge(pFavorites);

   unsigned int i = 0;
   for (i = 0; i < mlstRemovedFavorites.size(); ++i)
   {
      string strRemoved = mlstRemovedFavorites[i];
      pNewFavorites->removeAttribute(strRemoved);
   }

   for (i = 0; i < mlstFavorites.size(); ++i)
   {
      string level = mlstFavorites[i]->getLevel();
      string codewords = mlstFavorites[i]->getCodewords();
      string system = mlstFavorites[i]->getSystem();
      string releasability = mlstFavorites[i]->getFileReleasing();
      string countries = mlstFavorites[i]->getCountryCode();
      string exemption = mlstFavorites[i]->getDeclassificationExemption();
      const DateTime* pDeclassificationDate = mlstFavorites[i]->getDeclassificationDate();

      string classText;
      mlstFavorites[i]->getClassificationText(classText);

      FactoryResource<DynamicObject> pClassificationObject;
      pClassificationObject->setAttribute("level", level);
      pClassificationObject->setAttribute("codewords", codewords);
      pClassificationObject->setAttribute("system", system);
      pClassificationObject->setAttribute("releasability", releasability);
      pClassificationObject->setAttribute("countries", countries);
      pClassificationObject->setAttribute("exemption", exemption);
      if (pDeclassificationDate != NULL && pDeclassificationDate->isValid())
      {
         pClassificationObject->setAttribute("declassificationDate", *pDeclassificationDate);
      }
      pNewFavorites->setAttribute(classText, *pClassificationObject.get());
   }
   SecurityMarkingsDlg::setSettingFavorites(pNewFavorites.get());
   return true;
}

bool SecurityMarkingsDlg::deserializeFavorites()
{
   const DynamicObject* pFavorites = SecurityMarkingsDlg::getSettingFavorites();
   if (pFavorites == NULL)
   {
      return false;
   }
   
   clearFavorites();

   vector<string> previews;
   pFavorites->getAttributeNames(previews);
   for (unsigned int i = 0; i < previews.size(); ++i)
   {
      const DataVariant& var = pFavorites->getAttribute(previews[i]);
      const DynamicObject* pFavDynObj = dv_cast<DynamicObject>(&var);
      if (pFavDynObj == NULL)
      {
         continue;
      }
      const string* pLevel = dv_cast<string>(&pFavDynObj->getAttribute("level"));
      const string* pCodewords = dv_cast<string>(&pFavDynObj->getAttribute("codewords"));
      const string* pSystem = dv_cast<string>(&pFavDynObj->getAttribute("system"));
      const string* pReleasability = dv_cast<string>(&pFavDynObj->getAttribute("releasability"));
      const string* pCountries = dv_cast<string>(&pFavDynObj->getAttribute("countries"));
      const string* pExemption = dv_cast<string>(&pFavDynObj->getAttribute("exemption"));
      const DateTime* pDeclassDate = dv_cast<DateTime>(&pFavDynObj->getAttribute("declassificationDate"));

      FactoryResource<Classification> pClass;
      if (pLevel != NULL)
      {
         pClass->setLevel(*pLevel);
      }
      if (pCodewords != NULL)
      {
         pClass->setCodewords(*pCodewords);
      }
      if (pSystem != NULL)
      {
         pClass->setSystem(*pSystem);
      }
      if (pReleasability != NULL)
      {
         pClass->setFileReleasing(*pReleasability);
      }
      if (pCountries != NULL)
      {
         pClass->setCountryCode(*pCountries);
      }
      if (pExemption != NULL)
      {
         pClass->setDeclassificationExemption(*pExemption);
      }
      if (pDeclassDate != NULL && pDeclassDate->isValid())
      {
         pClass->setDeclassificationDate(pDeclassDate);
      }

      // sanity check - make sure our new classification matches the old favorite
      string classText;
      pClass->getClassificationText(classText);
      if (classText != previews[i])
      {
         // if not, kill the old setting, do not add our own
         const DynamicObject* pOriginalFavorites = SecurityMarkingsDlg::getSettingFavorites();
         if (pOriginalFavorites != NULL)
         {
            FactoryResource<DynamicObject> pNewFavorites;
            pNewFavorites->merge(pOriginalFavorites);
            pNewFavorites->removeAttribute(previews[i]);
            SecurityMarkingsDlg::setSettingFavorites(pNewFavorites.get());
         }
      }
      else
      {
         mlstFavorites.push_back(pClass.release());
      }
   }

   return true;
}

void SecurityMarkingsDlg::updateFavoritesCombo()
{
   mpFavoritesCombo->clear();
   for (unsigned int i = 0; i < mlstFavorites.size(); ++i)
   {
      string strClassText;
      mlstFavorites[i]->getClassificationText(strClassText);
      mpFavoritesCombo->insertItem(i, QString::fromStdString(strClassText));
   }
}

void SecurityMarkingsDlg::clearFavorites()
{
   Service<ApplicationServices> pApp;
   if (pApp.get() != NULL)
   {
      ObjectFactory* pObjFact = pApp->getObjectFactory();
      if (pObjFact != NULL)
      {
         for (unsigned int i = 0; i < mlstFavorites.size(); ++i)
         {
            pObjFact->destroyObject(mlstFavorites[i], "Classification");
         }
      }
   }

   mlstFavorites.clear();
}

void SecurityMarkingsDlg::resetDateTimeEdit()
{
   dateChanged(sender(), QDate());
}

void SecurityMarkingsDlg::checkAddButton(int iIndex)
{
   QWidget* pWidget = mpValueStack->widget(iIndex);
   if (pWidget == mpCodewordList || pWidget == mpFileReleasingList || pWidget == mpCountryCodeList)
   {
      mpAddButton->setDisabled(true);
   }
   else
   {
      mpAddButton->setDisabled(false);
   }
}

void SecurityMarkingsDlg::dateChanged(const QDate &date)
{
   dateChanged(sender(), date);
}

void SecurityMarkingsDlg::dateChanged(const QObject *pSender, const QDate &date)
{
   if (pSender == mpResetDeclassDateButton || pSender == mpDeclassDateEdit)
   {
      if (date.isValid() == false)
      {
         mpDeclassDateEdit->setDate(QDate());
      }
      mDeclassDateIsValid = date.isValid();
   }
   else if (pSender == mpResetDowngradeDateButton || pSender == mpDowngradeDateEdit)
   {
      if (date.isValid() == false)
      {
         mpDowngradeDateEdit->setDate(QDate());
      }
      mDowngradeDateIsValid = date.isValid();
   }
}
