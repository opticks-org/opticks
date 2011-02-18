/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include "SelectionGrid.h"

SelectionGrid::SelectionGrid(QWidget* parent) :
   QWidget(parent),
   miRows(1),
   miColumns(1),
   mbTracking(false),
   miCellWidth(5),
   miCellHeight(5),
   miBorderSpacing(2),
   miCellSpacing(2),
   mpDescription(NULL),
   mSelectedRow(-1),
   mSelectedColumn(-1)
{
   setMouseTracking(true);
   setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
   setFocusPolicy(Qt::StrongFocus);
}

SelectionGrid::~SelectionGrid()
{
}

void SelectionGrid::setNumRows(int iRows)
{
   if (iRows < 1)
   {
      miRows = 1;
   }
   else
   {
      miRows = iRows;
   }

   // Update the description label location
   updateDescriptionLocation();
}

int SelectionGrid::getNumRows() const
{
   return miRows;
}

void SelectionGrid::setNumColumns(int iColumns)
{
   if (iColumns < 1)
   {
      miColumns = 1;
   }
   else
   {
      miColumns = iColumns;
   }

   // Update the description label location
   updateDescriptionLocation();
}

int SelectionGrid::getNumColumns() const
{
   return miColumns;
}

void SelectionGrid::setCellTracking(bool bEnable)
{
   mbTracking = bEnable;
}

bool SelectionGrid::getCellTracking() const
{
   return mbTracking;
}

void SelectionGrid::keyPressEvent(QKeyEvent* pEvent)
{
   if (pEvent != NULL)
   {
      int row = mSelectedRow;
      int column = mSelectedColumn;

      if (pEvent->key() == Qt::Key_Left)
      {
         --column;
      }
      else if (pEvent->key() == Qt::Key_Right)
      {
         ++column;
      }
      else if (pEvent->key() == Qt::Key_Up)
      {
         --row;
      }
      else if (pEvent->key() == Qt::Key_Down)
      {
         ++row;
      }

      if (row < 0)
      {
         row = 0;
      }

      if (column < 0)
      {
         column = 0;
      }

      setSelectedCell(row, column);
   }
}

void SelectionGrid::mousePressEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      // Abort if the left mouse button is not pressed
      if (pEvent->button() != Qt::LeftButton)
      {
         return;
      }

      // Select the cell at the current mouse location
      QPoint ptMouse = pEvent->pos();
      setSelectedCell(ptMouse);
   }
}

void SelectionGrid::mouseMoveEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      bool bTracking = getCellTracking();
      if (bTracking == false)
      {
         if (pEvent->buttons() != Qt::LeftButton)
         {
            return;
         }
      }

      // Select the cell at the current mouse location
      QPoint ptMouse = pEvent->pos();
      setSelectedCell(ptMouse);
   }
}

void SelectionGrid::setCellWidth(int iWidth)
{
   if (iWidth < 5)
   {
      miCellWidth = 5;
   }
   else
   {
      miCellWidth = iWidth;
   }

   // Update the description label location
   updateDescriptionLocation();
}

int SelectionGrid::getCellWidth() const
{
   return miCellWidth;
}

void SelectionGrid::setCellHeight(int iHeight)
{
   if (iHeight < 5)
   {
      miCellHeight = 5;
   }
   else
   {
      miCellHeight = iHeight;
   }

   // Update the description label location
   updateDescriptionLocation();
}

int SelectionGrid::getCellHeight() const
{
   return miCellHeight;
}

void SelectionGrid::setBorderSpacing(int iSpacing)
{
   if (iSpacing < 2)
   {
      miBorderSpacing = 0;
   }
   else
   {
      miBorderSpacing = iSpacing;
   }
}

int SelectionGrid::getBorderSpacing() const
{
   return miBorderSpacing;
}

void SelectionGrid::setCellSpacing(int iSpacing)
{
   if (iSpacing < 2)
   {
      miCellSpacing = 0;
   }
   else
   {
      miCellSpacing = iSpacing;
   }
}

int SelectionGrid::getCellSpacing() const
{
   return miCellSpacing;
}

void SelectionGrid::setSelectedCell(int row, int column)
{
   if ((row == mSelectedRow) && (column == mSelectedColumn))
   {
      return;
   }

   if (row < getNumRows())
   {
      mSelectedRow = row;
   }

   if (column < getNumColumns())
   {
      mSelectedColumn = column;
   }

   emit cellSelected(mSelectedRow, mSelectedColumn);
}

