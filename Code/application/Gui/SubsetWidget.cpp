/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QTextStream>
#include <QtGui/QApplication>
#include <QtGui/QCompleter>
#include <QtGui/QFileDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>

#include "AppAssert.h"
#include "AppVerify.h"
#include "DimensionDescriptor.h"
#include "Icons.h"
#include "RasterUtilities.h"
#include "SubsetWidget.h"

using namespace std;

SubsetWidget::SubsetWidget(QWidget* pParent) :
   QWidget(pParent),
   mExportMode(false),
   mpStartRowCombo(NULL),
   mpEndRowCombo(NULL),
   mpRowSkipSpin(NULL),
   mpStartColumnCombo(NULL),
   mpEndColumnCombo(NULL),
   mpColumnSkipSpin(NULL),
   mpBandList(NULL)
{
   QFont boldFont = QApplication::font();
   boldFont.setBold(true);

   // Rows
   QLabel* pRowLabel = new QLabel("Rows", this);
   pRowLabel->setFont(boldFont);

   QFrame* pRowLine = new QFrame(this);
   pRowLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QLabel* pStartRowLabel = new QLabel("Start Row:", this);
   mpStartRowCombo = new QComboBox(this);
   mpStartRowCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
   mpRowModel = new QStringListModel(this);
   mpStartRowCombo->setModel(mpRowModel);

   QLabel* pEndRowLabel = new QLabel("End Row:", this);
   mpEndRowCombo = new QComboBox(this);
   mpEndRowCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
   mpEndRowCombo->setModel(mpRowModel);

   QLabel* pRowSkipLabel = new QLabel("Skip Factor:", this);
   mpRowSkipSpin = new QSpinBox(this);

   // Columns
   QLabel* pColumnLabel = new QLabel("Columns", this);
   pColumnLabel->setFont(boldFont);

   QFrame* pColumnLine = new QFrame(this);
   pColumnLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QLabel* pStartColumnLabel = new QLabel("Start Column:", this);
   mpStartColumnCombo = new QComboBox(this);
   mpStartColumnCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
   mpColumnModel = new QStringListModel(this);
   mpStartColumnCombo->setModel(mpColumnModel);

   QLabel* pEndColumnLabel = new QLabel("End Column:", this);
   mpEndColumnCombo = new QComboBox(this);
   mpEndColumnCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
   mpEndColumnCombo->setModel(mpColumnModel);

   QLabel* pColumnSkipLabel = new QLabel("Skip Factor:", this);
   mpColumnSkipSpin = new QSpinBox(this);

   // Bands
   QLabel* pBandLabel = new QLabel("Bands", this);
   pBandLabel->setFont(boldFont);

   QFrame* pBandLine = new QFrame(this);
   pBandLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   mpBandList = new QListView(this);
   mpBandList->setViewMode(QListView::ListMode);
   mpBandList->setResizeMode(QListView::Adjust);
   mpBandList->setWrapping(true);
   mpBandList->setFlow(QListView::TopToBottom);
   mpBandList->setMovement(QListView::Static);
   mpBandList->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpBandModel = new QStringListModel(this);
   mpBandList->setModel(mpBandModel);

   QPushButton* pCustomBandSelect = new QPushButton("Custom Selection...", this);

   // Layout
   QHBoxLayout* pRowsLayout = new QHBoxLayout();
   pRowsLayout->setMargin(0);
   pRowsLayout->setSpacing(5);
   pRowsLayout->addWidget(pRowLabel);
   pRowsLayout->addWidget(pRowLine, 10);

   QHBoxLayout* pColumnsLayout = new QHBoxLayout();
   pColumnsLayout->setMargin(0);
   pColumnsLayout->setSpacing(5);
   pColumnsLayout->addWidget(pColumnLabel);
   pColumnsLayout->addWidget(pColumnLine, 10);

   QHBoxLayout* pBandsLayout = new QHBoxLayout();
   pBandsLayout->setMargin(0);
   pBandsLayout->setSpacing(5);
   pBandsLayout->addWidget(pBandLabel);
   pBandsLayout->addWidget(pBandLine, 10);

   QGridLayout* pSubsetLayout = new QGridLayout(this);
   pSubsetLayout->setMargin(0);
   pSubsetLayout->setSpacing(10);
   pSubsetLayout->addLayout(pRowsLayout, 0, 0, 1, 3);
   pSubsetLayout->addWidget(pStartRowLabel, 1, 0);
   pSubsetLayout->addWidget(mpStartRowCombo, 1, 1);
   pSubsetLayout->addWidget(pEndRowLabel, 2, 0);
   pSubsetLayout->addWidget(mpEndRowCombo, 2, 1);
   pSubsetLayout->addWidget(pRowSkipLabel, 3, 0);
   pSubsetLayout->addWidget(mpRowSkipSpin, 3, 1);
   pSubsetLayout->addLayout(pColumnsLayout, 0, 3, 1, 5);
   pSubsetLayout->addWidget(pStartColumnLabel, 1, 3);
   pSubsetLayout->addWidget(mpStartColumnCombo, 1, 4);
   pSubsetLayout->addWidget(pEndColumnLabel, 2, 3);
   pSubsetLayout->addWidget(mpEndColumnCombo, 2, 4);
   pSubsetLayout->addWidget(pColumnSkipLabel, 3, 3);
   pSubsetLayout->addWidget(mpColumnSkipSpin, 3, 4);
   pSubsetLayout->setRowMinimumHeight(4, 15);
   pSubsetLayout->addLayout(pBandsLayout, 5, 0, 1, 8);
   pSubsetLayout->addWidget(mpBandList, 6, 0, 1, 8);
   pSubsetLayout->addWidget(pCustomBandSelect, 7, 0, 1, 8, Qt::AlignRight);
   pSubsetLayout->setRowStretch(6, 10);
   pSubsetLayout->setColumnStretch(2, 10);
   pSubsetLayout->setColumnStretch(5, 10);

   // Connections
   VERIFYNR(connect(mpStartRowCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfRowsChange())));
   VERIFYNR(connect(mpEndRowCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfRowsChange())));
   VERIFYNR(connect(mpRowSkipSpin, SIGNAL(valueChanged(int)), this, SLOT(notifyOfRowsChange())));
   VERIFYNR(connect(mpStartColumnCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfColumnsChange())));
   VERIFYNR(connect(mpEndColumnCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfColumnsChange())));
   VERIFYNR(connect(mpColumnSkipSpin, SIGNAL(valueChanged(int)), this, SLOT(notifyOfColumnsChange())));
   VERIFYNR(connect(mpBandList->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(notifyOfBandsChange())));
   VERIFYNR(connect(pCustomBandSelect, SIGNAL(clicked()), this, SLOT(customBandSelection())));
}

