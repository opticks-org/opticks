/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>

#include "AppConfig.h"
#include "AppVerify.h"
#include "Classification.h"
#include "DataFusion.h"
#include "DataFusionTests.h"
#include "DataFusionTools.h"
#include "DateTime.h"
#include "DimensionDescriptor.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "Poly2D.h"
#include "Polywarp.h"
#include "Progress.h"
#include "ProgressTracker.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "TestUtilities.h"

#include <iostream>
#include <fstream>
#include <string>
#if defined(UNIX_API)
#include <unistd.h>
#endif

// don't sleep on Windows during testing
#if defined(WIN_API)
#define sleep(x)
#endif

using namespace std;

const double SMALL_VALUE = 1e-4;

bool compareDatesFunc(ostream& stream, const char* date1Name, const char* date2Name,
                     const DateTime* pDate1, const DateTime* pDate2)
{
   if (pDate1->getStructured() != pDate2->getStructured())
   {
      stream << "Dates " + string(date1Name) + " and " + string(date2Name) + " are not equal!" << endl;
      return false;
   }
   return true;
}

#define compareDates(stream, date1, date2) compareDatesFunc(stream, #date1, #date2, date1, date2)

// begin PolywarpTests
PolywarpTests::PolywarpTests(ostream& output, ProgressTracker& tracker) : Test(output, tracker)
{
   myStage = ProgressTracker::Stage("Polywarp Tests", "app", "7EC610DE-0CC6-4a75-B65D-CAB022706268", 100);
   mStages.push_back(ProgressTracker::Stage("Positive Shift Test", "app", "E32556B9-4590-4670-B5E1-4BE6BEEB10D9", 20));
   mStages.push_back(ProgressTracker::Stage("Negative Shift Test", "app", "CF284597-EE1F-412d-9E7C-E10A313B0AA6", 20));
   mStages.push_back(ProgressTracker::Stage("Positive Shift and Scale Test", "app",
                                            "9086D23D-D7F6-4aff-847D-47AE787731E4", 20));
   mStages.push_back(ProgressTracker::Stage("Negative Shift and Scale Test", "app",
                                            "B9C9569C-BFCD-4f5c-BD4F-58B6E4063D26", 20));
   mStages.push_back(ProgressTracker::Stage("Variable Shift Test", "app", "76F00BAE-E721-41b8-8E76-6C8D324D4ADA", 20));
}

bool PolywarpTests::run(double pause)
{
   bool bSuccess = true;

   ProgressSubdivision division(&mProgressTracker, mStages);
   bSuccess = positiveShiftTest();
   if (bSuccess)
   {
      sleep(pause);
      bSuccess = negativeShiftTest();
   }
   if (bSuccess)
   {
      sleep(pause);
      bSuccess = positiveShiftAndScaleTest();
   }
   if (bSuccess)
   {
      sleep(pause);
      bSuccess = negativeShiftAndScaleTest();
   }
   if (bSuccess)
   {
      sleep(pause);
      bSuccess = varyXShiftTest();
   }
   sleep(pause);
   return bSuccess;
}

void PolywarpTests::setupInputMatrices(Vector<double>& xP, Vector<double>& yP,
                                       Vector<double>& xS, Vector<double>& yS,
                                       Vector<double>& kX, Vector<double>& kY,
                                       Vector<double>& expectedKX, Vector<double>& expectedKY)
{
   //Polywarp input declarations
   xP = yP = xS = yS = Vector<double>(4);
   kX = kY = expectedKX = expectedKY = Vector<double>(4);

   xP[0] = 36.0;
   xP[1] = 36.0;
   xP[2] = 53.0;
   xP[3] = 53.0;

   yP[0] = 2.0;
   yP[1] = 25.0;
   yP[2] = 25.0;
   yP[3] = 2.0;
}

