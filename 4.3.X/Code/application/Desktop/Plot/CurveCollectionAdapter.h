/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CURVECOLLECTIONADAPTER_H
#define CURVECOLLECTIONADAPTER_H

#include "CurveCollection.h"
#include "CurveCollectionImp.h"

class CurveCollectionAdapter : public CurveCollection, public CurveCollectionImp CURVECOLLECTIONADAPTEREXTENSION_CLASSES
{
public:
   CurveCollectionAdapter(PlotViewImp* pPlot, bool bPrimary);
   ~CurveCollectionAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   CURVECOLLECTIONADAPTER_METHODS(CurveCollectionImp)
};

#endif
