/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FilledObjectImp.h"

using namespace std;

FilledObjectImp::FilledObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                 LocationType pixelCoord) :
   GraphicObjectImp(id, type, pLayer, pixelCoord)
{
   addProperty("FillStyle");
   addProperty("HatchStyle");
   addProperty("FillColor");
   addProperty("LineOn");
   addProperty("LineWidth");
   addProperty("LineColor");
   addProperty("LineStyle");

   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
   mHandles.push_back(pixelCoord);
}

const string& FilledObjectImp::getObjectType() const
{
   static string type("FilledObjectImp");
   return type;
}

bool FilledObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "FilledObject"))
   {
      return true;
   }

   return GraphicObjectImp::isKindOf(className);
}
