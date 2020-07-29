/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TEST_CASE_H
#define TEST_CASE_H

#include <string>

class TestCase
{
public:
   TestCase(const std::string& name);
   virtual ~TestCase();
   std::string getName() const;
   virtual void runTest(int &passed, int &total, const std::string& indent = std::string(""));
private:
   virtual bool run() = 0;
   std::string mName;
};

#endif
