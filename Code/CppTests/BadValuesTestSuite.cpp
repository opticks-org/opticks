/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "BadValues.h"
#include "ObjectResource.h"
#include "StringUtilities.h"
#include "TestSuiteNewSession.h"

#include <string>

class BadValuesTestCase : public TestCase
{
public:
   BadValuesTestCase() : TestCase("BadValues") {}
   bool run()
   {
      bool success(true);
      FactoryResource<BadValues> pBadValues;
      issearf(pBadValues.get() != NULL);

      // set tolerance and bad values
      std::string toleranceStr = "0.0001";
      double tolerance = StringUtilities::fromDisplayString<double>(toleranceStr);
      std::string badValuesStr = "<-10, 0, 3.5<>1.4e1, >200";
      issea(pBadValues->setBadValues(badValuesStr, toleranceStr));
      issea(pBadValues->getBadValuesString() == badValuesStr);
      issea(pBadValues->getBadValueTolerance() == toleranceStr);
      const std::vector<std::string> individualBadValues = pBadValues->getIndividualBadValues();
      issearf(individualBadValues.size() == 1);
      issea(individualBadValues.front() == "0");
      const std::vector<std::pair<std::string, std::string> > badValueRanges = pBadValues->getBadValueRanges();
      issearf(badValueRanges.size() == 1);
      issea(badValueRanges.front().first == "3.5" && badValueRanges.front().second == "1.4e1");

      // test lower threshold criteria
      double dValue = -10.0;
      issea(pBadValues->isBadValue(dValue) == true);
      issea(pBadValues->isBadValue(dValue + tolerance) == false);

      // test upper threshold criteria
      dValue = 200.0;
      issea(pBadValues->isBadValue(dValue) == true);
      issea(pBadValues->isBadValue(dValue - tolerance) == false);

      // test range criteria
      dValue = 3.5;
      issea(pBadValues->isBadValue(dValue) == true);
      issea(pBadValues->isBadValue(dValue - tolerance) == false);
      dValue = 14.0;
      issea(pBadValues->isBadValue(dValue) == true);
      issea(pBadValues->isBadValue(dValue + tolerance) == false);

      // test individual value
      dValue = 0.0;
      issea(pBadValues->isBadValue(dValue) == true);
      issea(pBadValues->isBadValue(dValue - tolerance) == false);
      issea(pBadValues->isBadValue(dValue + tolerance) == false);

      // test bad inputs
      std::string badStringVal = "abcd";
      std::string goodStringVal = "6";
      std::string saveStr = pBadValues->getBadValueTolerance();
      issea(pBadValues->setBadValueTolerance(badStringVal) == false);
      issea(pBadValues->getBadValueTolerance() == saveStr);
      saveStr = pBadValues->getLowerBadValueThreshold();
      issea(pBadValues->setLowerBadValueThreshold(badStringVal) == false);
      issea(pBadValues->getLowerBadValueThreshold() == saveStr);
      saveStr = pBadValues->getUpperBadValueThreshold();
      issea(pBadValues->setUpperBadValueThreshold(badStringVal) == false);
      issea(pBadValues->getUpperBadValueThreshold() == saveStr);
      saveStr = pBadValues->getLowerBadValueThreshold();
      std::string badThresholdStr = "201";
      issea(pBadValues->setLowerBadValueThreshold(badThresholdStr) == false);  // try to set lower threshold > upper
      issea(pBadValues->getLowerBadValueThreshold() == saveStr);
      saveStr = pBadValues->getUpperBadValueThreshold();
      badThresholdStr = "-20";
      issea(pBadValues->setUpperBadValueThreshold(badThresholdStr) == false);  // try to set upper threshold < lower
      issea(pBadValues->getUpperBadValueThreshold() == saveStr);
      issea(pBadValues->addRange(badStringVal, goodStringVal) == false);
      issea(pBadValues->getBadValueRanges() == badValueRanges);
      issea(pBadValues->addBadValue(badStringVal) == false);
      issea(pBadValues->getIndividualBadValues() == individualBadValues);

      // test adding/changing bad values
      std::vector<int> valuesToAdd;
      valuesToAdd.push_back(1);
      valuesToAdd.push_back(2);
      valuesToAdd.push_back(3);
      pBadValues->addBadValues(valuesToAdd);
      const std::vector<std::string> individualBadValues2 = pBadValues->getIndividualBadValues();
      issea(individualBadValues2.size() == 4);
      for (std::vector<std::string>::size_type i = 0; i < valuesToAdd.size(); ++i)
      {
         issea(individualBadValues2[i + 1] == StringUtilities::toDisplayString<int>(valuesToAdd[i]));
      }
      dValue = 3.0;
      issea(pBadValues->isBadValue(dValue) == true);
      issea(pBadValues->isBadValue(dValue - tolerance) == false);
      issea(pBadValues->isBadValue(dValue + tolerance) == false);
      goodStringVal = "20.0";
      std::string goodStringVal2 = "25.0";
      issea(pBadValues->addRange(goodStringVal, goodStringVal2));
      const std::vector<std::pair<std::string, std::string> > badValueRanges2 = pBadValues->getBadValueRanges();
      issearf(badValueRanges2.size() == 2);
      issea(badValueRanges2.back().first == goodStringVal);
      issea(badValueRanges2.back().second == goodStringVal2);
      dValue = 20.0;
      issea(pBadValues->isBadValue(dValue) == true);
      issea(pBadValues->isBadValue(dValue - tolerance) == false);
      dValue = 25.0;
      issea(pBadValues->isBadValue(dValue) == true);
      issea(pBadValues->isBadValue(dValue + tolerance) == false);
      dValue = -5.0;
      issea(pBadValues->isBadValue(dValue) == false);
      std::string thresholdStr = "-5.0";
      issea(pBadValues->setLowerBadValueThreshold(thresholdStr));
      issea(pBadValues->isBadValue(dValue) == true);
      dValue = 200.0;
      issea(pBadValues->isBadValue(dValue) == true);
      thresholdStr = "2.01e2";
      issea(pBadValues->setUpperBadValueThreshold(thresholdStr));
      issea(pBadValues->isBadValue(dValue) == false);
      double originalTolerance = tolerance;  // 0.0001
      toleranceStr = "0.001";
      tolerance = StringUtilities::fromDisplayString<double>(toleranceStr);
      dValue = 0.0;
      issea(pBadValues->isBadValue(dValue) == true);
      issea(pBadValues->isBadValue(dValue - originalTolerance) == false);
      issea(pBadValues->isBadValue(dValue + originalTolerance) == false);
      issea(pBadValues->setBadValueTolerance(toleranceStr));
      issea(pBadValues->isBadValue(dValue) == true);
      issea(pBadValues->isBadValue(dValue - originalTolerance) == true);
      issea(pBadValues->isBadValue(dValue + originalTolerance) == true);
      issea(pBadValues->isBadValue(dValue) == true);
      issea(pBadValues->isBadValue(dValue - tolerance) == false);
      issea(pBadValues->isBadValue(dValue + tolerance) == false);
      return success;
   }
};

class BadValuesTestSuite : public TestSuiteNewSession
{
public:
   BadValuesTestSuite() : TestSuiteNewSession("BadValues")
   {
      addTestCase(new BadValuesTestCase);
   }
};

REGISTER_SUITE(BadValuesTestSuite)

