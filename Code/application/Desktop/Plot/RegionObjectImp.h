/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef REGIONOBJECTIMP_H
#define REGIONOBJECTIMP_H

#include <QtGui/QColor>

#include "ColorType.h"
#include "PlotObjectImp.h"
#include "TypesFile.h"

#include <vector>

class ColorMap;

class RegionObjectImp : public PlotObjectImp
{
   Q_OBJECT

public:
   RegionObjectImp(PlotViewImp* pPlot, bool bPrimary);
   ~RegionObjectImp();

   RegionObjectImp& operator= (const RegionObjectImp& object);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   PlotObjectType getType() const;
   void draw();

   bool isValid() const;
   bool isSolidColor() const;
   QColor getColor() const;
   const std::vector<ColorType>& getColors() const;
   int getTransparency() const;
   bool getDrawBorder() const;

   bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
   const QPixmap& getLegendPixmap(bool bSelected) const;
   bool getRegion(double &minX, double &minY, double &maxX, double &maxY) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   
public slots:
   void setRegion(double dMinX, double dMinY, double dMaxX, double dMaxY);
   void setColor(const QColor& clrRegion);
   void setColors(const std::vector<QColor>& colors);
   void setColors(const std::vector<ColorType>& colors);
   void setColors(const ColorMap& colorMap);
   void setTransparency(int iTransparency);
   void setDrawBorder(bool bBorder);

signals:
   void regionChanged(double dMinX, double dMinY, double dMaxX, double dMaxY);
   void colorsChanged(const std::vector<ColorType>& colors);
   void transparencyChanged(int iTransparency);
   void borderToggled(bool bBorder);

private:
   double mMinX;
   double mMinY;
   double mMaxX;
   double mMaxY;
   std::vector<ColorType> mColors;
   int mTransparency;
   bool mBorder;
};

#define REGIONOBJECTADAPTER_METHODS(impClass) \
   PLOTOBJECTADAPTER_METHODS(impClass) \
   virtual bool isValid() const \
   { \
      return impClass::isValid(); \
   } \
   void setRegion(double dMinX, double dMinY, double dMaxX, double dMaxY) \
   { \
      return impClass::setRegion(dMinX, dMinY, dMaxX, dMaxY); \
   } \
   bool getRegion(double &minX, double &minY, \
      double &maxX, double &maxY) const \
   { \
      return impClass::getRegion(minX, minY, maxX, maxY); \
   } \
   bool isSolidColor() const \
   { \
      return impClass::isSolidColor(); \
   } \
   void setColor(const ColorType& regionColor) \
   { \
      return impClass::setColor(QColor(regionColor.mRed, regionColor.mGreen, regionColor.mBlue)); \
   } \
   void setColors(const std::vector<ColorType>& colors) \
   { \
      return impClass::setColors(colors); \
   } \
   ColorType getColor() const \
   { \
      QColor color = impClass::getColor(); \
      return ColorType(color.red(), color.green(), color.blue()); \
   } \
   const std::vector<ColorType>& getColors() const \
   { \
      return impClass::getColors(); \
   } \
   void setTransparency(int iTransparency) \
   { \
      return impClass::setTransparency(iTransparency); \
   } \
   int getTransparency() const \
   { \
      return impClass::getTransparency(); \
   } \
   void setDrawBorder(bool bBorder) \
   { \
      return impClass::setDrawBorder(bBorder); \
   } \
   bool getDrawBorder() const \
   { \
      return impClass::getDrawBorder(); \
   }

#endif
