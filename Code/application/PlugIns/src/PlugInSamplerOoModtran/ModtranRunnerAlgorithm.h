/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODTRAN_RUNNER_ALGORITHM_H
#define MODTRAN_RUNNER_ALGORITHM_H

#include "AlgorithmPattern.h"
#include "ModtranRunnerInputs.h"

class RasterElement;
class Progress;

class ModtranRunnerAlgorithm : public AlgorithmPattern
{
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
   ModtranRunnerInputs mInputs;

public:
   ModtranRunnerAlgorithm(RasterElement *pRasterElement, Progress *pProgress, bool interactive);
};

#endif
