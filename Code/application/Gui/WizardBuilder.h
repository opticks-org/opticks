/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDBUILDER_H
#define WIZARDBUILDER_H

#include <Qt3Support/Q3Canvas>
#include <QtGui/QCheckBox>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>

class PlugInArgList;
class WizardItem;
class WizardObject;
class WizardObjectAdapter;
class WizardView;

/**
 *  A widget in which to chain plug-in and desktop service items together in sequence
 *  to create a wizard.
 *
 *  The WizardBuilder widget allows sequencing of plug-ins, desktop services, and
 *  specific values by manipulating items on a canvas displayed in a view widget.  The
 *  wizard builder looks similar to an application in that it has its own menu bar,
 *  toolbar, and status bar.  The central part of the wizard builder is split into two
 *  main widgets, the view and a parameters widget.
 *
 *  The view widget displays the desktop, plug-in, and specific value canvas items.  The
 *  items can be added by calling addDesktopItem(), addPlugInItem(), and addValueItem().
 *  The items contains various input and output arguments that can be connected to form
 *  a sequence of events.
 *
 *  The parameters widget contains numerous child widgets to set various parameters of the
 *  overall wizard.  The wizard and each item can be set for batch mode or interactive mode
 *  execution.  The input and output arguments for selected items are listed according to
 *  their name, type, item name, and item execution order.  Items can be connected with
 *  the connectItems() method, which uses the selected input and output argument.
 *
 *  Each item has a unique execution order identifier.  The order may be incremented for
 *  an item with the increaseCurrentItemOrder() and decreaseCurrentItemOrder() methods.
 *
 *  The wizard builder also allows saving and loading the current canvas configuration with
 *  openWizard() and saveWizard().
 *
 *  @see   WizardView
 */
class WizardBuilder : public QMainWindow
{
   Q_OBJECT

public:
   /**
    *  Constructs a WizardBuilder.
    *
    *  @param   pParent
    *           The parent widget.
    */
   WizardBuilder(QWidget* pParent = 0);

   /**
    *  Destructs the builder.
    *
    *  The destructor destroys the builder widget.  The canvas and all items are also
    *  destroyed.
    */
   ~WizardBuilder();

protected:
   /**
    *  Intercepts event notification for registered objects.
    *
    *  This method is used to intercept messages to the wizard and item widgets.
    *  The event is queried for a key press to delete the selected items if necessary.
    *
    *  @param   o
    *           The object sending the event message.
    *  @param   e
    *           The event message.
    *
    *  @return  TRUE if the event should not be passed to the respective widget.  FALSE
    *           if the event should be processed normally.
    */
   bool eventFilter(QObject* o, QEvent* e);

   /**
    *  Handles key press events.
    *
    *  This method override removes selected items if the Delete key is pressed.  All
    *  item widgets and the remaining item's execution order update accordingly.
    *
    *  @param   e
    *           The incoming key press event.
    */
   void keyPressEvent(QKeyEvent* e);

   /**
    *  Prompts the user to save the current wizard if necessary on close.
    *
    *  This method calls destroyCurrentWizard() to prompt the user to save the current
    *  wizard before closing the wizard builder window.
    *
    *  @param   e
    *           The incoming close event.
    */
   void closeEvent(QCloseEvent* e);

   /**
    *  Sets the wizard builder widget values from the wizard element data.
    *
    *  This method is generally called when loading an existing wizard.  The wizard element
    *  data is used to create canvas items and connections for all of the items in the
    *  wizard.  The wizard name and menu location are also added to the edit box widgets.
    *
    *  @param   pWizardObject
    *           The existing wizard element.
    */
   void buildWizard(WizardObject* pWizardObject);

   /**
    *  Creates input nodes for a wizard item.
    *
    *  This method creates input nodes for the given item based on the arguments in the
    *  given arg list.  Any previously existing nodes are first removed.
    *
    *  @param   pItem
    *           The wizard item for which to create the input nodes.  Cannot be NULL.
    *  @param   pArgList
    *           The arg list to use as the basis for the input nodes.  Cannot be NULL.
    *
    *  @see     WizardBuilder::buildOutputNodes
    */
   void buildInputNodes(WizardItem* pItem, const PlugInArgList* pArgList);

   /**
    *  Creates output nodes for a wizard item.
    *
    *  This method creates output nodes for the given item based on the arguments in the
    *  given arg list.  Any previously existing nodes are first removed.
    *
    *  @param   pItem
    *           The wizard item for which to create the output nodes.  Cannot be NULL.
    *  @param   pArgList
    *           The arg list to use as the basis for the output nodes.  Cannot be NULL.
    *
    *  @see     WizardBuilder::buildInputNodes
    */
   void buildOutputNodes(WizardItem* pItem, const PlugInArgList* pArgList);

