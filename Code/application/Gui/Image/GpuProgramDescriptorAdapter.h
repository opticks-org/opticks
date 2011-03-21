/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GPUPROGRAMDESCRIPTORADAPTER_H
#define GPUPROGRAMDESCRIPTORADAPTER_H

#include "GpuProgramDescriptor.h"
#include "GpuProgramDescriptorImp.h"

class GpuProgramDescriptorAdapter : public GpuProgramDescriptor, public GpuProgramDescriptorImp
{
public:
   GpuProgramDescriptorAdapter() {}
   GpuProgramDescriptorAdapter(const GpuProgramDescriptorAdapter &other) : GpuProgramDescriptorImp(other) {}
   ~GpuProgramDescriptorAdapter() {}

   GPUPROGRAMDESCRIPTORADAPTER_METHODS(GpuProgramDescriptorImp);

   GpuProgramDescriptor *copy() const
   {
      return new GpuProgramDescriptorAdapter(*this);
   }
};

#endif
