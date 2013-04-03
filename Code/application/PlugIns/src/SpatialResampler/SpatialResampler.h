/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SPATIALRESAMPLER_H
#define SPATIALRESAMPLER_H

#include "AlgorithmShell.h"

class SpatialResampler : public AlgorithmShell
{
public:
   SpatialResampler();
   virtual ~SpatialResampler();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   virtual bool setInteractive();
};

#endif