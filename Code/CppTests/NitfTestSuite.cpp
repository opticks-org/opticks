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
#include "Blob.h"
#include "ConfigurationSettingsImp.h"
#include "DateTime.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "Executable.h"
#include "FileFinder.h"
#include "FileResource.h"
#include "ImportDescriptor.h"
#include "LayerList.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "ProductView.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "WorkspaceWindow.h"

#include <QtCore/QMap>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QTextStream>

#include <boost/bind.hpp>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

class NitfBadArgsTestCase : public TestCase
{
public:
   NitfBadArgsTestCase() : TestCase("BadArgs") {}
   bool run()
   {
      bool success = true;
      string fileName = TestUtilities::getTestDataPath() + "i_3130b.ntf";

      // Verify that at least one import descriptor exists.
      ImporterResource impResource( "NITF Importer", fileName );
      vector<ImportDescriptor*> importDescriptors = impResource->getImportDescriptors();
      issearf(importDescriptors.empty() == false);

      // Verify that calling execute with invalid parameters results in execute returning false.
      Executable* pExecutable = dynamic_cast<Executable*>(impResource->getPlugIn());
      issearf(pExecutable != NULL);

      issearf(pExecutable->execute(NULL, NULL) == false);
      return success;
   }
};

class NitfTreParsingTestCase : public TestCase
{
public:
   NitfTreParsingTestCase() : TestCase("TreParsing") {}
   bool run()
   {
      bool success = true;

      // Relative path of the file to load
      const string fileName = TestUtilities::getTestDataPath() + "i_3130b.ntf";

      // Number of cubes expected to be present within the file.
      const unsigned int numCubes = 2;

      vector<DataElement*> dataElements = TestUtilities::loadDataSet(fileName, "NITF Importer", numCubes);
      issearf(dataElements.size() == numCubes);

      DataElement* pElement = Service<ModelServices>()->getElement(fileName + "-I1", TypeConverter::toString<RasterElement>(), NULL);
      issearf(pElement != NULL);

      const DynamicObject* const pMetadata = pElement->getMetadata();
      issearf(pMetadata != NULL);

      const string trePath[] = {"NITF", "TRE", "AIMIDB", "0", END_METADATA_NAME};
      const DataVariant& treMetadata = pMetadata->getAttributeByPath(trePath);
      issearf(treMetadata.isValid());

      const DynamicObject* const pTreMetadata = treMetadata.getPointerToValue<DynamicObject>();
      issearf(pTreMetadata != NULL);

      // Set up the name/value combinations to look for within the metadata
      FactoryResource<DateTime> pAcquisition;
      issearf(pAcquisition->set(1998, 2, 12, 8, 39, 12));

      vector<pair<string, string> > expectedTres;
      expectedTres.push_back(pair<string, string>("ACQUISITION_DATE", StringUtilities::toDisplayString(pAcquisition.get())));
      expectedTres.push_back(pair<string, string>("COUNTRY", ""));
      expectedTres.push_back(pair<string, string>("CURRENT_SEGMENT", "AA"));
      expectedTres.push_back(pair<string, string>("END_SEGMENT", "00"));
      expectedTres.push_back(pair<string, string>("END_TILE_COLUMN", "1"));
      expectedTres.push_back(pair<string, string>("END_TILE_ROW", "1"));
      expectedTres.push_back(pair<string, string>("FLIGHT_NO", "00"));
      expectedTres.push_back(pair<string, string>("LOCATION", ""));
      expectedTres.push_back(pair<string, string>("MISSION_IDENTIFICATION", "NOT AVAIL."));
      expectedTres.push_back(pair<string, string>("MISSION_NO", "UNKN"));
      expectedTres.push_back(pair<string, string>("OP_NUM", "0"));
      expectedTres.push_back(pair<string, string>("REPLAY", ""));
      expectedTres.push_back(pair<string, string>("REPRO_NUM", "0"));
      expectedTres.push_back(pair<string, string>("RESERVED1", ""));
      expectedTres.push_back(pair<string, string>("RESERVED2", ""));
      expectedTres.push_back(pair<string, string>("RESERVED3", ""));
      expectedTres.push_back(pair<string, string>("START_TILE_COLUMN", "1"));
      expectedTres.push_back(pair<string, string>("START_TILE_ROW", "1"));

      // Check that each expectedTre exists and its actual value matches its expected value.
      for (vector<pair<string,string> >::iterator iter = expectedTres.begin(); iter != expectedTres.end(); iter++)
      {
         const DataVariant& actualValue = pTreMetadata->getAttribute(iter->first);

         issearf(actualValue.isValid() == true);
         issearf(actualValue.toDisplayString() == iter->second);
      }

      for (vector<DataElement*>::const_iterator iter = dataElements.begin(); iter != dataElements.end(); iter++)
      {
         WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(
            Service<DesktopServices>()->getWindow((*iter)->getName(), SPATIAL_DATA_WINDOW));
         issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      }

      return success;
   }
};

