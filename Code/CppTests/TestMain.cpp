/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <xqilla/xqilla-dom3.hpp>

#include "AppConfig.h"
#include "ApplicationServicesImp.h"
#include "ApplicationWindow.h"
#include "AppVersion.h"
#include "ArgumentList.h"
#include "assert.h"
#include "ConfigurationSettingsImp.h"
#include "DesktopServicesImp.h"
#include "InteractiveApplication.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServicesImp.h"
#include "PlugInResource.h"
#include "TestBedTestUtilities.h"
#include "TestSuite.h"
#include "UtilityServicesImp.h"

#include <QtWidgets/QApplication>
#include <QtOpenGL/qgl.h>

#include <boost/test/impl/debug.ipp>
#include <boost/test/impl/execution_monitor.ipp>
#include <boost/test/utils/basic_cstring/io.hpp>

#if defined( WIN_API )
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;

#ifdef max
//we may need to undefine max so that numeric_limits<T>::max() works properly
#undef max
#endif

int test_main( int argc, char *argv[] );
static void runTests(int &passed, int &total);

class MainCaller
{
public:
   MainCaller( int argc, char** argv ): mArgc( argc ), mpArgv( argv ) {}    
   int operator()()
   {
      return test_main(mArgc, mpArgv);
   }

private:
   int mArgc;
   char** mpArgv;
};

int main( int argc, char *argv[] )
{
   ArgumentList* pArgumentList = NULL;
   pArgumentList = ArgumentList::instance();
   bool catchErrors = false;
   if( pArgumentList != NULL )
   {
      pArgumentList->registerOption( "deployment" );
      pArgumentList->registerOption( "debugDeployment" );
      pArgumentList->set( argc, argv );

      string argValue = pArgumentList->getOption( "catchErrors" );
      if( argValue == "true" )
      {
         catchErrors = true;
      }
   }

   int result = 0;
   if( catchErrors )
   {
      boost::execution_monitor monitor;
      MainCaller caller( argc, argv );
      try
      {
         boost::function0<int> main_func( MainCaller( argc, argv ) );
         result = monitor.execute( main_func );
      }
      catch( boost::execution_exception const& ex )
      {
         cout << "CAUGHT ERROR: " << ex.what() << endl;
         result = ex.code();
      }
   }
   else
   {
      result = test_main( argc, argv ); 
   }

   return result;
}

