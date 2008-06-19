/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Workspace.h"
#include "DesktopServices.h"
#include "WorkspaceWindow.h"
#include "WorkspaceWindowImp.h"

using namespace std;

Workspace::Workspace(QWidget* parent) :
   QWorkspace(parent),
   mbCustomTiling(false),
   mMaxFirst(true),
   mTilingType(TILE_GRID)
{
}

void Workspace::resizeEvent(QResizeEvent * e)
{
   QWorkspace::resizeEvent(e);
   if (mbCustomTiling)
   {
      QWidgetList Windows = windowList();
      QWidget *pWidget = NULL;

      int NumViews = mTileWindows.size();
      bool ListValid = (NumViews > 0);
      for (int i=0; i<NumViews; i++)
      {
         pWidget = mTileWindows[i];
         if (!Windows.contains(pWidget))
         {
            ListValid = false;
         }
      }

      if (ListValid)
      {
         refreshCustomView();
      }
      else
      {
         mbCustomTiling = false;
         mTileWindows.clear();
      }
   }
}

void Workspace::refreshCustomView()
{
   if (mbCustomTiling)
   {
      int numViews = mTileWindows.size();

      // if vector of windows is empty, tile all windows
      if (numViews < 1)
      {
         QWidgetList workspaceWindows = windowList();
         numViews = workspaceWindows.size();
         for (int i=0; i<numViews; ++i)
         {
            mTileWindows.push_back(workspaceWindows.at(i));
         }
      }

      int x;
      int y;
      int rowHeight;
      int colWidth;
      int leftOver;

      switch(mTilingType)
      {
      case TILE_GRID:
         {
            // handle special case for 1 window in list
            if (mTileWindows.size() == 1)
            {
               setWindow(mTileWindows.front(), 0, 0, width(), height());
               break;
            }

            bool odd(numViews % 2 == 1);
            int startAt(0);
            int evenColStart(0);
            int runTo(numViews-1);
            int halfViews = numViews / 2;
            colWidth = width() / halfViews;
            int oddColWidth(0);
            y = 0;
            if (odd)
            {
               rowHeight = height();
               colWidth = width() / (halfViews+1);
               oddColWidth = width() - colWidth * halfViews;
               QWidget* pWidget;
               if (mMaxFirst)
               {
                  pWidget = mTileWindows.at(startAt++);
                  x = 0;
                  evenColStart = oddColWidth;
               }
               else
               {
                  pWidget = mTileWindows.at(runTo--);
                  x = halfViews * colWidth;
               }
               setWindow(pWidget, x, y, oddColWidth, rowHeight);
            }

            rowHeight = height() / 2;
            int leftOver = width() - oddColWidth - colWidth * halfViews;
            
            int colPad(leftOver);
            if (leftOver > 0)
            {
               ++colWidth;
            }
            else
            {
               colPad = -1;
            }

            x = evenColStart;
            for (int i=startAt; i<startAt+halfViews; ++i)
            {
               if (colPad-- == 0)
               {
                  --colWidth;
               }
               setWindow(mTileWindows.at(i), x, y, colWidth, rowHeight);
               x += colWidth;
            }

            y += rowHeight;
            x = evenColStart;
            colPad = leftOver;
            if (leftOver > 0)
            {
               ++colWidth;
            }
            else
            {
               colPad = -1;
            }
            for (int i=startAt+halfViews; i<=runTo; ++i)
            {
               if (colPad-- == 0)
               {
                  --colWidth;
               }
               setWindow(mTileWindows.at(i), x, y, colWidth, rowHeight);
               x += colWidth;
            }
         }
         break;

      case TILE_HORIZONTAL:
         rowHeight = height();
         colWidth = width() / numViews;
         leftOver = width() - colWidth * numViews;
         if (leftOver > 0)
         {
            ++colWidth;
         }
         x = 0;
         y = 0;
         for (int i=0; i<numViews ; ++i)
         {
            if (leftOver-- == 0)
            {
               --colWidth;
            }
            setWindow(mTileWindows.at(i), x, y, colWidth, rowHeight);
            x += colWidth;
         }
         break;

      case TILE_VERTICAL:
         rowHeight = height() / numViews;
         colWidth = width();
         leftOver = height() - rowHeight * numViews;
         if (leftOver > 0)
         {
            ++rowHeight;
         }
         x = 0;
         y = 0;
         for (int i=0; i<numViews ; ++i)
         {
            if (leftOver-- == 0)
            {
               --rowHeight;
            }
            setWindow(mTileWindows.at(i), x, y, colWidth, rowHeight);
            y += rowHeight;
         }
         break;
      default:
         return;
         break;
      }

      Service<DesktopServices> pDesktop;
      if (pDesktop.get() != NULL)
      {
         WorkspaceWindow* pWork = dynamic_cast<WorkspaceWindow*>(mTileWindows.front());
         if (pWork != NULL)
         {
            pDesktop->setCurrentWorkspaceWindow(pWork);
         }
      }
   }
}

