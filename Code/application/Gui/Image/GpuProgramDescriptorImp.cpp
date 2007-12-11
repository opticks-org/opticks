/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppAssert.h"
#include "DynamicObject.h"
#include "GpuProgramDescriptorImp.h"

using namespace std;

GpuProgramDescriptorImp::GpuProgramDescriptorImp()
{
   REQUIRE(mpInputParams.get() != NULL);
}

GpuProgramDescriptorImp::GpuProgramDescriptorImp(const GpuProgramDescriptorImp& gpuProgramDescriptor)
{
   REQUIRE(mpInputParams.get() != NULL);

   mName = gpuProgramDescriptor.mName;
   mType = gpuProgramDescriptor.mType;
   mpInputParams->merge(gpuProgramDescriptor.mpInputParams.get());
}

GpuProgramDescriptorImp::~GpuProgramDescriptorImp()
{
}

void GpuProgramDescriptorImp::setName(const string &name)
{
   mName = name;
}

const string &GpuProgramDescriptorImp::getName() const
{
   return mName;
}

void GpuProgramDescriptorImp::setType(GpuProgramDescriptor::GpuProgramType type)
{
   mType = type;
}

GpuProgramDescriptor::GpuProgramType GpuProgramDescriptorImp::getType() const
{
   return mType;
}

bool GpuProgramDescriptorImp::removeParameter(const string &name)
{
   return mpInputParams->removeAttribute(name);
}

bool GpuProgramDescriptorImp::setParameter(const string &name, const DataVariant &value) 
{ 
   return mpInputParams->setAttribute(name, value);
}

const DynamicObject *GpuProgramDescriptorImp::getParameters() const
{
   return mpInputParams.get();
}

const DataVariant &GpuProgramDescriptorImp::getParameter(const string &name) const
{
   return mpInputParams->getAttribute(name);
}
