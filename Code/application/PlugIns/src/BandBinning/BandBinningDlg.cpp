/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFileDialog>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QTableView>
#include <QtGui/QToolButton>

#include "AppVerify.h"
#include "BandBinningDlg.h"
#include "BandBinningModel.h"
#include "BandBinningUtilities.h"
#include "ConfigurationSettings.h"
#include "Filename.h"
#include "RasterDataDescriptor.h"
#include "RasterUtilities.h"

#include <algorithm>
#include <limits>
#include <string>

BandBinningDlg::BandBinningDlg(const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands,
                               const RasterDataDescriptor* pDescriptor, QWidget* pParent) :
   QDialog(pParent),
   mpGroupedBandView(NULL),
   mpBandModel(NULL),
   mpDescriptor(pDescriptor),
   mOriginalGroupedBands(groupedBands)
{
   VERIFYNRV(mpDescriptor != NULL);
   VERIFYNRV(mpDescriptor->getBandCount() <= static_cast<unsigned int>(std::numeric_limits<int>::max()));

   setWindowTitle("Select Band Bins");
   mpGroupedBandView = new QTableView(this);
   mpGroupedBandView->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpGroupedBandView->setSelectionBehavior(QAbstractItemView::SelectRows);
   mpBandModel = new BandBinningModel(mpDescriptor, this);
   mpGroupedBandView->setModel(mpBandModel);
   QHeaderView* pVerticalHeader = mpGroupedBandView->verticalHeader();
   if (pVerticalHeader != NULL)
   {
      pVerticalHeader->hide();
   }

   QHeaderView* pHorizontalHeader = mpGroupedBandView->horizontalHeader();
   if (pHorizontalHeader != NULL)
   {
      pHorizontalHeader->setHighlightSections(false);
   }

   QToolButton* pOpenButton = new QToolButton(this);
   pOpenButton->setIcon(QIcon(":/icons/Open"));
   pOpenButton->setToolTip("Open Binning File");
   pOpenButton->setAutoRaise(true);

   QToolButton* pSaveButton = new QToolButton(this);
   pSaveButton->setIcon(QIcon(":/icons/Save"));
   pSaveButton->setToolTip("Save Binning File");
   pSaveButton->setAutoRaise(true);

   QToolButton* pAddButton = new QToolButton(this);
   pAddButton->setIcon(QIcon(":/icons/SpeedUp"));
   pAddButton->setToolTip("Add a Bin");
   pAddButton->setAutoRaise(true);

   QToolButton* pDeleteButton = new QToolButton(this);
   pDeleteButton->setIcon(QIcon(":/icons/SlowDown"));
   pDeleteButton->setToolTip("Remove Selected Bins");
   pDeleteButton->setAutoRaise(true);

   QToolButton* pClearButton = new QToolButton(this);
   pClearButton->setIcon(QIcon(":/icons/Delete"));
   pClearButton->setToolTip("Clear Bins");
   pClearButton->setAutoRaise(true);

   QToolButton* pResetButton = new QToolButton(this);
   pResetButton->setIcon(QIcon(":/icons/Undo"));
   pResetButton->setToolTip("Reset Bins");
   pResetButton->setAutoRaise(true);

   QToolButton* pUpButton = new QToolButton(this);
   pUpButton->setIcon(QIcon(":/icons/Increase"));
   pUpButton->setToolTip("Move Selected Bins Up");
   pUpButton->setAutoRaise(true);

   QToolButton* pDownButton = new QToolButton(this);
   pDownButton->setIcon(QIcon(":/icons/Decrease"));
   pDownButton->setToolTip("Move Selected Bins Down");
   pDownButton->setAutoRaise(true);

   QHBoxLayout* pButtonLayout = new QHBoxLayout;
   pButtonLayout->setSpacing(0);
   pButtonLayout->setMargin(0);
   pButtonLayout->addWidget(pOpenButton);
   pButtonLayout->addWidget(pSaveButton);
   pButtonLayout->addSpacing(15);
   pButtonLayout->addWidget(pAddButton);
   pButtonLayout->addWidget(pDeleteButton);
   pButtonLayout->addSpacing(15);
   pButtonLayout->addWidget(pClearButton);
   pButtonLayout->addWidget(pResetButton);
   pButtonLayout->addSpacing(15);
   pButtonLayout->addWidget(pUpButton);
   pButtonLayout->addWidget(pDownButton);
   pButtonLayout->addStretch();

   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   QDialogButtonBox* pButtonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(mpGroupedBandView, 10);
   pLayout->addLayout(pButtonLayout);
   pLayout->addWidget(pLine);
   pLayout->addWidget(pButtonBox);

   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));

   VERIFYNR(connect(pOpenButton, SIGNAL(clicked()), this, SLOT(openGroupedBands())));
   VERIFYNR(connect(pSaveButton, SIGNAL(clicked()), this, SLOT(saveGroupedBands())));
   VERIFYNR(connect(pAddButton, SIGNAL(clicked()), this, SLOT(addGroupedBand())));
   VERIFYNR(connect(pDeleteButton, SIGNAL(clicked()), this, SLOT(deleteGroupedBands())));
   VERIFYNR(connect(pClearButton, SIGNAL(clicked()), this, SLOT(clearGroupedBands())));
   VERIFYNR(connect(pResetButton, SIGNAL(clicked()), this, SLOT(resetGroupedBands())));
   VERIFYNR(connect(pUpButton, SIGNAL(clicked()), this, SLOT(moveUpGroupedBands())));
   VERIFYNR(connect(pDownButton, SIGNAL(clicked()), this, SLOT(moveDownGroupedBands())));

   VERIFYNR(connect(mpBandModel, SIGNAL(modelReset()), mpGroupedBandView, SLOT(resizeRowsToContents())));
   mpBandModel->setGroupedBands(groupedBands);
   mpGroupedBandView->resizeColumnsToContents();
}

