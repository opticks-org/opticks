/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CARTESIANGRIDLINESIMP_H
#define CARTESIANGRIDLINESIMP_H

#include "GridlinesImp.h"
#include "PlotGroupAdapter.h"

class CartesianGridlinesImp : public GridlinesImp
{
   Q_OBJECT

public:
   CartesianGridlinesImp(OrientationType orientation, PlotViewImp* pPlot, bool bPrimary);
   ~CartesianGridlinesImp();

   CartesianGridlinesImp& operator= (const CartesianGridlinesImp& object);

   PlotObjectType getType() const;
   void draw();

   OrientationType getOrientation() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

protected slots:
   void updateLocations();
   void updateColor(const QColor& lineColor);
   void updateLineWidth(int lineWidth);
   void updateLineStyle(LineStyle lineStyle);

private:
   OrientationType mOrientation;
   PlotGroupAdapter mLines;
};

#define CARTESIANGRIDLINESADAPTEREXTENSION_CLASSES \
   GRIDLINESADAPTEREXTENSION_CLASSES

#define CARTESIANGRIDLINESADAPTER_METHODS(impClass) \
   GRIDLINESADAPTER_METHODS(impClass) \
   OrientationType getOrientation() const \
   { \
      return impClass::getOrientation(); \
   }

#endif
