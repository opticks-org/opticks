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

#include "AttachmentPtr.h"
#include "DynamicObject.h"
#include "glCommon.h"
#include "RectangleObjectImp.h"
#include "GraphicProperty.h"
#include "TypesFile.h"
#include <map>

class Classification;
class GraphicLayer;
class QGLContext;

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

   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);
   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& className) const;

   bool processMousePress(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseMove(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseRelease(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   void setTextEditable(bool editable);
   bool isTextEditable() const;
   bool edit();
   void moveHandle(int handle, LocationType point, bool bMaintainAspect);

   virtual std::string getSubstitutedText();

public slots:
   void updateTexture();
   void updateBoundingBox();

protected:
   void invalidateTexture(Subject& subject, const std::string& signal, const boost::any& v);
   QFont getScaledFont(double minSize = -1.0, double maxSize = 256.0);

private:
   static QBitmap* sBitmap;
   QGLContext* mpDrawContext;
   int mTextureWidth;
   int mTextureHeight;
   int mDataWidth;
   int mDataHeight;
   bool mUpdateTexture;
   bool mUpdateBoundingBox;
   std::map<DynamicObject*, AttachmentPtr<DynamicObject>* > mMetadataObjects;
   std::map<Classification*, AttachmentPtr<Classification>* > mClassificationObjects;
   GlTextureResource mTextureResource;
   GlTextureResource mTempTextureResource;
   bool mTextEditable;

   TextObjectImp& operator= (const TextObjectImp& textObject);
   TextObjectImp(const TextObjectImp& textObject);
};

#define TEXTOBJECTADAPTEREXTENSION_CLASSES \
   RECTANGLEOBJECTADAPTEREXTENSION_CLASSES

#define TEXTOBJECTADAPTER_METHODS(impClass) \
   RECTANGLEOBJECTADAPTER_METHODS(impClass)

#endif
