/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "BadValues.h"
#include "BadValuesImp.h"
#include "ConfigurationSettings.h"
#include "ObjectResource.h"
#include "StringUtilities.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <QtCore/QString>

#include <limits>
#include <map>

XERCES_CPP_NAMESPACE_USE

BadValuesImp::BadValuesImp() :
   mToleranceStr(BadValues::getSettingTolerance()),
   mBadValuesBeingUpdated(false),
   mAdjustedUpper(std::numeric_limits<double>::max()),
   mAdjustedLower(-std::numeric_limits<double>::max()),
   mAdjustedSingleRangeValid(false),
   mAdjustedSingleRangeLower(std::numeric_limits<double>::max()),
   mAdjustedSingleRangeUpper(std::numeric_limits<double>::max())
{
   mTolerance = StringUtilities::fromDisplayString<double>(mToleranceStr);
}

BadValuesImp::~BadValuesImp()
{}

std::string BadValuesImp::getLowerBadValueThreshold() const
{
   return mLowerThresholdStr;
}

std::string BadValuesImp::getUpperBadValueThreshold() const
{
   return mUpperThresholdStr;
}

const std::vector<std::pair<std::string, std::string> >& BadValuesImp::getBadValueRanges() const
{
   return mRangeStrs;
}

const std::vector<std::string>& BadValuesImp::getIndividualBadValues() const
{
   return mBadValueStrs;
}

std::string BadValuesImp::getBadValuesString() const
{
   return mBadValuesAsString;
}

double BadValuesImp::getDefaultBadValue() const
{
   double defaultValue(-std::numeric_limits<double>::max());
   if (mBadValueStrs.empty() == false)
   {
      defaultValue = StringUtilities::fromDisplayString<double>(mBadValueStrs.front());
   }
   else if (mRangeStrs.empty() == false)
   {
      defaultValue = StringUtilities::fromDisplayString<double>(mRangeStrs.front().first);
   }
   else if (mLowerThresholdStr.empty() == false)
   {
      defaultValue = StringUtilities::fromDisplayString<double>(mLowerThresholdStr);
   }
   else if (mUpperThresholdStr.empty() == false)
   {
      defaultValue = StringUtilities::fromDisplayString<double>(mUpperThresholdStr);
   }

   return defaultValue;
}

bool BadValuesImp::isBadValue(double value) const
{
   // check for single bad value first for performance
   if (mAdjustedSingleRangeValid == true)
   {
      return value > mAdjustedSingleRangeLower && value < mAdjustedSingleRangeUpper;
   }

   if (empty())
   {
      return false;
   }

   // check thresholds
   if (mLowerThresholdStr.empty() == false && value < mAdjustedLower)
   {
      return true;
   }
   if (mUpperThresholdStr.empty() == false && value > mAdjustedUpper)
   {
      return true;
   }

   // check adjusted ranges which include individual values as range from value - tolerance to value + tolerance
   for (std::vector<std::pair<double, double> >::const_iterator it = mAdjustedRanges.begin();
      it != mAdjustedRanges.end(); ++it)
   {
      if (value > it->first && value < it->second)
      {
         return true;
      }
   }
   return false;
}

bool BadValuesImp::getSingleBadValueRange(double& lower, double& upper) const
{
   lower = mAdjustedSingleRangeLower;
   upper = mAdjustedSingleRangeUpper;
   return mAdjustedSingleRangeValid;
}

std::string BadValuesImp::getBadValueTolerance() const
{
   return mToleranceStr;
}

bool BadValuesImp::addRange(const std::string& startValStr, const std::string& endValStr)
{
   mErrorMsg.clear();
   if (isValidValueString(startValStr) == false)
   {
      mErrorMsg = "Input bad value range start string \"" + startValStr + "\"is invalid.";
      return false;
   }
   if (isValidValueString(endValStr) == false)
   {
      mErrorMsg = "Input bad value range end string \"" + endValStr + "\"is invalid.";
      return false;
   }
   std::pair<std::string, std::string> range(startValStr, endValStr);
   std::vector<std::pair<std::string, std::string> >::iterator it =
      std::find(mRangeStrs.begin(), mRangeStrs.end(), range);
   if (it == mRangeStrs.end())
   {
      mRangeStrs.push_back(range);
      generateAdjustedValues();
   }
   return true;
}

