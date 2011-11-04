/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CARTESIANGRIDLINESADAPTER_H
#define CARTESIANGRIDLINESADAPTER_H

#include "CartesianGridlines.h"
#include "CartesianGridlinesImp.h"

class CartesianGridlinesAdapter : public CartesianGridlines, public CartesianGridlinesImp CARTESIANGRIDLINESADAPTEREXTENSION_CLASSES
{
public:
   CartesianGridlinesAdapter(OrientationType orientation, PlotViewImp* pPlot, bool bPrimary);
   ~CartesianGridlinesAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   CARTESIANGRIDLINESADAPTER_METHODS(CartesianGridlinesImp)

private:
   CartesianGridlinesAdapter(const CartesianGridlinesAdapter& rhs);
};

#endif
