/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <QtGui/QBitmap>
#include <QtGui/QColorDialog>
#include <QtGui/QCompleter>
#include <QtGui/QDirModel>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include "CustomTreeWidget.h"
#include "IconImages.h"

CustomTreeWidget::CustomTreeWidget(QWidget* parent) :
   QTreeWidget(parent),
   mbFullColor(false),
   miColorWidth(50),
   mHorizontalGridlines(false),
   mVerticalGridlines(false),
   mpEdit(NULL),
   mpBrowse(NULL),
   mBrowseDir(QString()),
   mpCombo(NULL),
   mpSpin(NULL)
{
   // Initialization
   setUniformRowHeights(true);
   setIconSize(QSize(1000, 20));    // Ensure that the item icons are the appropriate size
                                    // and not scaled to the default small icon size

   QItemDelegate* pDelegate = new CustomTreeWidgetItemDelegate();
   setItemDelegate(pDelegate);

   // Connections
   QHeaderView* pHeader = header();
   if (pHeader != NULL)
   {
      connect(pHeader, SIGNAL(sectionResized(int, int, int)), this, SLOT(columnWidthChanged(int, int, int)));
   }

   connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(closeEdit()));
   connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(closeCombo()));
   connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(closeSpin()));
}

CustomTreeWidget::~CustomTreeWidget()
{
}

void CustomTreeWidget::clear()
{
   closeActiveCellWidget(false);

   mCellWidgets.clear();
   mComboBoxes.clear();
   mSpinBoxes.clear();
   mChecks.clear();
   mColors.clear();
   mFullCellEdit.clear();

   QTreeWidget::clear();
}

void CustomTreeWidget::mousePressEvent(QMouseEvent* e)
{
   closeActiveCellWidget(true);

   if (e == NULL)
   {
      return;
   }

   if (e->button() == Qt::LeftButton)
   {
      QPoint ptClick = e->pos();

      // Get the list view item
      QTreeWidgetItem* pItem = itemAt(ptClick);
      if (pItem != NULL)
      {
         bool bSelected = isItemSelected(pItem);
         if (bSelected == true)
         {
            // Use the contents x-coordinate to get the column index
            int iX = e->x();
            int iColumn = getColumnIndex(iX);

            // Check if the pixmap was clicked
            QRect rcPixmap = getCellPixmapRect(pItem, iColumn);
            if (rcPixmap.contains(ptClick) == true)
            {
               CellLocation cell;
               cell.pItem = pItem;
               cell.iColumn = iColumn;

               QMap<CellLocation, CheckState>::Iterator iterCheck;
               iterCheck = mChecks.find(cell);
               if (iterCheck != mChecks.end())
               {
                  CheckState eState = UNCHECKED;
                  eState = getCellCheckState(pItem, iColumn);
                  if ((eState == UNCHECKED) || (eState == SEMI_CHECKED))
                  {
                     setCellCheckState(pItem, iColumn, CHECKED);
                  }
                  else
                  {
                     setCellCheckState(pItem, iColumn, UNCHECKED);
                  }
               }

               QMap<CellLocation, QColor>::Iterator iterColor = mColors.find(cell);
               if (iterColor != mColors.end())
               {
                  QColor clrCurrent = getCellColor(pItem, iColumn);
                  QColor clrNew = QColorDialog::getColor(clrCurrent, this);
                  setCellColor(pItem, iColumn, clrNew);
               }
            }

            // Check if the edit widget was clicked
            QRect rcWidget = getCellWidgetRect(pItem, iColumn);
            if (rcWidget.contains(ptClick) == true)
            {
               activateCellWidget(pItem, iColumn);
            }
         }
      }
   }

   QTreeWidget::mousePressEvent(e);
}

void CustomTreeWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   if (e->button() == Qt::LeftButton)
   {
      QTreeWidgetItem* pItem = itemAt(e->pos());
      if (pItem != NULL)
      {
         bool bSelected = isItemSelected(pItem);
         if (bSelected == true)
         {
            mousePressEvent(e);
         }
      }
   }

   QTreeWidget::mouseDoubleClickEvent(e);
}

void CustomTreeWidget::keyPressEvent(QKeyEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   switch (e->key())
   {
      case Qt::Key_Delete:
         emit deleteKeyPressed();
         break;

      case Qt::Key_Escape:
      {
         closeActiveCellWidget(false);
         break;
      }

      case Qt::Key_Return:
      case Qt::Key_Enter:
      {
         closeActiveCellWidget(true);
         break;
      }

      case Qt::Key_Down:
      case Qt::Key_Up:
      {
         QModelIndex cellIndex = currentIndex();
         int iColumn = cellIndex.column();

         closeActiveCellWidget(true);
         QTreeWidget::keyPressEvent(e);
         repaint();

         QTreeWidgetItem* pItem = currentItem();
         if (pItem != NULL)
         {
            cellIndex = indexFromItem(pItem, iColumn);
            setCurrentIndex(cellIndex);

            activateCellWidget(pItem, iColumn);
         }

         break;
      }

      case Qt::Key_PageDown:
      case Qt::Key_PageUp:
      {
         closeActiveCellWidget(true);
         QTreeWidget::keyPressEvent(e);
         repaint();
         break;
      }

      default:
         QTreeWidget::keyPressEvent(e);
         break;
   }
}

