/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "BadValuesDlg.h"
#include "LabeledSection.h"
#include "LabeledSectionGroup.h"

#include <QtCore/QModelIndex>
#include <QtCore/QStringList>
#include <QtGui/QCheckBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleValidator>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QVBoxLayout>

#include <string>
using namespace std;

// Use Qt::WindowsFlag WindowCloseButtonHint to suppress displaying the "What's This" button in dialog
BadValuesDlg::BadValuesDlg(QWidget* pParent, const QString& statName, bool enableSetAll) :
   QDialog(pParent, Qt::WindowCloseButtonHint),
   mpSetAll(NULL)
{
   // create a grid layout for the entire dialog
   QVBoxLayout* pDialogLayout = new QVBoxLayout(this);
   pDialogLayout->setMargin(10);
   pDialogLayout->setSpacing(10);

   // create the threshold widgets
   QWidget* pThresholdWidget = new QWidget(this);
   QGridLayout* pThresholdLayout = new QGridLayout(pThresholdWidget);
   pThresholdLayout->setMargin(0);
   pThresholdLayout->setSpacing(5);
   mpEnableUpperThreshold = new QCheckBox("Upper:", pThresholdWidget);
   mpEnableUpperThreshold->setToolTip("Check this box to set an upper threshold\nfor bad values. If unchecked, "
      "no upper threshold will\nbe applied in determining bad values.");
   mpUpperThreshold = new QLineEdit(pThresholdWidget);
   mpUpperThreshold->setToolTip("Any value equal to or greater than\nthis value will be considered a bad value.");
   mpUpperThreshold->setValidator(new QDoubleValidator(mpUpperThreshold));
   mpUpperThreshold->setEnabled(false);
   mpEnableLowerThreshold = new QCheckBox("Lower:", pThresholdWidget);
   mpEnableLowerThreshold->setToolTip("Check this box to set a lower threshold\nfor bad values. If unchecked, "
      "no lower threshold will\nbe applied in determining bad values.");
   mpLowerThreshold = new QLineEdit(pThresholdWidget);
   mpLowerThreshold->setToolTip("Any value equal to or less than\nthis value will be considered a bad value.");
   mpLowerThreshold->setValidator(new QDoubleValidator(mpLowerThreshold));
   mpLowerThreshold->setEnabled(false);
   pThresholdLayout->addWidget(mpEnableUpperThreshold, 0, 0);
   pThresholdLayout->addWidget(mpUpperThreshold, 0, 1);
   pThresholdLayout->addWidget(mpEnableLowerThreshold, 1, 0);
   pThresholdLayout->addWidget(mpLowerThreshold, 1, 1);
   pThresholdLayout->setColumnStretch(1, 10);
   LabeledSection* pThresholdSection = new LabeledSection(pThresholdWidget, "Thresholds", this);

   // create the individual values widgets
   QWidget* pValuesWidget = new QWidget(this);
   QGridLayout* pValuesLayout = new QGridLayout(pValuesWidget);
   pValuesLayout->setMargin(0);
   pValuesLayout->setSpacing(5);
   mpValues = new QTreeWidget(pValuesWidget);
   mpValues->setHeaderHidden(true);
   mpValues->setRootIsDecorated(false);
   mpValues->setItemDelegate(new BadValueItemDelegate(mpValues));
   mpValues->setSelectionBehavior(QAbstractItemView::SelectItems);
   mpValues->setSelectionMode(QAbstractItemView::ExtendedSelection);
   QPushButton* pAddValueButton = new QPushButton("Add", pValuesWidget);
   QPushButton* pDeleteValuesButton = new QPushButton("Delete", pValuesWidget);
   pValuesLayout->addWidget(mpValues, 0, 0, 2, 1);
   pValuesLayout->addWidget(pAddValueButton, 0, 1);
   pValuesLayout->addWidget(pDeleteValuesButton, 1, 1, Qt::AlignTop);
   pValuesLayout->setRowStretch(1, 10);
   pValuesLayout->setColumnStretch(0, 10);
   LabeledSection* pValuesSection = new LabeledSection(pValuesWidget, "Individual Values", this);

   // create the range widgets
   QWidget* pRangesWidget = new QWidget(this);
   QGridLayout* pRangesLayout = new QGridLayout(pRangesWidget);
   pRangesLayout->setMargin(0);
   pRangesLayout->setSpacing(5);
   mpRanges = new QTreeWidget(pRangesWidget);
   mpRanges->setToolTip("Any value equal to or greater than the start value and\nless than or equal to the end "
      "value will be\nconsidered a bad value.");
   QStringList columnNames;
   columnNames << "Start" << "End";
   mpRanges->setHeaderLabels(columnNames);
   mpRanges->setRootIsDecorated(false);
   mpRanges->setItemDelegate(new BadValueItemDelegate(mpRanges));
   mpRanges->setSelectionBehavior(QAbstractItemView::SelectRows);
   mpRanges->setSelectionMode(QAbstractItemView::ExtendedSelection);
   QPushButton* pAddRangeButton = new QPushButton("Add", pRangesWidget);
   QPushButton* pDeleteRangesButton = new QPushButton("Delete", pRangesWidget);
   pRangesLayout->addWidget(mpRanges, 0, 0, 2, 1);
   pRangesLayout->addWidget(pAddRangeButton, 0, 1);
   pRangesLayout->addWidget(pDeleteRangesButton, 1, 1, Qt::AlignTop);
   pRangesLayout->setRowStretch(1, 10);
   pRangesLayout->setColumnStretch(0, 10);
   LabeledSection* pRangesSection = new LabeledSection(pRangesWidget, "Ranges", this);

   // create tolerance widgets
   QWidget* pToleranceWidget = new QWidget(this);
   QVBoxLayout* pToleranceLayout = new QVBoxLayout(pToleranceWidget);
   pToleranceLayout->setMargin(0);
   pToleranceLayout->setSpacing(5);
   mpTolerance = new QLineEdit(pToleranceWidget);
   mpTolerance->setValidator(new QDoubleValidator(mpTolerance));
   mpTolerance->setToolTip("This value is the minimum difference needed between\ntwo floating point values to "
      "consider them not equivalent.\ne.g. if tolerance = 0.001, then 1.000 & 1.0009 are considered equivalent,\n"
      "but if tolerance is 0.0001, then they are not equivalent.");
   pToleranceLayout->addWidget(mpTolerance);
   LabeledSection* pToleranceSection = new LabeledSection(pToleranceWidget, "Comparison Tolerance", this);

   // create and populate labeled section group
   LabeledSectionGroup* pSectionGroup = new LabeledSectionGroup(this);
   pSectionGroup->addSection(pThresholdSection);
   pSectionGroup->addSection(pValuesSection, 1000);
   pSectionGroup->addSection(pRangesSection, 1000);
   pSectionGroup->addSection(pToleranceSection);

   // create optionally displayed SetAll widgets if enabled
   if (enableSetAll)
   {
      QWidget* pSetAllWidget = new QWidget(this);
      QVBoxLayout* pSetAllLayout = new QVBoxLayout(pSetAllWidget);
      pSetAllLayout->setMargin(0);
      pSetAllLayout->setSpacing(5);
      mpSetAll = new QCheckBox("Set Bad Values for all Bands", pSetAllWidget);
      mpSetAll->setChecked(false);
      pSetAllLayout->addWidget(mpSetAll, 10);
      LabeledSection* pSetAllSection = new LabeledSection(pSetAllWidget, "Options", this);
      pSectionGroup->addSection(pSetAllSection);
   }

   pSectionGroup->addStretch(1);

   //create a horizontal, sunken line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::Sunken | QFrame::HLine);

   // create dialog button bar
   QDialogButtonBox* pButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, this);

   // add widgets to the dialog layout
   pDialogLayout->addWidget(pSectionGroup, 10);
   pDialogLayout->addWidget(pLine);
   pDialogLayout->addWidget(pButtons);

   setWindowTitle("Bad Values" + (statName.isEmpty() ? "" : " - " + statName));
   resize(320, 450);

   //connect the button clicks to the appropriate signals
   VERIFYNR(connect(mpEnableLowerThreshold, SIGNAL(toggled(bool)), mpLowerThreshold, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpEnableUpperThreshold, SIGNAL(toggled(bool)), mpUpperThreshold, SLOT(setEnabled(bool))));
   VERIFYNR(connect(pAddValueButton, SIGNAL(clicked()), this, SLOT(addBadValue())));
   VERIFYNR(connect(pDeleteValuesButton, SIGNAL(clicked()), this, SLOT(deleteBadValue())));
   VERIFYNR(connect(pAddRangeButton, SIGNAL(clicked()), this, SLOT(addRange())));
   VERIFYNR(connect(pDeleteRangesButton, SIGNAL(clicked()), this, SLOT(deleteRange())));
   VERIFYNR(connect(pButtons, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtons, SIGNAL(rejected()), this, SLOT(reject())));
}