bool BadValuesImp::addBadValue(const std::string& valueStr)
{
   mErrorMsg.clear();
   if (isValidValueString(valueStr) == false)
   {
      mErrorMsg = "Input bad value string \"" + valueStr + "\"is invalid.";
      return false;
   }

   std::vector<std::string>::iterator it = std::find(mBadValueStrs.begin(), mBadValueStrs.end(), valueStr);
   if (it == mBadValueStrs.end())
   {
      mBadValueStrs.push_back(valueStr);
      generateAdjustedValues();
   }
   return true;
}

void BadValuesImp::addBadValues(const std::vector<int>& badValues)
{
   bool valuesAdded(false);
   for (std::vector<int>::const_iterator iter = badValues.begin(); iter != badValues.end(); ++iter)
   {
      std::string valStr = StringUtilities::toDisplayString<int>(*iter);
      std::vector<std::string>::iterator sit = std::find(mBadValueStrs.begin(), mBadValueStrs.end(), valStr);
      if (sit == mBadValueStrs.end())
      {
         mBadValueStrs.push_back(valStr);
         valuesAdded = true;
      }
   }
   if (valuesAdded)
   {
      generateAdjustedValues();
   }
}

bool BadValuesImp::setLowerBadValueThreshold(const std::string& thresholdStr)
{
   mErrorMsg.clear();

   if (thresholdStr != mLowerThresholdStr)
   {
      // If the input string is not empty, validate the input. If it is empty, the threshold check will be disabled .
      if (thresholdStr.empty() == false)
      {
         if (isValidValueString(thresholdStr) == false)
         {
            mErrorMsg = "Input bad value lower threshold string \"" + thresholdStr + "\"is invalid.";
            return false;
         }

         // check that new lower threshold is less than the upper if an upper is set
         if (mUpperThresholdStr.empty() == false)
         {
            if (QString::fromStdString(thresholdStr).toDouble() > QString::fromStdString(mUpperThresholdStr).toDouble())
            {
               mErrorMsg = "Input bad value lower threshold \"" + thresholdStr +
                  "\"is greater than the current upper threshold of " + mUpperThresholdStr + ".";
               return false;
            }
         }
      }
      mLowerThresholdStr = thresholdStr;
      generateAdjustedValues();
   }
   return true;
}

bool BadValuesImp::setUpperBadValueThreshold(const std::string& thresholdStr)
{
   mErrorMsg.clear();

   if (thresholdStr != mUpperThresholdStr)
   {
      // If the input string is not empty, validate the input. If it is empty, the threshold check will be disabled .
      if (thresholdStr.empty() == false)
      {
         if (isValidValueString(thresholdStr) == false)
         {
            mErrorMsg = "Input bad value upper threshold string \"" + thresholdStr + "\"is invalid.";
            return false;
         }

         // check that new upper threshold is greater than the lower if a lower is set
         if (mLowerThresholdStr.empty() == false)
         {
            if (QString::fromStdString(thresholdStr).toDouble() < QString::fromStdString(mLowerThresholdStr).toDouble())
            {
               mErrorMsg = "Input bad value upper threshold \"" + thresholdStr +
                  "\"is less than the current lower threshold of " + mLowerThresholdStr + ".";
               return false;
            }
         }
      }
      mUpperThresholdStr = thresholdStr;
      generateAdjustedValues();
   }
   return true;
}

void BadValuesImp::setBadValues(const BadValues* pBadValues)
{
   if (pBadValues == NULL || compare(pBadValues))
   {
      return;
   }

   mToleranceStr = pBadValues->getBadValueTolerance();
   bool error(false);
   mTolerance = StringUtilities::fromDisplayString<double>(mToleranceStr, &error);
   VERIFYNRV(error != true);

   if (empty() && pBadValues->empty())  // finished since only the tolerance changed and there's no bad value criteria
   {
      return;
   }

   mLowerThresholdStr = pBadValues->getLowerBadValueThreshold();
   mUpperThresholdStr = pBadValues->getUpperBadValueThreshold();
   mBadValueStrs = pBadValues->getIndividualBadValues();
   mRangeStrs = pBadValues->getBadValueRanges();
   generateAdjustedValues();
}

