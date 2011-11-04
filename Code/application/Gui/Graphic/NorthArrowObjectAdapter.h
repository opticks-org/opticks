/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NORTHARROWOBJECTADAPTER_H
#define NORTHARROWOBJECTADAPTER_H

#include "NorthArrowObject.h"
#include "NorthArrowObjectImp.h"

class NorthArrowObjectAdapter : public NorthArrowObject, public NorthArrowObjectImp
   NORTHARROWOBJECTADAPTEREXTENSION_CLASSES
{
public:
   NorthArrowObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                           LocationType pixelCoord) :
      NorthArrowObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~NorthArrowObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("NorthArrowObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "NorthArrowObject"))
      {
         return true;
      }

      return NorthArrowObjectImp::isKindOf(className);
   }

   NORTHARROWOBJECTADAPTER_METHODS(NorthArrowObjectImp)

private:
   NorthArrowObjectAdapter(const NorthArrowObjectAdapter& rhs);
   NorthArrowObjectAdapter& operator=(const NorthArrowObjectAdapter& rhs);
};

#endif
