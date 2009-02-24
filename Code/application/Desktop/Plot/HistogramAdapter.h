/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HISTOGRAMADAPTER_H
#define HISTOGRAMADAPTER_H

#include "Histogram.h"
#include "HistogramImp.h"

class HistogramAdapter : public Histogram, public HistogramImp HISTOGRAMADAPTEREXTENSION_CLASSES
{
public:
   HistogramAdapter(PlotViewImp* pPlot, bool bPrimary);
   ~HistogramAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   HISTOGRAMADAPTER_METHODS(HistogramImp)
};

#endif