bool PolywarpTests::verifyVector(ProgressTracker::Stage& s, const Vector<double>& results, string name)
{
   unsigned int i = 0;
   bool bAllGood = true;
   int workDone = 0;
   for (i = 0; i < results.size(); i++)
   {
      workDone = static_cast<int>((i+1)*100/static_cast<double>(results.size()));
      if (fabs(results.at(i)) > SMALL_VALUE)
      {
         string msg = name + " Failed! " + "Value[" + QString::number(i).toStdString() + "] = " +
            QString::number(fabs(results[i])).toStdString() + " and should be smaller than " +
            QString::number(SMALL_VALUE).toStdString();
         mOutputStream << msg << endl;
         mProgressTracker.report(msg, workDone, WARNING);

         bAllGood = false;
      }
      else
      {
         mProgressTracker.report("Verifying Vectors...", workDone, NORMAL);
      }
   }
   return bAllGood;
}

bool PolywarpTests::positiveShiftTest()
{
   bool bSuccess = true;

   // POLYWARP TEST 1 - Constant Positive Shift
   Vector<double> xP;
   Vector<double> yP;
   Vector<double> xS;
   Vector<double> yS;
   Vector<double> kX;
   Vector<double> kY;
   Vector<double> expectedKX;
   Vector<double> expectedKY;
   setupInputMatrices(xP, yP, xS, yS, kX, kY, expectedKX, expectedKY);

   double xshift = 5.0;
   double yshift = 3.0;

   xS[0] = xP[0] + xshift;
   xS[1] = xP[1] + xshift; 
   xS[2] = xP[2] + xshift;
   xS[3] = xP[3] + xshift;
   yS[0] = yP[0] + yshift;
   yS[1] = yP[1] + yshift; 
   yS[2] = yP[2] + yshift;
   yS[3] = yP[3] + yshift;
   
   expectedKX[0] = xshift;
   expectedKX[1] = 0.0;
   expectedKX[2] = 1.0;
   expectedKX[3] = 0.0;

   expectedKY[0] = yshift;
   expectedKY[1] = 1.0;
   expectedKY[2] = 0.0;
   expectedKY[3] = 0.0;

   // int workIncrement = (maxWork-minWork)/2;
   polywarp(xS, yS, xP, yP, kX, kY, 1, mProgressTracker);

   Vector<double> resultsX = kX-expectedKX;
   Vector<double> resultsY = kY-expectedKY;

   bSuccess = verifyVector(myStage.getActiveStage(), resultsX, "Polywarp-1: X Results");
   if (bSuccess)
   {
      bSuccess = verifyVector(myStage.getActiveStage(), resultsY, "Polywarp-1: Y Results");
   }

   mProgressTracker.nextStage();
   return bSuccess;
}

bool PolywarpTests::negativeShiftTest()
{
   bool bSuccess = true;

   //POLYWARP TEST 2 - Constant Negative Shift
   Vector<double> xP;
   Vector<double> yP;
   Vector<double> xS;
   Vector<double> yS;
   Vector<double> kX;
   Vector<double> kY;
   Vector<double> expectedKX;
   Vector<double> expectedKY;
   setupInputMatrices(xP, yP, xS, yS, kX, kY, expectedKX, expectedKY);

   double xshift = -5.25;
   double yshift = -3.3;

   xS[0] = xP[0] + xshift;
   xS[1] = xP[1] + xshift; 
   xS[2] = xP[2] + xshift;
   xS[3] = xP[3] + xshift;
   yS[0] = yP[0] + yshift;
   yS[1] = yP[1] + yshift; 
   yS[2] = yP[2] + yshift;
   yS[3] = yP[3] + yshift;

   expectedKX[0] = xshift;
   expectedKX[1] = 0.0;
   expectedKX[2] = 1.0;
   expectedKX[3] = 0.0;

   expectedKY[0] = yshift;
   expectedKY[1] = 1.0;
   expectedKY[2] = 0.0;
   expectedKY[3] = 0.0;

   polywarp (xS, yS, xP, yP, kX, kY, 1, mProgressTracker);

   Vector<double> resultsX = kX-expectedKX;
   Vector<double> resultsY = kY-expectedKY;

   bSuccess = verifyVector(myStage.getActiveStage(), resultsX, "Polywarp-2: X Results");
   if (bSuccess)
   {
      bSuccess = verifyVector(myStage.getActiveStage(), resultsY, "Polywarp-2: Y Results");
   }

   mProgressTracker.nextStage();
   return bSuccess;
}

