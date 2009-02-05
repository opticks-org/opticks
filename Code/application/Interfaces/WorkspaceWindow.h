/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WORKSPACEWINDOW_H
#define WORKSPACEWINDOW_H

#include "ViewWindow.h"
#include "ConfigurationSettings.h"

class View;

/**
 *  Settings, characteristics, and features for windows displayed in the workspace area
 *  of the main application window.
 *
 *  Workspace windows are windows that are restricted to the workspace area of the main
 *  application window.  The workspace window is the primary tool for displaying data.
 *  A workspace window can float in the workspace area or be maximized or minimized in
 *  the workspace.  For each workspace window, a smaller overview window can also be
 *  displayed providing a thumbnail display of the entire view contents.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - Everything documented in ViewWindow.
 *
 *  @see     ViewWindow
 */
class WorkspaceWindow : public ViewWindow
{
public:
   SETTING(WindowHeight, WorkspaceWindow, unsigned int, 250)
   SETTING(WindowSize, WorkspaceWindow, WindowSizeType, MAXIMIZED)
   SETTING(WindowWidth, WorkspaceWindow, unsigned int, 450)
   SETTING(WindowPercentage, WorkspaceWindow, unsigned int, 40)
   SETTING(ConfirmClose, WorkspaceWindow, bool, true)

   /**
    *  Minimizes the window in the workspace.
    *
    *  Hides the display area of the window, leaving only the title bar showing.
    *  The title bar is then moved to the bottom of the workspace area.
    */
   virtual void minimize() = 0;

   /**
    *  Maximizes the window in the workspace.
    *
    *  Increases the size of the window to the maximum display area within the
    *  workspace.  Once maximized in the workspace, the window name is added
    *  to the application title bar in brackets [].
    */
   virtual void maximize() = 0;

   /**
    *  Maximizes the window over the entire screen area.
    *
    *  This method increases the size of the window to cover the entire area of
    *  the screen.  All other windows and toolbars for all applications are
    *  hidden.
    */
   virtual void fullScreen() = 0;

   /**
    *  Returns the window to its normal display size in the workspace.
    *
    *  This method restores a minimized, maximized, or full screen window to a
    *  floating state within the workspace area.  The size of the window returns
    *  to its previous size before it was minimized or maximized.
    */
   virtual void restore() = 0;

   /**
    *  Returns the active view in the window.
    *
    *  @return  A pointer to the active View in the window.  If the window is a
    *           SpatialDataWindow, a SpatialDataView is returned.  If the
    *           window is a ProductWindow, a ProductView is returned if the
    *           product view does not have an active edit view.  Otherwise, the
    *           active edit view in the product view is returned.  \b NULL is
    *           returned if the window does not contain an active View.
    *
    *  @see     ProductView::getActiveEditView()
    */
   virtual View* getActiveView() const = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteWindow.
    */
   virtual ~WorkspaceWindow() {}
};

#endif
