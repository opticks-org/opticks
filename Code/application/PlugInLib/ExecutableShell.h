/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXECUTABLESHELL_H
#define EXECUTABLESHELL_H

#include "Executable.h"
#include "PlugInShell.h"

#include <list>
#include <string>
#include <vector>

class PlugInArgList;

/**
 *  \ingroup ShellModule
 *  A base class for plug-in shells or plug-in instances.
 *
 *  This class provides a default implementation of the Executable
 *  plug-in interfaces and serves as an optional base class for specialized
 *  plug-in shell classes and/or custom plug-ins.
 *
 *  @see     PlugIn, Executable
 */
class ExecutableShell : public Executable, public PlugInShell
{
public:
   /**
    *  Creates the plug-in.
    *
    *  The constructor initializes the plug-in as follows:
    *  - isExecutedOnStartup() returns \b false.
    *  - isDestroyedAfterExecute() returns \b true.
    *  - isBatch() returns \b true.
    *  - hasAbort() returns \b false.
    *  - isBackground() returns \b false.
    *  - hasWizardSupport() returns \b true.
    *
    *  @see     ConfigurationSettings::getVersion()
    */
   ExecutableShell();

   /**
    *  Destroys the plug-in.
    */
   virtual ~ExecutableShell();

   // Executable
   bool isExecutedOnStartup() const;
   bool isDestroyedAfterExecute() const;
   const std::vector<std::string>& getMenuLocations() const;
   const char** getMenuIcon() const;

   /**
    *  @copydoc Executable::setBatch()
    *
    *  @default The default implementation sets an internal flag that can be
    *           queried with isBatch() and returns \b true.
    */
   bool setBatch();

   /**
    *  @copydoc Executable::setInteractive()
    *
    *  @default The default implementation sets an internal flag that can be
    *           queried with isBatch() and returns \b true.
    */
   bool setInteractive();

   /**
    *  @copydoc Executable::initialize()
    *
    *  @default The default implementation does nothing and returns \b true.
    */
   bool initialize();

   /**
    *  @copydoc Executable::abort()
    *
    *  @default If hasAbort() returns \b true, the default implementation sets
    *           an internal flag that can be queried with isAborted() and
    *           returns \b true.  If hasAbort() returns \b false, the default
    *           implementation returns \b false.
    */
   bool abort();

   /**
    *  @copydoc Executable::hasAbort()
    *
    *  @default The default implementation returns the value that was passed
    *           into setAbortSupported().  If setAbortSupported() has not yet
    *           been called, \b false is returned.
    */
   bool hasAbort();

   /**
    *  @copydoc Executable::isBackground()
    *
    *  @default The default implementation returns the value that was passed
    *           into setBackground().  If setBackground() has not yet been
    *           called, \b false is returned.
    */
   bool isBackground() const;


   /**
    *  Queries whether the plug-in is set to execute in batch mode.
    *
    *  @return  Returns \b true if the plug-in is set to execute in batch mode
    *           or \b false if the plug-in is set to execute in interactive
    *           mode.
    *
    *  @see     setBatch(), setInteractive()
    */
   bool isBatch() const;

   /**
    *  Queries whether the plug-in is in an aborting state.
    *
    *  @return  Returns \b true if the plug-in is currently aborting, otherwise
    *           returns \b false.  Also returns \b false if aborting is not
    *           supported.
    *
    *  @see     hasAbort(), abort()
    */
   bool isAborted() const;

   /**
    * Queries whether the plug-in supports being added as a wizard item.
    *
    * @return Returns \b true if the user should be able to add this
    *         plug-in to the wizard builder.  Returns \b false if
    *         the user should NOT be able to add this plug-in to
    *         the wizard builder.
    *
    * @see setWizardSupported
    */
   bool hasWizardSupport() const;

protected:
   /**
    *  Sets whether the plug-in is automatically executed when the
    *  application starts.
    *
    *  @param   bExecute
    *           Set this value to true if the plug-in should be
    *           automatically executed, otherwise false.
    */
   void executeOnStartup(bool bExecute);

   /**
    *  Sets whether the plug-in is automatically destroyed after it is
    *  successfully executed. Plug-ins which are not successfully executed are
    *  automatically destroyed by the application.
    *
    *  After a plug-in is successfully executed, it can be destroyed by the application
    *  or stay resident in memory.  A plug-in may need to exist after execution,
    *  such as those containing modeless GUI components that require user
    *  interaction or continually display data.
    *
    *  @param   bDestroy
    *           Set this value to true if the plug-in should be automatically
    *           destroyed after it is successfully executed, otherwise false.
    *
    *  @see     execute()
    */
   void destroyAfterExecute(bool bDestroy);

