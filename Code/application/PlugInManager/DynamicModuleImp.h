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

/**
 *  Dynamic Module 
 *
 *  Platform independant class that supports dynamic linking for 
 *  Windows Dynamic Link Libraries (DLL's) and Solaris/Linux Dynamic
 *  Shared Objectes (DSO's).
 */
class DynamicModuleImp : public DynamicModule
{
public:
   /**
    *  Default constructor for a Dynamic Module
    *
    *  This constructor creates an empty Dynamic Module .
    *  No library is loaded and only default information is set.
    */
   DynamicModuleImp();

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
   DynamicModuleImp(const char* moduleName);

   /**
    *  Default destructor for a Dynamic Module
    *
    *  This destructor deletes all dynamic memory associated
    *  with the Dynamic Library.  As part of this destructor,
    *  it unloads and deletes the associated Dynamic Link Library
    *  or Dynamic Shared Object.
    */
   virtual ~DynamicModuleImp();

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
   bool load(const char* moduleName);

   /**
    *  Remove the Dynamic Library from memory
    *
    *  The unload() method removes the Dynamic Link Library (DLL) or
    *  Dynamic Shared Object (DSO) from memory.
    *
    *  @return  This method returns true if the library is sucessfully
    *           unloaded from memory.
    */
   bool unload();

   /**
    *  Has the Module been loaded?
    *
    *  The isLoaded() method returns true if the Dynamic Link Library (DLL)
    *  or the Dynamic Shared Object (DSO) has been loaded and is resident 
    *  in memory.
    *
    *  @return  The method returns true if the library has already been loaded.
    */
   bool isLoaded() const;

   /**
    *  Get the address of a library procedure.
    *
    *  The getProcedureAddress() method returns the address of the specified 
    *  exported Dynamic Link Library (DLL) or the Dynamic Shared Object (DSO)
    *  procedure or function.  If the procedure or function does not exist
    *  NULL is returned.
    *
    *  @param   procName
    *           The string name of the exported procedure or function.
    *
    *  @return  This method returns the address of the procedure or 
    *           function, otherwise NULL is returned.
    */
   DMPROC getProcedureAddress(const char* procName) const;

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
    *  @return  This method returns the address of the procedure or 
    *           function, otherwise NULL is returned.
    */
   DMPROC getProcedureAddress(int ordinal) const;
#endif

private:
   void* mpLibHandle;
};

#endif
