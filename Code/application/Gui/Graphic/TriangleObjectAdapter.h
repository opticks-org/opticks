/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TRIANGLEOBJECTADAPTER_H
#define TRIANGLEOBJECTADAPTER_H

#include "TriangleObject.h"
#include "TriangleObjectImp.h"

class TriangleObjectAdapter : public TriangleObject, public TriangleObjectImp
{
public:
   TriangleObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                         LocationType pixelCoord) :
      TriangleObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~TriangleObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("TriangleObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "TriangleObject"))
      {
         return true;
      }

      return TriangleObjectImp::isKindOf(className);
   }

   TRIANGLEOBJECTADAPTER_METHODS(TriangleObjectImp)
};

#endif