BadValuesDlg::~BadValuesDlg()
{}

void BadValuesDlg::setBadValues(const BadValues* pBadValues)
{
   mBadValues.setBadValues(pBadValues);
   initialize();
}

const BadValues* BadValuesDlg::getBadValues() const
{
   return &mBadValues;
}

std::string BadValuesDlg::getBadValuesString() const
{
   return mBadValues.getBadValuesString();
}

bool BadValuesDlg::getSetAll() const
{
   return (mpSetAll == NULL ? false : mpSetAll->isChecked());
}

void BadValuesDlg::accept()
{
   bool success(false);
   QString badValuesStr;
   QString separator(", ");
   if (mpEnableLowerThreshold->isChecked())
   {
      double value = mpLowerThreshold->text().toDouble(&success);
      if (!success)
      {
         QMessageBox::warning(this, "Bad Values", "Invalid entry for the lower threshold.");
         return;
      }
      badValuesStr += "<" + mpLowerThreshold->text();
   }

   for (int index = 0; index < mpValues->topLevelItemCount(); ++index)
   {
      QTreeWidgetItem* pItem = mpValues->topLevelItem(index);
      if (pItem == NULL)
      {
         continue;
      }
      double value = pItem->text(0).toDouble(&success);
      if (!success)
      {
         QMessageBox::warning(this, "Bad Values", "Invalid entry for an individual bad value.");
         mpValues->clearSelection();
         pItem->setSelected(true);
         return;
      }
      badValuesStr += (badValuesStr.isEmpty() ? "" : separator) + pItem->text(0);
   }

   for (int index = 0; index < mpRanges->topLevelItemCount(); ++index)
   {
      QTreeWidgetItem* pItem = mpRanges->topLevelItem(index);
      if (pItem == NULL)
      {
         continue;
      }
      double start = pItem->text(0).toDouble(&success);
      if (!success)
      {
         QMessageBox::warning(this, "Bad Values", "Invalid entry for the start of a range.");
         mpRanges->clearSelection();
         pItem->setSelected(true);
         return;
      }
      double end = pItem->text(1).toDouble(&success);
      if (!success)
      {
         QMessageBox::warning(this, "Bad Values", "Invalid entry for the end of a range.");
         mpRanges->clearSelection();
         pItem->setSelected(true);
         return;
      }
      if (start > end)
      {
         QMessageBox::warning(this, "Bad Values", "Invalid range - the start is greater than the end.");
         mpRanges->clearSelection();
         pItem->setSelected(true);
         return;
      }
      badValuesStr += (badValuesStr.isEmpty() ? "" : separator) +
         pItem->text(0) + "<>" + pItem->text(1);
   }

   if (mpEnableUpperThreshold->isChecked())
   {
      double value = mpUpperThreshold->text().toDouble(&success);
      if (!success)
      {
         QMessageBox::warning(this, "Bad Values", "Invalid entry for the upper threshold.");
         return;
      }
      badValuesStr += (badValuesStr.isEmpty() ? "" : separator) + ">" + mpUpperThreshold->text();
   }

   mBadValues.startBadValuesUpdate();
   mBadValues.setBadValues(badValuesStr.toStdString(), mpTolerance->text().toStdString());
   mBadValues.endBadValuesUpdate();
   QDialog::accept();
}

