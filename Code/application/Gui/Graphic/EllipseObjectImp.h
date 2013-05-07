/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ELLIPSEOBJECTIMP_H
#define ELLIPSEOBJECTIMP_H

#include "FilledObjectImp.h"
#include "TypesFile.h"

class GraphicLayer;

class EllipseObjectImp : public FilledObjectImp
{
public:
   EllipseObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);

   void draw(double zoomFactor) const;
   bool hit(LocationType pixelCoord) const;
   bool getExtents(std::vector<LocationType>& dataCoords) const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   EllipseObjectImp(const EllipseObjectImp& rhs);
   EllipseObjectImp& operator=(const EllipseObjectImp& rhs);
};

#define ELLIPSEOBJECTADAPTEREXTENSION_CLASSES \
   FILLEDOBJECTADAPTEREXTENSION_CLASSES

#define ELLIPSEOBJECTADAPTER_METHODS(impClass) \
   FILLEDOBJECTADAPTER_METHODS(impClass)

#endif
