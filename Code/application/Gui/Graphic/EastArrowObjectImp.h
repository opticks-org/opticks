/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EASTARROWOBJECTIMP_H
#define EASTARROWOBJECTIMP_H

#include "DirectionalArrowObjectImp.h"

class GraphicLayer;

class EastArrowObjectImp : public DirectionalArrowObjectImp
{
public:
   EastArrowObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

protected:
   std::string getFileIndicator() const;
   void orient();

private:
   EastArrowObjectImp(const EastArrowObjectImp& rhs);
   EastArrowObjectImp& operator=(const EastArrowObjectImp& rhs);
};

#define EASTARROWOBJECTADAPTEREXTENSION_CLASSES \
   DIRECTIONALARROWOBJECTADAPTEREXTENSION_CLASSES

#define EASTARROWOBJECTADAPTER_METHODS(impClass) \
   DIRECTIONALARROWOBJECTADAPTER_METHODS(impClass)

#endif
