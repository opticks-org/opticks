/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXTERNAL_H
#define EXTERNAL_H

/**
 * \cond INTERNAL
 *  Primary plug-in interface to the main application
 *
 *  Defines the external abstract interface to the main application.
 *  This interface can be used to gain access to services provided by
 *  the main application.
 */
class External
{
public:
   /**
    *  Get descriptive information about the Module
    *
    *  The queryInterface() method is used to ask the main application
    *  for a reference to a service.  If the service is not supported 
    *  this method returns false.  Otherwise the reference is returned 
    *  as the interfaceAddress parameter.
    * 
    *  @param   interfaceName
    *           Interface requested
    *  @param   interfaceAddress
    *           NULL or reference to interface
    *
    *  @return  TRUE if the interface is supported.
    */
   virtual bool queryInterface( const char *interfaceName, void** interfaceAddress ) = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~External() {}
};
/// \endcond

#endif
