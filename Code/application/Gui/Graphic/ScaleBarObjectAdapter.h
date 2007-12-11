/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SCALEBAROBJECTADAPTER_H
#define SCALEBAROBJECTADAPTER_H

#include "ScaleBarObject.h"
#include "ScaleBarObjectImp.h"

class ScaleBarObjectAdapter : public ScaleBarObject, public ScaleBarObjectImp
{
public:
   ScaleBarObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                         LocationType pixelCoord) :
      ScaleBarObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~ScaleBarObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("ScaleBarObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "ScaleBarObject"))
      {
         return true;
      }

      return ScaleBarObjectImp::isKindOf(className);
   }

   SCALEBAROBJECTADAPTER_METHODS(ScaleBarObjectImp)
};

#endif
