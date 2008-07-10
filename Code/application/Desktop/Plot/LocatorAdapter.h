/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LOCATORADAPTER_H
#define LOCATORADAPTER_H

#include "Locator.h"
#include "LocatorImp.h"

class LocatorAdapter : public Locator, public LocatorImp LOCATORADAPTEREXTENSION_CLASSES
{
public:
   LocatorAdapter(PlotViewImp* pPlot, bool bPrimary);
   ~LocatorAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   LOCATORADAPTER_METHODS(LocatorImp)
};

#endif
