/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
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
 * To create (1) Add the include directories "..\interfaces, ..\libplugin"
 *           (2) Add the library directories "..\libplugin"
 *           (3) Add libplugin.lib to the link directives
 *           (4) Change the name of the DLL to "spXXXX.dll" with XXXX being
 *               the name of the Module.
 * @pkg     PlugIn
 */

#include "testPlugin.h"
#include "ModuleManager.h"
#include "Base.h"
#include "Description.h"

using namespace std;

//
// These static variables are used to describe the Module.  Set 
// these according to how you want the Module configured.  
//
unsigned int ModuleManager::mTotalPlugIns = 1;

const char *ModuleManager::mspName = "CppTests";
const char *ModuleManager::mspVersion = "1.0";
const char *ModuleManager::mspDescription = "CppTests";
const char *ModuleManager::mspValidationKey = "none";
bool ModuleManager::mLicenseRequired = false;

/** 
 *  Create an instance of a Plug-In
 *
 *  This method is used to create an instance of a Plug-In.  The parameter
 *  plugInNumber is used to determine which Plug-In to create.  PLUG-INS 
 *  DEVELOPERS EDIT THIS CLASS TO ADD INSTANCE CREATION FOR THIER PLUG-IN.
 *
 *  @param   plugInNumber
 *           Plug-In to access within the Module.  First Plug-In number
 *           starts with zero.
 *
 *  @return  This method returns reference to the new Plug-In.
 *
 */
Base* ModuleManager::getPlugIn( unsigned int plugInNumber )
{
   Base* plugIn = NULL;

   switch ( plugInNumber )
   {
      case 0:
         plugIn = reinterpret_cast<Base*>( new COMET_tests() );
         break;

      default:
         break;
   }

   return plugIn;
}

/**
 *  Is the licensing information valid?
 *
 *  The validateLicense() method tests the licensing information
 *  and determines if the given licensing information is valid.
 *  The licensing information is in the form of a dynamic 
 *  object.  This allows the license to contain whatever
 *  information is needed by the specific licensing alogorithm
 *  chosen.  Plug-In developers insert code within this method
 *  to test and validate the license key or initialize the licensing
 *  software used.  If a licenseRequired is not set this method is
 *  not called.
 *
 *  @param   license
 *           Specific licensing information for this Module.
 *
 *  @return  The method returns true if the given licensing 
 *           information is valid.
 */
bool ModuleManager::validateLicense( DynamicObject *license )
{
    string key;
    string *data;

    if ( license == NULL ) 
	{
        mLicenseValid = false;
        return mLicenseValid;
    }

    // Get interested licensing information

    data = ( string* ) license->get( "licenseKey", string( "string" ) );

    if ( data == NULL )
	{
        mLicenseValid = false;
        return mLicenseValid;
    }
    // Test for valid license

    mLicenseValid = true;
    return mLicenseValid;
}

