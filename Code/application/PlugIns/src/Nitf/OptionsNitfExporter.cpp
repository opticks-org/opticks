/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDateEdit>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>

#include "AppVerify.h"
#include "Classification.h"
#include "DateTime.h"
#include "OptionsNitfExporter.h"
#include "UtilityServices.h"

using namespace std;

const QString OptionsNitfExporter::LEVEL = "Classification Level";
const QString OptionsNitfExporter::SYSTEM = "System/Country Code";
const QString OptionsNitfExporter::CODEWORDS = "Codewords";
const QString OptionsNitfExporter::FILE_CONTROL = "Control and Handling";
const QString OptionsNitfExporter::FILE_RELEASING = "File Releasing";
const QString OptionsNitfExporter::DECLASSIFICATION_TYPE = "Declassification Type";
const QString OptionsNitfExporter::DECLASSIFICATION_DATE = "Declassification Date";
const QString OptionsNitfExporter::DECLASSIFICATION_EXEMPTION = "Declassification Exemption";
const QString OptionsNitfExporter::FILE_DOWNGRADE = "Downgrade To";
const QString OptionsNitfExporter::DOWNGRADE_DATE = "Downgrade Date";
const QString OptionsNitfExporter::AUTHORITY_TYPE = "Authority Type";
const QString OptionsNitfExporter::AUTHORITY = "Authority Description";
const QString OptionsNitfExporter::CLASSIFICATION_REASON = "Classification Reason";
const QString OptionsNitfExporter::SOURCE_DATE = "Security Source Date";
const QString OptionsNitfExporter::SECURITY_CONTROL_NUMBER = "Security Control Number";
const QString OptionsNitfExporter::FILE_COPY_NUMBER = "File Copy Number";
const QString OptionsNitfExporter::FILE_NUMBER_OF_COPIES = "Number of Copies";
const QString OptionsNitfExporter::DESCRIPTION = "Description/Comments";

OptionsNitfExporter::OptionsNitfExporterElement::OptionsNitfExporterElement(const QString& name,
   const string& originalValue, const unsigned int& maxLength,
   const QString& defaultValue, QWidget* pSelf, QWidget* pOther) :
   mName(name),
   mDefaultValue(defaultValue),
   mOriginalValue(originalValue),
   mMaxLength(maxLength),
   mpSelf(pSelf),
   mpOther(pOther),
   mModifiedValueIsTruncated(false)
{
   mpLabel = new QLabel(mName);
   setValue(originalValue);
}

void OptionsNitfExporter::OptionsNitfExporterElement::setValue(const string& newValue)
{
   if (mMaxLength > 0 && newValue.size() > mMaxLength)
   {
      if (mModifiedValueIsTruncated == false)
      {
         mpLabel->setText("*" + mName);
      }

      mModifiedValue = newValue.substr(0, mMaxLength);
      mModifiedValueIsTruncated = true;
   }
   else
   {
      if (mModifiedValueIsTruncated == true)
      {
         mpLabel->setText(mName);
      }
      
      mModifiedValue = newValue;
      mModifiedValueIsTruncated = false;
   }
}

OptionsNitfExporter::OptionsNitfExporter(const Classification* const pClassification, QWidget* pParent) :
   QWidget(pParent),
   mDateTimeFormat("%Y%m%d"),
   mQDateFormat("yyyyMMdd"),
   mDateParseFormat("%4d%2d%2d"),
   mpClassification(pClassification)
{
   // Initialization
   VERIFYNRV(createElements());
   setWindowTitle("Classification");

   // Data layout
   QGridLayout* pDataLayout = new QGridLayout(this);
   pDataLayout->setSpacing(10);
   pDataLayout->setMargin(10);

   // Add labels to describe the element grid
   QLabel* pOriginalValueLabel = new QLabel("Original Value", this);
   pDataLayout->addWidget(pOriginalValueLabel, 0, 1);

   QLabel* pModifiedValueLabel = new QLabel("Exported Value", this);
   pDataLayout->addWidget(pModifiedValueLabel, 0, 2);

   // For each element in mElements, add a row to the display
   unsigned int row = 1;
   for (vector<OptionsNitfExporterElement>::iterator iter = mElements.begin(); iter != mElements.end(); iter++, row++)
   {
      // Name of this element
      if (iter->mpLabel != NULL)
      {
         pDataLayout->addWidget(iter->mpLabel, row, 0);
      }

      // Original value of this element
      QLineEdit* pOriginalValue = new QLineEdit(QString::fromStdString(iter->mOriginalValue), this);
      pOriginalValue->setReadOnly(true);
      pDataLayout->addWidget(pOriginalValue, row, 1);

      // Modified value of this element
      if (iter->mpSelf != NULL)
      {
         pDataLayout->addWidget(iter->mpSelf, row, 2);
         pOriginalValue->setToolTip(iter->mpSelf->toolTip());
      }

      // Other auxiliary widget for this element
      if (iter->mpOther != NULL)
      {
         pDataLayout->addWidget(iter->mpOther, row, 3);
      }
   }

   QLabel* pHelpText = new QLabel("Fields marked with an asterisk (*) must be modified "
      "in order to comply with the NITF 2.1 specification (MIL-STD-2500C).");
   pDataLayout->addWidget(pHelpText, row++, 0, 1, 4);
}

