/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CLASSIFICATIONLAYERIMP_H
#define CLASSIFICATIONLAYERIMP_H

#include <QtGui/QColor>
#include <QtGui/QFont>

#include "AnnotationLayerImp.h"
#include "FontImp.h"

class GraphicProperty;
class TextObject;
class TextObjectImp;

class ClassificationLayerImp : public AnnotationLayerImp
{
   Q_OBJECT

public:
   ClassificationLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   ~ClassificationLayerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   ClassificationLayerImp &operator =(const ClassificationLayerImp &classificationLayer);

   GraphicObject* addObject(const GraphicObjectType& objectType, LocationType point);
   bool removeObject(GraphicObject* pObject, bool bDelete);
   std::list<GraphicObject*> getObjects() const;
   std::list<GraphicObject*> getObjects(const GraphicObjectType& objectType) const;
   GraphicObject* getObjectByName(const std::string &name) const { return NULL; }
   bool selectObject(GraphicObject* pObject);
   void selectAllObjects();
   bool isObjectSelected(GraphicObject* pObject) const;
   void getSelectedObjects(std::list<GraphicObject*>& selectedObjects) const;
   void getSelectedObjects(const GraphicObjectType& objectType,
      std::list<GraphicObject*>& selectedObjects) const;
   unsigned int getNumSelectedObjects() const;
   unsigned int getNumSelectedObjects(const GraphicObjectType& objectType) const;
   bool deselectObject(GraphicObject* pObject);
   void deselectAllObjects();
   void groupSelection();
   void ungroupSelection();
   void clearSelection();
   void popFront();
   void pushBack();

   bool setBoundingBox(LocationType llCorner, LocationType urCorner);
   bool setText(const char* pText);
   const char* getText() const;
   bool setRotation(double angle);

   bool processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseMove(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   QFont getClassificationFont() const;
   QColor getClassificationColor() const;
   TextObject* getTopText() const;
   TextObject* getBottomText() const;

   std::vector<ColorType> getColors() const;
   void draw();

   bool getShowLabels() const;
   void setShowLabels(bool bShowLabels);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setClassificationFont(const QFont& classificationFont);
   void setClassificationColor(const QColor& classificationColor);
   void reset();

signals:
   void fontChanged(const QFont& classificationFont);
   void colorChanged(const QColor& classificationColor);

protected:
   const FontImp& getClassificationFontImp() const;

protected slots:
   void updateProperties(GraphicProperty* pProperty);

private:
   FontImp mClassificationFont;
   QColor mClassificationColor;
   TextObjectImp *mpTopText;
   TextObjectImp *mpBottomText;
};

#define CLASSIFICATIONLAYERADAPTEREXTENSION_CLASSES \
   ANNOTATIONLAYERADAPTEREXTENSION_CLASSES

#define CLASSIFICATIONLAYERADAPTER_METHODS(impClass) \
   ANNOTATIONLAYERADAPTER_METHODS(impClass) \
   void setClassificationFont(const Font& classificationFont) \
   { \
      impClass::setClassificationFont(classificationFont.getQFont()); \
   } \
   const Font& getClassificationFont() const \
   { \
      return impClass::getClassificationFontImp(); \
   } \
   void setClassificationColor(ColorType classificationColor) \
   { \
      if (classificationColor.isValid() == true) \
      { \
         QColor clrClassification(classificationColor.mRed, classificationColor.mGreen, classificationColor.mBlue); \
         impClass::setClassificationColor(clrClassification); \
      } \
   } \
   ColorType getClassificationColor() const \
   { \
      ColorType classificationColor; \
      \
      QColor clrClassification = impClass::getClassificationColor(); \
      if (clrClassification.isValid() == true) \
      { \
         classificationColor.mRed = clrClassification.red(); \
         classificationColor.mGreen = clrClassification.green(); \
         classificationColor.mBlue = clrClassification.blue(); \
      } \
      \
      return classificationColor; \
   }

#endif