   /**
    *  Sets a wizard item to run in interactive or batch mode.
    *
    *  @param   pItem
    *           The wizard item for which to set the batch mode flag.  Cannot be NULL.
    *  @param   bBatch
    *           TRUE to set the item to run in batch mode.  FALSE to set the item to
    *           run in interactive mode.
    *
    *  @return  TRUE if the item execution mode was successfully set, otherwise FALSE.
    *
    *  @see     WizardBuilder::setItemBatchMode
    */
   bool setItemBatchMode(WizardItem* pItem, bool bBatch);

   /**
    *  Modifies the settings of a wizard item.
    *
    *  This method is only available for Value items.  An error message appears if the
    *  given item does not have a Value type.  For Value items, a dialog is invoked for
    *  the user to modify the specific value of the given item.  If the value type is
    *  changed, all connections with other items are broken.
    *
    *  @param   pItem
    *           The wizard item to edit.  Cannot be NULL.  Should have the Value type to
    *           avoid an error message.
    *
    *  @see     WizardBuilder::editItem
    */
   void editItem(WizardItem* pItem);

   /**
    *  Updates the execution order on each item in the view.
    *
    *  @see     WizardBuilder::increaseCurrentItemOrder
    *  @see     WizardBuilder::decreaseCurrentItemOrder
    */
   void refreshOrder();

   /**
    *  Returns the execution order of a wizard item.
    *
    *  @param   pItem
    *           The wizard item to edit.  Cannot be NULL.  Should have the Value type to
    *           avoid an error message.
    *
    *  @return  The one-based execution order for the wizard item.  If the item does not
    *           exist in the wizard, a value of -1 is returned.
    */
   int getExecutionOrder(WizardItem* pItem);

   /**
    *  Closes the current wizard
    *
    *  This method checks the current wizard for changes and prompts the user to save the
    *  wizard as necessary.  If the user chooses to continue, the canvas items are removed,
    *  and the wizard element in the data model is deleted.
    *
    *  @return  TRUE if the canvas items and the wizard element are successfully deleted,
    *           otherwise FALSE.
    *
    *  @see     WizardBuilder::saveWizard
    *  @see     WizardBuilder::saveNewWizard
    */
   bool destroyCurrentWizard();

protected slots:
   /**
    *  Creates a new, empty wizard.
    *
    *  This method prompts the user to save the existing wizard if necessary, and then
    *  clears the canvas, destroys the current wizard element, and creates a new wizard
    *  element.
    *
    *  @see     WizardBuilder::openWizard
    */
   void newWizard();

   /**
    *  Opens an existing wizard from a file.
    *
    *  This method invokes a file selection dialog in which to select a wizard file to load
    *  into the wizard builder.  The user is prompted to save the current wizard if
    *  necessary before deleting it.
    *
    *  @see     WizardBuilder::newWizard
    *  @see     WizardBuilder::saveWizard
    */
   void openWizard();

   /**
    *  Saves the current wizard.
    *
    *  This method saves the current wizard to its file.  If the wizard has not previously
    *  been saved, the saveNewWizard() method is called to select a filename.
    *
    *  @return  TRUE if the wizard file was successfully saved, otherwise FALSE.
    *
    *  @see     WizardBuilder::saveNewWizard
    *  @see     WizardBuilder::openWizard
    */
   bool saveWizard();

   /**
    *  Saves the current wizard to a new file.
    *
    *  This method invokes a file selection dialog in which to select a filename to store
    *  the wizard.
    *
    *  @return  TRUE if the new wizard file was successfully saved, otherwise FALSE.
    *
    *  @see     WizardBuilder::saveWizard
    */
   bool saveNewWizard();

   /**
    *  Configues the layout of the page as it would be printed.
    *
    *  This method invokes a dialog in which to specify printing parameters for the items
    *  in the view widget.
    *
    *  @see     WizardBuilder::printWizard
    */
   void wizardPageSetup();

   /**
    *  Prints the current wizard.
    *
    *  This method sends to current wizard to the printer.  The items in the view widget are
    *  printed with their connection lines, but no selection nodes are printed.
    *
    *  @see     WizardBuilder::wizardPageSetup
    */
   void printWizard();

   /**
    *  Closes the wizard builder.
    *
    *  This methods prompts the user to save the current wizard if necessary, and then
    *  deletes the wizard element and closes the wizard builder.
    */
   void exitBuilder();

