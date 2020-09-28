/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#ifndef OSSIMVERSION_H
#define OSSIMVERSION_H

/// Define an integer preprocessor OSSIM_VERSION_NUMBER 

#include <ossim/ossimConfig.h>
// Ossim 1.6.6 defines VERSIONs in ossimConfig.h, and doesn't have ossimVersion.h
// Ossim 1.9.0 defines VERSIONs in ossimVersion.h
#ifndef OSSIM_MAJOR_VERSION_NUMBER
#include <ossim/ossimVersion.h>
#endif

#ifndef OSSIM_VERSION_NUMBER
#define OSSIM_VERSION_NUMBER (100*100*(OSSIM_MAJOR_VERSION_NUMBER) + 100*(OSSIM_MINOR_VERSION_NUMBER) + (OSSIM_PATCH_VERSION_NUMBER))
#endif

#endif