BandBinningDlg::~BandBinningDlg()
{}

const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& BandBinningDlg::getGroupedBands()
{
   return mpBandModel->getGroupedBands();
}

void BandBinningDlg::accept()
{
   std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> > groupedBands = getGroupedBands();
   if (groupedBands.empty())
   {
      QMessageBox::warning(this, "Error", "No band groups were specified.");
      return;
   }

   if (BandBinningUtilities::preprocessGroupedBands(groupedBands))
   {
      if (groupedBands.empty())
      {
         QMessageBox::warning(this, "Error", "Band groups were specified, but none of them are valid for this data."
            "Check the band groups and try again.");
         return;
      }

      // Yes: set the new band groups and return without accepting so the user can view them
      // No: set the new band groups and accept the dialog without additional confirmation of the changes
      // Cancel: do not set the new band groups and do not accept the dialog
      QMessageBox::StandardButtons response = QMessageBox::question(this, "Band Groups Modified",
         "One or more band groups must be modified or removed in order to proceed.\n"
         "Would you like to view the modified set of band groups before proceeding?",
         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No);
      if (response == QMessageBox::Cancel)
      {
         return;
      }

      mpBandModel->setGroupedBands(groupedBands);
      if (response == QMessageBox::Yes)
      {
         return;
      }
   }

   QDialog::accept();
}

