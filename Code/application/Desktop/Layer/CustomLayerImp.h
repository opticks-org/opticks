/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CUSTOMLAYERIMP_H
#define CUSTOMLAYERIMP_H

#include "AttachmentPtr.h"
#include "DrawObject.h"
#include "LayerImp.h"
#include "TypesFile.h"

#include <boost/any.hpp>
#include <string>

#include <QtGui/QCursor>

class CustomLayerImp : public LayerImp
{
   Q_OBJECT

public:
   static bool isKindOfLayer(const std::string& className);
   static void getLayerTypes(std::vector<std::string>& classList);

   CustomLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   virtual ~CustomLayerImp();

   void setDrawObject(DrawObject* pDrawObject);
   const DrawObject* getDrawObject() const;
   DrawObject* getDrawObject();
   CustomLayerImp& operator= (const CustomLayerImp& CustomLayer);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

   LayerType getLayerType() const;

   virtual void draw();
   virtual bool getExtents(double& x1, double& y1, double& x4, double& y4);
   virtual bool getExtents(std::vector<LocationType>& worldCoords);

   virtual bool acceptsMouseEvents() const;
   virtual QCursor getMouseCursor() const;
   virtual const QCursor& getEditMouseCursor() const;

   virtual void setAcceptsMouseEvents(bool accept);
   virtual void setEditMouseCursor(const QCursor& cursor);

   virtual bool processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   virtual bool processMouseMove(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   virtual bool processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   virtual bool processMouseDoubleClick(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

public slots:
   virtual void reset();

private:
   AttachmentPtr<DrawObject> mpDrawObject;
   void drawObjectDeleted(Subject& subject, const std::string& signal, const boost::any& v);
   void drawObjectModified(Subject& subject, const std::string& signal, const boost::any& v);
   bool mMouseEventsSupported;
   QCursor mMouseCursor;
};

#define CUSTOMLAYERADAPTEREXTENSION_CLASSES \
   LAYERADAPTEREXTENSION_CLASSES

#define CUSTOMLAYERADAPTER_METHODS(impClass) \
   LAYERADAPTER_METHODS(impClass) \
   void setDrawObject(DrawObject* pDrawObj) \
   { \
      impClass::setDrawObject(pDrawObj); \
   } \
   const DrawObject* getDrawObject() const \
   { \
      return impClass::getDrawObject(); \
   } \
   DrawObject* getDrawObject() \
   { \
      return impClass::getDrawObject(); \
   } \
   bool acceptsMouseEvents() const \
   { \
      return impClass::acceptsMouseEvents(); \
   } \
   void setAcceptsMouseEvents(bool accept) \
   { \
      impClass::setAcceptsMouseEvents(accept); \
   } \
   void setEditMouseCursor(const QCursor& cursor) \
   { \
      impClass::setEditMouseCursor(cursor); \
   } \
   const QCursor& getEditMouseCursor() const \
   { \
      return impClass::getEditMouseCursor(); \
   }

#endif
