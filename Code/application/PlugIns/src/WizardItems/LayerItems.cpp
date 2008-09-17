/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LayerItems.h"
#include "TypeConverter.h"

class AnnotationElement;
class AoiElement;
class GcpList;
class RasterElement;

using namespace std;

LayerItems::LayerItems()
{
}

LayerItems::~LayerItems()
{
}

string LayerItems::getModelType(LayerType eType)
{
   if (eType == AOI_LAYER)
   {
      return TypeConverter::toString<AoiElement>();
   }

   if (eType == ANNOTATION)
   {
      return TypeConverter::toString<AnnotationElement>();
   }

   if (eType == GCP_LAYER)
   {
      return TypeConverter::toString<GcpList>();
   }

   if (eType == LAT_LONG || eType == PSEUDOCOLOR ||
      eType == RASTER || eType == THRESHOLD)
   {
      return TypeConverter::toString<RasterElement>();
   }

   return string();
}
