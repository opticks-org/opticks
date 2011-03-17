/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include "AppConfig.h"

#if !defined(DEPRECATED_MODULE_TYPE)
#error You must define DEPRECATED_MODULE_TYPE in order to continue using the legacy module type.
#else

#if !(defined(SOLARIS) || defined(WIN_API))
#error Legacy modules (version 1) are not supported on this platform.
#endif

#pragma message(__FILE__ "(" STRING(__LINE__) ") You are using the deprecated method of registering modules and plug-ins.  See REGISTER_MODULE and REGISTER_PLUGIN_BASIC for more information.")
#include "PlugInRegistration.h"

#endif

#endif
