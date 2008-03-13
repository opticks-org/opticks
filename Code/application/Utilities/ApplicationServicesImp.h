/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef APPLICATIONSERVICESIMP_H
#define APPLICATIONSERVICESIMP_H

#include "ApplicationServices.h"
#include "SubjectImp.h"

class ApplicationServicesImp : public ApplicationServices, public SubjectImp
{
public:
   static ApplicationServicesImp* instance();
   static void destroy();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   bool attach(const std::string& signal, const Slot& slot);
   bool detach(const std::string& signal, const Slot& slot);

   void setBatch();
   void setInteractive();
   bool isBatch() const;
   bool isInteractive() const;

   ConfigurationSettings* getConfigurationSettings();
   DataVariantFactory* getDataVariantFactory();
   ObjectFactory* getObjectFactory();
   SessionManager* getSessionManager();

   virtual bool getJvm(JavaVM *&pJvm, JNIEnv *&pEnv);

protected:
   ApplicationServicesImp();
   virtual ~ApplicationServicesImp();

private:
   static ApplicationServicesImp* spInstance;
   bool mbBatch;
   static bool mDestroyed;
};

#endif
