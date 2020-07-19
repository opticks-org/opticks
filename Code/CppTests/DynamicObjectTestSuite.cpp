/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationServices.h"
#include "assert.h"
#include "AttachmentPtr.h"
#include "ConfigurationSettingsImp.h"
#include "DesktopServicesImp.h"
#include "DynamicObject.h"
#include "FilenameImp.h"
#include "ModelServicesImp.h"
#include "ObjectResource.h"
#include "SpatialDataWindow.h"
#include "SpatialDataView.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "UtilityServicesImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

class DynamicObjectCreationTest : public TestCase
{
public:
   DynamicObjectCreationTest() : TestCase("Creation") {}

   bool run()
   {
      bool success = true;

      DynamicObject *pDynObj = NULL;
      pDynObj = reinterpret_cast<DynamicObject*>( Service<ObjectFactory>()->createObject( "DynamicObject" ) );
      issea( pDynObj != NULL );

      // make sure it is empty
      int numItems = 0;
      numItems = pDynObj->getNumAttributes();
      issea( numItems == 0 );

      // add attributes
      unsigned char myChar = 'A';
      double myDouble = 3.14159;
      string myString = "This is my test string.";
      bool ok = false;

      ok = pDynObj->setAttribute( "testChar", myChar );
      issea( ok == true );
      ok = pDynObj->setAttribute( "PI", myDouble );
      issea( ok == true );
      ok = pDynObj->setAttribute( "testString", myString );
      issea( ok == true );
      vector<string> vecStrings;
      vecStrings.push_back("Hello World");
      vecStrings.push_back("Foo Bar Baz");
      ok = pDynObj->setAttribute( "test Vec Strings", vecStrings );
      issea( ok == true );

      numItems = pDynObj->getNumAttributes();
      issea( numItems == 4 );

      DataVariant& attrVal = pDynObj->getAttribute( "PI" );
      issea( attrVal.isValid() == true );
      string piValue = attrVal.toXmlString();
      issea( piValue == "3.14159" );
      issea( piValue != "3.1415999" );

      DataVariant var;
      var = pDynObj->getAttribute( "testChar");
      issea( var.isValid() && *var.getPointerToValue<unsigned char>() == 'A' );
      var = pDynObj->getAttribute( "testString");
      issea( var.isValid() && *var.getPointerToValue<string>() == "This is my test string." );
      var = pDynObj->getAttribute( "test Vec Strings" );
      issea( var.isValid() );
      const vector<string>& vecStringRef = dv_cast<vector<string> >(var);
      issea(vecStringRef.size() == 2);
      issea(vecStringRef[0] == "Hello World");
      issea(vecStringRef[1] == "Foo Bar Baz");

      string stringVecAsText = var.toXmlString();
      string typeAsText = var.getTypeName();
      var = DataVariant();
      issea(var.isValid() == false);
      var.fromXmlString(typeAsText, stringVecAsText);
      issea(var.isValid());
      const vector<string>& vecStringRef3 = dv_cast<vector<string> >(var);
      issea(vecStringRef3.size() == 2);
      issea(vecStringRef3[0] == "Hello World");
      issea(vecStringRef3[1] == "Foo Bar Baz");

      // add a 5th attribute
      int myInt = 327;
      ok = pDynObj->setAttribute( "testInt", myInt );
      issea( ok == true );
      numItems = pDynObj->getNumAttributes();
      issea( numItems == 5 );

      // now remove an attribute
      ok = pDynObj->removeAttribute( "testChar" );
      issea( ok == true );
      
      // verify that it is no longer there
      numItems = pDynObj->getNumAttributes();
      issea( numItems == 4 );
      var = DataVariant();
      var = pDynObj->getAttribute( "testChar" );
      issea( var.isValid() == false );

      // now create a 2nd DynamicObject
      DynamicObject *pDynObj2 = NULL;
      pDynObj2 = reinterpret_cast<DynamicObject*>( Service<ObjectFactory>()->createObject( "DynamicObject" ) );
      issea( pDynObj2 != NULL );

      // add 3 attributes
      double myDouble2 = 3.14;
      vector<int> myVector;
      for( int i = 0; i < 5; i++ )
      {
         myVector.push_back( ( i + 1 ) * 2 );  // vector containing 2,4,6,8,10
      }

      ok = pDynObj2->setAttribute( "testChar", myChar );
      issea( ok == true );
      ok = pDynObj2->setAttribute( "PI", myDouble2 ); // same name, different value
      issea( ok == true );
      ok = pDynObj2->setAttribute( "testVector", myVector );
      issea( ok == true );

      numItems = pDynObj2->getNumAttributes();
      issea( numItems == 3 );

      // now merge the two Dynamic Objects
      pDynObj->merge( pDynObj2 );

      // verify the merge
      numItems = pDynObj->getNumAttributes();
      issea( numItems == 6 );
      numItems = pDynObj2->getNumAttributes();
      issea( numItems == 3 );

      // contents of pDynObj should be: PI, testString, testInt, testChar, testVector, test Vec Strings
      var = DataVariant();
      var = pDynObj->getAttribute( "PI" );
      issea( var.isValid() && *var.getPointerToValue<double>() != 3.14159 );
      issea( var.isValid() && *var.getPointerToValue<double>() == 3.14 ); // this value should have overwritten the original
      var = pDynObj->getAttribute( "testString" );
      issea( var.isValid() && *var.getPointerToValue<string>() == "This is my test string." );   
      var = pDynObj->getAttribute( "testInt" );
      issea( var.isValid() && *var.getPointerToValue<int>() == 327 );
      var = pDynObj->getAttribute( "testChar" );
      issea( var.isValid() && *var.getPointerToValue<unsigned char>() == 'A' );

      var = pDynObj->getAttribute( "testVector" );
      vector<int> retrievedVector = *var.getPointerToValue<vector<int> >();
      for( int j = 0; j < 5; j++ )
      {
         issea( retrievedVector.at( j ) == ( j + 1 ) * 2 );
      }

      var = pDynObj->getAttribute( "test Vec Strings" );
      const vector<string>& vecStringRef2 = dv_cast<vector<string> >(var);
      issea(vecStringRef2.size() == 2);
      issea(vecStringRef2[0] == "Hello World");
      issea(vecStringRef2[1] == "Foo Bar Baz");

      // contents of pDynObj2 should be: testChar, PI, testVector
      var = pDynObj2->getAttribute( "testChar" );
      issea( var.isValid() && *var.getPointerToValue<unsigned char>() == 'A' );
      var = pDynObj2->getAttribute( "PI" );
      issea( var.isValid() && *var.getPointerToValue<double>() == 3.14 );
      var = pDynObj2->getAttribute( "testVector" );
      vector<int> retrievedVector2 = *var.getPointerToValue<vector<int> >();
      for( int k = 0; k < 5; k++ )
      {
         issea( retrievedVector2.at( k ) == ( k + 1 ) * 2 );
      }

      // now that the merge has been verified, clear them both
      pDynObj->clear();
      numItems = pDynObj->getNumAttributes();
      issea( numItems == 0 );
      pDynObj2->clear();
      numItems = pDynObj2->getNumAttributes();
      issea( numItems == 0 );

      return success;
   }
};

