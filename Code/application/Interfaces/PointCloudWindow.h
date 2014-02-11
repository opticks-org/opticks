/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDWINDOW_H
#define POINTCLOUDWINDOW_H

#include "WorkspaceWindow.h"

class PointCloudView;

/**
 *  A window containing a point cloud view.
 *
 *  The point cloud window is a type of workspace window that contains a point
 *  cloud view. 
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - Everything documented in WorkspaceWindow.
 *
 *  @see WorkspaceWindow, PointCloudView
 */
class PointCloudWindow : public WorkspaceWindow
{
public:
   /**
    * Accessor for the point cloud view contained in this window.
    *
    * @return A pointer to the view.
    */
   virtual PointCloudView* getPointCloudView() const = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteWindow.
    */
   virtual ~PointCloudWindow() {}
};

#endif