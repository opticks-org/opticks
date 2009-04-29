/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTIMP_H
#define POINTIMP_H

#include <QtGui/QColor>
#include <QtGui/QPixmap>

#include "AttachmentPtr.h"
#include "PlotObjectImp.h"
#include "PointSet.h"
#include "TypesFile.h"
#include "Point.h"

class PointImp : public PlotObjectImp
{
   Q_OBJECT

public:
   PointImp(PlotViewImp* pPlot, bool bPrimary);
   PointImp(PlotViewImp* pPlot, bool bPrimary, LocationType point);
   PointImp(PlotViewImp* pPlot, bool bPrimary, double dX, double dY);
   ~PointImp();

   PointImp& operator= (const PointImp& object);

   PlotObjectType getType() const;
   void draw();
   void draw(LocationType pixelSize);

   double getXLocation() const;
   double getYLocation() const;
   const LocationType& getLocation() const;

   Point::PointSymbolType getSymbol() const;
   int getSymbolSize() const;
   QColor getColor() const;

   virtual const PointSet* getPointSet() const;
   virtual PointSet* getPointSet();
   virtual void setPointSet(PointSet* pPointSet);

   bool hit(LocationType point) const;
   bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
   const QPixmap& getLegendPixmap(bool bSelected) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setLocation(const LocationType& location);
   void setLocation(double dX, double dY);
   void setSymbol(const Point::PointSymbolType& eSymbol);
   void setSymbolSize(int iSize);
   void setColor(const QColor& clrSymbol);

signals:
   void locationChanged(const LocationType& location);
   void symbolChanged(const Point::PointSymbolType& eSymbol);
   void symbolSizeChanged(int iSize);
   void colorChanged(const QColor& clrSymbol);

private:
   LocationType mLocation;
   Point::PointSymbolType mSymbol;
   int mSymbolSize;
   QColor mColor;
   AttachmentPtr<PointSet> mpPointSet;
};

#define POINTADAPTEREXTENSION_CLASSES \
   PLOTOBJECTADAPTEREXTENSION_CLASSES

#define POINTADAPTER_METHODS(impClass) \
   PLOTOBJECTADAPTER_METHODS(impClass) \
   double getXLocation() const \
   { \
      return impClass::getXLocation(); \
   } \
   double getYLocation() const \
   { \
      return impClass::getYLocation(); \
   } \
   const LocationType& getLocation() const \
   { \
      return impClass::getLocation(); \
   } \
   Point::PointSymbolType getSymbol() const \
   { \
      return impClass::getSymbol(); \
   } \
   int getSymbolSize() const \
   { \
      return impClass::getSymbolSize(); \
   } \
   ColorType getColor() const \
   { \
      QColor color = impClass::getColor(); \
      return ColorType(color.red(), color.green(), color.blue()); \
   } \
   const PointSet* getPointSet() const \
   { \
      return impClass::getPointSet(); \
   } \
   PointSet* getPointSet() \
   { \
      return impClass::getPointSet(); \
   } \
   bool hit(LocationType point) \
   { \
      return impClass::hit(point); \
   } \
   void setLocation(const LocationType& location) \
   { \
      return impClass::setLocation(location); \
   } \
   void setLocation(double dX, double dY) \
   { \
      return impClass::setLocation(dX, dY); \
   } \
   void setSymbol(const Point::PointSymbolType& eSymbol) \
   { \
      return impClass::setSymbol(eSymbol); \
   } \
   void setSymbolSize(int iSize) \
   { \
      return impClass::setSymbolSize(iSize); \
   } \
   void setColor(const ColorType& clrSymbol) \
   { \
      return impClass::setColor(QColor(clrSymbol.mRed, clrSymbol.mGreen, clrSymbol.mBlue)); \
   } \
    void setPointSet(PointSet* pPointSet) \
   { \
      return impClass::setPointSet(pPointSet); \
   }
  
#endif
