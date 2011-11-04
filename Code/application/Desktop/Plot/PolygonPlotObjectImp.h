/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLYGONPLOTOBJECTIMP_H
#define POLYGONPLOTOBJECTIMP_H

#include "ColorType.h"
#include "PointSetImp.h"
#include "TypesFile.h"

class PolygonPlotObjectImp : public PointSetImp
{
   Q_OBJECT

public:
   PolygonPlotObjectImp(PlotViewImp* pPlot, bool bPrimary);
   ~PolygonPlotObjectImp();

   PolygonPlotObjectImp& operator= (const PolygonPlotObjectImp& object);

   PlotObjectType getType() const;
   void draw();

   QColor getFillColor() const;
   FillStyle getFillStyle() const;
   SymbolType getHatchStyle() const;
   bool isFilled() const;

   bool hit(LocationType point) const;
   const QPixmap& getLegendPixmap(bool bSelected) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setFillColor(const QColor& fillColor);
   void setFillStyle(const FillStyle& fillStyle);
   void setHatchStyle(const SymbolType& hatchStyle);

signals:
   void fillColorChanged(const QColor& fillColor);
   void fillStyleChanged(const FillStyle& fillStyle);
   void hatchStyleChanged(const SymbolType& hatchStyle);

private:
   PolygonPlotObjectImp(const PolygonPlotObjectImp& rhs);

   QColor mFillColor;
   FillStyle mFillStyle;
   SymbolType mHatchStyle;
};

#define POLYGONPLOTOBJECTADAPTEREXTENSION_CLASSES \
   POINTSETADAPTEREXTENSION_CLASSES

#define POLYGONPLOTOBJECTADAPTER_METHODS(impClass) \
   POINTSETADAPTER_METHODS(impClass) \
   ColorType getFillColor() const \
   { \
      QColor color = impClass::getFillColor(); \
      return ColorType(color.red(), color.green(), color.blue()); \
   } \
   FillStyle getFillStyle() const \
   { \
      return impClass::getFillStyle(); \
   } \
   SymbolType getHatchStyle() const \
   { \
      return impClass::getHatchStyle(); \
   } \
   bool isFilled() const \
   { \
      return impClass::isFilled(); \
   } \
   void setFillColor(const ColorType& fillColor) \
   { \
      return impClass::setFillColor(QColor(fillColor.mRed, fillColor.mGreen, fillColor.mBlue)); \
   } \
   void setFillStyle(const FillStyle& fillStyle) \
   { \
      return impClass::setFillStyle(fillStyle); \
   } \
   void setHatchStyle(const SymbolType& hatchStyle) \
   { \
      return impClass::setHatchStyle(hatchStyle); \
   }

#endif
