/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LOCATORIMP_H
#define LOCATORIMP_H

#include <QtCore/QString>
#include <QtGui/QColor>

#include "LocationType.h"
#include "PlotObjectImp.h"
#include "Locator.h"
#include "TypesFile.h"

class LocatorImp : public PlotObjectImp
{
   Q_OBJECT

public:
   LocatorImp(PlotViewImp* pPlot, bool bPrimary);
   ~LocatorImp();

   LocatorImp& operator= (const LocatorImp& object);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   virtual PlotObjectType getType() const;
   virtual void draw();

   void setLocation(const LocationType& location, bool updateText = true);
   LocationType getLocation() const;
   void setText(const QString& strTextX, const QString& strTextY);
   void getText(QString& strTextX, QString& strTextY) const;
   void setStyle(Locator::LocatorStyle style);
   Locator::LocatorStyle getStyle() const;
   void setColor(const QColor& locatorColor);
   QColor getColor() const;
   void setLineWidth(int iWidth);
   int getLineWidth() const;
   void setLineStyle(LineStyle eStyle);
   LineStyle getLineStyle() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

signals:
   void locationChanged(const LocationType& location);
   void styleChanged(Locator::LocatorStyle style);
   void textChanged(const QString&, const QString&);

protected slots:
   void updateExtents();

private:
   LocatorImp(const LocatorImp& rhs);

   double mMinX;
   double mMinY;
   double mMaxX;
   double mMaxY;
   LocationType mLocation;
   Locator::LocatorStyle mStyle;
   QColor mColor;
   int mLineWidth;
   LineStyle mLineStyle;
   QString mTextX;
   QString mTextY;
};

#define LOCATORADAPTEREXTENSION_CLASSES \
   PLOTOBJECTADAPTEREXTENSION_CLASSES

#define LOCATORADAPTER_METHODS(impClass) \
   PLOTOBJECTADAPTER_METHODS(impClass) \
   void setLocation(const LocationType& location, bool updateText = true) \
   { \
      return impClass::setLocation(location, updateText); \
   } \
   LocationType getLocation() const \
   { \
      return impClass::getLocation(); \
   } \
   void setText(const std::string& strTextX, const std::string& strTextY) \
   { \
      return impClass::setText(QString::fromStdString(strTextX), \
         QString::fromStdString(strTextY)); \
   }\
   void getText(std::string& strTextX, std::string& strTextY) const \
   { \
      QString textX, textY;\
      impClass::getText(textX, textY); \
      strTextX = textX.toStdString(); \
      strTextY = textY.toStdString(); \
   } \
   void setStyle(LocatorStyle style) \
   { \
      return impClass::setStyle(style); \
   } \
   LocatorStyle getStyle() const \
   { \
      return impClass::getStyle(); \
   } \
   void setColor(const ColorType& locatorColor) \
   { \
      return impClass::setColor(QColor(locatorColor.mRed, locatorColor.mGreen, locatorColor.mBlue)); \
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
   void setLineStyle(LineStyle lineStyle) \
   { \
      return impClass::setLineStyle(lineStyle); \
   } \
   LineStyle getLineStyle() const \
   { \
      return impClass::getLineStyle(); \
   }

#endif
