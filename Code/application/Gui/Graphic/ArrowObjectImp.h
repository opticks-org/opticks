/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ARROWOBJECTIMP_H
#define ARROWOBJECTIMP_H

#include "LineObjectImp.h"
#include "TypesFile.h"

class GraphicLayer;

class ArrowObjectImp : public LineObjectImp
{
public:
   ArrowObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);

   void draw(double zoomFactor) const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;
};

#define ARROWOBJECTADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass)

#endif
