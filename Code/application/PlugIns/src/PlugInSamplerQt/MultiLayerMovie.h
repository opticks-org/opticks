/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MULTILAYERMOVIE_H
#define MULTILAYERMOVIE_H

#include "AlgorithmShell.h"

class RasterLayer;
class SpatialDataWindow;

class MultiLayerMovie : public AlgorithmShell
{
public:
   MultiLayerMovie();

public:
   bool getInputSpecification( PlugInArgList *& );
   bool getOutputSpecification( PlugInArgList *& );
   bool execute( PlugInArgList *, PlugInArgList * );

private:
   RasterElement* mpRaster1;
   RasterElement* mpRaster2;
   RasterElement* mpRaster3;
   SpatialDataWindow* mpWindow;
   RasterLayer* mpLayer1;
   RasterLayer* mpLayer2;
   RasterLayer* mpLayer3;
   static const int mNumRows = 256;
   static const int mNumCols = 256;
   static const int mNumBands = 256;

   bool createRasterElements();
   bool populateRasterElements();
   bool createWindow();
   bool createLayer(SpatialDataView* pView, RasterElement* pElement, RasterLayer*& pLayer,
      double xScale = 1.0, double xOffset = 0.0, double yScale = 1.0, double yOffset = 0.0);
   bool setupAnimations();
};

#endif
