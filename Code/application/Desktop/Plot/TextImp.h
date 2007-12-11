/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TEXTIMP_H
#define TEXTIMP_H

#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QPixmap>

#include "PlotObjectImp.h"
#include "TypesFile.h"

class TextImp : public PlotObjectImp
{
   Q_OBJECT

public:
   TextImp(PlotViewImp* pPlot, bool bPrimary);
   TextImp(PlotViewImp* pPlot, bool bPrimary, const LocationType& point);
   TextImp(PlotViewImp* pPlot, bool bPrimary, double dX, double dY);
   ~TextImp();

   TextImp& operator= (const TextImp& object);

   PlotObjectType getType() const;
   void draw();

   double getXLocation() const;
   double getYLocation() const;
   const LocationType& getLocation() const;

   QString getText() const;
   QFont getFont() const;
   QColor getColor() const;

   bool hit(LocationType point) const;
   bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
   const QPixmap& getLegendPixmap(bool bSelected) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setLocation(const LocationType& location);
   void setLocation(double dX, double dY);
   void setText(const QString& strText);
   void setFont(const QFont& textFont);
   void setColor(const QColor& clrText);

signals:
   void locationChanged(const LocationType& location);
   void textChanged(const QString& strText);

private:
   LocationType mLocation;
   QString mText;
   QFont mFont;
   QColor mColor;
};

#define TEXTADAPTER_METHODS(impClass) \
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
   std::string getText() const \
   { \
      return impClass::getText().toStdString(); \
   } \
   ColorType getColor() const \
   { \
      QColor color = impClass::getColor(); \
      return ColorType(color.red(), color.green(), color.blue()); \
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
   void setText(const std::string& strText) \
   { \
      return impClass::setText(QString::fromStdString(strText)); \
   } \
   void setColor(const ColorType& clrText) \
   { \
      return impClass::setColor(QColor(clrText.mRed, clrText.mGreen, clrText.mBlue)); \
   }

#endif
