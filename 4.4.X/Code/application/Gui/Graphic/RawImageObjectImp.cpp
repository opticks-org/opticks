/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "RawImageObjectImp.h"

using namespace std;

RawImageObjectImp::RawImageObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                     LocationType pixelCoord) :
   ImageObjectImp(id, type, pLayer, pixelCoord)
{
}

bool RawImageObjectImp::setImage(const QImage& image, ColorType transparent)
{
   return setImageData(image, transparent);
}

bool RawImageObjectImp::setObjectImage(const unsigned int* pData, int iWidth, int iHeight, ColorType transparent)
{
   return setImage(pData, iWidth, iHeight, transparent);
}

bool RawImageObjectImp::setImage(const unsigned int* pData, int iWidth, int iHeight, ColorType transparent)
{
   return setImageData(pData, iWidth, iHeight, transparent);
}

bool RawImageObjectImp::toXml(XMLWriter* pXml) const
{
   return false;
}

bool RawImageObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   return false;
}

const string& RawImageObjectImp::getObjectType() const
{
   static string type("RawImageObjectImp");
   return type;
}

bool RawImageObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RawImageObject"))
   {
      return true;
   }

   return ImageObjectImp::isKindOf(className);
}
