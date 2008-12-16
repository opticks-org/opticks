/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MEASUREMENTOBJECTADAPTER_H
#define MEASUREMENTOBJECTADAPTER_H

#include "GraphicObject.h"
#include "MeasurementObjectImp.h"

class MeasurementObjectAdapter : public GraphicObject, public MeasurementObjectImp
   MEASUREMENTOBJECTADAPTEREXTENSION_CLASSES
{
public:
   MeasurementObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                            LocationType pixelCoord) :
      MeasurementObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~MeasurementObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("MeasurementObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "MeasurementObject"))
      {
         return true;
      }

      return MeasurementObjectImp::isKindOf(className);
   }

   MEASUREMENTOBJECTADAPTER_METHODS(MeasurementObjectImp)
};

#endif