class DynamicObjectNestingTest : public TestCase
{
public:
   DynamicObjectNestingTest() : TestCase("Nesting") {}
   bool run()
   {
      bool success = true;

      DynamicObject *pDynObj = NULL;
      pDynObj = reinterpret_cast<DynamicObject*>( Service<ObjectFactory>()->createObject( "DynamicObject" ) );
      issea( pDynObj != NULL );

      DynamicObject *pDynObj2 = NULL;
      pDynObj2 = reinterpret_cast<DynamicObject*>( Service<ObjectFactory>()->createObject( "DynamicObject" ) );
      issea( pDynObj2 != NULL );

      DynamicObject *pDynObj3 = NULL;
      pDynObj3 = reinterpret_cast<DynamicObject*>( Service<ObjectFactory>()->createObject( "DynamicObject" ) );
      issea( pDynObj3 != NULL );

      ProcessingLocation inMemory = IN_MEMORY;
      ProcessingLocation origFile = ON_DISK_READ_ONLY;
      unsigned int myInt = 396;
      string myString = "Nested Dynamic Objects are fun.";

      bool ok = false;
      int numItems = 0;
      DataVariant var;

      ok = pDynObj->setAttribute( "dynamicObject2", *pDynObj2 );
      issea( ok == true );
      ok = pDynObj2->setAttribute( "dynamicObject3", *pDynObj3 );
      issea( ok == true );

      ok = pDynObj2->setAttribute( "processingLocation", inMemory );
      issea( ok == true );
      ok = pDynObj2->setAttribute( "testInteger", myInt );
      issea( ok == true );
      ok = pDynObj3->setAttribute( "processingLocation", origFile );
      issea( ok == true );
      ok = pDynObj3->setAttribute( "testString", myString );
      issea( ok == true );

      numItems = pDynObj->getNumAttributes();
      issea( numItems == 1 ); // should contain pDynObj2
      vector<string> attributeNames;
      pDynObj->getAttributeNames( attributeNames );
      issea( attributeNames.size() == 1 );
      issea( attributeNames.at( 0 ) == "dynamicObject2" );
      
      numItems = pDynObj2->getNumAttributes();
      issea( numItems == 3 ); // should contain pDynObj3, processingLocation(inMemory), testInteger
      attributeNames.clear();
      pDynObj2->getAttributeNames( attributeNames );
      issea( attributeNames.size() == 3 );

      numItems = pDynObj3->getNumAttributes();
      issea( numItems == 2 ); // should contain processingLocation(origFile), testString
      attributeNames.clear();
      pDynObj3->getAttributeNames( attributeNames );
      issea( attributeNames.size() == 2 );

      // try to do a merge
      pDynObj2->merge( pDynObj3 );

      // check the result
      numItems = pDynObj->getNumAttributes();
      issea( numItems == 1 ); // should contain pDynObj2
      attributeNames.clear();
      pDynObj->getAttributeNames( attributeNames );
      issea( attributeNames.size() == 1 );
      issea( attributeNames.at( 0 ) == "dynamicObject2" );
      ok = pDynObj->getAttribute( "dynamicObject2" ).isValid();
      issea( ok == true );

      numItems = pDynObj2->getNumAttributes();
      issea( numItems == 4 ); // should contain pDynObj3, processingLocation(origFile), testInteger, testString      
      attributeNames.clear();
      pDynObj2->getAttributeNames( attributeNames );
      issea( attributeNames.size() == 4 );
      ok = pDynObj2->getAttribute( "dynamicObject3" ).isValid();
      issea( ok == true );
      var = pDynObj2->getAttribute( "processingLocation" );
      issea( var.isValid() && *var.getPointerToValue<ProcessingLocation>() == ON_DISK_READ_ONLY );
      var = pDynObj2->getAttribute( "testInteger" );
      issea( var.isValid() && *var.getPointerToValue<unsigned int>() == 396 );
      var = pDynObj2->getAttribute( "testString" );
      issea( var.isValid() && *var.getPointerToValue<string>() == "Nested Dynamic Objects are fun." );

      numItems = pDynObj3->getNumAttributes();
      issea( numItems == 2 ); // should contain processingLocation(origFile), testString
      attributeNames.clear();
      pDynObj3->getAttributeNames( attributeNames );
      issea( attributeNames.size() == 2 );
      var = pDynObj3->getAttribute( "processingLocation" );
      issea( var.isValid() && *var.getPointerToValue<ProcessingLocation>() == ON_DISK_READ_ONLY );
      var = pDynObj3->getAttribute( "testString" );
      issea( var.isValid() && *var.getPointerToValue<string>() == "Nested Dynamic Objects are fun." );

      return success;
   }
};