void CustomTreeWidget::activateCellWidget(QTreeWidgetItem* pItem, int iColumn)
{
   if (pItem == NULL)
   {
      return;
   }

   // Check if the selected cell is editable
   WidgetType eType = getCellWidgetType(pItem, iColumn);
   if (eType == NO_WIDGET)
   {
      return;
   }

   // Scroll vertically to display the cell if necessary
   scrollToItem(pItem);

   // Get the edit widget rectangle
   QRect rcWidget = getCellWidgetRect(pItem, iColumn);

   // Scroll horizontally to display the cell if necessary
   int iScrollLength = 0;

   QPoint cellCenter = rcWidget.center();
   QModelIndex cellIndex = indexAt(cellCenter);

   QRect rcCellVisible = visualRect(cellIndex);
   if (rcWidget.right() > rcCellVisible.right())
   {
      iScrollLength = rcWidget.right() - rcCellVisible.right() - 1;
   }

   else if (rcWidget.left() < 0)
   {
      iScrollLength = rcWidget.left() - 2;
   }

   scroll(iScrollLength, 0);
   rcWidget.setLeft(rcWidget.left() - iScrollLength);
   rcWidget.setRight(rcWidget.right() - iScrollLength);

   // Shorten the cell rect to ensure the gridlines are displayed
   if (areGridlinesShown(Qt::Vertical) == true)
   {
      rcWidget.setRight(rcWidget.right() - 1);
   }

   if (areGridlinesShown(Qt::Horizontal) == true)
   {
      rcWidget.setBottom(rcWidget.bottom() - 1);
   }

   // Get the cell text
   QString strCellText = pItem->text(iColumn);

   // Invoke the edit widget
   switch (eType)
   {
      case LINE_EDIT:         // Fall through to the next case
      case BROWSE_FILE_EDIT:  // Fall through to the next case
      case BROWSE_DIR_EDIT:   // Fall through to the next case
      case SHORTCUT_EDIT:
      {
         // Invoke an edit box and select the text
         if (mpEdit == NULL)
         {
            mpEdit = new QLineEdit(viewport());
            if (mpEdit != NULL)
            {
               mpEdit->setFrame(false);
               mpEdit->installEventFilter(this);
               connect(mpEdit, SIGNAL(returnPressed()), this, SLOT(acceptEditText()));
            }
         }

         if (mpEdit != NULL)
         {
            mpEdit->setText(strCellText);
            mpEdit->setGeometry(rcWidget);
            mpEdit->show();
            mpEdit->setFocus();
            mpEdit->selectAll();

            if ((eType == BROWSE_FILE_EDIT) || (eType == BROWSE_DIR_EDIT))
            {
               QCompleter *pCompleter = new QCompleter(this);
               pCompleter->setModel(new QDirModel(QStringList(), QDir::NoDotAndDotDot | 
                  ((eType == BROWSE_DIR_EDIT) ? (QDir::Dirs | QDir::Drives) : QDir::AllEntries),
                  QDir::DirsFirst, pCompleter));
               mpEdit->setCompleter(pCompleter);
               if (mpBrowse == NULL)
               {
                  QPixmap pixOpen(IconImages::OpenIcon);
                  pixOpen.setMask(pixOpen.createHeuristicMask());
                  QIcon icnBrowse(pixOpen);

                  mpBrowse = new QPushButton(icnBrowse, QString(), viewport());
                  if (mpBrowse != NULL)
                  {
                     mpBrowse->setFixedWidth(25);
                     connect(mpBrowse, SIGNAL(clicked()), this, SLOT(browse()));
                  }
               }

               if (mpBrowse != NULL)
               {
                  QRect rcButton = rcWidget;
                  rcButton.setLeft(rcWidget.right() - mpBrowse->width());

                  mpBrowse->setGeometry(rcButton);
                  mpBrowse->show();
               }
            }
         }

         break;
      }

      case COMBO_BOX:
      {
         mpCombo = getComboBox(pItem, iColumn);
         if (mpCombo != NULL)
         {
            int index = mpCombo->findText(strCellText);
            if (index == -1)
            {
               for (int i = 0; i < mpCombo->count(); ++i)
               {
                  QIcon comboIcon = mpCombo->itemIcon(i);
                  if (comboIcon.isNull() == false)
                  {
                     QIcon itemIcon = pItem->icon(iColumn);

                     QRect rcCell = getCellRect(pItem, iColumn);
                     QPixmap comboPix = comboIcon.pixmap(rcCell.width(), rcCell.height());
                     QPixmap itemPix = itemIcon.pixmap(rcCell.width(), rcCell.height());

                     QImage comboImage = comboPix.toImage();
                     QImage itemImage = itemPix.toImage();

                     if (comboImage == itemImage)
                     {
                        index = i;
                        break;
                     }
                  }
               }
            }

            if (index != -1)
            {
               mpCombo->setCurrentIndex(index);
            }
            else if (mpCombo->isEditable() == true)
            {
               mpCombo->setEditText(strCellText);
            }

            mpCombo->setGeometry(rcWidget);
            mpCombo->setFocus();
            mpCombo->show();
         }

         break;
      }

      case SPIN_BOX:
      {
         mpSpin = getSpinBox(pItem, iColumn);
         if (mpSpin != NULL)
         {
            int iValue = 0;
            iValue = strCellText.toInt();
            mpSpin->setValue(iValue);

            mpSpin->setGeometry(rcWidget);
            mpSpin->setFocus();
            mpSpin->show();
         }

         break;
      }
   }
}

void CustomTreeWidget::columnWidthChanged(int iColumn, int iOldWidth, int iNewWidth)
{
   QTreeWidgetItem* pItem = currentItem();
   if (pItem != NULL)
   {
      QModelIndex cellIndex = currentIndex();
      int iCurrentColumn = cellIndex.column();

      QRect rcWidget = getCellWidgetRect(pItem, iCurrentColumn);

      // Resize the edit box
      if (mpEdit != NULL)
      {
         if (mpEdit->isVisible() == true)
         {
            mpEdit->setGeometry(rcWidget);

            if (mpBrowse != NULL)
            {
               if (mpBrowse->isVisible() == true)
               {
                  QRect rcButton = rcWidget;
                  rcButton.setLeft(rcWidget.right() - mpBrowse->width());

                  mpBrowse->setGeometry(rcButton);
               }
            }
         }
      }

      // Resize the combo box
      if (mpCombo != NULL)
      {
         mpCombo->setGeometry(rcWidget);
      }

      // Resize the spin box
      if (mpSpin != NULL)
      {
         mpSpin->setGeometry(rcWidget);
      }
   }

   // Resize the color pixmaps
   if (mbFullColor == true)
   {
      QTreeWidgetItemIterator iter(this);
      while (*iter != NULL)
      {
         QTreeWidgetItem* pCurrentItem = *iter;
         if (pCurrentItem != NULL)
         {
            QColor clrCell = getCellColor(pCurrentItem, iColumn);
            if (clrCell.isValid() == true)
            {
               setCellColor(pCurrentItem, iColumn, clrCell);
            }
         }

         ++iter;
      }
   }
}

