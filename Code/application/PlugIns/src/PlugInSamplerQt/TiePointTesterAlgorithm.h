/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIEPOINT_TESTER_ALGORITHM_H
#define TIEPOINT_TESTER_ALGORITHM_H

#include <math.h>

#include "AlgorithmPattern.h"
#include "TiePointList.h"

#include "TiePointTesterInputs.h"
#include "Node.h"

#include <vector>

class Progress;
class RasterElement;

class TiePointTesterAlgorithm : public AlgorithmPattern
{
   int findCubes(RasterElement* pCubes[2]);
   void computeTiePoints(size_t xSize, size_t ySize, std::vector<TiePoint>& points);
   void addLayer(RasterElement *pCube, TiePointList *pList, bool isMission);

   // obligations from base class
   bool initialize(void *pAlgorithmData);
   bool preprocess();
   bool processAll();
   bool postprocess();
   bool canAbort() const;
   bool doAbort();

   /**
    *    The values that the algorithm will run on 
    */
   TiePointTesterInputs mInputs;

public:
   TiePointTesterAlgorithm(RasterElement &rasterElement, Progress *pProgress, bool interactive);
};

#endif
