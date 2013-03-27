/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BADVALUESIMP_H
#define BADVALUESIMP_H

#include "ConfigurationSettings.h"
#include "SerializableImp.h"
#include "SubjectImp.h"

#include <string>
#include <vector>

class BadValues;

class BadValuesImp : public SubjectImp, public Serializable
{
public:
   BadValuesImp();

   BadValuesImp& operator= (const BadValuesImp& rhs);

   const BadValues* getBadValues() const;
   BadValues* getBadValues();
   virtual double getDefaultBadValue() const;
   virtual bool isBadValue(double value) const;
   virtual bool getSingleBadValueRange(double& lower, double& upper) const;

   virtual std::string getLowerBadValueThreshold() const;
   virtual std::string getUpperBadValueThreshold() const;
   virtual const std::vector<std::pair<std::string, std::string> >& getBadValueRanges() const;
   virtual const std::vector<std::string>& getIndividualBadValues() const;
   virtual std::string getBadValuesString() const;
   virtual std::string getBadValueTolerance() const;

   virtual bool addRange(const std::string& startValStr, const std::string& endValStr);
   virtual bool addBadValue(const std::string& valueStr);
   virtual void addBadValues(const std::vector<int>& badValues);
   virtual bool setLowerBadValueThreshold(const std::string& thresholdStr);
   virtual bool setUpperBadValueThreshold(const std::string& thresholdStr);
   virtual void setBadValues(const BadValues* pBadValues);
   virtual bool setBadValues(const std::string& badValStr, const std::string& toleranceStr = std::string());
   virtual bool setBadValueTolerance(const std::string& badValToleranceStr);
   virtual std::string getLastErrorMsg();
   virtual void clear();
   virtual bool empty() const;
   virtual bool compare(const BadValues* pBadValues) const;

   virtual void startBadValuesUpdate();
   virtual void endBadValuesUpdate();

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf( const std::string& className) const;

protected:
   virtual ~BadValuesImp();
   void generateAdjustedValues();
   void generateBadValuesString();
   bool isValidValueString(const std::string& valueStr) const;

private:
   std::string mBadValuesAsString;
   std::string mLowerThresholdStr;
   std::string mUpperThresholdStr;
   std::string mToleranceStr;
   std::vector<std::pair<std::string, std::string> > mRangeStrs;
   std::vector<std::string> mBadValueStrs;
   std::vector<std::pair<double, double> > mAdjustedRanges;
   double mTolerance;
   double mAdjustedUpper;
   double mAdjustedLower;
   bool mAdjustedSingleRangeValid;
   double mAdjustedSingleRangeLower;
   double mAdjustedSingleRangeUpper;
   bool mBadValuesBeingUpdated;
   std::string mErrorMsg;
};

#define BADVALUESADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES \
   SERIALIZABLEADAPTEREXTENSION_CLASSES

#define BADVALUESADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   SERIALIZABLEADAPTER_METHODS(impClass) \
   bool addRange(const std::string& startValStr, const std::string& endValStr) \
   { \
      return impClass::addRange(startValStr, endValStr); \
   } \
   bool addBadValue(const std::string& valueStr) \
   { \
      return impClass::addBadValue(valueStr); \
   } \
   void addBadValues(const std::vector<int>& badValues) \
   { \
      impClass::addBadValues(badValues); \
   } \
   bool isBadValue(double value) const \
   { \
      return impClass::isBadValue(value); \
   } \
   bool getSingleBadValueRange(double& lower, double& upper) const \
   { \
      return impClass::getSingleBadValueRange(lower, upper); \
   } \
   bool setLowerBadValueThreshold(const std::string& thresholdStr) \
   { \
      return impClass::setLowerBadValueThreshold(thresholdStr); \
   } \
   bool setUpperBadValueThreshold(const std::string& thresholdStr) \
   { \
      return impClass::setUpperBadValueThreshold(thresholdStr); \
   } \
   std::string getLowerBadValueThreshold() const \
   { \
      return impClass::getLowerBadValueThreshold(); \
   } \
   std::string getUpperBadValueThreshold() const \
   { \
      return impClass::getUpperBadValueThreshold(); \
   } \
   const std::vector<std::pair<std::string, std::string> >& getBadValueRanges() const \
   { \
      return impClass::getBadValueRanges(); \
   } \
   const std::vector<std::string>& getIndividualBadValues() const \
   { \
      return impClass::getIndividualBadValues(); \
   } \
   std::string getBadValuesString() const \
   { \
      return impClass::getBadValuesString(); \
   } \
   double getDefaultBadValue() const \
   { \
      return impClass::getDefaultBadValue(); \
   } \
   void setBadValues(const BadValues* pBadValues) \
   { \
      impClass::setBadValues(pBadValues); \
   } \
   bool setBadValues(const std::string& badValStr, const std::string& toleranceStr = std::string()) \
   { \
      return impClass::setBadValues(badValStr, toleranceStr); \
   } \
   std::string getLastErrorMsg() \
   { \
      return impClass::getLastErrorMsg(); \
   } \
   void clear() \
   { \
      impClass::clear(); \
   } \
   bool empty() const \
   { \
      return impClass::empty(); \
   } \
   bool compare(const BadValues* pBadValues) const \
   { \
      return impClass::compare(pBadValues); \
   } \
   std::string getBadValueTolerance() const \
   { \
      return impClass::getBadValueTolerance(); \
   } \
   bool setBadValueTolerance(const std::string& badValToleranceStr) \
   { \
      return impClass::setBadValueTolerance(badValToleranceStr); \
   } \
   void startBadValuesUpdate() \
   { \
      impClass::startBadValuesUpdate(); \
   } \
   void endBadValuesUpdate() \
   { \
      impClass::endBadValuesUpdate(); \
   }

#endif