void BadValuesDlg::initialize()
{
   std::string threshold = mBadValues.getUpperBadValueThreshold();
   if (threshold.empty() == false)
   {
      mpUpperThreshold->setText(QString::fromStdString(threshold));
      mpEnableUpperThreshold->setChecked(true);
   }
   else
   {
      mpEnableUpperThreshold->setChecked(false);
   }

   threshold = mBadValues.getLowerBadValueThreshold();
   if (threshold.empty() == false)
   {
      mpLowerThreshold->setText(QString::fromStdString(threshold));
      mpEnableLowerThreshold->setChecked(true);
   }
   else
   {
      mpEnableLowerThreshold->setChecked(false);
   }

   mpValues->clear();
   std::vector<std::string> values = mBadValues.getIndividualBadValues();
   for (std::vector<std::string>::const_iterator iter = values.begin(); iter != values.end(); ++iter)
   {
      QTreeWidgetItem* pItem = new QTreeWidgetItem();
      pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
      pItem->setText(0, QString::fromStdString(*iter));
      mpValues->addTopLevelItem(pItem);
   }

   mpRanges->clear();
   std::vector<std::pair<std::string, std::string> > ranges = mBadValues.getBadValueRanges();
   for (std::vector<std::pair<std::string, std::string> >::const_iterator iter = ranges.begin();
      iter != ranges.end(); ++iter)
   {
      QTreeWidgetItem* pItem = new QTreeWidgetItem();
      pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
      pItem->setText(0, QString::fromStdString(iter->first));
      pItem->setText(1, QString::fromStdString(iter->second));
      mpRanges->addTopLevelItem(pItem);
   }

   mpTolerance->setText(QString::fromStdString(mBadValues.getBadValueTolerance()));
}

