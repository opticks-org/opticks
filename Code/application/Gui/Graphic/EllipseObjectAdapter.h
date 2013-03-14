/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ELLIPSEOBJECTADAPTER_H
#define ELLIPSEOBJECTADAPTER_H

#include "EllipseObject.h"
#include "EllipseObjectImp.h"

class EllipseObjectAdapter : public EllipseObject, public EllipseObjectImp ELLIPSEOBJECTADAPTEREXTENSION_CLASSES
{
public:
   EllipseObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
      EllipseObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~EllipseObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("EllipseObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "EllipseObject"))
      {
         return true;
      }

      return EllipseObjectImp::isKindOf(className);
   }

   ELLIPSEOBJECTADAPTER_METHODS(EllipseObjectImp)

private:
   EllipseObjectAdapter(const EllipseObjectAdapter& rhs);
   EllipseObjectAdapter& operator=(const EllipseObjectAdapter& rhs);
};

#endif
