/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOMOSAICCHIP_H
#define GEOMOSAICCHIP_H

#include "AlgorithmShell.h"
#include "SpatialDataView.h"

class SpatialDataView;

class GeoMosaicChip  : public AlgorithmShell
{
public:
   GeoMosaicChip();
   virtual ~GeoMosaicChip();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

private:
   SpatialDataView* mpView;
};

#endif
