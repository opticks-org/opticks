/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GPUPROGRAMDESCRIPTORIMP_H
#define GPUPROGRAMDESCRIPTORIMP_H

#include "GpuProgramDescriptor.h"
#include "ObjectResource.h"
#include <vector>

class GpuProgramDescriptorImp
{
public:
   /**
    * Constructor.
    *
    * @throws AssertException if the input parameters are invalid.
    */
   GpuProgramDescriptorImp();

   /**
    * Copy Constructor.
    *
    * @param gpuProgramDescriptor
    *        Copy this object.
    *
    * @throws AssertException if the input parameters are invalid.
    */
   GpuProgramDescriptorImp(const GpuProgramDescriptorImp& gpuProgramDescriptor);
   virtual ~GpuProgramDescriptorImp();

   virtual void setName(const std::string &name);
   virtual const std::string& getName() const;
   virtual void setType(GpuProgramDescriptor::GpuProgramType type);
   virtual GpuProgramDescriptor::GpuProgramType getType() const;
   virtual bool removeParameter(const std::string &name);
   virtual bool setParameter(const std::string &name, const DataVariant &value);
   virtual const DynamicObject *getParameters() const;
   virtual const DataVariant& getParameter(const std::string &name) const;

private:
   std::string mName;
   GpuProgramDescriptor::GpuProgramType mType;
   FactoryResource<DynamicObject> mpInputParams;
};

// Adapter must implement GpuProgramDescriptor::copy()
#define GPUPROGRAMDESCRIPTORADAPTER_METHODS(impClass) \
   void setName(const std::string &name) \
   { \
      impClass::setName(name); \
   } \
   const std::string& getName() const \
   { \
      return impClass::getName(); \
   } \
   void setType(GpuProgramDescriptor::GpuProgramType type) \
   { \
      impClass::setType(type); \
   } \
   GpuProgramDescriptor::GpuProgramType getType() const \
   { \
      return impClass::getType(); \
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
