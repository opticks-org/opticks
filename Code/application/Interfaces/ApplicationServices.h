/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef APPLICATIONSERVICES_H
#define APPLICATIONSERVICES_H

#include "Service.h"
#include "Subject.h"

#include <string>

class ConfigurationSettings;
class DataVariantFactory;
class ObjectFactory;
class SessionManager;

/** we should really include jni.h here but we want to avoid needing Java installed if getJvm() is not used **/
struct JNIEnv_;
typedef JNIEnv_ JNIEnv;
struct JavaVM_;
typedef JavaVM_ JavaVM;

/**
 *  Provides access to application specific interfaces.
 *
 *  This class provides a means for clients to access interfaces of
 *  singleton objects maintained in the Studio.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The application or session is closed.
 *  - Everything else documented in Subject.
 */
class ApplicationServices : public Subject
{
public:
   /**
    *  Emitted when the application is closed.
    *
    *  The application is still fully created when this signal is emitted. Plug-
    *  ins should typically attach to SessionClosed instead.
    */
   SIGNAL_METHOD(ApplicationServices, ApplicationClosed)

   /**
    *  Emitted when the session is closed.
    *
    *  The session is still fully created when this signal is emitted. Plug-ins
    *  should attach to this signal if they need to do cleanup before being
    *  destroyed on session close.
    */
   SIGNAL_METHOD(ApplicationServices, SessionClosed)

   /**
    *  Queries whether the application is executing in batch mode.
    *
    *  @return  Returns \b true if the application is executing in batch mode.
    *           Returns \b false if the application is executing in interactive
    *           mode.
    *
    *  @see     isInteractive()
    */
   virtual bool isBatch() const = 0;

   /**
    *  Queries whether the application is executing in interactive mode.
    *
    *  @return  Returns \b true if the application is executing in interactive
    *           mode.  Returns \b false if the application is executing in
    *           batch mode.
    *
    *  @see     isBatch()
    */
   virtual bool isInteractive() const = 0;

   /**
    *  Get the handle to the configuration settings currently being
    *  employed by the application.
    *
    *  @return   A pointer to an interface for the configuration settings.
    */
   virtual ConfigurationSettings* getConfigurationSettings() = 0;

   /**
    *  Get the handle to the object factory currently being
    *  employed by the application.
    *
    *  @return   A pointer to an interface for the object factory.
    */
   virtual ObjectFactory* getObjectFactory() = 0;

   /**
    *  Get the handle to the DataVariant factory currently being
    *  employed by the application.
    *
    *  @return   A pointer to an interface for the DataVariant factory.
    */
   virtual DataVariantFactory* getDataVariantFactory() = 0;

   /**
    *  Get the handle to the session manager currently being
    *  employed by the application.
    *
    *  @return   A pointer to an interface for the session manager.
    */
   virtual SessionManager* getSessionManager() = 0;

   /**
    *  Get a Java VM
    *
    *  @param   pJvm
    *           return the JVM pointer
    *  @param   pEnv
    *           return the JNI environment pointer
    *
    *  @return  True on success or false on failure
    */
   virtual bool getJvm(JavaVM *&pJvm, JNIEnv *&pEnv) = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~ApplicationServices() {}
};

#endif
