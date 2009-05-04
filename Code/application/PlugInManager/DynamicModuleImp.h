/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DYNAMICMODULEIMP_H
#define DYNAMICMODULEIMP_H

#include "AppConfig.h"
#include "DynamicModule.h"

typedef void(*DMPROC)();

class DynamicModuleImp : public DynamicModule
{
public:
   DynamicModuleImp();
   DynamicModuleImp(const std::string& moduleName);
   virtual ~DynamicModuleImp();

   virtual bool load(const std::string& moduleName);
   bool unload();
   bool isLoaded() const;
   DMPROC getProcedureAddress(const std::string& procName) const;

#if defined(WIN_API)
   DMPROC getProcedureAddress(int ordinal) const;
#endif

private:
   void* mpLibHandle;
};

#endif
