/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "ConfigurationSettingsImp.h"
#include "ConnectionManager.h"
#include "FileFinder.h"
#include "ModelServicesImp.h"
#include "ObjectFactoryImp.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"

using namespace std;

class FileFinderTest : public TestCase
{
public:
   FileFinderTest() : TestCase("Run") {}
   bool run()
   {
      bool success = true;

      FileFinder *pFileFinder = NULL;
      pFileFinder = reinterpret_cast<FileFinder*>(ObjectFactoryImp::instance()->createObject("FileFinder"));
      issea(pFileFinder != NULL);

      string testDataPath = TestUtilities::getTestDataPath();

      bool ok = false;
      ok = pFileFinder->findFile(testDataPath + "FileFinder", "*.sio");
      issea(ok == true);

      // sio files in dir are: afghan.sio, daytonchip.sio, StandardR03 copy with spaces.sio, tipjul5bands.sio, tipjul5bands_recompute.sio
      string name = "";
      ok = pFileFinder->findNextFile(); //get the first file
      issea(ok == true);
      while(ok)
      {
         name = pFileFinder->getFileName();
         if(name == "afghan.sio")
         {
            ok = pFileFinder->findNextFile();
         }

         else if(name == "daytonchip.sio")
         {
            ok = pFileFinder->findNextFile();
         }

         else if(name == "StandardR03 copy with spaces.sio")
         {
            ok = pFileFinder->findNextFile();
         }

         else if(name == "tipjul5bands.sio")
         {
            ok = pFileFinder->findNextFile();
         }

         else if(name == "tipjul5bands_recompute.sio")
         {
            ok = pFileFinder->findNextFile();
         }
      }

      // try to change the criteria
      ok = pFileFinder->findFile(testDataPath + "FileFinder", "*.*");
      issea(ok == true);

      while(ok == true) // loop until the end of the file list
      {
         ok = pFileFinder->findNextFile();
         name = pFileFinder->getFileName();
         if(name == "." || name == "..")
         {
            issea(pFileFinder->isDots() == true);
            issea(pFileFinder->isDirectory() == true);
         }
         else if (name == "TestDir")
         {
            issea(pFileFinder->isDots() == false);
            issea(pFileFinder->isDirectory() == true);
         }
         else
         {
            issea(pFileFinder->isDots() == false);
            issea(pFileFinder->isDirectory() == false);
         }
      }

      // try to change the criteria again
      ok = pFileFinder->findFile(TestUtilities::getTestDataPath(), "*.i*");
      issea(ok == true);

      double length = 0.0;
      string path = "";
      string fullpath = "";
      string title = "";

      // run a series of tests on two of the files found
      ok = pFileFinder->findNextFile();
      issea(ok == true);
      ok = pFileFinder->findNextFile();
      issea(ok == true);
      ok = pFileFinder->findNextFile();
      issea(ok == true);
      ok = pFileFinder->findNextFile();
      issea(ok == true);
      name = pFileFinder->getFileName(); // should be koreatm.i1
      issea(name == "koreatm.i1");
      length = pFileFinder->getLength();
      issea(length == 37509873);
      path = pFileFinder->getFilePath();
      issea(path == TestUtilities::getTestDataPath());
      ok = pFileFinder->getFileTitle(title);
      issea(ok == true);
      issea(title == "koreatm");
      ok = pFileFinder->getFullPath(fullpath);
      issea(ok == true);
      issea(fullpath == TestUtilities::getTestDataPath() + "koreatm.i1");

      ok = pFileFinder->findNextFile();
      issea(ok == true);
      name = pFileFinder->getFileName(); // should be koreatm.i2
      issea(name == "koreatm.i2");
      length = pFileFinder->getLength();
      issea(length == 37509873);
      path = pFileFinder->getFilePath();
      issea(path == TestUtilities::getTestDataPath());
      ok = pFileFinder->getFileTitle(title);
      issea(ok == true);
      issea(title == "koreatm");
      ok = pFileFinder->getFullPath(fullpath);
      issea(ok == true);
      issea(fullpath == TestUtilities::getTestDataPath() + "koreatm.i2");

      ok = pFileFinder->findFile(TestUtilities::getTestDataPath(),
         "koreatm.i3"); // modified before daylight saving time
      ok = pFileFinder->findNextFile();
      issea(ok == true);

      ok = pFileFinder->findFile(TestUtilities::getTestDataPath() + "ODRM",
         "output.raw"); // modified during daylight saving time
      ok = pFileFinder->findNextFile();
      issea(ok == true);

      ObjectFactoryImp::instance()->destroyObject(pFileFinder, "FileFinder");
      pFileFinder = NULL;

      return success;
   }
};

