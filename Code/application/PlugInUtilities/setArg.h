/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef SETARG_H
#define SETARG_H

#include "PlugInArgList.h"
#include "PlugInManagerServices.h"

/**
 * Construct arg lists arguments.
 *
 * @param  pPlugInManager
 *         Handle to the plugin manager services objcet used to allocate
 *         new plugin arguments.
 *
 * @param  pArgList
 *         Handle to a PlugInArgList object, to which the new argument is
 *         to be added.
 *
 * @param  name
 *         Identifying key name of the argument to be inserted to the list.
 *
 * @param  type
 *         Data type of the argument being added to the list.
 *
 * @param  defaultValue
 *         Value to be inserted as the default for the argument.
 *
 * @param  actualValue
 *         Value to be used instead of the default, if any.
 */
void setArg(PlugInManagerServices* pPlugInManager,
            PlugInArgList* pArgList,
            const char* name,
            const char* type,
            const void* defaultValue,
            const void* actualValue);

#endif

 