void BadValuesDlg::addBadValue()
{
   BadValueInputDlg dlg(this);
   if (dlg.exec() == QDialog::Accepted)
   {
      QString valStr = dlg.getBadValue();
      QList<QTreeWidgetItem*> items = mpValues->findItems(valStr, Qt::MatchExactly);
      if (items.isEmpty())
      {
         QTreeWidgetItem* pItem = new QTreeWidgetItem();
         pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
         pItem->setText(0, valStr);
         mpValues->addTopLevelItem(pItem);
      }
      else
      {
         QMessageBox::warning(this, "Add Bad Value", "The value " + valStr + " is already in the bad values list.");
      }
   }
}

void BadValuesDlg::deleteBadValue()
{
   QList<QTreeWidgetItem*> items = mpValues->selectedItems();
   QListIterator<QTreeWidgetItem*> it(items);
   while (it.hasNext())
   {
      QTreeWidgetItem* pItem = it.next();
      if (pItem != NULL)
      {
         delete pItem;
      }
   }
}

void BadValuesDlg::addRange()
{
   RangeInputDlg dlg(this);
   if (dlg.exec() == QDialog::Accepted)
   {
      QString rangeStart = dlg.getRangeStart();
      QString rangeEnd = dlg.getRangeEnd();
      for (int index = 0; index < mpRanges->topLevelItemCount(); ++index)
      {
         QTreeWidgetItem* pItem = mpRanges->topLevelItem(index);
         if (pItem == NULL)
         {
            continue;
         }
         if (pItem->text(0) == rangeStart && pItem->text(1) == rangeEnd)
         {
            QMessageBox::warning(this, "Add Bad Value Range",
               "The range " + rangeStart + "<>" + rangeEnd + " is already in the bad values list.");
            return;
         }
      }
      QTreeWidgetItem* pItem = new QTreeWidgetItem();
      pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
      pItem->setText(0, rangeStart);
      pItem->setText(1, rangeEnd);
      mpRanges->addTopLevelItem(pItem);
   }
}

void BadValuesDlg::deleteRange()
{
   QList<QTreeWidgetItem*> items = mpRanges->selectedItems();
   QListIterator<QTreeWidgetItem*> it(items);
   while (it.hasNext())
   {
      QTreeWidgetItem* pItem = it.next();
      if (pItem != NULL)
      {
         delete pItem;
      }
   }
}

//=======================================================
// Bad value input dialog
//=======================================================

// Use Qt::WindowsFlag WindowCloseButtonHint to suppress displaying the "What's This" button in dialog
BadValueInputDlg::BadValueInputDlg(QWidget* pParent) :
   QDialog(pParent, Qt::WindowCloseButtonHint)
{
   setWindowTitle("Add Bad Value");
   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);

   QLabel* pLabel = new QLabel("Bad Value:", this);
   mpBadValue = new QLineEdit(this);
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, this);
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::Sunken | QFrame::HLine);
   QRegExp numericExp;
   numericExp.setPattern("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$");
   QRegExpValidator* pValidator = new QRegExpValidator(numericExp, this);
   mpBadValue->setValidator(pValidator);

   pLayout->addWidget(pLabel, 0, 0);
   pLayout->addWidget(mpBadValue, 0, 1);
   pLayout->addWidget(pLine, 1, 0, 1, 2, Qt::AlignBottom);
   pLayout->addWidget(pButtonBox, 2, 0, 1, 2);
   pLayout->setRowStretch(1, 10);
   pLayout->setColumnStretch(1, 10);

   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
}

BadValueInputDlg::~BadValueInputDlg()
{}

QString BadValueInputDlg::getBadValue() const
{
   return mpBadValue->text();
}

void BadValueInputDlg::accept()
{
   bool success(false);
   QString valueStr = getBadValue();
   if (valueStr.isEmpty())
   {
      QMessageBox::warning(this, "Add Bad Value", "No value has been entered.");
      return;
   }
   double value = valueStr.toDouble(&success);
   if (!success)
   {
      QMessageBox::warning(this, "Add Bad Value", "The entry must be a valid numeric string.");
      return;
   }

   QDialog::accept();
}

//=======================================================
// Bad value range input dialog
//=======================================================