bool CustomTreeWidget::setCellCheckState(QTreeWidgetItem* pItem, int iColumn, CheckState eState)
{
   if (pItem == NULL)
   {
      return false;
   }

   QPixmap checkStatePix;
   if (eState == CHECKED)
   {
      checkStatePix = IconImages::CheckedIcon;
   }
   else if (eState == SEMI_CHECKED)
   {
      checkStatePix = IconImages::SemiCheckedIcon;
   }
   else
   {
      checkStatePix = IconImages::UncheckedIcon;
   }

   pItem->setIcon(iColumn, QIcon(checkStatePix));

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   if (mChecks[cell] != eState)
   {
      mChecks[cell] = eState;
      emit cellCheckChanged(pItem, iColumn);
   }

   return true;
}

CustomTreeWidget::CheckState CustomTreeWidget::getCellCheckState(QTreeWidgetItem* pItem, int iColumn) const
{
   CheckState eState = UNCHECKED;

   if (pItem == NULL)
   {
      return eState;
   }

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   QMap<CellLocation, CheckState>::const_iterator iter = mChecks.find(cell);
   if (iter != mChecks.end())
   {
      eState = iter.value();
   }

   return eState;
}

bool CustomTreeWidget::setCellColor(QTreeWidgetItem* pItem, int iColumn, QColor clrCell)
{
   if (pItem == NULL)
   {
      return false;
   }

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   if (clrCell.isValid() == true)
   {
      int iWidth = miColorWidth;
      if (mbFullColor == true)
      {
         iWidth = columnWidth(iColumn);
      }

      QPixmap pix = QPixmap(iWidth - 3, 14);
      QRect colorRect = pix.rect();
      colorRect.adjust(0, 0, -1, -1);

      QPainter p;
      p.begin(&pix);
      p.setBrush(QBrush(clrCell));
      p.setPen(QPen(Qt::black));
      p.drawRect(colorRect);
      p.end();

      pItem->setIcon(iColumn, QIcon(pix));

      if (mColors[cell] != clrCell)
      {
         mColors[cell] = clrCell;
         emit cellColorChanged(pItem, iColumn);
      }

      return true;
   }

   return false;
}

QColor CustomTreeWidget::getCellColor(QTreeWidgetItem* pItem, int iColumn) const
{
   QColor clrCell;

   if (pItem == NULL)
   {
      return clrCell;
   }

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   QMap<CellLocation, QColor>::const_iterator iter = mColors.find(cell);
   if (iter != mColors.end())
   {
      clrCell = iter.value();
   }

   return clrCell;
}

void CustomTreeWidget::setFullCellColor(bool bFullCell)
{
   mbFullColor = bFullCell;
}

bool CustomTreeWidget::getFullCellColor() const
{
   return mbFullColor;
}

void CustomTreeWidget::setColorWidth(int iWidth)
{
   miColorWidth = iWidth;
}

int CustomTreeWidget::getColorWidth() const
{
   return miColorWidth;
}