   /**
    *  Copies the selected wizard items to the clipboard and then deletes the items.
    *
    *  @see     WizardBuilder::copy
    */
   void cut();

   /**
    *  Copies the selected wizard items to the clipboard.
    *
    *  This methods copies all currently selected items to the clipboard.  All connections
    *  between copied items are preserved.
    */
   void copy();

   /**
    *  Adds the clipboard wizard items to the current wizard.
    *
    *  This methods inserts the items in the clipboard into the current wizard.  All previously
    *  existing items are deselected, and the inserted items are selected.  If interactive
    *  items are pasted into a wizard set to run entirely in batch mode, the items are converted
    *  to operate in batch mode, and any connections may be lost.
    */
   void paste();

   /**
    *  Adds a new item representing a desktop service function.
    *
    *  This method invokes a dialog containing all of the available desktop service items
    *  to add to the wizard.  The selected item is added to the view, and it is set to run
    *  in interactive mode.  If the wizard is set to run in batch mode, the item is added
    *  in batch mode.
    *
    *  @see     WizardBuilder::addPlugInItem
    *  @see     WizardBuilder::addValueItem
    */
   void addDesktopItem();

   /**
    *  Adds a new item representing a plug-in.
    *
    *  This method invokes a dialog containing a list of all plug-ins found in the
    *  specified plug-in directory.  The selected plug-in is then added to the view,
    *  and it is set to run in interactive mode.  If the wizard is set to run in batch
    *  mode, the item is added in batch mode.
    *
    *  @see     WizardBuilder::addDesktopItem
    *  @see     WizardBuilder::addValueItem
    */
   void addPlugInItem();

   /**
    *  Adds a new item representing a plug-in.
    *
    *  This method invokes a dialog for the user to select a data type and enter a single
    *  value.  A name can also be associated with the value.  An item is then added to the
    *  view containing a single output node.
    *
    *  @see     WizardBuilder::addDesktopItem
    *  @see     WizardBuilder::addPlugInItem
    */
   void addValueItem();

   /**
    *  Selects all wizard items.
    */
   void selectAllItems();

   /**
    *  Modifies the user-defined value of a Value item.
    *
    *  This method invokes a dialog to allow the user to modify the specific value of a
    *  selected Value item.  Only one item can be selected.  If the value type is changed,
    *  all connections with other items are broken.
    *
    *  @see     WizardBuilder::editItem
    */
   void editItem();

   /**
    *  Modifies the user-defined value of a Value item.
    *
    *  This method invokes a dialog to allow the user to modify the specific value of the
    *  wizard item represented by the given tree widget item.  If the wizard item does not
    *  have the value type, an error message is displayed.
    *
    *  @param   pTreeItem
    *           The tree widget item representing the wizard value item to edit.  Cannot be
    *           NULL.
    *
    *  @see     WizardBuilder::editItem
    */
   void editItem(QTreeWidgetItem* pTreeItem);

   /**
    *  Removes the currently selected items.
    *
    *  This method removes all selected items from the view.  Any lines connected to the
    *  selected items are also removed.
    */
   void removeItems();

   /**
    *  Handles mouse press events for a wizard item.
    *
    *  This method invokes a popup menu if the right mouse button is pressed on a valid
    *  wizard item.  The menu contains commands to perform modifications to the wizard
    *  item only and not the nodes within the item.
    *
    *  @param   e
    *           The incoming mouse event.
    *  @param   pItem
    *           The wizard item on which the user pressed a mouse button.
    */
   void itemMousePressEvent(QMouseEvent* e, WizardItem* pItem);

   /**
    *  Handles double click events for a wizard item.
    *
    *  This method currently calls editItem() for the given wizard item.
    *
    *  @param   e
    *           The incoming mouse event.
    *  @param   pItem
    *           The wizard item on which the user double-clicked.
    */
   void itemMouseDoubleClickEvent(QMouseEvent* e, WizardItem* pItem);

   /**
    *  Updates the wizard name in the data model element.
    *
    *  This method is called when the user enters text in the wizard name edit box.  The
    *  name of the wizard element in the data model is updated accordingly.
    *
    *  @param   strName
    *           The new wizard name.
    */
   void setWizardName(const QString& strName);

   /**
    *  Updates the menu location in the data model element.
    *
    *  This method is called when the user enters text in the menu location edit box.  The
    *  location of the wizard element in the data model is updated accordingly.
    *
    *  @param   strMenu
    *           The new menu location.  The string should contain a backslash '\' to
    *           separate menus and the command name.  For example, "Wizards\MyWizard"
    *           adds the MyWizard menu command to the Wizards menu.
    */
   void setWizardMenuLocation(const QString& strMenu);

