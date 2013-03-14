/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GCPLISTUNDO_H
#define GCPLISTUNDO_H

#include "GcpList.h"
#include "UndoAction.h"

#include <list>

class SetGcpPoints : public UndoAction
{
public:
   SetGcpPoints(GcpList* pGcpList, const std::list<GcpPoint>& oldPoints, const std::list<GcpPoint>& newPoints);

   void executeUndo();
   void executeRedo();

private:
   SetGcpPoints(const SetGcpPoints& rhs);
   SetGcpPoints& operator=(const SetGcpPoints& rhs);
   std::list<GcpPoint> mOldPoints;
   std::list<GcpPoint> mNewPoints;
};


class AddGcpPoint : public UndoAction
{
public:
   AddGcpPoint(GcpList* pGcpList, GcpPoint point);

   void executeUndo();
   void executeRedo();

private:
   AddGcpPoint(const AddGcpPoint& rhs);
   AddGcpPoint& operator=(const AddGcpPoint& rhs);
   GcpPoint mPoint;
};


class RemoveGcpPoint : public UndoAction
{
public:
   RemoveGcpPoint(GcpList* pGcpList, GcpPoint point);

   void executeUndo();
   void executeRedo();

private:
   RemoveGcpPoint(const RemoveGcpPoint& rhs);
   RemoveGcpPoint& operator=(const RemoveGcpPoint& rhs);
   GcpPoint mPoint;
};

#endif
