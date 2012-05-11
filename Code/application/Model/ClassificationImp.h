/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CLASSIFICATIONIMP_H
#define CLASSIFICATIONIMP_H

#include "DynamicObjectImp.h"

#include <string>

class Classification;
class DateTime;
class DateTimeImp;

class ClassificationImp : public DynamicObjectImp
{
public:
   ClassificationImp();
   virtual ~ClassificationImp();

   ClassificationImp& operator =(const ClassificationImp& rhs);

   const std::string& getLevel() const;
   void setLevel(const std::string& myLevel);
   bool hasGreaterLevel(const Classification* pClassification) const;
   const std::string& getSystem() const;
   void setSystem(const std::string& mySystem);
   const std::string& getCodewords() const;
   void setCodewords(const std::string& myCodewords);
   const std::string& getFileControl() const;
   void setFileControl(const std::string& myFileControl);
   const std::string& getFileReleasing() const;
   void setFileReleasing(const std::string& myFileReleasing);
   const std::string& getClassificationReason() const;
   void setClassificationReason(const std::string& myClassificationReason);
   const std::string& getDeclassificationType() const;
   void setDeclassificationType(const std::string& myDeclassificationType);
   const DateTime* getDeclassificationDate() const;
   void setDeclassificationDate(const DateTime* myDeclassificationDate);
   const std::string& getDeclassificationExemption() const;
   void setDeclassificationExemption(const std::string& myDeclassificationExemption);
   const std::string& getFileDowngrade() const;
   void setFileDowngrade(const std::string& myFileDowngrade);
   const std::string& getCountryCode() const;
   void setCountryCode(const std::string& myCountryCode);
   const DateTime* getDowngradeDate() const;
   void setDowngradeDate(const DateTime* myDowngradeDate);
   const std::string& getDescription() const;
   void setDescription(const std::string& myDescription);
   const std::string& getAuthority() const;
   void setAuthority(const std::string& myAuthority);
   const std::string& getAuthorityType() const;
   void setAuthorityType(const std::string& myAuthorityType);
   const DateTime* getSecuritySourceDate() const;
   void setSecuritySourceDate(const DateTime* mySecuritySourceDate);
   const std::string& getSecurityControlNumber() const;
   void setSecurityControlNumber(const std::string& mySecurityControlNumber);
   const std::string& getFileCopyNumber() const;
   void setFileCopyNumber(const std::string& myFileCopyNumber);
   const std::string& getFileNumberOfCopies() const;
   void setFileNumberOfCopies(const std::string& myFileNumberOfCopies);

   bool setClassification(const std::string& classificationText);
   void setClassification(const Classification* pClassification);
   void getClassificationText(std::string& classificationText) const;
   bool compare(const DynamicObject* pObject) const;
   bool compare(const Classification* pClassification) const;
   bool isValid(std::string& errorMessage) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf( const std::string& className) const;

private:
   std::string mLevel;
   std::string mSystem;
   std::string mCodewords;
   bool mCodewordsDefaulted;
   std::string mFileControl;
   std::string mFileReleasing;
   std::string mClassificationReason;
   std::string mDeclassificationType;
   DateTimeImp* mpDeclassificationDate;
   std::string mDeclassificationExemption;
   std::string mFileDowngrade;
   std::string mCountryCode;
   DateTimeImp* mpDowngradeDate;
   std::string mDescription;
   std::string mAuthority;
   std::string mAuthorityType;
   DateTimeImp* mpSecuritySourceDate;
   std::string mSecurityControlNumber;
   std::string mFileCopyNumber;
   std::string mFileNumberOfCopies;

   void setDate(DateTimeImp* pOldDateTime, const DateTime* pNewDateTime);
   const DateTime* getDate(DateTimeImp* pDateTimeImp) const;
};

#define CLASSIFICATIONADAPTEREXTENSION_CLASSES \
   DYNAMICOBJECTADAPTEREXTENSION_CLASSES

