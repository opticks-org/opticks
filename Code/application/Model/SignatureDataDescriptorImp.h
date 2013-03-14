/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATUREDATADESCRIPTORIMP_H
#define SIGNATUREDATADESCRIPTORIMP_H

#include "DataDescriptorImp.h"

#include <map>
#include <set>
#include <string>
#include <vector>

class DataDescriptor;
class Units;

using XERCES_CPP_NAMESPACE_QUALIFIER DOMNode;

class SignatureDataDescriptorImp : public DataDescriptorImp
{
public:
   SignatureDataDescriptorImp(const std::string& name, const std::string& type, DataElement* pParent);
   SignatureDataDescriptorImp(const std::string& name, const std::string& type, const std::vector<std::string>& parent);
   SignatureDataDescriptorImp(DOMNode* pDocument, unsigned int version, DataElement* pParent);
   virtual ~SignatureDataDescriptorImp();

   using DataDescriptorImp::copy;

   void setUnits(const std::string& name, const Units* pUnits);
   const Units* getUnits(const std::string& name) const;
   std::set<std::string> getUnitNames() const;

   virtual DataDescriptor* copy(const std::string& name, DataElement* pParent) const;
   virtual DataDescriptor* copy(const std::string& name, const std::vector<std::string>& parent) const;
   virtual bool clone(const DataDescriptor* pDescriptor);

   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getDataDescriptorTypes(std::vector<std::string>& classList);
   static bool isKindOfDataDescriptor(const std::string& className);

protected:
   void notifyUnitsModified(Subject& subject, const std::string& signal, const boost::any& data);

private:
   void clearUnits();

   std::map<std::string, Units*> mUnits;
};

#define SIGNATUREDATADESCRIPTORADAPTEREXTENSION_CLASSES \
   DATADESCRIPTORADAPTEREXTENSION_CLASSES

#define SIGNATUREDATADESCRIPTORADAPTER_METHODS(impClass) \
   DATADESCRIPTORADAPTER_METHODS(impClass) \
   void setUnits(const std::string& name, const Units* pUnits) \
   { \
      impClass::setUnits(name, pUnits); \
   } \
   const Units* getUnits(const std::string& name) const \
   { \
      return impClass::getUnits(name); \
   } \
   std::set<std::string> getUnitNames() const \
   { \
      return impClass::getUnitNames(); \
   }

#endif
