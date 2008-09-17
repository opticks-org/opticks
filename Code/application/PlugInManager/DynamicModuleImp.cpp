/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

/**
 *  Dynamic Module 
 *
 *  Platform independant class that supports dynamic linking for 
 *  Windows Dynamic Link Libraries (DLL's) and Solaris/Linux Dynamic
 *  Shared Objectes (DSO's).
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

/**
 *  Main constructor for a Dynamic Module
 *
 *  This constructor creates a Dynamic Module for a Dynamic
 *  Library associated with the given filename parameter and the
 *  Dynamic Library is loaded.
 *
 *  @param   moduleName
 *           Full path name for the Dynamic Library.
 */
DynamicModuleImp::DynamicModuleImp( const char *name )
    : libHandle(0)
{
    load( name );
}

/**
 *  Default destructor for a Dynamic Module
 *
 *  This destructor deletes all dynamic memory associated
 *  with the Dynamic Library.  As part of this destructor,
 *  it unloads and deletes the associated Dynamic Link Library
 *  or Dynamic Shared Object.
 */
DynamicModuleImp::~DynamicModuleImp( void )
{
    unload();
}

/**
 *  Loads the Dynamic Library
 *
 *  The load() method loads the Dynamic Link Library (DLL) or
 *  Dynamic Shared Object (DSO) into memory.
 *
 *  @param   moduleName
 *           Full path name for the Dynamic Library.
 *  @return  This method returns true if the library is sucessfully
 *           loaded.
 */
bool DynamicModuleImp::load( const char *name )
{
    //
    //  Load the Windows Dynamic Link Library or 
    //  the UNIX the Dynamic Shared Object.
    //

    if (name != NULL && ::strlen(name) > 0)
    {
        if (libHandle == NULL)
        {
#if defined(WIN_API)
           ::GetLastError();
            libHandle = (void*) ::LoadLibrary( name );
#elif defined(SOLARIS)
            libHandle = ::dlopen( name, RTLD_NOW | RTLD_GROUP );
#else

#endif
        }
    }

    if (libHandle == NULL) 
    {
#if defined(WIN_API)
         DWORD error = ::GetLastError();
         LPVOID msg(0);
         DWORD cnt = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                     FORMAT_MESSAGE_FROM_SYSTEM |
                                     FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL,
                                     error,
                                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                     (LPTSTR)&msg,
                                     0,
                                     NULL);
         if(cnt > 0)
         {
            std::string errorMessage("Error opening DLL in PlugIns folder: ");
            errorMessage += (TCHAR*)msg;
            MessageResource message("PlugInError", "app", "9EB8716F-6B0F-4716-8E7F-89CB97590B5D");
            message->addProperty("message", errorMessage);
            message->addProperty("filename", name);
            message->finalize();
            LocalFree(msg);
         }
#elif defined(SOLARIS)
         std::string errorMessage("Error opening dynamic shared object in the PlugIns folder:\n");
         errorMessage += dlerror();
         MessageResource message("PlugInError", "app", "9EB8716F-6B0F-4716-8E7F-89CB97590B5D");
         message->addProperty("message", errorMessage);
         message->addProperty("filename", name);
         message->finalize();
#else
#error "Unsupported platform"
#endif
       return false;
    }
    return true;
}

/**
 *  Remove the Dynamic Library from memory
 *
 *  The unload() method removes the Dynamic Link Library (DLL) or
 *  Dynamic Shared Object (DSO) from memory.
 *
 *  @return  This method returns true if the library is sucessfully
 *           unloaded from memory.
 */
bool DynamicModuleImp::unload( void )
{
    if (libHandle != NULL)
    {
#if defined(WIN_API)
        ::FreeLibrary( (HMODULE) libHandle );
#elif defined(SOLARIS)
        ::dlclose( libHandle );
#else
#error "Unsupported platform"
#endif
    }

    libHandle = NULL;

    return true;
}

/**
 *  Get the address of a library procedure.
 *
 *  The getProcedureAddress() method returns the address of the specified 
 *  exported Dynamic Link Library (DLL) or the Dynamic Shared Object (DSO)
 *  procedure or function.  If the procedure or function does not exist
 *  NULL is returned.
 *
 *  @param   name
 *           The string name of the exported procedure or function.
 *
 *  @return  This method returns the address of the procedure or 
 *           function, otherwise NULL is returned.
 */
DMPROC DynamicModuleImp::getProcedureAddress( const char *procName ) const
{
    DMPROC proc = NULL;

    if (libHandle != NULL)
    {
#if defined(WIN_API)
        proc = (DMPROC) ::GetProcAddress((HMODULE) libHandle, procName);
        DWORD temp = GetLastError();
 
#elif defined(SOLARIS)
        proc = (DMPROC) ::dlsym(libHandle, procName);
#else
#error "Unsupported platform"
#endif
    }
    
    return proc;
}



#if defined(WIN_API)
/**
 *  Get the address of a library procedure given the ordinal number.
 *
 *  The getProcedureAddress() method returns the address of the specified 
 *  exported Dynamic Link Library (DLL) or the Dynamic Shared Object (DSO)
 *  procedure or function.  If the procedure or function does not exist
 *  NULL is returned.  This is a Windows specific call.  An ordinal number
 *  is defined for each exported procedure and function.  This method 
 *  is somewhat faster than the getProcedureAddress( name ) method.
 *
 *  @param   ordinal
 *           The ordinal number of the exported procedure or function.
 *
 *  @return  This method returns the address of the procedure or 
 *           function, otherwise NULL is returned.
 */
DMPROC DynamicModuleImp::getProcedureAddress(int ordinal) const
{
    DMPROC proc = NULL;

    if (libHandle != NULL)
    {
        proc = (DMPROC)::GetProcAddress((HMODULE) libHandle, MAKEINTRESOURCE(ordinal));
    }

    return proc;
}

#endif

 
