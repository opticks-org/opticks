/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MENUBAR_H
#define MENUBAR_H

#include <string>

class QAction;
class QMenu;

/**
 *  Container for menus with user executable commands.
 *
 *  This class provides access to the menu structure from which users can
 *  execute a variety of actions.  Plug-ins can use the menu bar to actively
 *  add and organize actions.  The application contains a menu bar located at
 *  the top of the main application window that can be obtained by calling
 *  DesktopServices::getMainMenuBar().  In addition to the main menu bar, each
 *  toolbar contains a menu bar that is the first widget on the toolbar that
 *  can be obtained by calling ToolBar::getMenuBar().
 *
 *  Plug-ins should use the MenuBar class when Qt slot methods within the
 *  plug-in should be called when the command is invoked.  For this to work
 *  properly, these plug-ins should stay resident after execution.
 *
 *  There are several approaches to adding and organizing menus and commands on
 *  the menu bar.  One approach is that a menu can be added with addMenu(), and
 *  then custom actions can be added directly to the menu.  Each action that is
 *  added directly to a menu will not have a keyboard shortcut initialized from
 *  the user-defined configuration settings  If the user should be able to edit
 *  and save the keyboard shortcut, insertCommand() should be called or
 *  DesktopServices::initializeAction() should be called prior to adding the
 *  action to the menu.
 *
 *  A second approach to adding and organizing menus and commands on the menu
 *  bar is that an existing menu can be added with the insertMenu() method.
 *
 *  A third approach is that a command can be directly added to the menu bar
 *  with the addCommand() method.  There are two versions of the method, one
 *  that allows the user to edit and save a keyboard shortcut, and one that
 *  does not allow the user to edit and save a keyboard shortcut.  Both
 *  methods take a string location, which will create any menus and submenus as
 *  necessary.  This means that the menu and/or submenus do not need to be
 *  created before adding the command to the menu bar.
 *
 *  The MenuBar class should not be used by plug-ins that desire the
 *  Executable::execute() method to be called when a command is invoked.  Use
 *  the PlugInShell::setMenuLocation() and PlugInShell::addMenuLocation()
 *  methods instead, or override Executable::getMenuLocations().
 */
class MenuBar
{
public:
   /**
    *  Creates a menu on the menu bar or a submenu on an existing menu.
    *
    *  @param   location
    *           The full location and name of the menu relative to this menu
    *           bar.  The string should contain slashes between menu and
    *           submenu names.  The last name in the string should be the name
    *           of the menu to add.  Examples of valid menu location strings
    *           are as follows:
    *           - &File/&Import
    *           - Geo
    *  @param   pBefore
    *           The menu or command that should immediately follow the added
    *           menu.  The menu is added at the end of the menu bar or submenu
    *           if \b NULL is passed in or if the given action does not exist
    *           on the parent menu or menu bar.
    *
    *  @return  A pointer to the new Qt menu.  \b NULL is returned if the menu
    *           already exists.
    *
    *  @see     insertMenu()
    */
   virtual QMenu* addMenu(const std::string& location, QAction* pBefore = NULL) = 0;

   /**
    *  Adds an existing menu to the menu bar or a submenu.
    *
    *  @param   pMenu
    *           A pointer to the existing Qt menu to add.
    *  @param   pParentMenu
    *           A pointer to the menu that should be the parent menu of
    *           \em pMenu.  If NULL is passed in, the menu will be added
    *           directly to the menu bar.
    *  @param   pBefore
    *           The menu or command that should immediately follow the added
    *           menu.  The menu is added at the end of the menu bar or submenu
    *           if \b NULL is passed in or if the given action does not exist
    *           on the parent menu or menu bar.
    *
    *  @return  A pointer to the Qt action created by the menu bar or submenu
    *           to which the menu was added.  A valid value is returned if the
    *           given menu already exists on the menu bar or the given parent
    *           menu.
    */
   virtual QAction* insertMenu(QMenu* pMenu, QMenu* pParentMenu = NULL, QAction* pBefore = NULL) = 0;

