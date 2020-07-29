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
#include "ConnectionManager.h"
#include "DataVariant.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "ImportDescriptor.h"
#include "ModelServicesImp.h"
#include "ObjectResource.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "SignatureSet.h"
#include "Subject.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "Units.h"
#include <vector>
using namespace std;

string pFile = "EnviTest.hdr";

int badBandsList[] = {
   0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0
};

class EnviFieldParsingTestCase : public TestCase
{
public:
   EnviFieldParsingTestCase() : TestCase("FieldParsing") {}
   bool run()
   {
      bool success = true;

      ImporterResource pPlugIn2("ENVI Importer", TestUtilities::getTestDataPath() + pFile);

      vector<ImportDescriptor*> importDescriptors = pPlugIn2->getImportDescriptors();
      issea(importDescriptors.empty() == false);

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issea(pImportDescriptor != NULL);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issea( pDescriptor != NULL );

      RasterFileDescriptor *pFileDescriptor = NULL;
      pFileDescriptor = dynamic_cast<RasterFileDescriptor*>( pDescriptor->getFileDescriptor() );
      issea( pFileDescriptor != NULL );

      if( success )
      {
         success = success && tst_assert( pFileDescriptor->getRowCount() == 181 );
         success = success && tst_assert( pFileDescriptor->getColumnCount() == 97 );
         success = success && tst_assert( pFileDescriptor->getBandCount() == 210 );
         success = success && tst_assert( pDescriptor->getDataType() == INT2SBYTES );
         success = success && tst_assert( pFileDescriptor->getInterleaveFormat() == BIP );
         success = success && tst_assert( pFileDescriptor->getHeaderBytes() == 0 );
         success = success && tst_assert( pFileDescriptor->getTrailerBytes() == 0 );
         success = success && tst_assert( pFileDescriptor->getPrelineBytes() == 0 );
         success = success && tst_assert( pFileDescriptor->getPostlineBytes() == 0 );
         success = success && tst_assert( pFileDescriptor->getPrebandBytes() == 0 );
         success = success && tst_assert( pFileDescriptor->getPostbandBytes() == 0 );
         success = success && tst_assert( pFileDescriptor->getBandFiles().size() == 0 );
         success = success && tst_assert( pFileDescriptor->getEndian() == LITTLE_ENDIAN_ORDER );

         issea( pPlugIn2->execute() == true );
         vector<DataElement*> elements = pPlugIn2->getImportedElements();
         issearf(elements.empty() == false);
         RasterElement *pRaster = dynamic_cast<RasterElement*>(elements.front());
         pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
         issearf(pDescriptor);
         pFileDescriptor = dynamic_cast<RasterFileDescriptor*>( pDescriptor->getFileDescriptor() );
         issearf(pFileDescriptor);

         if( success )
         {
            success = success && tst_assert( pDescriptor->getBandCount() == 168 );
            if( success )
            {
               int j = 0;
               for( int i = 0; success && i < 210; ++i )
               {
                  if( badBandsList[i] != 0 )
                  {
                     success = success && tst_assert( j < 168 );
                     int activeNumber = pFileDescriptor->getOnDiskBand( i ).getActiveNumber();
                     success = success && tst_assert( activeNumber == j );
                     ++j;
                  }
               }
            }
            success = success && tst_assert( pFileDescriptor->getBandCount() != pDescriptor->getBandCount() );
            success = success && tst_assert( pFileDescriptor->getBandCount() == 210 );
            success = success && tst_assert( pDescriptor->getBandCount() == 168 );
            success = success && tst_assert( pDescriptor->getDisplayMode() == RGB_MODE );
            success = success && tst_assert( pDescriptor->getOriginalBand(
               pDescriptor->getDisplayBand( GRAY ).getOriginalNumber()).getActiveNumber() == 0 );
            success = success && tst_assert( pDescriptor->getOriginalBand(
               pDescriptor->getDisplayBand( RED ).getOriginalNumber()).getActiveNumber() == 45 );
            success = success && tst_assert( pDescriptor->getOriginalBand(
               pDescriptor->getDisplayBand( GREEN ).getOriginalNumber()).getActiveNumber() == 30 );
            success = success && tst_assert( pDescriptor->getOriginalBand(
               pDescriptor->getDisplayBand( BLUE ).getOriginalNumber()).getActiveNumber() == 10 );
            success = success && tst_assert( ( pFileDescriptor->getRowCount() == pDescriptor->getRowCount() ) &&
               ( pFileDescriptor->getColumnCount() == pDescriptor->getColumnCount() ) );
         }
      }

      return success;
   }
};