class NitfDesParsingTestCase : public TestCase
{
public:
   NitfDesParsingTestCase() : TestCase("DesParsing") {}
   bool run()
   {
      bool success = true;

      // Relative path of the file to load
      const string fileName = TestUtilities::getTestDataPath() + "Nitf/i_3130s.ntf";

      vector<DataElement*> dataElements = TestUtilities::loadDataSet(fileName, "NITF Importer", 1);
      issearf(dataElements.size() == 1);

      const DynamicObject* const pMetadata = dataElements.front()->getMetadata();
      issearf(pMetadata != NULL);

      // DES 0 - Custom DES
      const string des0Path[] = {"NITF", "DES", "DES_000", END_METADATA_NAME};
      const DataVariant& des0Metadata = pMetadata->getAttributeByPath(des0Path);
      issearf(des0Metadata.isValid());

      const DynamicObject* const pDes0Metadata = des0Metadata.getPointerToValue<DynamicObject>();
      issearf(pDes0Metadata != NULL);

      const DataVariant& des0DE = pDes0Metadata->getAttribute("DE");
      issearf(des0DE.isValid() && des0DE.toDisplayString() == "DE");

      const DataVariant& des0DESID = pDes0Metadata->getAttribute("DESID");
      issearf(des0DESID.isValid() && des0DESID.toDisplayString() == "BSDESA DES");

      const DataVariant& des0DESVER = pDes0Metadata->getAttribute("DESVER");
      issearf(des0DESVER.isValid() || des0DESVER.toDisplayString() == "99");

      const DataVariant& des0DECLAS = pDes0Metadata->getAttribute("DECLAS");
      issearf(des0DECLAS.isValid() && des0DECLAS.toDisplayString() == "U");

      const DataVariant& des0DESSHL = pDes0Metadata->getAttribute("DESSHL");
      issearf(des0DESSHL.isValid() && des0DESSHL.toDisplayString() == "17");

      const DataVariant& dvDes0DESSHF = pDes0Metadata->getAttribute("DESSHF");
      issearf(dvDes0DESSHF.isValid());

      const Blob* pBlob = dvDes0DESSHF.getPointerToValue<Blob>();
      issearf(pBlob != NULL);
      const vector<unsigned char>& des0DESSHF = pBlob->get();

      unsigned char expectedDes0DESSHF[] = {"YOUPARSEDITRIGHT!"};
      issearf(des0DESSHF.size() == sizeof(expectedDes0DESSHF) - 1);

      for (unsigned int i = 0; i < des0DESSHF.size(); i++)
      {
         issearf(des0DESSHF[i] == expectedDes0DESSHF[i]);
      }

      const DataVariant& dvDes0DESDATA = pDes0Metadata->getAttribute("DESDATA");
      issearf(dvDes0DESDATA.isValid());

      pBlob = dvDes0DESDATA.getPointerToValue<Blob>();
      issearf(pBlob != NULL);
      const vector<unsigned char>& des0DESDATA = pBlob->get();

      unsigned char expectedDes0DESDATA[] = {"This is the BSDESA data!"};
      issearf(des0DESDATA.size() == sizeof(expectedDes0DESDATA) - 1);

      for (unsigned int i = 0; i < des0DESDATA.size(); i++)
      {
         issearf(des0DESDATA[i] == expectedDes0DESDATA[i]);
      }

      // DES 1 - TRE_OVERFLOW
      const string des1Path[] = {"NITF", "DES", "DES_001", END_METADATA_NAME};
      const DataVariant& des1Metadata = pMetadata->getAttributeByPath(des1Path);
      issearf(des1Metadata.isValid());

      const DynamicObject* const pDes1Metadata = des1Metadata.getPointerToValue<DynamicObject>();
      issearf(pDes1Metadata != NULL);

      const DataVariant& des1DE = pDes1Metadata->getAttribute("DE");
      issearf(des1DE.isValid() && des1DE.toDisplayString() == "DE");

      const DataVariant& des1DESID = pDes1Metadata->getAttribute("DESID");
      issearf(des1DESID.isValid() && des1DESID.toDisplayString() == "TRE_OVERFLOW");

      const DataVariant& des1DESVER = pDes1Metadata->getAttribute("DESVER");
      issearf(des1DESVER.isValid() && des1DESVER.toDisplayString() == "99");

      const DataVariant& des1DECLAS = pDes1Metadata->getAttribute("DECLAS");
      issearf(des1DECLAS.isValid() && des1DECLAS.toDisplayString() == "U");

      const DataVariant& des1DESOFLW = pDes1Metadata->getAttribute("DESOFLW");
      issearf(des1DESOFLW.isValid() && des1DESOFLW.toDisplayString() == "IXSHD");

      const DataVariant& des1DESITEM = pDes1Metadata->getAttribute("DESITEM");
      issearf(des1DESITEM.isValid() && des1DESITEM.toDisplayString() == "1");

      const DataVariant& des1DESSHL = pDes1Metadata->getAttribute("DESSHL");
      issearf(des0DESSHL.isValid() && des1DESSHL.toDisplayString() == "0");

      const DataVariant& dvDes1DESDATA = pDes1Metadata->getAttribute("DESDATA");
      issearf(dvDes1DESDATA.isValid());

      pBlob = dvDes1DESDATA.getPointerToValue<Blob>();
      issearf(pBlob != NULL);
      const vector<unsigned char>& des1DESDATA = pBlob->get();

      unsigned char expectedDes1DESDATA[] = {"RSMECA020582_8                                                             "
         "                1097686984-2                            1097686984-1                            YN060701       "
         " -2.42965895449297E+06-4.76049894293300E+06+3.46898407315533E+06+8.90698769551156E-01+2.48664813021570E-01-3.80"
         "554217799520E-01-4.54593996792805E-01+4.87215943350720E-01-7.45630553709282E-01+0.00000000000000E+00+8.37129879"
         "594448E-01+5.47004172461403E-01                                        010203040506                    07+1.000"
         "00000000000E+04+0.00000000000000E+00+0.00000000000000E+00+0.00000000000000E+00+0.00000000000000E+00+0.000000000"
         "00000E+00+0.00000000000000E+00+1.00000000000000E+04+0.00000000000000E+00+0.00000000000000E+00+0.00000000000000E"
         "+00+0.00000000000000E+00+0.00000000000000E+00+1.00000000000000E+04+0.00000000000000E+00+0.00000000000000E+00+0."
         "00000000000000E+00+0.00000000000000E+00+1.00000000000000E-02+0.00000000000000E+00+0.00000000000000E+00+0.000000"
         "00000000E+00+1.00000000000000E-02+0.00000000000000E+00+0.00000000000000E+00+1.00000000000000E-02+0.000000000000"
         "00E+00+1.00000000000000E-0212+1.00000000000000E+00+0.00000000000000E+00+0.00000000000000E+00+0.00000000000000E+"
         "00-1.00008180582392E+00-1.12034941179319E-05-2.10752438266345E-04+5.63463048812097E+02+2.10378883953695E+03-2.4"
         "1611212796172E-01+1.74077606469715E+02+1.30090790755539E-05-9.99948371588978E-01-1.72117126106038E-04-1.9191407"
         "0771260E+03+6.18659651109072E+02-2.05224408741474E-01+2.29140527279625E+02-1.86630976260055E-05+1.3354067211431"
         "0E-05-9.96993670323955E-01+6.54126345905533E+01+6.28447904925560E+01-7.12897716619175E-01+8.65284339742136E+02+"
         "8.11147043037291E-10+5.05833115782289E-09+1.98515896646458E-07+8.89408193457486E-01-2.82198086811556E-01-3.5826"
         "3892248047E-02-2.22004302514752E-01+7.65809972358675E-09+4.51844221980706E-11-2.27444324243187E-07+2.4815429221"
         "2040E-01+9.61896090156108E-01+2.26850142389836E-02+2.44306540762403E-01+4.67478312056560E-09+3.44957253450455E-"
         "09+5.81354683265607E-09-3.79954723401751E-01-3.23454861955480E-02+9.99096833418669E-01-9.45501478705329E-03RSMI"
         "DA016282_8                                                                             1097686984-2            "
         "                                                                                                FRAME          "
         "                         19700101000000.000000                                                          H      "
         "                                                                                                               "
         "                                                                                                               "
         "                        +4.24047939155360E+00+5.78777725848860E-01-1.00674864824209E+03+4.24144365986582E+00+5."
         "78775090401513E-01-1.00674864824209E+03+4.24048136302023E+00+5.79504931428166E-01-1.00674864824209E+03+4.241413"
         "05108606E+00+5.79543706280712E-01-1.00674864824209E+03+4.24046589874976E+00+5.78784982058331E-01+9.932513517579"
         "06E+02+4.24082525985090E+00+5.78783999886410E-01+9.93251351757906E+02+4.24046663362936E+00+5.79056053623058E-01"
         "+9.93251351757906E+02+4.24081385032552E+00+5.79070504034830E-01+9.93251351757906E+02                           "
         "                                                    00000000000092920000000000009122                           "
         "                                                                                                               "
         "                                                                                                               "
         "                                                                                                               "
         "                                                                                 RSMPCA010742_8                "
         "                                                             1097686984-2                            001001+6.1"
         "9186806602187E-01+3.72771990557440E-01+4.64600000000000E+03+4.56100000000000E+03+4.24096939577019E+00+5.7915832"
         "1291012E-01-6.74864824209362E+00+5.59920000000000E+03+5.49720000000000E+03+5.83421668078010E-04+4.6605964138418"
         "7E-04+1.00000000000186E+03111008+4.61255322175402E-01+1.47720777603914E+00+2.13600702305268E-02+0.0000000000000"
         "0E+00+3.88751694388177E-01+0.00000000000000E+00+0.00000000000000E+00+0.00000000000000E+00111008+1.0000000000000"
         "0E+00+5.08155386598166E-02-2.96977305437078E-02+0.00000000000000E+00-4.47362569751380E-01+0.00000000000000E+00+"
         "0.00000000000000E+00+0.00000000000000E+00111008+3.64152616081756E-01-3.70746186946868E-02+1.49556757502953E+00+"
         "0.00000000000000E+00+3.59800851214419E-01+0.00000000000000E+00+0.00000000000000E+00+0.00000000000000E+00111008+"
         "1.00000000000000E+00+5.06210125516810E-02-2.98450105075734E-02+0.00000000000000E+00-4.47409831082355E-01+0.0000"
         "0000000000E+00+0.00000000000000E+00+0.00000000000000E+00RSMPIA005912_8                                         "
         "                                    1097686984-2                            -1.37241587646741E+10+6.80345003035"
         "694E+09-2.53961725272867E+09-2.73740874871126E+04-8.41052141609109E+08+5.97072375992576E+08+6.42450733136118E+0"
         "3+6.77395740580080E+06+2.27081764262021E+02+1.03024536087538E-03-1.99837981040581E+09+4.46286444870403E+08+3.61"
         "714766643827E+09-3.62940014004744E+03+1.95441015791188E+07-1.05767897568751E+09-2.69030105715253E+02+7.66625782"
         "038074E+08+8.24211130197391E+03+9.01748615982117E-04001001001+9.29200000000000E+03+9.12200000000000E+03"};

      issearf(des1DESDATA.size() == sizeof(expectedDes1DESDATA) - 1);
      for (unsigned int i = 0; i < des1DESDATA.size(); i++)
      {
         issearf(des1DESDATA[i] == expectedDes1DESDATA[i]);
      }

      for (vector<DataElement*>::const_iterator iter = dataElements.begin(); iter != dataElements.end(); iter++)
      {
         WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(
            Service<DesktopServices>()->getWindow((*iter)->getName(), SPATIAL_DATA_WINDOW));
         issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      }

      return success;
   }
};