OptionsNitfExporter::~OptionsNitfExporter()
{
   // Do nothing
}

bool OptionsNitfExporter::isValid(string& errorMessage)
{
   bool isValid = true;
   for (vector<OptionsNitfExporterElement>::iterator iter = mElements.begin(); iter != mElements.end(); iter++)
   {
      if (iter->mModifiedValueIsTruncated == true)
      {
         errorMessage += QString("The \"" + iter->mName + "\" field is too long.\n").toStdString();
         errorMessage += "It will be truncated to \"" + iter->mModifiedValue + "\".\n\n";
         isValid = false;
      }
   }

   return isValid;
}

string OptionsNitfExporter::getModifiedValue(const QString& name)
{
   string modifiedValue;

   if (name == LEVEL || getModifiedValue(LEVEL) != "U")
   {
      for (vector<OptionsNitfExporterElement>::iterator iter = mElements.begin(); iter != mElements.end(); iter++)
      {
         if (name == iter->mName)
         {
            if (iter->mpSelf == NULL || iter->mpSelf->isEnabled() == true)
            {
               if (iter->mModifiedValue.empty() == true)
               {
                  modifiedValue = iter->mDefaultValue.toStdString();
               }
               else
               {
                  modifiedValue = iter->mModifiedValue;
               }
            }

            break;
         }
      }
   }

   return modifiedValue;
}

void OptionsNitfExporter::valueModified(const QString& newValue)
{
   QWidget* pSender = dynamic_cast<QWidget*>(sender());
   VERIFYNRV(pSender != NULL);

   for (vector<OptionsNitfExporterElement>::iterator iter = mElements.begin(); iter != mElements.end(); iter++)
   {
      if (pSender == iter->mpSelf)
      {
         iter->setValue(newValue.toStdString());
         break;
      }
   }
}

void OptionsNitfExporter::dateModified(const QDate& newDate)
{
   valueModified(newDate.toString(mQDateFormat));
}

bool OptionsNitfExporter::createLineEditElement(const QString& name, const QString& toolTip,
   const string& originalValue, const unsigned int& maxLength,
   const QString& defaultValue, const QString& inputMask)
{
   QLineEdit* pLineEdit = new QLineEdit(this);
   if (maxLength > 0)
   {
      pLineEdit->setMaxLength(static_cast<int>(maxLength));
   }

   if (originalValue.empty() == true)
   {
      pLineEdit->setText(defaultValue);
   }
   else
   {
      pLineEdit->setText(QString::fromStdString(originalValue));
   }

   if (defaultValue.isEmpty() == true)
   {
      pLineEdit->setToolTip(toolTip);
   }
   else
   {
      pLineEdit->setToolTip(toolTip + "\nIf Classified, Default Value: \"" + defaultValue + "\"");
   }

   pLineEdit->setInputMask(inputMask);
   VERIFY(connect(pLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(valueModified(const QString&))));
   mElements.push_back(OptionsNitfExporterElement(name, originalValue, maxLength, defaultValue, pLineEdit));
   return true;
}

bool OptionsNitfExporter::createComboBoxElement(const QString& name, const QString& toolTip,
   const string& originalValue, const QStringList& availableValues,
   const unsigned int& maxLength)
{
   QComboBox* pComboBox = new QComboBox(this);
   OptionsNitfExporterElement element(name, originalValue, maxLength, QString(), pComboBox);
   pComboBox->addItems(availableValues);
   pComboBox->setCurrentIndex(availableValues.indexOf(QString(element.mModifiedValue.c_str())));
   pComboBox->setToolTip(toolTip);
   VERIFY(connect(pComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(valueModified(const QString&))));
   mElements.push_back(element);
   return true;
}

