/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COLORGRID_H
#define COLORGRID_H

#include <QtGui/QColor>

#include "SelectionGrid.h"

#include <vector>

class ColorGrid : public SelectionGrid
{
   Q_OBJECT

public:
   ColorGrid(QWidget* parent = 0);
   ~ColorGrid();

   QColor getSelectedColor() const;

public slots:
   void setSelectedColor(const QColor& clrSelect);

signals:
   void colorSelected(const QColor& clrSelected);

protected:
   void paintEvent(QPaintEvent* e);

   QColor cellToColor(int iRow, int iColumn);
   void colorToCell(const QColor& clrCell, int& iRow, int& iColumn);

protected slots:
   void setSelectedColor(int iRow, int iColumn);

private:
   ColorGrid(const ColorGrid& rhs);
   ColorGrid& operator=(const ColorGrid& rhs);
   std::vector<QColor> mColors;
   QColor mSelectedColor;
};

#endif
