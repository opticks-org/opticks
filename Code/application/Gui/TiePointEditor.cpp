/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QHeaderView>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QLayout>

#include "AppVerify.h"
#include "TiePointEditor.h"
#include "TiePointListUndo.h"
#include "TiePointTableModel.h"
#include "View.h"

#include <string>
using namespace std;

TiePointEditor::TiePointEditor(QWidget* pParent) :
   QDialog(pParent),
   mpTableView(NULL),
   mpAddButton(NULL),
   mpDeleteButton(NULL),
   mpGoToButton(NULL),
   mpLayer(NULL),
   mpTableModel(new TiePointTableModel(this))
{
   // Table
   mpTableView = new QTableView(this);
   mpTableView->setSelectionMode(QAbstractItemView::SingleSelection);
   mpTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked |
      QAbstractItemView::EditKeyPressed);
   mpTableView->setSortingEnabled(false);
   mpTableView->setCornerButtonEnabled(false);
   mpTableView->setModel(mpTableModel);

   QHeaderView* pHorizHeader = mpTableView->horizontalHeader();
   if (pHorizHeader != NULL)
   {
      pHorizHeader->setDefaultSectionSize(75);
      pHorizHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   }

   QHeaderView* pVerticalHeader = mpTableView->verticalHeader();
   if (pVerticalHeader != NULL)
   {
      pVerticalHeader->setDefaultSectionSize(20);
   }

   // Buttons
   mpAddButton = new QPushButton("&Add", this);
   mpDeleteButton = new QPushButton("&Delete", this);
   mpGoToButton = new QPushButton("&Go To...", this);
   QPushButton* pCloseButton = new QPushButton("&Close", this);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);
   pGrid->addWidget(mpTableView, 0, 0, 1, 4);
   pGrid->addWidget(mpAddButton, 1, 0, Qt::AlignRight);
   pGrid->addWidget(mpDeleteButton, 1, 1);
   pGrid->addWidget(mpGoToButton, 1, 2);
   pGrid->addWidget(pCloseButton, 1, 3);
   pGrid->setRowStretch(0, 10);
   pGrid->setColumnStretch(0, 10);

   // Initialization
   setWindowTitle("Tie Point Editor");
   setModal(false);
   resize(700, 400);
   enableButtons(false);

   // Connections
   VERIFYNR(connect(mpTableModel, SIGNAL(pointsModified(const std::vector<TiePoint>&, const std::vector<TiePoint>&)),
      this, SLOT(pointsEdited(const std::vector<TiePoint>&, const std::vector<TiePoint>&))));
   VERIFYNR(connect(mpAddButton, SIGNAL(clicked()), this, SLOT(addPoint())));
   VERIFYNR(connect(mpDeleteButton, SIGNAL(clicked()), this, SLOT(deletePoint())));
   VERIFYNR(connect(mpGoToButton, SIGNAL(clicked()), this, SLOT(goToRow())));
   VERIFYNR(connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close())));
}

TiePointEditor::~TiePointEditor()
{
}

bool TiePointEditor::setTiePointLayer(Layer* pLayer)
{
   if (pLayer != NULL && pLayer->getLayerType() != TIEPOINT_LAYER)
   {
      return false;
   }

   // Reset the member pointer
   mpLayer.reset(dynamic_cast<TiePointLayer*>(pLayer));

   // Update the dialog caption
   QString caption = "Tie Point Editor";
   if (mpLayer.get() != NULL)
   {
      string name = mpLayer->getDisplayName();
      if (name.empty() == true)
      {
         name = mpLayer->getName();
      }

      if (name.empty() == false)
      {
         caption += " - " + QString::fromStdString(name);
      }
   }

   setWindowTitle(caption);

   // Update the tie points
   TiePointList* pTiePointList = NULL;
   if (mpLayer.get() != NULL)
   {
      pTiePointList = dynamic_cast<TiePointList*>(mpLayer->getDataElement());
   }

   mpTableModel->setTiePointList(pTiePointList);

   // Enable the buttons
   enableButtons(mpLayer.get() != NULL);
   return true;
}

