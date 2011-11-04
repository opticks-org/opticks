/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GCPLAYERADAPTER_H
#define GCPLAYERADAPTER_H

#include "GcpLayer.h"
#include "GcpLayerImp.h"

class GcpList;

class GcpLayerAdapter : public GcpLayer, public GcpLayerImp GCPLAYERADAPTEREXTENSION_CLASSES
{
public:
   GcpLayerAdapter(const std::string& id, const std::string& layerName, GcpList* pGcpList);
   ~GcpLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   GCPLAYERADAPTER_METHODS(GcpLayerImp)

private:
   GcpLayerAdapter(const GcpLayerAdapter& rhs);
};

#endif
