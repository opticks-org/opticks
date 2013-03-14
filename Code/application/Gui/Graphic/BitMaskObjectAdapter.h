/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BITMASKOBJECTADAPTER_H
#define BITMASKOBJECTADAPTER_H

#include "BitMaskObject.h"
#include "BitMaskObjectImp.h"

class BitMaskObjectAdapter : public BitMaskObject, public BitMaskObjectImp BITMASKOBJECTADAPTEREXTENSION_CLASSES
{
public:
   BitMaskObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
      BitMaskObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~BitMaskObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("BitMaskObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "BitMaskObject"))
      {
         return true;
      }

      return BitMaskObjectImp::isKindOf(className);
   }

   BITMASKOBJECTADAPTER_METHODS(BitMaskObjectImp)

private:
   BitMaskObjectAdapter(const BitMaskObjectAdapter& rhs);
   BitMaskObjectAdapter& operator=(const BitMaskObjectAdapter& rhs);
};

#endif
