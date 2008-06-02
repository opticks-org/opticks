/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ASPAMADAPTER_H__
#define ASPAMADATPER_H__

#include "Aspam.h"
#include "AspamImp.h"

/**
 * Adapter class for the default Aspam model implementation.
 */
class AspamAdapter : public Aspam, public AspamImp ASPAMADAPTEREXTENSION_CLASSES
{
public:
   AspamAdapter() {}
   ~AspamAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("AspamAdapter");
      return type;
   }
   bool isKindOf(const std::string& className) const
   {
      if((className == getObjectType()) || (className == "Aspam"))
      {
         return true;
      }
      return AspamImp::isKindOf(className);
   }

   ASPAMADAPTER_METHODS(AspamImp)
};

#endif

