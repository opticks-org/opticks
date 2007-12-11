/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "GpuProgramDescriptor.h"
#include "GpuProgramDescriptorImp.h"
#include "ImageFilterDescriptorImp.h"

#include <algorithm>
#include <boost/bind.hpp>

using namespace std;

ImageFilterDescriptorImp::ImageFilterDescriptorImp() :
   mName(""),
   mDescription(""),
   mType(ImageFilterDescriptor::GPU_PROCESS)
{
}

ImageFilterDescriptorImp::ImageFilterDescriptorImp(const ImageFilterDescriptorImp &filterDescriptor)
{
   mName = filterDescriptor.mName;
   mDescription = filterDescriptor.mDescription;
   mType = filterDescriptor.mType;
   mGpuPrograms.resize(filterDescriptor.mGpuPrograms.size());
   transform(filterDescriptor.mGpuPrograms.begin(), filterDescriptor.mGpuPrograms.end(), mGpuPrograms.begin(),
      boost::bind(&GpuProgramDescriptor::copy, _1));
}

ImageFilterDescriptorImp::~ImageFilterDescriptorImp()
{
   clearImagePrograms();
}

void ImageFilterDescriptorImp::setName(const string &name) 
{ 
   mName = name;
}

const string& ImageFilterDescriptorImp::getName() const
{
   return mName;
}

void ImageFilterDescriptorImp::setDescription(const string &description) 
{ 
   mDescription = description;
}

const string& ImageFilterDescriptorImp::getDescription() const
{
   return mDescription;
}

void ImageFilterDescriptorImp::setType(ImageFilterDescriptor::ImageProcessType type)
{ 
   mType = type;
}

ImageFilterDescriptor::ImageProcessType ImageFilterDescriptorImp::getType() const
{
   return mType;
}

void ImageFilterDescriptorImp::clearImagePrograms()
{
   for(vector<GpuProgramDescriptor*>::iterator it = mGpuPrograms.begin(); it != mGpuPrograms.end(); ++it)
   {
      delete dynamic_cast<GpuProgramDescriptorImp*>(*it);
   }
   mGpuPrograms.clear();
}

void ImageFilterDescriptorImp::clear()
{
   mName.clear();
   mDescription.clear();
   mType = ImageFilterDescriptor::GPU_PROCESS;
   clearImagePrograms();
}

const vector<GpuProgramDescriptor*>& ImageFilterDescriptorImp::getGpuPrograms() const
{
   return mGpuPrograms;
}

bool ImageFilterDescriptorImp::addGpuProgram(GpuProgramDescriptor *pGpuDescriptor)
{ 
   VERIFY(pGpuDescriptor != NULL);
   if (pGpuDescriptor->getName().empty())
   {
      return false;
   }

   // insert Gpu program descriptor into vector if it can be compiled

   mGpuPrograms.push_back(pGpuDescriptor);
   return true;
}

void ImageFilterDescriptorImp::removeGpuProgram(const GpuProgramDescriptor& gpuDescriptor)
{
   string programName = gpuDescriptor.getName();
   for(vector<GpuProgramDescriptor*>::iterator gpuDescriptorIter = mGpuPrograms.begin();
         gpuDescriptorIter != mGpuPrograms.end(); ++gpuDescriptorIter)
   {
      if ((*gpuDescriptorIter)->getName() == programName)
      {
         delete dynamic_cast<GpuProgramDescriptorImp*>(*gpuDescriptorIter);
         mGpuPrograms.erase(gpuDescriptorIter);
         break;
      }
   }
}

bool ImageFilterDescriptorImp::hasGpuProgram(const GpuProgramDescriptor& gpuDescriptor) const
{
   string programName = gpuDescriptor.getName();
   for(vector<GpuProgramDescriptor*>::const_iterator gpuDescriptorIter = mGpuPrograms.begin();
         gpuDescriptorIter != mGpuPrograms.end(); ++gpuDescriptorIter)
   {
      if ((*gpuDescriptorIter)->getName() == programName)
      {
         return true;
      }
   }

   return false;
}
