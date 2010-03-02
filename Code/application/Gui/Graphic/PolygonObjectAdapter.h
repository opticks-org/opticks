/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLYGONOBJECTADAPTER_H
#define POLYGONOBJECTADAPTER_H

#include "PolygonObject.h"
#include "PolygonObjectImp.h"

class PolygonObjectAdapter : public PolygonObject, public PolygonObjectImp POLYGONOBJECTADAPTEREXTENSION_CLASSES
{
public:
   PolygonObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
      PolygonObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~PolygonObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("PolygonObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "PolygonObject"))
      {
         return true;
      }

      return PolygonObjectImp::isKindOf(className);
   }

   POLYGONOBJECTADAPTER_METHODS(PolygonObjectImp)
};

#endif