CustomTreeWidget::WidgetType CustomTreeWidget::getCellWidgetType(QTreeWidgetItem* pItem, int iColumn) const
{
   WidgetType eType = NO_WIDGET;

   if (pItem == NULL)
   {
      return eType;
   }

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   QMap<CellLocation, WidgetType>::const_iterator iter = mCellWidgets.find(cell);
   if (iter != mCellWidgets.end())
   {
      eType = iter.value();
   }

   return eType;
}

bool CustomTreeWidget::setCellWidgetType(QTreeWidgetItem* pItem, int iColumn, WidgetType eType)
{
   if ((pItem == NULL) || (iColumn == -1))
   {
      return false;
   }

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   mCellWidgets[cell] = eType;
   return true;
}

void CustomTreeWidget::setFullCellEdit(QTreeWidgetItem* pItem, int iColumn, bool bFullCell)
{
   if (pItem == NULL)
   {
      return;
   }

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   mFullCellEdit[cell] = bFullCell;
}

bool CustomTreeWidget::getFullCellEdit(QTreeWidgetItem* pItem, int iColumn) const
{
   if (pItem == NULL)
   {
      return false;
   }

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   bool bFullCell = true;

   QMap<CellLocation, bool>::const_iterator iter = mFullCellEdit.find(cell);
   if (iter != mFullCellEdit.end())
   {
      bFullCell = iter.value();
   }

   return bFullCell;
}

bool CustomTreeWidget::setComboBox(QTreeWidgetItem* pItem, int iColumn, QComboBox* pCombo)
{
   if ((pItem == NULL) || (pCombo == NULL))
   {
      return false;
   }

   WidgetType eType = getCellWidgetType(pItem, iColumn);
   if (eType != COMBO_BOX)
   {
      return false;
   }

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   pCombo->setParent(viewport());
   pCombo->hide();
   mComboBoxes[cell] = pCombo;

   return true;
}

QComboBox* CustomTreeWidget::getComboBox(QTreeWidgetItem* pItem, int iColumn) const
{
   if (pItem == NULL)
   {
      return NULL;
   }

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   QComboBox* pCombo = NULL;

   QMap<CellLocation, QComboBox*>::const_iterator iter = mComboBoxes.find(cell);
   if (iter != mComboBoxes.end())
   {
      pCombo = iter.value();
   }

   return pCombo;
}

bool CustomTreeWidget::setSpinBox(QTreeWidgetItem* pItem, int iColumn, QSpinBox* pSpin)
{
   if ((pItem == NULL) || (pSpin == NULL))
   {
      return false;
   }

   WidgetType eType = getCellWidgetType(pItem, iColumn);
   if (eType != SPIN_BOX)
   {
      return false;
   }

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   pSpin->setParent(viewport());
   pSpin->hide();
   mSpinBoxes[cell] = pSpin;

   return true;
}

QSpinBox* CustomTreeWidget::getSpinBox(QTreeWidgetItem* pItem, int iColumn) const
{
   if (pItem == NULL)
   {
      return NULL;
   }

   CellLocation cell;
   cell.pItem = pItem;
   cell.iColumn = iColumn;

   QSpinBox* pSpin = NULL;

   QMap<CellLocation, QSpinBox*>::const_iterator iter = mSpinBoxes.find(cell);
   if (iter != mSpinBoxes.end())
   {
      pSpin = iter.value();
   }

   return pSpin;
}

bool CustomTreeWidget::hitTest(QPoint ptCoord, QTreeWidgetItem* pItem, int iColumn)
{
   QTreeWidgetItem* pPointItem = itemAt(ptCoord);
   if (pPointItem == pItem)
   {
      int iX = ptCoord.x();

      int iPointColumn = getColumnIndex(iX);
      if (iPointColumn == iColumn)
      {
         return true;
      }
   }

   return false;
}

void CustomTreeWidget::setGridlinesShown(Qt::Orientations orientations, bool bShow)
{
   if (orientations & Qt::Horizontal)
   {
      mHorizontalGridlines = bShow;
   }

   if (orientations & Qt::Vertical)
   {
      mVerticalGridlines = bShow;
   }
}

