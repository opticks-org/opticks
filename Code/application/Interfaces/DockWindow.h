/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DOCKWINDOW_H
#define DOCKWINDOW_H

#include "ViewWindow.h"

class QMenu;

/**
 *  Settings, characteristics, and features for docking windows.
 *
 *  This class defines all of the services available to dock windows.  A dock
 *  window can attach to an edge of the main application window,or can float
 *  independently on the desktop.  When the user right-clicks in the window's
 *  context, a \ref contextmenus "context menu" is invoked allowing the user to
 *  change the display characteristics of the window.  The window also contains
 *  a button to hide itself.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: dock(), undock(), show(), and hide().
 *  - The user right-clicks in the window to invoke a context menu.
 *  - Everything documented in ViewWindow.
 *
 *  @see     ViewWindow
 */
class DockWindow : public ViewWindow
{
public:
   /**
    *  Emitted when the window is docked.
    */
   SIGNAL_METHOD(DockWindow, Docked)

   /**
    *  Emitted when the window is undocked.
    */
   SIGNAL_METHOD(DockWindow, Undocked)

   /**
    *  Emitted when the window is shown.
    */
   SIGNAL_METHOD(DockWindow, Shown)

   /**
    *  Emitted when the window is hidden.
    */
   SIGNAL_METHOD(DockWindow, Hidden)

   /**
    *  Emitted with boost::any<ContextMenu*> when the user right-clicks to
    *  invoke a context menu.
    *
    *  This signal provides a means by which an object can be notified when a
    *  context menu is invoked by the user clicking inside a dock window.  To
    *  receive notification for when a context menu is invoked when the user
    *  clicks on any session item, attach to the
    *  DesktopServices::signalAboutToShowContextMenu() signal instead.
    *
    *  This signal is emitted after getContextMenuActions() is called and
    *  after the DesktopServices::signalAboutToShowContextMenu() signal is
    *  emitted, but before the context menu is shown to give attached objects a
    *  chance to add or modify the context menu that will be displayed to the
    *  user.
    *
    *  The ContextMenu pointer value is guaranteed to be non-\c NULL.  The
    *  session items vector in the context menu contains the dock window.
    *
    *  @see     \ref callingsequence "Context menu calling sequence"
    */
   SIGNAL_METHOD(DockWindow, AboutToShowContextMenu)

   /**
    *  Associates an icon with the dock window.
    *
    *  The icon associated with the dock window will appear in the session
    *  explorer.
    *
    *  @param   icon
    *           The icon to associate with the dock window.
    *
    *  @see     getIcon()
    */
   virtual void setIcon(const QIcon& icon) = 0;

   /**
    *  @copydoc SessionItem::getContextMenuActions()
    *
    *  @default The default implementation returns the context menu actions
    *           listed \ref dockwindow "here".  The default actions can be
    *           removed or additional actions can be added by attaching to the
    *           signalAboutToShowContextMenu() signal.
    */
   virtual std::list<ContextMenuAction> getContextMenuActions() const = 0;

   /**
    *  Attaches the view to the edge of the main application window.
    *
    *  A docked window is fixed to an adge of the application window and can be
    *  resized in one direction only.  The title bar is replaced by a gripper bar
    *  with small buttons to hide the window or maximize available space if other
    *  views are docked along the same wdge of the application window.  A docked
    *  window cannot be minimized or maximized.
    *
    *  @notify  This method will notify signalModified() if the window was
    *           successfully docked from a previously undocked state.
    *
    *  @see     undock()
    */
   virtual void dock() = 0;

   /**
    *  Floats a docked window over the desktop.
    *
    *  This method detaches a docked window from the main application window and
    *  floats it over the desktop.  The title bar reappears, and the window can
    *  be minimized and maximized.
    *
    *  @notify  This method will notify signalModified() if the window was
    *           successfully undocked from a previously docked state.
    *
    *  @see     dock()
    */
   virtual void undock() = 0;

   /**
    *  Shows the dock window.
    *
    *  This method displays the dock window in its former state, docked or floating.
    *  If the dock window is already displayed, this method does nothing.
    *
    *  @notify  This method will notify signalModified() if the window was
    *           successfully displayed from a previously hidden state.
    *
    *  @see     hide()
    */
   virtual void show() = 0;

   /**
    *  Hides the dock window.
    *
    *  This method hides the dock window but does not delete it.  A hidden dock window
    *  can be displayed again from the main application right-click menu.  If the dock
    *  window is already hidden, this method does nothing.
    *
    *  @notify  This method will notify signalModified() if the window was
    *           successfully hidden from a previously visible state.
    *
    *  @see     show()
    */
   virtual void hide() = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteWindow.
    */
   virtual ~DockWindow() {}
};

#endif
