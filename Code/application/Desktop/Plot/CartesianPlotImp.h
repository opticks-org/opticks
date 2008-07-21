/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CARTESIANPLOTIMP_H
#define CARTESIANPLOTIMP_H

#include <QtGui/QAction>

#include "PlotViewImp.h"
#include "CartesianGridlinesAdapter.h"
#include "TypesFile.h"

class CartesianPlotImp : public PlotViewImp
{
   Q_OBJECT

public:
   CartesianPlotImp(const std::string& id, const std::string& viewName, QGLContext* pDrawContext = 0,
      QWidget* pParent = 0);
   ~CartesianPlotImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfView(const std::string& className);
   static void getViewTypes(std::vector<std::string>& classList);

   CartesianPlotImp& operator= (const CartesianPlotImp& cartesianPlot);
   View* copy(QGLContext* pDrawContext = 0, QWidget* pParent = 0) const;
   bool copy(View *pView) const;
   using ViewImp::setName;

   PlotType getPlotType() const;

   // Gridlines
   CartesianGridlines* getGridlines(OrientationType orientation);
   const CartesianGridlines* getGridlines(OrientationType orientation) const;

   // Scale type
   ScaleType getXScaleType() const;
   ScaleType getYScaleType() const;

   // Data type
   QString getXDataType() const;
   QString getYDataType() const;

   // Coordinate transformations
   void translateWorldToData(double worldX, double worldY, double& dataX, double& dataY) const;
   void translateDataToWorld(double dataX, double dataY, double& worldX, double& worldY) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setXScaleType(ScaleType eType);
   void setYScaleType(ScaleType eType);
   void setXDataType(const QString& strDataType);
   void setYDataType(const QString& strDataType);
   void zoomCustom();

signals:
   void xScaleTypeChanged(ScaleType scaleType);
   void yScaleTypeChanged(ScaleType scaleType);
   void xDataTypeChanged(const QString& strDataType);
   void yDataTypeChanged(const QString& strDataType);

protected:
   void drawGridlines();

protected slots:
   void setMajorHorizontalGridlines(bool bShow);
   void setMajorVerticalGridlines(bool bShow);
   void setMinorHorizontalGridlines(bool bShow);
   void setMinorVerticalGridlines(bool bShow);

private:
   // Gridlines
   CartesianGridlinesAdapter mHorizontalGridlines;
   CartesianGridlinesAdapter mVerticalGridlines;

   // Scale type
   ScaleType mXScaleType;
   ScaleType mYScaleType;

   // Data type
   QString mXDataType;
   QString mYDataType;
};

#define CARTESIANPLOTADAPTEREXTENSION_CLASSES \
   PLOTVIEWADAPTEREXTENSION_CLASSES

#define CARTESIANPLOTADAPTER_METHODS(impClass) \
   PLOTVIEWADAPTER_METHODS(impClass) \
   CartesianGridlines* getGridlines(OrientationType orientation) \
   { \
      return impClass::getGridlines(orientation); \
   } \
   const CartesianGridlines* getGridlines(OrientationType orientation) const \
   { \
      return impClass::getGridlines(orientation); \
   } \
   void setXScaleType(ScaleType scaleType) \
   { \
      impClass::setXScaleType(scaleType); \
   } \
   void setYScaleType(ScaleType scaleType) \
   { \
      impClass::setYScaleType(scaleType); \
   } \
   ScaleType getXScaleType() const \
   { \
      return impClass::getXScaleType(); \
   } \
   ScaleType getYScaleType() const \
   { \
      return impClass::getYScaleType(); \
   } \
   void setXDataType(const std::string& dataType) \
   { \
      impClass::setXDataType(QString::fromStdString(dataType)); \
   } \
   void setYDataType(const std::string& dataType) \
   { \
      impClass::setYDataType(QString::fromStdString(dataType)); \
   } \
   std::string getXDataType() const \
   { \
      return impClass::getXDataType().toStdString(); \
   } \
   std::string getYDataType() const \
   { \
      return impClass::getYDataType().toStdString(); \
   }

#endif
