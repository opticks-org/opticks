/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DEMO_ALGORITHM_H
#define DEMO_ALGORITHM_H

#include <math.h>

#include "AlgorithmPattern.h"

#include "DemoInputs.h"
#include "Node.h"

#include <vector>

class Progress;
class RasterElement;
class Step;

/**
 * Implements the algorithm that converts the Nodes to a colormap.
 */
class DemoAlgorithm : public AlgorithmPattern
{
   // obligations from base class
   bool initialize(void *pAlgorithmData);
   bool preprocess();
   bool processAll();
   bool postprocess();
   bool canAbort() const;
   bool doAbort();

   // local private methods
   void applyColormap(RasterElement &cube, std::vector<ColorType> &colormap) const;
   void populateColormapFromNodes(std::vector<ColorType> & colormap) const;

   /**
    *    The values that the algorithm will run on 
    */
   DemoInputs mInputs;

   // The message log step
   Step *mpStep;

public:
   DemoAlgorithm(RasterElement &rasterElement, Progress *pProgress, bool interactive);
};

#endif
