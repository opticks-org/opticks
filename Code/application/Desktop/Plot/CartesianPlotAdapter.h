/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CARTESIANPLOTADAPTER_H
#define CARTESIANPLOTADAPTER_H

#include "CartesianPlot.h"
#include "CartesianPlotImp.h"

class CartesianPlotAdapter : public CartesianPlot, public CartesianPlotImp CARTESIANPLOTEXTENSION_CLASSES
{
public:
   CartesianPlotAdapter(const std::string& id, const std::string& viewName, QGLContext* pDrawContext = 0,
      QWidget* pParent = 0);
   ~CartesianPlotAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   CARTESIANPLOTADAPTER_METHODS(CartesianPlotImp)
};

#endif
