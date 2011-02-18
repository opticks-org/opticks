/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Undo.h"
#include "View.h"

using namespace std;

///////////////
// UndoGroup //
///////////////

UndoGroup::UndoGroup(View* pView, const string& text) :
   mpView(pView),
   mEndGroupOnDestroy(true)
{
   if (mpView != NULL)
   {
      mEndGroupOnDestroy = !(mpView->inUndoGroup());
      mpView->startUndoGroup(text);
   }
}

UndoGroup::~UndoGroup()
{
   if ((mpView != NULL) && (mEndGroupOnDestroy == true))
   {
      mpView->endUndoGroup();
   }
}

//////////////
// UndoLock //
//////////////

UndoLock::UndoLock(View* pView) :
   mpView(pView),
   mUnlockOnDestroy(true)
{
   if (mpView != NULL)
   {
      mUnlockOnDestroy = !(mpView->isUndoBlocked());
      mpView->blockUndo();
   }
}

UndoLock::~UndoLock()
{
   if ((mpView != NULL) && (mUnlockOnDestroy == true))
   {
      mpView->unblockUndo();
   }
}
