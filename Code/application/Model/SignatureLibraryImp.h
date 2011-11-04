/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATURELIBRARYIMP_H
#define SIGNATURELIBRARYIMP_H

#include "AttachmentPtr.h"
#include "LibrarySignatureAdapter.h"
#include "RasterElement.h"
#include "SignatureSetImp.h"

#include <map>
#include <string>
#include <vector>

class SignatureLibraryImp : public SignatureSetImp
{
public:
   SignatureLibraryImp(const DataDescriptorImp& descriptor, const std::string& id);
   ~SignatureLibraryImp();

   const std::vector<double>& getAbscissa() const;
   const std::vector<double>& getOriginalAbscissa() const;
   const RasterElement *getOriginalOrdinateData() const;
   RasterElement *getOriginalOrdinateData();
   const double *getOrdinateData() const;
   const double *getOrdinateData(unsigned int index) const;
   std::set<std::string> getSignatureNames() const;
   std::string getSignatureName(unsigned int index) const;
   const Signature *getSignature(unsigned int index) const;
   const Signature* getSignature(const std::string& name) const;
   const std::string& getAbscissaName() const;
   bool resample(const std::vector<double> &abscissa);
   void desample();
   bool import(const std::string& filename, const std::string& importerName, Progress* pProgress = NULL);

   bool insertSignature(Signature* pSignature);
   bool insertSignatures(const std::vector<Signature*>& signatures);
   unsigned int getNumSignatures() const;
   bool hasSignature(Signature* pSignature) const;
   std::vector<Signature*> getSignatures() const;
   bool removeSignature(Signature* pSignature, bool bDelete = false);
   bool removeSignatures(const std::vector<Signature*>& signatures, bool bDelete = false);
   void clear(bool bDelete = false);

   DataElement* copy(const std::string& name, DataElement* pParent) const;

   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);
   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfElement(const std::string& className);

private:
   SignatureLibraryImp(const SignatureLibraryImp& rhs);
   SignatureLibraryImp& operator=(const SignatureLibraryImp& rhs);
   void ordinateDeleted(Subject &subject, const std::string &signal, const boost::any &data);
   void ordinateModified(Subject &subject, const std::string &signal, const boost::any &data);
   std::vector<double> mAbscissa;
   std::vector<double> mOriginalAbscissa;
   std::map<std::string, Signature *> mSignatureNames;
   std::vector<LibrarySignatureAdapter*> mSignatures;
   std::vector<double> mResampledData;
   AttachmentPtr<RasterElement> mpOdre;
   std::string mAbscissaName;
   bool mNeedToResample;
};

#define SIGNATURELIBRARYADAPTEREXTENSION_CLASSES \
   SIGNATURESETADAPTEREXTENSION_CLASSES

#define SIGNATURELIBRARYADAPTER_METHODS(impClass) \
   SIGNATURESETADAPTER_METHODS(impClass) \
   const std::vector<double>& getAbscissa() const \
   { \
      return impClass::getAbscissa(); \
   } \
   const std::vector<double>& getOriginalAbscissa() const \
   { \
      return impClass::getOriginalAbscissa(); \
   } \
   const RasterElement *getOriginalOrdinateData() const \
   { \
      return impClass::getOriginalOrdinateData(); \
   } \
   RasterElement *getOriginalOrdinateData() \
   { \
      return impClass::getOriginalOrdinateData(); \
   } \
   const double *getOrdinateData() const \
   { \
      return impClass::getOrdinateData(); \
   } \
   const double *getOrdinateData(unsigned int index) const \
   { \
      return impClass::getOrdinateData(index); \
   } \
   std::set<std::string> getSignatureNames() const \
   { \
      return impClass::getSignatureNames(); \
   } \
   std::string getSignatureName(unsigned int index) const \
   { \
      return impClass::getSignatureName(index); \
   } \
   const Signature *getSignature(unsigned int index) const \
   { \
      return impClass::getSignature(index); \
   } \
   const Signature* getSignature(const std::string& name) const \
   { \
      return impClass::getSignature(name); \
   } \
   const std::string& getAbscissaName() const \
   { \
      return impClass::getAbscissaName(); \
   } \
   bool resample(const std::vector<double> &abscissa) \
   { \
      return impClass::resample(abscissa); \
   } \
   void desample() \
   { \
      impClass::desample(); \
   } \
   bool import(const std::string& filename, const std::string& importerName, Progress* pProgress = NULL) \
   { \
      return impClass::import(filename, importerName, pProgress); \
   }

#endif
