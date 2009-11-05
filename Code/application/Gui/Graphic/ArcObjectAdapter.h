/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ARCOBJECTADAPTER_H
#define ARCOBJECTADAPTER_H

#include "ArcObject.h"
#include "ArcObjectImp.h"

class ArcObjectAdapter : public ArcObject, public ArcObjectImp ARCOBJECTADAPTEREXTENSION_CLASSES
{
public:
   ArcObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
      ArcObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~ArcObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("ArcObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "ArcObject"))
      {
         return true;
      }

      return ArcObjectImp::isKindOf(className);
   }

   ARCOBJECTADAPTER_METHODS(ArcObjectImp)
};

#endif
