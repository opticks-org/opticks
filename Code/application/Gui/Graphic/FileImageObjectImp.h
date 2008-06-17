/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILEIMAGEOBJECTIMP_H
#define FILEIMAGEOBJECTIMP_H

#include "ImageObjectImp.h"

class GraphicLayer;

class FileImageObjectImp : public ImageObjectImp
{
public:
   FileImageObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~FileImageObjectImp() {}

   bool setProperty(const GraphicProperty *pProp);

   virtual bool processMousePress(LocationType screenCoord, 
                               Qt::MouseButton button,
                               Qt::MouseButtons buttons,
                               Qt::KeyboardModifiers modifiers);

   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   bool mLoading;
};

#define FILEIMAGEOBJECTADAPTEREXTENSION_CLASSES \
   IMAGEOBJECTADAPTEREXTENSION_CLASSES

#define FILEIMAGEOBJECTADAPTER_METHODS(impClass) \
   IMAGEOBJECTADAPTER_METHODS(impClass)

#endif