int test_main( int argc, char *argv[] )
{
   // initialize xerces and xqilla
   try
   {
      XQillaPlatformUtils::initialize();
   }
   catch(const XERCES_CPP_NAMESPACE_QUALIFIER XMLException&)
   {
      printf("Unable to initialize Xerces/XQilla\n");
      return 1;
   }

   AssertionCounter::initialize();

   // Create the Qt application
   //QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);

   QApplication appOpticks( argc, argv );
   appOpticks.setFont( QFont( "Tahoma", 8 ) );

   InteractiveApplication wb(appOpticks);

   printf( "Initializing Opticks\n" );

   bool configSettingsValid = false;
   string configSettingsErrorMsg = "";

   ConfigurationSettingsImp* pSettings = ConfigurationSettingsImp::instance();
   if (pSettings != NULL)
   {
      configSettingsValid = pSettings->isInitialized();
      if (pSettings->getInitializationErrorMsg() != NULL)
      {
         configSettingsErrorMsg = pSettings->getInitializationErrorMsg();
      }
      if (configSettingsValid)
      {
         pSettings->validateInitialization();
         configSettingsValid = pSettings->isInitialized();
         if (pSettings->getInitializationErrorMsg() != NULL)
         {
            configSettingsErrorMsg = pSettings->getInitializationErrorMsg();
         }
      }
   }

   if (!configSettingsValid)
   {
      if (configSettingsErrorMsg.empty())
      {
         configSettingsErrorMsg = "Unable to locate configuration settings";
      }
      cerr << endl << APP_NAME << " ERROR: " << configSettingsErrorMsg << endl;
      return -1;
   }
   else
   {
      if (!configSettingsErrorMsg.empty())
      {
         cerr << endl << APP_NAME << " WARNING: " << configSettingsErrorMsg << endl;
      }
   }

   // Set the application to run in interactive mode
   ApplicationServicesImp* pApp = ApplicationServicesImp::instance();
   if (pApp != NULL)
   {
      pApp->setInteractive();
   }

   unsigned int numProcessors = UtilityServicesImp::instance()->getNumProcessors();
   DataVariant dvNumProcessors(numProcessors);
   pSettings->adoptTemporarySetting(ConfigurationSettings::getSettingThreadCountKey(), dvNumProcessors);

   QCoreApplication::instance()->processEvents();

   // Get the plug-in path from the configuration settings
   string plugPath = pSettings->getPlugInPath();

   DesktopServicesImp* pDs = DesktopServicesImp::instance();
   pDs->useMessageBox(false);
   PlugInManagerServicesImp* pPlugInMgr = PlugInManagerServicesImp::instance();
   pPlugInMgr->buildPlugInList( plugPath );

   // Create the main GUI window
   ApplicationWindow* pAppWindow = new ApplicationWindow();

   // Execute startup plug-ins
   vector<PlugInDescriptor*> plugIns = pPlugInMgr->getPlugInDescriptors();
   
   for (unsigned int i = 0; i < plugIns.size(); ++i)
   {
      PlugInDescriptor* pDescriptor = plugIns.at(i);
      if (pDescriptor == NULL)
      {
         continue;
      }
      if (pDescriptor->isExecutedOnStartup() == true)
      {
         ExecutableResource plugIn(pDescriptor->getName(), string(), NULL, false);
         plugIn->execute();
      }
   }

   // Restore the previous position and visibility state of the toolbars and dock windows
   pAppWindow->restoreConfiguration();

   // Display the main application window
   pAppWindow->show();

   printf( "------------------------\n" );

   int passed = 0, total = 0;
   runTests(passed, total);

   int retValue = 0;
   if (passed != total)
   {
      retValue = 1;
   }

   pAppWindow->close();
   delete pAppWindow;

   return retValue;
}

