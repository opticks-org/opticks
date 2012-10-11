/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGEFILTERDESCRIPTORIMP_H
#define IMAGEFILTERDESCRIPTORIMP_H

#include "ImageFilterDescriptor.h"
#include "ObjectResource.h"

#include <vector>

class ImageFilterDescriptorImp
{
public:
   ImageFilterDescriptorImp();
   ImageFilterDescriptorImp(const ImageFilterDescriptorImp &filterDescriptor);
   virtual ~ImageFilterDescriptorImp();

   virtual ImageFilterDescriptor* copy() const = 0;

   void setName(const std::string &name);
   const std::string& getName() const;
   void setDescription(const std::string &description);
   const std::string& getDescription() const;
   void setType(ImageFilterDescriptor::ImageProcessType type);
   ImageFilterDescriptor::ImageProcessType getType() const;
   bool addGpuProgram(GpuProgramDescriptor *pGpuDescriptor);
   void removeGpuProgram(const GpuProgramDescriptor& gpuDescriptor);
   bool hasGpuProgram(const GpuProgramDescriptor& gpuDescriptor) const;
   void clearImagePrograms();
   void clear();
   const std::vector<GpuProgramDescriptor*>& getGpuPrograms() const;
   bool removeParameter(const std::string &name);
   bool setParameter(const std::string &name, const DataVariant &value);
   const DynamicObject *getParameters() const;
   const DataVariant& getParameter(const std::string &name) const;

private:
   std::string mName;                                       // name of the image filter
   std::string mDescription;                                // description of the image filter
   ImageFilterDescriptor::ImageProcessType  mType;          // type of the image filter
   std::vector<GpuProgramDescriptor*> mGpuPrograms;          // GPU programs used by the image filter
   FactoryResource<DynamicObject> mpInputParams;
};

#define IMAGEFILTERDESCRIPTOREXTENSION_CLASSES

// Adapter must implement ImageFilterDescriptor::copy()
#define IMAGEFILTERDESCRIPTORADAPTER_METHODS(impClass) \
   const std::string& getName() const \
   { \
      return impClass::getName(); \
   } \
   const std::string& getDescription() const \
   { \
      return impClass::getDescription(); \
   } \
   ImageProcessType getType() const \
   { \
      return impClass::getType(); \
   } \
   bool hasGpuProgram(const GpuProgramDescriptor& gpuDescriptor) const \
   { \
      return impClass::hasGpuProgram(gpuDescriptor); \
   } \
   const std::vector<GpuProgramDescriptor*>& getGpuPrograms() const \
   { \
      return impClass::getGpuPrograms(); \
   } \
   bool removeParameter(const std::string &name) \
   { \
      return impClass::removeParameter(name); \
   } \
   bool setParameter(const std::string &name, const DataVariant &value) \
   { \
      return impClass::setParameter(name, value); \
   } \
   const DynamicObject *getParameters() const \
   { \
      return impClass::getParameters(); \
   } \
   const DataVariant& getParameter(const std::string &name) const \
   { \
      return impClass::getParameter(name); \
   }

#endif
