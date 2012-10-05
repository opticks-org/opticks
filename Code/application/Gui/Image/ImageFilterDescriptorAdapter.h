/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGEFILTERDESCRIPTORADAPTER_H
#define IMAGEFILTERDESCRIPTORADAPTER_H

#include "ImageFilterDescriptor.h"
#include "ImageFilterDescriptorImp.h"

class ImageFilterDescriptorAdapter : public ImageFilterDescriptor, public ImageFilterDescriptorImp
   IMAGEFILTERDESCRIPTOREXTENSION_CLASSES
{
public:
   ImageFilterDescriptorAdapter() {}
   ImageFilterDescriptorAdapter(const ImageFilterDescriptorAdapter &other) : ImageFilterDescriptorImp(other) {}
   ~ImageFilterDescriptorAdapter() {}

   IMAGEFILTERDESCRIPTORADAPTER_METHODS(ImageFilterDescriptorImp);

   ImageFilterDescriptor *copy() const
   {
      return new ImageFilterDescriptorAdapter(*this);
   }
};

#endif
