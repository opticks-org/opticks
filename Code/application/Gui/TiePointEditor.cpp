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
#include <QtGui/QKeyEvent>
#include <QtGui/QLayout>

#include "AppVerify.h"
#include "Slot.h"
#include "TiePointEditor.h"
#include "TiePointLayer.h"
#include "TiePointList.h"
#include "TiePointListUndo.h"
#include "View.h"

TiePointEditor::TiePointEditor(QWidget* parent) :
   QDialog(parent),
   mpTable(NULL),
   mpAddButton(NULL),
   mpDeleteButton(NULL),
   mpGoToButton(NULL),
   mpCloseButton(NULL),
   mpScrollBar(NULL),
   mpLayer(NULL),
   mpTiePointList(NULL)
{
   // Table
   QStringList columnHeaders;
   columnHeaders += "Ref X";
   columnHeaders += "Ref Y";
   columnHeaders += "Mission X";
   columnHeaders += "Mission Y";
   columnHeaders += "Confidence";
   columnHeaders += "Phi";

   mpTable = new QTableWidget(this);
   mpTable->setColumnCount(6);
   mpTable->setHorizontalHeaderLabels(columnHeaders);
   mpTable->setSelectionMode(QAbstractItemView::NoSelection);
   mpTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   mpTable->installEventFilter(this);

   QHeaderView* pHorizHeader = mpTable->horizontalHeader();
   if (pHorizHeader != NULL)
   {
      pHorizHeader->setDefaultSectionSize(75);
      pHorizHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   }

   QHeaderView* pVerticalHeader = mpTable->verticalHeader();
   if (pVerticalHeader != NULL)
   {
      pVerticalHeader->setDefaultSectionSize(20);
   }

   // Scroll bar
   mpScrollBar = new QScrollBar(this);
   mpScrollBar->setOrientation(Qt::Vertical);

   // Buttons
   mpAddButton = new QPushButton("&Add", this);
   mpDeleteButton = new QPushButton("&Delete", this);
   mpGoToButton = new QPushButton("&Go To...", this);
   mpCloseButton = new QPushButton("&Close", this);

   // Layout
   QHBoxLayout* pTableLayout = new QHBoxLayout();
   pTableLayout->setMargin(0);
   pTableLayout->setSpacing(0);
   pTableLayout->addWidget(mpTable, 10);
   pTableLayout->addWidget(mpScrollBar);

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(mpAddButton);
   pButtonLayout->addWidget(mpDeleteButton);
   pButtonLayout->addWidget(mpGoToButton);
   pButtonLayout->addWidget(mpCloseButton);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   pLayout->addLayout(pTableLayout, 10);
   pLayout->addLayout(pButtonLayout);

   // Initialization
   setWindowTitle("Tie Point Editor");
   setModal(false);
   resize(700, 400);
   enableButtons(mpTiePointList != NULL);

   // Connections
   VERIFYNR(connect(mpTable, SIGNAL(cellChanged(int, int)), this, SLOT(updatePoint(int, int))));
   VERIFYNR(connect(mpAddButton, SIGNAL(clicked()), this, SLOT(addPoint())));
   VERIFYNR(connect(mpDeleteButton, SIGNAL(clicked()), this, SLOT(deletePoint())));
   VERIFYNR(connect(mpGoToButton, SIGNAL(clicked()), this, SLOT(goToRow())));
   VERIFYNR(connect(mpCloseButton, SIGNAL(clicked()), this, SLOT(close())));
   VERIFYNR(connect(mpScrollBar, SIGNAL(valueChanged(int)), this, SLOT(updateTiePointTable())));
}

TiePointEditor::~TiePointEditor()
{
}

void TiePointEditor::elementModified(Subject &subject, const std::string &signal, const boost::any &data)
{
   if (dynamic_cast<TiePointList*>(&subject) == mpTiePointList)
   {
      updateTiePointTable();
   }
}

