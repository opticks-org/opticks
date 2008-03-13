/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILEDESCRIPTORADAPTER_H
#define FILEDESCRIPTORADAPTER_H

#include "FileDescriptor.h"
#include "FileDescriptorImp.h"

class FileDescriptorAdapter : public FileDescriptor, public FileDescriptorImp
{
public:
   FileDescriptorAdapter();
   ~FileDescriptorAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // FileDescriptor
   FileDescriptor* copy() const;

   FILEDESCRIPTORADAPTER_METHODS(FileDescriptorImp)
};

#endif
