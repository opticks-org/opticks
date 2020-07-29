/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "TestCase.h"
#include <cstdio>

using namespace std;

TestCase::TestCase( const string& name ) : mName( name ) {}

string TestCase::getName() const
{
   return mName;
}

TestCase::~TestCase()
{
}

void TestCase::runTest( int &passed, int &total, const string& indent )
{
   printf( "-Entering the %s test...\n", getName().c_str() );
   bool success = run();
   if( !success )
   {
      printf( "    -The %s test failed.\n", getName().c_str() );
   }
   else
   {
      printf( "-The %s test passed successfully.\n", getName().c_str() );
   }
   passed += success;
   total++;
}