bool CustomTreeWidget::areGridlinesShown(Qt::Orientation orientation) const
{
   bool bGridlines = false;
   if (orientation == Qt::Horizontal)
   {
      bGridlines = mHorizontalGridlines;
   }
   else if (orientation == Qt::Vertical)
   {
      bGridlines = mVerticalGridlines;
   }

   return bGridlines;
}

bool CustomTreeWidget::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if ((pObject != NULL) && (pEvent != NULL))
   {
      if (pObject == mpEdit)
      {
         // Get the current item
         QTreeWidgetItem* pItem = currentItem();
         if (pItem != NULL)
         {
            // Get the current column from the selected item
            QModelIndex cellIndex = currentIndex();
            int iColumn = cellIndex.column();

            // Convert from the entered text to a keyboard shortcut
            if (getCellWidgetType(pItem, iColumn) == SHORTCUT_EDIT)
            {
               if (pEvent->type() == QEvent::KeyPress)
               {
                  QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);

                  int key = pKeyEvent->key();
                  if ((key != Qt::Key_Control) && (key != Qt::Key_Shift) && (key != Qt::Key_Alt) &&
                     (key != Qt::Key_Meta) && (key != Qt::Key_Enter) && (key != Qt::Key_Return))
                  {
                     int keys[4] = { 0, 0, 0, 0 };

                     unsigned int index = 0;
                     for (index = 0; index < mShortcut.count(); ++index)
                     {
                        keys[index] = mShortcut[index];
                     }

                     if (key == Qt::Key_Backspace)
                     {
                        if (index > 0)
                        {
                           keys[--index] = 0;
                        }
                     }
                     else
                     {
                        if (index < 4)
                        {
                           Qt::KeyboardModifiers modifiers = pKeyEvent->modifiers();
                           if (modifiers & Qt::AltModifier)
                           {
                              key |= Qt::AltModifier;
                           }

                           if (modifiers & Qt::ControlModifier)
                           {
                              key |= Qt::ControlModifier;
                           }

                           if (modifiers & Qt::MetaModifier)
                           {
                              key |= Qt::MetaModifier;
                           }

                           if ((modifiers & Qt::ShiftModifier) && (key != Qt::Key_Backtab))
                           {
                              key |= Qt::ShiftModifier;
                           }

                           keys[index] = key;
                        }
                     }

                     mShortcut = QKeySequence(keys[0], keys[1], keys[2], keys[3]);

                     QString strShortcut = mShortcut.toString();
                     mpEdit->setText(strShortcut);
                     return true;
                  }
               }
            }
         }
      }
   }

   return QTreeWidget::eventFilter(pObject, pEvent);
}

void CustomTreeWidget::closeActiveCellWidget(bool bAcceptEdit)
{
   if (mpEdit != NULL)
   {
      if (mpEdit->isVisible() == true)
      {
         if (bAcceptEdit == true)
         {
            acceptEditText();
         }
         else
         {
            closeEdit();
         }
      }
   }

   if (mpCombo != NULL)
   {
      if (bAcceptEdit == true)
      {
         acceptComboText();
      }
      else
      {
         closeCombo();
      }
   }

   if (mpSpin != NULL)
   {
      if (bAcceptEdit == true)
      {
         acceptSpinText();
      }
      else
      {
         closeSpin();
      }
   }
}

void CustomTreeWidget::setBrowseDir(const QString& strDirectory)
{
   QString strDir;
   if (strDirectory.isEmpty() == false)
   {
      QFileInfo fileInfo(strDirectory);
      if (fileInfo.isDir() == true)
      {
         strDir = strDirectory;
      }
   }

   mBrowseDir = strDir;
}