class EnviSignatureLibraryImportTestCase : public TestCase
{
public:
   EnviSignatureLibraryImportTestCase() : TestCase("SignatureLibraryImport"), mbDeleteDetected(false) {}
   void update(Subject &subject, const string &signal, const boost::any& v)
   {
      if (signal == SIGNAL_NAME(Subject, Deleted))
      {
         mbDeleteDetected = true;
      }
   }

   bool run()
   {
      bool success = true;

      ImporterResource pPlugIn2("Auto Importer", TestUtilities::getTestDataPath() + "Signatures/manmade1.sli");
      issea( pPlugIn2->execute() == true );
      vector<DataElement*> elements = pPlugIn2->getImportedElements();

      issea(elements.size() == 1);
      SignatureSet *pSet = dynamic_cast<SignatureSet*>(elements[0]);
      issea(pSet != NULL);
      vector<Signature*> sigs = pSet->getSignatures();
      issea(sigs.size() == 14);
      string sig6Name = sigs[6]->getName();
      issea(sig6Name == "Black gloss paint (Paints 0402UUUPNT)");
      DataVariant dvRef = sigs[6]->getData("Reflectance");
      DataVariant dvWav = sigs[6]->getData("Wavelength");
      vector<double> reflectances;
      issea(dvRef.getValue<vector<double> >(reflectances));
      issea(fabs(reflectances[0]-0.0281670) < 0.0000001);
      vector<double> wavelengths;
      issea(dvWav.getValue<vector<double> >(wavelengths));
      issea(fabs(wavelengths[1]-0.42200) < 0.0000001);
      dvWav = sigs[5]->getData("Wavelength");
      issea(dvWav.getValue<vector<double> >(wavelengths));
      issea(fabs(wavelengths[1]-0.42200) < 0.0000001);
      const Units *pUnits = NULL;
      issea((pUnits = pSet->getUnits("Reflectance")) != NULL);
      issea(pUnits->getScaleFromStandard() == 1.0);
      issea(pUnits->getUnitType() == CUSTOM_UNIT);

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : We need to write a DataElementGroup test suite (tjohnson)")

      return success;
   }

private:
   bool mbDeleteDetected;
};

class EnviSignatureLibraryMetadataTestCase : public TestCase
{
public:
   EnviSignatureLibraryMetadataTestCase() : TestCase("SignatureLibraryMetadata") {}

   bool run()
   {
      bool success = true;

      ImporterResource pImporter("Auto Importer", TestUtilities::getTestDataPath() +
         "Signatures/SignatureWithMetadata.hdr");
      issearf(pImporter->execute());
      vector<DataElement*> elements = pImporter->getImportedElements();

      issearf(elements.size() == 1);
      ModelResource<SignatureSet> pSet(dynamic_cast<SignatureSet*>(elements[0]));
      issearf(pSet.get() != NULL);

      // make sure this is the correct signature library
      vector<Signature*> sigs = pSet->getSignatures();
      issearf(sigs.size() == 1);
      string sigName = sigs[0]->getName();
      issearf(sigName == "Bush Muhly Grass");

      // now check the metadata
      const DynamicObject *pMetadata = pSet->getMetadata();
      issearf(pMetadata != NULL);
      issearf(pMetadata->getNumAttributes() == 48);
      // check a few arbitrary metadata entries
      issea(pMetadata->getAttribute("measuring organization").toXmlString() == "USAETL");
      issea(pMetadata->getAttribute("data quality").toXmlString() == "Low");
      issea(pMetadata->getAttribute("creation process").toXmlString() == "Ingest holdings");
      issea(pMetadata->getAttribute("metadata chapter version").toXmlString() == "1.6");

      return success;
   }
};

class EnviTestSuite : public TestSuiteNewSession
{
public:
   EnviTestSuite() : TestSuiteNewSession( "Envi" )
   {
      addTestCase( new EnviFieldParsingTestCase );
      addTestCase( new EnviSignatureLibraryImportTestCase );
      addTestCase( new EnviSignatureLibraryMetadataTestCase );
   }
};

REGISTER_SUITE( EnviTestSuite )
