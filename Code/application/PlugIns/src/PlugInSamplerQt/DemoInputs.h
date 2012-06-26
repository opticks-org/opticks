/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DEMOINPUTS_H
#define DEMOINPUTS_H

#include "Node.h"

/**
 *  Implements the data transfer structure between the GUI and the algorithm.
 */
struct DemoInputs
{
   /**
    *  The list of nodes that a colormap is to be made from 
    */
   std::vector<PlugInSamplerQt::Node> mNodes;

   /**
    *  The number of colors to make in the colormap 
    */
   unsigned int mNumberOfCells;
};

#endif
