/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HISTOGRAMPLOTADAPTER_H
#define HISTOGRAMPLOTADAPTER_H

#include "HistogramPlot.h"
#include "HistogramPlotImp.h"

class HistogramPlotAdapter : public HistogramPlot, public HistogramPlotImp HISTOGRAMPLOTEXTENSION_CLASSES
{
public:
   HistogramPlotAdapter(const std::string& id, const std::string& viewName, QGLContext* pDrawContext = 0,
      QWidget* pParent = 0);
   ~HistogramPlotAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   HISTOGRAMPLOTADAPTER_METHODS(HistogramPlotImp)
};

#endif
