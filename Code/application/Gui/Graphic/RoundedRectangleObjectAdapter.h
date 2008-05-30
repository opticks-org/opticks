/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ROUNDEDRECTANGLEOBJECTADAPTER_H
#define ROUNDEDRECTANGLEOBJECTADAPTER_H

#include "RoundedRectangleObject.h"
#include "RectangleObjectImp.h"

class RoundedRectangleObjectAdapter : public RoundedRectangleObject, public RoundedRectangleObjectImp ROUNDEDRECTANGLEOBJECTADAPTEREXTENSION_CLASSES
{
public:
   RoundedRectangleObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                 LocationType pixelCoord) :
      RoundedRectangleObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~RoundedRectangleObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("RoundedRectangleObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "RoundedRectangleObject"))
      {
         return true;
      }

      return RoundedRectangleObjectImp::isKindOf(className);
   }

   ROUNDEDRECTANGLEOBJECTADAPTER_METHODS(RoundedRectangleObjectImp)
};

#endif
