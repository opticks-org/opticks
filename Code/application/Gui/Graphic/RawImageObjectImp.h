/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RAWIMAGEOBJECTIMP_H
#define RAWIMAGEOBJECTIMP_H

#include "ImageObjectImp.h"

class GraphicLayer;

class RawImageObjectImp : public ImageObjectImp
{
public:
   RawImageObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~RawImageObjectImp() {}

   virtual bool setObjectImage(const unsigned int* pData, int iWidth, int iHeight, ColorType transparent);
   bool setImage(const QImage& image, ColorType transparent = ColorType(-1, -1, -1));
   bool setImage(const unsigned int* pData, int iWidth, int iHeight, ColorType transparent = ColorType(-1, -1, -1));

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;
};

#define RAWIMAGEOBJECTADAPTEREXTENSION_CLASSES \
   IMAGEOBJECTADAPTEREXTENSION_CLASSES

#define RAWIMAGEOBJECTADAPTER_METHODS(impClass) \
   IMAGEOBJECTADAPTER_METHODS(impClass)


#endif
