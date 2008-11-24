/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "DynamicModuleImp.h"
#include "MessageLogResource.h"

#if defined(WIN_API)
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(SOLARIS)
#include <dlfcn.h>
#else
#error "Unsupported platform"
#endif

#include <errno.h>
#include <string.h>
#include <string>

DynamicModuleImp::DynamicModuleImp() :
   mpLibHandle(0)
{
}

DynamicModuleImp::DynamicModuleImp(const char* moduleName) :
   mpLibHandle(0)
{
   load(moduleName);
}

DynamicModuleImp::~DynamicModuleImp()
{
   unload();
}

bool DynamicModuleImp::load(const char* moduleName)
{
   //  Load the Windows Dynamic Link Library or the UNIX the Dynamic Shared Object.
   if (moduleName != NULL && ::strlen(moduleName) > 0)
   {
      if (mpLibHandle == NULL)
      {
#if defined(WIN_API)
         ::GetLastError();
         mpLibHandle = (void*) ::LoadLibrary(moduleName);
#elif defined(SOLARIS)
         mpLibHandle = ::dlopen(moduleName, RTLD_NOW | RTLD_GROUP);
#else
#error "Unsupported platform"
#endif
      }
   }

   if (mpLibHandle == NULL)
   {
#if defined(WIN_API)
      DWORD error = ::GetLastError();
      LPVOID msg(0);
      DWORD cnt = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
         FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0, NULL);
      if (cnt > 0)
      {
         std::string errorMessage("Error opening DLL in PlugIns folder: ");
         errorMessage += (TCHAR*)msg;
         MessageResource message("PlugInError", "app", "9EB8716F-6B0F-4716-8E7F-89CB97590B5D");
         message->addProperty("message", errorMessage);
         message->addProperty("filename", moduleName);
         message->finalize();
         LocalFree(msg);
      }
#elif defined(SOLARIS)
      std::string errorMessage("Error opening dynamic shared object in the PlugIns folder:\n");
      errorMessage += dlerror();
      MessageResource message("PlugInError", "app", "9EB8716F-6B0F-4716-8E7F-89CB97590B5D");
      message->addProperty("message", errorMessage);
      message->addProperty("filename", moduleName);
      message->finalize();
#else
#error "Unsupported platform"
#endif
      return false;
   }

   return true;
}

bool DynamicModuleImp::unload()
{
   if (mpLibHandle != NULL)
   {
#if defined(WIN_API)
      ::FreeLibrary((HMODULE) mpLibHandle);
#elif defined(SOLARIS)
      ::dlclose(mpLibHandle);
#else
#error "Unsupported platform"
#endif
   }

   mpLibHandle = NULL;
   return true;
}

bool DynamicModuleImp::isLoaded() const
{
   return (mpLibHandle != 0);
}

DMPROC DynamicModuleImp::getProcedureAddress(const char* procName) const
{
   DMPROC proc = NULL;

   if (mpLibHandle != NULL)
   {
#if defined(WIN_API)
      proc = (DMPROC) ::GetProcAddress((HMODULE) mpLibHandle, procName);
      DWORD temp = GetLastError();

#elif defined(SOLARIS)
      proc = (DMPROC) ::dlsym(mpLibHandle, procName);
#else
#error "Unsupported platform"
#endif
   }

   return proc;
}

#if defined(WIN_API)
DMPROC DynamicModuleImp::getProcedureAddress(int ordinal) const
{
   DMPROC proc = NULL;
   if (mpLibHandle != NULL)
   {
      proc = (DMPROC)::GetProcAddress((HMODULE) mpLibHandle, MAKEINTRESOURCE(ordinal));
   }

   return proc;
}
#endif
