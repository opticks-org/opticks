/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COREMODULEDESCRIPTOR_H
#define COREMODULEDESCRIPTOR_H 

#include "ModuleDescriptor.h"

class CoreModuleDescriptor : public ModuleDescriptor
{
public:
   CoreModuleDescriptor(const std::string& id, std::map<std::string, std::string> &plugInIds);
   ~CoreModuleDescriptor();

   virtual bool load();
   virtual void unload();
   virtual PlugIn* createInterface(unsigned int plugInNumber);
   virtual PlugIn* createInterface(PlugInDescriptorImp* plugIn);
   virtual const bool isValidatedModule() const;
   virtual bool isLoaded() const;
};

#endif
