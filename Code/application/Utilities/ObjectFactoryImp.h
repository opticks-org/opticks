/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef __OBJCTFCTIMP_H
#define __OBJCTFCTIMP_H

#include "ObjectFactory.h"

#include <string>

class ObjectFactoryImp : public ObjectFactory
{
public:
   static ObjectFactoryImp* instance();
   static void destroy();

   virtual void* createObject(const std::string& className);
   virtual void destroyObject(void* pObject, const std::string& className);
   virtual void* createObjectVector(const std::string& className);
   virtual void destroyObjectVector(void* pVector, const std::string& className);

protected:
   ObjectFactoryImp() {};
   virtual ~ObjectFactoryImp() {};

   /**
   * Determines if the className is a vector and parses the template argument out
   * @param className
   *        Type to check for "vector<int>"-formed types.
   * @param vectorClass
   *        Changed to hold the parsed template argument, if a vector
   *        (ie, "int", from above).  No change otherwise
   * @return True if the className is a vector
   */
   bool parseClassString(const std::string& className, std::string& vectorClass);

private:
   static ObjectFactoryImp* spInstance;
   static bool mDestroyed;
};

#endif   // __OBJCTFCTIMP_H
