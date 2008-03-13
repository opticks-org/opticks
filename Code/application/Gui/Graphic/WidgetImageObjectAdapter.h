/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIDGETIMAGEOBJECTADAPTER_H
#define WIDGETIMAGEOBJECTADAPTER_H

#include "WidgetImageObject.h"
#include "WidgetImageObjectImp.h"

class WidgetImageObjectAdapter : public WidgetImageObject, public WidgetImageObjectImp
{
public:
   WidgetImageObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                            LocationType pixelCoord) :
      WidgetImageObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~WidgetImageObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("WidgetImageObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "WidgetImageObject"))
      {
         return true;
      }

      return WidgetImageObjectImp::isKindOf(className);
   }

   WIDGETIMAGEOBJECTADAPTER_METHODS(WidgetImageObjectImp)
};

#endif
