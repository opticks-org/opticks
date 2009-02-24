/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LATLONINSERTOBJECTADAPTER_H
#define LATLONINSERTOBJECTADAPTER_H

#include "LatLonInsertObject.h"
#include "LatLonInsertObjectImp.h"

class LatLonInsertObjectAdapter : public LatLonInsertObject, public LatLonInsertObjectImp
   LATLONINSERTOBJECTADAPTEREXTENSION_CLASSES
{
public:
   LatLonInsertObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                             LocationType pixelCoord) :
      LatLonInsertObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~LatLonInsertObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("LatLonInsertObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "LatLonInsertObject"))
      {
         return true;
      }

      return LatLonInsertObjectImp::isKindOf(className);
   }

   LATLONINSERTOBJECTADAPTER_METHODS(LatLonInsertObjectImp)
};

#endif
