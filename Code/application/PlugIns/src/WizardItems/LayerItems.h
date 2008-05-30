/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef LAYERITEMS_H
#define LAYERITEMS_H

#include "DesktopItems.h"
#include "TypesFile.h"

#include <string>

class LayerItems : public DesktopItems
{
public:
   LayerItems();
   ~LayerItems();

protected:
   virtual LayerType getLayerType() const = 0;

   std::string getLayerType(LayerType eType);
   std::string getModelType(LayerType eType);
};

#endif
