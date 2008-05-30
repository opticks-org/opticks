/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

/**
 * Module Manager 
 *
 * The Module Manager is used to inform the main application about the
 * Plug-Ins within the Module.  It is also used to create, destroy, and provide
 * access to the Plug-Ins.  Plug-In developers edit this class to build
 * a Plug-In Module composed of thier Plug-Ins. This is a singleton class.  
 * Only one instance of this class exists at a given time.  Use the instance() 
 * method to get a reference to the class.
 *
 * This file contains all the common methods for the Module Manager Class.
 * Plug-In Module developers need not edit this file.
 */

#include <stdlib.h>

#include "ModuleManager.h"

//
// The attribute singleton is the only allowable reference
// to the ModuleManager singleton class.
//
ModuleManager* ModuleManager::mspSingleton = NULL;

/**
 *  Returns the instance of this singleton class.
 *
 *  The instance() method controls the instantiation of this class  
 *  and returns a reference to the singleton instance.  If the class 
 *  has not been instantiated, it creates the instance, stores
 *  it internally, and then returns a reference to the new 
 *  reference.
 *
 *  @return  This method returns the singleton class instance.
 *
 *  PLUG-IN DEVELOPERS DO NOT EDIT THIS CLASS.
 */
ModuleManager* ModuleManager::instance() 
{
    if (mspSingleton == NULL)
    {
        mspSingleton = new ModuleManager;
    }
    return mspSingleton;
}

/**
 *  Constructor which can NOT be called outside this class.
 *
 *  The default constructor is protected.  The allows the instance()
 *  method to control the number of instances of this class.  To
 *  instantiate this class the ModuleManager::instance() 
 *  method must be called.
 */
ModuleManager::ModuleManager() :
   mpServices(NULL)
{
}

/**
 * Destructor which can not be invoked outside this class.
 *
 * This destructor deletes all dynamic memory associated with the
 * class.
 */
ModuleManager::~ModuleManager()
{
}
