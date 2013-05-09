/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "Workspace.h"
#include "WorkspaceWindow.h"
#include "WorkspaceWindowImp.h"

#include <QtCore/QEvent>

#include <algorithm>
using namespace std;

Workspace::Workspace(QWidget* pParent) :
   QMdiArea(pParent),
   mMaxFirst(true),
   mTilingType(TILE_GRID),
   mUpdatingWorkspace(false)
{
   // This controls how subwindows are tiled and/or cascaded by the base class.
   setActivationOrder(QMdiArea::StackingOrder);

   // Set the minimum size to (1, 1) to allow dock windows to take over as much of the workspace as possible.
   // Setting this to (0, 0) causes the minimum size to be ignored.
   setMinimumSize(1, 1);
}

Workspace::~Workspace()
{}

bool Workspace::eventFilter(QObject* pObject, QEvent* pEvent)
{
   bool value = QMdiArea::eventFilter(pObject, pEvent);

   // An event filter is automatically installed on each QMdiSubWindow by the QMdiArea base class, so an event
   // filter should not be explicitly installed or removed in this class
   QMdiSubWindow* pWindow = qobject_cast<QMdiSubWindow*>(pObject);
   if ((pWindow != NULL) && (pEvent != NULL))
   {
      // Disable custom tiling when one of the tiled windows is moved or resized
      if (std::find(mTileWindows.begin(), mTileWindows.end(), pWindow) != mTileWindows.end())
      {
         QEvent::Type eventType = pEvent->type();
         if ((eventType == QEvent::Move) || (eventType == QEvent::Resize))
         {
            // Do not disable custom tiling if the window move or resize is caused by updating the tiled windows
            // within the workspace as a result of performing the initial tiling or resizing the workspace
            if (mUpdatingWorkspace == false)
            {
               disableCustomTiling();
            }
         }
      }
   }

   return value;
}

void Workspace::resizeEvent(QResizeEvent* pEvent)
{
   QMdiArea::resizeEvent(pEvent);

   // Update the tiled windows to fit the new size of the MDI area
   if (mTileWindows.empty() == false)
   {
      QList<QMdiSubWindow*> windows = subWindowList();

      // Disable custom tiling if one of the tiled windows was deleted
      bool listValid = true;
      for (vector<QMdiSubWindow*>::const_iterator iter = mTileWindows.begin(); iter != mTileWindows.end(); ++iter)
      {
         QMdiSubWindow* pWindow = *iter;
         VERIFYNRV(pWindow != NULL);

         if (windows.contains(pWindow) == false)
         {
            listValid = false;
            break;
         }
      }

      if (listValid)
      {
         refreshCustomView();
      }
      else
      {
         disableCustomTiling();
      }
   }
}

void Workspace::disableCustomTiling()
{
   if (mTileWindows.empty() == true)
   {
      return;
   }

   QList<QMdiSubWindow*> windows = subWindowList();
   for (int i = 0; i < windows.count(); ++i)
   {
      QMdiSubWindow* pWindow = windows[i];
      if (pWindow != NULL)
      {
         VERIFYNR(disconnect(pWindow, SIGNAL(windowStateChanged(Qt::WindowStates, Qt::WindowStates)), this,
            SLOT(windowStateChanged(Qt::WindowStates, Qt::WindowStates))));
      }
   }

   mTileWindows.clear();
}

void Workspace::windowStateChanged(Qt::WindowStates oldState, Qt::WindowStates newState)
{
   // Disable the custom tiling if a non-tiled window was activated or if any window was maximized,
   // minimized, restored, or made full screen
   QMdiSubWindow* pWindow = qobject_cast<QMdiSubWindow*>(sender());
   if (pWindow != NULL)
   {
      if (std::find(mTileWindows.begin(), mTileWindows.end(), pWindow) != mTileWindows.end())
      {
         // The window is tiled, so remove the active window state change since it does not affect the tiling
         oldState &= ~(Qt::WindowActive);
         newState &= ~(Qt::WindowActive);
      }
   }

   if (oldState != newState)
   {
      disableCustomTiling();
   }
}

