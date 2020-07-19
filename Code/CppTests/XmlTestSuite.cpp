/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "assert.h"
#include "FileResource.h"
#include "ConfigurationSettings.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "XercesIncludes.h"
#include "xmlbase.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <limits>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

class XmlPathTest : public TestCase
{
public:
   XmlPathTest() : TestCase("XmlPathTest") {}
   bool run()
   {
      // Create several test cases where pathTests[i].first has a path and pathTests[i].second has a URL.
      bool success = true;
      std::vector<std::pair<std::string, std::string> > pathTests;
      pathTests.push_back(std::make_pair<std::string, std::string>("./rel/file.xml", "file:///././rel/file.xml"));
      pathTests.push_back(std::make_pair<std::string, std::string>("../parent/dir", "file:///./../parent/dir"));
      pathTests.push_back(std::make_pair<std::string, std::string>("space/fi le.xml", "file:///./space/fi%20le.xml"));
      pathTests.push_back(std::make_pair<std::string, std::string>("C:/dir/f ile.txt", "file:///C:/dir/f%20ile.txt"));
      pathTests.push_back(std::make_pair<std::string, std::string>("G:/dir", "file:///G:/dir"));
      pathTests.push_back(std::make_pair<std::string, std::string>("/usr/local/bin/ls", "file:///usr/local/bin/ls"));
      pathTests.push_back(std::make_pair<std::string, std::string>("/usr/local", "file:///usr/local"));
      pathTests.push_back(std::make_pair<std::string, std::string>("110%", "file:///./110%25"));
      pathTests.push_back(std::make_pair<std::string, std::string>("%", "file:///./%25"));
      pathTests.push_back(std::make_pair<std::string, std::string>(" ", "file:///./%20"));
      pathTests.push_back(std::make_pair<std::string, std::string>("\"", "file:///./%22"));
      pathTests.push_back(std::make_pair<std::string, std::string>("<", "file:///./%3c"));
      pathTests.push_back(std::make_pair<std::string, std::string>(">", "file:///./%3e"));
      pathTests.push_back(std::make_pair<std::string, std::string>("#", "file:///./%23"));
      pathTests.push_back(std::make_pair<std::string, std::string>("{", "file:///./%7b"));
      pathTests.push_back(std::make_pair<std::string, std::string>("}", "file:///./%7d"));
      pathTests.push_back(std::make_pair<std::string, std::string>("|", "file:///./%7c"));
      pathTests.push_back(std::make_pair<std::string, std::string>("\\", "file:///./%5c"));
      pathTests.push_back(std::make_pair<std::string, std::string>("^", "file:///./%5e"));
      pathTests.push_back(std::make_pair<std::string, std::string>("~", "file:///./%7e"));
      pathTests.push_back(std::make_pair<std::string, std::string>("[", "file:///./%5b"));
      pathTests.push_back(std::make_pair<std::string, std::string>("]", "file:///./%5d"));
      pathTests.push_back(std::make_pair<std::string, std::string>("`", "file:///./%60"));
      pathTests.push_back(std::make_pair<std::string, std::string>("(", "file:///./%28"));
      pathTests.push_back(std::make_pair<std::string, std::string>(")", "file:///./%29"));

      // Test each case in each direction (URL -> Path and Path -> URL).
      for(std::vector<std::pair<std::string, std::string> >::const_iterator iter = pathTests.begin();
         iter != pathTests.end();
         ++iter)
      {
         issearf(XmlBase::PathToURL(iter->first) == iter->second);
         issearf(XmlBase::URLtoPath(X(iter->second.c_str())) == iter->first);
      }

      return success;
   }
};

