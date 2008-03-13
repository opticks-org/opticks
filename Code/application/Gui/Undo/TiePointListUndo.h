/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIEPOINT_UNDO_H
#define TIEPOINT_UNDO_H

#include "TiePointList.h"
#include "UndoAction.h"

#include <vector>

class SetTiePoints : public UndoAction
{
public:
   SetTiePoints(TiePointList* pTiePointList, const std::vector<TiePoint>& oldVertices,
      const std::vector<TiePoint>& newVertices);

   void executeUndo();
   void executeRedo();

private:
   std::vector<TiePoint> mOldVertices;
   std::vector<TiePoint> mNewVertices;
};

#endif
