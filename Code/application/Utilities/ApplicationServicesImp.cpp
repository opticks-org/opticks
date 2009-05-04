/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationServicesImp.h"
#include "AppConfig.h"
#include "ConfigurationSettingsImp.h"
#include "DataVariant.h"
#include "DataVariantFactoryImp.h"
#include "ObjectFactoryImp.h"
#include "SessionManagerImp.h"

#if defined(WIN_API)
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(UNIX_API)
#include <dlfcn.h>
#include <link.h>
#else
#error "Platform not supported."
#endif

using namespace std;

ApplicationServicesImp* ApplicationServicesImp::spInstance = NULL;
bool ApplicationServicesImp::mDestroyed = false;

ApplicationServicesImp* ApplicationServicesImp::instance()
{
   if (spInstance == NULL)
   {
      if (mDestroyed)
      {
         throw std::logic_error("Attempting to use ApplicationServices after "
            "destroying it.");
      }
      spInstance = new ApplicationServicesImp;
   }

   return spInstance;
}

void ApplicationServicesImp::destroy()
{
   if (mDestroyed)
   {
      throw std::logic_error("Attempting to destroy ApplicationServices after "
         "destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

ApplicationServicesImp::ApplicationServicesImp() :
   mbBatch(true)
{
}

ApplicationServicesImp::~ApplicationServicesImp()
{
}

const string& ApplicationServicesImp::getObjectType() const
{
   static string sType("ApplicationServicesImp");
   return sType;
}

bool ApplicationServicesImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ApplicationServices"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void ApplicationServicesImp::setBatch()
{
   mbBatch = true;
}

void ApplicationServicesImp::setInteractive()
{
   mbBatch = false;
}

bool ApplicationServicesImp::isBatch() const
{
   return mbBatch;
}

bool ApplicationServicesImp::isInteractive() const
{
   return !mbBatch;
}

ConfigurationSettings* ApplicationServicesImp::getConfigurationSettings()
{
   return ConfigurationSettingsImp::instance();
}

ObjectFactory* ApplicationServicesImp::getObjectFactory()
{
   return ObjectFactoryImp::instance();
}

DataVariantFactory* ApplicationServicesImp::getDataVariantFactory()
{
   return DataVariantFactoryImp::instance();
}

SessionManager* ApplicationServicesImp::getSessionManager()
{
   static bool sAttached = false;

   SessionManager* pSession = SessionManagerImp::instance();
   if (!sAttached)
   {
      pSession->attach(SIGNAL_NAME(SessionManager, Closed), 
         Signal(Service<ApplicationServices>().get(), SIGNAL_NAME(ApplicationServices, SessionClosed)));
      sAttached = true;
   }

   return pSession;
}

bool ApplicationServicesImp::getJvm(JavaVM *&pJvm, JNIEnv *&pEnv)
{
   const Filename* pJvmLoaderLibrary = ConfigurationSettings::getSettingJvmLoaderLibraryPath();
   if (pJvmLoaderLibrary == NULL)
   {
      return false;
   }
   string jvmLoaderLibrary = pJvmLoaderLibrary->getFullPathAndName();

#if defined(SOLARIS) || defined(LINUX)
   void* pJvmLoaderLibraryHandle(dlopen(jvmLoaderLibrary.c_str(), RTLD_NOW | RTLD_GLOBAL));
   if (pJvmLoaderLibraryHandle == NULL)
   {
      return false;
   }

   void *pGetJvm(dlsym(pJvmLoaderLibraryHandle, "getJvm"));
   if (pGetJvm == NULL)
   {
      dlclose(pJvmLoaderLibraryHandle);
      return false;
   }

   bool rval(((bool (*)(JavaVM*&, JNIEnv*&))pGetJvm)(pJvm, pEnv));

   return rval;
#elif defined(WIN_API)
   HMODULE pJvmLoaderLibraryHandle(LoadLibrary(jvmLoaderLibrary.c_str()));
   if (pJvmLoaderLibraryHandle == NULL)
   {
      return false;
   }

   FARPROC pGetJvm(GetProcAddress(pJvmLoaderLibraryHandle, "getJvm"));
   if (pGetJvm == NULL)
   {
      FreeLibrary(pJvmLoaderLibraryHandle);
      return false;
   }

   bool rval(((bool (*)(JavaVM*&, JNIEnv*&))pGetJvm)(pJvm, pEnv));

   return rval;
#else
#error "Platform not supported."
#endif
}