   /**
    *  Creates a command on a menu in the menu bar.
    *
    *  This method creates a command on a menu in the the menu bar and creates
    *  menus and submenus as necessary.  The command is added directly to a
    *  menu and the user will not be able to edit and save a keyboard shortcut
    *  in the user-defined configuration settings.  To allow the user to edit
    *  the keyboard shortcut, call the overloaded
    *  addCommand(const std::string&, const std::string&, QAction*) method
    *  instead.
    *
    *  @param   location
    *           The full location and name of the command relative to this menu
    *           bar.  The string should contain slashes between menu and
    *           submenu names and the command name.  The last name in the
    *           string should be the name of the command to add.  Examples of
    *           valid command location strings are as follows:
    *           - &File/&Import/Layer
    *           - Geo/Georeference
    *  @param   pBefore
    *           The menu or command that should immediately follow the added
    *           command.  The command is added at the end of the menu bar or
    *           submenu if \b NULL is passed in or if the given action does not
    *           exist on the parent menu.
    *
    *  @return  A pointer to the Qt action created by the menu to which the
    *           command was added.  A valid value is returned if the given
    *           command already exists.
    *
    *  @see     insertCommand()
    */
   virtual QAction* addCommand(const std::string& location, QAction* pBefore = NULL) = 0;

   /**
    *  Creates a command on a menu in the menu bar.
    *
    *  This method creates a command on a menu in the the menu bar and creates
    *  menus and submenus as necessary.  The command is added directly to a
    *  menu and a given shortcut context allows the user to edit and save the
    *  keyboard shortcut in the user-defined configuration settings.  If the
    *  should not be able to edit the shortcut, call the overloaded
    *  addCommand(const std::string&, QAction*) method instead or first create
    *  the menu and add the command directly to the menu.
    *
    *  @param   location
    *           The full location and name of the command relative to this menu
    *           bar.  The string should contain slashes between menu and
    *           submenu names and the command name.  The last name in the
    *           string should be the name of the command to add.  Examples of
    *           valid command location strings are as follows:
    *           - &File/&Import/Layer
    *           - Geo/Georeference
    *  @param   shortcutContext
    *           The context name for the keyboard shortcut of the action.  When
    *           the user edits keyboard shortcuts, the shortcuts are grouped
    *           according to their context string.  Contexts can be nested by
    *           using a slash (/) between context levels.  If the top-level
    *           context name is the same as a plug-in name, the shortcut is
    *           grouped under a single Plug-Ins context.
    *  @param   pBefore
    *           The menu or command that should immediately follow the added
    *           command.  The command is added at the end of the menu bar or
    *           submenu if \b NULL is passed in or if the given action does not
    *           exist on the parent menu.
    *
    *  @return  A pointer to the Qt action created by the menu to which the
    *           command was added.  A valid value is returned if the given
    *           command already exists.
    *
    *  @see     insertCommand()
    */
   virtual QAction* addCommand(const std::string& location, const std::string& shortcutContext,
      QAction* pBefore = NULL) = 0;

   /**
    *  Adds an existing command to a given menu on the menu bar.
    *
    *  This method adds an existing command to a menu in the the menu bar and
    *  allows the user to edit and save a keyboard shortcut based on a given
    *  context.  If the should not be able to edit the shortcut, call the
    *  QMenu::insertAction() method instead.
    *
    *  @param   pCommand
    *           The command to add.
    *  @param   pMenu
    *           A pointer to the menu to which \em pCommand should be added.
    *           This method does nothing if \b NULL is passed in.
    *  @param   shortcutContext
    *           The context name for the keyboard shortcut of the action.  When
    *           the user edits keyboard shortcuts, the shortcuts are grouped
    *           according to their context string.  Contexts can be nested by
    *           using a slash (/) between context levels.  If the top-level
    *           context name is the same as a plug-in name, the shortcut is
    *           grouped under a single Plug-Ins context.  Passing in an empty
    *           string indicates that the user should not be able to edit the
    *           keyboard shortcut of the command, which is the same as calling
    *           QMenu::insertAction().
    *  @param   pBefore
    *           The menu or command that should immediately follow the added
    *           command.  The command is added at the end of the menu bar or
    *           submenu if \b NULL is passed in or if the given action does not
    *           exist on the given menu.
    *
    *  @return  Returns \b true if the command was successfully added;
    *           otherwise returns \b false.
    *
    *  @see     addCommand()
    */
   virtual bool insertCommand(QAction* pCommand, QMenu* pMenu, const std::string& shortcutContext,
      QAction* pBefore = NULL) = 0;

