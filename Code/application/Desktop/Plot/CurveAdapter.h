/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CURVEADAPTER_H
#define CURVEADAPTER_H

#include "Curve.h"
#include "CurveImp.h"

class CurveAdapter : public Curve, public CurveImp CURVEADAPTEREXTENSION_CLASSES
{
public:
   CurveAdapter(PlotViewImp* pPlot, bool bPrimary);
   ~CurveAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   CURVEADAPTER_METHODS(CurveImp)
};

#endif
