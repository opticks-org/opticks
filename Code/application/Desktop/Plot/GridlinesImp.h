/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRIDLINESIMP_H
#define GRIDLINESIMP_H

#include <QtGui/QColor>

#include "PlotObjectImp.h"
#include "TypesFile.h"

class GridlinesImp : public PlotObjectImp
{
   Q_OBJECT

public:
   GridlinesImp(PlotViewImp* pPlot, bool bPrimary);
   ~GridlinesImp();

   GridlinesImp& operator= (const GridlinesImp& object);

   bool areMinorGridlinesEnabled() const;
   QColor getColor() const;
   int getLineWidth() const;
   LineStyle getLineStyle() const;
   int getMaxNumMajorLines() const;
   int getMaxNumMinorLines() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setVisible(bool bVisible);
   void enableMinorGridlines(bool bEnable);
   void setColor(const QColor& clrLine);
   void setLineWidth(int iWidth);
   void setLineStyle(LineStyle eStyle);
   void setMaxNumMajorLines(int numLines);
   void setMaxNumMinorLines(int numLines);

signals:
   void minorGridlinesEnabled(bool bEnable);
   void colorChanged(const QColor& lineColor);
   void lineWidthChanged(int lineWidth);
   void lineStyleChanged(LineStyle lineStyle);
   void maxNumMajorLinesChanged(int numLines);
   void maxNumMinorLinesChanged(int numLines);

protected slots:
   virtual void updateLocations() = 0;

private:
   bool mMinorGridlines;
   QColor mColor;
   int mLineWidth;
   LineStyle mLineStyle;
   int mMaxMajorLines;
   int mMaxMinorLines;
};

#define GRIDLINESADAPTER_METHODS(impClass) \
   PLOTOBJECTADAPTER_METHODS(impClass) \
   void enableMinorGridlines(bool bEnable) \
   { \
      impClass::enableMinorGridlines(bEnable); \
   } \
   bool areMinorGridlinesEnabled() const \
   { \
      return impClass::areMinorGridlinesEnabled(); \
   } \
   void setColor(const ColorType& lineColor) \
   { \
      impClass::setColor(QColor(lineColor.mRed, lineColor.mGreen, lineColor.mBlue)); \
   } \
   ColorType getColor() const \
   { \
      QColor lineColor = impClass::getColor(); \
      return ColorType(lineColor.red(), lineColor.green(), lineColor.blue()); \
   } \
   void setLineWidth(int lineWidth) \
   { \
      impClass::setLineWidth(lineWidth); \
   } \
   int getLineWidth() const \
   { \
      return impClass::getLineWidth(); \
   } \
   void setLineStyle(LineStyle lineStyle) \
   { \
      impClass::setLineStyle(lineStyle); \
   } \
   LineStyle getLineStyle() const \
   { \
      return impClass::getLineStyle(); \
   } \
   void setMaxNumMajorLines(int numLines) \
   { \
      impClass::setMaxNumMajorLines(numLines); \
   } \
   void setMaxNumMinorLines(int numLines) \
   { \
      impClass::setMaxNumMinorLines(numLines); \
   } \
   int getMaxNumMajorLines() const \
   { \
      return impClass::getMaxNumMajorLines(); \
   } \
   int getMaxNumMinorLines() const \
   { \
      return impClass::getMaxNumMinorLines(); \
   }

#endif
