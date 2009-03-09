/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TRAILOBJECTADAPTER_H
#define TRAILOBJECTADAPTER_H

#include "TrailObject.h"
#include "TrailObjectImp.h"

class TrailObjectAdapter : public TrailObject, public TrailObjectImp TRAILOBJECTADAPTEREXTENSION_CLASSES
{
public:
   TrailObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
      TrailObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~TrailObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("TrailObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "TrailObject"))
      {
         return true;
      }

      return TrailObjectImp::isKindOf(className);
   }

   TRAILOBJECTADAPTER_METHODS(TrailObjectImp)
};

#endif