class FileFinderTwoInstanceTest : public TestCase
{
public:
   FileFinderTwoInstanceTest() : TestCase("TwoInstance") {}
   bool run()
   {
      bool success = true;

      FileFinder *pMyFileFinderOne = NULL;
      pMyFileFinderOne = reinterpret_cast<FileFinder*>(ObjectFactoryImp::instance()->createObject("FileFinder"));
      issea(pMyFileFinderOne != NULL);

      FileFinder *pMyFileFinderTwo = NULL;
      pMyFileFinderTwo = reinterpret_cast<FileFinder*>(ObjectFactoryImp::instance()->createObject("FileFinder"));
      issea(pMyFileFinderTwo != NULL);

      string testDataPath = TestUtilities::getTestDataPath();

      bool okOne = false;
      bool okTwo = false;
      okOne = pMyFileFinderOne->findFile(testDataPath + "FileFinder", "*.sio");
      issea(okOne == true);
      okTwo = pMyFileFinderTwo->findFile(testDataPath + "FileFinder", "*.bip");
      issea(okTwo == true);

      // make sure the two seperate FileFinder instances maintain two seperate file lists
      okOne = pMyFileFinderOne->findNextFile();
      issea(okOne == true);
      okTwo = pMyFileFinderTwo->findNextFile();
      issea(okTwo == true);

      // sio files in dir are: afghan.sio, daytonchip.sio, StandardR03 copy with spaces.sio, tipjul5bands.sio, tipjul5bands_recompute.sio
      // bip files in dir are: cube14000x14000x2x2ulsb.bip, cube1x1x1x1slsb.bip, EnviTest.bip, StandardR03.bip
      string nameOne = "";
      string nameTwo = "";
      double length = 0.0;

      while(okOne || okTwo)
      {
         nameOne = pMyFileFinderOne->getFileName();
         nameTwo = pMyFileFinderTwo->getFileName();

         if(nameTwo == "cube14000x14000x2x2ulsb.bip")
         {
            length = pMyFileFinderTwo->getLength();
            issea(length == 45);
            okTwo = pMyFileFinderTwo->findNextFile();
         }
         else if(nameTwo == "cube1x1x1x1slsb.bip")
         {
            length = pMyFileFinderTwo->getLength();
            issea(length == 37);
            okTwo = pMyFileFinderTwo->findNextFile();
         }
         else if(nameTwo == "EnviTest.bip")
         {
            length = pMyFileFinderTwo->getLength();
            issea(length == 30);
            okTwo = pMyFileFinderTwo->findNextFile();
         }
         else if(nameTwo == "StandardR03.bip")
         {
            length = pMyFileFinderTwo->getLength();
            issea(length == 33);
            okTwo = pMyFileFinderTwo->findNextFile(); // no more bip files
         }

         if(nameOne == "afghan.sio")
         {
            length = pMyFileFinderOne->getLength();
            issea(length == 28);
            okOne = pMyFileFinderOne->findNextFile();
         }     

         else if(nameOne == "daytonchip.sio")
         {
            length = pMyFileFinderOne->getLength();
            issea(length == 32);
            okOne = pMyFileFinderOne->findNextFile();
         }
         else if(nameOne == "StandardR03 copy with spaces.sio")
         {
            length = pMyFileFinderOne->getLength();
            issea(length == 50);
            okOne = pMyFileFinderOne->findNextFile();
         }
         else if(nameOne == "tipjul5bands.sio")
         {
            length = pMyFileFinderOne->getLength();
            issea(length == 34);
            okOne = pMyFileFinderOne->findNextFile();
         }
         else if(nameOne == "tipjul5bands_recompute.sio")
         {
            length = pMyFileFinderOne->getLength();
            issea(length == 44);
            okOne = pMyFileFinderOne->findNextFile(); // no more sio files
         }
      }

      ObjectFactoryImp::instance()->destroyObject(pMyFileFinderOne, "FileFinder");
      pMyFileFinderOne = NULL;
      ObjectFactoryImp::instance()->destroyObject(pMyFileFinderTwo, "FileFinder");
      pMyFileFinderTwo = NULL;

      return success;
   }
};

class FileFinderTestSuite : public TestSuiteNewSession
{
public:
   FileFinderTestSuite() : TestSuiteNewSession("FileFinder")
   {
      addTestCase(new FileFinderTest);
      addTestCase(new FileFinderTwoInstanceTest);
   }
};

REGISTER_SUITE(FileFinderTestSuite)