bool OptionsNitfExporter::createDateEditElement(const QString& name, const QString& toolTip,
   const DateTime* pOriginalValue, const unsigned int& maxLength)
{
   QDateEdit* pDateEdit = new QDateEdit(this);
   string originalValue;
   if (pOriginalValue != NULL && pOriginalValue->isValid() == true)
   {
      originalValue = pOriginalValue->getFormattedUtc(mDateTimeFormat);
   }
   else
   {
      originalValue = QDate::currentDate().toString(mQDateFormat).toStdString();
      pDateEdit->setEnabled(false);
   }

   int year, month, day;
   sscanf(originalValue.c_str(), mDateParseFormat.c_str(), &year, &month, &day);

   QDate originalDate(year, month, day);
   pDateEdit->setDate(originalDate);
   pDateEdit->setToolTip(toolTip);

   QCheckBox* pEnabled = new QCheckBox("Enabled", this);
   pEnabled->setChecked(pDateEdit->isEnabled());

   VERIFY(connect(pDateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(dateModified(const QDate&))));
   VERIFY(connect(pEnabled, SIGNAL(toggled(bool)), pDateEdit, SLOT(setEnabled(bool))));
   mElements.push_back(OptionsNitfExporterElement(name,
      originalValue, maxLength, QString(), pDateEdit, pEnabled));
   return true;
}