   /**
    *  Sets the execution mode for the entire wizard.
    *
    *  This method sets the execution mode for the entire wizard.  Setting the wizard
    *  to run in batch mode automatically sets all items to run in batch mode.  Setting
    *  the wizard to interactive mode allows for items to be set to run in either interactive
    *  or batch mode.
    *
    *  @param   bBatch
    *           TRUE if the wizard should run in batch mode.  FALSE if the wizard should
    *           run in interactive mode.
    *
    *  @see     WizardBuilder::setItemBatchMode
    */
   void setWizardBatchMode(bool bBatch);

   /**
    *  Sets a location for a wizard item.
    *
    *  This method is called when the user enters text in the wizard name edit box.  The
    *  name of the wizard element in the data model is updated accordingly.
    *
    *  @param   pItem
    *           The wizard item for which to set the location.  Cannot be NULL.
    *  @param   dX
    *           The new x-coordinate for the wizard item in local coordinates.
    *  @param   dY
    *           The new y-coordinate for the wizard item in local coordinates.
    */
   void setItemPosition(WizardItem* pItem, double dX, double dY);

   /**
    *  Sets the execution mode for the currently selected wizard items.
    *
    *  This method sets the execution mode for selected items.  For Desktop and PlugIn items,
    *  the input and output nodes update accordingly.  A Desktop item in batch mode contains
    *  no inputs or outputs since the desktop services are not available in batch mode.  A
    *  red border on the item indicates that is is set to run in batch mode.
    *
    *  @see     WizardBuilder::setWizardBatchMode
    */
   void setItemBatchMode();

   /**
    *  Connects the selected input and output argument nodes.
    *
    *  This connects the selected input node and output node by adding a connection line.
    *  The execution order of the two items is updated if necessary to preserve the flow
    *  of the wizard.
    */
   void connectItems();

   /**
    *  Updates the execution order of all items based on the two given items being connected.
    *
    *  This updates the execution order of all wizard items when the two given items are
    *  connected, and the modified flag is set.  If any item is selected, the information
    *  in the tree widgets is also updated to reflect the new execution order.
    *
    *  @param   pOutputItem
    *           The wizard item containing the output node that was connected.  Cannot be
    *           NULL.
    *  @param   pInputItem
    *           The wizard item containing the input node that was connected.  Cannot be
    *           NULL.
    */
   void updateConnectedItemsOrder(WizardItem* pOutputItem, WizardItem* pInputItem);

   /**
    *  Increases the execution order of the selected item.
    *
    *  This method sets the execution order of a single selected item to occur later in the
    *  wizard.  The new exeuction order is based on the connected output nodes.  If the new
    *  resulting order forces a connected item to have a value greater than the total number
    *  of wizard items, the order on the selected item is not changed.
    *
    *  @see     WizardBuilder::decreaseCurrentItemOrder
    */
   void increaseCurrentItemOrder();

   /**
    *  Decreases the execution order of the selected item.
    *
    *  This method sets the execution order of a single selected item to occur earlier in the
    *  wizard.  The new exeuction order is based on the connected input nodes.  If the new
    *  resulting order forces a connected item to have a value less than one, the order on
    *  the selected item is not changed.
    *
    *  @see     WizardBuilder::decreaseCurrentItemOrder
    */
   void decreaseCurrentItemOrder();

   /**
    *  Updates the data widgets for the selected items.
    *
    *  This method forces an update of the input and output tree widgets and the item batch
    *  mode indicator for the selected items.
    */
   void updateItemInfo();

   /**
    *  Sets the wizard as modified.
    *
    *  This method flags the wizard as modified, which will prompt the user before changing
    *  wizards as needing to be saved.
    */
   void setModified();

   /**
    *  Executes the wizard
    *
    *  This method provides a means to ensure that all items are connected properly by
    *  launching the wizard.  The Wizard Executor plug-in is launched to run the desktop
    *  services, plug-ins, and values in the sequence indicated by the execution order of
    *  each item.
    */
   void runWizard();

private:
   WizardObjectAdapter* mpWizard;
   QString mFilename;

   Q3Canvas* mpCanvas;
   WizardView* mpView;

   bool mbModified;

   QTreeWidget* mpInputsTree;
   QTreeWidget* mpOutputsTree;
   QCheckBox* mpWizardBatchCheck;
   QLineEdit* mpNameEdit;
   QLineEdit* mpMenuEdit;
   QCheckBox* mpItemBatchCheck;
   QPushButton* mpConnectButton;
};

#endif