bool PolywarpTests::positiveShiftAndScaleTest()
{
   bool bSuccess = true;

   //POLYWARP TEST 3 - Constant Positive Shift and Scale
   Vector<double> xP;
   Vector<double> yP;
   Vector<double> xS;
   Vector<double> yS;
   Vector<double> kX;
   Vector<double> kY;
   Vector<double> expectedKX;
   Vector<double> expectedKY;
   setupInputMatrices(xP, yP, xS, yS, kX, kY, expectedKX, expectedKY);

   double xshift = 2.0;
   double yshift = 6.3;
   double xscale = 2.0;
   double yscale = 3.5;

   xS[0] = xP[0] * xscale + xshift;
   xS[1] = xP[1] * xscale + xshift;
   xS[2] = xP[2] * xscale + xshift;
   xS[3] = xP[3] * xscale + xshift;
   yS[0] = yP[0] * yscale + yshift;
   yS[1] = yP[1] * yscale + yshift;
   yS[2] = yP[2] * yscale + yshift;
   yS[3] = yP[3] * yscale + yshift;

   expectedKX[0] = xshift;
   expectedKX[1] = 0.0;
   expectedKX[2] = xscale;
   expectedKX[3] = 0.0;

   expectedKY[0] = yshift;
   expectedKY[1] = yscale;
   expectedKY[2] = 0.0;
   expectedKY[3] = 0.0;

   polywarp (xS, yS, xP, yP, kX, kY, 1, mProgressTracker);

   Vector<double> resultsX = kX-expectedKX;
   Vector<double> resultsY = kY-expectedKY;

   bSuccess = verifyVector(myStage.getActiveStage(), resultsX, "Polywarp-3: X Results");
   if (bSuccess)
   {
      bSuccess = verifyVector(myStage.getActiveStage(), resultsY, "Polywarp-3: Y Results");
   }

   mProgressTracker.nextStage();
   return bSuccess;
}

bool PolywarpTests::negativeShiftAndScaleTest()
{
   bool bSuccess = true;

   //POLYWARP TEST 4 - Constant Negative Shift and Scale
   Vector<double> xP;
   Vector<double> yP;
   Vector<double> xS;
   Vector<double> yS;
   Vector<double> kX;
   Vector<double> kY;
   Vector<double> expectedKX;
   Vector<double> expectedKY;
   setupInputMatrices(xP, yP, xS, yS, kX, kY, expectedKX, expectedKY);

   double xshift = -2.0;
   double yshift = -6.3;
   double xscale = -2.0;
   double yscale = -3.5;

   xS[0] = xP[0] * xscale + xshift;
   xS[1] = xP[1] * xscale + xshift; 
   xS[2] = xP[2] * xscale + xshift;
   xS[3] = xP[3] * xscale + xshift;
   yS[0] = yP[0] * yscale + yshift;
   yS[1] = yP[1] * yscale + yshift; 
   yS[2] = yP[2] * yscale + yshift;
   yS[3] = yP[3] * yscale + yshift;

   expectedKX[0] = xshift;
   expectedKX[1] = 0.0;
   expectedKX[2] = xscale;
   expectedKX[3] = 0.0;
   
   expectedKY[0] = yshift;
   expectedKY[1] = yscale;
   expectedKY[2] = 0.0;
   expectedKY[3] = 0.0;

   polywarp (xS, yS, xP, yP, kX, kY, 1, mProgressTracker);

   Vector<double> resultsX = kX-expectedKX;
   Vector<double> resultsY = kY-expectedKY;

   bSuccess = verifyVector(myStage.getActiveStage(), resultsX, "Polywarp-4: X Results");
   if (bSuccess)
   {
      bSuccess = verifyVector(myStage.getActiveStage(), resultsY, "Polywarp-4: Y Results");
   }

   mProgressTracker.nextStage();
   return bSuccess;
}