class DynamicObjectSerializeTestCase : public TestCase
{
public:
   DynamicObjectSerializeTestCase() : TestCase("Serialize") {}
   bool run()
   {
      bool success = true;
      FILE *filePtr = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }

      filePtr = fopen( ( tempHome + "/dynObjTest.tmp" ).c_str(), "w" );
      issea( filePtr != NULL );

      DynamicObject *pDynObj1 = NULL;
      pDynObj1 = reinterpret_cast<DynamicObject*>( Service<ObjectFactory>()->createObject( "DynamicObject" ) );
      issea( pDynObj1 != NULL );

      string myString = "Mary had a little lamb.";
      vector<string> myVector;
      myVector.push_back( "Mary" );
      myVector.push_back( "had a" );
      myVector.push_back( "little" );
      myVector.push_back( "lamb." );
      unsigned short myUShort = 12345;
      char myChar = 'Q';
      bool ok = false;
      int numItems = 0;
      DataVariant var;

      FactoryResource<Filename> pFilenameRes;
#if defined(WIN_API)
      string filename = "C:/some good/path.txt";
#else
      string filename = "/C/some good/path.txt";
#endif
      pFilenameRes->setFullPathAndName(filename);
      issea( pFilenameRes->getFullPathAndName() == filename );

      FactoryResource<Filename> pFilenameRes2;
#if defined(WIN_API)
      string filename2 = "C:/path with/spaces/file with spaces.txt";
#else
      string filename2 = "/path with/spaces/file with spaces.txt";
#endif
      pFilenameRes2->setFullPathAndName(filename2);
      issea( pFilenameRes2->getFullPathAndName() == filename2);
      vector<Filename*> myFilenameVec;
      myFilenameVec.push_back(pFilenameRes.get());
      myFilenameVec.push_back(pFilenameRes2.get());

      ok = pDynObj1->setAttribute( "testVector", myVector );
      issea( ok == true );
      ok = pDynObj1->setAttribute( "testString", myString );
      issea( ok == true );
      ok = pDynObj1->setAttribute( "testUShort", myUShort );
      issea( ok == true );
      ok = pDynObj1->setAttribute( "testChar", myChar );
      issea( ok == true );
      ok = pDynObj1->setAttribute( "testFilename", *pFilenameRes.get() );
      issea( ok == true );
      ok = pDynObj1->setAttribute( "testFilenameVec", myFilenameVec );
      issea( ok == true );

      numItems = pDynObj1->getNumAttributes();
      issea( numItems == 6 );

      // test serialize/deserialize for dynamic object
      XMLWriter xwrite( "DynamicObject" );
      ok = pDynObj1->toXml( &xwrite );
      issea( ok == true );
      xwrite.writeToFile( filePtr );
      fclose( filePtr );

