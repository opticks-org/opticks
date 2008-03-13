/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TRIANGLEOBJECTIMP_H
#define TRIANGLEOBJECTIMP_H

#include "FilledObjectImp.h"
#include "TypesFile.h"

class GraphicLayer;

class TriangleObjectImp : public FilledObjectImp
{
public:
   TriangleObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);

   bool setProperty(const GraphicProperty* pProperty);
   void draw(double zoomFactor) const;
   void moveHandle(int handle, LocationType pixel, bool bMaintainAspect = false);
   void updateHandles();
   bool hit(LocationType pixelCoord) const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;
};

#define TRIANGLEOBJECTADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass)

#endif
