/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANNOTATIONLAYERADAPTER_H
#define ANNOTATIONLAYERADAPTER_H

#include "AnnotationLayer.h"
#include "AnnotationLayerImp.h"

class AnnotationElement;

class AnnotationLayerAdapter : public AnnotationLayer, public AnnotationLayerImp ANNOTATIONLAYERADAPTEREXTENSION_CLASSES
{
public:
   AnnotationLayerAdapter(const std::string& id, const std::string& layerName, AnnotationElement* pAnno);
   ~AnnotationLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   ANNOTATIONLAYERADAPTER_METHODS(AnnotationLayerImp)

private:
   AnnotationLayerAdapter(const AnnotationLayerAdapter& rhs);
};

#endif
