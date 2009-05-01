/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATUREIMP_H
#define SIGNATUREIMP_H

#include "DataElementImp.h"
#include "DataVariant.h"

#include <boost/shared_ptr.hpp>
#include <map>
#include <set>

class Units;
class UnitsImp;

/**
 * NOTE: Need to automatically put sensor, location, and collection
 * time in signature description. 
 */
class SignatureImp : public DataElementImp
{
public:
   SignatureImp(const DataDescriptorImp& descriptor, const std::string& id);
   ~SignatureImp();

   virtual const DataVariant& getData(const std::string& name) const;
   virtual void adoptData(const std::string& name, DataVariant& data);
   virtual const Units *getUnits(const std::string& name) const;
   void setUnits(const std::string& name, const Units* pUnits);
   std::set<std::string> getDataNames() const;
   std::set<std::string> getUnitNames() const;

   virtual DataElement* copy(const std::string& name, DataElement* pParent) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfElement(const std::string& className);
   static void getElementTypes(std::vector<std::string>& classList);

private:
   void setData(const std::string& name, DataVariant& data, bool adopt);
   std::map<std::string, DataVariant> mData;
   std::map<std::string, boost::shared_ptr<UnitsImp> > mUnits;
   DataVariant mNullData;
};

#define SIGNATUREADAPTEREXTENSION_CLASSES \
   DATAELEMENTADAPTEREXTENSION_CLASSES

#define SIGNATUREADAPTER_METHODS(impClass) \
   DATAELEMENTADAPTER_METHODS(impClass) \
   const DataVariant& getData(const std::string& name) const \
   { \
      return impClass::getData(name); \
   } \
   void adoptData(const std::string& name, DataVariant& data) \
   { \
      impClass::adoptData(name, data); \
   } \
   const Units* getUnits(const std::string& name) const \
   { \
      return impClass::getUnits(name); \
   } \
   void setUnits(const std::string& name, const Units* pUnits) \
   { \
      impClass::setUnits(name, pUnits); \
   } \
   std::set<std::string> getDataNames() const \
   { \
      return impClass::getDataNames(); \
   } \
   std::set<std::string> getUnitNames() const \
   { \
      return impClass::getUnitNames(); \
   }

#endif
