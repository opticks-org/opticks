/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CURVECOLLECTIONIMP_H
#define CURVECOLLECTIONIMP_H

#include <QtGui/QColor>

#include "PlotObjectImp.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class Curve;

class CurveCollectionImp : public PlotObjectImp
{
   Q_OBJECT

public:
   CurveCollectionImp(PlotViewImp* pPlot, bool bPrimary);
   ~CurveCollectionImp();

   CurveCollectionImp& operator= (const CurveCollectionImp& object);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   virtual PlotObjectType getType() const;
   virtual void draw();

   Curve* addCurve();
   bool insertCurve(Curve* pCurve);
   const std::vector<Curve*>& getCurves() const;
   unsigned int getNumCurves() const;
   bool deleteCurve(Curve* pCurve);
   void clear();

   QColor getColor() const;
   int getLineWidth() const;
   LineStyle getLineStyle() const;

   virtual bool hit(LocationType point) const;
   virtual bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
   virtual const QPixmap& getLegendPixmap(bool bSelected) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   virtual void setSelected(bool bSelect);

   void setColor(const QColor& clrCurve);
   void setLineWidth(int iWidth);
   void setLineStyle(LineStyle eStyle);

signals:
   void curveAdded(Curve* pCurve);
   void curveDeleted(Curve* pCurve);
   void pointsChanged();

private:
   std::vector<Curve*> mCurves;

   QColor mColor;
   int mLineWidth;
   LineStyle mLineStyle;
};

#define CURVECOLLECTIONADAPTEREXTENSION_CLASSES \
   PLOTOBJECTADAPTEREXTENSION_CLASSES

#define CURVECOLLECTIONADAPTER_METHODS(impClass) \
   PLOTOBJECTADAPTER_METHODS(impClass) \
   Curve* addCurve() \
   { \
      return impClass::addCurve(); \
   } \
   const std::vector<Curve*>& getCurves() const \
   { \
      return impClass::getCurves(); \
   } \
   unsigned int getNumCurves() const \
   { \
      return impClass::getNumCurves(); \
   } \
   bool deleteCurve(Curve* pCurve) \
   { \
      return impClass::deleteCurve(pCurve); \
   } \
   void clear() \
   { \
      return impClass::clear(); \
   } \
   void setColor(const ColorType& collectionColor) \
   { \
      return impClass::setColor(QColor(collectionColor.mRed, collectionColor.mGreen, collectionColor.mBlue)); \
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
