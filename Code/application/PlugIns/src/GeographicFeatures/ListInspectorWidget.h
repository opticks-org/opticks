/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LISTINSPECTORWIDGET_H
#define LISTINSPECTORWIDGET_H

#include <QtGui/QWidget>

class QListWidget;
class QListWidgetItem;
class QSplitter;

/**
 * A class to use a single inspector widget instance for a number of data objects.
 *
 * A list view on the left shows the available data objects.  When the
 * user changes the selection in the list, signals are emitted to save the
 * state of the inspector, and load the state from another object.
 * Add and remove buttons are placed under the list view.
 *
 * This class should not normally be subclassed.  The signals emitted should
 * be sufficient to allow typical usage.  Users of this class will need to separately
 * track the actual data being displayed in this widget.
 */
class ListInspectorWidget : public QWidget
{
   Q_OBJECT

public:
   /**
    * Construct the ListInspectorWidget with the provided inspector.
    *
    * @param pInspector
    *        The inspector widget.  This object will reparent this widget.
    * @param pParent
    *        The parent QWidget.
    */
   ListInspectorWidget(QWidget *pInspector, QWidget *pParent = NULL);

   /**
    * Destroy the ListInspectorWidget.
    */
   ~ListInspectorWidget();

   /**
    * Apply any outstanding changes.
    *
    * This is typically called when a dialog containing this widget is dismissed.
    */
   void applyChanges();

   /**
    * Add an item to the list view.
    *
    * @param text
    *        The text for the new QListWidgetItem.
    *
    * @return The created QListWidgetItem.
    */
   QListWidgetItem *addItem(const std::string &text);

   /**
    * Sets the currently selected item in the list view.
    *
    * @param pItem
    *        The item to select.
    */
   void setCurrentItem(QListWidgetItem* pItem);

   /**
    * Get the inspector which was passed into the constructor.
    *
    * @return The inspector widget.
    */
   QWidget *getInspector();

   /**
    * Get a name which is unique to the list.
    *
    * @param name
    *        The name to use to generate a unique name for.
    *
    * @return A unique name.
    */
   std::string getUniqueName(const std::string &name) const;

   /**
    * Clear all entries from the list.
    */
   void clearList();

   /**
    * Set whether to hide the list and add/remove buttons.
    *
    * @param hidden
    *        If true, the list will become hidden.  If false, the list will
    *        become shown.
    */
   void setHideList(bool hidden);

signals:
   /**
    * This signal is emitted whenever the add button is pressed.
    *
    * Users of this class are expected to connect to this signal and present
    * some GUI to determine what to add, then call addItem() for each item added.
    *
    * @see addItem(const std::string &)
    */
   void addItems();

   /**
    * This signal is emitted whenever the state of the inspector should be saved.
    *
    * @param pInspector
    *        The inspector widget to save.
    * @param pItem
    *        The QListWidgetItem that represents the data to be saved. May be NULL.
    */
   void saveInspector(QWidget *pInspector, QListWidgetItem *pItem);

   /**
    * This signal is emitted whenever the state of the inspector should be loaded.
    *
    * @param pInspector
    *        The inspector widget to load into.
    * @param pItem
    *        The QListWidgetItem that represents the data to be loaded. May be NULL.
    */
   void loadInspector(QWidget *pInspector, QListWidgetItem *pItem);

   /**
    * This signal is emitted whenever an item is being removed.
    *
    * The ListInspectorWidget takes care of any housekeeping needed for the list view.
    * client code only needs to remove data from its separate data store.
    *
    * @param pItem
    *        The QListWidgetItem that represents the data to be removed. May be NULL.
    */
   void removeItem(QListWidgetItem *pItem);

private slots:
   void setDisplayedItem(QListWidgetItem *pDisplayedItem);
   void removeDisplayedItem();

private:
   ListInspectorWidget(const ListInspectorWidget& rhs);
   ListInspectorWidget& operator=(const ListInspectorWidget& rhs);
   QListWidget* mpListWidget;
   QWidget* mpInspector;
   QSplitter* mpSplitter;

   QListWidgetItem* mpDisplayedItem;
   bool mDeleteRowOnSave;
};

#endif
