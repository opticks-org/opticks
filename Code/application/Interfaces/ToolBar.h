/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "ConfigurationSettings.h"
#include "Window.h"

#include <string>
#include <vector>

class MenuBar;
class QAction;
class QWidget;

/**
 *  A widget containing buttons and other widgets to invoke custom processes.
 *
 *  Each toolbar contains a menu bar as the first widget in the toolbar in
 *  which command actions can be added that relate to the other widgets and
 *  buttons on the toolbar.
 *
 *  This class provides a means for objects to add and remove buttons and other
 *  widgets  from toolbars displayed in the main application window.  The
 *  location of the button or widget can also be specified relative to the
 *  other buttons and widgets on the toolbar.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: addButton(), insertWidget(),
 *    addSeparator(), and removeItem().
 *  - Everything else documented in Window.
 *
 *  @see     Window
 */
class ToolBar : public Window
{
public:
   SETTING(Opacity, ToolBar, int, 100);

   /**
    *  Emitted when the toolbar is shown.
    */
   SIGNAL_METHOD(ToolBar, Shown)

   /**
    *  Emitted when the toolbar is hidden.
    */
   SIGNAL_METHOD(ToolBar, Hidden)

   /**
    *  Returns the menu bar associated with this toolbar.
    *
    *  @return  A pointer to the toolbar's menu bar.
    */
   virtual MenuBar* getMenuBar() const = 0;

   /**
    *  Adds the button to the toolbar.
    *
    *  This method adds a button to the toolbar based on the given action.  The
    *  object calling this method is responsible for setting the icon and any
    *  other associated properties with the action and also any necessary
    *  connections to slot methods.
    *
    *  The button is added directly to the toolbar and the user will not be
    *  able to edit and save a keyboard shortcut in the user-defined
    *  configuration settings.  To allow the user to edit the keyboard
    *  shortcut, call the overloaded
    *  addButton(QAction*, const std::string&, QAction*) method or call
    *  DesktopServices::initializeAction() prior to calling this method.
    *
    *  @param   pAction
    *           The action to add to the toolbar as a button.
    *  @param   pBefore
    *           The button or widget that should immediately follow the added
    *           button.  The button is added at the end of the toolbar if
    *           \b NULL is passed in or if the given action does not exist on
    *           the toolbar.
    *
    *  @notify  This method will notify Subject::signalModified if the action
    *           was successfully added to the toolbar.
    *
    *  @see     addSeparator()
    */
   virtual void addButton(QAction* pAction, QAction* pBefore = NULL) = 0;

   /**
    *  Adds the button to the toolbar.
    *
    *  This method adds a button to the toolbar based on the given action and
    *  allows the user to edit and save a keyboard shortcut based on the given
    *  context.  If the user should not be able to edit the shortcut, call the
    *  overloaded addButton(QAction*, QAction*) method instead.
    *
    *  The object calling this method is responsible for setting the icon and
    *  any other associated properties with the action and also any necessary
    *  connections to slot methods.
    *
    *  @param   pAction
    *           The action to add to the toolbar as a button.
    *  @param   shortcutContext
    *           The context name for the keyboard shortcut of the action.  When
    *           the user edits keyboard shortcuts, the shortcuts are grouped
    *           according to their context string.  Contexts can be nested by
    *           using a slash (/) between context levels.  If the top-level
    *           context name is the same as a plug-in name, the shortcut is
    *           grouped under a single Plug-Ins context.  Passing in an empty
    *           string indicates that the user should not be able to edit the
    *           keyboard shortcut of the command, which is the same as calling
    *           the overloaded addButton(QAction*, QAction*) method.
    *  @param   pBefore
    *           The button or widget that should immediately follow the added
    *           button.  The button is added at the end of the toolbar if
    *           \b NULL is passed in or if the given action does not exist on
    *           the toolbar.
    *
    *  @notify  This method will notify Subject::signalModified if the action
    *           was successfully added to the toolbar.
    *
    *  @see     addSeparator()
    */
   virtual void addButton(QAction* pAction, const std::string& shortcutContext, QAction* pBefore = NULL) = 0;

   /**
    *  Adds an existing widget to the toolbar.
    *
    *  @param   pWidget
    *           The widget to add to the toolbar.
    *  @param   pBefore
    *           The button or widget that should immediately follow the added
    *           widget.  The separator is added at the end of the toolbar if
    *           \b NULL is passed in or if the given action does not exist on
    *           the toolbar.
    *
    *  @return  A pointer to the Qt action for the widget created by the
    *           toolbar.
    *
    *  @notify  This method will notify Subject::signalModified if the widget
    *           was successfully added to the toolbar.
    */
   virtual QAction* insertWidget(QWidget* pWidget, QAction* pBefore = NULL) = 0;

   /**
    *  Adds a separator to the toolbar.
    *
    *  This method creates a separator widget on the toolbar to separate
    *  buttons into groups.
    *
    *  @param   pBefore
    *           The button or widget that should immediately follow the added
    *           separator.  The separator is added at the end of the toolbar if
    *           \b NULL is passed in or if the given action does not exist on
    *           the toolbar.
    *
    *  @return  A pointer to the Qt action for the separator created by the
    *           toolbar.
    *
    *  @notify  This method will notify Subject::signalModified if the separator
    *           was successfully added to the toolbar.
    */
   virtual QAction* addSeparator(QAction* pBefore = NULL) = 0;

   /**
    *  Returns all buttons and widgets on the toolbar as actions.
    *
    *  @return  A vector of actions defining all buttons and widgets on the
    *           toolbar.
    */
   virtual std::vector<QAction*> getItems() const = 0;

   /**
    *  Removes a button or widget from the toolbar.
    *
    *  This method removes the button or widget defined by the given action
    *  from the toolbar.  The action is not deleted.
    *
    *  @param   pAction
    *           The action defining the button or widget to remove from the
    *           toolbar.
    *
    *  @notify  This method will notify Subject::signalModified if the action
    *           was successfully removed from the toolbar.
    */
   virtual void removeItem(QAction* pAction) = 0;

   /**
    *  Shows the toolbar.
    *
    *  This method will display the toolbar only if it's hidden.
    *  If the toolbar already shown, this method will do nothing.
    *
    *  @notify  This method will notify signalShown() if the window was
    *           successfully displayed from a previously hidden state.
    *
    *  @see     hide()
    */
   virtual void show() = 0;

   /**
    *  Hides the toolbar.
    *
    *  This method will hide the toolbar only if it's shown.
    *  If the toolbar already hidden, this method will do nothing.
    *
    *  @notify  This method will notify signalHidden() if the window was
    *           successfully hidden from a previously visible state.
    *
    *  @see     show()
    */
   virtual void hide() = 0;

   /**
    *  This method will return the current visiblity state of the toolbar.
    *
    *  @return  Returns \c true if the toolbar is shown or \c false
    *           if the toolbar is hidden.
    */
   virtual bool isShown() const = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteWindow.
    */
   virtual ~ToolBar() {}
};

#endif
