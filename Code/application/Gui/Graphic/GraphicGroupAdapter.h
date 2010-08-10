/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICGROUPADAPTER_H
#define GRAPHICGROUPADAPTER_H

#include "GraphicGroup.h"
#include "GraphicGroupImp.h"

class GraphicGroupAdapter : public GraphicGroup, public GraphicGroupImp GRAPHICGROUPADAPTEREXTENSION_CLASSES
{
public:
   GraphicGroupAdapter(const std::string& id, GraphicObjectType type, GraphicLayer *pLayer, LocationType pixelCoord) :
      GraphicGroupImp(id, type, pLayer, pixelCoord)
   {
   }

   ~GraphicGroupAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("GraphicGroupAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "GraphicGroup"))
      {
         return true;
      }

      return GraphicGroupImp::isKindOf(className);
   }

   GRAPHICGROUPADAPTER_METHODS(GraphicGroupImp)
};

#endif
