/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TEST_SUITE_NEW_SESSION_H
#define TEST_SUITE_NEW_SESSION_H

#include <string>

#include "TestSuite.h"

class TestSuiteNewSession : public TestSuite
{
public:
   TestSuiteNewSession(const std::string& name);
   virtual ~TestSuiteNewSession();
   virtual void runTest(int &passed, int &total, const std::string& indent = "");
};

#endif