void TiePointEditor::elementDeleted(Subject &subject, const std::string &signal, const boost::any &data)
{
   if (dynamic_cast<TiePointList*>(&subject) == mpTiePointList)
   {
      setTiePointLayer(NULL);
   }
}

bool TiePointEditor::setTiePointLayer(Layer* pLayer)
{
   if (pLayer != NULL && pLayer->getLayerType() != TIEPOINT_LAYER)
   {
      return false;
   }

   if (mpTiePointList != NULL)
   {
      mpTiePointList->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointEditor::elementDeleted));
      mpTiePointList->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &TiePointEditor::elementModified));
      mpTiePointList->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointEditor::tiePointListDeleted));
   }

   mpLayer = dynamic_cast<TiePointLayer*>(pLayer);
   if (mpLayer != NULL)
   {
      mpTiePointList = static_cast<TiePointList*>(mpLayer->getDataElement());
   }
   else
   {
      mpTiePointList = NULL;
   }

   if (mpTiePointList != NULL)
   {
      mpTiePointList->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointEditor::elementDeleted));
      mpTiePointList->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &TiePointEditor::elementModified));
      mpTiePointList->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointEditor::tiePointListDeleted));
   }

   mpScrollBar->setValue(0);
   updateTiePointTable();
   return true;
}

unsigned int TiePointEditor::numVisibleRows() const
{
   if (mpTiePointList == NULL)
   {
      return 0;
   }

   if (mpTable->rowCount() == 0)
   {
      mpTable->setRowCount(1);
   }
   int headerHeight = 0;

   QHeaderView* pHeader = mpTable->horizontalHeader();
   if (pHeader != NULL)
   {
      headerHeight = pHeader->height();
   }

   int rowHeight = mpTable->rowHeight(0);
   unsigned long properNumRows = (mpTable->height()-headerHeight) / rowHeight;
   unsigned long size = mpTiePointList->getTiePoints().size();
   properNumRows = std::min(properNumRows, size);
   return properNumRows;
}

void TiePointEditor::updateTiePointTable()
{
   if (mpTable == NULL)
   {
      return;
   }

   // Delete all table items without clearing the header names
   mpTable->clearContents();

   // ensure we're displaying the proper number of rows
   unsigned int properNumRows = numVisibleRows();
   if (mpTable->rowCount() != properNumRows)
   {
      mpTable->setRowCount(properNumRows);
   }

   unsigned int numPoints = (mpTiePointList!=NULL?mpTiePointList->getTiePoints().size():0);

   // ensure scroll bar is set properly
   int scrollBarMax = 0;
   if (numPoints > properNumRows)
   {
      scrollBarMax = numPoints - properNumRows;
   }
   if (mpScrollBar->value() > scrollBarMax)
   {
      mpScrollBar->setValue(scrollBarMax);
   }
   if (mpScrollBar->maximum() != scrollBarMax)
   {
      mpScrollBar->setMaximum(scrollBarMax);
   }
   mpScrollBar->setPageStep(properNumRows);
   
   // update the table
   VERIFYNR(disconnect(mpTable, SIGNAL(cellChanged(int, int)), this, SLOT(updatePoint(int, int))));

   QStringList rowLabels;
   for (unsigned int i=0; i<properNumRows; ++i)
   {
      if (mpScrollBar->value()+i >= numPoints)
      {
         break; // shouldn't be able to happen, but we'd crash if it did
      }
      rowLabels += QString::number(i+1+mpScrollBar->value());
      const TiePoint &point (mpTiePointList->getTiePoints().at(mpScrollBar->value()+i));

      QTableWidgetItem* pItem = new QTableWidgetItem(QString::number(point.mReferencePoint.mX + 1));
      mpTable->setItem(i, 0, pItem);

      pItem = new QTableWidgetItem(QString::number(point.mReferencePoint.mY + 1));
      mpTable->setItem(i, 1, pItem);

      pItem = new QTableWidgetItem(QString::number(point.mMissionOffset.mX));
      mpTable->setItem(i, 2, pItem);

      pItem = new QTableWidgetItem(QString::number(point.mMissionOffset.mY));
      mpTable->setItem(i, 3, pItem);

      pItem = new QTableWidgetItem(QString::number(point.mConfidence));
      mpTable->setItem(i, 4, pItem);

      pItem = new QTableWidgetItem(QString::number(point.mPhi));
      mpTable->setItem(i, 5, pItem);
   }
   mpTable->setVerticalHeaderLabels(rowLabels);
   enableButtons(mpTiePointList != NULL);
   VERIFYNR(connect(mpTable, SIGNAL(cellChanged(int, int)), this, SLOT(updatePoint(int, int))));
}

