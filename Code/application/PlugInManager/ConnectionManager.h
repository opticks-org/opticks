/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <string>
#include "External.h"

/**
 * Connection Manager 
 *
 * This class is responsible for providing all external connections
 * with access to the Model, Desktop, and Plug-In Services.
 * This is a singleton class.  Only one instance of this class exists at
 * a given time.  Use the instance() method to get a reference to the class.
 */
class ConnectionManager : public External
{
protected:
   /**
    *  Constructor which can NOT be called outside this class.
    *
    *  The default constructor is protected.  The allows the instance()
    *  method to control the number of instances of this class.  To
    *  instantiate this class the ConnectionManager::instance() 
    *  method must be called.
    */
   ConnectionManager() {};

   /**
    * Destructor which can not be invoked outside this class.
    *
    * This destructor deletes all dynamic memory associated with the
    * class.
    */
   ~ConnectionManager() {};

public:
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
   static ConnectionManager* instance();
   static void destroy();

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
   bool queryInterface( const char *interfaceName,
                         void** interfaceAddress );
   /**
    *  Maps the connection manager instance.
    *
    *  Sets the singleton address so that there is only one Connection 
    *  Manager instance between the main application and the plug-ins.
    *
    *  @param  manager  The connection manager instance created from the
    *                   main applicaiton.
    */
   static void mapConnection( ConnectionManager* pManager );

private:
   static ConnectionManager* spInstance;
   static bool mDestroyed;
};

#endif
