/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CURVEIMP_H
#define CURVEIMP_H

#include <QtGui/QColor>
#include <QtGui/QPixmap>

#include "LocationType.h"
#include "PlotObjectImp.h"
#include "TypesFile.h"

#include <vector>

class CurveImp : public PlotObjectImp
{
   Q_OBJECT

public:
   CurveImp(PlotViewImp* pPlot, bool bPrimary);
   ~CurveImp();

   CurveImp& operator= (const CurveImp& object);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   virtual PlotObjectType getType() const;
   virtual void draw();

   bool setPoints(const std::vector<LocationType>& points);
   const std::vector<LocationType>& getPoints() const;

   QColor getColor() const;
   int getLineWidth() const;
   LineStyle getLineStyle() const;

   virtual bool hit(LocationType point) const;
   virtual bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
   virtual const QPixmap& getLegendPixmap(bool bSelected) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setColor(const QColor& clrCurve);
   void setLineWidth(int iWidth);
   void setLineStyle(LineStyle eStyle);

signals:
   void pointsChanged(const std::vector<LocationType>& points);

private:
   CurveImp(const CurveImp& rhs);

   std::vector<LocationType> mPoints;

   QColor mColor;
   int mLineWidth;
   LineStyle mLineStyle;
};

#define CURVEADAPTEREXTENSION_CLASSES \
   PLOTOBJECTADAPTEREXTENSION_CLASSES

#define CURVEADAPTER_METHODS(impClass) \
   PLOTOBJECTADAPTER_METHODS(impClass) \
   bool setPoints(const std::vector<LocationType>& points) \
   { \
      return impClass::setPoints(points); \
   } \
   const std::vector<LocationType>& getPoints() const \
   { \
      return impClass::getPoints(); \
   } \
   void setColor(const ColorType& curveColor) \
   { \
      return impClass::setColor(QColor(curveColor.mRed, curveColor.mGreen, curveColor.mBlue)); \
   } \
   ColorType getColor() const \
   { \
      QColor color = impClass::getColor(); \
      return ColorType(color.red(), color.green(), color.blue()); \
   } \
   void setLineWidth(int iWidth) \
   { \
      return impClass::setLineWidth(iWidth); \
   } \
   int getLineWidth() const \
   { \
      return impClass::getLineWidth(); \
   } \
   void setLineStyle(const LineStyle& lineStyle) \
   { \
      return impClass::setLineStyle(lineStyle); \
   } \
   LineStyle getLineStyle() const \
   { \
      return impClass::getLineStyle(); \
   }

#endif
