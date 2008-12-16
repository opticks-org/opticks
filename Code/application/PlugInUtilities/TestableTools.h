/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef TESTABLE_TOOLS_H
#define TESTABLE_TOOLS_H

#include <iostream>
#include <ostream>
#include <string>
#include <cstdio>

/**
  * The assertion logger is used in conjunction with the Testable interface to dump errors
  * to std::cout.
  *
  * Do not call directly.
  *
  * This method is called by the macro testableAssert, which takes an ostream (not currently used)
  * and the assertion condition to be tested. The condition MUST evaluate to true or false.
  *
  * Example: testableAssert(std::cout, pParams != NULL)
  */
bool assertionLogger(std::ostream &outputer, std::string code, std::string file, int line)
{
   std::cout << "Assertion failed: '" << code << "', File: " << file << ", Line: " << line << std::endl;
   return false;
}

#ifndef testableAssert
#define testableAssert(outputer, code) ((code) ? true : assertionLogger(outputer, #code, __FILE__, __LINE__))
#endif

#endif
