/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGEOBJECTIMP_H
#define IMAGEOBJECTIMP_H

#include <QtGui/QImage>

#include "glCommon.h"
#include "RectangleObjectImp.h"

class GraphicLayer;

class ImageObjectImp : public RectangleObjectImp
{
public:
   ~ImageObjectImp();

   // publicly accessible through AnnotationObject interface
   const unsigned int* getObjectImage(int &width, int &height, ColorType &transparent) const;
   const unsigned int* getImage(int &iWidth, int &iHeight, ColorType &transparent) const;
   void draw(double zoomFactor) const;

   virtual bool processMouseMove(LocationType screenCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers);
   virtual bool processMouseRelease(LocationType screenCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void temporaryGlContextChange();

protected:
   ImageObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);

   bool setImageData(const QImage& image, ColorType transparent = ColorType(-1, -1, -1));
   bool setImageData(const unsigned int* pData, int iWidth, int iHeight,
      ColorType transparent = ColorType(-1, -1, -1));

   void updateBoundingBox();

private:
   class ImageData
   {
   public:
      ImageData(const unsigned int* pData, int iWidth, int iHeight, ColorType transparent);
      ~ImageData();

      const unsigned int* getImageData(int &iWidth, int &iHeight, ColorType& transparent) const;
      unsigned int getTexture(int& iWidth, int& iHeight) const;
      void generateTextures();

      void glContextPush();
      void glContextPop();

   private:
      mutable std::auto_ptr<unsigned int> mpData;
      int mWidth;
      int mHeight;
      int mTextureWidth;
      int mTextureHeight;
      int mReferenceCount;
      ColorType mTransparent;
      GLuint mTextureId;
      std::stack<GLuint> mTextureIdStack;
   };

private:
   ImageData* mpImageData;
   mutable bool mNeedUpdate;
};

#define IMAGEOBJECTADAPTEREXTENSION_CLASSES \
   RECTANGLEOBJECTADAPTEREXTENSION_CLASSES

#define IMAGEOBJECTADAPTER_METHODS \
   RECTANGLEOBJECTADAPTER_METHODS

#endif