bool PolywarpTests::varyXShiftTest()
{
   bool bSuccess = true;

   //POLYWARP TEST 5 - Varying X Shift
   Vector<double> xP;
   Vector<double> yP;
   Vector<double> xS;
   Vector<double> yS;
   Vector<double> kX;
   Vector<double> kY;
   Vector<double> expectedKX;
   Vector<double> expectedKY;
   setupInputMatrices(xP, yP, xS, yS, kX, kY, expectedKX, expectedKY);

   xS[0] = xP[0];
   xS[1] = xP[1] + 2.0; 
   xS[2] = xP[2] + 2.0;
   xS[3] = xP[3];
   yS[0] = yP[0] + 2.0;
   yS[1] = yP[1] + 2.0; 
   yS[2] = yP[2];
   yS[3] = yP[3] - 2.0;

   expectedKX[0] = -0.173913;
   expectedKX[1] = 0.0869565;
   expectedKX[2] = 1.0;
   expectedKX[3] = 0.0;

   expectedKY[0] = 10.8389;
   expectedKY[1] = 0.815857;
   expectedKY[2] = -0.245524;
   expectedKY[3] = 0.00511509;

   polywarp (xS, yS, xP, yP, kX, kY, 1, mProgressTracker);

   Vector<double> resultsX = kX-expectedKX;
   Vector<double> resultsY = kY-expectedKY;

   bSuccess = verifyVector(myStage.getActiveStage(), resultsX, "Polywarp-5: X Results");
   if (bSuccess)
   {
      bSuccess = verifyVector(myStage.getActiveStage(), resultsY, "Polywarp-5: Y Results");
   }

   mProgressTracker.nextStage();
   return bSuccess;
}

// begin Poly2DTests
Poly2DTests::Poly2DTests(ostream& output, ProgressTracker& tracker) : Test(output, tracker)
{
   myStage = ProgressTracker::Stage("Poly2D Tests", "app", "DB0ADBB4-05A9-4d89-9BE9-761E160271EC", 100);

   mStages.push_back(ProgressTracker::Stage("Identity Test", "app", "ABFE1841-CD8F-4b26-80B5-F18F4593F3A7", 20));
   mStages.push_back(ProgressTracker::Stage("Positive Shift Test", "app", "B04EDF2C-58F4-432e-AEEE-E9E1940C0049", 20));
   mStages.push_back(ProgressTracker::Stage("Positive Scale Test", "app", "E1758988-7DB9-4efe-AA73-A98F3DA35A01", 20));
   mStages.push_back(ProgressTracker::Stage("Positive Shift And Scale Test", "app",
                                            "C4D0A1F5-4040-4bb4-AE14-0FCC3DA4E081", 20));
}

bool Poly2DTests::run(double pause)
{
   bool bSuccess = true;

   ProgressSubdivision division(&mProgressTracker, mStages);
   bSuccess = identityTest();
   if (bSuccess)
   {
      sleep(pause);
      bSuccess = positiveShiftTest();
   }
   if (bSuccess)
   {
      sleep(pause);
      bSuccess = positiveScaleTest();
   }
   if (bSuccess)
   {
      sleep(pause);
      bSuccess = positiveShiftAndScaleTest();
   }
   sleep(pause);
   return bSuccess;
}