SubsetWidget::~SubsetWidget()
{}

void SubsetWidget::setExportMode(bool enableExportMode)
{
   mExportMode = enableExportMode;
}

void SubsetWidget::setRows(const vector<DimensionDescriptor>& rows, const vector<DimensionDescriptor>& selectedRows)
{
   if ((rows == mRows) && (selectedRows == getSubsetRows()))
   {
      return;
   }

   disconnect(mpStartRowCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfRowsChange()));
   disconnect(mpEndRowCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfRowsChange()));
   disconnect(mpRowSkipSpin, SIGNAL(valueChanged(int)), this, SLOT(notifyOfRowsChange()));

   mRows = rows;

   DimensionDescriptor startSelectRow;
   DimensionDescriptor stopSelectRow;

   if (selectedRows.empty() == false)
   {
      startSelectRow = selectedRows.front();
      stopSelectRow = selectedRows.back();
   }
   else if (mRows.empty() == false)
   {
      startSelectRow = mRows.front();
      stopSelectRow = mRows.back();
   }

   unsigned int startRowIndex = 0;
   unsigned int stopRowIndex = mRows.size() - 1;

   QStringList lstRowNames;
   for (unsigned int i = 0; i < mRows.size(); ++i)
   {
      DimensionDescriptor rowDim = mRows[i];
      QString name;
      if (rowDim.isOriginalNumberValid())
      {
         unsigned int originalNumber = rowDim.getOriginalNumber() + 1;
         name.append(QString::number(originalNumber));
      }
      if (compareDimensionDescriptors(rowDim, startSelectRow))
      {
         startRowIndex = i;
      }
      if (compareDimensionDescriptors(rowDim, stopSelectRow))
      {
         stopRowIndex = i;
      }
      lstRowNames.append(name);
   }
   mpRowModel->setStringList(lstRowNames);

   unsigned int skipFactor = 0;
   if (selectedRows.empty() == false)
   {
      if (mExportMode == false)
      {
         RasterUtilities::determineSkipFactor(selectedRows, skipFactor);
      }
      else
      {
         RasterUtilities::determineExportSkipFactor(selectedRows, skipFactor);
      }
   }
   else if (mRows.empty() == false)
   {
      if (mExportMode == false)
      {
         RasterUtilities::determineSkipFactor(mRows, skipFactor);
      }
      else
      {
         RasterUtilities::determineExportSkipFactor(mRows, skipFactor);
      }
   }

   mpStartRowCombo->setCurrentIndex(startRowIndex);
   mpEndRowCombo->setCurrentIndex(stopRowIndex);

   mpRowSkipSpin->setMinimum(0);
   mpRowSkipSpin->setMaximum(mRows.size() - 1);
   mpRowSkipSpin->setSingleStep(1);
   mpRowSkipSpin->setValue(skipFactor);

   VERIFYNR(connect(mpStartRowCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfRowsChange())));
   VERIFYNR(connect(mpEndRowCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfRowsChange())));
   VERIFYNR(connect(mpRowSkipSpin, SIGNAL(valueChanged(int)), this, SLOT(notifyOfRowsChange())));

   emit subsetRowsChanged(mRows);
   emit modified();
}

