/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PixelObjectImp.h"
#include "AppVerify.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "glCommon.h"

using namespace std;

PixelObjectImp::PixelObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                               LocationType pixelCoord) :
   GraphicObjectImp(id, type, pLayer, pixelCoord),
   mDisplayList(0)
{
   addProperty("DrawMode");
}

PixelObjectImp::~PixelObjectImp()
{
   if (mDisplayList != 0)
   {
      glDeleteLists(mDisplayList, 1);   
   }
}

void PixelObjectImp::draw(double zoomFactor) const
{
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   if (pLayer != NULL && pLayer->willDrawAsPixels())
   {
      if (mDisplayList == 0)
      {
         mDisplayList = glGenLists(1);
      }

      if (mDisplayList == 0)
      {
         // there is some problem with the display lists, just draw in immediate mode
         drawPixels(zoomFactor);
      }
      else
      {
         if (mDisplayListDirty)
         {
            glNewList(mDisplayList, GL_COMPILE);
            drawPixels(zoomFactor);
            glEndList();
            mDisplayListDirty = false;
         }
         glCallList(mDisplayList);
      }
   }
   else
   {
      drawVector(zoomFactor);
   }
}

const string& PixelObjectImp::getObjectType() const
{
   static string type("PixelObjectImp");
   return type;
}

bool PixelObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PixelObject"))
   {
      return true;
   }

   return GraphicObjectImp::isKindOf(className);
}
