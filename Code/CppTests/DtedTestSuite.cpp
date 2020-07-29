/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "TestCase.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"

#include <vector>
using namespace std;

class DtedImportTestCase : public TestCase
{
public:
   DtedImportTestCase() : TestCase("DtedImport") {}

   bool run()
   {
      bool success = true;

      ImporterResource importer("DTED Importer", TestUtilities::getTestDataPath() + "Dted/W109/N30.dt1");
      issea(importer->execute());
      vector<DataElement*> elements = importer->getImportedElements();

      issea(elements.size() == 1);
      RasterElement *pElement = dynamic_cast<RasterElement*>(elements[0]);
      issea(pElement != NULL);
      const RasterDataDescriptor *pDataDescriptor =
         dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      issea(pDataDescriptor != NULL);
      issea(pDataDescriptor->getRowCount() == 1201);
      issea(pDataDescriptor->getColumnCount() == 1201);
      issea(pDataDescriptor->getBandCount() == 1);

      return success;
   }
};

class DtedTestSuite : public TestSuiteNewSession
{
public:
   DtedTestSuite() : TestSuiteNewSession( "Dted" )
   {
      addTestCase(new DtedImportTestCase);
   }
};

REGISTER_SUITE( DtedTestSuite )
