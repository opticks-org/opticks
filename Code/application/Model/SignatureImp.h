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

   virtual const DataVariant &getData(std::string name) const;
   virtual void setData(std::string name, const DataVariant &data);
   virtual const Units *getUnits(std::string name) const;
   void setUnits(std::string name, const Units *pUnits);
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
   std::map<std::string,DataVariant> mData;
   std::map<std::string,boost::shared_ptr<UnitsImp> > mUnits;
   DataVariant mNullData;
};

#define SIGNATUREADAPTER_METHODS(impClass) \
   DATAELEMENTADAPTER_METHODS(impClass) \
   const DataVariant& getData(std::string name) const \
   { \
      return impClass::getData(name); \
   } \
   void setData(std::string name, const DataVariant& data) \
   { \
      impClass::setData(name, data); \
   } \
   const Units* getUnits(std::string name) const \
   { \
      return impClass::getUnits(name); \
   } \
   void setUnits(std::string name, const Units* pUnits) \
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
