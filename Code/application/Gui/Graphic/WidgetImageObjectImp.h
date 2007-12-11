/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIDGETIMAGEOBJECTIMP_H
#define WIDGETIMAGEOBJECTIMP_H

#include <QtGui/QImage>
#include <QtGui/QWidget>

#include "ImageObjectImp.h"

class GraphicLayer;

class WidgetImageObjectImp : public ImageObjectImp
{
public:
   WidgetImageObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~WidgetImageObjectImp() {}

   bool setWidget(QWidget* pWidget);
   bool setWidgetImage(const QImage& image);

   bool processMousePress(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;
};

#define WIDGETIMAGEOBJECTADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass)

#endif