void SelectionGrid::getSelectedCell(int& row, int& column) const
{
   row = mSelectedRow;
   column = mSelectedColumn;
}

void SelectionGrid::addDescriptionLabel()
{
   if (mpDescription == NULL)
   {
      QRect rcLastCell = getCellRect(miRows - 1, miColumns - 1);

      int iLeft = miBorderSpacing;
      int iRight = rcLastCell.right() + 1;
      int iTop = rcLastCell.bottom() + miCellSpacing + 1;
      int iBottom = iTop + 19;

      mpDescription = new QLabel(this);
      mpDescription->setFrameStyle(QFrame::Panel | QFrame::Sunken);
      mpDescription->setGeometry(iLeft, iTop, iRight - iLeft, iBottom - iTop);
      mpDescription->setAlignment(Qt::AlignCenter);
   }
}

void SelectionGrid::setDescriptionText(const QString& strText)
{
   if (mpDescription != NULL)
   {
      mpDescription->setText(strText);
   }
}

QString SelectionGrid::getDescriptionText()
{
   QString strDescription;
   if (mpDescription != NULL)
   {
      strDescription = mpDescription->text();
   }

   return strDescription;
}

void SelectionGrid::updateDescriptionLocation()
{
   if (mpDescription != NULL)
   {
      QRect rcLastCell = getCellRect(miRows - 1, miColumns - 1);

      int iLeft = miBorderSpacing;
      int iRight = rcLastCell.right() + 1;
      int iTop = rcLastCell.bottom() + miCellSpacing + 1;
      int iBottom = iTop + 19;

      mpDescription->setGeometry(iLeft, iTop, iRight - iLeft, iBottom - iTop);
   }
}

void SelectionGrid::hitTest(const QPoint& ptMouse, int& iRow, int& iColumn)
{
   iRow = -1;
   iColumn = -1;

   for (int i = 0; i < miRows; i++)
   {
      for (int j = 0; j < miColumns; j++)
      {
         QRect rcCell = getCellRect(i, j);
         if (rcCell.contains(ptMouse) == TRUE)
         {
            iRow = i;
            iColumn = j;
         }
      }
   }
}

QRect SelectionGrid::getCellRect(int iRow, int iColumn)
{
   int iLeft = miBorderSpacing + (miCellWidth * iColumn) + (miCellSpacing * iColumn);
   int iTop = miBorderSpacing + (miCellHeight * iRow) + (miCellSpacing * iRow);

   return QRect(iLeft, iTop, miCellWidth, miCellHeight);
}

void SelectionGrid::drawFocusRect(QPainter* p)
{
   if ((mSelectedRow >= 0) && (mSelectedColumn >= 0))
   {
      QRect rcCell = getCellRect(mSelectedRow, mSelectedColumn);
      rcCell.adjust(0, 0, -1, -1);     // Shrink the rectangle by one to account for the
                                       // pen width when the painter draws the rectangle

      int iLeft = rcCell.left();
      int iTop = rcCell.top();
      int iRight = rcCell.right();
      int iBottom = rcCell.bottom();

      QRect rcOuterBorder;
      rcOuterBorder.setCoords(iLeft - 1, iTop - 1, iRight + 1, iBottom + 1);

      QRect rcInnerBorder;
      rcInnerBorder.setCoords(iLeft + 1, iTop + 1, iRight - 1, iBottom - 1);

      p->setPen(Qt::black);
      p->drawRect(rcOuterBorder);
      p->drawRect(rcInnerBorder);

      p->setPen(Qt::white);
      p->drawRect(rcCell);
   }
}

QSize SelectionGrid::sizeHint() const
{
   int iWidth = (miBorderSpacing * 2) + (miCellWidth * miColumns) + (miCellSpacing * (miColumns - 1));
   int iHeight = (miBorderSpacing * 2) + (miCellHeight * miRows) + (miCellSpacing * (miRows - 1));

   if (mpDescription != NULL)
   {
      int iLabelHeight = mpDescription->height();
      iHeight += miCellSpacing + iLabelHeight;
   }

   return QSize(iWidth, iHeight);
}

void SelectionGrid::setSelectedCell(const QPoint& widgetPos)
{
   // Get the row and column from the widget coordinate
   int iRow = -1;
   int iColumn = -1;
   hitTest(widgetPos, iRow, iColumn);

   // Select the cell
   if ((iRow != -1) && (iColumn != -1))
   {
      setSelectedCell(iRow, iColumn);
   }
}