// Use Qt::WindowsFlag WindowCloseButtonHint to suppress displaying the "What's This" button in dialog
RangeInputDlg::RangeInputDlg(QWidget* pParent) :
   QDialog(pParent, Qt::WindowCloseButtonHint)
{
   setWindowTitle("Add Bad Value Range");
   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);

   QLabel* pStartLabel = new QLabel("Start:", this);
   mpRangeStart = new QLineEdit(this);
   QLabel* pEndLabel = new QLabel("End:", this);
   mpRangeEnd = new QLineEdit(this);
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, this);
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::Sunken | QFrame::HLine);
   QRegExp numericExp;
   numericExp.setPattern("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$");
   QRegExpValidator* pValidator = new QRegExpValidator(numericExp, this);
   mpRangeStart->setValidator(pValidator);
   mpRangeEnd->setValidator(pValidator);

   pLayout->addWidget(pStartLabel, 0, 0);
   pLayout->addWidget(mpRangeStart, 0, 1);
   pLayout->addWidget(pEndLabel, 1, 0);
   pLayout->addWidget(mpRangeEnd, 1, 1);
   pLayout->addWidget(pLine, 2, 0, 1, 2, Qt::AlignBottom);
   pLayout->addWidget(pButtonBox, 3, 0, 1, 2);
   pLayout->setRowStretch(2, 10);
   pLayout->setColumnStretch(1, 10);

   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
}

RangeInputDlg::~RangeInputDlg()
{}

QString RangeInputDlg::getRangeStart() const
{
   return mpRangeStart->text();
}

QString RangeInputDlg::getRangeEnd() const
{
   return mpRangeEnd->text();
}

void RangeInputDlg::accept()
{
   bool success(false);
   QString startValueStr = getRangeStart();
   if (startValueStr.isEmpty())
   {
      QMessageBox::warning(this, "Add Bad Value Range", "No value has been entered for the start of the range.");
      return;
   }
   double startValue = startValueStr.toDouble(&success);
   if (!success)
   {
      QMessageBox::warning(this, "Add Bad Value Range", "The entry for the start of the range must be a valid "
         "numeric string.");
      return;
   }

   QString endValueStr = getRangeEnd();
   if (endValueStr.isEmpty())
   {
      QMessageBox::warning(this, "Add Bad Value Range", "No value has been entered for the end of the range.");
      return;
   }
   double endValue = endValueStr.toDouble(&success);
   if (!success)
   {
      QMessageBox::warning(this, "Add Bad Value Range", "The entry for the end of the range must be a valid "
         "numeric string.");
      return;
   }
   if (startValue > endValue)
   {
      QMessageBox::warning(this, "Add Bad Value Range", "The entry for the start of the range must be less "
         "than the value for the end of the range.");
      return;
   }

   if (startValueStr == endValueStr)
   {
      QMessageBox::warning(this, "Add Bad Value Range", "The start and end of the range are the same value. "
         "Enter this value as an individual bad value instead of a range.");
      return;
   }

   QDialog::accept();
}

BadValueItemDelegate::BadValueItemDelegate(QObject* pParent) :
   QStyledItemDelegate(pParent)
{}

BadValueItemDelegate::~BadValueItemDelegate()
{}

QWidget* BadValueItemDelegate::createEditor(QWidget* pParent, const QStyleOptionViewItem& option,
   const QModelIndex& index) const
{
   if (index.isValid() == false)
   {
      return NULL;
   }

   QLineEdit* pBadValueEdit = new QLineEdit(pParent);
   pBadValueEdit->setFrame(false);
   pBadValueEdit->setValidator(new QDoubleValidator(pBadValueEdit));

   return pBadValueEdit;
}

void BadValueItemDelegate::setEditorData(QWidget* pEditor, const QModelIndex& index) const
{
   QLineEdit* pBadValueEdit = dynamic_cast<QLineEdit*>(pEditor);
   if (pBadValueEdit == NULL)
   {
      return;
   }

   QString badValueText = index.model()->data(index, Qt::EditRole).toString();
   pBadValueEdit->setText(badValueText);
}

void BadValueItemDelegate::setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const
{
   if (pModel == NULL)
   {
      return;
   }

   QLineEdit* pBadValueEdit = dynamic_cast<QLineEdit*>(pEditor);
   if (pBadValueEdit == NULL)
   {
      return;
   }

   QString badValueText = pBadValueEdit->text();
   pModel->setData(index, QVariant(badValueText), Qt::EditRole);
}

void BadValueItemDelegate::updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option,
   const QModelIndex& index) const
{
   if (pEditor != NULL)
   {
      pEditor->setGeometry(option.rect);
   }
}