bool BadValuesImp::setBadValues(const std::string& badValStr, const std::string& toleranceStr)
{
   mErrorMsg.clear();
   if ((badValStr == mBadValuesAsString && toleranceStr == mToleranceStr) ||
      (empty() && badValStr.empty() && toleranceStr.empty()))
   {
       // nothing to do, the bad values criteria are already set to these values or no values were passed in and
       // this instance is empty.
      return true;
   }

   FactoryResource<BadValues> pNewBadValues;
   pNewBadValues->startBadValuesUpdate();

   // check for valid tolerance string - if the string is empty, the existing tolerance will be used
   if (toleranceStr.empty())
   {
      pNewBadValues->setBadValueTolerance(mToleranceStr);
   }
   else
   {
      if (isValidValueString(toleranceStr) == false)
      {
         mErrorMsg = "The tolerance string \"" + toleranceStr + "\" is invalid.";
         return false;
      }

      pNewBadValues->setBadValueTolerance(toleranceStr);
   }

   std::string tmpLowerThreshold;
   std::string tmpUpperThreshold;
   if (badValStr.empty() == false)
   {
      std::string tmpStr;
      std::vector<std::string> valueStrs = StringUtilities::split(badValStr, ',');
      for (std::vector<std::string>::const_iterator iter = valueStrs.begin(); iter != valueStrs.end(); ++iter)
      {
         std::string valStr = StringUtilities::stripWhitespace(*iter);
         if (valStr.empty())
         {
            continue;
         }
         if (valStr[0] == '<')                             // lower threshold identifier
         {
            tmpStr = valStr.substr(1);
            if (isValidValueString(tmpStr))
            {
               tmpLowerThreshold = tmpStr;
            }
            else
            {
               mErrorMsg = "The lower threshold input \"" + valStr + "\" is invalid.";
               return false;
            }
         }
         else if (valStr[0] == '>')                        // upper threshold identifier
         {
            tmpStr = valStr.substr(1);
            if (isValidValueString(tmpStr))
            {
               tmpUpperThreshold = tmpStr;
            }
            else
            {
               mErrorMsg = "The upper threshold input \"" + valStr + "\" is invalid.";
               return false;
            }
         }
         else if (valStr.find("<>") != std::string::npos)  // range identifier
         {
            std::string::size_type pos = valStr.find("<");
            std::string startStr = valStr.substr(0, pos);
            pos = valStr.find(">");
            std::string endStr = valStr.substr(pos + 1);
            if (isValidValueString(startStr) && isValidValueString(endStr))
            {
               if (QString::fromStdString(startStr).toDouble() > QString::fromStdString(endStr).toDouble())
               {
                  mErrorMsg = "Invalid range - the start of range \"" + valStr +
                     "\" is greater than the end of the range.";
                  return false;
               }
               pNewBadValues->addRange(startStr, endStr);
            }
            else                                           // only individual value left
            {
               mErrorMsg = "The range input \"" + valStr + "\" is invalid.";
               return false;
            }
         }
         else
         {
            if (isValidValueString(valStr))
            {
               pNewBadValues->addBadValue(valStr);
            }
            else
            {
               mErrorMsg = "The input \"" + valStr + "\" is invalid.";
               return false;
            }
         }
      }
   }

   // now check that lower threshold if specified is less than the upper threshold if specified
   if (tmpLowerThreshold.empty() == false && tmpUpperThreshold.empty() == false)
   {
      double lower = QString::fromStdString(tmpLowerThreshold).toDouble();  // already checked string conversions
      double upper = QString::fromStdString(tmpUpperThreshold).toDouble();
      if (lower > upper)
      {
         mErrorMsg = "The input lower threshold \"" + tmpLowerThreshold + "\" is greater than the input "
            "upper threshold \"" + tmpUpperThreshold + "\".";
         return false;
      }
   }

   // set the thresholds if specified
   if (tmpLowerThreshold.empty() == false)
   {
      if (pNewBadValues->setLowerBadValueThreshold(tmpLowerThreshold) == false)
      {
         mErrorMsg = pNewBadValues->getLastErrorMsg();
         return false;
      }
   }
   if (tmpUpperThreshold.empty() == false)
   {
      if (pNewBadValues->setUpperBadValueThreshold(tmpUpperThreshold) == false)
      {
         mErrorMsg = pNewBadValues->getLastErrorMsg();
         return false;
      }
   }

   pNewBadValues->endBadValuesUpdate();
   setBadValues(pNewBadValues.get());
   return true;
}