   /**
    *  Retrieves the menu or command at a given location.
    *
    *  @param   location
    *           The full location and name of the command relative to this menu
    *           bar.  The string should contain slashes between menu and
    *           submenu names and the command name.  The last name in the
    *           string should be the name of the command to add.  Examples of
    *           valid command location strings are as follows:
    *           - &File/&Import/Layer
    *           - Geo/Georeference
    *
    *  @return  A pointer to the Qt action represented by the given menu or
    *           command.  \b NULL is returned if the menu or command in the
    *           given location does not exist.
    */
   virtual QAction* getMenuItem(const std::string& location) const = 0;

   /**
    *  Retrieves the Qt action associated with a given menu.
    *
    *  This method differs from QMenu::menuAction() in that a valid value is
    *  returned only if the menu exists on the menu bar or one of its menus.
    *
    *  @param   pMenu
    *           The menu for which to get its associated Qt action.
    *
    *  @return  A pointer to the Qt action represented by the given menu.
    *           \b NULL is returned if the given menu does not exist on the
    *           menu bar or one of its menus.
    */
   virtual QAction* getMenuItem(QMenu* pMenu) const = 0;

   /**
    *  Retrieves the menu associated with a given Qt action.
    *
    *  @param   pAction
    *           The Qt action for which to get its associated menu.
    *
    *  @return  A pointer to the Qt menu represented by the given action.
    *           \b NULL is returned if the given menu action does not exist on
    *           the menu bar or one of its menus or if the given action does
    *           not represent a menu.
    *
    *  @see     isMenu()
    */
   virtual QMenu* getMenu(QAction* pAction) const = 0;

   /**
    *  Queries whether a given Qt action is associated with a menu.
    *
    *  @param   pAction
    *           The Qt action to query if is represents a menu.
    *
    *  @return  This method returns \b true if the given action is represented
    *           by a menu or submenu contained on the menu bar; otherwise
    *           returns \b false.
    *
    *  @see     getMenu(), isCommand()
    */
   virtual bool isMenu(QAction* pAction) const = 0;

   /**
    *  Queries whether a given Qt action is associated with a command.
    *
    *  @param   pAction
    *           The Qt action to query if is represents a command.
    *
    *  @return  This method returns \b true if the given action is represented
    *           by a command contained on the menu bar or one of its menus;
    *           otherwise returns \b false.
    *
    *  @see     isMenu()
    */
   virtual bool isCommand(QAction* pAction) const = 0;

   /**
    *  Removes a menu from the menu bar or one of its menus.
    *
    *  If the menu is successfully removed, it is also deleted.
    *
    *  @param   pMenu
    *           The menu to remove.
    *
    *  @return  This method returns \b true if the menu is successfully removed
    *           and deleted; otherwise returns \b false.
    */
   virtual bool removeMenu(QMenu* pMenu) = 0;

   /**
    *  Removes a menu or command from the menu bar or one of its menus.
    *
    *  If a menu is successfully removed, it is also deleted.
    *
    *  @param   pAction
    *           The Qt action representing the menu or command to remove.
    *
    *  @return  This method returns \b true if the menu or command is
    *           successfully removed; otherwise returns \b false.
    */
   virtual bool removeMenuItem(QAction* pAction) = 0;

protected:
   /**
    * This should not be deleted directly.
    */
   virtual ~MenuBar() {}
};

#endif