class XmlBase64Test : public TestCase
{
public:
   XmlBase64Test() : TestCase("XmlBase64Test") {}
   bool run()
   {
      // Create several test cases with known binary data.
      bool success = true;
      std::vector<std::vector<unsigned int> > base64Tests(4);
      getWalkingOnes(base64Tests[0]);
      getComplement(base64Tests[0], base64Tests[1]);
      getRandom(base64Tests[2]);
      getComplement(base64Tests[2], base64Tests[3]);

      // Test each case in each direction (Encoding and Decoding).
      for(std::vector<std::vector<unsigned int> >::iterator iter = base64Tests.begin();
         success && iter != base64Tests.end();
         ++iter)
      {
         std::string checksum;
         XMLByte* pDataXml = XmlBase::encodeBase64(&iter->at(0), iter->size(), NULL, &checksum);
         issearf(pDataXml != NULL);

         // Use standard issea so that memory is freed properly.
         unsigned int* pData = XmlBase::decodeBase64(pDataXml, iter->size(), checksum);
         issea(pData != NULL);
         issea(memcmp(pData, &iter->at(0), iter->size() * sizeof(unsigned int)) == 0);

         ::operator delete(pDataXml);
         delete [] pData;
      }

      return success;
   }

private:
   void getWalkingOnes(std::vector<unsigned int>& data)const
   {
      // Create a walking ones pattern from the LSB starting at 0, i.e.:
      // For a 4-byte unsigned integer, this yields:
      //    data[0]  = 0x00000000
      //    data[1]  = 0x00000001
      //    data[2]  = 0x00000002
      //    data[3]  = 0x00000004
      //    data[4]  = 0x00000008
      //    data[5]  = 0x00000010
      //    . . .
      //    data[32] = 0x80000000
      // To do this, allocate one entry per bit (8 * sizeof(unsigned int) and an additional entry for index 0.
      data.resize(8 * sizeof(unsigned int) + 1, 0);

      // Increment to set the LSB.
      ++data[1];

      // Index 0 and index 1 have been populated. Fill in the rest of the entries.
      for (std::vector<unsigned int>::iterator iter = data.begin() + 2; iter != data.end(); ++iter)
      {
         // Shift the 1 to the left by one position.
         *iter = *(iter - 1) << 1;
      }
   }

   void getRandom(std::vector<unsigned int>& data)const
   {
      // Create several pseudorandom values with an arbitrary seed.
      srand(1337);
      data.resize(1000);
      for (std::vector<unsigned int>::iterator iter = data.begin(); iter != data.end(); ++iter)
      {
         // The call to rand() does not always populate all bits (e.g.: on Windows, RAND_MAX is 32767).
         // Generate some random numbers, multiply them, and cast to an unsigned int so that all bits can be populated.
         // Use bitwise and during the downcast instead of static_cast to avoid a run-time type failure on Windows.
         uint64_t value1 = static_cast<uint64_t>(rand());
         uint64_t value2 = static_cast<uint64_t>(rand());
         uint64_t value3 = static_cast<uint64_t>(rand());
         uint64_t value4 = static_cast<uint64_t>(rand());
         *iter = (value1 * value2 * value3 * value4) & std::numeric_limits<unsigned int>::max();
      }
   }

   void getComplement(const std::vector<unsigned int>& src, std::vector<unsigned int>& dst)
   {
      dst.resize(src.size());
      for (std::vector<unsigned int>::size_type i = 0; i < src.size(); ++i)
      {
         dst[i] = ~src[i];
      }
   }
};

