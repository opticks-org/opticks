/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTSETIMP_H
#define POINTSETIMP_H

#include <QtGui/QColor>
#include <QtGui/QPixmap>

#include "ColorType.h"
#include "PlotObjectImp.h"
#include "TypesFile.h"
#include "Point.h"

#include <boost/any.hpp>
#include <string>
#include <vector>

class PointSetImp : public PlotObjectImp
{
   Q_OBJECT

public:
   PointSetImp(PlotViewImp* pPlot, bool bPrimary);
   ~PointSetImp();

   PointSetImp& operator= (const PointSetImp& object);

   PlotObjectType getType() const;
   void draw();

   // Points
   Point* addPoint();
   Point* addPoint(double dX, double dY);
   bool insertPoint(Point* pPoint);
   void setPoints(const std::vector<Point*>& points);
   std::vector<Point*> getPoints() const;
   unsigned int getNumPoints() const;
   bool hasPoint(Point* pPoint) const;
   bool removePoint(Point* pPoint, bool bDelete);
   void clear(bool bDelete);

   // Symbols
   bool areSymbolsDisplayed() const;

   // Line
   bool isLineDisplayed() const;
   QColor getLineColor() const;
   int getLineWidth() const;
   LineStyle getLineStyle() const;

   Point* hitPoint(LocationType point) const;
   bool hit(LocationType point) const;
   bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
   const QPixmap& getLegendPixmap(bool bSelected) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   // Points
   void setVisible(bool bVisible);
   void setSelected(bool bSelect);

   // Symbols
   void displaySymbols(bool bDisplay);
   void setPointSymbol(const Point::PointSymbolType& eSymbol);
   void setPointSymbolSize(unsigned int symbolSize);
   void setPointColor(const QColor& clrSymbol);

   // Line
   void displayLine(bool bDisplay);
   void setLineColor(const QColor& clrLine);
   void setLineWidth(int iWidth);
   void setLineStyle(const LineStyle& eStyle);
   
   void setInteractive(bool interactive);
   bool getInteractive();

   void deleteSelectedPoints(bool filterVisible);

signals:
   void pointAdded(Point* pPoint);
   void pointRemoved(Point* pPoint);
   void pointsSet(const std::vector<Point*>& points);
   void pointLocationChanged(Point* pPoint, const LocationType& location);
   void lineDisplayChanged(bool display);

protected:
   void propagateLocationChanged(Subject& subject, const std::string& signal, const boost::any& value);

private:
   // Points
   std::vector<Point*> mPoints;

   // Symbols
   bool mSymbols;

   // Line
   bool mLine;
   QColor mLineColor;
   int mLineWidth;
   LineStyle mLineStyle;

   bool mInteractive;
   bool mDirty;
};

#define POINTSETADAPTEREXTENSION_CLASSES \
   PLOTOBJECTADAPTEREXTENSION_CLASSES

#define POINTSETADAPTER_METHODS(impClass) \
   PLOTOBJECTADAPTER_METHODS(impClass) \
   virtual Point* addPoint() \
   { \
      return impClass::addPoint(); \
   } \
   Point* addPoint(double dX, double dY) \
   { \
      return impClass::addPoint(dX, dY); \
   } \
   bool insertPoint(Point* pPoint) \
   { \
      return impClass::insertPoint(pPoint); \
   } \
   void setPoints(const std::vector<Point*>& points) \
   { \
      return impClass::setPoints(points); \
   } \
   std::vector<Point*> getPoints() const \
   { \
      return impClass::getPoints(); \
   } \
   unsigned int getNumPoints() const \
   { \
      return impClass::getNumPoints(); \
   } \
   bool hasPoint(Point* pPoint) const \
   { \
      return impClass::hasPoint(pPoint); \
   } \
   bool removePoint(Point* pPoint, bool bDelete) \
   { \
      return impClass::removePoint(pPoint, bDelete); \
   } \
   void clear(bool bDelete) \
   { \
      return impClass::clear(bDelete); \
   } \
   bool areSymbolsDisplayed() const \
   { \
      return impClass::areSymbolsDisplayed(); \
   } \
   void displaySymbols(bool bDisplay) \
   { \
      return impClass::displaySymbols(bDisplay); \
   } \
   bool isLineDisplayed() const \
   { \
      return impClass::isLineDisplayed(); \
   } \
   ColorType getLineColor() const \
   { \
      QColor color = impClass::getLineColor(); \
      return ColorType(color.red(), color.green(), color.blue()); \
   } \
   int getLineWidth() const \
   { \
      return impClass::getLineWidth(); \
   } \
   LineStyle getLineStyle() const \
   { \
      return impClass::getLineStyle(); \
   } \
   void displayLine(bool bDisplay) \
   { \
      return impClass::displayLine(bDisplay); \
   } \
   Point* hitPoint(LocationType point) const \
   { \
      return impClass::hitPoint(point); \
   } \
   bool hit(LocationType point) const \
   { \
      return impClass::hit(point); \
   } \
   void setLineColor(const ColorType& clrLine) \
   { \
      return impClass::setLineColor(QColor(clrLine.mRed, clrLine.mGreen, clrLine.mBlue)); \
   } \
   void setLineWidth(int iWidth) \
   { \
      return impClass::setLineWidth(iWidth); \
   } \
   void setLineStyle(const LineStyle& eStyle) \
   { \
      return impClass::setLineStyle(eStyle); \
   } \
   void setInteractive(bool interactive) \
   { \
      return impClass::setInteractive(interactive); \
   } \
   bool getInteractive() \
   { \
      return impClass::getInteractive(); \
   }

#endif