      DynamicObject *pDynObj2 = NULL;
      pDynObj2 = reinterpret_cast<DynamicObject*>( Service<ObjectFactory>()->createObject( "DynamicObject" ) );
      issea( pDynObj2 != NULL );

      MessageLog *pLog( UtilityServicesImp::instance()->getMessageLog()->getLog( "session" ) );
      XmlReader xml( pLog, false );
      
      FilenameImp inputFile( tempHome + "/dynObjTest.tmp" );
      
      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc( NULL );
      try
      {
         doc = xml.parse( &inputFile );
      }
      catch( XmlBase::XmlException & )
      {
         // do nothing
      }
      DOMElement *rootelement( NULL );
      if( doc != NULL )
      {
         rootelement = doc->getDocumentElement();
      }
      issea( rootelement != NULL );

      if( success )
      {
         unsigned int formatVersion = atoi( A( rootelement->getAttribute( X( "version" ) ) ) );
         for( DOMNode *chld = rootelement->getFirstChild(); chld != NULL; chld = chld->getNextSibling() )
         {
            if( XMLString::equals( chld->getNodeName(), X( "DynamicObject" ) ) )
            {
               ok = pDynObj2->fromXml( chld, formatVersion );
               issea( ok == true );

               var = pDynObj2->getAttribute( "testString" );
               issea( var.isValid() && *var.getPointerToValue<string>() == "Mary had a little lamb." );
               var = pDynObj2->getAttribute( "testUShort" );
               issea( var.isValid() && *var.getPointerToValue<unsigned short>() == 12345 );
               var = pDynObj2->getAttribute( "testChar" );
               issea( var.isValid() && *var.getPointerToValue<char>() == 'Q' );

               var = pDynObj2->getAttribute( "testVector" );
               const vector<string>& vecStringRef2 = dv_cast<vector<string> >(var);
               issea(vecStringRef2.size() == 4);
               issea(vecStringRef2[0] == "Mary");
               issea(vecStringRef2[1] == "had a");
               issea(vecStringRef2[2] == "little");
               issea(vecStringRef2[3] == "lamb.");

               Filename *pFilename = NULL;
               var = pDynObj2->getAttribute( "testFilename" );
               issea( var.isValid() );
               if (success)
               {
                  pFilename = var.getPointerToValue<Filename>();
                  issea(pFilename != NULL);
                  issea(pFilename->getFullPathAndName() == filename);
               }

               var = pDynObj2->getAttribute( "testFilenameVec" );
               issea( var.isValid() );
               const vector<Filename*>& vecFilenameRef = dv_cast<vector<Filename*> >(var);
               issea(vecFilenameRef.size() == 2);
               issea( *(vecFilenameRef[0]) == *(pFilenameRes.get()) );
               issea( *(vecFilenameRef[1]) == *(pFilenameRes2.get()) );               
            }
         }
      }

      return success;
   }
};

class DynamicObjectNestedSerializeTestCase : public TestCase
{
public:
   DynamicObjectNestedSerializeTestCase() : TestCase("NestedSerialize") {}
   bool run()
   {
      bool success = true;
      FILE *filePtr = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }
      filePtr = fopen( ( tempHome + "/dynObjNestTest.tmp" ).c_str(), "w" );
      issea( filePtr != NULL );

      DynamicObject *pDynObj1 = NULL;
      pDynObj1 = reinterpret_cast<DynamicObject*>( Service<ObjectFactory>()->createObject( "DynamicObject" ) );
      issea( pDynObj1 != NULL );

      DynamicObject *pDynObj2 = NULL;
      pDynObj2 = reinterpret_cast<DynamicObject*>( Service<ObjectFactory>()->createObject( "DynamicObject" ) );
      issea( pDynObj2 != NULL );

      string myString1 = "this is a test";
      bool ok = false;
          
      ok = pDynObj1->setAttribute( "myTestString", myString1 );
      issea( ok == true );
      ok = pDynObj2->setAttribute( "myDynamicObject", *pDynObj1 );
      issea( ok == true );
  
      // copy of dynObj1 which is inside dynObj2
      const DynamicObject* pDynObj1b = NULL;
      pDynObj1b = pDynObj2->getAttribute( "myDynamicObject" ).getPointerToValue<DynamicObject>();
      issea( pDynObj1b != NULL );

      // test serialize/deserialize for nested object
      XMLWriter xwrite( "DynamicObject" );
      ok = pDynObj2->toXml( &xwrite );
      issea( ok == true );
      xwrite.writeToFile( filePtr );
      fclose( filePtr );

      MessageLog *pLog( UtilityServicesImp::instance()->getMessageLog()->getLog( "session" ) );
      XmlReader xml( pLog, false );
      
      FilenameImp inputFile( tempHome + "/dynObjNestTest.tmp" );
      
      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc( NULL );
      try
      {
         doc = xml.parse( &inputFile );
      }
      catch( XmlBase::XmlException & )
      {
         // do nothing
      }
      DOMElement *rootelement( NULL );
      if( doc != NULL )
      {
         rootelement = doc->getDocumentElement();
      }
      issea( rootelement != NULL );

