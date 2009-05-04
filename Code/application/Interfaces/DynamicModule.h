/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DYNAMICMODULE_H
#define DYNAMICMODULE_H

#include <string>

typedef void(*DMPROC)();

/**
 *  Dynamic %Module
 *
 *  Platform independant class that supports dynamic linking for 
 *  Windows Dynamic Link Libraries (DLL's) and Solaris/Linux Dynamic
 *  Shared Objects (DSO's).
 */
class DynamicModule
{
public:
   /**
    *  Loads the Dynamic Library
    *
    *  This method loads the Dynamic Link Library (DLL) or
    *  Dynamic Shared Object (DSO) into memory.
    *
    *  @param   moduleName
    *           Full path name for the dynamic library.
    *
    *  @return  This method returns true if the library is sucessfully
    *           loaded.
    */
   virtual bool load(const std::string& moduleName) = 0;

   /**
    *  Remove the dynamic library from memory
    *
    *  This method removes the Dynamic Link Library (DLL) or
    *  Dynamic Shared Object (DSO) from memory.
    *
    *  @return  This method returns true if the library is sucessfully
    *           unloaded from memory.
    */
   virtual bool unload() = 0;

   /**
    *  Has the module been loaded?
    *
    *  This method returns true if the Dynamic Link Library (DLL)
    *  or the Dynamic Shared Object (DSO) has been loaded and is resident 
    *  in memory.
    *
    *  @return  The method returns true if the library has already been loaded.
    */
   virtual bool isLoaded() const = 0;

   /**
    *  Gets the address of a library procedure.
    *
    *  This method returns the address of the specified  exported
    *  Dynamic Link Library (DLL) or the Dynamic Shared Object (DSO)
    *  procedure or function.  If the procedure or function does not
    *  exist, NULL is returned.
    *
    *  @param   name
    *           The string name of the exported procedure or function.
    *
    *  @return  This method returns the address of the procedure or
    *           function, otherwise NULL is returned.
    */
   virtual DMPROC getProcedureAddress(const std::string& name) const = 0;

protected:
   /**
    * This should be destroyed by calling PlugInManagerServices::destroyDynamicModule.
    */
   virtual ~DynamicModule() {}
};

#endif   // DYNAMICMODULE_H
