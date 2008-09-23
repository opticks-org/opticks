/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef __DYNMCOBJADAPTER_H
#define __DYNMCOBJADAPTER_H

#include "DynamicObject.h"
#include "DynamicObjectImp.h"

#include <string>
#include <vector>

class DynamicObjectAdapter : public DynamicObject, public DynamicObjectImp DYNAMICOBJECTADAPTEREXTENSION_CLASSES
{
public:
   DynamicObjectAdapter()
   {
   }

   virtual ~DynamicObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   virtual const std::string& getObjectType() const
   {
      static std::string type("DynamicObjectAdapter");
      return type;
   }

   virtual bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "DynamicObject"))
      {
         return true;
      }

      return DynamicObjectImp::isKindOf(className);
   }

   DYNAMICOBJECTADAPTER_METHODS(DynamicObjectImp)
};

#endif   // __DYNMCOBJADAPTER_H
