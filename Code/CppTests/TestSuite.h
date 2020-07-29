/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <list>
#include <vector>
#include <string>

#include "TestBedTestUtilities.h"
#include "TestCase.h"

class TestSuite : public TestCase
{
public:
   TestSuite(const std::string& name);
   virtual ~TestSuite();
   void setDisabledTestCases(const std::vector<std::string>& testList);
   void addTestCase(TestCase*);
   std::list<TestCase*> getAllTestCases() const;
   virtual void runTest(int &passed, int &total, const std::string& indent = "");
private:
   bool run() { return false; }
   std::vector<std::string> mDisabledTestCases;
   std::list<TestCase*> mTestCases;
};

class TestSuiteFactory
{
public:
   TestSuiteFactory()
   {
      TestUtilities::getFactoryVector().push_back(this);
   }

   virtual TestSuite* createSuite() const
   {
      return NULL;
   }
};

#define REGISTER_SUITE(suiteClass) \
namespace \
{ \
   class suiteClass##SuiteFactory : public TestSuiteFactory \
   { \
      TestSuite* createSuite() const \
      { \
         return new suiteClass(); \
      } \
   }; \
   static suiteClass##SuiteFactory suiteClass##FactoryInstance; \
}

#endif