const vector<DimensionDescriptor>& SubsetWidget::getRows() const
{
   return mRows;
}

vector<DimensionDescriptor> SubsetWidget::getSubsetRows() const
{
   if (mRows.empty() == true)
   {
      return vector<DimensionDescriptor>();
   }

   unsigned int startRow = mpStartRowCombo->currentIndex();
   unsigned int endRow = mpEndRowCombo->currentIndex();
   unsigned int rowSkip = mpRowSkipSpin->value();

   VERIFYRV(startRow < mRows.size(), vector<DimensionDescriptor>());
   VERIFYRV(endRow < mRows.size(), vector<DimensionDescriptor>());

   DimensionDescriptor startRowDim = mRows[startRow];
   DimensionDescriptor endRowDim = mRows[endRow];
   vector<DimensionDescriptor> subsetRows =
      RasterUtilities::subsetDimensionVector(mRows, startRowDim, endRowDim, rowSkip);
   return subsetRows;
}

unsigned int SubsetWidget::getSubsetRowCount() const
{
   vector<DimensionDescriptor> subsetRows = getSubsetRows();
   return subsetRows.size();
}

void SubsetWidget::setColumns(const vector<DimensionDescriptor>& columns,
                              const vector<DimensionDescriptor>& selectedColumns)
{
   if ((columns == mColumns) && (selectedColumns == getSubsetColumns()))
   {
      return;
   }

   mColumns = columns;

   disconnect(mpStartColumnCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfColumnsChange()));
   disconnect(mpEndColumnCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfColumnsChange()));
   disconnect(mpColumnSkipSpin, SIGNAL(valueChanged(int)), this, SLOT(notifyOfColumnsChange()));

   DimensionDescriptor startSelectCol;
   DimensionDescriptor stopSelectCol;

   if (selectedColumns.empty() == false)
   {
      startSelectCol = selectedColumns.front();
      stopSelectCol = selectedColumns.back();
   }
   else if (mColumns.empty() == false)
   {
      startSelectCol = mColumns.front();
      stopSelectCol = mColumns.back();
   }

   unsigned int startColumnIndex = 0;
   unsigned int stopColumnIndex = mColumns.size() - 1;

   QStringList lstColumnNames;
   for (unsigned int i = 0; i < mColumns.size(); ++i)
   {
      DimensionDescriptor colDim = mColumns[i];
      QString name;
      if (colDim.isOriginalNumberValid())
      {
         unsigned int originalNumber = colDim.getOriginalNumber() + 1;
         name.append(QString::number(originalNumber));
      }
      if (compareDimensionDescriptors(colDim, startSelectCol))
      {
         startColumnIndex = i;
      }
      if (compareDimensionDescriptors(colDim, stopSelectCol))
      {
         stopColumnIndex = i;
      }
      lstColumnNames.append(name);
   }
   mpColumnModel->setStringList(lstColumnNames);

   unsigned int skipFactor = 0;
   if (selectedColumns.empty() == false)
   {
      if (mExportMode == false)
      {
         RasterUtilities::determineSkipFactor(selectedColumns, skipFactor);
      }
      else
      {
         RasterUtilities::determineExportSkipFactor(selectedColumns, skipFactor);
      }
   }
   else if (mColumns.empty() == false)
   {
      if (mExportMode == false)
      {
         RasterUtilities::determineSkipFactor(mColumns, skipFactor);
      }
      else
      {
         RasterUtilities::determineExportSkipFactor(mColumns, skipFactor);
      }
   }

   mpStartColumnCombo->setCurrentIndex(startColumnIndex);
   mpEndColumnCombo->setCurrentIndex(stopColumnIndex);

   mpColumnSkipSpin->setMinimum(0);
   mpColumnSkipSpin->setMaximum(mColumns.size() - 1);
   mpColumnSkipSpin->setSingleStep(1);
   mpColumnSkipSpin->setValue(skipFactor);

   VERIFYNR(connect(mpStartColumnCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfColumnsChange())));
   VERIFYNR(connect(mpEndColumnCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyOfColumnsChange())));
   VERIFYNR(connect(mpColumnSkipSpin, SIGNAL(valueChanged(int)), this, SLOT(notifyOfColumnsChange())));

   emit subsetColumnsChanged(mColumns);
   emit modified();
}