static void runTests(int &passed, int &total)
{
   TestSuite opticksTestSuite( "Opticks" );

   printf( "Adding tests...\n" );

   //create map of the test suites by name.
   //no test execution code should be performed
   //in the test suite constructors
   map<string, TestSuite*> testSuitesByName;
   vector<TestSuiteFactory*>& factories = TestUtilities::getFactoryVector();
   for (vector<TestSuiteFactory*>::iterator factoryIter = factories.begin();
        factoryIter != factories.end();
        ++factoryIter)
   {
      TestSuiteFactory* pFactory = *factoryIter;
      TestSuite* pSuite = pFactory->createSuite();
      if (pSuite != NULL)
      {
         string suiteName = pSuite->getName();
         testSuitesByName[suiteName] = pSuite;
      }
   }

   // Parse the test list file to determine what test suites and test cases should be executed
   string testListFile;

   ArgumentList* pArgumentList = ArgumentList::instance();
   if (pArgumentList != NULL)
   {
      testListFile = pArgumentList->getOption("testList");
   }
   if(testListFile.empty())
   {
      testListFile = "TestList.txt";
   }

   ifstream listfile(testListFile.c_str());
   if(!listfile.good())
   {
      bool success = true;
      issea(listfile.good());
      return;
   }
   string currentLine;
   while( listfile.eof() == false )
   {
      getline(listfile, currentLine); 
      if( currentLine.empty() == false )
      {
         //we have at least one character
         if( currentLine[0] != '#' )
         {
            //this line is not commented out.
            int colonPos = static_cast<int>( currentLine.find( ':' ) );
            if( colonPos != string::npos )
            {
               string suiteName = currentLine.substr( 0, colonPos );
               string otherText = currentLine.substr( colonPos + 1, currentLine.length() - colonPos - 1 );

               //make sure that the given test suite exists first
               if( testSuitesByName.find( suiteName ) != testSuitesByName.end() )
               {
                  vector<string> disabledTestCases;
                  TestSuite* pCurrentSuite = testSuitesByName[suiteName];
                  opticksTestSuite.addTestCase( pCurrentSuite );
                  testSuitesByName.erase( suiteName );

                  //parse the otherText into a vector of strings, where
                  //each string is space separated.
                  vector<string> testCases;
                  istringstream inputStream( otherText );
                  string item;
                  while( inputStream >> item )
                  {
                     testCases.push_back( item );
                  }

                  //now determine if the testCase is being turned on
                  //or off
                  vector<string>::iterator iterTestCases;
                  string testCaseName;
                  bool removed = true;
                  for( iterTestCases = testCases.begin(); iterTestCases != testCases.end(); iterTestCases++ )
                  {
                     testCaseName = *iterTestCases;
                     if( testCaseName.empty() )
                     {
                        continue;
                     }
                     if( testCaseName[0] == '-' )
                     {
                        removed = true;                        
                     }
                     else if( testCaseName[0] == '+' )
                     {
                        removed = false;
                     }
                     testCaseName = testCaseName.substr( 1, testCaseName.length() - 1 );
                     if( testCaseName == "All" )
                     {
                        if( !removed )
                        {
                           disabledTestCases.clear();
                        }
                        else
                        {
                           list<TestCase*>::iterator iterTC;
                           list<TestCase*> testCaseVec = pCurrentSuite->getAllTestCases();
                           for( iterTC = testCaseVec.begin(); iterTC != testCaseVec.end(); iterTC++ )
                           {
                              TestCase* pTestCase = *iterTC;
                              if( pTestCase == NULL )
                              {
                                 continue;
                              }
                              disabledTestCases.push_back( pTestCase->getName() );
                           }
                        }
                     }
                     else
                     {
                        vector<string>::iterator foundPos;
                        foundPos = find( disabledTestCases.begin(), disabledTestCases.end(), testCaseName );
                        if( removed )
                        {
                           if( foundPos == disabledTestCases.end() )
                           {
                              //the test case was removed, so put it into the
                              //list of disabled test cases
                              disabledTestCases.push_back( testCaseName );
                           }
                        }
                        else
                        {
                           if( foundPos != disabledTestCases.end() )
                           {
                              //the test case was added, so removed it from
                              //the list of disabled test cases.
                              disabledTestCases.erase( foundPos );
                           }
                        }
                     }
                  }
                  pCurrentSuite->setDisabledTestCases( disabledTestCases );
               }
            }
            else
            {
               //we didn't find any colon
               //so just add the test suite.
               if( testSuitesByName.find( currentLine ) != testSuitesByName.end() )
               {
                  opticksTestSuite.addTestCase( testSuitesByName[currentLine] );
                  testSuitesByName.erase( currentLine );
               }
            }
         }
      }
   }

   listfile.close();
   opticksTestSuite.runTest( passed, total );

   printf( "-------------------\nOpticks total: passed %d of %d test case(s)\n", passed, total );
   printf( "%d of %d assertions failed\n", AssertionCounter::getNumFailed(), AssertionCounter::get() );

   if( passed == total )
   {
      printf( "-----------------\nOK\n\n" );
   }
   else
   {
      printf( "\nError summary:\n");
      AssertionFailureLog::print();
      printf( "-----------------\nERRORS\n\n" );
   }

   //delete all of the TestSuites that we not added to the opticksTestSuite
   map<string, TestSuite*>::iterator testIter;
   for( testIter = testSuitesByName.begin(); testIter != testSuitesByName.end(); testIter++ )
   {
      TestSuite* pCurSuite = testIter->second;
      if( pCurSuite != NULL )
      {
         delete pCurSuite;
      }
   }
   testSuitesByName.clear();
}

