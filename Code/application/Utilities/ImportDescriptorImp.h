/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORTDESCRIPTORIMP_H
#define IMPORTDESCRIPTORIMP_H

#include "ImportDescriptor.h"

class DataDescriptor;

class ImportDescriptorImp : public ImportDescriptor
{
public:
   ImportDescriptorImp(DataDescriptor* pDescriptor);
   ~ImportDescriptorImp();

   void setDataDescriptor(DataDescriptor* pDescriptor);
   DataDescriptor* getDataDescriptor() const;

   void setImported(bool bImport);
   bool isImported() const;

private:
   DataDescriptor* mpDescriptor;
   bool mImported;
};

#endif
