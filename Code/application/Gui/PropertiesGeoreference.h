/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESGEOREFERENCE_H
#define PROPERTIESGEOREFERENCE_H

#include "ObjectResource.h"
#include "PropertiesShell.h"
#include "RasterDataDescriptor.h"

class PropertiesGeoreference : public PropertiesShell
{
public:
   PropertiesGeoreference();
   virtual ~PropertiesGeoreference();

   virtual bool initialize(SessionItem* pSessionItem);
   virtual bool applyChanges();

protected:
   virtual QWidget* createWidget();

private:
   DataDescriptorResource<RasterDataDescriptor> mpDescriptor;
};

#endif
