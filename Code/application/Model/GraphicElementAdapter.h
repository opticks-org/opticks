/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICELEMENTADAPTER_H
#define GRAPHICELEMENTADAPTER_H

#include "GraphicElement.h"
#include "GraphicElementImp.h"

class GraphicElementAdapter : public GraphicElement, public GraphicElementImp
{
public:
   GraphicElementAdapter(const DataDescriptorImp& descriptor, const std::string& id);
   ~GraphicElementAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   GRAPHICELEMENTADAPTER_METHODS(GraphicElementImp)
};

#endif
