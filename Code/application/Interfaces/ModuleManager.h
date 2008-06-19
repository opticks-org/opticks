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

#include "External.h"

class PlugIn;

/**
 *  Plug-in management within a module
 *
 *  The module manager is used to inform the main application about the
 *  plug-ins within the module.  It is also used to create, destroy, and 
 *  provide access to the plug-ins.  Plug-in developers edit this class to 
 *  build a plug-in module composed of their plug-ins. This is a singleton 
 *  class, where only one instance of this class exists at a given time.  Use 
 *  the instance() method to get a reference to the class.
 */
class ModuleManager
{
protected:
   /**
    *  Constructor which can NOT be called outside this class.
    *
    *  The default constructor is protected to control the number of
    *  instances of this class.  Use the instance() method to obtain
    *  access to the class.
    */
   ModuleManager();

   /**
    *  Destructor which can not be invoked outside this class.
    *
    *  This destructor deletes all dynamic memory associated with the
    *  class.
    */
   ~ModuleManager();

public:
   /**
    *  Returns the instance of this singleton class.
    *
    *  This method controls the instantiation of this class and
    *  returns a reference to the singleton instance.  If the class
    *  has not been instantiated, it creates the instance, stores
    *  it internally, and then returns a reference to the new 
    *  reference.
    *
    *  @return  This method returns the singleton class instance.
    */
   static ModuleManager* instance();

   /**
    *  Gets the name of the module.
    *
    *  This method is used to get the name of the module.
    *
    *  @return  This method returns the string name of the module.
    */
   static char *getName()
   {
      return (char*)mspName;
   }

   /**
    *  Gets the version of the module.
    *
    *  This method is used to get the version of the module.  The
    *  version is a string in the following format: "1.2.3".
    *
    *  @return  This method returns the string version of the module.
    */
   static char *getVersion()
   {
      return (char*)mspVersion;
   }

   /**
    *  Gets the description of the module.
    *
    *  This method is used to get a string describing the purpose
    *  and operation of the module.
    *
    *  @return  A string containing the description of the module.
    */
   static char *getDescription()
   {
      return (char*)mspDescription;
   }

   /**
    *  Returns the number of plug-ins within the module.
    *
    *  This method may be implemented to allow for a dynamic number
    *  of plug-ins.  This may be useful for a plug-in suite which can
    *  be configured based on data files present.  Each time this method
    *  is called, it may return a different number.
    *
    *  @return  The number of plug-ins within the module.
    */
   static unsigned int getTotalPlugIns();

   static char *getValidationKey()
   {
      return (char*)mspValidationKey;
   }

   /**
    *  Sets the reference to the main application External services.
    *
    *  This method sets the reference to the services class provided
    *  by the main application.  The services class can be used to
    *  query the main application for services that it provides and
    *  get access to those services.
    *
    *  @param   address
    *           The External services class to set.
    */
   void setService( External* address )
   {
      mpServices = address;
   }

   /**
    *  Gets the reference to the main application External services.
    *
    *  This method gets the reference to the services class provided
    *  by the main application.  The services class can be used to
    *  query the main application for services that it provides and
    *  get access to those services.
    *
    *  @return  Returns a reference to the main application External
    *           services.
    *
    *  @see     External
    */
   External* getService()
   {
      return mpServices;
   }

   /** 
    *  Creates an instance of a plug-in.
    *
    *  This method is used to create an instance of a Plug-In.  PLUG-IN
    *  DEVELOPERS SHOULD EDIT THIS CLASS TO ADD INSTANCE CREATION FOR THEIR
    *  PLUG-IN.
    *
    *  Plug-in developers are responsible for ensuring a consistent ordering
    *  to plug-ins for each time the module is queried for its plug-in count.
    *  If this module has dynamically defined plug-ins, the mapping of plug-in
    *  number to plug-in must remain constant until getTotalPlugIns() is called

    *
    *  @param   plugInNumber
    *           The plug-in to create within the module.  The number of the
    *           first plug-in in the module is zero.
    *
    *  @return  This method returns a pointer to the new plug-in.
    *
    */
   PlugIn* getPlugIn( unsigned int plugInNumber );

   /**
    *  Returns the UUID for the module.
    *
    *  This method returns a unique id for the module. This value must be the
    *  same between instances of the application. It should be formatted as a
    *  UUID.
    *
    *  @return This method returns a module's unique id.
    */
   static char *getUniqueId()
   {
      return const_cast<char*>(mspUniqueId);
   }

private:
   static const char*    mspName;
   static const char*    mspVersion;
   static const char*    mspDescription;
   static const char*    mspValidationKey;
   static const char*    mspUniqueId;

   External*             mpServices;

   static ModuleManager* mspSingleton;
};

#endif
