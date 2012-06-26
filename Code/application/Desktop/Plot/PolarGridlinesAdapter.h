/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLARGRIDLINESADAPTER_H
#define POLARGRIDLINESADAPTER_H

#include "PolarGridlines.h"
#include "PolarGridlinesImp.h"

class PolarGridlinesAdapter : public PolarGridlines, public PolarGridlinesImp POLARGRIDLINESADAPTEREXTENSION_CLASSES
{
public:
   PolarGridlinesAdapter(PlotViewImp* pPlot, bool bPrimary);
   ~PolarGridlinesAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   POLARGRIDLINESADAPTER_METHODS(PolarGridlinesImp)

private:
   PolarGridlinesAdapter(const PolarGridlinesAdapter& rhs);
};

#endif