      if( success )
      {
         DynamicObject *pDynObj3 = NULL;
         pDynObj3 = reinterpret_cast<DynamicObject*>( Service<ObjectFactory>()->createObject( "DynamicObject" ) );
         issea( pDynObj3 != NULL );

         if( success )
         {
            unsigned int formatVersion = atoi( A( rootelement->getAttribute( X( "version" ) ) ) );
            for( DOMNode *chld = rootelement->getFirstChild(); chld != NULL; chld = chld->getNextSibling() )
            {
               if( XMLString::equals( chld->getNodeName(), X( "DynamicObject" ) ) )
               {
                  ok = pDynObj3->fromXml( chld, formatVersion );
                  issea( ok == true );

                  const DynamicObject *pDynObj4 = NULL;
                  pDynObj4 = pDynObj3->getAttribute( "myDynamicObject" ).getPointerToValue<DynamicObject>();
                  issea( pDynObj4 != NULL );
                  unsigned int num = pDynObj3->getNumAttributes();
                  issea( num == 1 );

                  if( success )
                  {
                     const string *pMyString2 = pDynObj4->getAttribute( "myTestString" ).getPointerToValue<string>();

                     // same string in the nested obj?
                     issea( pMyString2 != NULL );
                     issea( *pMyString2 == myString1 );
                  }
               }
            }
         }
      }

      return success;
   }
};

class DynamicObjectObserver
{
public:
   DynamicObjectObserver(Subject& subj) :
      mSubject(&subj, SIGNAL_NAME(Subject, Modified), Slot(this, &DynamicObjectObserver::modified)), mModifiedCount(0)
   {}
   virtual ~DynamicObjectObserver()
   {}
   void modified(Subject &subject, const string &signal, const boost::any &v)
   {
      mModifiedCount++;
   }
   unsigned int mModifiedCount;
   AttachmentPtr<Subject> mSubject;
};

class DynamicObjectAdoptiveMergeTestCase : public TestCase
{
public:
   DynamicObjectAdoptiveMergeTestCase() : TestCase("AdoptiveMerge") {}
   bool run()
   {
      bool success = true;

      DataVariant dummyInt(static_cast<int>(1));
      int* pSourceInt = dv_cast<int>(&dummyInt);
      {
         FactoryResource<DynamicObject> pSource;
         FactoryResource<DynamicObject> pDest;
         DynamicObjectObserver pDestObs(*pDest);
         //test adopting a NULL parented dynamic object into a normal dynamic object.
         pDest->setAttribute("foo", *pSource.get());
         issea(pDestObs.mModifiedCount == 1);
         DataVariant& dvFoo = pDest->getAttribute("foo");
         DynamicObject* pSourceObj = dv_cast<DynamicObject>(&dvFoo);
         pSourceObj->setAttribute("newAttr", 1); //make sure that mutating pSource will
            //cause it to notify pDest properly without crashing.
         issea(pDestObs.mModifiedCount == 2);

         FactoryResource<DynamicObject> pDest2;
         DynamicObjectObserver pDest2Obs(*pDest2);
         pDest2->adoptAttribute("bar", dvFoo); //move between parents
         issea(pDestObs.mModifiedCount == 2);
         issea(pDest2Obs.mModifiedCount = 1);
         issea(dv_cast<DynamicObject>(&pDest2->getAttribute("bar")) == pSourceObj);
         issea(dv_cast<DynamicObject>(&pDest->getAttribute("foo")) == NULL);
         pSourceObj->setAttribute("newAttr", 2); //make sure that mutating pSource will
            //cause it to notify pDest2 properly without crashing.
         issea(pDestObs.mModifiedCount == 2);
         issea(pDest2Obs.mModifiedCount == 2);
         pSourceObj->setAttribute("newAttr2", 1); //make sure that mutating pSource will
            //cause it to notify pDest2 properly without crashing.
         issea(pDest2Obs.mModifiedCount == 3);

         pDest2->adoptAttribute("bar", dummyInt);
         issea(pDest2Obs.mModifiedCount == 4);
         issea(dv_cast<int>(&pDest2->getAttribute("bar")) == pSourceInt);
         issea(dummyInt.getPointerToValue<DynamicObject>() == pSourceObj);
         pSourceObj = dv_cast<DynamicObject>(&dummyInt);
         pSourceObj->setAttribute("newAttr", 3); //make sure that mutating pSource will
            //cause it to no longer notify any parents
         issea(pDestObs.mModifiedCount == 2);
         issea(pDest2Obs.mModifiedCount == 4);
      }
      DynamicObject* pSourceObj = dv_cast<DynamicObject>(&dummyInt);
      pSourceObj->setAttribute("newAttr", 4); //make sure that mutating pSource will
         //cause it to no longer notify any parents     

      FactoryResource<DynamicObject> pDest;

      pDest->setAttributeByPath("/a/b/c", 1);
      pDest->setAttributeByPath("/a/b/d", 2);
      pDest->setAttributeByPath("/a/e", 3);
      pDest->setAttributeByPath("/a/f", 4);

      {
         FactoryResource<DynamicObject> pSource;
         pSource->setAttributeByPath("/a/b/c", 5);
         pSource->setAttributeByPath("/a/e", 6);
         pSource->setAttributeByPath("/a/g", 7);
         pSource->setAttributeByPath("/h", 8);

         pDest->adoptiveMerge(pSource.get());
      }

      issearf(dv_cast<int>(pDest->getAttributeByPath("/a/b/c")) == 5);
      issearf(dv_cast<int>(pDest->getAttributeByPath("/a/b/d")) == 2);
      issearf(dv_cast<int>(pDest->getAttributeByPath("/a/e")) == 6);
      issearf(dv_cast<int>(pDest->getAttributeByPath("/a/f")) == 4);
      issearf(dv_cast<int>(pDest->getAttributeByPath("/a/g")) == 7);
      issearf(dv_cast<int>(pDest->getAttributeByPath("/h")) == 8);

      {
         FactoryResource<DynamicObject> pSource;
         pSource->setAttributeByPath("/a/b/c", 9);
         pSource->setAttributeByPath("/a/b/d", 10);
         pSource->setAttributeByPath("/a/f", 11);
         pSource->setAttributeByPath("/a/g", 12);
         pSource->setAttributeByPath("/h", 13);

         pDest->adoptiveMerge(pSource.get());
      }

      issearf(dv_cast<int>(pDest->getAttributeByPath("/a/b/c")) == 9);
      issearf(dv_cast<int>(pDest->getAttributeByPath("/a/b/d")) == 10);
      issearf(dv_cast<int>(pDest->getAttributeByPath("/a/e")) == 6);
      issearf(dv_cast<int>(pDest->getAttributeByPath("/a/f")) == 11);
      issearf(dv_cast<int>(pDest->getAttributeByPath("/a/g")) == 12);
      issearf(dv_cast<int>(pDest->getAttributeByPath("/h")) == 13);

      return success;
   }
};

