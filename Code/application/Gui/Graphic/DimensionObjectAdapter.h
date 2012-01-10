/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DIMENSIONOBJECTADAPTER_H
#define DIMENSIONOBJECTADAPTER_H

#include "DimensionObject.h"
#include "DimensionObjectImp.h"

class DimensionObjectAdapter : public DimensionObject, public DimensionObjectImp DIMENSIONOBJECTADAPTEREXTENSION_CLASSES
{
public:
   DimensionObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                          LocationType pixelCoord) :
      DimensionObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~DimensionObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("DimensionObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "DimensionObject"))
      {
         return true;
      }

      return DimensionObjectImp::isKindOf(className);
   }

   DIMENSIONOBJECTADAPTER_METHODS(DimensionObjectImp)

private:
   DimensionObjectAdapter(const DimensionObjectAdapter& rhs);
   DimensionObjectAdapter& operator=(const DimensionObjectAdapter& rhs);
};

#endif
