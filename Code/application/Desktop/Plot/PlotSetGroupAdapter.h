/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTSETGROUPADAPTER_H
#define PLOTSETGROUPADAPTER_H

#include "PlotSetGroup.h"
#include "PlotSetGroupImp.h"

class PlotSetGroupAdapter : public PlotSetGroup, public PlotSetGroupImp PLOTSETGROUPADAPTEREXTENSION_CLASSES
{
public:
   PlotSetGroupAdapter(QWidget* pParent = NULL);
   virtual ~PlotSetGroupAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   PLOTSETGROUPADAPTER_METHODS(PlotSetGroupImp)

private:
   PlotSetGroupAdapter(const PlotSetGroupAdapter& rhs);
   PlotSetGroupAdapter& operator=(const PlotSetGroupAdapter& rhs);
};

#endif