class XmlStringTest : public TestCase
{
public:
   XmlStringTest() : TestCase("XmlStringTest") {}
   bool run()
   {
      bool success = true;

      // Valid UTF-8 sequences are available at http://en.wikipedia.org/wiki/UTF-8.
      // Create some simple strings for testing.
      std::vector<std::string> stringTests;
      stringTests.push_back("simple");
      stringTests.push_back("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
      stringTests.push_back(MICRON);

      // Test all valid one-byte encodings except for the string containing
      // only 0x00 because it messes up some of the comparisons later.
      for (unsigned int i = 1; i < 0x0000007f; ++i)
      {
         stringTests.push_back(std::string(reinterpret_cast<char*>(&i), 1));
      }

      // Test all valid two-byte encodings.
      for (unsigned int i = 0x0000C200; i < 0x0000DF00; i += 0x00000100)
      {
         for (unsigned int j = 0x00000080; j < 0x000000BF; ++j)
         {
            unsigned int value = i + j;
            stringTests.push_back(std::string(reinterpret_cast<char*>(&value), 2));
         }
      }

      // Test all valid three-byte encodings.
      for (unsigned int i = 0x00E00000; i < 0x00EF0000; i += 0x00010000)
      {
         for (unsigned int j = 0x00008000; j < 0x0000BF00; j += 0x00000100)
         {
            for (unsigned int k = 0x00000080; k < 0x000000BF; ++k)
            {
               unsigned int value = i + j + k;
               stringTests.push_back(std::string(reinterpret_cast<char*>(&value), 3));
            }
         }
      }

      // Test all valid four-byte encodings.
      for (unsigned int i = 0xF0000000; i < 0xF4000000; i += 0x01000000)
      {
         for (unsigned int j = 0x00800000; j < 0x00BF0000; j += 0x00010000)
         {
            for (unsigned int k = 0x00008000; k < 0x0000BF00; k += 0x00000100)
            {
               for (unsigned int m = 0x00000080; m < 0x000000BF; ++m)
               {
                  unsigned int value = i + j + k + m;
                  stringTests.push_back(std::string(reinterpret_cast<char*>(&value), 4));
               }
            }
         }
      }

      // Test several pseudorandom combinations.
      srand(1337);
      const unsigned int numAdditions = static_cast<unsigned int>(rand()) % 1000;
      for (unsigned int i = 0; i < numAdditions; ++i)
      {
         std::string newValue;

         // Combine this many elements.
         const unsigned int numCombinations = (static_cast<unsigned int>(rand()) % 100) + 1;
         for (unsigned int j = 0; j < numCombinations; ++j)
         {
            // The call to rand() does not always populate all bits (e.g.: on Windows, RAND_MAX is 32767).
            // Generate some random numbers, multiply them, cast to a size_type and take mod number of elements.
            // Use bitwise and during the downcast instead of static_cast to avoid a run-time type failure on Windows.
            uint64_t value1 = static_cast<uint64_t>(rand());
            uint64_t value2 = static_cast<uint64_t>(rand());
            uint64_t value3 = static_cast<uint64_t>(rand());
            uint64_t value4 = static_cast<uint64_t>(rand());
            std::vector<std::string>::size_type index = ((value1 * value2 * value3 * value4) &
               std::numeric_limits<std::vector<std::string>::size_type>::max()) % stringTests.size();
            newValue += stringTests[index];
         }

         stringTests.push_back(newValue);
      }

      for (std::vector<std::string>::const_iterator iter = stringTests.begin(); iter != stringTests.end(); ++iter)
      {
         XERCES_CPP_NAMESPACE_USE
         OpticksXStr fromAscii(iter->c_str());                                   // Construct OpticksXStr on the stack.
         isseab(fromAscii.asciiForm() == iter->c_str());                         // Pointer compared to input argument.
         isseab(fromAscii.asciiForm() == A(iter->c_str()));                      // Pointer compared to A.
         isseab(std::string(fromAscii.asciiForm(), iter->length()) == *iter);    // std::string::operator ==.
         isseab(XMLString::equals(fromAscii.unicodeForm(), X(iter->c_str())));   // Unicode compared to X.

         OpticksXStr fromUnicode(fromAscii.unicodeForm());                       // Construct OpticksXStr on the stack.
         isseab(fromUnicode.unicodeForm() == fromAscii.unicodeForm());           // Pointer compared to input argument.
         isseab(XMLString::equals(fromUnicode.unicodeForm(), X(iter->c_str()))); // Pointer compared to X.

         // Comparisons to check that the OpticksXStr instances match.
         isseab(XMLString::equals(fromAscii.unicodeForm(), fromUnicode.unicodeForm()));
      }

      return success;
   }
};

class XmlWriterTest : public TestCase
{
public:
   XmlWriterTest() : TestCase("XmlWriterTest") {}
   bool run()
   {
      bool success = true;
      std::string filename = ConfigurationSettings::getSettingTempPath()->getFullPathAndName() + "/xmltest.xml";

      // Write a file to the temp directory, and read it later with XmlReaderTest.
      // The following example code generates this DOM tree.
      // n.b.: Changing this tree affects the tests in XmlReaderTest.
      //       Do not change this structure without also updating XmlReaderTest.
      //<topLevel>
      //  <child a="42" b="This is an attribute">
      //    <grandchild>
      //       Here is some text.
      //    </grandchild>
      //  </child>
      //  <anotherchild val="1 2 3 4"/>
      //</topLevel>
      XMLWriter xml("topLevel", NULL, false);

      // This code is in a loop to test multiple writes with the same class instance.
      // Multiple writes are interesting to test because there have historically
      // been issues (memory leaks, crashes, etc) using this style of programming.
      for (int i = 0; i < 5; ++i)
      {
         xml.pushAddPoint(xml.addElement("child"));
         xml.addAttr("a", 42);
         xml.addAttr("b", "This is an attribute");
         xml.pushAddPoint(xml.addElement("grandchild"));
         xml.addText("Here is some text.");
         xml.popAddPoint();
         xml.popAddPoint();
         std::vector<int> intVect;
         intVect.push_back(1);
         intVect.push_back(2);
         intVect.push_back(3);
         intVect.push_back(4);
         xml.pushAddPoint(xml.addElement("anotherchild"));
         xml.addAttr("val", intVect);

         // Overwrite the same file with the same data multiple times.
         FileResource pFile(filename.c_str(), "wt");
         issearf(pFile.get());
         xml.writeToFile(pFile.get());
      }

      return success;
   }
};

class XmlReaderTest : public TestCase
{
public:
   XmlReaderTest() : TestCase("XmlReaderTest") {}
   bool run()
   {
      XERCES_CPP_NAMESPACE_USE
      bool success = true;
      std::string filename = ConfigurationSettings::getSettingTempPath()->getFullPathAndName() + "/xmltest.xml";

      // Read the file written to the temp directory by XmlWriterTest.
      // Test the equivalent text using XmlReader::parseString.
      {
         XmlReader reader(NULL, false);
         const std::string xmlString =
            "<topLevel>"
              "<child a=\"42\" b=\"This is an attribute\">"
                "<grandchild>"
                   "Here is some text."
                "</grandchild>"
              "</child>"
              "<anotherchild val=\"1 2 3 4 \"/>"
            "</topLevel>";

         // This code is in a loop to test multiple parses with the same class instance.
         // Multiple parses are interesting to test because there have historically
         // been issues (memory leaks, crashes, etc) using this style of programming.
         for (unsigned int i = 0; i < 5; ++i)
         {
            DOMDocument* pDocument = i % 2 ? reader.parse(filename) : reader.parseString(xmlString);
            issearf(pDocument != NULL);

            DOMElement* pRootElement = pDocument->getDocumentElement();
            issearf(pRootElement != NULL);
            issearf(XMLString::equals(pRootElement->getNodeName(), X("topLevel")));

            for (DOMNode* pChild = pRootElement->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
            {
               if (XMLString::equals(pChild->getNodeName(), X("child")))
               {
                  DOMElement* pChildElement = static_cast<DOMElement*>(pChild);
                  issearf(pChild != NULL);
                  issearf(pChild->getParentNode() == pRootElement);
                  issearf(A(pChildElement->getAttribute(X("a"))) == std::string("42"));
                  issearf(A(pChildElement->getAttribute(X("b"))) == std::string("This is an attribute"));

                  for (DOMNode* pGrandchild = pChild->getFirstChild();
                     pGrandchild != NULL;
                     pGrandchild = pGrandchild->getNextSibling())
                  {
                     if (XMLString::equals(pGrandchild->getNodeName(), X("grandchild")))
                     {
                        issearf(pGrandchild != NULL);
                        issearf(pGrandchild->getParentNode() == pChild);
                        issearf(A(pGrandchild->getTextContent()) == std::string("Here is some text."));
                     }
                  }
               }
               else if (XMLString::equals(pChild->getNodeName(), X("anotherchild")))
               {
                  DOMElement* pChildElement = static_cast<DOMElement*>(pChild);
                  issearf(pChild != NULL);
                  issearf(pChild->getParentNode() == pRootElement);
                  issearf(A(pChildElement->getAttribute(X("val"))) == std::string("1 2 3 4 "));
               }
            }

            // At this point, the entire document has been checked.
            // Now test some XPath 2.0 queries.
            DOMXPathResult* pResult = reader.query("/topLevel", DOMXPathResult::FIRST_RESULT_TYPE);
            issearf(pResult != NULL);
            DOMNode* pNode = pResult->getNodeValue();
            issearf(pNode != NULL);
            issearf(A(pNode->getNodeName()) == std::string("topLevel"));

            pResult = reader.query("//child", DOMXPathResult::FIRST_RESULT_TYPE);
            issearf(pResult != NULL);
            pNode = pResult->getNodeValue();
            issearf(pNode != NULL);
            issearf(A(pNode->getNodeName()) == std::string("child"));

            pResult = reader.query("//grandchild/text()", DOMXPathResult::FIRST_RESULT_TYPE);
            issearf(pResult != NULL);
            issearf(A(pResult->getStringValue()) == std::string("Here is some text."));
         }
      }

      // Test that the same file fails when validated against the xsd.
      {
         XmlReader reader;
         DOMDocument* pDocument = reader.parse(filename);
         issearf(pDocument != NULL);

         DOMElement* pRootElement = pDocument->getDocumentElement();
         issearf(pRootElement == NULL);
      }

      // Test a syntactically invalid XML file.
      {
         // Overwrite the file with invalid XML.
         {
            // Use r+b even though it is text to read \r\n as two characters on Windows.
            // Read in the existing file in one fell swoop.
            FileResource pFile(filename.c_str(), "r+b");
            issearf(pFile.get() != NULL);
            fseek(pFile.get(), 0, SEEK_END);
            long fileSize = ftell(pFile.get());
            issearf(fileSize > 0);
            rewind(pFile.get());
            std::vector<char> contents(fileSize);
            issearf(fread(&contents[0], 1, fileSize, pFile.get()) == fileSize);
            rewind(pFile.get());

            // Delete some characters from the beginning and the end.
            const unsigned int offset = 4;
            issearf(2 * offset < fileSize);
            issearf(fwrite(&contents[offset], 1, fileSize - 2 * offset, pFile.get()) == fileSize - 2 * offset);
         }

         // Check that the now invalid file fails.
         {
            XmlReader reader(NULL, false);
            DOMDocument* pDocument = reader.parse(filename);
            issearf(pDocument != NULL);

            DOMElement* pRootElement = pDocument->getDocumentElement();
            issearf(pRootElement == NULL);
         }
      }

      // Test a file which is not XML.
      {
         XmlReader reader(NULL, false);
         DOMDocument* pDocument = reader.parse(TestUtilities::getTestDataPath() + "EnviTest.hdr");
         issearf(pDocument != NULL);

         DOMElement* pRootElement = pDocument->getDocumentElement();
         issearf(pRootElement == NULL);
      }

      return success;
   }
};

class XmlReaderStaticTest : public TestCase
{
public:
   XmlReaderStaticTest() : TestCase("XmlReaderStaticTest") {}
   bool run()
   {
      bool success = true;

      // StrToVector and StringStreamAssigner tests.
      std::vector<unsigned char> byteVec;
      std::vector<unsigned char> byteVec2;
      byteVec.push_back('0');
      byteVec.push_back('1');
      std::auto_ptr<std::vector<unsigned char> > pByteVec(reinterpret_cast<std::vector<unsigned char>* >(
         XmlReader::StrToVector<unsigned char, XmlReader::StringStreamAssigner<unsigned char> >(X("0 1"))));
      XmlReader::StrToVector<unsigned char, XmlReader::StringStreamAssigner<unsigned char> >(byteVec2, X("0 1"));
      issearf(pByteVec.get() != NULL && *pByteVec == byteVec && byteVec == byteVec2);

      std::vector<int> intVec;
      std::vector<int> intVec2;
      intVec.push_back(-1);
      intVec.push_back(2);
      intVec.push_back(3);
      intVec.push_back(4);
      std::auto_ptr<std::vector<int> > pIntVec(reinterpret_cast<std::vector<int>* >(
         XmlReader::StrToVector<int, XmlReader::StringStreamAssigner<int> >(X("-1 2 3 4"))));
      XmlReader::StrToVector<int, XmlReader::StringStreamAssigner<int> >(intVec2, X("-1 2 3 4"));
      issearf(pIntVec.get() != NULL && *pIntVec == intVec && intVec == intVec2);

      std::vector<double> doubleVec;
      std::vector<double> doubleVec2;
      doubleVec.push_back(0.0);
      doubleVec.push_back(1e-3);
      doubleVec.push_back(1234.0);
      std::auto_ptr<std::vector<double> > pDoubleVec(reinterpret_cast<std::vector<double>* >(
         XmlReader::StrToVector<double, XmlReader::StringStreamAssigner<double> >(X("0 1e-3  1234.0"))));
      XmlReader::StrToVector<double, XmlReader::StringStreamAssigner<double> >(doubleVec2, X("0 1e-3  1234.0"));
      issearf(pDoubleVec.get() != NULL && *pDoubleVec == doubleVec && doubleVec == doubleVec2);

      // StrToLocation tests.
      LocationType location;
      issearf(XmlReader::StrToLocation(X("123 456"), location) &&
         location.mX == 123.0 && location.mY == 456.0);

      issearf(XmlReader::StrToLocation(X("1e-3 3.14 -42"), location) &&
         location.mX == 0.001 && location.mY == 3.14);

      // StrToQuadCoord tests.
      double a, b, c, d;
      XmlReader::StrToQuadCoord(X("1.2 3.4 5.6 7.8"), a, b, c, d);
      issearf(a == 1.2 && b == 3.4 && c == 5.6 && d == 7.8);

      XmlReader::StrToQuadCoord(X("12.4 44.0 16.85"), a, b, c, d);
      issearf(a == 12.4 && b == 44.0 && c == 16.85 && d == 0.0);

      XmlReader::StrToQuadCoord(X("42.43 15.0"), a, b, c, d);
      issearf(a == 42.43 && b == 15.0 && c == 0.0 && d == 0.0);

      return success;
   }
};

class XmlTestSuite : public TestSuiteNewSession
{
public:
   XmlTestSuite() : TestSuiteNewSession("Xml")
   {
      addTestCase(new XmlPathTest);
      addTestCase(new XmlBase64Test);
      addTestCase(new XmlStringTest);
      addTestCase(new XmlWriterTest);  // XmlWriter should run before XmlReader because
      addTestCase(new XmlReaderTest);  // XmlReader tests a file created by XmlWriter.
      addTestCase(new XmlReaderStaticTest);
   }
};

REGISTER_SUITE(XmlTestSuite)
