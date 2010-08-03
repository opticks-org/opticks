/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include "ColorGrid.h"

ColorGrid::ColorGrid(QWidget* parent) :
   SelectionGrid(parent)
{
   setNumRows(5);
   setNumColumns(5);
   setCellWidth(20);
   setCellHeight(20);
   setBorderSpacing(2);
   setCellSpacing(2);

   mColors.push_back(Qt::black);
   mColors.push_back(Qt::darkRed);
   mColors.push_back(Qt::darkGreen);
   mColors.push_back(Qt::darkYellow);
   mColors.push_back(QColor(128, 0, 255));

   mColors.push_back(Qt::darkBlue);
   mColors.push_back(Qt::darkMagenta);
   mColors.push_back(Qt::darkCyan);
   mColors.push_back(Qt::lightGray);
   mColors.push_back(QColor(255, 0, 128));

   mColors.push_back(QColor(0, 255, 128));
   mColors.push_back(QColor(128, 255, 0));
   mColors.push_back(QColor(255, 128, 0));
   mColors.push_back(Qt::gray);
   mColors.push_back(QColor(0, 128, 255));

   mColors.push_back(Qt::darkGray);
   mColors.push_back(Qt::red);
   mColors.push_back(Qt::green);
   mColors.push_back(Qt::yellow);
   mColors.push_back(QColor(128, 128, 255));

   mColors.push_back(Qt::blue);
   mColors.push_back(Qt::magenta);
   mColors.push_back(Qt::cyan);
   mColors.push_back(Qt::white);
   mColors.push_back(QColor(255, 128, 128));

   connect(this, SIGNAL(cellSelected(int, int)), this, SLOT(setSelectedColor(int, int)));
}

ColorGrid::~ColorGrid()
{
}

void ColorGrid::setSelectedColor(const QColor& clrSelect)
{
   if (clrSelect == mSelectedColor)
   {
      return;
   }

   mSelectedColor = clrSelect;

   int iRow = -1;
   int iColumn = -1;
   colorToCell(clrSelect, iRow, iColumn);

   setSelectedCell(iRow, iColumn);
   repaint();

   if ((iRow != -1) && (iColumn != -1))
   {
      emit colorSelected(mSelectedColor);
   }
}

QColor ColorGrid::getSelectedColor() const
{
   return mSelectedColor;
}

void ColorGrid::paintEvent(QPaintEvent* e)
{
   QRect rcWidget = rect();
   QPixmap pix(rcWidget.size());
   pix.fill(this, 0, 0);

   QPainter p(&pix);

   int iNumRows = getNumRows();
   int iNumColumns = getNumColumns();

   int iIndex = 0;

   for (int i = 0; i < iNumRows; i++)
   {
      for (int j = 0; j < iNumColumns; j++)
      {
         QBrush* pBrush = new QBrush(mColors[iIndex]);
         QRect rcCell = getCellRect(i, j);

         qDrawWinPanel(&p, rcCell.x(), rcCell.y(), rcCell.width(), rcCell.height(),
            palette(), true, pBrush);

         iIndex++;
      }
   }

   drawFocusRect(&p);

   p.end();

   p.begin(this);
   p.drawPixmap(rcWidget.topLeft(), pix);
}

QColor ColorGrid::cellToColor(int iRow, int iColumn)
{
   QColor cellColor;
   if ((iRow >= 0) && (iColumn >= 0))
   {
      // Get the number of grid columns
      int iNumColumns = getNumColumns();

      // Calculate the color list index
      int iIndex = (iNumColumns * iRow) + iColumn;

      // Get the selected color from the list
      if (iIndex != -1)
      {
         cellColor = mColors[iIndex];
      }
   }

   return cellColor;
}

void ColorGrid::colorToCell(const QColor& clrCell, int& iRow, int& iColumn)
{
   // Get the number of known colors
   int iNumColors = mColors.size();

   // Cycle through the color list and compare the input color
   int iIndex = -1;
   for (int i = 0; i < iNumColors; i++)
   {
      QColor clrGridCell = mColors[i];
      if (clrCell == clrGridCell)
      {
         // Set the color list index
         iIndex = i;
      }
   }

   // Get the number of columns
   int iNumColumns = getNumColumns();

   // Calculate the row and column number
   if (iIndex != -1)
   {
      iRow = iIndex / iNumColumns;
      iColumn = iIndex % iNumColumns;
   }
}

void ColorGrid::setSelectedColor(int iRow, int iColumn)
{
   QColor selectedColor = cellToColor(iRow, iColumn);
   setSelectedColor(selectedColor);
}
