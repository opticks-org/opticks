/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTVIEWADAPTER_H
#define PLOTVIEWADAPTER_H

#include "PlotView.h"
#include "PlotViewImp.h"

class PlotViewAdapter : public PlotView, public PlotViewImp
{
public:
   PlotViewAdapter(const std::string& id, const std::string& viewName, QGLContext* drawContext = 0,
      QWidget* parent = 0);
   ~PlotViewAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   PLOTVIEWADAPTER_METHODS(PlotViewImp)
};

#endif