void TiePointEditor::resizeEvent ( QResizeEvent *pEvent )
{
   QDialog::resizeEvent(pEvent);
   updateTiePointTable();
}

bool TiePointEditor::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if (pEvent == NULL) 
   {
      return false;
   }

   if (pObject == mpTable)
   {
      if (pEvent->type() == QEvent::KeyPress)
      {
         QKeyEvent *pKeyEvent = static_cast<QKeyEvent*>(pEvent);
         int key = pKeyEvent->key();
         int modifiers = pKeyEvent->modifiers();
         if (key == Qt::Key_PageUp && modifiers == 0)
         {
            int numRows = numVisibleRows();
            int newValue = std::max(0, mpScrollBar->value()-numRows);
            mpScrollBar->setValue(newValue);
            return true;
         }
         else if (key == Qt::Key_PageDown && modifiers == 0)
         {
            int numRows = numVisibleRows();
            int numPoints = (mpTiePointList!=NULL?mpTiePointList->getTiePoints().size():0);
            int newValue = std::min(static_cast<int>(numPoints)-numRows, mpScrollBar->value()+numRows);
            mpScrollBar->setValue(newValue);
            return true;
         }
         else if (key == Qt::Key_Up && modifiers == 0)
         {
            if (mpTable->currentRow() == 0 && mpScrollBar->value() != 0)
            {
               mpScrollBar->setValue(mpScrollBar->value()-1);
               return true;
            }
         }
         else if (key == Qt::Key_Down && modifiers == 0)
         {
            int numRows = numVisibleRows();
            if (mpTable->currentRow() == numRows-1 && mpScrollBar->value() != mpScrollBar->maximum())
            {
               mpScrollBar->setValue(mpScrollBar->value()+1);
               return true;
            }
         }
         else if (key == Qt::Key_Home)
         {
            mpScrollBar->setValue(mpScrollBar->minimum());
            mpTable->setCurrentCell(0, 0);
            return true;
         }
         else if (key == Qt::Key_End)
         {
            int numRows = numVisibleRows();
            mpScrollBar->setValue(mpScrollBar->maximum());
            mpTable->setCurrentCell(numRows, 0);
            return true;
         }
      }
   }
   return false;
}

void TiePointEditor::goToRow()
{
   bool bOk = true;
   int numPoints = (mpTiePointList!=NULL?mpTiePointList->getTiePoints().size():0);
   int newValue = QInputDialog::getInteger(this, "Go To Row", "Enter the row in the table to go to", 
      mpScrollBar->value()+1, 1, numPoints, 1, &bOk);
   if (bOk)
   {
      --newValue;
      goToRow(newValue);
   }
}

void TiePointEditor::goToRow(int row)
{
   int selectionRow = row;
   row = std::min(row, mpScrollBar->maximum());
   selectionRow -= row;
   mpScrollBar->setValue(row);
   mpTable->setCurrentCell(selectionRow, 0);
}