#define CLASSIFICATIONADAPTER_METHODS(impClass) \
   DYNAMICOBJECTADAPTER_METHODS(impClass) \
   const std::string& getLevel() const \
   { \
      return impClass::getLevel(); \
   } \
   void setLevel(const std::string& myLevel) \
   { \
      impClass::setLevel(myLevel); \
   } \
   bool hasGreaterLevel(const Classification* pClassification) const \
   { \
      return impClass::hasGreaterLevel(pClassification); \
   } \
   const std::string& getSystem() const \
   { \
      return impClass::getSystem(); \
   } \
   void setSystem(const std::string& mySystem) \
   { \
      impClass::setSystem(mySystem); \
   } \
   const std::string& getCodewords() const \
   { \
      return impClass::getCodewords(); \
   } \
   void setCodewords(const std::string& myCodewords) \
   { \
      impClass::setCodewords(myCodewords); \
   } \
   const std::string& getFileControl() const \
   { \
      return impClass::getFileControl(); \
   } \
   void setFileControl(const std::string& myFileControl) \
   { \
      impClass::setFileControl(myFileControl); \
   } \
   const std::string& getFileReleasing() const \
   { \
      return impClass::getFileReleasing(); \
   } \
   void setFileReleasing(const std::string& myFileReleasing) \
   { \
      impClass::setFileReleasing(myFileReleasing); \
   } \
   const std::string& getClassificationReason() const \
   { \
      return impClass::getClassificationReason(); \
   } \
   void setClassificationReason(const std::string& myClassificationReason) \
   { \
      impClass::setClassificationReason(myClassificationReason); \
   } \
   const std::string& getDeclassificationType() const \
   { \
      return impClass::getDeclassificationType(); \
   } \
   void setDeclassificationType(const std::string& myDeclassificationType) \
   { \
      impClass::setDeclassificationType(myDeclassificationType); \
   } \
   const DateTime* getDeclassificationDate() const \
   { \
      return impClass::getDeclassificationDate(); \
   } \
   void setDeclassificationDate(const DateTime* myDeclassificationDate) \
   { \
      impClass::setDeclassificationDate(myDeclassificationDate); \
   } \
   const std::string& getDeclassificationExemption() const \
   { \
      return impClass::getDeclassificationExemption(); \
   } \
   void setDeclassificationExemption(const std::string& myDeclassificationExemption) \
   { \
      impClass::setDeclassificationExemption(myDeclassificationExemption); \
   } \
   const std::string& getFileDowngrade() const \
   { \
      return impClass::getFileDowngrade(); \
   } \
   void setFileDowngrade(const std::string& myFileDowngrade) \
   { \
      impClass::setFileDowngrade(myFileDowngrade); \
   } \
   const std::string& getCountryCode() const \
   { \
      return impClass::getCountryCode(); \
   } \
   void setCountryCode(const std::string& myCountryCode) \
   { \
      impClass::setCountryCode(myCountryCode); \
   } \
   const DateTime* getDowngradeDate() const \
   { \
      return impClass::getDowngradeDate(); \
   } \
   void setDowngradeDate(const DateTime* myDowngradeDate) \
   { \
      impClass::setDowngradeDate(myDowngradeDate); \
   } \
   const std::string& getDescription() const \
   { \
      return impClass::getDescription(); \
   } \
   void setDescription(const std::string& myDescription) \
   { \
      impClass::setDescription(myDescription); \
   } \
   const std::string& getAuthority() const \
   { \
      return impClass::getAuthority(); \
   } \
   void setAuthority(const std::string& myAuthority) \
   { \
      impClass::setAuthority(myAuthority); \
   } \
   const std::string& getAuthorityType() const \
   { \
      return impClass::getAuthorityType(); \
   } \
   void setAuthorityType(const std::string& myAuthorityType) \
   { \
      impClass::setAuthorityType(myAuthorityType); \
   } \
   const DateTime* getSecuritySourceDate() const \
   { \
      return impClass::getSecuritySourceDate(); \
   } \
   void setSecuritySourceDate(const DateTime* mySecuritySourceDate) \
   { \
      impClass::setSecuritySourceDate(mySecuritySourceDate); \
   } \
   const std::string& getSecurityControlNumber() const \
   { \
      return impClass::getSecurityControlNumber(); \
   } \
   void setSecurityControlNumber(const std::string& mySecurityControlNumber) \
   { \
      impClass::setSecurityControlNumber(mySecurityControlNumber); \
   } \
   const std::string& getFileCopyNumber() const \
   { \
      return impClass::getFileCopyNumber(); \
   } \
   void setFileCopyNumber(const std::string& myFileCopyNumber) \
   { \
      impClass::setFileCopyNumber(myFileCopyNumber); \
   } \
   const std::string& getFileNumberOfCopies() const \
   { \
      return impClass::getFileNumberOfCopies(); \
   } \
   void setFileNumberOfCopies(const std::string& myFileNumberOfCopies) \
   { \
      impClass::setFileNumberOfCopies(myFileNumberOfCopies); \
   } \
   void getClassificationText(std::string& classificationText) const \
   { \
      impClass::getClassificationText(classificationText); \
   } \
   bool setClassification(const std::string& classificationText) \
   { \
      return impClass::setClassification(classificationText); \
   } \
   void setClassification(const Classification* pClassification) \
   { \
      impClass::setClassification(pClassification); \
   } \
   bool compare(const Classification* pClassification) const \
   { \
      return impClass::compare(pClassification); \
   } \
   bool isValid(std::string& errorMessage) const \
   { \
      return impClass::isValid(errorMessage); \
   }

#endif
