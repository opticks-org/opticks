/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CGMOBJECTADAPTER_H
#define CGMOBJECTADAPTER_H

#include "CgmObject.h"
#include "CgmObjectImp.h"

class CgmObjectAdapter : public CgmObject, public CgmObjectImp CGMOBJECTADAPTEREXTENSION_CLASSES
{
public:
   CgmObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
      CgmObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~CgmObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("CgmObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "CgmObject"))
      {
         return true;
      }

      return CgmObjectImp::isKindOf(className);
   }

   CGMOBJECTADAPTER_METHODS(CgmObjectImp)
};

#endif