class NitfImportTestCase : public TestCase
{
public:
   NitfImportTestCase() : TestCase("NitfImport") {}
   bool run()
   {
      bool success = true;
      vector<string> fileNames;
      const QString rootWorkingDirectory = QString::fromStdString(TestUtilities::getTestDataPath()) + "Nitf/clevel3/";
      QString workingDirectory = rootWorkingDirectory;

      QFile file("NitfImportFileList.txt");
      issearf(file.open(QIODevice::ReadOnly | QIODevice::Text) == true);

      QTextStream fileText(&file);
      while (fileText.atEnd() == false)
      {
         QString fileName = fileText.readLine();

         // Empty lines and comments.
         if (fileName.isEmpty() == false && fileName.at(0) != '#')
         {
            // First, look for absolute directories or filenames.
            QFileInfo fileInfo(fileName);
            if (fileInfo.isDir() == true)
            {
               workingDirectory = fileName;
               continue;
            }
            else if (fileInfo.isFile() == true)
            {
               fileNames.push_back(fileInfo.fileName().toStdString());
               continue;
            }

            // Second, look for directories or filenames relative to rootWorkingDirectory.
            fileInfo.setFile(rootWorkingDirectory + fileName);
            if (fileInfo.isDir() == true)
            {
               workingDirectory = rootWorkingDirectory + fileName;
               continue;
            }
            else if (fileInfo.isFile() == true)
            {
               fileNames.push_back((rootWorkingDirectory + fileName).toStdString());
               continue;
            }

            // Finally, look for directories or filenames relative to workingDirectory.
            fileInfo.setFile(workingDirectory + fileName);
            if (fileInfo.isDir() == true)
            {
               workingDirectory = workingDirectory + fileName;
               continue;
            }
            else if (fileInfo.isFile() == true)
            {
               fileNames.push_back((workingDirectory + fileName).toStdString());
               continue;
            }

            // Not a relative or absolute file or directory -- update the file list.
            issearf(false);
         }
      }

      for (vector<string>::const_iterator iter = fileNames.begin(); iter != fileNames.end(); iter++)
      {
         QFileInfo fileInfo(QString::fromStdString(*iter));
         QString filename = fileInfo.fileName();
         if (filename.isEmpty() == false)
         {
            std::cout << "    Importing file: " << filename.toStdString() << std::endl;
         }

         vector<DataElement*> pDataElements = TestUtilities::loadDataSet(*iter, "NITF Importer", 1);
         issea(pDataElements.empty() == false);
         for (unsigned int i = 0; i < pDataElements.size(); i++)
         {
            WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(
               Service<DesktopServices>()->getWindow(pDataElements[i]->getName(), SPATIAL_DATA_WINDOW));
            issea(TestUtilities::destroyWorkspaceWindow(pWindow));
         }
      }

      return success;
   }
};

