/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLYLINEOBJECTADAPTER_H
#define POLYLINEOBJECTADAPTER_H

#include "PolylineObject.h"
#include "PolylineObjectImp.h"

class PolylineObjectAdapter : public PolylineObject, public PolylineObjectImp POLYLINEOBJECTADAPTEREXTENSION_CLASSES
{
public:
   PolylineObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                         LocationType pixelCoord) :
      PolylineObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~PolylineObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("PolylineObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "PolylineObject"))
      {
         return true;
      }

      return PolylineObjectImp::isKindOf(className);
   }

   POLYLINEOBJECTADAPTER_METHODS(PolylineObjectImp)

private:
   PolylineObjectAdapter(const PolylineObjectAdapter& rhs);
   PolylineObjectAdapter& operator=(const PolylineObjectAdapter& rhs);
};

#endif
