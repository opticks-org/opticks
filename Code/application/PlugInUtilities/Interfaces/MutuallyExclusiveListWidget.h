/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MUTUALLYEXCLUSIVELISTWIDGET_H
#define MUTUALLYEXCLUSIVELISTWIDGET_H

#include <QtGui/QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;

/**
 *  A widget that allow users to select an ordered set of items from a master
 *  list.
 *
 *  The mutually exclusive list widget contains two QListWidget objects: one
 *  list to contain available items to select, and another list to contain
 *  the ordered set of items selected from the first list.  When an item is
 *  selected from the available items list it is added to the selected items
 *  list and no longer appears in the available items list.  Both list widgets
 *  are single selection and cannot be sorted.
 *
 *  The widget also contains Add and Remove buttons to transfer items between
 *  the two lists, and labels to provide a description of each list.
 */
class MutuallyExclusiveListWidget : public QWidget
{
   Q_OBJECT

public:
   /**
    *  Creates a new mutually exclusive list widget with empty lists and default
    *  labels of "Available Items" and "Selected Items".
    *
    *  @param   pParent
    *           The parent widget.
    */
   MutuallyExclusiveListWidget(QWidget* pParent = NULL);

   /**
    *  Destroys the mutually exclusive list widget.
    */
   virtual ~MutuallyExclusiveListWidget();

   /**
    *  Sets the text description of the available items list.
    *
    *  @param   label
    *           The available items list description.
    *
    *  @see     setSelectedItemsLabel()
    */
   void setAvailableItemsLabel(const QString& label);

   /**
    *  Returns the text description of the available items list.
    *
    *  @return  The available items list description.
    *
    *  @see     getSelectedItemsLabel()
    */
   QString getAvailableItemsLabel() const;

   /**
    *  Sets the list of available items to select from.
    *
    *  This methods clears the existing contents of both the available items
    *  and selected items lists.  The method then populates the available items
    *  list with the given names.  To set initially selected items call
    *  selectItems() after calling this method.
    *
    *  @param   items
    *           The list of available items.
    */
   void setAvailableItems(const QStringList& items);

   /**
    *  Returns the list of remaining available items to select from.
    *
    *  This methods returns only the items in the available items list that
    *  have not been selected.
    *
    *  @return  The list of remaining available items.
    *
    *  @see     getSelectedItems()
    */
   QStringList getAvailableItems() const;

   /**
    *  Sets the text description of the selected items list.
    *
    *  @param   label
    *           The selected items list description.
    *
    *  @see     setAvailableItemsLabel()
    */
   void setSelectedItemsLabel(const QString& label);

   /**
    *  Returns the text description of the selected items list.
    *
    *  @return  The selected items list description.
    *
    *  @see     getAvailableItemsLabel()
    */
   QString getSelectedItemsLabel() const;

   /**
    *  Returns the list of selected items.
    *
    *  @return  The list of selected items.
    *
    *  @see     getAvailableItems()
    */
   QStringList getSelectedItems() const;

public slots:
   /**
    *  Moves items from the available items list to the selected items list.
    *
    *  @param   items
    *           The items to move from the available items list to the selected
    *           items list.
    *
    *  @see     removeItems()
    */
   void selectItems(const QStringList& items);

   /**
    *  Moves items from the selected items list to the available items list.
    *
    *  @param   items
    *           The items to move from the selected items list to the available
    *           items list.
    *
    *  @see     selectItems()
    */
   void removeItems(const QStringList& items);

signals:
   /**
    *  A signal that is emitted when an item moves from the available items list
    *  to the selected items list.
    *
    *  The signal is emitted when the user clicks the Add button or when the
    *  selectItems() method is called.  If multiple items are moved as a result
    *  of calling selectItems(), the signal is emitted once for each item.
    *
    *  @param   item
    *           The item that moved from the available items list to the
    *           selected items list.
    */
   void itemSelected(const QString& item);

   /**
    *  A signal that is emitted when an item moves from the selected items list
    *  to the available items list.
    *
    *  The signal is emitted when the user clicks the Remove button or when the
    *  removeItems() method is called.  If multiple items are moved as a result
    *  of calling removeItems(), the signal is emitted once for each item.
    *
    *  @param   item
    *           The item that moved from the selected items list to the
    *           available items list.
    */
   void itemRemoved(const QString& item);

protected slots:
   /**
    *  Moves the currently selected item in the available items list to the
    *  selected items list.
    *
    *  This method is called automatically when the user clicks the Add button,
    *  and should not need to be called explicitly.
    */
   void addSelectedItem();

   /**
    *  Moves the currently selected item in the selected items list to the
    *  available items list.
    *
    *  This method is called automatically when the user clicks the Remove
    *  button, and should not need to be called explicitly.
    */
   void removeSelectedItem();

private:
   void selectAvailableItem(QListWidgetItem* pAvailableItem);
   void removeSelectedItem(QListWidgetItem* pSelectedItem);

   QLabel* mpAvailableLabel;
   QListWidget* mpAvailableList;
   QLabel* mpSelectedLabel;
   QListWidget* mpSelectedList;
};

#endif
