/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TEXTOBJECTIMP_H
#define TEXTOBJECTIMP_H

#include <QtGui/QBitmap>

#include "glCommon.h"
#include "RectangleObjectImp.h"
#include "GraphicProperty.h"
#include "TypesFile.h"

class GraphicLayer;

class TextObjectImp : public RectangleObjectImp
{
   Q_OBJECT

public:
   TextObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~TextObjectImp();

   void draw(double zoomFactor) const;

   bool setProperty(const GraphicProperty* pProperty);
   bool hit(LocationType pixelCoord) const;
   bool replicateObject(const GraphicObject* pObject);

   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   bool processMousePress(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseMove(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseRelease(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   void temporaryGlContextChange();

   bool edit();
   void moveHandle(int handle, LocationType point, bool bMaintainAspect);

public slots:
   void updateTexture();
   void updateBoundingBox();

protected:
   QFont getScaledFont(double minSize = -1.0, double maxSize = 256.0);
   void drawTexture() const;

private:
   static QBitmap* sBitmap;
   GLuint mTextureId;
   int mTextureWidth, mTextureHeight;
   int mDataWidth, mDataHeight;
   bool mUpdateTexture;
   bool mUpdateBoundingBox;
   std::stack<GLuint> mTextureIdStack;
};

#define TEXTOBJECTADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass)

#endif
