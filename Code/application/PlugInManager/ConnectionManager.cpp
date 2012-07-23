/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

/**
 * Connection Manager 
 *
 * This class is responsible for providing all external connections
 * with access to the Model, Desktop, and Plug-In Services.
 * This is a singleton class.  Only one instance of this class exists at
 * a given time.  Use the instance() method to get a reference to the class.
 */

#include "AnimationServicesImp.h"
#include "ApplicationServicesImp.h"
#include "ConnectionManager.h"
#include "DesktopServicesImp.h"
#include "ModelServicesImp.h"
#include "PlugInManagerServicesImp.h"
#include "UtilityServicesImp.h"

#include <string>
using namespace std;

//
// The attribute spInstance is the only allowable reference
// to the ConnectionManager singleton class.
//
ConnectionManager* ConnectionManager::spInstance = NULL;
bool ConnectionManager::mDestroyed = false;

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
*/
ConnectionManager* ConnectionManager::instance() 
{
   if (spInstance == NULL)
   {
      if (mDestroyed)
      {
         throw std::logic_error("Attempting to use ConnectionManager after "
            "destroying it.");
      }
      spInstance = new ConnectionManager;
   }

   return spInstance;
}

void ConnectionManager::destroy()
{
   if (mDestroyed)
   {
      throw std::logic_error("Attempting to destroy ConnectionManager after "
         "destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

/**
*  Maps the connection manager instance.
*
*  This method is used to ensure multiple instances of the services
*  are not created in plug-ins using language extensions.
*
*  @param  manager  The connection manager instance created from the
*                   main applicaiton.
*/
void ConnectionManager::mapConnection(ConnectionManager* pManager)
{
   if (mDestroyed)
   {
      throw std::logic_error("Attempting to use ConnectionManager after destroying it.");
   }

   if ((spInstance != NULL) && (spInstance != pManager))
   {
      delete spInstance;
   }

   spInstance = pManager;
}

/**
*  Query the Main Application Services
*  
*  The queryInterface() method is used to interrogate 
*  the Main Application for services it supports.  The interfaceName
*  input parameter is a string name describing the service. 
*  The interfaceAddress parameter is returned as NULL if the 
*  interface is not supported, otherwise a reference to the 
*  interface is returned.
* 
*  @param interfaceName
*         Service requested
*
*  @param interfaceAddress
*         NULL or reference to service
*
*  @return TRUE is the interface is supported
*/
bool ConnectionManager::queryInterface(const char* interfaceName, void** interfaceAddress)
{
   *interfaceAddress = NULL;

   if (strcmp(interfaceName, "ModelServices2") == 0)
   {
      *interfaceAddress = static_cast<ModelServices*>(ModelServicesImp::instance());
   }

   if (strcmp(interfaceName, "PlugInManagerServices2") == 0)
   {
      *interfaceAddress = static_cast<PlugInManagerServices*>(PlugInManagerServicesImp::instance());
   }

   if (strcmp(interfaceName, "UtilityServices2") == 0)
   {
      *interfaceAddress = static_cast<UtilityServices*>(UtilityServicesImp::instance());
   }

   if (strcmp(interfaceName, "AnimationServices2") == 0)
   {
      *interfaceAddress = static_cast<AnimationServices*>(AnimationServicesImp::instance());
   }

   if (strcmp(interfaceName, "DesktopServices2") == 0)
   {
      *interfaceAddress = static_cast<DesktopServices*>(DesktopServicesImp::instance());
   }

   if (strcmp(interfaceName, "ApplicationServices2") == 0)
   {
      *interfaceAddress = static_cast<ApplicationServices*>(ApplicationServicesImp::instance());
   }

   if (*interfaceAddress != NULL)
   {
      return true;
   }

   return false;
}