void BandBinningDlg::openGroupedBands()
{
   const Filename* pDirectory = ConfigurationSettings::getSettingImportPath();
   QString initialDirectory;
   if (pDirectory != NULL)
   {
      initialDirectory = QString::fromStdString(pDirectory->getFullPathAndName());
   }

   std::string filename = QFileDialog::getOpenFileName(this, "Open Binning File",
      initialDirectory, "Binning Files (*.txt);;All Files (*)").toStdString();
   if (filename.empty() == false)
   {
      std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> > groupedBands =
         BandBinningUtilities::readFile(filename, mpDescriptor);
      if (groupedBands.empty() == false ||
         QMessageBox::question(this, "No Bins Found", "No bins were found in the specified file.\n"
         "Do you want to clear the dialog?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
      {
         mpBandModel->setGroupedBands(groupedBands);
      }
   }
}

void BandBinningDlg::saveGroupedBands()
{
   std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> > groupedBands = getGroupedBands();
   if (groupedBands.empty())
   {
      QMessageBox::warning(this, "Error", "No band groups were specified.");
      return;
   }

   if (BandBinningUtilities::preprocessGroupedBands(groupedBands))
   {
      if (groupedBands.empty())
      {
         QMessageBox::warning(this, "Error", "Band groups were specified, but none of them are valid for this data."
            "Check the band groups and try again.");
         return;
      }

      if (QMessageBox::question(this, "Band Groups Modified", 
         "One or more band groups must be modified to save the file.\n"
         "Would you like to continue?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No)
      {
         return;
      }

      mpBandModel->setGroupedBands(groupedBands);
   }

   const Filename* pDirectory = ConfigurationSettings::getSettingExportPath();
   QString initialDirectory;
   if (pDirectory != NULL)
   {
      initialDirectory = QString::fromStdString(pDirectory->getFullPathAndName());
   }

   std::string filename = QFileDialog::getSaveFileName(this, "Save Binning File",
      initialDirectory, "Binning Files (*.txt);;All Files (*)").toStdString();
   if (filename.empty() == false && BandBinningUtilities::writeFile(filename, groupedBands) == false)
   {
      QMessageBox::warning(this, "Error", "Unable to save the bin file.\nCheck file permissions and try again.");
   }
}

void BandBinningDlg::addGroupedBand()
{
   // Build the list of output bands.
   // Always add one extra band so that the user can append the item to the end of the list (default behavior).
   const int row = mpBandModel->rowCount();
   VERIFYNRV(row >= 0);
   QStringList outputBands;
   for (int i = 0; i < row + 1; ++i)   // Extend one extra row to allow the new bin to be appended.
   {
      outputBands << QString::number(i + 1); // Display is 1-based; storage is 0-based.
   }

   // Build the list of input bands, using band names if available.
   QStringList inputBands;
   std::vector<std::string> bandNames = RasterUtilities::getBandNames(mpDescriptor);
   for (std::vector<std::string>::iterator iter = bandNames.begin(); iter != bandNames.end(); ++iter)
   {
      inputBands.append(QString::fromStdString(*iter));
   }

   // Build the dialog to display to the user.
   QDialog dialog(this);
   dialog.setWindowTitle("Add Bin");
   QLabel* pOutputBandLabel = new QLabel("Output Band", &dialog);
   QComboBox* pOutputBandComboBox = new QComboBox(&dialog);
   pOutputBandComboBox->setToolTip("Enter the band in the output where this bin should appear. "
      "By default, the new bin will appear as the last band in the new image.");
   pOutputBandComboBox->addItems(outputBands);
   pOutputBandComboBox->setCurrentIndex(row);

   QLabel* pFirstInputBandLabel = new QLabel("First Input Band", &dialog);
   QComboBox* pFirstInputComboBox = new QComboBox(&dialog);
   pFirstInputComboBox->setToolTip("Enter the first band for this bin. "
      "Bins range from first band to last band and are inclusive.");
   pFirstInputComboBox->addItems(inputBands);

   QLabel* pLastInputBandLabel = new QLabel("Last Input Band", &dialog);
   QComboBox* pLastInputComboBox = new QComboBox(&dialog);
   pFirstInputComboBox->setToolTip("Enter the last band for this bin. "
      "Bins range from first band to last band and are inclusive.");
   pLastInputComboBox->addItems(inputBands);

   QGridLayout* pTopLayout = new QGridLayout;
   pTopLayout->addWidget(pOutputBandLabel, 0, 0);
   pTopLayout->addWidget(pOutputBandComboBox, 0, 1);
   pTopLayout->addWidget(pFirstInputBandLabel, 1, 0);
   pTopLayout->addWidget(pFirstInputComboBox, 1, 1);
   pTopLayout->addWidget(pLastInputBandLabel, 2, 0);
   pTopLayout->addWidget(pLastInputComboBox, 2, 1);
   pTopLayout->setRowStretch(3, 10);
   pTopLayout->setColumnStretch(1, 10);

   QFrame* pLine = new QFrame(&dialog);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, &dialog);

   QVBoxLayout* pLayout = new QVBoxLayout(&dialog);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addLayout(pTopLayout);
   pLayout->addWidget(pLine);
   pLayout->addWidget(pButtonBox);

   VERIFYNRV(connect(pButtonBox, SIGNAL(accepted()), &dialog, SLOT(accept())));
   VERIFYNRV(connect(pButtonBox, SIGNAL(rejected()), &dialog, SLOT(reject())));
   if (dialog.exec() == QDialog::Accepted)
   {
      DimensionDescriptor firstBand = mpDescriptor->getActiveBand(pFirstInputComboBox->currentIndex());
      VERIFYNRV(firstBand.isOriginalNumberValid());

      DimensionDescriptor lastBand = mpDescriptor->getActiveBand(pLastInputComboBox->currentIndex());
      VERIFYNRV(lastBand.isOriginalNumberValid());

      mpBandModel->addGroupedBand(pOutputBandComboBox->currentIndex(), std::make_pair(firstBand, lastBand));
   }
}

void BandBinningDlg::deleteGroupedBands()
{
   // Both the underlying std::vector and this method could be refactored for performance.
   // This method creates an unnecessary copy of the unselected band groups, however, because the underlying model is
   // of type std::vector, this is still faster than removing each row individually and reorganizing the std::vector
   // and updating the GUI for each removal.

   // Build an unsorted list of all selected rows.
   QItemSelectionModel* pModel = mpGroupedBandView->selectionModel();
   VERIFYNRV(pModel != NULL);
   QModelIndexList selectedRows = pModel->selectedRows();
   if (selectedRows.empty() == true)
   {
      return;
   }

   std::list<unsigned int> rows;
   foreach(QModelIndex index, selectedRows)
   {
      if (index.isValid() == true && index.row() >= 0)
      {
         rows.push_back(static_cast<unsigned int>(index.row()));
      }
   }

   // Make a copy of each grouped band whose row is not selected.
   const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands =
      mpBandModel->getGroupedBands();
   std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> > newGroupedBands;
   newGroupedBands.reserve(groupedBands.size() - rows.size());
   for (unsigned int i = 0; i < groupedBands.size(); ++i)
   {
      std::list<unsigned int>::iterator iter = std::find(rows.begin(), rows.end(), i);
      if (iter != rows.end())
      {
         rows.erase(iter);
      }
      else
      {
         newGroupedBands.push_back(groupedBands[i]);
      }
   }

   // Update the model.
   mpBandModel->setGroupedBands(newGroupedBands);
}

void BandBinningDlg::clearGroupedBands()
{
   mpBandModel->setGroupedBands(std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >());
}

void BandBinningDlg::resetGroupedBands()
{
   mpBandModel->setGroupedBands(mOriginalGroupedBands);
}

void BandBinningDlg::moveUpGroupedBands()
{
   // Both the underlying std::vector and this method could be refactored for performance.
   // This method performs swaps in such a manner that contiguous selections are not optimized.

   // Sort all selected rows, readjust the selection to account for the move, and move each selected item up.
   QItemSelectionModel* pModel = mpGroupedBandView->selectionModel();
   VERIFYNRV(pModel != NULL);
   QModelIndexList selectedRows = pModel->selectedRows();
   pModel->clearSelection();

   std::vector<int> rows;
   foreach(QModelIndex index, selectedRows)
   {
      if (index.isValid() == true)
      {
         rows.push_back(index.row());
         QModelIndex newIndex = mpBandModel->index(index.row() - 1, 0);
         if (newIndex.isValid() == true)
         {
            pModel->select(newIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
         }
      }
   }

   // This must be sorted in ascending order so that moveUp occurs in the proper sequence.
   std::sort(rows.begin(), rows.end());
   for (std::vector<int>::iterator iter = rows.begin(); iter != rows.end(); ++iter)
   {
      mpBandModel->moveUp(*iter);
   }
}

void BandBinningDlg::moveDownGroupedBands()
{
   // Both the underlying std::vector and this method could be refactored for performance.
   // This method performs swaps in such a manner that contiguous selections are not optimized.

   // Sort all selected rows, readjust the selection to account for the move, and move each selected item down.
   QItemSelectionModel* pModel = mpGroupedBandView->selectionModel();
   VERIFYNRV(pModel != NULL);
   QModelIndexList selectedRows = pModel->selectedRows();
   pModel->clearSelection();

   std::vector<int> rows;
   foreach(QModelIndex index, selectedRows)
   {
      if (index.isValid() == true)
      {
         rows.push_back(index.row());
         QModelIndex newIndex = mpBandModel->index(index.row() + 1, 0);
         if (newIndex.isValid() == true)
         {
            pModel->select(newIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
         }
      }
   }

   // This must be sorted in descending order so that moveDown occurs in the proper sequence.
   std::sort(rows.rbegin(), rows.rend());
   for (std::vector<int>::iterator iter = rows.begin(); iter != rows.end(); ++iter)
   {
      mpBandModel->moveDown(*iter);
   }
}

void BandBinningDlg::keyPressEvent(QKeyEvent* pEvent)
{
   if (pEvent != NULL && pEvent->key() == Qt::Key_Delete)
   {
      deleteGroupedBands();
   }
   else
   {
      QDialog::keyPressEvent(pEvent);
   }
}
