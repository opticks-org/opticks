/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MULTIMOVIE_H
#define MULTIMOVIE_H

#include "AlgorithmShell.h"

class RasterElement;
class RasterLayer;
class SpatialDataWindow;

class MultiMovie : public AlgorithmShell
{
public:
   MultiMovie();

public:
   bool getInputSpecification( PlugInArgList *& );
   bool getOutputSpecification( PlugInArgList *& );
   bool execute( PlugInArgList *, PlugInArgList * );

private:
   RasterElement* mpRaster1;
   RasterElement* mpRaster2;
   RasterElement* mpRaster3;
   SpatialDataWindow* mpWindow1;
   SpatialDataWindow* mpWindow2;
   SpatialDataWindow* mpWindow3;
   RasterLayer* mpLayer1;
   RasterLayer* mpLayer2;
   RasterLayer* mpLayer3;
   static const int mNumRows = 256;
   static const int mNumCols = 256;
   static const int mNumBands = 256;

   bool createRasterElements();
   bool populateRasterElements();
   bool createWindow(RasterElement *pElement, SpatialDataWindow *&pWindow, RasterLayer *&pLayer);
   bool setupAnimations();
};

#endif