void TiePointEditor::addPoint()
{
   if (mpLayer.get() == NULL)
   {
      return;
   }

   TiePointList* pTiePointList = dynamic_cast<TiePointList*>(mpLayer->getDataElement());
   if (pTiePointList == NULL)
   {
      return;
   }

   const vector<TiePoint>& oldPoints = pTiePointList->getTiePoints();
   vector<TiePoint> tiePoints = oldPoints;
   tiePoints.push_back(TiePoint());

   View* pView = mpLayer->getView();
   if (pView != NULL)
   {
      pView->addUndoAction(new SetTiePoints(pTiePointList, oldPoints, tiePoints));
   }

   int numTiePoints = static_cast<int>(tiePoints.size());
   pTiePointList->adoptTiePoints(tiePoints);
   goToRow(numTiePoints - 1);
}

void TiePointEditor::deletePoint()
{
   if (mpLayer.get() == NULL)
   {
      return;
   }

   TiePointList* pTiePointList = dynamic_cast<TiePointList*>(mpLayer->getDataElement());
   if (pTiePointList == NULL)
   {
      return;
   }

   QModelIndex index = mpTableView->currentIndex();
   if (index.isValid() == false)
   {
      QMessageBox::warning(this, "Tie Point Editor", "Please select a tie point to delete.");
      return;
   }

   int currentRow = index.row();
   const vector<TiePoint>& oldPoints = pTiePointList->getTiePoints();
   vector<TiePoint> points = oldPoints;

   vector<TiePoint>::iterator pPoint = points.begin() + index.row();
   points.erase(pPoint);

   View* pView = mpLayer->getView();
   if (pView != NULL)
   {
      pView->addUndoAction(new SetTiePoints(pTiePointList, oldPoints, points));
   }

   pTiePointList->adoptTiePoints(points);

   if (currentRow == mpTableModel->rowCount())
   {
      --currentRow;
   }

   goToRow(currentRow);
}

void TiePointEditor::goToRow()
{
   int numRows = mpTableModel->rowCount();
   if (numRows == 0)
   {
      return;
   }

   QModelIndex index = mpTableView->currentIndex();

   int currentRow = 0;
   if (index.isValid() == true)
   {
      currentRow = index.row();
   }

   bool bOk = true;

   int row = QInputDialog::getInteger(this, "Go To Tie Point",
      "Enter the tie point number to display at the top of the table", currentRow + 1, 1, numRows, 1, &bOk);
   if (bOk)
   {
      goToRow(--row);
   }
}

void TiePointEditor::goToRow(int row)
{
   if ((row < 0) || (row >= mpTableModel->rowCount()))
   {
      return;
   }

   int currentColumn = 0;

   QModelIndex index = mpTableView->currentIndex();
   if (index.isValid() == true)
   {
      currentColumn = index.column();
   }

   QModelIndex newIndex = mpTableModel->index(row, currentColumn);
   if (newIndex.isValid() == true)
   {
      mpTableView->scrollTo(newIndex, QAbstractItemView::PositionAtTop);
      mpTableView->setCurrentIndex(newIndex);
      mpTableView->setFocus();
   }
}

void TiePointEditor::pointsEdited(const vector<TiePoint>& oldPoints, const vector<TiePoint>& newPoints)
{
   if (mpLayer.get() == NULL)
   {
      return;
   }

   TiePointList* pTiePointList = dynamic_cast<TiePointList*>(mpLayer->getDataElement());
   if (pTiePointList != NULL)
   {
      View* pView = mpLayer->getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetTiePoints(pTiePointList, oldPoints, newPoints));
      }
   }
}

void TiePointEditor::showEvent(QShowEvent* pEvent)
{
   QDialog::showEvent(pEvent);
   emit visibilityChanged(true);
}

void TiePointEditor::hideEvent(QHideEvent* pEvent)
{
   QDialog::hideEvent(pEvent);
   emit visibilityChanged(false);
}

void TiePointEditor::enableButtons(bool bEnable)
{
   mpAddButton->setEnabled(bEnable);
   mpDeleteButton->setEnabled(bEnable);
   mpGoToButton->setEnabled(bEnable);
}