const vector<DimensionDescriptor>& SubsetWidget::getColumns() const
{
   return mColumns;
}

vector<DimensionDescriptor> SubsetWidget::getSubsetColumns() const
{
   if (mColumns.empty() == true)
   {
      return vector<DimensionDescriptor>();
   }

   unsigned int startColumn = mpStartColumnCombo->currentIndex();
   unsigned int endColumn = mpEndColumnCombo->currentIndex();
   unsigned int columnSkip = mpColumnSkipSpin->value();

   VERIFYRV(startColumn < mColumns.size(), vector<DimensionDescriptor>());
   VERIFYRV(endColumn < mColumns.size(), vector<DimensionDescriptor>());

   DimensionDescriptor startColumnDim = mColumns[startColumn];
   DimensionDescriptor endColumnDim = mColumns[endColumn];
   vector<DimensionDescriptor> subsetColumns =
      RasterUtilities::subsetDimensionVector(mColumns, startColumnDim, endColumnDim, columnSkip);
   return subsetColumns;
}

unsigned int SubsetWidget::getSubsetColumnCount() const
{
   vector<DimensionDescriptor> subsetColumns = getSubsetColumns();
   return subsetColumns.size();
}

void SubsetWidget::setBands(const vector<DimensionDescriptor>& bands, const vector<string>& bandNames,
                            const vector<DimensionDescriptor>& selectedBands)
{
   if ((bands == mBands) && (selectedBands == getSubsetBands()))
   {
      return;
   }

   disconnect(mpBandList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(notifyOfBandsChange()));

   mBands = bands;

   QStringList lstBandNames;
   for (unsigned int i = 0; i < mBands.size(); ++i)
   {
      DimensionDescriptor bandDim = mBands[i];
      QString name;
      if ((i < bandNames.size()) && (bands.size() == bandNames.size()))
      {
         name = QString::fromStdString(bandNames[i]);
      }
      else
      {
         name = "Band ";
         if (bandDim.isOriginalNumberValid())
         {
            unsigned int originalNumber = bandDim.getOriginalNumber() + 1;
            name.append(QString::number(originalNumber));
         }
      }
      lstBandNames.append(name);
   }
   mpBandModel->setStringList(lstBandNames);

   if (selectedBands.empty() || (selectedBands.size() == bands.size()))
   {
      mpBandList->selectAll();
   }
   else
   {
      QItemSelection totalSelection;
      bool inRange = false;
      int rangeStart;
      int rangeEnd;
      for (unsigned int allBandIndex = 0, selectedBandIndex = 0;
         (allBandIndex < mBands.size()) && (selectedBandIndex < selectedBands.size());
         ++allBandIndex)
      {
         if (inRange)
         {
            if (compareDimensionDescriptors(mBands[allBandIndex], selectedBands[selectedBandIndex]))
            {
               rangeEnd = allBandIndex;
               selectedBandIndex++;
            }
            else
            {
               totalSelection.select(mpBandModel->index(rangeStart), mpBandModel->index(rangeEnd));
               inRange = false;
            }
         }
         else
         {
            if (compareDimensionDescriptors(mBands[allBandIndex], selectedBands[selectedBandIndex]))
            {
               rangeStart = allBandIndex;
               rangeEnd = allBandIndex;
               inRange = true;
               selectedBandIndex++;
            }
         }
      }
      if (inRange)
      {
         totalSelection.select(mpBandModel->index(rangeStart), mpBandModel->index(rangeEnd));
      }

      mpBandList->selectionModel()->select(totalSelection, QItemSelectionModel::Select);
   }

   connect(mpBandList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(notifyOfBandsChange()));

   emit subsetBandsChanged(mBands);
   emit modified();
}