void Workspace::refreshCustomView()
{
   if (mTileWindows.empty() == false)
   {
      int numViews = mTileWindows.size();
      int x;
      int y;
      int rowHeight;
      int colWidth;
      int leftOver;

      mUpdatingWorkspace = true;
      switch (mTilingType)
      {
      case TILE_GRID:
         {
            // handle special case for 1 window in list
            if (mTileWindows.size() == 1)
            {
               setSubWindow(mTileWindows.front(), 0, 0, width(), height());
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
               QMdiSubWindow* pSubWindow = NULL;
               if (mMaxFirst)
               {
                  pSubWindow = mTileWindows.at(startAt++);
                  x = 0;
                  evenColStart = oddColWidth;
               }
               else
               {
                  pSubWindow = mTileWindows.at(runTo--);
                  x = halfViews * colWidth;
               }
               setSubWindow(pSubWindow, x, y, oddColWidth, rowHeight);
            }

            rowHeight = height() / 2;
            leftOver = width() - oddColWidth - colWidth * halfViews;

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
            for (int i = startAt; i < startAt + halfViews; ++i)
            {
               if (colPad-- == 0)
               {
                  --colWidth;
               }
               setSubWindow(mTileWindows.at(i), x, y, colWidth, rowHeight);
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
            for (int i = startAt + halfViews; i <= runTo; ++i)
            {
               if (colPad-- == 0)
               {
                  --colWidth;
               }
               setSubWindow(mTileWindows.at(i), x, y, colWidth, rowHeight);
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
         for (int i = 0; i < numViews ; ++i)
         {
            if (leftOver-- == 0)
            {
               --colWidth;
            }
            setSubWindow(mTileWindows.at(i), x, y, colWidth, rowHeight);
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
         for (int i = 0; i < numViews ; ++i)
         {
            if (leftOver-- == 0)
            {
               --rowHeight;
            }
            setSubWindow(mTileWindows.at(i), x, y, colWidth, rowHeight);
            y += rowHeight;
         }
         break;

      default:
         break;
      }

      mUpdatingWorkspace = false;
   }
}

void Workspace::cascadeSubWindows()
{
   disableCustomTiling();
   QMdiArea::cascadeSubWindows();
}

void Workspace::tile(const TilingType eType)
{
   disableCustomTiling();
   mTilingType = eType;

   QList<QMdiSubWindow*> windows = subWindowList();
   int numWindows = windows.count();
   if (numWindows < 2)
   {
      QMdiArea::tileSubWindows();
      return;
   }

   int leftOver(0);
   switch (mTilingType)
   {
   case TILE_GRID:
      QMdiArea::tileSubWindows();
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
         QMdiSubWindow* pSubWindow(NULL);
         for (int i = 0; i < numWindows; ++i)
         {
            pSubWindow = windows.at(i);
            if (pSubWindow != NULL)
            {
               if (leftOver-- == 0)
               {
                  --wWidth;
               }
               setSubWindow(pSubWindow, x, 0, wWidth, height());
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
         QMdiSubWindow* pSubWindow(NULL);
         for (int i = 0; i < numWindows; ++i)
         {
            pSubWindow = windows.at(i);
            if (pSubWindow != NULL)
            {
               if (leftOver-- == 0)
               {
                  --wHeight;
               }
               setSubWindow(pSubWindow, 0, y, width(), wHeight);
               y += wHeight;
            }
         }
      }
      break;
   default:
      break;
   }
}

bool Workspace::tileWindows(const vector<WorkspaceWindow*>& windows, bool maxFirst, const TilingType eType)
{
   disableCustomTiling();
   mTilingType = eType;

   if (windows.empty() == true)
   {
      return false;
   }

   QList<QMdiSubWindow*> workspaceWindows = subWindowList();
   for (vector<WorkspaceWindow*>::const_iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      WorkspaceWindowImp* pWindow = dynamic_cast<WorkspaceWindowImp*>(*iter);
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
      mTileWindows.clear();
      return false;
   }

   mMaxFirst = maxFirst;
   refreshCustomView();

   // Ensure that all tiled windows are on top of any non-tiled windows
   for (vector<QMdiSubWindow*>::reverse_iterator iter = mTileWindows.rbegin(); iter != mTileWindows.rend(); ++iter)
   {
      QMdiSubWindow* pWindow = *iter;
      if (pWindow != NULL)
      {
         // Activate the window to bring it to the top, which must occur before connecting to the
         // windowStateChanged() signal to prevent disabling the custom tiling if a non-tiled window
         // is currently active
         setActiveSubWindow(pWindow);
      }
   }

   // Connect all windows to disable the custom tiling when the display state changes
   for (int i = 0; i < workspaceWindows.count(); ++i)
   {
      QMdiSubWindow* pWindow = workspaceWindows[i];
      if (pWindow != NULL)
      {
         VERIFYNR(connect(pWindow, SIGNAL(windowStateChanged(Qt::WindowStates, Qt::WindowStates)), this,
            SLOT(windowStateChanged(Qt::WindowStates, Qt::WindowStates))));
      }
   }

   return true;
}

void Workspace::setSubWindow(QMdiSubWindow* pSubWindow, int x, int y, int winWidth, int winHeight)
{
   if (pSubWindow == NULL)
   {
      return;
   }

   pSubWindow->showNormal();
   pSubWindow->resize(winWidth, winHeight);
   pSubWindow->move(x, y);
}
