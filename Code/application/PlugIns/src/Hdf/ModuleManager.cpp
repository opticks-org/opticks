/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 
#include "PlugInRegistration.h"

REGISTER_MODULE(OpticksHdf);

// required to include by Hdf5Pager.h
#include <hdf5.h>

// required to include by Hdf4Pager.h
#include <hdf.h>

#include "Hdf4Pager.h"
#include "Hdf5Pager.h"

REGISTER_PLUGIN_BASIC(OpticksHdf, Hdf5Pager);
REGISTER_PLUGIN_BASIC(OpticksHdf, Hdf4Pager);
