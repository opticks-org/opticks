/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FRAMELABELOBJECTADAPTER_H
#define FRAMELABELOBJECTADAPTER_H

#include "FrameLabelObject.h"
#include "FrameLabelObjectImp.h"

class FrameLabelObjectAdapter : public FrameLabelObject, public FrameLabelObjectImp
   FRAMELABELOBJECTADAPTEREXTENSION_CLASSES
{
public:
   FrameLabelObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                           LocationType pixelCoord) :
      FrameLabelObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~FrameLabelObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("FrameLabelObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "FrameLabelObject"))
      {
         return true;
      }

      return FrameLabelObjectImp::isKindOf(className);
   }

   FRAMELABELOBJECTADAPTER_METHODS(FrameLabelObjectImp)
};

#endif
