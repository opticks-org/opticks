/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATA_VARIANT_FACTORY_IMP_H
#define DATA_VARIANT_FACTORY_IMP_H

#include "DataVariantFactory.h"

#include <map>
#include <string>

class DataValueWrapper;

class DataVariantFactoryImp : public DataVariantFactory
{
public:
   static DataVariantFactoryImp *instance();
   static void destroy();

   DataValueWrapper* createWrapper(const void *pObject, const std::string &className, bool strict = true);
   DataValueWrapper* createWrapper(DOMNode *pDocument, int version);

protected:
   DataVariantFactoryImp()
   {
      initializeMaps();
   }
   virtual ~DataVariantFactoryImp();

private:   
   typedef DataValueWrapper*(*WrapperCreatorProc)(const void*);
   typedef std::map<std::string,WrapperCreatorProc> ObjectMapType3;

   void registerType(const std::string type, WrapperCreatorProc creationFunc);

   template<class DataValueWrapperImp>
   void registerType()
   {
      std::vector<std::string> typesToRegister = DataValueWrapperImp::getSupportedTypes();
      for (std::vector<std::string>::const_iterator iter = typesToRegister.begin();
           iter != typesToRegister.end(); ++iter)
      {
         registerType(*iter, DataValueWrapperImp::createWrapper);
      }
   }

   ObjectMapType3 sCreateWrapperMap;

   static DataVariantFactoryImp *spInstance;
   static bool mDestroyed;

   void initializeMaps();
};

#endif
