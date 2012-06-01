/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOREFERENCEDESCRIPTORADAPTER_H
#define GEOREFERENCEDESCRIPTORADAPTER_H

#include "GeoreferenceDescriptor.h"
#include "GeoreferenceDescriptorImp.h"

class GeoreferenceDescriptorAdapter : public GeoreferenceDescriptor, public GeoreferenceDescriptorImp
   GEOREFERENCEDESCRIPTORADAPTEREXTENSION_CLASSES
{
public:
   GeoreferenceDescriptorAdapter();
   virtual ~GeoreferenceDescriptorAdapter();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   GEOREFERENCEDESCRIPTORADAPTER_METHODS(GeoreferenceDescriptorImp)
};

#endif
