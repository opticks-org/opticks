/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include "PixmapGrid.h"

using namespace std;

PixmapGrid::PixmapGrid(QWidget* parent) :
   SelectionGrid(parent),
   mCellBackgroundColor(Qt::white)
{
   connect(this, SIGNAL(cellSelected(int, int)), this, SLOT(setSelectedPixmap(int, int)));
}

PixmapGrid::~PixmapGrid()
{
}

void PixmapGrid::setNumRows(int iRows)
{
   SelectionGrid::setNumRows(iRows);
   updatePixmaps();
}

void PixmapGrid::setNumColumns(int iColumns)
{
   SelectionGrid::setNumColumns(iColumns);
   updatePixmaps();
}

bool PixmapGrid::setPixmap(int iRow, int iColumn, const QPixmap& pix, const QString& identifier, const QString& strDescription)
{
   // Check for an invalid input cell
   int iNumRows = getNumRows();
   if ((iRow < 0) || (iRow >= iNumRows))
   {
      return false;
   }

   int iNumColumns = getNumColumns();
   if ((iColumn < 0) || (iColumn >= iNumColumns))
   {
      return false;
   }

   // Set the cell pixmap
   int iIndex = (iNumColumns * iRow) + iColumn;
   if (iIndex < 0)
   {
      return false;
   }

   CellPixmap currentPix;
   currentPix.mPixmap = pix;
   currentPix.mDescription = strDescription;
   currentPix.mIdentifier = identifier;

   mPixmaps[iIndex] = currentPix;

   // Update the cell size if necessary
   int iPixmapMargin = 10;

   int iWidth = pix.width() + iPixmapMargin;
   int iHeight = pix.height() + iPixmapMargin;

   int iCellWidth = getCellWidth();
   int iCellHeight = getCellHeight();

   if (iWidth > iCellWidth)
   {
      setCellWidth(iWidth);
   }

   if (iHeight > iCellHeight)
   {
      setCellHeight(iHeight);
   }

   // Add the description label if a description is set
   if (strDescription.isEmpty() == false)
   {
      addDescriptionLabel();
   }

   return true;
}

QPixmap PixmapGrid::getPixmap(int iRow, int iColumn) const
{
   CellPixmap selectedPix = cellToPixmap(iRow, iColumn);
   return selectedPix.mPixmap;
}

QPixmap PixmapGrid::getPixmap(const QString& identifier) const
{
   int iRow, iColumn;
   identifierToCell(identifier, iRow, iColumn);
   CellPixmap selectedPix = cellToPixmap(iRow, iColumn);
   return selectedPix.mPixmap;
}

QString PixmapGrid::getPixmapDescription(int iRow, int iColumn) const
{
   if ((iRow >= 0) && (iColumn >= 0))
   {
      int numColumns = getNumColumns();
      int index = (numColumns * iRow) + iColumn;

      map<int, CellPixmap>::const_iterator iter = mPixmaps.find(index);
      if (iter != mPixmaps.end())
      {
         CellPixmap currentCell = iter->second;
         return currentCell.mDescription;
      }
   }

   return QString();
}

QString PixmapGrid::getPixmapDescription(const QString& identifier) const
{
   int iRow, iColumn;
   identifierToCell(identifier, iRow, iColumn);
   CellPixmap selectedPix = cellToPixmap(iRow, iColumn);
   return selectedPix.mDescription;
}

QString PixmapGrid::getPixmapIdentifier(int iRow, int iColumn) const
{
   if ((iRow >= 0) && (iColumn >= 0))
   {
      int numColumns = getNumColumns();
      int index = (numColumns * iRow) + iColumn;

      map<int, CellPixmap>::const_iterator iter = mPixmaps.find(index);
      if (iter != mPixmaps.end())
      {
         CellPixmap currentCell = iter->second;
         return currentCell.mIdentifier;
      }
   }

   return QString();
}

void PixmapGrid::setSelectedPixmap(const QString& identifier)
{
   int row = -1;
   int column = -1;
   identifierToCell(identifier, row, column);

   setSelectedPixmap(row, column);
}

QPixmap PixmapGrid::getSelectedPixmap() const
{
   return mSelectedCell.mPixmap;
}

QString PixmapGrid::getSelectedPixmapDescription() const
{
   return mSelectedCell.mDescription;
}

QString PixmapGrid::getSelectedPixmapIdentifier() const
{
   return mSelectedCell.mIdentifier;
}

void PixmapGrid::setCellBackgroundColor(const QColor& backgroundColor)
{
   bool bValid = backgroundColor.isValid();
   if (bValid == true)
   {
      mCellBackgroundColor = backgroundColor;
   }
}

QColor PixmapGrid::getCellBackgroundColor() const
{
   return mCellBackgroundColor;
}

