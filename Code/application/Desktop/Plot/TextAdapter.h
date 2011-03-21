/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TEXTADAPTER_H
#define TEXTADAPTER_H

#include "Text.h"
#include "TextImp.h"

class TextAdapter : public Text, public TextImp TEXTADAPTEREXTENSION_CLASSES
{
public:
   TextAdapter(PlotViewImp* pPlot, bool bPrimary);
   TextAdapter(PlotViewImp* pPlot, bool bPrimary, const LocationType& point);
   TextAdapter(PlotViewImp* pPlot, bool bPrimary, double dX, double dY);
   ~TextAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   TEXTADAPTER_METHODS(TextImp)
};

#endif
