/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataDescriptorImp.h"
#include "ImportDescriptorImp.h"

ImportDescriptorImp::ImportDescriptorImp(DataDescriptor* pDescriptor) :
   mpDescriptor(pDescriptor),
   mImported(true)
{
}

ImportDescriptorImp::~ImportDescriptorImp()
{
   setDataDescriptor(NULL);
}

void ImportDescriptorImp::setDataDescriptor(DataDescriptor* pDescriptor)
{
   if (pDescriptor != mpDescriptor)
   {
      DataDescriptorImp* pDescriptorImp = dynamic_cast<DataDescriptorImp*>(mpDescriptor);
      if (pDescriptorImp != NULL)
      {
         delete pDescriptorImp;
      }

      mpDescriptor = pDescriptor;
   }
}

DataDescriptor* ImportDescriptorImp::getDataDescriptor() const
{
   return mpDescriptor;
}

void ImportDescriptorImp::setImported(bool bImport)
{
   mImported = bImport;
}

bool ImportDescriptorImp::isImported() const
{
   return mImported;
}
