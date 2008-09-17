/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ARROWIMP_H
#define ARROWIMP_H

#include "PlotObjectImp.h"
#include "PointSetAdapter.h"
#include "PolygonPlotObjectAdapter.h"
#include "TypesFile.h"

class Point;

class ArrowImp : public PlotObjectImp
{
   Q_OBJECT

public:
   ArrowImp(PlotViewImp* pPlot, bool bPrimary);
   ArrowImp(ArrowStyle arrowStyle, PlotViewImp* pPlot, bool bPrimary);
   ~ArrowImp();

   ArrowImp& operator= (const ArrowImp& object);

   PlotObjectType getType() const;
   void draw();

   ArrowStyle getArrowStyle() const;
   LocationType getBaseLocation() const;
   LocationType getTipLocation() const;
   QColor getColor() const;

   bool hit(LocationType point) const;
   bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
   const QPixmap& getLegendPixmap(bool bSelected) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setArrowStyle(const ArrowStyle& eStyle);
   void setLocation(const LocationType& baseLocation, const LocationType& tipLocation);
   void setBaseLocation(const LocationType& baseLocation);
   void setTipLocation(const LocationType& tipLocation);
   void setColor(const QColor& newColor);

protected:
   Point* getBasePoint() const;
   Point* getTipPoint() const;

protected slots:
   void updateArrowHead();

private:
   ArrowStyle mStyle;
   PointSetAdapter mLine;
   PolygonPlotObjectAdapter mArrowHead;
};

#define ARROWADAPTEREXTENSION_CLASSES \
   PLOTOBJECTADAPTEREXTENSION_CLASSES

#define ARROWADAPTER_METHODS(impClass) \
   PLOTOBJECTADAPTER_METHODS(impClass) \
   ArrowStyle getArrowStyle() const \
   { \
      return impClass::getArrowStyle(); \
   } \
   LocationType getBaseLocation() const \
   { \
      return impClass::getBaseLocation(); \
   } \
   LocationType getTipLocation() const \
   { \
      return impClass::getTipLocation(); \
   } \
   ColorType getColor() const \
   { \
      QColor color = impClass::getColor(); \
      return ColorType(color.red(), color.blue(), color.green()); \
   } \
   bool hit(LocationType point) \
   { \
      return impClass::hit(point); \
   } \
   void setArrowStyle(const ArrowStyle& eStyle) \
   { \
      return impClass::setArrowStyle(eStyle); \
   } \
   void setLocation(const LocationType& baseLocation, const LocationType& tipLocation) \
   { \
      return impClass::setLocation(baseLocation, tipLocation); \
   } \
   void setBaseLocation(const LocationType& baseLocation) \
   { \
      return impClass::setBaseLocation(baseLocation); \
   } \
   void setTipLocation(const LocationType& tipLocation) \
   { \
      return impClass::setTipLocation(tipLocation); \
   } \
   void setColor(const ColorType& newColor) \
   { \
      return impClass::setColor(QColor(newColor.mRed, newColor.mGreen, newColor.mBlue)); \
   }

#endif
