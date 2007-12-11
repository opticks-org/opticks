/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VIEWOBJECTIMP_H
#define VIEWOBJECTIMP_H

#include "RectangleObjectImp.h"

class TextObjectImp;
class View;
class ViewImp;

class ViewObjectImp : public RectangleObjectImp
{
   Q_OBJECT

public:
   ViewObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~ViewObjectImp();

   void setView(View* pView);
   View* getView() const;

   void draw(double zoomFactor) const;
   bool setProperty(const GraphicProperty* pProperty);

   bool replicateObject(const GraphicObject* pObject);
   bool processMouseRelease(LocationType screenCoord, 
                            Qt::MouseButton button,
                            Qt::MouseButtons buttons,
                            Qt::KeyboardModifiers modifiers);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;
   bool toXml(XMLWriter *pWriter) const;
   bool fromXml(DOMNode *pDocument, unsigned int version);

protected slots:
   void setBackgroundColor(const QColor& clrBackground);
   void updateTextColor();
   void refresh();
   void redraw();

private:
   ViewImp* mpView;
   TextObjectImp *mpInvalidText;
   mutable bool mUpdateBounds;
};

#define VIEWOBJECTADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass)

#endif
