/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RAWIMAGEOBJECTADAPTER_H
#define RAWIMAGEOBJECTADAPTER_H

#include "RawImageObject.h"
#include "RawImageObjectImp.h"

class RawImageObjectAdapter : public RawImageObject, public RawImageObjectImp RAWIMAGEOBJECTADAPTEREXTENSION_CLASSES
{
public:
   RawImageObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                         LocationType pixelCoord) :
      RawImageObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~RawImageObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("RawImageObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "RawImageObject"))
      {
         return true;
      }

      return RawImageObjectImp::isKindOf(className);
   }

   RAWIMAGEOBJECTADAPTER_METHODS(RawImageObjectImp)

private:
   RawImageObjectAdapter(const RawImageObjectAdapter& rhs);
   RawImageObjectAdapter& operator=(const RawImageObjectAdapter& rhs);
};

#endif
