/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "PlugInDescriptor.h"
#include "PlugInResource.h"
#include "PlugInManagerServices.h"
#include "Testable.h"
#include "TestSuiteNewSession.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

// To enable or disable a test, use the name of the plug-in with spaces removed
// eg: To disable the "Data Fusion" plug-in, add "-DataFusion" to the appropriate line of the TestList file
class TestableTestCase : public TestCase
{
public:
   TestableTestCase(const string& plugInName, const string& plugInNameNoSpaces) :
      TestCase(plugInNameNoSpaces), mPlugInName(plugInName) {}
   bool run()
   {
      bool success = true;
      PlugInResource pPlugIn(mPlugInName);
      issearf(pPlugIn.get() != NULL);
      Testable* pTestable = dynamic_cast<Testable*>(pPlugIn.get());
      issearf(pTestable != NULL);

      stringstream stream;
      try
      {
         issea(pTestable->runAllTests(NULL, stream) == true);
      }
      catch (...)
      {
         issea(false);
      }

      if (stream.str().size() > 0)
      {
         cout << mPlugInName << " returned the message \"" << stream.str() << "\"." << endl;
      }

      return success;
   }
private:
   string mPlugInName;
};

class TestableTestSuite : public TestSuiteNewSession
{
public:
   TestableTestSuite() : TestSuiteNewSession("Testable")
   {
      // Add all Testable plug-ins to the suite
      vector<PlugInDescriptor*> plugInDescriptors = Service<PlugInManagerServices>()->getPlugInDescriptors();
      for (vector<PlugInDescriptor*>::iterator iter = plugInDescriptors.begin();
         iter != plugInDescriptors.end(); ++iter)
      {
         const PlugInDescriptor* pPlugInDescriptor = *iter;
         if (pPlugInDescriptor->isTestable() == true)
         {
            const string plugInName = pPlugInDescriptor->getName();

            string plugInNameNoSpaces = plugInName;
            string::iterator endPos = remove(plugInNameNoSpaces.begin(), plugInNameNoSpaces.end(), ' ');
            plugInNameNoSpaces.erase(endPos, plugInNameNoSpaces.end());

            addTestCase(new TestableTestCase(plugInName, plugInNameNoSpaces));
         }
      }
   }
};

REGISTER_SUITE(TestableTestSuite)
