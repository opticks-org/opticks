/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SELECTIONGRID_H
#define SELECTIONGRID_H

#include <QtCore/QPoint>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

class SelectionGrid : public QWidget
{
   Q_OBJECT

public:
   SelectionGrid(QWidget* parent = 0);
   ~SelectionGrid();

   virtual void setNumRows(int iRows);
   int getNumRows() const;
   virtual void setNumColumns(int iColumns);
   int getNumColumns() const;

   void setCellTracking(bool bEnable);
   bool getCellTracking() const;

   QSize sizeHint() const;

protected:
   void keyPressEvent(QKeyEvent* pEvent);
   void mousePressEvent(QMouseEvent* pEvent);
   void mouseMoveEvent(QMouseEvent* pEvent);

   virtual void setCellWidth(int iWidth);
   int getCellWidth() const;
   virtual void setCellHeight(int iHeight);
   int getCellHeight() const;
   virtual void setBorderSpacing(int iSpacing);
   int getBorderSpacing() const;
   virtual void setCellSpacing(int iSpacing);
   int getCellSpacing() const;

   void setSelectedCell(int row, int column);
   void getSelectedCell(int& row, int& column) const;

   void addDescriptionLabel();
   void setDescriptionText(const QString& strText);
   QString getDescriptionText();

   void hitTest(const QPoint& ptMouse, int& iRow, int& iColumn);
   QRect getCellRect(int iRow, int iColumn);
   void drawFocusRect(QPainter* p);

signals:
   void cellSelected(int row, int column);

private:
   SelectionGrid(const SelectionGrid& rhs);
   SelectionGrid& operator=(const SelectionGrid& rhs);
   int miRows;
   int miColumns;

   bool mbTracking;
   int miCellWidth;
   int miCellHeight;
   int miBorderSpacing;
   int miCellSpacing;

   QLabel* mpDescription;
   int mSelectedRow;
   int mSelectedColumn;

   void setSelectedCell(const QPoint& widgetPos);
   void updateDescriptionLocation();
};

#endif
