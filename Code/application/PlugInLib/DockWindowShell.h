/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DOCKWINDOWSHELL_H
#define DOCKWINDOWSHELL_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtGui/QAction>
#include <QtGui/QWidget>

#include "AttachmentPtr.h"
#include "DockWindow.h"
#include "ExecutableShell.h"

/**
 *  A plug-in shell class for creating dock windows.
 *
 *  This class provides capabilities to create a dock window in the main
 *  application window.  A dock window plug-in is executed on application
 *  startup and when a new session is created.  The execute() method creates
 *  the dock window with the same name as the plug-in and hides it by default.
 *
 *  Subclasses are only responsible for implementing the following items:
 *  - Creating the action that toggles the display of the dock window and
 *    adding it to a menu and/or toolbar in an override of the createAction()
 *    method.
 *  - Creating the widget that is used as the dock window contents in an
 *    override of the createWidget() method.
 *  - Removing the action from a menu and/or toolbar in the subclass destructor.
 *    Do not delete the action if it is parented to \c this.
 *
 *  Subclasses do not need to destroy the contents widget since it is
 *  automatically destroyed when the dock window is destroyed.
 *
 *  @see     ExecutableShell
 */
class DockWindowShell : public QObject, public ExecutableShell
{
   Q_OBJECT

public:
   using ExecutableShell::setName;

   /**
    *  Creates a dock window plug-in.
    *
    *  The constructor sets the following values on the plug-in:
    *  - The type is set to PlugInManagerServices::DockWindowType().
    *  - Multiple instances are not allowed.
    *  - The plug-in is executed on startup.
    *  - The plug-in is not destroyed after executing.
    *  - Wizard support is disabled.
    *  - The plug-in defaults to execute in interactive mode.
    *
    *  @see     getType()
    */
   DockWindowShell();

   /**
    *  Destroys the dock window plug-in.
    *
    *  The destructor destroys the dock window.  Subclasses should remove the
    *  action created in createAction() from the menu and/or toolbar to which
    *  it was added.  Subclasses do not need
    *  to destroy the contents widget created in createWidget() since it is
    *  destroyed when the dock window is destroyed.
    */
   virtual ~DockWindowShell();

   /**
    *  @copydoc Executable::setBatch()
    *
    *  @default The default implementation calls the base class to set the
    *           internal flag and returns \c false indicating that batch mode
    *           operation is not supported.
    */
   virtual bool setBatch();

   /**
    *  @copydoc Executable::getInputSpecification()
    *
    *  @default The default implementation sets \em pArgList to \c NULL and
    *           returns \c true.
    */
   virtual bool getInputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc Executable::getOutputSpecification()
    *
    *  @default The default implementation sets \em pArgList to \c NULL and
    *           returns \c true.
    */
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);

   /**
    *  Creates the dock window.
    *
    *  This method is called on application startup and when a new session is
    *  created.  The createAction() method is called for subclasses to create
    *  an action and add it to a menu and/or toolbar.  If a valid non-\c NULL
    *  action is returned, this method sets the action to be a checkable action,
    *  sets the action to not auto-repeat, and connects its triggered() signal
    *  to the displayDockWindow() slot.
    *
    *  After initializing the action, the dock window is created by calling
    *  DesktopServices::createWindow() with the window name set to the same name
    *  as the plug-in.  If the dock window already exists, the existing window
    *  is used instead of creating a new window.  If a new window is created, it
    *  is hidden by default.
    *
    *  After the dock window is created or the existing dock window is obtained,
    *  the icon which appears in the session explorer is set to the created
    *  action's icon, and the createWidget() method is called to create the
    *  widget that should be used as the dock window's contents widget.  If
    *  createWidget() returns a valid non-\c NULL widget, this method
    *  automatically sets it into the dock window by calling
    *  DockWindow::setWidget().
    *
    *  @param   pInArgList
    *           This parameter is ignored since the default implementation of
    *           getInputSpecification() does not define any input arguments.
    *  @param   pOutArgList
    *           This parameter is ignored since the default implementation of
    *           getOutputSpecification() does not define any output arguments.
    *
    *  @return  Returns \c true if the action has been successfully created and
    *           the dock window has been successfully created or the existing
    *           window obtained from DesktopServices.  Returns \c false if the
    *           plug-in is set to execute in batch mode, if the call to
    *           createAction() returns \c false, or if the dock window does not
    *           already exist and it could not be created from DesktopServices.
    */
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   /**
    *  Saves the current state of the dock window plug-in.
    *
    *  The current displayed state of the dock window is saved so that the
    *  check state of the action can be properly initialized when loaded.
    *  Subclasses should override this method to save the current state of
    *  the contents widget if necessary.
    *
    *  @param   serializer
    *           The object with which to save the item as part of the current
    *           session.
    *
    *  @return  Returns \c true if the window display state was successfully
    *           saved; otherwise returns \c false.
    */
   virtual bool serialize(SessionItemSerializer& serializer) const;

   /**
    *  Loads the current state of the dock window plug-in.
    *
    *  The displayed state of the dock window is loaded and the check state of
    *  the action initialized.  Subclasses should override this method to load
    *  the current state of the contents widget if necessary.
    *
    *  @param   deserializer
    *           The object from which the item is loaded as part of the current
    *           session.
    *
    *  @return  Returns \c true if the window display state was successfully
    *           loaded; otherwise returns \c false.
    */
   virtual bool deserialize(SessionItemDeserializer& deserializer);