bool Poly2DTests::runTest(string inputFile, string outputFile, string testName,
                          const Vector<double>& kX, const Vector<double>& kY,
                          unsigned int nx, unsigned int ny, unsigned int newx, unsigned int newy)
{
   string fullPath = TestUtilities::getTestDataPath() + "DataFusion/";
   inputFile = fullPath + inputFile;

   Service<ModelServices> pModel;
   VERIFY(pModel.get() != NULL);

   FactoryResource<Classification> pClass;
   VERIFY(pClass.get() != NULL);

   FactoryResource<DateTime> pDeClassDate;
   VERIFY(pDeClassDate.get() != NULL);
   FactoryResource<DateTime> pDowngradeDate;
   VERIFY(pDowngradeDate.get() != NULL);
   FactoryResource<DateTime> pSecuritySrcDate;
   VERIFY(pSecuritySrcDate.get() != NULL);

   VERIFY(pDeClassDate->set(1941, 12, 7, 5, 29, 03) == true);
   VERIFY(pDeClassDate->isValid() == true);
   // this should be an invalid date because 2015 is not a leap year
   VERIFY(pDowngradeDate->set(2015, 2, 29, 12, 13, 14) != true);
   VERIFY(pDowngradeDate->isValid() != true);
   // this should be a valid date because 2016 is a leap year
   VERIFY(pDowngradeDate->set(2016, 2, 29, 12, 13, 14) == true);
   VERIFY(pDowngradeDate->isValid() == true);
   VERIFY(pSecuritySrcDate->set(1970, 3, 4, 5, 58, 59) == true);
   VERIFY(pSecuritySrcDate->isValid() == true);

   const string S_LEVEL = "C";
   const string S_SYSTEM = "dummy-pc";
   const string S_CODEWORD = "DataFusionTest";
   const string S_FILECONTROL = "Computer";
   const string S_FILERELEASING = "Public";
   const string S_EXEMPTION = "none";
   const string S_COUNTRYCODE = "USA";
   const string S_DESCRIPTION = "This is a test description";
   const string S_AUTHORITY = "none needed";
   const string S_AUTHORITYTYPE = "O";
   const string S_SCN = "12345";
   const string S_FILECOPYNUMBER = "42";
   const string S_NUMCOPIES = "24";
   
   pClass->setLevel(S_LEVEL);
   pClass->setSystem(S_SYSTEM);
   pClass->setCodewords(S_CODEWORD);
   pClass->setFileControl(S_FILECONTROL);
   pClass->setFileReleasing(S_FILERELEASING);
   pClass->setDeclassificationExemption(S_EXEMPTION);
   pClass->setCountryCode(S_COUNTRYCODE);
   pClass->setDescription(S_DESCRIPTION);
   pClass->setAuthority(S_AUTHORITY);
   pClass->setAuthorityType(S_AUTHORITYTYPE);
   pClass->setSecurityControlNumber(S_SCN);
   pClass->setFileCopyNumber(S_FILECOPYNUMBER);
   pClass->setFileNumberOfCopies(S_NUMCOPIES);
   pClass->setDeclassificationDate(pDeClassDate.get());
   pClass->setDowngradeDate(pDowngradeDate.get());
   pClass->setSecuritySourceDate(pSecuritySrcDate.get());

   ModelResource<RasterElement> pInput(RasterUtilities::createRasterElement("InputMatrix", ny, nx, FLT8BYTES,
      true, NULL)); 
   VERIFY(pInput.get() != NULL);

   pInput->getDataDescriptor()->setClassification(pClass.get());

   FactoryResource<DataRequest> pRequest;
   pRequest->setWritable(true);

   unsigned int rowIndex = 0;
   unsigned int colIndex = 0;

   ifstream resultsFile((fullPath+outputFile).c_str());

   if (!resultsFile.good())
   {
      string msg = outputFile + " does not exist or you do not have access!";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }

   // Scope the DataAccessor since it must be destroyed before the ModelResource is destroyed.
   {
      ifstream testFile(inputFile.c_str());

      if (testFile.eof())
      {
         string msg = inputFile + " does not exist or you do not have access!";
         mOutputStream << msg << endl;
         mProgressTracker.report(msg, 100, WARNING);
         return false;
      }

      DataAccessor rmda = pInput->getDataAccessor(pRequest.release());
      if (!rmda.isValid())
      {
         string msg = testName + ": Input matrix data accessor is invalid!";
         mOutputStream << msg << endl;
         mProgressTracker.report(msg, 100, WARNING);
         return false;
      }

      while (!testFile.eof() && rowIndex < ny)
      {
         while (!testFile.eof() && colIndex < nx)
         {
            string str;
            testFile >> str;
            static_cast<double*>(rmda->getRow())[colIndex++] = atof(str.c_str());
         }
         colIndex = 0;
         rmda->nextRow();
         rowIndex++;
      }
      testFile.close();
   }

   string msg = testName + ": Poly2d failed!";
   ModelResource<RasterElement> pOutput(reinterpret_cast<RasterElement*>(NULL));
   try
   {
      pOutput = ModelResource<RasterElement>(poly_2D<double>(
         NULL, pInput.get(), kX, kY, newx, newy, 0, 0, 1, mProgressTracker));
   }
   // If the operation fails due to an exception (bug/unrecoverable error), provide details
   catch (AssertException& exc)
   {
      msg = msg + " Cause: " + exc.getText();
   }
   catch (FusionException& exc)
   {
      msg = msg + " Cause: " + exc.toString();
   }

   if (pOutput.get() == NULL)
   {
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }

   const Classification* pOutputClass = pOutput->getClassification();
   if (pOutputClass == NULL)
   {
      msg = testName + ": pOutput->getClassification() returned NULL!";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }

   if (pOutputClass->getLevel() != S_LEVEL)
   {
      msg = testName + ": pOutputClass->getLevel() != S_LEVEL";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getSystem() != S_SYSTEM)
   {
      msg = testName + ": pOutputClass->getSystem() != S_SYSTEM";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getCodewords() != S_CODEWORD)
   {
      msg = testName + ": pOutputClass->getCodewords() != S_CODEWORD";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getFileControl() != S_FILECONTROL)
   {
      msg = testName + ": pOutputClass->getFileControl() != S_FILECONTROL";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getFileReleasing() != S_FILERELEASING)
   {
      msg = testName + ": pOutputClass->getFileReleasing() != S_FILERELEASING";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getDeclassificationExemption() != S_EXEMPTION)
   {
      msg = testName + ": pOutputClass->getDeclassificationExemption() != S_EXEMPTION";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getCountryCode() != S_COUNTRYCODE)
   {
      msg = testName + ": pOutputClass->getCountryCode() != S_COUNTRYCODE";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getDescription() != S_DESCRIPTION)
   {
      msg = testName + ": pOutputClass->getDescription() != S_DESCRIPTION";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getAuthority() != S_AUTHORITY)
   {
      msg = testName + ": pOutputClass->getAuthority() != S_AUTHORITY";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getAuthorityType() != S_AUTHORITYTYPE)
   {
      msg = testName + ": pOutputClass->getAuthorityType() != S_AUTHORITYTYPE";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getSecurityControlNumber() != S_SCN)
   {
      msg = testName + ": pOutputClass->getSecurityControlNumber() != S_SCN";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getFileCopyNumber() != S_FILECOPYNUMBER)
   {
      msg = testName + ": pOutputClass->getFileCopyNumber() != S_FILECOPYNUMBER";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   if (pOutputClass->getFileNumberOfCopies() != S_NUMCOPIES)
   {
      msg = testName + ": pOutputClass->getFileNumberOfCopies() != S_NUMCOPIES";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }
   const DateTime* pOutputDate = pOutputClass->getDeclassificationDate();
   if (pOutputDate == NULL || pDeClassDate.get() == NULL)
   {
      msg = testName + ": pOutputClass->getDeclassificationDate() == NULL || pDeClassDate.get() == NULL";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }

   if (compareDates(mOutputStream, pOutputDate, pDeClassDate.get()) == false)
   {
      return false;
   }

   pOutputDate = pOutputClass->getDowngradeDate();
   if (pOutputDate == NULL || pDowngradeDate.get() == NULL)
   {
      msg = testName + ": pOutputClass->getDowngradeDate() == NULL || pDowngradeDate.get() == NULL";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }

   if (compareDates(mOutputStream, pOutputDate, pDowngradeDate.get()) == false)
   {
      return false;
   }

   pOutputDate = pOutputClass->getSecuritySourceDate();
   if (pOutputDate == NULL || pSecuritySrcDate.get() == NULL)
   {
      msg = testName + ": pOutputClass->getSecuritySourceDate() == NULL || pSecuritySrcDate.get() == NULL";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 100, WARNING);
      return false;
   }

   if (compareDates(mOutputStream, pOutputDate, pSecuritySrcDate.get()) == false)
   {
      return false;
   }

   ModelResource<RasterElement> pExpected(RasterUtilities::createRasterElement("ExpectedMatrix",
      newy, newx, FLT8BYTES));
   VERIFY(pExpected.get() != NULL);

   // Scope the DataAccessor since it must be destroyed before the ModelResource is destroyed.
   {
      DataAccessor rmda = pExpected->getDataAccessor();
      if (!rmda.isValid())
      {
         msg = testName + ": Expected matrix data accessor is invalid!";
         mProgressTracker.report(msg, 100, WARNING);
         return false;
      }

      rowIndex = 0;
      colIndex = 0;
      while (!resultsFile.eof() && rowIndex < newy)
      {
         while (!resultsFile.eof() && colIndex < newx)
         {
            string str;
            resultsFile >> str;
            static_cast<double*>(rmda->getRow())[colIndex++] = atof(str.c_str());
         }
         colIndex = 0;
         rmda->nextRow();
         rowIndex++;
      }

      resultsFile.close();
   }

   return verifyMatrix(myStage.getActiveStage(), pOutput.get(), pExpected.get(), testName);
}

