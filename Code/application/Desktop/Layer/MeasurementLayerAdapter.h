/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MEASUREMENTLAYERADAPTER_H
#define MEASUREMENTLAYERADAPTER_H

#include "MeasurementLayer.h"
#include "MeasurementLayerImp.h"

class AnnotationElement;

class MeasurementLayerAdapter : public MeasurementLayer, public MeasurementLayerImp
{
public:
   MeasurementLayerAdapter(const std::string& id, const std::string& layerName, AnnotationElement* pElement);
   ~MeasurementLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   MEASUREMENTLAYERADAPTER_METHODS(MeasurementLayerImp)

private:
   MeasurementLayerAdapter(const MeasurementLayerAdapter& rhs);
};

#endif