protected:
   /**
    *  Creates the action to toggle the display of the dock window.
    *
    *  This method is called from execute() and subclasses must implement it to
    *  create an action and add it to a menu and/or toolbar.  Typically, the
    *  action is created by calling MenuBar::addCommand().  After the action is
    *  created, only the action icon, tool tip, and status tip need to be set.
    *  The execute() method will set the action to be checkable and to not
    *  auto-repeat, and will connect it to the displayDockWindow() slot.
    *
    *  The action should be removed from the menu and/or toolbar in the subclass destructor.
    *  Do not delete the action if it is parented to \c this.
    *
    *  Subclasses do not need to store a pointer to the created action.  It can
    *  be retrieved by calling getAction().
    *
    *  @return  A pointer to the created action.  The dock window will not be
    *           created and execute() will return \c false if \c NULL is
    *           returned.
    */
   virtual QAction* createAction() = 0;

   /**
    *  Returns the action that is used to toggle the display of the dock window.
    *
    *  @return  Returns a pointer to the action that was created in
    *           createAction().  Returns \c NULL if createAction() has not yet
    *           been called.
    *
    *  @see     getAction() const
    */
   QAction* getAction();

   /**
    *  Returns read-only access to the action that is used to toggle the display
    *  of the dock window.
    *
    *  @return  Returns a const pointer to the action that was created in
    *           createAction().  Returns \c NULL if createAction() has not yet
    *           been called.  The action represented by the returned pointer
    *           should not be modified.  To modify its values, call the
    *           non-const version of getAction() instead.
    *
    *  @see     getAction()
    */
   const QAction* getAction() const;

   /**
    *  Creates the widget that is added to the dock window as its contents.
    *
    *  This method is called from execute() after the action has been
    *  successfully created.  Subclasses do not need to destroy the created
    *  widget because it is automatically destroyed when the dock window is
    *  destroyed.
    *
    *  @return  A pointer to the created widget that is set into the dock
    *           window.  The dock window will still be successfully created if
    *           \c NULL is returned, but the window will be empty.
    */
   virtual QWidget* createWidget() = 0;

   /**
    *  Returns the widget that is set into the dock window.
    *
    *  This method is a convenience method that calls DockWindow::getWidget().
    *
    *  @return  Returns a pointer to the widget that was created in
    *           createWidget().  Returns \c NULL if createWidget() has not yet
    *           been called.
    */
   QWidget* getWidget() const;

   /**
    *  Returns a pointer to the dock window.
    *
    *  @return  Returns a pointer to the dock window that was created in
    *           execute().  Returns \c NULL if the dock window has not yet been
    *           created, or if execute() returned \c false.
    *
    *  @see     getDockWindow() const
    */
   DockWindow* getDockWindow();

   /**
    *  Returns read-only access to the dock window.
    *
    *  @return  Returns a const pointer to the dock window that was created in
    *           execute().  Returns \c NULL if the dock window has not yet been
    *           created, or if execute() returned \c false.  The dock window
    *           represented by the returned pointer should not be modified.  To
    *           modify its values, call the non-const version of getDockWindow()
    *           instead.
    *
    *  @see     getDockWindow()
    */
   const DockWindow* getDockWindow() const;

protected slots:
   /**
    *  Toggles the display state of the dock window.
    *
    *  This slot is called automatically when the user triggers the action
    *  created in createAction().
    *
    *  @param   bDisplay
    *           Shows the window if the value is \c true or hides the window if
    *           the value is \c false.
    */
   void displayDockWindow(bool bDisplay);

private:
   DockWindowShell(const DockWindowShell& rhs);
   DockWindowShell& operator=(const DockWindowShell& rhs);

   void windowShown(Subject& subject, const std::string& signal, const boost::any& value);
   void windowHidden(Subject& subject, const std::string& signal, const boost::any& value);

   QPointer<QAction> mpWindowAction;
   AttachmentPtr<DockWindow> mpDockWindow;
};

#endif