bool Poly2DTests::verifyMatrix(ProgressTracker::Stage& s, RasterElement* pResults, RasterElement* pExpected,
                               string name)
{
   const double MAX_PCT_ERROR = 1.0;
   double mismatches = 0.0;

   VERIFY(pResults != NULL);
   VERIFY(pExpected != NULL);

   const RasterDataDescriptor* pDescExp = dynamic_cast<const RasterDataDescriptor*>(pExpected->getDataDescriptor());
   const RasterDataDescriptor* pDescRes = dynamic_cast<const RasterDataDescriptor*>(pResults->getDataDescriptor());
   
   VERIFY(pDescExp != NULL);
   VERIFY(pDescRes != NULL);

   unsigned int numRows = pDescRes->getRowCount();
   unsigned int numCols = pDescRes->getColumnCount();

   VERIFY(pDescExp->getRowCount() == pDescRes->getRowCount());
   VERIFY(pDescExp->getColumnCount() == pDescRes->getColumnCount());

   DataAccessor r = pResults->getDataAccessor();
   DataAccessor e = pExpected->getDataAccessor();

   for (unsigned int i = 0; i < pDescRes->getRowCount(); i++)
   {
      VERIFY(r.isValid());
      VERIFY(e.isValid());

      int workDone = static_cast<int>(static_cast<double>((i+1)*100)/pDescRes->getRowCount());

      double* rRow = static_cast<double*>(r->getRow());
      double* eRow = static_cast<double*>(e->getRow());

      string msg = "Verifying Results Matrices...";
      ReportingLevel lvl = NORMAL;

      for (unsigned int j = 0; j < pDescRes->getColumnCount(); j++)
      {
         double value = fabs(rRow[j]-eRow[j]);
         if (value > SMALL_VALUE)
         {
            mismatches++;
            msg = (name + " Failed! " + "Value[" +
                   QString::number(i).toStdString() + "][" +
                   QString::number(j).toStdString() + "] = " +
                   QString::number(value).toStdString() + " and should be smaller than " +
                   QString::number(SMALL_VALUE).toStdString());
            lvl = WARNING;
            mOutputStream << msg << endl;
            mProgressTracker.report(msg, workDone, lvl);
         }
      }
      mProgressTracker.report(msg, workDone, NORMAL);
      e->nextRow();
      r->nextRow();
   }

   double percentError = 100 * mismatches / (numRows * numCols);
   bool bSuccess = (percentError < MAX_PCT_ERROR);

   if (!bSuccess)
   {
      string msg = name + " failed due to a high percent of error!";
      mOutputStream << msg << endl;
      mProgressTracker.report(msg, 0, WARNING);
   }
   return bSuccess;
}

