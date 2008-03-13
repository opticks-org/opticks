/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LINEOBJECTADAPTER_H
#define LINEOBJECTADAPTER_H

#include "LineObject.h"
#include "LineObjectImp.h"

class LineObjectAdapter : public LineObject, public LineObjectImp
{
public:
   LineObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
      LineObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~LineObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("LineObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "LineObject"))
      {
         return true;
      }

      return LineObjectImp::isKindOf(className);
   }

   LINEOBJECTADAPTER_METHODS(LineObjectImp)
};

#endif
