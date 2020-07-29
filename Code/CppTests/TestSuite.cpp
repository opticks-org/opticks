/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DesktopServicesImp.h"
#include "ModelServicesImp.h"
#include "SpatialDataWindow.h"
#include "TestSuite.h"
#include "TestCase.h"

using namespace std;

#include <algorithm>

TestSuite::TestSuite( const string& name ) : TestCase( name )
{
}

TestSuite::~TestSuite()
{
   list<TestCase*>::iterator iter;
   for( iter = mTestCases.begin(); iter != mTestCases.end(); iter++ )
   {
      TestCase* pTestCase = *iter;
      if( pTestCase != NULL )
      {
         delete pTestCase;
      }
   }
   mTestCases.clear();
}

void TestSuite::addTestCase( TestCase* pCase )
{
   mTestCases.push_back( pCase );
}

void TestSuite::setDisabledTestCases( const vector<string>& testList )
{
   mDisabledTestCases = testList;
}

list<TestCase*> TestSuite::getAllTestCases() const
{
   return mTestCases;
}

void TestSuite::runTest( int &passed, int &total, const string& indent )
{
   int numPassed = 0;
   int numRun = 0;
   bool success = true;
   list<TestCase*>::iterator it;
   int casesPassed = 0, casesTotal = 0;
   printf( "%s%s test suite\n", indent.c_str(), getName().c_str() );
   TestCase* pTestCase = NULL;
   for( it = mTestCases.begin(); it != mTestCases.end(); it++ )
   {
      pTestCase = *it;
      if( pTestCase != NULL )
      {
         string testName = pTestCase->getName();
         if( find( mDisabledTestCases.begin(), mDisabledTestCases.end(), testName ) == mDisabledTestCases.end() )
         {
            // only execute the test case if we couldn't
            // find it in the list of disabled test cases
            int tempPassed = 0, tempTotal = 0;
            ( *it )->runTest( tempPassed, tempTotal, indent + "  " );
            casesPassed += tempPassed;
            casesTotal += tempTotal;
            bool runSuccess = ( tempPassed == tempTotal );
            success &= runSuccess;
            numPassed += runSuccess;
            numRun++;
         }
      }
   }

   passed += casesPassed;
   total += casesTotal;

   printf( "  %s%s test suite passed %d of %d run test case(s)", indent.c_str(), getName().c_str(), numPassed, numRun );

   size_t numSkipped = mTestCases.size() - numRun;
   if( numSkipped > 0 )
   {
      printf( ", %d skipped", (int)numSkipped );
   }
   printf( "\n" );
}