QRect CustomTreeWidget::getCellRect(QTreeWidgetItem* pItem, int iColumn) const
{
   if (pItem == NULL)
   {
      return QRect(0, 0, 0, 0);
   }

   QModelIndex cellIndex = indexFromItem(pItem, iColumn);
   QRect cellRect = visualRect(cellIndex);

   return cellRect;
}

QRect CustomTreeWidget::getCellPixmapRect(QTreeWidgetItem* pItem, int iColumn) const
{
   if (pItem == NULL)
   {
      return QRect(0, 0, 0, 0);
   }

   // Get the cell rectangle
   QRect rcCell = getCellRect(pItem, iColumn);

   QIcon icon = pItem->icon(iColumn);
   if (icon.isNull() == true)
   {
      return QRect(0, 0, 0, 0);
   }

   QSize iconSize = icon.actualSize(QSize(rcCell.width(), rcCell.height()));

   QRect rcPixmap = rcCell;
   rcPixmap.setRight(rcPixmap.left() + iconSize.width());

   return rcPixmap;
}

QRect CustomTreeWidget::getCellWidgetRect(QTreeWidgetItem* pItem, int iColumn) const
{
   if (pItem == NULL)
   {
      return QRect(0, 0, 0, 0);
   }

   // Get the cell rectangle
   QRect rcCell = getCellRect(pItem, iColumn);

   // Check if the widget should cover the entire cell
   bool bFullCell = getFullCellEdit(pItem, iColumn);
   if (bFullCell == true)
   {
      return rcCell;
   }

   // Adjust the cell rectangle for the icon
   QRect rcWidget = rcCell;

   QIcon icon = pItem->icon(iColumn);
   if (icon.isNull() == false)
   {
      QSize iconSize = icon.actualSize(QSize(rcCell.width(), rcCell.height()));
      rcWidget.setLeft(rcCell.left() + iconSize.width());
   }

   return rcWidget;
}

void CustomTreeWidget::drawRow(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
   QTreeWidget::drawRow(pPainter, option, index);

   if (pPainter != NULL)
   {
      QTreeWidgetItem* pItem = itemFromIndex(index);
      if (pItem != NULL)
      {
         pPainter->setPen(QPen(Qt::gray));

         for (int i = 0; i < columnCount(); ++i)
         {
            QModelIndex currentIndex = indexFromItem(pItem, i);
            QRect cellRect = visualRect(currentIndex);

            // Draw horizontal gridlines
            if (mHorizontalGridlines == true)
            {
               pPainter->drawLine(cellRect.left() - 1, cellRect.top() - 1, cellRect.right(), cellRect.top() - 1);
               pPainter->drawLine(cellRect.left() - 1, cellRect.bottom(), cellRect.right(), cellRect.bottom());
            }

            // Draw vertical gridlines
            if (mVerticalGridlines == true)
            {
               pPainter->drawLine(cellRect.left() - 1, cellRect.top() - 1, cellRect.left() - 1, cellRect.bottom());
               pPainter->drawLine(cellRect.right(), cellRect.top() - 1, cellRect.right(), cellRect.bottom());
            }
         }
      }
   }
}

void CustomTreeWidget::setCurrentCellText(const QString& strText)
{
   // Get the current item
   QTreeWidgetItem* pItem = currentItem();
   if (pItem == NULL)
   {
      return;
   }

   // Get the current column from the selected item
   QModelIndex cellIndex = currentIndex();
   int iColumn = cellIndex.column();

   if (mpCombo != NULL)
   {
      int iIndex = mpCombo->currentIndex();
      if (iIndex != -1)
      {
         QIcon currentIcon = pItem->icon(iIndex);
         QIcon newIcon = mpCombo->itemIcon(iIndex);

         QRect rcCell = getCellRect(pItem, iColumn);
         QPixmap currentPix = currentIcon.pixmap(rcCell.width(), rcCell.height());
         QPixmap newPix = newIcon.pixmap(rcCell.width(), rcCell.height());

         QImage currentImage = currentPix.toImage();
         QImage newImage = newPix.toImage();

         if (currentImage != newImage)
         {
            pItem->setIcon(iColumn, newIcon);
            emit cellIconChanged(pItem, iColumn);
         }
      }
   }

   if (pItem->text(iColumn) != strText)
   {
      pItem->setText(iColumn, strText);
      emit cellTextChanged(pItem, iColumn);
   }
}

