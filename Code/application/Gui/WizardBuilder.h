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

#include <QtGui/QMainWindow>

#include "ConfigurationSettings.h"

class WizardItemProperties;
class WizardObject;
class WizardProperties;
class WizardView;

/**
 *  A widget in which to chain plug-in and desktop service items together in sequence
 *  to create a wizard.
 *
 *  The WizardBuilder widget allows sequencing of plug-ins, desktop services, and
 *  specific values by manipulating items on a canvas displayed in a view widget.  The
 *  wizard builder looks similar to an application in that it has its own menu bar,
 *  toolbar, and status bar.  The central part of the wizard builder is split into
 *  several widgets: the view containing the items, dock windows displaying properties
 *  of the wizard and selected wizard item, and a dock window containing a palette of
 *  all items that can be added to the wizard.
 *
 *  The palette dock window contains a list of available wizard items arranged by type.
 *  For convenience, it also has a tab that displays all items simultaneously.  Multiple
 *  items can be selected and dragged onto the view widget to add the items to the wizard.
 *  Multiple items are added in a cascading fashion.
 *
 *  The view widget displays the desktop, plug-in, and specific value items.  The items
 *  contain various input and output arguments that can be connected to form a
 *  sequence of events.  Selected items have a context menu containing actions relevant
 *  to the items, and the view itself has a separate context menu containing actions
 *  relevant to the wizard.
 *
 *  Each item has a unique execution order identifier.  The order may be incremented for
 *  an item with the increaseCurrentItemOrder() and decreaseCurrentItemOrder() methods.
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
    *  The destructor destroys the builder widget.  The view widget and all items are also
    *  destroyed.
    */
   virtual ~WizardBuilder();

protected:
   SETTING(Configuration, WizardBuilder, std::string, "")
   SETTING(Geometry, WizardBuilder, std::string, "")

   /**
    *  Prompts the user to save the current wizard if necessary on close.
    *
    *  This method calls destroyCurrentWizard() to prompt the user to save the current
    *  wizard before closing the wizard builder window.
    *
    *  @param   pEvent
    *           The incoming close event.
    */
   void closeEvent(QCloseEvent* pEvent);

   /**
    *  Destroys the current wizard.
    *
    *  This method checks the current wizard for changes and prompts the user to save the
    *  wizard as necessary.  If the user chooses to continue, the wizard items are removed,
    *  and the wizard object is deleted.
    *
    *  @return  Returns \c true if the wizard items and the wizard object are successfully
    *           deleted, otherwise returns \c false.
    *
    *  @see     newWizard(), openWizard()
    */
   bool destroyCurrentWizard();

protected slots:
   /**
    *  Creates a new, empty wizard.
    *
    *  This method prompts the user to save the existing wizard if necessary, and then
    *  clears the wizard items, destroys the current wizard object, and creates a new
    *  wizard object.
    *
    *  @see     openWizard()
    */
   void newWizard();

   /**
    *  Opens an existing wizard from a file.
    *
    *  This method invokes a file selection dialog in which to select a wizard file to load
    *  into the wizard builder.  The user is prompted to save the current wizard if
    *  necessary before deleting it.
    *
    *  @see     newWizard()
    */
   void openWizard();

   /**
    *  Updates the caption in the wizard builder title bar.
    *
    *  This method updates the caption in the title bar according to a given filename.  The
    *  caption is in the form of "Wizard Builder - <filename>".  If the wizard does not have
    *  an associated filename, the caption is set to "Wizard Builder - Untitled".
    *
    *  @param   filename
    *           The filename to display in the title bar.
    */
   void updateCaption(const QString& filename);

   /**
    *  Copies the selected wizard items to the clipboard and then deletes the items.
    *
    *  @see     copy()
    */
   void cut();

   /**
    *  Copies the selected wizard items to the clipboard.
    *
    *  This methods copies all currently selected items to the clipboard.  All connections
    *  between copied items are preserved.
    *
    *  @see     cut()
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
    *  Increases the execution order of the selected item.
    *
    *  This method sets the execution order of a single selected item to occur later in the
    *  wizard.  The new exeuction order is based on the connected output nodes.  If the new
    *  resulting order forces a connected item to have a value greater than the total number
    *  of wizard items, the order on the selected item is not changed.
    *
    *  @see     decreaseCurrentItemOrder()
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
    *  @see     increaseCurrentItemOrder()
    */
   void decreaseCurrentItemOrder();

   /**
    *  Updates the wizard item properties in the dock window widget for the
    *  selected item.
    */
   void updateItemProperties();

private:
   WizardBuilder(const WizardBuilder& rhs);
   WizardBuilder& operator=(const WizardBuilder& rhs);
   WizardView* mpView;
   WizardProperties* mpWizardProperties;
   WizardItemProperties* mpItemProperties;
};

#endif
