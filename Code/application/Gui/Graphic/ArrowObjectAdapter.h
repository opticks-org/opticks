/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ARROWOBJECTADAPTER_H
#define ARROWOBJECTADAPTER_H

#include "ArrowObject.h"
#include "ArrowObjectImp.h"

class ArrowObjectAdapter : public ArrowObject, public ArrowObjectImp
{
public:
   ArrowObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
      ArrowObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~ArrowObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("ArrowObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "ArrowObject"))
      {
         return true;
      }

      return ArrowObjectImp::isKindOf(className);
   }

   ARROWOBJECTADAPTER_METHODS(ArrowObjectImp)
};

#endif
