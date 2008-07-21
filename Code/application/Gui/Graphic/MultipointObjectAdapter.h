/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MULTIPOINTOBJECTADAPTER_H
#define MULTIPOINTOBJECTADAPTER_H

#include "MultipointObject.h"
#include "MultipointObjectImp.h"

class MultipointObjectAdapter : public MultipointObject, public MultipointObjectImp MULTIPOINTOBJECTADAPTEREXTENSION_CLASSES
{
public:
   MultipointObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                           LocationType pixelCoord) :
      MultipointObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~MultipointObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("MultipointObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "MultipointObject"))
      {
         return true;
      }

      return MultipointObjectImp::isKindOf(className);
   }

   MULTIPOINTOBJECTADAPTER_METHODS(MultipointObjectImp)
};

#endif
