/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <QtGui/QMdiArea>

#include "TypesFile.h"

#include <vector>

class QMdiSubWindow;
class WorkspaceWindow;

/**
 *  Provides method to control tiling of views in desktop.
 *
 *  Extends QWorkspace to allow control of how windows are tiled. Only the windows (views)
 *  in the passed list are displayed. The views are displayed in two rows by number of columns
 *  needed to display all the views in the list. For an odd number of views, either the first or the last
 *  view is displayed in a full column (twice height of other views) based on value of passed boolean, maxFirst.
 *
 *  @see     ApplicationWindow
 *  @warning Because of the custom tiling, this class hides the base class cascadeSubWindows() method. Use caution
 *           when calling this method as it is not virtual and pointers to the base class will not work correctly
 *           when custom tiling is in effect. Pointers to type Workspace should be used whenever possible.
 */

class Workspace : public QMdiArea
{
public:
   Workspace(QWidget *parent = 0);
   bool tileWindows(const std::vector<WorkspaceWindow*>& windows, 
                    bool maxFirst = true, const TilingType eType = TILE_GRID);

public slots:
   void tile(const TilingType eType = TILE_GRID);
   void cascadeSubWindows();

protected:
   void resizeEvent(QResizeEvent* e);

private:
   Workspace(const Workspace& rhs);
   Workspace& operator=(const Workspace& rhs);
   bool mbCustomTiling;
   bool mMaxFirst;
   TilingType mTilingType;
   std::vector<QMdiSubWindow*> mTileWindows;
   void refreshCustomView();
   void setSubWindow(QMdiSubWindow* pSubWindow, int x, int y, int winWidth, int winHeight);
};

#endif