const vector<DimensionDescriptor>& SubsetWidget::getBands() const
{
   return mBands;
}

class QModelIndexCompare
{
public:
   bool operator()(const QModelIndex& val1, const QModelIndex& val2)
   {
      return val1.row() < val2.row();
   }
};

vector<DimensionDescriptor> SubsetWidget::getSubsetBands() const
{
   vector<DimensionDescriptor> subsetBands;
   QModelIndexList selectedIndexes = mpBandList->selectionModel()->selectedIndexes();
   qSort(selectedIndexes.begin(), selectedIndexes.end(), QModelIndexCompare());
   unsigned int selectedBands = selectedIndexes.size();
   unsigned int curBandIndex;
   for (unsigned int curBand = 0; curBand < selectedBands; ++curBand)
   {
      curBandIndex = selectedIndexes[curBand].row();
      if (curBandIndex < mBands.size())
      {
         DimensionDescriptor bandDim = mBands[curBandIndex];
         if (bandDim.isValid())
         {
            subsetBands.push_back(bandDim);
         }
      }
   }

   return subsetBands;
}

unsigned int SubsetWidget::getSubsetBandCount() const
{
   return mpBandList->selectionModel()->selectedIndexes().size();
}

void SubsetWidget::setBadBandFileDirectory(const QString& strDirectory)
{
   mBadBandFileDir = strDirectory;
}

QSize SubsetWidget::sizeHint() const
{
   return QSize(450, 300);
}

void SubsetWidget::customBandSelection()
{
   BandCustomSelectionDialog dialog(this, mpBandModel, mBadBandFileDir);
   if (dialog.exec() == QDialog::Rejected)
   {
      return;
   }

   if (dialog.isSubsetSelected())
   {
      int startBand = dialog.getStartBandIndex();
      if ((startBand < 0) || (startBand >= static_cast<int>(mBands.size())))
      {
         return;
      }
      int endBand = dialog.getStopBandIndex();
      if ((endBand < 0) || (endBand >= static_cast<int>(mBands.size())))
      {
         return;
      }
      int bandSkip = dialog.getSkipFactor();

      disconnect(mpBandList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
         this, SLOT(notifyOfBandsChange()));

      mpBandList->selectionModel()->clear();

      QItemSelection totalSelection;
      if (bandSkip == 0)
      {
         QModelIndex startBandToSelect = mpBandModel->index(startBand);
         QModelIndex endBandToSelect = mpBandModel->index(endBand);
         totalSelection.select(startBandToSelect, endBandToSelect);
      }
      else
      {
         int totalBands = mpBandModel->rowCount();
         for (int i = startBand; i < endBand; ++i)
         {
            QModelIndex bandToSelect = mpBandModel->index(i);
            if (bandToSelect.isValid())
            {
               totalSelection.select(bandToSelect, bandToSelect);
            }

            i += bandSkip;
         }
      }
      mpBandList->selectionModel()->select(totalSelection, QItemSelectionModel::Select);

      connect(mpBandList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
         this, SLOT(notifyOfBandsChange()));
      notifyOfBandsChange();
   }
   else
   {
      vector<unsigned int> badBands;
      QFile file(dialog.getBadBandFile());
      file.open(QIODevice::ReadOnly);

      QTextStream ts(&file);

      QString strBand = ts.readLine();
      while (strBand.isNull() == false)
      {
         if (strBand.isEmpty() == false)
         {
            bool bSuccess = false;

            unsigned int badBand = strBand.toUInt(&bSuccess);
            if (bSuccess == true)
            {
               for (unsigned int i = 0; i < mBands.size(); ++i)
               {
                  DimensionDescriptor bandDim = mBands[i];
                  if (bandDim.isValid())
                  {
                     unsigned int originalNumber = bandDim.getOriginalNumber() + 1;
                     if (originalNumber == badBand)
                     {
                        badBands.push_back(bandDim.getOnDiskNumber());
                        break;
                     }
                  }
               }
            }
         }

         strBand = ts.readLine();
      }

      file.close();

      mpBandList->selectAll();
      for (unsigned int i = 0; i < badBands.size(); ++i)
      {
         QModelIndex bandToDeselect = mpBandModel->index(static_cast<int>(badBands[i]));
         if (bandToDeselect.isValid())
         {
            mpBandList->selectionModel()->select(bandToDeselect, QItemSelectionModel::Deselect);
         }
      }
   }
}

