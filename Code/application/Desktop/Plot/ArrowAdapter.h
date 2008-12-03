/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ARROWADAPTER_H
#define ARROWADAPTER_H

#include "Arrow.h"
#include "ArrowImp.h"

class ArrowAdapter : public Arrow, public ArrowImp ARROWADAPTEREXTENSION_CLASSES
{
public:
   ArrowAdapter(PlotViewImp* pPlot, bool bPrimary);
   ArrowAdapter(ArrowStyle arrowStyle, PlotViewImp* pPlot, bool bPrimary);
   ~ArrowAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   ARROWADAPTER_METHODS(ArrowImp)
};

#endif
