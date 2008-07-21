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

enum ShapeTypeEnum { POINT_SHAPE, POLYLINE_SHAPE, POLYGON_SHAPE, MULTIPOINT_SHAPE, UNKNOWN_SHAPE };

/**
 * @EnumWrapper ::ShapeTypeEnum.
 */
typedef EnumWrapper<ShapeTypeEnum> ShapeType;


#endif   // SHAPEFILETYPES_H
