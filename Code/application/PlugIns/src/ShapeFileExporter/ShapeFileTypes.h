/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SHAPEFILETYPES_H
#define SHAPEFILETYPES_H

#include "EnumWrapper.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

namespace ShapefileTypes
{
   enum ShapeTypeEnum {POINT_SHAPE, POLYLINE_SHAPE, POLYGON_SHAPE, MULTIPOINT_SHAPE};

   /**
    * @EnumWrapper ::ShapeTypeEnum.
    */
   typedef EnumWrapper<ShapeTypeEnum> ShapeType;

   int getIndex(ShapeType type);
}

namespace StringUtilities
{
   template<>
   std::string toDisplayString(const ShapefileTypes::ShapeType& value, bool* pError);

   template<>
   std::string toXmlString(const ShapefileTypes::ShapeType& value, bool* pError);

   template<>
   ShapefileTypes::ShapeType fromDisplayString<ShapefileTypes::ShapeType>(std::string valueText, bool* pError);

   template<>
   ShapefileTypes::ShapeType fromXmlString<ShapefileTypes::ShapeType>(std::string valueText, bool* pError);

   template<>
   std::string toDisplayString(const std::vector<ShapefileTypes::ShapeType>& value, bool* pError);

   template<>
   std::string toXmlString(const std::vector<ShapefileTypes::ShapeType>& value, bool* pError);

   template<>
   std::vector<ShapefileTypes::ShapeType> fromDisplayString<std::vector<ShapefileTypes::ShapeType> >(
      std::string valueText, bool* pError);

   template<>
   std::vector<ShapefileTypes::ShapeType> fromXmlString<std::vector<ShapefileTypes::ShapeType> >(
      std::string valueText, bool* pError);
}
#endif   // SHAPEFILETYPES_H
