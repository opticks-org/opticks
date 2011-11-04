/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLARPLOTADAPTER_H
#define POLARPLOTADAPTER_H

#include "PolarPlot.h"
#include "PolarPlotImp.h"

class PolarPlotAdapter : public PolarPlot, public PolarPlotImp POLARPLOTADAPTEREXTENSION_CLASSES
{
public:
   PolarPlotAdapter(const std::string& id, const std::string& viewName, QGLContext* pDrawContext = 0,
      QWidget* pParent = 0);
   ~PolarPlotAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   POLARPLOTADAPTER_METHODS(PolarPlotImp)

private:
   PolarPlotAdapter(const PolarPlotAdapter& rhs);
};

#endif
