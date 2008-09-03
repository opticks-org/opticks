/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef GUI_FUNCTORS_H
#define GUI_FUNCTORS_H

#include "TypesFile.h"

#include <list>
#include <vector>
#include <algorithm>

class GetAnnoObjectColors
{
public:
   GetAnnoObjectColors(std::vector<ColorType>&colors) : 
      mColors(colors) {}
   bool operator()(GraphicObject* pObject)
   {
      if (pObject->getGraphicObjectType() == GROUP_OBJECT)
      {
         const std::list<GraphicObject*> &allObjects = static_cast<GraphicGroup*>(pObject)->getObjects();
         std::for_each(allObjects.begin(), allObjects.end(), *this);
      }
      else
      {
         ColorType lineColor = pObject->getLineColor();
         if (lineColor.isValid() == true)
         {
            mColors.push_back(lineColor);
         }

         ColorType fillColor = pObject->getFillColor();
         if (fillColor.isValid() == true)
         {
            mColors.push_back(fillColor);
         }

         ColorType textColor = pObject->getTextColor();
         if (textColor.isValid() == true)
         {
            mColors.push_back(textColor);
         }
      }
      return true;
   }
private:
   std::vector<ColorType> &mColors;
};

#endif