void SubsetWidget::notifyOfRowsChange()
{
   vector<DimensionDescriptor> subsetRows = getSubsetRows();

   emit subsetRowsChanged(subsetRows);
   emit modified();
}

void SubsetWidget::notifyOfColumnsChange()
{
   vector<DimensionDescriptor> subsetColumns = getSubsetColumns();

   emit subsetColumnsChanged(subsetColumns);
   emit modified();
}

void SubsetWidget::notifyOfBandsChange()
{
   vector<DimensionDescriptor> subsetBands = getSubsetBands();

   emit subsetBandsChanged(subsetBands);
   emit modified();
}

bool SubsetWidget::compareDimensionDescriptors(DimensionDescriptor left, DimensionDescriptor right)
{
   if (mExportMode)
   {
      if (left.isActiveNumberValid() != right.isActiveNumberValid() ||
         left.isOriginalNumberValid() != right.isOriginalNumberValid())
      {
         return false;
      }

      return ((!left.isOriginalNumberValid() || (left.getOriginalNumber() == right.getOriginalNumber())) &&
         (!left.isActiveNumberValid() || (left.getActiveNumber() == right.getActiveNumber())));
   }
   else
   {
      return left == right;
   }
}

BandCustomSelectionDialog::BandCustomSelectionDialog(QWidget* pParent, QStringListModel* pBandList,
                                                     QString badBandsDir) :
   QDialog(pParent),
   mpBandListModel(pBandList),
   mBadBandDir(badBandsDir)
{
   setModal(true);
   setWindowTitle("Band Selection Options");
   resize(360, 250);

   QLabel* pStartBandLabel = new QLabel("Start Band:", this);
   mpStartBandCombo = new QComboBox(this);
   mpStartBandCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
   mpStartBandCombo->setModel(mpBandListModel);

   QLabel* pEndBandLabel = new QLabel("End Band:", this);
   mpEndBandCombo = new QComboBox(this);
   mpEndBandCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
   mpEndBandCombo->setModel(mpBandListModel);
   mpEndBandCombo->setCurrentIndex(mpEndBandCombo->count()-1);

   QLabel* pBandSkipLabel = new QLabel("Skip Factor:", this);
   mpBandSkipSpin = new QSpinBox(this);
   mpBandSkipSpin->setMinimum(0);
   mpBandSkipSpin->setMaximum(mpEndBandCombo->count() - 1);
   mpBandSkipSpin->setSingleStep(1);

   QLabel* pBadBandFileLabel = new QLabel("File:", this);
   mpBadBandFile = new QLineEdit(this);
   mpBadBandFile->setEnabled(false);
   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   mpBadBandFileBtn = new QPushButton(pIcons->mOpen, "", this); 
   mpBadBandFileBtn->setEnabled(false);

   mpSubsetSelection = new QRadioButton("Subset");
   mpSubsetSelection->setChecked(true);
   mpBadBandsSelection = new QRadioButton("Bad Bands File");

   QPushButton* pOk = new QPushButton("OK", this);
   pOk->setDefault(true);
   QPushButton* pCancel = new QPushButton("Cancel", this);

   QGridLayout* pDialogLayout = new QGridLayout(this);
   pDialogLayout->setMargin(10);
   pDialogLayout->setSpacing(10);
   pDialogLayout->addWidget(mpSubsetSelection, 0, 0, 1, 3);
   pDialogLayout->addWidget(pStartBandLabel, 1, 1);
   pDialogLayout->addWidget(mpStartBandCombo, 1, 2);
   pDialogLayout->addWidget(pEndBandLabel, 2, 1);
   pDialogLayout->addWidget(mpEndBandCombo, 2, 2);
   pDialogLayout->addWidget(pBandSkipLabel, 3, 1);
   pDialogLayout->addWidget(mpBandSkipSpin, 3, 2);
   pDialogLayout->setRowMinimumHeight(4, 10);
   pDialogLayout->addWidget(mpBadBandsSelection, 5, 0, 1, 3);
   pDialogLayout->addWidget(pBadBandFileLabel, 6, 1);
   QHBoxLayout* pBadBandLayout = new QHBoxLayout();
   pBadBandLayout->setMargin(0);
   pBadBandLayout->setSpacing(5);
   pBadBandLayout->addWidget(mpBadBandFile, 10);
   pBadBandLayout->addWidget(mpBadBandFileBtn);
   pDialogLayout->addLayout(pBadBandLayout, 6, 2);
   QFrame* pSeparator = new QFrame(this);
   pSeparator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   pDialogLayout->addWidget(pSeparator, 7, 0, 1, 3, Qt::AlignBottom);
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(pOk);
   pButtonLayout->addWidget(pCancel);
   pDialogLayout->addLayout(pButtonLayout, 8, 0, 1, 3);
   pDialogLayout->setRowStretch(7, 10);

   VERIFYNR(connect(pCancel, SIGNAL(clicked()), this, SLOT(reject())));
   VERIFYNR(connect(pOk, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(mpBadBandFileBtn, SIGNAL(clicked()), this, SLOT(selectBadBandFile())));
   VERIFYNR(connect(mpBadBandsSelection, SIGNAL(clicked()), this, SLOT(selectionChanged())));
   VERIFYNR(connect(mpSubsetSelection, SIGNAL(clicked()), this, SLOT(selectionChanged())));
}

void BandCustomSelectionDialog::selectBadBandFile()
{
   QString strFilename = QFileDialog::getOpenFileName(this, "Select Bad Bands File", mBadBandDir,
      "Text Files (*.txt);;All Files (*.*)");
   if (strFilename.isEmpty() == true)
   {
      return;
   }
   mpBadBandFile->setText(strFilename);
}

void BandCustomSelectionDialog::selectionChanged()
{
   QRadioButton* pButton = dynamic_cast<QRadioButton*>(sender());
   if (pButton == mpBadBandsSelection)
   {
      mpStartBandCombo->setEnabled(false);
      mpEndBandCombo->setEnabled(false);
      mpBandSkipSpin->setEnabled(false);
      mpBadBandFile->setEnabled(true);
      mpBadBandFileBtn->setEnabled(true);
   }
   else if (pButton == mpSubsetSelection)
   {
      mpStartBandCombo->setEnabled(true);
      mpEndBandCombo->setEnabled(true);
      mpBandSkipSpin->setEnabled(true);
      mpBadBandFile->setEnabled(false);
      mpBadBandFileBtn->setEnabled(false);
   }
}

bool BandCustomSelectionDialog::isSubsetSelected()
{
   return mpSubsetSelection->isChecked();
}

QString BandCustomSelectionDialog::getBadBandFile()
{
   return mpBadBandFile->text();
}

int BandCustomSelectionDialog::getStartBandIndex()
{
   return mpStartBandCombo->currentIndex();
}

int BandCustomSelectionDialog::getStopBandIndex()
{
   return mpEndBandCombo->currentIndex();
}

int BandCustomSelectionDialog::getSkipFactor()
{
   return mpBandSkipSpin->value();
}