class DynamicObjectComparisonTestCase : public TestCase
{
public:
   DynamicObjectComparisonTestCase() :
      TestCase("Comparison")
   {}

   bool run()
   {
      bool success = true;
      string testDataPath = TestUtilities::getTestDataPath();

      // Create a nested dynamic object with values of various types
      FactoryResource<DynamicObject> pObject;
      issearf(pObject.get() != NULL);

      bool boolValue = true;
      int intValue = 123;
      double doubleValue = 987.6;
      ProcessingLocation enumValue = IN_MEMORY;
      string stringValue = "Test string";
      string filenameValue1 = testDataPath + "FileOne.ext";
      string filenameValue2 = testDataPath + "FileTwo.ext";
      string filenameValue3 = testDataPath + "FileThree.ext";
      string filenameValue4 = testDataPath + "FileFour.ext";

      FactoryResource<Filename> pFilenameValue1;
      pFilenameValue1->setFullPathAndName(filenameValue1);

      FactoryResource<Filename> pFilenameValue2;
      pFilenameValue2->setFullPathAndName(filenameValue2);

      FactoryResource<Filename> pFilenameValue3;
      pFilenameValue3->setFullPathAndName(filenameValue3);

      FactoryResource<Filename> pFilenameValue4;
      pFilenameValue4->setFullPathAndName(filenameValue4);

      FactoryResource<DateTime> pDateTimeValue;
      pDateTimeValue->setToCurrentTime();

      vector<bool> boolVector;
      boolVector.push_back(true);
      boolVector.push_back(false);
      boolVector.push_back(false);
      boolVector.push_back(true);

      vector<int> intVector;
      intVector.push_back(234);
      intVector.push_back(345);
      intVector.push_back(456);
      intVector.push_back(567);

      vector<double> doubleVector;
      doubleVector.push_back(876.5);
      doubleVector.push_back(765.4);
      doubleVector.push_back(654.3);
      doubleVector.push_back(543.2);

      // Enum vectors are not currently supported in DataVariant

      vector<string> stringVector;
      stringVector.push_back("four");
      stringVector.push_back("three");
      stringVector.push_back("two");
      stringVector.push_back("one");

      vector<Filename*> filenameVector;
      filenameVector.push_back(pFilenameValue1.get());
      filenameVector.push_back(pFilenameValue2.get());
      filenameVector.push_back(pFilenameValue3.get());
      filenameVector.push_back(pFilenameValue4.get());

      // DateTime vectors are not currently supported in DataVariant

      FactoryResource<DynamicObject> pChildObject;
      issearf(pChildObject.get() != NULL);
      issea(pChildObject->setAttribute("boolValue", boolValue));
      issea(pChildObject->setAttribute("intValue", intValue));
      issea(pChildObject->setAttribute("doubleValue", doubleValue));
      issea(pChildObject->setAttribute("enumValue", enumValue));
      issea(pChildObject->setAttribute("stringValue", stringValue));
      issea(pChildObject->setAttribute("filenameValue", *pFilenameValue1.get()));
      issea(pChildObject->setAttribute("dateTimeValue", *pDateTimeValue.get()));
      issea(pChildObject->setAttribute("boolVector", boolVector));
      issea(pChildObject->setAttribute("intVector", intVector));
      issea(pChildObject->setAttribute("doubleVector", doubleVector));
      issea(pChildObject->setAttribute("stringVector", stringVector));
      issea(pChildObject->setAttribute("filenameVector", filenameVector));

      issea(pObject->setAttribute("boolValue", boolValue));
      issea(pObject->setAttribute("intValue", intValue));
      issea(pObject->setAttribute("doubleValue", doubleValue));
      issea(pObject->setAttribute("enumValue", enumValue));
      issea(pObject->setAttribute("stringValue", stringValue));
      issea(pObject->setAttribute("filenameValue", *pFilenameValue1.get()));
      issea(pObject->setAttribute("dateTimeValue", *pDateTimeValue.get()));
      issea(pObject->setAttribute("boolVector", boolVector));
      issea(pObject->setAttribute("intVector", intVector));
      issea(pObject->setAttribute("doubleVector", doubleVector));
      issea(pObject->setAttribute("stringVector", stringVector));
      issea(pObject->setAttribute("filenameVector", filenameVector));
      issea(pObject->setAttribute("childObject", *(pChildObject.get())));

      // Create a new dynamic object as a copy of the original object
      FactoryResource<DynamicObject> pCompareObject;
      issearf(pCompareObject.get() != NULL);

      pCompareObject->merge(pObject.get());

      // Compare the copied object to the original object
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->compare(pObject.get()) == true);

