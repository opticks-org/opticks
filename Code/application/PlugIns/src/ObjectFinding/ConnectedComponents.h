/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONNECTEDCOMPONENTS_H__
#define CONNECTEDCOMPONENTS_H__

#include "AlgorithmShell.h"
#include "ProgressTracker.h"
#include <vector>

class BitMask;
class RasterElement;
class SpatialDataView;

class ConnectedComponents : public AlgorithmShell
{
public:
   ConnectedComponents();
   virtual ~ConnectedComponents();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

private:
   bool createPseudocolor(unsigned short maxLabel) const;

   mutable ProgressTracker mProgress;
   SpatialDataView* mpView;
   RasterElement* mpLabels;
   int mXOffset;
   int mYOffset;
};

#endif