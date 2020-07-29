/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "TestSuiteNewSession.h"

#include "SessionManagerImp.h"

using namespace std;

#include <algorithm>

TestSuiteNewSession::TestSuiteNewSession( const string& name ) : TestSuite( name )
{
}

TestSuiteNewSession::~TestSuiteNewSession()
{
}

void TestSuiteNewSession::runTest( int &passed, int &total, const string& indent )
{
   SessionManagerImp *pManagerImp = SessionManagerImp::instance();
   pManagerImp->newSession();

   TestSuite::runTest(passed, total, indent);
}
