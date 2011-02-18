/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SPATIALDATAVIEWADAPTER_H
#define SPATIALDATAVIEWADAPTER_H

#include "SpatialDataView.h"
#include "SpatialDataViewImp.h"

class SpatialDataViewAdapter : public SpatialDataView, public SpatialDataViewImp SPATIALDATAVIEWADAPTEREXTENSION_CLASSES
{
public:
   SpatialDataViewAdapter(const std::string& id, const std::string& viewName, QGLContext* drawContext = 0,
      QWidget* parent = 0);
   ~SpatialDataViewAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SPATIALDATAVIEWADAPTER_METHODS(SpatialDataViewImp)
};

#endif
