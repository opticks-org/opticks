/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICLAYERADAPTER_H
#define GRAPHICLAYERADAPTER_H

#include <QtCore/QString>

#include "GraphicLayer.h"
#include "GraphicLayerImp.h"

class GraphicElement;

class GraphicLayerAdapter : public GraphicLayer, public GraphicLayerImp GRAPHICLAYERADAPTEREXTENSION_CLASSES
{
public:
   GraphicLayerAdapter(const std::string& id, const std::string& layerName, GraphicElement* pElement);
   ~GraphicLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   GRAPHICLAYERADAPTER_METHODS(GraphicLayerImp)
};

#endif