bool Poly2DTests::identityTest()
{
   Vector<double> kX(4);
   Vector<double> kY(4);

   //POLY2D TEST 1 - Identity 
   kX[0] = 1.0;
   kX[1] = 0.0;
   kX[2] = 1.0;
   kX[3] = 0.0;
   
   kY[0] = 1.0;
   kY[1] = 1.0;
   kY[2] = 0.0;
   kY[3] = 0.0;
   
   unsigned int newx = 4;
   unsigned int newy = 4;
   unsigned int nx = 10;
   unsigned int ny = 10;

   bool bSuccess = runTest("IdentityTest.txt", "ExpectedIdentityResults.txt", "Poly2d-1", kX, kY, nx, ny, newx, newy);
   mProgressTracker.nextStage();
   return bSuccess;
}

bool Poly2DTests::positiveShiftTest()
{
   Vector<double> kX(4);
   Vector<double> kY(4);

   //POLY2D TEST 2 - Constant Positive Shift
   kX[0] = 3.0;
   kX[1] = 0.0;
   kX[2] = 1.0;
   kX[3] = 0.0;

   kY[0] = 2.5;
   kY[1] = 1.0;
   kY[2] = 0.0;
   kY[3] = 0.0;

   unsigned int newx = 5;
   unsigned int newy = 5;
   unsigned int nx = 10;
   unsigned int ny = 10;

   bool bSuccess = runTest("ShiftTest.txt", "ExpectedShiftResults.txt", "Poly2d-2", kX, kY, nx, ny, newx, newy);
   mProgressTracker.nextStage();
   return bSuccess;
}