class NitfRpcTestCase : public TestCase
{
public:
   NitfRpcTestCase() : TestCase("Rpc") {}
   bool run()
   {
      bool success = true;
      string fileName = TestUtilities::getTestDataPath() + "Nitf/nine_large.ntf";

      ImporterResource impResource("NITF Importer", fileName);
      issearf(impResource->execute());
      vector<DataElement*> elements = impResource->getImportedElements();
      issearf(elements.size() == 9);

      vector<pair<LocationType, LocationType> > checkVals; // expected, tolerance
      checkVals.push_back(make_pair(LocationType(33.36320294444, 44.35263944444), LocationType(6.5e-3, 6.293e-6)));
      checkVals.push_back(make_pair(LocationType(33.36320294444, 44.35586505556), LocationType(6.3e-6, 3.22e-3)));
      checkVals.push_back(make_pair(LocationType(33.36320294444, 44.35909066667), LocationType(6.3e-6, 6.46e-3)));
      checkVals.push_back(make_pair(LocationType(33.35997733333, 44.35263944444), LocationType(6.31e-6, 6.293e-6)));
      checkVals.push_back(make_pair(LocationType(33.35997733333, 44.35586505556), LocationType(6.31e-6, 3.22e-3)));
      checkVals.push_back(make_pair(LocationType(33.35997733333, 44.35909066667), LocationType(3.22e-3, 3.24e-3)));
      checkVals.push_back(make_pair(LocationType(33.35675172222, 44.35263944444), LocationType(6.46e-3, 3.22e-3)));
      checkVals.push_back(make_pair(LocationType(33.35675172222, 44.35586505556), LocationType(3.24e-3, 6.296e-6)));
      checkVals.push_back(make_pair(LocationType(33.35675172222, 44.35909066667), LocationType(6.31e-6, 6.299e-6)));

      LocationType pixelCheck(256, 256);

      // georeference each dataset
      bool hasNotFailed = success;
      for(vector<DataElement*>::size_type idx = 0; idx < 9; idx++)
      {
         // keep track of failure so the test fails properly, but also make sure we test all the
         // available elements' diffs
         hasNotFailed = hasNotFailed && success;
         success = true;
       
         RasterElement *pElement = dynamic_cast<RasterElement*>(elements[idx]);
         isseac(pElement != NULL);
         ExecutableResource georef("RPC Georeference");
         georef->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pElement);
         isseac(georef->execute());
         LocationType diff = pElement->convertPixelToGeocoord(pixelCheck) - checkVals[idx].first;
         isseac(fabs(diff.mX) < checkVals[idx].second.mX && fabs(diff.mY) < checkVals[idx].second.mY);
      }
      for_each(elements.begin(), elements.end(), boost::bind(&ModelServices::destroyElement, Service<ModelServices>().get(), _1));
      success = success && hasNotFailed;
      return success;
   }
};

class NitfTestSuite : public TestSuiteNewSession
{
public:
   NitfTestSuite() : TestSuiteNewSession( "Nitf" )
   {
      addTestCase(new NitfBadArgsTestCase);
      addTestCase(new NitfTreParsingTestCase);
      addTestCase(new NitfDesParsingTestCase);
      addTestCase(new NitfImportTestCase);
      addTestCase(new NitfRpcTestCase);
   }
};

REGISTER_SUITE( NitfTestSuite )
