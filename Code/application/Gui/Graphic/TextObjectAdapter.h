/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TEXTOBJECTADAPTER_H
#define TEXTOBJECTADAPTER_H

#include "TextObject.h"
#include "TextObjectImp.h"

class TextObjectAdapter : public TextObject, public TextObjectImp
{
public:
   TextObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
      TextObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~TextObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("TextObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "TextObject"))
      {
         return true;
      }

      return TextObjectImp::isKindOf(className);
   }

   TEXTOBJECTADAPTER_METHODS(TextObjectImp)
};

#endif