void PixmapGrid::paintEvent(QPaintEvent* pEvent)
{
   QRect rcWidget = rect();
   QPixmap pix(rcWidget.size());
   pix.fill(this, 0, 0);

   QPainter p(&pix);
   p.setPen(QPen(Qt::black, 2));

   int iNumRows = getNumRows();
   int iNumColumns = getNumColumns();

   int iIndex = 0;
   for (int i = 0; i < iNumRows; i++)
   {
      for (int j = 0; j < iNumColumns; j++)
      {
         QBrush* pBrush = new QBrush(mCellBackgroundColor);
         QRect rcCell = getCellRect(i, j);

         qDrawWinPanel(&p, rcCell.x(), rcCell.y(), rcCell.width(), rcCell.height(), palette(), true, pBrush);

         int iLeft = rcCell.left();
         int iTop = rcCell.top();
         int iRight = rcCell.right();
         int iBottom = rcCell.bottom();

         map<int, CellPixmap>::const_iterator iter = mPixmaps.find(iIndex);
         if (iter != mPixmaps.end())
         {
            CellPixmap cell = iter->second;

            QPixmap currentPix = cell.mPixmap;
            if (currentPix.isNull() == false)
            {
               int iWidth = currentPix.width();
               int iHeight = currentPix.height();

               QPoint ptTopLeft(((iLeft + iRight + 1) / 2) - (iWidth / 2), ((iTop + iBottom + 1) / 2) - (iHeight / 2));
               p.drawPixmap(ptTopLeft, currentPix);
            }
         }

         ++iIndex;
      }
   }

   drawFocusRect(&p);
   p.end();

   p.begin(this);
   p.drawPixmap(rcWidget.topLeft(), pix);
}

PixmapGrid::CellPixmap PixmapGrid::cellToPixmap(int iRow, int iColumn) const
{
   if ((iRow >= 0) && (iColumn >= 0))
   {
      int numColumns = getNumColumns();
      int index = (numColumns * iRow) + iColumn;

      map<int, CellPixmap>::const_iterator iter = mPixmaps.find(index);
      if (iter != mPixmaps.end())
      {
         CellPixmap currentCell = iter->second;
         return currentCell;
      }
   }

   return CellPixmap();
}

void PixmapGrid::pixmapToCell(const QPixmap& pix, int& iRow, int& iColumn) const
{
   iRow = -1;
   iColumn = -1;

   map<int, CellPixmap>::const_iterator iter;
   for (iter = mPixmaps.begin(); iter != mPixmaps.end(); ++iter)
   {
      CellPixmap currentCell = iter->second;

      QImage currentImage = currentCell.mPixmap.toImage();
      QImage image = pix.toImage();

      if (currentImage == image)
      {
         int index = iter->first;
         int numColumns = getNumColumns();

         iRow = index / numColumns;
         iColumn = index % numColumns;
         break;
      }
   }
}

void PixmapGrid::identifierToCell(const QString& identifier, int& iRow, int& iColumn) const
{
   iRow = -1;
   iColumn = -1;

   map<int, CellPixmap>::const_iterator iter;
   for (iter = mPixmaps.begin(); iter != mPixmaps.end(); ++iter)
   {
      CellPixmap currentCell = iter->second;
      if (currentCell.mIdentifier == identifier)
      {
         int index = iter->first;
         int numColumns = getNumColumns();

         iRow = index / numColumns;
         iColumn = index % numColumns;
         break;
      }
   }
}


void PixmapGrid::updatePixmaps()
{
   int iRows = getNumRows();
   int iColumns = getNumColumns();
   int iPixmaps = iRows * iColumns;

   map<int, CellPixmap> tempMap = mPixmaps;

   map<int, CellPixmap>::iterator iter;
   for (iter = tempMap.begin(); iter != tempMap.end(); ++iter)
   {
      int iIndex = iter->first;
      if ((iIndex < 0) || (iIndex > iPixmaps - 1))
      {
         mPixmaps.erase(iIndex);

         // If the pixmap being erased is the selected pixmap, clear the selection
         int row = iIndex / iColumns;
         int column = iIndex % iColumns;
         int selectedRow = -1;
         int selectedColumn = -1;
         getSelectedCell(selectedRow, selectedColumn);

         if ((selectedRow == row) && (selectedColumn == column))
         {
            setSelectedCell(-1, -1);
         }
      }
   }
}

void PixmapGrid::updateDescription()
{
   QString strDescription = getSelectedPixmapDescription();
   setDescriptionText(strDescription);
}

void PixmapGrid::setSelectedPixmap(int iRow, int iColumn)
{
   CellPixmap selectedCell;
   if ((iRow >= 0) && (iColumn >= 0))
   {
      int numColumns = getNumColumns();
      int index = (numColumns * iRow) + iColumn;

      map<int, CellPixmap>::const_iterator iter = mPixmaps.find(index);
      if (iter == mPixmaps.end())
      {
         int orgRow = -1, orgCol = -1;        
         identifierToCell(mSelectedCell.mIdentifier, orgRow, orgCol);
         disconnect(this, SIGNAL(cellSelected(int, int)), this, SLOT(setSelectedPixmap(int, int)));
         setSelectedCell(orgRow, orgCol);
         connect(this, SIGNAL(cellSelected(int, int)), this, SLOT(setSelectedPixmap(int, int)));
         return;
      }
      else
      {
         selectedCell = iter->second;
      }
   }

   // Check for an unchanged pixmap
   QImage image = selectedCell.mPixmap.toImage();
   QImage selectedImage = mSelectedCell.mPixmap.toImage();

   if (image == selectedImage)
   {
      if (selectedImage.isNull() == false)
      {
         return;
      }

      QString currentIdentifier = getSelectedPixmapIdentifier();
      QString selectedIdentifier = getPixmapIdentifier(iRow, iColumn);
      if (selectedIdentifier == currentIdentifier)
      {
         return;
      }
   }

   mSelectedCell = selectedCell;

   setSelectedCell(iRow, iColumn);
   repaint();
   updateDescription();
   emit pixmapSelected(mSelectedCell.mPixmap);
   emit pixmapSelected(mSelectedCell.mIdentifier);
}
