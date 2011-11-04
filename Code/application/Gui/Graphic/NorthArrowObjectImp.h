/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NORTHARROWOBJECTIMP_H
#define NORTHARROWOBJECTIMP_H

#include "DirectionalArrowObjectImp.h"

class GraphicLayer;

class NorthArrowObjectImp : public DirectionalArrowObjectImp
{
public:
   NorthArrowObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

protected:
   std::string getFileIndicator() const;
   void orient();

private:
   NorthArrowObjectImp(const NorthArrowObjectImp& rhs);
   NorthArrowObjectImp& operator=(const NorthArrowObjectImp& rhs);
};

#define NORTHARROWOBJECTADAPTEREXTENSION_CLASSES \
   DIRECTIONALARROWOBJECTADAPTEREXTENSION_CLASSES

#define NORTHARROWOBJECTADAPTER_METHODS(impClass) \
   DIRECTIONALARROWOBJECTADAPTER_METHODS(impClass)

#endif
