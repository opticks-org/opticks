/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CLASSIFICATIONLAYERADAPTER_H
#define CLASSIFICATIONLAYERADAPTER_H

#include "ClassificationLayer.h"
#include "ClassificationLayerImp.h"

class AnnotationElement;

class ClassificationLayerAdapter : public ClassificationLayer, public ClassificationLayerImp CLASSIFICATIONLAYERADAPTEREXTENSION_CLASSES
{
public:
   ClassificationLayerAdapter(const std::string& id, const std::string& layerName, AnnotationElement* pElement);
   ~ClassificationLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   CLASSIFICATIONLAYERADAPTER_METHODS(ClassificationLayerImp)
};

#endif