bool BadValuesImp::setBadValueTolerance(const std::string& badValToleranceStr)
{
   mErrorMsg.clear();
   if (isValidValueString(badValToleranceStr) == false)
   {
      mErrorMsg = "Input tolerance string \"" + badValToleranceStr + "\"is invalid.";
      return false;
   }

   if (badValToleranceStr != mToleranceStr)
   {
      mToleranceStr = badValToleranceStr;
      mTolerance = StringUtilities::fromDisplayString<double>(mToleranceStr);
      if (!empty())  // only update if some bad values criteria is set
      {
         generateAdjustedValues();
      }
   }
   return true;
}

void BadValuesImp::clear()
{
   mLowerThresholdStr.clear();
   mAdjustedLower = -std::numeric_limits<double>::max();
   mUpperThresholdStr.clear();
   mAdjustedUpper = std::numeric_limits<double>::max();
   mBadValuesAsString.clear();
   mAdjustedRanges.clear();
   mRangeStrs.clear();
   mBadValueStrs.clear();
   mToleranceStr = BadValues::getSettingTolerance();
   mTolerance = StringUtilities::fromDisplayString<double>(mToleranceStr);
   mAdjustedSingleRangeValid = false;
   mAdjustedSingleRangeLower = std::numeric_limits<double>::max();
   mAdjustedSingleRangeUpper = std::numeric_limits<double>::max();

   if (!mBadValuesBeingUpdated)
   {
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

bool BadValuesImp::empty() const
{
   return mBadValuesAsString.empty();
}

bool BadValuesImp::compare(const BadValues* pBadValues) const
{
   const BadValuesImp* pImp = dynamic_cast<const BadValuesImp*>(pBadValues);
   if (pImp == NULL)
   {
      return false;
   }

   if (mToleranceStr != pImp->mToleranceStr)
   {
      return false;
   }

   if (mBadValueStrs != pImp->mBadValueStrs)
   {
      return false;
   }

   if (mRangeStrs != pImp->mRangeStrs)
   {
      return false;
   }

   if (mBadValuesAsString != pImp->mBadValuesAsString)
   {
      return false;
   }

   return true;
}

void BadValuesImp::startBadValuesUpdate()
{
   mBadValuesBeingUpdated = true;
}

void BadValuesImp::endBadValuesUpdate()
{
   // only update the adjusted values after a call to startBadValuesUpdate has occurred
   if (mBadValuesBeingUpdated)
   {
      mBadValuesBeingUpdated = false;
      generateAdjustedValues();
   }
}

void BadValuesImp::generateAdjustedValues()
{
   if (mBadValuesBeingUpdated)
   {
      return;
   }

   std::string previousBadValuesStr = mBadValuesAsString;
   mAdjustedRanges.clear();

   double value;

   // generate adjusted thresholds if necessary
   if (mLowerThresholdStr.empty() == false)
   {
      bool error(false);
      value = StringUtilities::fromDisplayString<double>(mLowerThresholdStr, &error);
      if (error == false)
      {
         mAdjustedLower = value + mTolerance;
      }
   }
   else
   {
      mAdjustedLower = -std::numeric_limits<double>::max();
   }

   if (mUpperThresholdStr.empty() == false)
   {
      bool error(false);
      value = StringUtilities::fromDisplayString<double>(mUpperThresholdStr, &error);
      if (error == false)
      {
         mAdjustedUpper = value - mTolerance;
      }
   }
   else
   {
      mAdjustedUpper = std::numeric_limits<double>::max();
   }

   // generate adjusted ranges
   std::map<double, double> tempRanges;
   if (mRangeStrs.empty() == false)
   {
      for (std::vector<std::pair<std::string, std::string> >::const_iterator it = mRangeStrs.begin();
         it != mRangeStrs.end(); ++it)
      {
         bool errorFirst(false);
         bool errorSecond(false);
         double start = StringUtilities::fromDisplayString<double>(it->first, &errorFirst);
         double end = StringUtilities::fromDisplayString<double>(it->second, &errorSecond);
         if (errorFirst || errorSecond || start > end)  // skip bad ranges
         {
            continue;
         }

         // check if start of range below lower threshold
         if (start <= mAdjustedLower)
         {
            if (end <= mAdjustedLower)                  // entire range below lower threshold so skip it
            {
               continue;
            }
            else if (end < mAdjustedUpper)              // range overlaps lower so adjust threshold
            {
               mAdjustedLower = end - mTolerance;
               continue;
            }
            else                                        // bad range for current thresholds - no good values possible
            {
               continue;
            }
         }

         // now check upper
         if (end >= mAdjustedUpper)
         {
            if (start >= mAdjustedUpper)                // entire range above upper threshold so skip it
            {
               continue;
            }
            else if (start < mAdjustedUpper)            // range overlaps upper so adjust threshold
            {
               mAdjustedUpper = start - mTolerance;
            }
            else                                        // bad range for current thresholds - no good values possible
            {
               continue;
            }
         }
         tempRanges[start - mTolerance] = end + mTolerance;
      }
   }

   if (mBadValueStrs.empty() == false)
   {
      for (std::vector<std::string>::const_iterator it = mBadValueStrs.begin(); it != mBadValueStrs.end(); ++it)
      {
         bool error(false);
         value = StringUtilities::fromDisplayString<double>(*it);
         if (error || value <= mAdjustedLower || value >= mAdjustedUpper)
         {
            continue;
         }

         // check if value is in a range
         bool valueInRange(false);
         for (std::map<double,double>::const_iterator it = tempRanges.begin(); it != tempRanges.end(); ++it)
         {
            if (value > it->first && value < it->second)
            {
               valueInRange = true;
               break;
            }
         }

         if (valueInRange == false)
         {
            tempRanges[value - mTolerance] = value + mTolerance;
         }
         else
         {
            valueInRange = false;
         }
      }
   }

   // now push the sorted, adjusted ranges into mAdjustedRanges
   for (std::map<double,double>::const_iterator it = tempRanges.begin(); it != tempRanges.end(); ++it)
   {
      std::pair<double, double> range(it->first, it->second);
      mAdjustedRanges.push_back(range);
   }

   // special case a single range for better performance when calling isBadValue()
   if (mAdjustedRanges.size() == 1)
   {
      mAdjustedSingleRangeValid = true;
      mAdjustedSingleRangeLower = mAdjustedRanges.front().first;
      mAdjustedSingleRangeUpper = mAdjustedRanges.front().second;
   }
   else
   {
      mAdjustedSingleRangeValid = false;
      mAdjustedSingleRangeLower = std::numeric_limits<double>::max();
      mAdjustedSingleRangeUpper = std::numeric_limits<double>::max();
   }

   generateBadValuesString();
   notify(SIGNAL_NAME(Subject, Modified));
}

const std::string& BadValuesImp::getObjectType() const
{
   static std::string sType("BadValuesImp");
   return sType;
}

bool BadValuesImp::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "BadValues"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void BadValuesImp::generateBadValuesString()
{
   std::string separator(", ");
   std::string tempStr;
   if (mLowerThresholdStr.empty() == false)
   {
      tempStr = "<" + mLowerThresholdStr;
   }
   if (mBadValueStrs.empty() == false)
   {
      for (std::vector<std::string>::const_iterator iter = mBadValueStrs.begin(); iter != mBadValueStrs.end(); ++iter)
      {
         tempStr += (tempStr.empty() ? "" : separator) + *iter;
      }
   }
   if (mRangeStrs.empty() == false)
   {
      for (std::vector<std::pair<std::string, std::string> >::const_iterator iter = mRangeStrs.begin();
         iter != mRangeStrs.end(); ++iter)
      {
         tempStr += (tempStr.empty() ? "" : separator) + iter->first + "<>" + iter->second;
      }
   }
   if (mUpperThresholdStr.empty() == false)
   {
      tempStr += (tempStr.empty() ? "" : separator) + ">" + mUpperThresholdStr;
   }

   mBadValuesAsString = tempStr;
}

bool BadValuesImp::toXml(XMLWriter* pXml) const
{
   VERIFY(pXml != NULL);
   pXml->addAttr("badValuesAsString", mBadValuesAsString);
   pXml->addAttr("badValuesToleranceString", mToleranceStr);
   return true;
}
bool BadValuesImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   DOMElement* pElem(static_cast<DOMElement*>(pDocument));
   VERIFY(pElem != NULL);
   return setBadValues(A(pElem->getAttribute(X("badValuesAsString"))),
      A(pElem->getAttribute(X("badValuesToleranceString"))));
}

bool BadValuesImp::isValidValueString(const std::string& valueStr) const
{
   bool ok(false);
   QString::fromStdString(valueStr).toDouble(&ok);  // ok will be false for an empty string
   return ok;
}

std::string BadValuesImp::getLastErrorMsg()
{
   std::string msg = mErrorMsg;
   mErrorMsg.clear();
   return msg;
}