/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTWIDGETADAPTER_H
#define PLOTWIDGETADAPTER_H

#include "PlotWidget.h"
#include "PlotWidgetImp.h"

class PlotWidgetAdapter : public PlotWidget, public PlotWidgetImp
{
public:
   PlotWidgetAdapter(const std::string& id, const std::string& plotName, PlotType plotType, PlotSet* pPlotSet,
      QWidget* pParent = 0);
   ~PlotWidgetAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   PLOTWIDGETADAPTER_METHODS(PlotWidgetImp)
};

#endif
