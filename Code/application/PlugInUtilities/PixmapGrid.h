/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PIXMAPGRID_H
#define PIXMAPGRID_H

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QPixmap>

#include "SelectionGrid.h"

#include <map>

class PixmapGrid : public SelectionGrid
{
   Q_OBJECT

public:
   PixmapGrid(QWidget* parent = 0);
   ~PixmapGrid();

   void setNumRows(int iRows);
   void setNumColumns(int iColumns);

   bool setPixmap(int iRow, int iColumn, const QPixmap& pix, const QString& identifier,
      const QString& strDescription = QString());
   QPixmap getPixmap(int iRow, int iColumn) const;
   QPixmap getPixmap(const QString& identifier) const;
   QString getPixmapDescription(int iRow, int iColumn) const;
   QString getPixmapDescription(const QString& identifier) const;
   QString getPixmapIdentifier(int iRow, int iColumn) const;

   void setSelectedPixmap(const QString& identifier);
   QPixmap getSelectedPixmap() const;
   QString getSelectedPixmapDescription() const;
   QString getSelectedPixmapIdentifier() const;

   void setCellBackgroundColor(const QColor& backgroundColor);
   QColor getCellBackgroundColor() const;

signals:
   void pixmapSelected(const QPixmap& selectedPix);
   void pixmapSelected(const QString& selectedIdentifier);

private:
   class CellPixmap
   {
   public:
      QPixmap mPixmap;
      QString mDescription;
      QString mIdentifier;
   };

protected:
   void paintEvent(QPaintEvent* pEvent);

   CellPixmap cellToPixmap(int iRow, int iColumn) const;
   void pixmapToCell(const QPixmap& pix, int& iRow, int& iColumn) const;
   void identifierToCell(const QString& identifier, int& iRow, int& iColumn) const;

protected slots:
   void setSelectedPixmap(int iRow, int iColumn);

private:
   PixmapGrid(const PixmapGrid& rhs);
   PixmapGrid& operator=(const PixmapGrid& rhs);

   std::map<int, CellPixmap> mPixmaps;
   CellPixmap mSelectedCell;
   QColor mCellBackgroundColor;

   void updatePixmaps();
   void updateDescription();
};

#endif
