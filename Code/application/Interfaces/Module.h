/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODULE_H
#define MODULE_H

#include "AppConfig.h"

class External;
class PlugIn;

/**
 * \cond INTERNAL
 */
extern "C" int LINKAGE opticks_get_module_interface_version();
/// \endcond

/**
 *  \cond INTERNAL
 *  Gets descriptive information about the module.
 *
 *  This method returns the module name, version, description, total
 *  number of plug-ins within the module, and the module validation 
 *  confirmation string.
 *
 *  @param   name
 *           Returns the name of the module.
 *  @param   version
 *           Returns the version of the module.
 *  @param   description
 *           Returns a description of the use and purpose of the module.
 *  @param   totalPlugIns
 *           Returns the number of plug-ins within the module.
 *  @param   validationKey
 *           Returns a key that the Studio can check to see if the plug-in
 *           is a validated plug-in.  Validation allows users to discriminate
 *           between plug-ins in the R&D stage, custom plug-ins, and fully tested
 *           and validated plug-ins.
 *  @param   pModuleId
 *           Returns a unique identifier for this module.
 *
 *  @return  This method returns true if completed successfully.
 */
extern "C" bool LINKAGE get_name( char **name, 
                                    char **version, 
                                    char **description,
                                    unsigned int *totalPlugIns,
                                    char **validationKey,
                                    char **pModuleId);
/// \endcond

/**
 *  \cond INTERNAL
 *  Initializes the module.
 *
 *  This method initializes the Module.  Initialization includes
 *  instantiating the ModuleManager, setting the reference to the
 *  main application's services interface.
 *
 *  @param   services
 *           Gives the module a reference to the main application's
 *           External services interface.
 *
 *  @return  This method returns true if completed successfully.
 */
extern "C" bool LINKAGE initialize( External *services );
/// \endcond

/**
 *  \cond INTERNAL
 *  Instantiates the given plug-in.
 *
 *  This method instantiates the given plug-in and returns the reference
 *  to the new plug-in.
 *
 *  @param   plugInNumber
 *           Plug-in to access within the module.
 *  @param   interfaceAddress
 *           NULL or reference to interface.
 *
 *  @return  This method returns true if completed successfully.
 */
extern "C" bool LINKAGE instantiate_interface(unsigned int plugInNumber, PlugIn** interfaceAddress);
/// \endcond

/**
 *  \cond INTERNAL
 *  Destroys all plug-ins within the module.
 *
 *  This method tells the module to delete all plug-ins within 
 *  it.
 *
 *  @return  This method returns true if completed successfully.
 */
extern "C" bool LINKAGE destroy();
/// \endcond

#endif
