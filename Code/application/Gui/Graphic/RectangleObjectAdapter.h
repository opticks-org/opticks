/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RECTANGLEOBJECTADAPTER_H
#define RECTANGLEOBJECTADAPTER_H

#include "RectangleObject.h"
#include "RectangleObjectImp.h"

class RectangleObjectAdapter : public RectangleObject, public RectangleObjectImp RECTANGLEOBJECTADAPTEREXTENSION_CLASSES
{
public:
   RectangleObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                          LocationType pixelCoord) :
      RectangleObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~RectangleObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("RectangleObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "RectangleObject"))
      {
         return true;
      }

      return RectangleObjectImp::isKindOf(className);
   }

   RECTANGLEOBJECTADAPTER_METHODS(RectangleObjectImp)
};

#endif
