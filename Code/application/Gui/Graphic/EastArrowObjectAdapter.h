/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EASTARROWOBJECTADAPTER_H
#define EASTARROWOBJECTADAPTER_H

#include "EastArrowObject.h"
#include "EastArrowObjectImp.h"

class EastArrowObjectAdapter : public EastArrowObject, public EastArrowObjectImp EASTARROWOBJECTADAPTEREXTENSION_CLASSES
{
public:
   EastArrowObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                          LocationType pixelCoord) :
      EastArrowObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~EastArrowObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("EastArrowObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "EastArrowObject"))
      {
         return true;
      }

      return EastArrowObjectImp::isKindOf(className);
   }

   EASTARROWOBJECTADAPTER_METHODS(EastArrowObjectImp)
};

#endif