int CustomTreeWidget::getColumnIndex(int iItemX) const
{
   int iIndex = -1;

   QHeaderView* pHeader = header();
   if (pHeader != NULL)
   {
      iIndex = pHeader->logicalIndexAt(iItemX);
   }

   return iIndex;
}

void CustomTreeWidget::browse()
{
   QTreeWidgetItem* pItem = currentItem();
   if (pItem == NULL)
   {
      return;
   }

   QModelIndex cellIndex = currentIndex();
   int iColumn = cellIndex.column();

   QString strInitialText = pItem->text(iColumn);

   QFileInfo fileInfo(strInitialText);

   QString strLocation;
   WidgetType eType = getCellWidgetType(pItem, iColumn);
   if (eType == BROWSE_FILE_EDIT)
   {
      if (fileInfo.isFile() == false)
      {
         strInitialText = mBrowseDir;
      }

      strLocation = QFileDialog::getOpenFileName(this, QString(), strInitialText);
   }
   else if (eType == BROWSE_DIR_EDIT)
   {
      if (fileInfo.isDir() == false)
      {
         strInitialText = mBrowseDir;
      }

      strLocation = QFileDialog::getExistingDirectory(this, QString(), strInitialText);
   }

   if (strLocation.isEmpty() == false)
   {
      strLocation.replace(QRegExp("\\\\"), "/");
      closeEdit();
      setCurrentCellText(strLocation);

      if (eType == BROWSE_FILE_EDIT)
      {
         emit fileSelected(pItem, iColumn);
      }
      else if (eType == BROWSE_DIR_EDIT)
      {
         emit directorySelected(pItem, iColumn);
      }
   }
   else if (mpEdit != NULL)
   {
      if (mpEdit->isVisible() == true)
      {
         mpEdit->setFocus();
      }
   }
}

void CustomTreeWidget::acceptEditText()
{
   if (mpEdit == NULL)
   {
      return;
   }
   QTreeWidgetItem* pItem = currentItem();
   if (pItem == NULL)
   {
      return;
   }
   QString strText = mpEdit->text();
   if(getCellWidgetType(pItem, currentIndex().column()) == BROWSE_DIR_EDIT)
   {
      QFileInfo fileInfo(strText);
      if(fileInfo.isFile())
      {
         strText = fileInfo.absolutePath();
      }
   }
   setCurrentCellText(strText);
   closeEdit();
}

void CustomTreeWidget::closeEdit()
{
   if (mpEdit == NULL)
   {
      return;
   }

   mpEdit->hide();

   mShortcut = QKeySequence();

   if (mpBrowse != NULL)
   {
      if (mpBrowse->isVisible() == true)
      {
         mpBrowse->hide();
      }
   }

   viewport()->setFocus();
}

void CustomTreeWidget::acceptComboText()
{
   if (mpCombo == NULL)
   {
      return;
   }

   setCurrentCellText(mpCombo->currentText());
   closeCombo();
}

void CustomTreeWidget::closeCombo()
{
   if (mpCombo == NULL)
   {
      return;
   }

   mpCombo->hide();
   mpCombo = NULL;

   viewport()->setFocus();
}

void CustomTreeWidget::acceptSpinText()
{
   if (mpSpin == NULL)
   {
      return;
   }

   setCurrentCellText(mpSpin->text());
   closeSpin();
}

void CustomTreeWidget::closeSpin()
{
   if (mpSpin == NULL)
   {
      return;
   }

   mpSpin->hide();
   mpSpin = NULL;

   viewport()->setFocus();
}
