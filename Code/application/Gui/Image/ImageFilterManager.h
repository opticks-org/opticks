/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGE_FILTER_MANAGER_H
#define IMAGE_FILTER_MANAGER_H

#include "Service.h"

#include <map>
#include <string>
#include <vector>

class ImageFilterDescriptor;
class ImageFilterDescriptorImp;

class ImageFilterManager
{
public:
   std::vector<std::string> getAvailableFilters() const;
   unsigned int getNumAvailableFilters() const;

   ImageFilterDescriptor* createFilterDescriptor(const std::string& filterName) const;

protected:
   void buildFilterList(const std::string& filterPath = std::string());
   void clearFilters();

   std::vector<ImageFilterDescriptorImp*> loadFilterDescriptors(const std::string& filterPath) const;
   std::vector<ImageFilterDescriptorImp*> deserialize(const std::string& filename) const;

private:
   ImageFilterManager();
   ~ImageFilterManager();

   friend class DesktopServicesImp;

private:
   std::map<std::string, ImageFilterDescriptorImp*> mImageFilters;
};

template<>
ImageFilterManager* Service<ImageFilterManager>::get() const;

#endif
