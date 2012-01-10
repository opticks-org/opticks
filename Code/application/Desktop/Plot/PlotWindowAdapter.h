/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTWINDOWADAPTER_H
#define PLOTWINDOWADAPTER_H

#include "PlotWindow.h"
#include "PlotWindowImp.h"

class PlotWindowAdapter : public PlotWindow, public PlotWindowImp PLOTWINDOWADAPTEREXTENSION_CLASSES
{
public:
   PlotWindowAdapter(const std::string& id, const std::string& windowName, QWidget* parent = 0);
   ~PlotWindowAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   PLOTWINDOWADAPTER_METHODS(PlotWindowImp)

private:
   PlotWindowAdapter(const PlotWindowAdapter& rhs);
   PlotWindowAdapter& operator=(const PlotWindowAdapter& rhs);
};

#endif
