/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AOILAYERIMP_H
#define AOILAYERIMP_H

#include <QtCore/QPoint>
#include <QtGui/QColor>
#include <QtGui/QFont>

#include "ColorType.h"
#include "GraphicLayerImp.h"
#include "TypesFile.h"

class AoiLayerImp : public GraphicLayerImp
{
   Q_OBJECT

public:
   AoiLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   ~AoiLayerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void draw();

   AoiLayerImp& operator= (const AoiLayerImp& aoiLayer);

   LayerType getLayerType() const;
   using LayerImp::setName;

   std::vector<ColorType> getColors() const;
   QColor getColor() const;
   SymbolType getSymbol() const;
   ModeType getMode() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

   LocationType correctCoordinate(const LocationType &coord) const;

   GraphicObject *addObject(const GraphicObjectType &objectType, LocationType point);

   QCursor getMouseCursor() const;

   bool processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseMove(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   virtual QColor getLabelColor(const GraphicObjectImp *pObj);

   virtual bool mayDrawAsPixels() const;
   virtual bool willDrawAsPixels() const;
   bool hitLabel(const QPoint& screenCoord) const;

public slots:
   void setColor(const QColor& aoiColor);
   void setSymbol(const SymbolType& aoiSymbol);
   void reset();
   void setMode(ModeType mode);
   void objectWasAdded(GraphicObject *pObject);

signals:
   void colorChanged(const QColor& aoiColor);
   void symbolChanged(const SymbolType& aoiSymbol);
   void modeChanged(ModeType mode);

protected:
   void layerActivated(bool activated);
   virtual void drawGroup();

   QColor mColor;
   SymbolType mSymbol;

private:
   bool mustDrawAsBitmask() const;

   double mLabelHandleSize;
   LocationType mLabelOffset;
   LocationType mLabelLocation;
   bool mLabelHandleDrawn;
   bool mLabelMoving;
   ModeType mCurrentMode;
   QFont mFont;

   static unsigned int sNumLayers;
};

#define AOILAYERADAPTEREXTENSION_CLASSES \
   GRAPHICLAYERADAPTEREXTENSION_CLASSES

#define AOILAYERADAPTER_METHODS(impClass) \
   GRAPHICLAYERADAPTER_METHODS(impClass) \
   void setColor(const ColorType& aoiColor) \
   { \
      QColor clrAoi; \
      if (aoiColor.isValid() == true) \
      { \
         clrAoi.setRgb(aoiColor.mRed, aoiColor.mGreen, aoiColor.mBlue); \
      } \
      \
      impClass::setColor(clrAoi); \
   } \
   ColorType getColor() const \
   { \
      ColorType aoiColor; \
      \
      QColor clrAoi = impClass::getColor(); \
      if (clrAoi.isValid() == true) \
      { \
         aoiColor.mRed = clrAoi.red(); \
         aoiColor.mGreen = clrAoi.green(); \
         aoiColor.mBlue = clrAoi.blue(); \
      } \
      \
      return aoiColor; \
   } \
   void setSymbol(const SymbolType& aoiSymbol) \
   { \
      return impClass::setSymbol(aoiSymbol); \
   } \
   SymbolType getSymbol() const \
   { \
      return impClass::getSymbol(); \
   } \
   void setMode(ModeType mode) \
   { \
      impClass::setMode(mode); \
   } \
   ModeType getMode() const \
   { \
      return impClass::getMode(); \
   }

#endif
