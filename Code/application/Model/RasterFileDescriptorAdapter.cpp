/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "RasterFileDescriptorAdapter.h"

using namespace std;

RasterFileDescriptorAdapter::RasterFileDescriptorAdapter()
{
}

RasterFileDescriptorAdapter::~RasterFileDescriptorAdapter()
{
}

// TypeAwareObject
const string& RasterFileDescriptorAdapter::getObjectType() const
{
   static string type("RasterFileDescriptorAdapter");
   return type;
}

bool RasterFileDescriptorAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RasterFileDescriptor"))
   {
      return true;
   }

   return RasterFileDescriptorImp::isKindOf(className);
}

// FileDescriptor
FileDescriptor* RasterFileDescriptorAdapter::copy() const
{
   RasterFileDescriptorAdapter* pDescriptor = new RasterFileDescriptorAdapter();
   if (pDescriptor != NULL)
   {
      *pDescriptor = *this;
   }

   return pDescriptor;
}
