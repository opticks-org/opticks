/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ShapeFileTypes.h"
#include "StringUtilitiesMacros.h"

namespace StringUtilities
{
   BEGIN_ENUM_MAPPING_ALIAS(ShapefileTypes::ShapeType, ShapefileTypesShapeType)
      ADD_ENUM_MAPPING(ShapefileTypes::POINT_SHAPE, "Point", "point")
      ADD_ENUM_MAPPING(ShapefileTypes::POLYLINE_SHAPE, "Polyline", "polyline")
      ADD_ENUM_MAPPING(ShapefileTypes::POLYGON_SHAPE, "Polygon", "polygon")
      ADD_ENUM_MAPPING(ShapefileTypes::MULTIPOINT_SHAPE, "Multi-Point", "multi_point")
   END_ENUM_MAPPING()
}

namespace ShapefileTypes
{
   int getIndex(ShapeType type)
   {
      std::vector<ShapeType> shapes = StringUtilities::getAllEnumValues<ShapeType>();
      if (shapes.empty())
      {
         return -1;
      }
      int numShapes = static_cast<int>(shapes.size());
      for (int index = 0; index < numShapes; ++index)
      {
         if (type == shapes[index])
         {
            return index;
         }
      }
      return -1;
   }

}
