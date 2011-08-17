/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SPATIALDATAWINDOW_H
#define SPATIALDATAWINDOW_H

#include "ColorType.h"
#include "WorkspaceWindow.h"

#include <string>

class SpatialDataView;

/**
 *  A window containing a spatial data view.
 *
 *  The spatial data window is a type of workspace window that contains a spatial
 *  data view.  The class is provided for convenience when creating and using
 *  product views.  The window also provides a means to export a subset of the
 *  raster data in the view.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - Everything documented in WorkspaceWindow.
 *
 *  @see     WorkspaceWindow, SpatialDataView
 */
class SpatialDataWindow : public WorkspaceWindow
{
public:
   SETTING(OverviewTrailColor, SpatialDataWindow, ColorType, ColorType())
   SETTING(OverviewTrailThreshold, SpatialDataWindow, unsigned int, 100)

   /**
    *  Returns the spatial data view contained in the window.
    *
    *  @return  A pointer to the spatial data view displayed in the window.
    *
    *  @see     SpatialDataView
    */
   virtual SpatialDataView* getSpatialDataView() const = 0;

   /**
    *  Allows the user to export a subset of the raster element.
    *
    *  This method invokes a dialog in which the user can graphically select
    *  a subset area of the raster element in the view to export to either
    *  another spatial data window or to a file.
    */
   virtual void exportSubset() = 0;

   /**
    *  Toggles the display of the overview window.
    *
    *  The overview window contains a small thumbnail view of the view in this
    *  window.  A selection box in the overview illustrates the displayed area of
    *  this window.  The display in this window can be panned and zoomed by
    *  clicking a dragging the selection box in the overview.
    *
    *  @param   bShow
    *           Set this value to TRUE to show the overview window or to FALSE to
    *           hide the overview window.
    */
   virtual void showOverviewWindow(bool bShow) = 0;

   /**
    *  Queries whether the overview window is displayed.
    *
    *  @return  TRUE if the overview window is displayed or FALSE if the overview
    *           window is hidden.
    */
   virtual bool isOverviewWindowShown() const = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteWindow.
    */
   virtual ~SpatialDataWindow() {}
};

#endif
