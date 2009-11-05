/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "TiePointListUndo.h"

using namespace std;

//////////////////
// SetTiePoints //
//////////////////

SetTiePoints::SetTiePoints(TiePointList* pTiePointList, const vector<TiePoint>& oldVertices,
                           const vector<TiePoint>& newVertices) :
   UndoAction(pTiePointList),
   mOldVertices(oldVertices),
   mNewVertices(newVertices)
{
   setText("Set Tie Points");
}

void SetTiePoints::executeUndo()
{
   TiePointList* pList = static_cast<TiePointList*>(getSessionItem());
   if (pList != NULL)
   {
      vector<TiePoint> vertices = mOldVertices;
      pList->adoptTiePoints(vertices);
   }
}

void SetTiePoints::executeRedo()
{
   TiePointList* pList = static_cast<TiePointList*>(getSessionItem());
   if (pList != NULL)
   {
      vector<TiePoint> vertices = mNewVertices;
      pList->adoptTiePoints(vertices);
   }
}