      // Create separate instances of each of the attributes in the dynamic object
      bool boolValue2 = false;
      int intValue2 = 321;
      double doubleValue2 = 6.789;
      ProcessingLocation enumValue2 = ON_DISK_READ_ONLY;
      string stringValue2 = "String test";

      FactoryResource<DateTime> pDateTimeValue2;
      pDateTimeValue2->set(2002, 2, 2);

      vector<bool> boolVector2;
      boolVector2.push_back(false);
      boolVector2.push_back(true);
      boolVector2.push_back(true);
      boolVector2.push_back(false);

      vector<int> intVector2;
      intVector2.push_back(432);
      intVector2.push_back(543);
      intVector2.push_back(654);
      intVector2.push_back(765);

      vector<double> doubleVector2;
      doubleVector2.push_back(5.678);
      doubleVector2.push_back(4.567);
      doubleVector2.push_back(3.456);
      doubleVector2.push_back(2.345);

      vector<string> stringVector2;
      stringVector2.push_back("five");
      stringVector2.push_back("six");
      stringVector2.push_back("seven");
      stringVector2.push_back("eight");

      vector<Filename*> filenameVector2;
      filenameVector2.push_back(pFilenameValue4.get());
      filenameVector2.push_back(pFilenameValue3.get());
      filenameVector2.push_back(pFilenameValue2.get());
      filenameVector2.push_back(pFilenameValue1.get());

