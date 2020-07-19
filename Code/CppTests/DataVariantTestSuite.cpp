/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "ApplicationServices.h"
#include "DataVariant.h"
#include "DateTime.h"
#include "DynamicObject.h"
#include "Filename.h"
#include "Int64.h"
#include "ObjectResource.h"
#include "TestSuiteNewSession.h"
#include "UInt64.h"

#include <limits>
#include <vector>
using namespace std;

static int primes[] = {2, 3, 5, 7, 11, 13, 17, 19};

class DataVariantTestCase : public TestCase
{
public:
   DataVariantTestCase() : TestCase("DataVariant") {}
   template <typename T>
   bool testBasicType()
   {
      bool success = true;
      T tVar = numeric_limits<T>::max();
      DataVariant dvT(tVar);
      T tVar2;
      issea(dvT.getValue(tVar2));
      issea(tVar == tVar2);
      return success;
   }
   template <typename T>
   bool testBasicTypeVector()
   {
      bool success = true;
      vector<T> vecT;
      for (int i=0; i<sizeof(primes)/sizeof(primes[0]); ++i)
      {
         vecT.push_back(static_cast<T>(primes[i]));
      }
      DataVariant dvT(vecT);
      vecT.clear();
      issea(dvT.getValue(vecT));
      for (int i=0; i<sizeof(primes)/sizeof(primes[0]); ++i)
      {
         issea(vecT[i] == primes[i]);
      }
      return success;
   }
   bool run()
   {
      bool success = true;

      // basic C data types
      signed char signedCharVar = numeric_limits<signed char>::max();
      DataVariant dvSignedChar(signedCharVar);
      signedCharVar = 0;
      issea(dvSignedChar.getValue(signedCharVar));
      issea(signedCharVar == numeric_limits<signed char>::max());
      issea(*dvSignedChar.getPointerToValue<signed char>() == numeric_limits<signed char>::max());
      issea(dvSignedChar.getPointerToValue<int>() == NULL);

      unsigned char unsignedCharVar = numeric_limits<unsigned char>::max();
      DataVariant dvUnsignedChar(unsignedCharVar);
      unsignedCharVar = 0;
      issea(dvUnsignedChar.getValue(unsignedCharVar));
      issea(unsignedCharVar == numeric_limits<unsigned char>::max());
      issea(*dvUnsignedChar.getPointerToValue<unsigned char>() == numeric_limits<unsigned char>::max());
      issea(dvUnsignedChar.getPointerToValue<int>() == NULL);

      issea(testBasicType<char>());
      issea(testBasicType<signed char>());
      issea(testBasicType<unsigned char>());
      issea(testBasicType<short>());
      issea(testBasicType<unsigned short>());
      issea(testBasicType<int>());
      issea(testBasicType<unsigned int>());
      issea(testBasicType<long>());
      issea(testBasicType<unsigned long>());
      issea(testBasicType<int64_t>());
      issea(testBasicType<uint64_t>());
      issea(testBasicType<Int64>());
      issea(testBasicType<UInt64>());

      int intVar = numeric_limits<int>::max();
      DataVariant dvint(intVar);
      intVar = 0;
      issea(dvint.getValue(intVar));
      issea(intVar == numeric_limits<int>::max());
      intVar = 0;
      intVar = dv_cast<int>(dvint);
      issea(intVar == numeric_limits<int>::max());
      const int *pIntVar = dv_cast<int>(&dvint);
      issea(pIntVar != NULL);
      issea(*pIntVar == numeric_limits<int>::max());

      issea(testBasicType<float>());
      issea(testBasicType<double>());

      bool boolVar = true;
      DataVariant dvbool(boolVar);
      boolVar = false;
      issea(dvbool.getValue(boolVar));
      issea(boolVar == true);

      string stringVar = "Hello";
      DataVariant dvString(stringVar);
      stringVar = "World";
      issea(dvString.getValue(stringVar));
      issea(stringVar == "Hello");

      FactoryResource<DateTime> pDateTime;
      pDateTime->set(2006, 3, 11);
      DataVariant dvDateTime(*pDateTime.get());
      pDateTime->set(2001, 9, 11, 10, 42, 07);
      DateTime *pDateTimePtr = dvDateTime.getPointerToValue<DateTime>();
      issea(pDateTimePtr != NULL);
      issea(pDateTime->isTimeValid());
      issea(pDateTimePtr->isTimeValid() == false);
      issea(dv_cast<DateTime>(dvDateTime).getStructured() == pDateTimePtr->getStructured());

      FactoryResource<DynamicObject> pDynObj;
      unsigned char myChar = 'A';
      bool ok = pDynObj->setAttribute( "testChar", myChar );
      issea( ok == true );
      const unsigned char *pChar = pDynObj->getAttribute("testChar").getPointerToValue<unsigned char>();
      issea(pChar != NULL);
      issea(*pChar == 'A');

      myChar = 'B';
      DataVariant adoptedChar( myChar );
      issea(adoptedChar.isValid() );
      ok = pDynObj->adoptAttribute( "testChar", adoptedChar );
      issea( ok == true );
      issea( adoptedChar.isValid() );
      pChar = adoptedChar.getPointerToValue<unsigned char>();
      issea(pChar != NULL);
      issea(*pChar == 'A');
      pChar = pDynObj->getAttribute("testChar").getPointerToValue<unsigned char>();
      issea(pChar != NULL);
      issea(*pChar == 'B');

      myChar = 'C';
      DataVariant newChar( myChar );
      issea(newChar.isValid() );
      ok = pDynObj->adoptAttribute( "newChar", newChar );
      issea( ok == true );
      issea( newChar.isValid() == false );
      pChar = pDynObj->getAttribute("newChar").getPointerToValue<unsigned char>();
      issea(pChar != NULL);
      issea(*pChar == 'C');

      signed char mySChar = 'D';
      DataVariant newSChar( mySChar );
      issea(newSChar.isValid() );
      ok = pDynObj->adoptAttribute( "newSChar", newSChar );
      issea( ok == true );
      issea( newSChar.isValid() == false );
      signed char* pSChar = pDynObj->getAttribute("newSChar").getPointerToValue<signed char>();
      issea(pSChar != NULL);
      issea(*pSChar == 'D');

      // Test deep copy of the DynamicObject
      DataVariant dvDynObj(*pDynObj.get());
      pDynObj->clear();
      DynamicObject *pDynObjPtr = dvDynObj.getPointerToValue<DynamicObject>();
      issea(pDynObjPtr != NULL);
      pChar = pDynObjPtr->getAttribute("testChar").getPointerToValue<unsigned char>();
      issea(pChar != NULL);
      issea(*pChar == 'B');
      pChar = pDynObjPtr->getAttribute("newChar").getPointerToValue<unsigned char>();
      issea(pChar != NULL);
      issea(*pChar == 'C');

#if defined(WIN_API)
      string filename1 = "C:/some/path.txt";
      string filename2 = "C:/some/other/path.txt";
      string filename3 = "C:/yet/another/path.txt";
      string filename4 = "D:/some/path.txt";
      string filename5 = "D:/some/other/path.txt";
      string filename6 = "D:/yet/another/path.txt";
#else
      string filename1 = "/C/some/path.txt";
      string filename2 = "/C/some/other/path.txt";
      string filename3 = "/C/yet/another/path.txt";
      string filename4 = "/D/some/path.txt";
      string filename5 = "/D/some/other/path.txt";
      string filename6 = "/D/yet/another/path.txt";
#endif

      FactoryResource<Filename> pFilename;
      pFilename->setFullPathAndName(filename1);
      DataVariant dvFilename(*pFilename.get());
      DataVariant dvFilename2(dvFilename);
      pFilename->setFullPathAndName(filename2);
      DataVariant dvFilename3(*pFilename.get());
      dvFilename3 = dvFilename2;
      Filename *pFilenamePtr = dvFilename.getPointerToValue<Filename>();
      issea(pFilenamePtr != NULL);
      issea(pFilename->getFullPathAndName() == filename2);
      issea(pFilenamePtr->getFullPathAndName() == filename1);
      Filename *pFilenamePtr2 = dvDateTime.getPointerToValue<Filename>();
      issea(pFilenamePtr2 == NULL);
      pFilenamePtr2 = dvFilename.getPointerToValue<Filename>();
      issea(pFilenamePtr2 != NULL);
      issea(string(pFilenamePtr->getFullPathAndName()) == pFilenamePtr2->getFullPathAndName());
      pFilenamePtr = dvFilename.getPointerToValue<Filename>();
      pFilenamePtr2 = dvFilename2.getPointerToValue<Filename>();
      issea(pFilenamePtr->getFullPathAndName() == filename1);
      issea(pFilenamePtr2->getFullPathAndName() == filename1);
      pFilenamePtr2 = dvFilename3.getPointerToValue<Filename>();
      issea(pFilenamePtr2->getFullPathAndName() == filename1);
      dvFilename3 = DataVariant();
      issea(dvFilename3.isValid() == false);
      pFilenamePtr2 = NULL;
      pFilenamePtr2 = dv_cast<Filename>(&dvFilename);
      issea(pFilenamePtr2 != NULL);
      issea(pFilenamePtr2->getFullPathAndName() == filename1);

      issea(testBasicTypeVector<signed char>());
      issea(testBasicTypeVector<unsigned char>());
      issea(testBasicTypeVector<char>());
      issea(testBasicTypeVector<unsigned short>());
      issea(testBasicTypeVector<short>());
      issea(testBasicTypeVector<unsigned int>());
      issea(testBasicTypeVector<int>());
      issea(testBasicTypeVector<unsigned long>());
      issea(testBasicTypeVector<long>());
      issea(testBasicTypeVector<int64_t>());
      issea(testBasicTypeVector<uint64_t>());
      issea(testBasicTypeVector<UInt64>());
      issea(testBasicTypeVector<Int64>());
      issea(testBasicTypeVector<float>());
      issea(testBasicTypeVector<double>());

      vector<bool> vecBool;
      // primes[i]&4!=0: f,f,t,t,f,f,t,f,f
      for (int i=0; i<sizeof(primes)/sizeof(primes[0]); ++i)
      {
         vecBool.push_back((primes[i]&4) != 0);
      }
      DataVariant dvvecBool(vecBool);
      vecBool.clear();
      issea(dvvecBool.getValue(vecBool));
      for (int i=0; i<sizeof(primes)/sizeof(primes[0]); ++i)
      {
         issea(vecBool[i] == ((primes[i]&4)!=0));
      }

      vector<string> vecString;
      vecString.push_back("Hello");
      vecString.push_back("World");
      DataVariant dvVecString(vecString);
      vecString.clear();
      issea(dvVecString.getValue(vecString));
      issea(vecString.size() == 2);
      issea(vecString[0] == "Hello");
      issea(vecString[1] == "World");

      FactoryResource<Filename> pFilenameRes1;
      FactoryResource<Filename> pFilenameRes2;
      FactoryResource<Filename> pFilenameRes3;
      vector<Filename*> vecFilename;
      pFilenameRes1->setFullPathAndName(filename1);
      pFilenameRes2->setFullPathAndName(filename2);
      pFilenameRes3->setFullPathAndName(filename3);
      vecFilename.push_back(pFilenameRes1.get());
      vecFilename.push_back(pFilenameRes2.get());
      vecFilename.push_back(pFilenameRes3.get());
      DataVariant dvVecFilename(vecFilename);
      pFilenameRes1->setFullPathAndName(filename4);
      pFilenameRes2->setFullPathAndName(filename5);
      pFilenameRes3->setFullPathAndName(filename6);
      vecFilename.clear();
      issea(dvVecFilename.getValue(vecFilename));
      issea(vecFilename.size() == 3);
      issea(vecFilename[0]->getFullPathAndName() == filename1);
      issea(vecFilename[1]->getFullPathAndName() == filename2);
      issea(vecFilename[2]->getFullPathAndName() == filename3);

      return success;
   }
};

class DataVariantTestSuite : public TestSuiteNewSession
{
public:
   DataVariantTestSuite() : TestSuiteNewSession( "DataVariant" )
   {
      addTestCase( new DataVariantTestCase );
   }
};

REGISTER_SUITE( DataVariantTestSuite )
