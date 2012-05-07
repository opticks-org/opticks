/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLARGRIDLINESIMP_H
#define POLARGRIDLINESIMP_H

#include "GridlinesImp.h"
#include "TypesFile.h"

#include <vector>

class PolarGridlinesImp : public GridlinesImp
{
   Q_OBJECT

public:
   PolarGridlinesImp(PlotViewImp* pPlot, bool bPrimary);
   ~PolarGridlinesImp();

   PolarGridlinesImp& operator= (const PolarGridlinesImp& object);

   PlotObjectType getType() const;
   void draw();

   double getRadialInterval() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setRadialInterval(double angle);

protected slots:
   void updateLocations();

private:
   PolarGridlinesImp(const PolarGridlinesImp& rhs);

   double mAngle;
   std::vector<double> mDrawLocations;
};

#define POLARGRIDLINESADAPTEREXTENSION_CLASSES \
   GRIDLINESADAPTEREXTENSION_CLASSES

#define POLARGRIDLINESADAPTER_METHODS(impClass) \
   GRIDLINESADAPTER_METHODS(impClass) \
   void setRadialInterval(double angle) \
   { \
      return impClass::setRadialInterval(angle); \
   } \
   double getRadialInterval() const \
   { \
      return impClass::getRadialInterval(); \
   }

#endif