      // Swap out each of the values individually in the copied parent object
      // and compare the copied parent object against the original parent object
      issea(pCompareObject->setAttribute("boolValue", boolValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("boolValue", boolValue));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->setAttribute("intValue", intValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("intValue", intValue));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->setAttribute("doubleValue", doubleValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("doubleValue", doubleValue));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->setAttribute("enumValue", enumValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("enumValue", enumValue));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->setAttribute("stringValue", stringValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("stringValue", stringValue));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->setAttribute("filenameValue", *pFilenameValue2.get()));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("filenameValue", *pFilenameValue1.get()));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->setAttribute("dateTimeValue", *pDateTimeValue2.get()));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("dateTimeValue", *pDateTimeValue.get()));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->setAttribute("boolVector", boolVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("boolVector", boolVector));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->setAttribute("intVector", intVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("intVector", intVector));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->setAttribute("doubleVector", doubleVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("doubleVector", doubleVector));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->setAttribute("stringVector", stringVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("stringVector", stringVector));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->setAttribute("filenameVector", filenameVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("filenameVector", filenameVector));
      issea(pObject->compare(pCompareObject.get()) == true);

      // Swap out each of the values individually in the copied child object
      // and compare the copied parent object against the original parent object
      DynamicObject* pNewChildObject = &dv_cast<DynamicObject>(pObject->getAttribute("childObject"));

      issea(pNewChildObject->setAttribute("boolValue", boolValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("boolValue", boolValue));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->setAttribute("intValue", intValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("intValue", intValue));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->setAttribute("doubleValue", doubleValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("doubleValue", doubleValue));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->setAttribute("enumValue", enumValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("enumValue", enumValue));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->setAttribute("stringValue", stringValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("stringValue", stringValue));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->setAttribute("filenameValue", *pFilenameValue2.get()));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("filenameValue", *pFilenameValue1.get()));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->setAttribute("dateTimeValue", *pDateTimeValue2.get()));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("dateTimeValue", *pDateTimeValue.get()));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->setAttribute("boolVector", boolVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("boolVector", boolVector));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->setAttribute("intVector", intVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("intVector", intVector));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->setAttribute("doubleVector", doubleVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("doubleVector", doubleVector));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->setAttribute("stringVector", stringVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("stringVector", stringVector));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->setAttribute("filenameVector", filenameVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("filenameVector", filenameVector));
      issea(pObject->compare(pCompareObject.get()) == true);

      // Add and remove attributes from the copied parent object and compare
      // the copied parent object against the original parent object
      issea(pCompareObject->removeAttribute("boolValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("boolValue", boolValue));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("boolValue2", boolValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("boolValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->removeAttribute("intValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("intValue", intValue));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("intValue2", intValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("intValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->removeAttribute("doubleValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("doubleValue", doubleValue));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("doubleValue2", doubleValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("doubleValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->removeAttribute("enumValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("enumValue", enumValue));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("enumValue2", enumValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("enumValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->removeAttribute("stringValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("stringValue", stringValue));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("stringValue2", stringValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("stringValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->removeAttribute("filenameValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("filenameValue", *pFilenameValue1.get()));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("filenameValue2", *pFilenameValue2.get()));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("filenameValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->removeAttribute("dateTimeValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("dateTimeValue", *pDateTimeValue.get()));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("dateTimeValue2", *pDateTimeValue2.get()));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("dateTimeValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->removeAttribute("boolVector"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("boolVector", boolVector));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("boolVector2", boolVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("boolVector2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->removeAttribute("intVector"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("intVector", intVector));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("intVector2", intVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("intVector2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->removeAttribute("doubleVector"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("doubleVector", doubleVector));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("doubleVector2", doubleVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("doubleVector2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->removeAttribute("stringVector"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("stringVector", stringVector));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("stringVector2", stringVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("stringVector2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pCompareObject->removeAttribute("filenameVector"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->setAttribute("filenameVector", filenameVector));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pCompareObject->setAttribute("filenameVector2", filenameVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pCompareObject->removeAttribute("filenameVector2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      // Add and remove attributes from the copied child object and compare
      // the copied parent object against the original parent object
      issea(pNewChildObject->removeAttribute("boolValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("boolValue", boolValue));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("boolValue2", boolValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("boolValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->removeAttribute("intValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("intValue", intValue));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("intValue2", intValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("intValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->removeAttribute("doubleValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("doubleValue", doubleValue));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("doubleValue2", doubleValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("doubleValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->removeAttribute("enumValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("enumValue", enumValue));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("enumValue2", enumValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("enumValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->removeAttribute("stringValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("stringValue", stringValue));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("stringValue2", stringValue2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("stringValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->removeAttribute("filenameValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("filenameValue", *pFilenameValue1.get()));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("filenameValue2", *pFilenameValue2.get()));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("filenameValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->removeAttribute("dateTimeValue"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("dateTimeValue", *pDateTimeValue.get()));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("dateTimeValue2", *pDateTimeValue2.get()));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("dateTimeValue2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->removeAttribute("boolVector"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("boolVector", boolVector));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("boolVector2", boolVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("boolVector2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->removeAttribute("intVector"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("intVector", intVector));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("intVector2", intVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("intVector2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->removeAttribute("doubleVector"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("doubleVector", doubleVector));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("doubleVector2", doubleVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("doubleVector2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->removeAttribute("stringVector"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("stringVector", stringVector));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("stringVector2", stringVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("stringVector2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      issea(pNewChildObject->removeAttribute("filenameVector"));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->setAttribute("filenameVector", filenameVector));
      issea(pObject->compare(pCompareObject.get()) == true);
      issea(pNewChildObject->setAttribute("filenameVector2", filenameVector2));
      issea(pObject->compare(pCompareObject.get()) == false);
      issea(pNewChildObject->removeAttribute("filenameVector2"));
      issea(pObject->compare(pCompareObject.get()) == true);

      return success;
   }
};

class DynamicObjectTestSuite : public TestSuiteNewSession
{
public:
   DynamicObjectTestSuite() : TestSuiteNewSession( "DynamicObject" )
   {
      addTestCase( new DynamicObjectCreationTest );
      addTestCase( new DynamicObjectNestingTest );
      addTestCase( new DynamicObjectSerializeTestCase );
      addTestCase( new DynamicObjectNestedSerializeTestCase );
      addTestCase( new DynamicObjectAdoptiveMergeTestCase );
      addTestCase( new DynamicObjectComparisonTestCase );
   }
};

REGISTER_SUITE( DynamicObjectTestSuite )