   /**
    *  Sets a single menu location and command name from which the plug-in
    *  is executed.
    *
    *  This method sets the location and name of a single menu command that
    *  is used to execute the plug-in.  %Any existing menu command locations
    *  are removed to set the given location.  Additional menu commands can be
    *  specified to execute this plug-in by calling the addMenuLocation()
    *  method instead.
    *
    *  @param   menuLocation
    *           The menu location and name of the menu commmand used to execute
    *           the plug-in.<br><br>The string should be formatted with
    *           brackets ([,]) to specify a toolbar and a slash (/) to indicate
    *           submenus.  The toolbar name appears first in the string,
    *           followed by an optional slash, and then the menus, submenus and
    *           command name separated by slashes.  If the optional slash
    *           appears following the toolbar name, this indicates that the
    *           submenus and command should be added to the default toolbar
    *           menu, which has the same name as the toolbar.  If a slash does
    *           not follow the toolbar name, the menus and command are added
    *           directly to the toolbar.  If the string does not include a
    *           toolbar name, the menus and command are added to the main menu
    *           bar.  The string cannot end with a slash, and the name after
    *           the last slash indicates the command name.  An empty string
    *           indicates that the plug-in does not have a menu location, and
    *           that it should not be executed from the menus.<br><br>
    *           Examples of the menu string include the following:
    *           <ul><li>[Geo]/Georeference</li>
    *           <li>&Tools/Flicker %Window</li></ul>
    *
    *  @see     addMenuLocation()
    */
   void setMenuLocation(const std::string& menuLocation);

   /**
    *  Adds a menu location and command name from which the plug-in is
    *  executed.
    *
    *  This method adds an additional menu command that is used to execute the
    *  plug-in.  Derived plug-ins that call this method should specify the
    *  following input arg in getInputSpecification():
    *  <ul><li>Name: "Menu Command"</li>
    *  <li>Type: "string"</li></ul>
    *
    *  When the execute() method is called, this plug-in arg in the input arg
    *  list should be populated with the command name that was used to execute
    *  the plug-in.
    *
    *  @param   menuLocation
    *           The menu location and name of the menu commmand used to execute
    *           the plug-in.<br><br>The string should be formatted with
    *           brackets ([,]) to specify a toolbar and a slash (/) to indicate
    *           submenus.  The toolbar name appears first in the string,
    *           followed by an optional slash, and then the menus, submenus and
    *           command name separated by slashes.  If the optional slash
    *           appears following the toolbar name, this indicates that the
    *           submenus and command should be added to the default toolbar
    *           menu, which has the same name as the toolbar.  If a slash does
    *           not follow the toolbar name, the menus and command are added
    *           directly to the toolbar.  If the string does not include a
    *           toolbar name, the menus and command are added to the main menu
    *           bar.  The string cannot end with a slash, and the name after
    *           the last slash indicates the command name.  An empty string
    *           indicates that the plug-in does not have a menu location, and
    *           that it should not be executed from the menus.<br><br>
    *           Examples of the menu string include the following:
    *           <ul><li>[Geo]/Georeference</li>
    *           <li>&Tools/Flicker %Window</li></ul>
    *
    *  @see     setMenuLocation()
    */
   void addMenuLocation(const std::string& menuLocation);

   /**
    *  Sets an optional icon for a menu command or toolbar button from which
    *  the plug-in is executed.
    *
    *  @param   pIcon
    *           The icon in XPM format.
    *
    *  @see     setIcon()
    */
   void setMenuIcon(const char* pIcon[]);

   /**
    *  Sets whether the plug-in supports aborting.
    *
    *  @param   bSupported
    *           If this value is \b true, aborting is enabled and the default
    *           implementation of hasAbort() will return \b true.  If this
    *           value is \b false, aborting is disabled and the default
    *           implementation of hasAbort() will return \b false.
    */
   void setAbortSupported(bool bSupported);

   /**
    *  Sets whether the plug-in performs background processing.
    *
    *  @param   bBackground
    *           If this value is \b true, the plug-in is set to perform
    *           background processing.  Destruction and possible unloading of
    *           the plug-in will be delayed until the plug-in indicates that
    *           background processing has ended.
    *
    *  @see     isBackground(), DesktopServices::registerCallback()
    */
   void setBackground(bool bBackground);

   /**
    * Sets whether the plug-in supports being added to a wizard.
    *
    * @param bSupported
    *        If this value is \b true, then a user will
    *        be able to add this plug-in to a wizard in the wizard builder.
    *        If this value is \b false, then a user will
    *        not be able to add this plug-in to a wizard
    *        in the wizard builder.
    *
    * @see hasWizardSupport()
    */
   void setWizardSupported(bool bSupported);

   /**
    * Set to \c true if the plug-in has been aborted.
    *
    * Subclasses should typically use isAborted().  This is
    * only made available for the case where a pointer or reference
    * to an abort flag is passed into other code.
    */
   mutable bool mAborted;

private:
   bool mExecuteOnStartup;
   bool mDestroyAfterExecute;
   std::vector<std::string> mMenuLocations;
   const char** mpMenuIcon;
   bool mBatch;
   bool mHasAbort;
   bool mBackground;
   bool mHasWizard;
};

#endif
