/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VIEWOBJECTADAPTER_H
#define VIEWOBJECTADAPTER_H

#include "ViewObject.h"
#include "ViewObjectImp.h"

class ViewObjectAdapter : public ViewObject, public ViewObjectImp VIEWOBJECTADAPTEREXTENSION_CLASSES
{
public:
   ViewObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
      ViewObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~ViewObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("ViewObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "ViewObject"))
      {
         return true;
      }

      return ViewObjectImp::isKindOf(className);
   }

   VIEWOBJECTADAPTER_METHODS(ViewObjectImp)
};

#endif