bool OptionsNitfExporter::createElements()
{
   QString toolTip;
   VERIFY(mpClassification != NULL);

   // Overall classification level.
   QStringList classificationLevels;
   Service<UtilityServices> pUtilities;
   string strClassification = pUtilities->getDefaultClassification();
   VERIFY(strClassification.empty() == false);
   switch (strClassification[0])
   {
      case 'T':
      {
         classificationLevels.append("T");
         // fall through
      }

      case 'S':
      {
         classificationLevels.append("S");
         // fall through
      }

      case 'C':
      {
         classificationLevels.append("C");
         break;
         // do not fall through since Restricted files should never be created on any T, S, or C machine
      }

      case 'R':
      {
         classificationLevels.append("R");
         break;
      }
   }

   classificationLevels.append("U");
   toolTip = "U - Unclassified\nR - Restricted\nC - Confidential\nS - Secret\nT - Top Secret";
   VERIFY(createComboBoxElement(LEVEL, toolTip, mpClassification->getLevel(), classificationLevels, 1));

   // Country code/system used to classify the file.
   toolTip = "National or Multinational security system (see FIPS PUB 10-4) used to classify the file\n"\
             "\"XN\" shall be used for NATO\n";
   VERIFY(createLineEditElement(SYSTEM, toolTip, mpClassification->getSystem(), 2, "US"));

   toolTip = "Codewords associated with this file";
   VERIFY(createLineEditElement(CODEWORDS, toolTip, mpClassification->getCodewords(), 11));

   toolTip = "Security control and/or handling instructions associated with this file";
   VERIFY(createLineEditElement(FILE_CONTROL, toolTip, mpClassification->getFileControl(), 2));

   toolTip = "Entities which this file may be released to";
   VERIFY(createLineEditElement(FILE_RELEASING, toolTip, mpClassification->getFileReleasing(), 20));

   QStringList declassificationTypes;
   declassificationTypes.append("  ");
   declassificationTypes.append("DD");
   declassificationTypes.append("DE");
   declassificationTypes.append("GD");
   declassificationTypes.append("GE");
   declassificationTypes.append("O");
   declassificationTypes.append("X");

   toolTip = "DD - Declassify on the specified date\n"\
             "DE - Declassify on the specified event\n"\
             "GD - Downgrade on the specified date\n"\
             "GE - Downgrade on the specified event\n"\
             "O  - Original Authority's Determination Required\n"\
             "X  - Exempt from automatic declassification";
   VERIFY(createComboBoxElement(DECLASSIFICATION_TYPE, toolTip,
      mpClassification->getDeclassificationType(), declassificationTypes, 2));

   toolTip = "Date to declassify the data (only valid if Declassification Type is DD)";
   VERIFY(createDateEditElement(DECLASSIFICATION_DATE, toolTip, mpClassification->getDeclassificationDate()));

   QStringList declassificationExemptions;
   declassificationExemptions.append(" ");
   declassificationExemptions.append("X1");
   declassificationExemptions.append("X2");
   declassificationExemptions.append("X3");
   declassificationExemptions.append("X4");
   declassificationExemptions.append("X5");
   declassificationExemptions.append("X6");
   declassificationExemptions.append("X7");
   declassificationExemptions.append("X8");
   declassificationExemptions.append("X251");
   declassificationExemptions.append("X252");
   declassificationExemptions.append("X253");
   declassificationExemptions.append("X254");
   declassificationExemptions.append("X255");
   declassificationExemptions.append("X256");
   declassificationExemptions.append("X257");
   declassificationExemptions.append("X258");
   declassificationExemptions.append("X259");
   toolTip = "Declassification exemption reason (only valid if Declassification Type is X)\n"\
             "X1 through X8 - Declassification Exemptions found in DOD 5200.1-R, paragraphs 4-202b(1) to (8)\n"\
             "X251 through X259 - Declassification Exemptions found in DOD 5200.1-R, paragraphs 4-301a(1) to (9)";
   VERIFY(createComboBoxElement(DECLASSIFICATION_EXEMPTION, toolTip,
      mpClassification->getDeclassificationExemption(), declassificationExemptions, 4));

   QStringList downgradeLevels;
   downgradeLevels.append(" ");
   downgradeLevels.append("R");
   downgradeLevels.append("C");
   downgradeLevels.append("S");
   toolTip = "Downgrade to the specified classification (only valid if Declassification Type is GD or GE)\n"\
             "R - Restricted\nC - Confidential\nS - Secret";
   VERIFY(createComboBoxElement(FILE_DOWNGRADE, toolTip, mpClassification->getFileDowngrade(), downgradeLevels, 1));

   toolTip = "Date to declassify the data (only valid if Declassification Type is GD)";
   VERIFY(createDateEditElement(DOWNGRADE_DATE, toolTip, mpClassification->getDowngradeDate()));

   QStringList authorityTypes;
   authorityTypes.append(" ");
   authorityTypes.append("O");
   authorityTypes.append("D");
   authorityTypes.append("M");
   toolTip = "The type of authority used to classify the file\n"\
      "O - Original Classification Authority\n"\
      "D - Derivative from a single source\n"\
      "M - Derivative from multiple sources";
   VERIFY(createComboBoxElement(AUTHORITY_TYPE, toolTip, mpClassification->getAuthorityType(), authorityTypes, 1));

   toolTip = "Description of the Authority used for classification";
   VERIFY(createLineEditElement(AUTHORITY, toolTip, mpClassification->getAuthority(), 40));

   QStringList reasons;
   reasons.push_back(" ");
   reasons.push_back("A");
   reasons.push_back("B");
   reasons.push_back("C");
   reasons.push_back("D");
   reasons.push_back("E");
   reasons.push_back("F");
   reasons.push_back("G");
   toolTip = "Reasons for classification per EO 12598, Section 1.5(a) to (g)";
   VERIFY(createComboBoxElement(CLASSIFICATION_REASON, toolTip, mpClassification->getClassificationReason(), reasons));

   toolTip = "The date of the most recent security source used to derive the classification of the file";
   VERIFY(createDateEditElement(SOURCE_DATE, toolTip, mpClassification->getSecuritySourceDate()));

   toolTip = "Security Control Number";
   VERIFY(createLineEditElement(SECURITY_CONTROL_NUMBER, toolTip, mpClassification->getSecurityControlNumber(), 15));

   toolTip = "The copy number of the file";
   const QString copyNumberInputMask = "00000;0";
   const QString copyNumberDefault = "00000";
   VERIFY(createLineEditElement(FILE_COPY_NUMBER, toolTip,
      mpClassification->getFileCopyNumber(), 5, copyNumberDefault, copyNumberInputMask));

   toolTip = "The total number of copies";
   VERIFY(createLineEditElement(FILE_NUMBER_OF_COPIES, toolTip,
      mpClassification->getFileNumberOfCopies(), 5, copyNumberDefault, copyNumberInputMask));

   toolTip = "Additional information (such as an event description if Declassification Type is DE or GE)";
   VERIFY(createLineEditElement(DESCRIPTION, toolTip, mpClassification->getDescription(), 43));

   return true;
}
