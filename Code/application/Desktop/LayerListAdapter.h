/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LAYERLISTADAPTER_H
#define LAYERLISTADAPTER_H

#include "LayerList.h"
#include "LayerListImp.h"

class LayerListAdapter : public LayerList, public LayerListImp LAYERLISTADAPTEREXTENSION_CLASSES
{
public:
   LayerListAdapter();
   ~LayerListAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   LAYERLISTADAPTER_METHODS(LayerListImp)
};

#endif
