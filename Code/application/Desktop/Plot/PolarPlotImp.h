/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLARPLOTIMP_H
#define POLARPLOTIMP_H

#include <QtGui/QAction>

#include "PlotViewImp.h"
#include "PolarGridlinesAdapter.h"

class PolarPlotImp : public PlotViewImp
{
   Q_OBJECT

public:
   PolarPlotImp(const std::string& id, const std::string& viewName, QGLContext* pDrawContext = 0,
      QWidget* pParent = 0);
   ~PolarPlotImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfView(const std::string& className);
   static void getViewTypes(std::vector<std::string>& classList);

   PolarPlotImp& operator= (const PolarPlotImp& polarPlot);
   View* copy(QGLContext* pDrawContext = 0, QWidget* pParent = 0) const;
   bool copy(View *pView) const;
   using ViewImp::setName;

   PlotType getPlotType() const;

   // Gridlines
   PolarGridlines* getGridlines();
   const PolarGridlines* getGridlines() const;

   // Coordinate transformations
   void translateWorldToData(double worldX, double worldY, double& dataX, double& dataY) const;
   void translateDataToWorld(double dataX, double dataY, double& worldX, double& worldY) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

protected:
   void drawGridlines();

protected slots:
   void setMajorGridlines(bool bShow);
   void setMinorGridlines(bool bShow);

private:
   // Gridlines
   PolarGridlinesAdapter mGridlines;
};

#define POLARPLOTEXTENSION_CLASSES \
   PLOTVIEWEXTENSION_CLASSES

#define POLARPLOTADAPTER_METHODS(impClass) \
   PLOTVIEWADAPTER_METHODS(impClass) \
   PolarGridlines* getGridlines() \
   { \
      return impClass::getGridlines(); \
   } \
   const PolarGridlines* getGridlines() const \
   { \
      return impClass::getGridlines(); \
   }

#endif