void TiePointEditor::addPoint()
{
   if (mpTiePointList == NULL)
   {
      return;
   }

   std::vector<TiePoint> oldPoints = mpTiePointList->getTiePoints();
   std::vector<TiePoint> tiePoints = oldPoints;
   tiePoints.push_back(TiePoint());

   if (mpLayer != NULL)
   {
      View* pView = mpLayer->getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetTiePoints(mpTiePointList, oldPoints, tiePoints));
      }
   }

   int numTiePoints = tiePoints.size();
   mpTiePointList->adoptTiePoints(tiePoints);
   goToRow(numTiePoints - 1);
}

void TiePointEditor::updatePoint(int row, int column)
{
   if (mpTiePointList == NULL)
   {
      return;
   }

   std::vector<TiePoint> oldPoints = mpTiePointList->getTiePoints();
   std::vector<TiePoint> tiePoints = oldPoints;

   QTableWidgetItem* pItem = mpTable->item(row, column);
   if (pItem == NULL)
   {
      return;
   }

   QString text = pItem->text();

   int intField = 0;
   float floatField = 0.0;
   bool bOk = true;
   if (mpScrollBar->value()+row >= static_cast<int>(tiePoints.size()))
   {
      return; // shouldn't be able to happen, but we'd crash if it did
   }

   TiePoint &point(tiePoints.at(mpScrollBar->value()+row));
   switch (column)
   {
      case 0:
         intField = text.toInt(&bOk);
         if (bOk) point.mReferencePoint.mX = intField-1;
         break;
      case 1:
         intField = text.toInt(&bOk);
         if (bOk) point.mReferencePoint.mY = intField-1;
         break;
      case 2:
         floatField = text.toFloat(&bOk);
         if (bOk) point.mMissionOffset.mX = floatField;
         break;
      case 3:
         floatField = text.toFloat(&bOk);
         if (bOk) point.mMissionOffset.mY = floatField;
         break;
      case 4:
         intField = text.toInt(&bOk);
         if (bOk) point.mConfidence = intField;
         break;
      case 5:
         intField = text.toInt(&bOk);
         if (bOk) point.mPhi = intField;
         break;
      default:
         break;
   }

   if (bOk)
   {
      if (mpLayer != NULL)
      {
         View* pView = mpLayer->getView();
         if (pView != NULL)
         {
            pView->addUndoAction(new SetTiePoints(mpTiePointList, oldPoints, tiePoints));
         }
      }

      mpTiePointList->adoptTiePoints(tiePoints);
   }
}

void TiePointEditor::deletePoint()
{
   if (mpTiePointList == NULL)
   {
      return;
   }

   if (mpTable->currentItem() == NULL)
   {
      return;
   }

   std::vector<TiePoint> oldPoints = mpTiePointList->getTiePoints();
   std::vector<TiePoint> points = oldPoints;

   std::vector<TiePoint>::iterator pPoint = points.begin() + mpScrollBar->value() + mpTable->currentRow();
   points.erase(pPoint);

   if (mpLayer != NULL)
   {
      View* pView = mpLayer->getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetTiePoints(mpTiePointList, oldPoints, points));
      }
   }

   mpTiePointList->adoptTiePoints(points);
}

void TiePointEditor::showEvent( QShowEvent *e )
{
   QDialog::showEvent(e);
   updateTiePointTable();
   emit visibilityChanged(true);
}

void TiePointEditor::closeEvent( QCloseEvent *e )
{
   QDialog::closeEvent(e);
   emit visibilityChanged(false);
}

void TiePointEditor::tiePointListDeleted(Subject& subject, const std::string& signal, const boost::any& value)
{
   TiePointList* pTiePointList = dynamic_cast<TiePointList*>(&subject);
   if (pTiePointList == mpTiePointList)
   {
      setTiePointLayer(NULL);
   }
}

void TiePointEditor::enableButtons(bool bEnable)
{
   mpAddButton->setEnabled(bEnable);
   mpDeleteButton->setEnabled(bEnable);
   mpGoToButton->setEnabled(bEnable);
}