void Workspace::cascade()
{
   mbCustomTiling = false;
   mTileWindows.clear();
   QWorkspace::cascade();
}

void Workspace::tile(const TilingType eType)
{
   mbCustomTiling = false;
   mTilingType = eType;
   mTileWindows.clear();

   QWidgetList windows = windowList();
   int numWindows = windows.count();
   if (numWindows < 2) {
      QWorkspace::tile();
      return;
   }

   int leftOver(0);
   switch(mTilingType)
   {
   case TILE_GRID:
      QWorkspace::tile();
      break;
   case TILE_HORIZONTAL:
      {
         int wWidth = width() / numWindows;
         leftOver = width() - wWidth * numWindows;
         if (leftOver > 0)
         {
            ++wWidth;
         }
         int x = 0;
         QWidget* pWidget(NULL);
         for (int i=0; i<numWindows; ++i) 
         {
            pWidget = windows.at(i);
            if (pWidget != NULL)
            {
               if (leftOver-- == 0)
               {
                  --wWidth;
               }
               setWindow(pWidget, x, 0, wWidth, height());
               x += wWidth;
            }
         }
      }
      break;
   case TILE_VERTICAL:
      {
         int wHeight = height() / numWindows;
         leftOver = height() - wHeight * numWindows;
         if (leftOver > 0)
         {
            ++wHeight;
         }
         int y = 0;
         QWidget* pWidget(NULL);
         for (int i=0; i<numWindows; ++i) {
            pWidget = windows.at(i);
            if (pWidget != NULL)
            {
               if (leftOver-- == 0)
               {
                  --wHeight;
               }
               setWindow(pWidget, 0, y, width(), wHeight);
               y += wHeight;
            }
         }
      }
      break;
   default:
       break;
   }
}

bool Workspace::tileWindows(const vector<WorkspaceWindow*>& windows, 
                            bool maxFirst, const TilingType eType)
{
   mbCustomTiling  = false;
   mTilingType = eType;
   mTileWindows.clear();

   QWidgetList workspaceWindows = windowList();
   for (unsigned int i = 0; i < windows.size(); i++)
   {
      WorkspaceWindowImp* pWindow = dynamic_cast<WorkspaceWindowImp*>(windows.at(i));
      if (pWindow != NULL)
      {
         if (workspaceWindows.contains(pWindow) == true)
         {
            mTileWindows.push_back(pWindow);
         }
      }
   }

   if (windows.size() != mTileWindows.size())
   {
      return false;
   }

   mbCustomTiling  = true;
   mMaxFirst = maxFirst;
   refreshCustomView();

   return true;
}

void Workspace::setWindow(QWidget* pWidget, int x, int y, int winWidth, int winHeight)
{
   if (pWidget == NULL)
   {
      return;
   }

   pWidget->showNormal();

   QWidget* pParent = pWidget->parentWidget();
   if (pParent != NULL)
   {
      pParent->resize(winWidth, winHeight);
      pParent->move(x, y);
      pParent->raise();
   }
}
