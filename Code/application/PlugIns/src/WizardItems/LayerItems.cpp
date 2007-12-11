/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "LayerItems.h"
#include "StringUtilities.h"
#include "TypesFile.h"

using namespace std;

LayerItems::LayerItems()
{
}

LayerItems::~LayerItems()
{
}

string LayerItems::getModelType(LayerType eType)
{
   string modelType = "";

   if (eType == AOI_LAYER) modelType = "AoiElement";
   else if (eType == CONTOUR_MAP) modelType = "Terrain";
   else if (eType == GCP_LAYER) modelType = "GcpList";
   else if (eType == LAT_LONG) modelType = "RasterElement";
   else if (eType == PSEUDOCOLOR) modelType = "RasterElement";
   else if (eType == RASTER) modelType = "RasterElement";
   else if (eType == THRESHOLD) modelType = "RasterElement";

   return modelType;
}
