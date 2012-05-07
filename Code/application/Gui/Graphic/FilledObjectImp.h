/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILLEDOBJECTIMP_H
#define FILLEDOBJECTIMP_H

#include "GraphicObjectImp.h"
#include "TypesFile.h"

class GraphicLayer;

class FilledObjectImp : public GraphicObjectImp
{
public:
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

protected:
   FilledObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);

private:
   FilledObjectImp(const FilledObjectImp& rhs);
   FilledObjectImp& operator=(const FilledObjectImp& rhs);
};

#define FILLEDOBJECTADAPTEREXTENSION_CLASSES \
   GRAPHICOBJECTADAPTEREXTENSION_CLASSES

#define FILLEDOBJECTADAPTER_METHODS \
   GRAPHICOBJECTADAPTER_METHODS

#endif
