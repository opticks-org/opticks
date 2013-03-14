/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GcpListUndo.h"

using namespace std;

//////////////////
// SetGcpPoints //
//////////////////

SetGcpPoints::SetGcpPoints(GcpList* pGcpList, const list<GcpPoint>& oldPoints, const list<GcpPoint>& newPoints) :
   UndoAction(pGcpList),
   mOldPoints(oldPoints),
   mNewPoints(newPoints)
{
   setText("Set GCPs");
}

void SetGcpPoints::executeUndo()
{
   GcpList* pGcpList = dynamic_cast<GcpList*>(getSessionItem());
   if (pGcpList != NULL)
   {
      if (pGcpList->getSelectedPoints() != mOldPoints)
      {
         pGcpList->clearPoints();
         pGcpList->addPoints(mOldPoints);
      }
   }
}

void SetGcpPoints::executeRedo()
{
   GcpList* pGcpList = dynamic_cast<GcpList*>(getSessionItem());
   if (pGcpList != NULL)
   {
      if (pGcpList->getSelectedPoints() != mNewPoints)
      {
         pGcpList->clearPoints();
         pGcpList->addPoints(mNewPoints);
      }
   }
}

/////////////////
// AddGcpPoint //
/////////////////

AddGcpPoint::AddGcpPoint(GcpList* pGcpList, GcpPoint point) :
   UndoAction(pGcpList),
   mPoint(point)
{
   setText("Add GCP");
}

void AddGcpPoint::executeUndo()
{
   GcpList* pGcpList = dynamic_cast<GcpList*>(getSessionItem());
   if (pGcpList != NULL)
   {
      pGcpList->removePoint(mPoint);
   }
}

void AddGcpPoint::executeRedo()
{
   GcpList* pGcpList = dynamic_cast<GcpList*>(getSessionItem());
   if (pGcpList != NULL)
   {
      pGcpList->addPoint(mPoint);
   }
}

////////////////////
// RemoveGcpPoint //
////////////////////

RemoveGcpPoint::RemoveGcpPoint(GcpList* pGcpList, GcpPoint point) :
   UndoAction(pGcpList),
   mPoint(point)
{
   setText("Remove GCP");
}

void RemoveGcpPoint::executeUndo()
{
   GcpList* pGcpList = dynamic_cast<GcpList*>(getSessionItem());
   if (pGcpList != NULL)
   {
      pGcpList->addPoint(mPoint);
   }
}

void RemoveGcpPoint::executeRedo()
{
   GcpList* pGcpList = dynamic_cast<GcpList*>(getSessionItem());
   if (pGcpList != NULL)
   {
      pGcpList->removePoint(mPoint);
   }
}