bool Poly2DTests::positiveScaleTest()
{
   Vector<double> kX(4);
   Vector<double> kY(4);

   //POLY2D TEST 3 - Constant Positive Scale
   kX[0] = 3.0;
   kX[1] = 0.0;
   kX[2] = 1.0;
   kX[3] = 0.0;

   kY[0] = 2.5;
   kY[1] = 1.0;
   kY[2] = 0.0;
   kY[3] = 0.0;

   unsigned int newx = 5;
   unsigned int newy = 5;
   unsigned int nx = 10;
   unsigned int ny = 10;

   bool bSuccess = runTest("ScaleTest.txt", "ExpectedScaleResults.txt", "Poly2d-3", kX, kY, nx, ny, newx, newy);
   mProgressTracker.nextStage();
   return bSuccess;
}

bool Poly2DTests::positiveShiftAndScaleTest()
{
   Vector<double> kX(4);
   Vector<double> kY(4);

   //POLY2D TEST 4 - Constant Positive Shift and Scale
   kX[0] = 3.0;
   kX[1] = 0.0;
   kX[2] = 1.0;
   kX[3] = 0.0;

   kY[0] = 2.5;
   kY[1] = 1.0; 
   kY[2] = 0.0;
   kY[3] = 0.0;

   unsigned int newx = 9;
   unsigned int newy = 9;
   unsigned int nx = 10;
   unsigned int ny = 10;

   bool bSuccess = runTest("ShiftandScaleTest.txt", "ExpectedShiftandScaleResults.txt", "Poly2d-4",
      kX, kY, nx, ny, newx, newy);
   mProgressTracker.nextStage();
   return bSuccess;
}

bool DataFusion::runOperationalTests(Progress* progress, ostream& failure)
{
   return true;
}

bool DataFusion::runAllTests(Progress* pProgress, ostream& failure)
{
   bool bSuccess = true;
   vector<ProgressTracker::Stage> vStages;
   
   PolywarpTests pwarpTests(failure, mProgressTracker);
   Poly2DTests p2dTests(failure, mProgressTracker);
   vStages.push_back(pwarpTests.getStage());
   vStages.push_back(p2dTests.getStage());
   
   mProgressTracker.initialize(pProgress, "Running all tests...", "app", "3E4C8879-97FA-4ddb-B6A5-60BAA437E609");
   mProgressTracker.subdivideCurrentStage(vStages);
   bSuccess = pwarpTests.run(1);
   mProgressTracker.nextStage();
   if (bSuccess)
   {
      bSuccess = p2dTests.run(1);
   }  
   return bSuccess;
}
